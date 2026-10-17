# Q5 log: smooth netplay between unequal machines

Side session `claude-side-q5`, branch `engine/q5-netpace` (from origin/main `0bfab3ac`).
Chrome 155 on this PC (i7-14700K: 8 performance cores with two threads each and 12
efficiency cores; RTX 4070 SUPER; Windows 11), Playwright, the owner's CISO picked as a
file. Q4 benchmarked on the same PC during the session and the owner's own Chrome had
busy tabs open; every run's log has the CPU load at its start and end, and before/after
runs were interleaved.

The owner played online between a fast desktop (i7-14700K + RTX 4070 SUPER, full speed
solo) and a ThinkPad (Ryzen 7 PRO 7840U, Radeon 780M, 32 GB) that "kinda struggled":
online play was "choppy/slow".

## 1. Status

| # | Deliverable | State |
|---|---|---|
| 1 | Netplay statistics overlay: both players' speed, emulation ms per field, frames shown, round trip and jitter, input delay, waits and who waits for whom, underruns; Copy stats | Done (section 3) |
| 2 | Even lockstep pacing at the slower machine's sustainable pace | Done (section 4); measured in section 5 |
| 3 | Automatic input delay from the measured link | Done (section 6) |
| 4 | Picture Auto that respects the GPU | Done (section 7) |
| 5 | A note when this machine is not keeping up | Done (section 8) |
| Q5b | Win back the speed even pacing cost (roles), Auto delay measured twice | Done (section 15) |

## 2. How the measurements were made

- **Two players on one PC.** `netplay_check.mjs` (E5) runs the real server and two pages
  that log in, create and join a game and play E5c's two-player script (`innings.txt`:
  E3a's path into an exhibition game with both players' presses). With this script the
  game now ends up in its attract-mode demo match (the pages' last pictures show "Demo
  Mode"): full in-game rendering (players, crowd), the same on both pages, which is what
  matters here. Both pages run unpaced to VI field 4,000 (`--fast-until`), then paced;
  the measured stretch is fields 4,300 to about 6,900 (frame 6,000), 45-70 s of play.
- **The slower machine, simulated.** Player 2's page runs with Q2's `hostSlow=1.3` or
  `1.6` (its engine threads spin 0.3 or 0.6 times their working time, and DevTools
  throttles its main thread by the same factor): the ThinkPad's stand-in.
- **Two machines, not one.** Each page in its own browser instance (`--separate`: its
  own GPU process), sound on (a click on each page), the presentation trace on, and
  every browser process pinned to the performance cores (`--affinity FFFF`). The first
  runs (one browser, unpinned) had fields 8-22 % dearer than the pinned runs on both
  pages; which cores they ran on was not recorded, so whether the efficiency cores or the
  machine's load then caused it is not established. Those runs are not used.
- **Numbers per page** (`measured` in each summary): game speed (fields per wall second
  over 59.94), emulation ms per field (the Worker's time in `mvp_run_fields`), new frames
  shown per second and the intervals between them (the engine's presents that showed a
  new picture, from the render thread's trace), the share of intervals over 25 and 50
  ms, waits for the other player per second and the share of time waiting (the session
  driver's stalls), audio underruns per minute (the worklet), and the pace.
- A note on "over 25 ms": below full speed, frames cannot come every 16.7 ms on a 60 Hz
  display; at 0.8x a quarter of the intervals are two refreshes (33 ms) however even the
  pacing is. The p95 and the share over 50 ms (three refreshes) show the stutter.

## 3. The statistics overlay

`src/netstats.ts`, wired in `src/main.ts`; `static/index.html` and `style.css`.

- **Opening it:** the **Stats** button in the toolbar, or the backquote key (the key left
  of 1; it is now refused as a game binding, like F, M and P). Off by default; the choice
  is kept in the browser (`localStorage["mvp.overlay.v1"]`). `stats=1` in the URL shows it
  from the start.
- **What it shows**, over the top left of the picture, for this computer and, online, for
  the other player's, over the last 3 seconds:

  | Row | Meaning |
  |---|---|
  | Game speed | VI fields per second over 59.94 (1.00x = full speed) |
  | Emulation per field | the engine Worker's ms per field (16.7 ms is a field: more cannot keep up) |
  | Could run at | the speed this machine could run the game at (the pacer's capacity) |
  | Frames shown | new pictures presented per second |
  | Waiting for the other | online: the share of time this side waited for the other's input, and waits per second |
  | Sound underruns | per minute, and in all |
  | Picture | the internal resolution, and the GPU's ms per frame (timestamp queries) |

  Under the table: the round trip (recent) and its jitter, the input delay, and who holds
  the game back: "Both games run at 0.68x: player 2's computer sets the pace (it could
  run at 0.70x)", or "Full pace, but this side waits for player 2 20% of the time (the
  network, or player 2's computer)", or "Neither computer holds the game back". Solo,
  only this computer's column, and "This computer cannot keep up (it could run the game
  at 0.79x)" when so.
- **Copy stats** puts the same as text on the clipboard (with both machines' thread
  count, GPU adapter and browser, the engine flavour, and the pace with both input
  leads), for the owner to paste into a chat. An example, as the slower page gave it (two
  pages on this PC, player 2 at `hostSlow=1.3`, delay Auto, while another session's build
  loaded the machine; the GPU is the same RTX 4070 SUPER on both):

  ```
  MVP Baseball 2005 online stats, 2026-10-17 05:59:25 UTC
                         You (player 2)  Player 1
  Game speed             0.60x           0.62x
  Emulation per field    26.3 ms         19.4 ms
  Could run at           0.60x           0.81x
  Frames shown           36/s            37/s
  Waiting for the other  0% (0.6/s)      0% (3.9/s)
  Sound underruns        0.0/min (6)     0.0/min (8)
  Picture                2x, GPU 1.0 ms  2x, GPU 1.2 ms
  Round trip 24 ms, jitter 10.9 ms; input delay 2 frames.
  Both games run at 0.59x: this computer sets the pace (it could run at 0.60x).
  This computer: 28 threads, nvidia lovelace, Chrome 155; threaded engine.
  Player 1's computer: 28 threads, nvidia lovelace, Chrome 155; threaded engine.
  Pace: 0.59x (this side proposes 0.59, the other 0.75); input lead 81 ms here, 43 ms there.
  ```
- **How the numbers travel:** each page measures itself from its Worker's reports (stats
  windows twice a second: fields, the run's ms, frames shown, the GPU's time; the
  session's waits; the worklet's underruns) and sends a summary once a second as a text
  message on the data channel (`{"mvp":"stats","v":1,...}`, like Q3's `pad`). The
  receiving page keeps only the known fields with their types (`parsePeerStats`). The
  engine never sees these messages, and nothing in the overlay talks to the engine:
  display only.

## 4. Even pacing

`src/pacer.ts` (the controller and the schedule), `src/worker.ts` (the paced loop),
`net/driver.h` (the input lead), `net/session.h` (when each peer input arrived).

**Before**, each page paced itself to the wall clock at full speed. The slower machine
could not keep up: it ran flat out, behind its clock, and chased, often with two-field
batches that show one picture for two fields (it showed 64-72 % of the fields it ran at
a simulated 1.3x slower machine, 64-66 % at 1.6x). The faster machine ran ahead until it
reached a frame whose input from the slower one had not arrived, waited (stopping its run
at that poll), ran when the input came, and waited again: about 35 waits a second, a
fifth to a third of the time (section 5). Its frames therefore followed the slower machine's inputs as
they arrived over the network, and its schedule, behind by the time it waited, tried to
catch up each time.

**Now**, online:

1. **Capacity.** Each Worker adds up its busy time per paced batch (the run, which
   includes waiting for its render thread, and the present) and the fields it ran; a
   batch that ran no field (waiting) adds its time to the next. Every 15 fields the mean
   cost per field enters a smoothed cost (a time constant of 30 fields, up or down: the
   cost does not depend on the pace, so following it quickly cannot oscillate).
   Capacity = 16.68 ms / cost.
2. **Proposal and pace.** Four times a second each side proposes its capacity times 0.98,
   floored to 0.01 and at most 1; a lower proposal takes effect at once, a higher one
   once it has had room for two more steps for half a second. The proposal, the capacity and the
   input lead go to the other page as a text message (`{"mvp":"pace","v":1,"n",...}`,
   relayed by both pages; older ones ignored, the other side's counts for 2 s). Both sides
   run at the lower proposal.
3. **The schedule** (`Schedule`): field k is due at start + k x 16.68 ms / pace. A change of
   pace keeps the current position. The paced loop runs one field when one is due: online
   at the slot's own time (a timer set after each field; before the final build, at the
   Worker's next animation frame, up to a refresh late), or at once in its own task when
   it is behind. Online the debt is at most two fields (never a two-field batch) and,
   after a wait for the other player's input, one field: a wait moves the schedule on
   instead of being caught up.
4. **Phase.** `NetplayDriver` notes for every frame poll how long before the run needed
   the other player's input that input arrived (needed: when the run reached the poll, or
   when the host loop wanted to run on to it and `may_run()` said no; negative when it
   waited); `mvp_net_lead()` is the median of the last 24 frames. With both sides at the
   same pace, side A's lead minus side B's is twice A's lateness behind B. Each side moves
   its schedule by 60 % a second of half the difference, at most 3 % of the elapsed time,
   so the leads settle equal: each side as far from waiting as the delay allows.
5. **Sound.** The Worker tells the AudioWorklet the pace (`{"type":"pace"}` on its port);
   the resampler's time-stretch starts at that speed (`setPace`, the controller's integral
   term set to it) instead of discovering it from a buffer that runs low; at full speed it
   ends by itself as the buffer fills.
6. **Small things:** a batch that ran no field ends without presenting the same picture
   again (`mvp_batch_end`: the render thread just stops waiting for records), and
   `mvp_run_fields` returns at once while the gate is closed, so a waiting page no longer
   wakes its render thread every 4 ms.

Solo games keep the full-speed schedule and the chase (Q2-P1).

Nothing here can change what the guest computes: the pace, the schedule, the phase, the
reports and the leads decide when fields run, never what is in them; the inputs still
enter only at the frame polls, through the session (section 9).

## 5. Measurements

Before = origin/main `0bfab3ac`'s site; after = this branch (runs `h`-`l`: the build before
the timed slots; `m` and `n`: the final build; the two do not differ beyond the runs'
spread). Delay 3, two browser instances pinned to the performance cores, the
measured stretch fields 4,300-6,900 (section 2). Each pair ran back to back; "CPU load"
is the whole machine's at the start and end of a run (Q4 and the owner's own Chrome were
busy at times). Every run ended with both pages' guest lines and window hashes identical
(section 9).

### 5.1 One player's machine 1.3x slower (the ThinkPad's stand-in)

Means over five pairs (the range of the speed in brackets):

| | Player | Speed | New frames shown /s | New-frame interval p95 | > 25 ms | > 50 ms | Waits /s | Waiting | Underruns /min |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| before | P1 (fast) | 0.811x (0.766-0.892) | 47.2 | 38.8 ms | 24.0 % | 2.3 % | 34.7 | 18.6 % | 0.2 |
| before | P2 (1.3x slower) | | 33.5 | 53.0 ms | 54.0 % | 6.9 % | 2.3 | 0.1 % | 0.9 |
| after | P1 (fast) | 0.724x (0.683-0.744) | 43.0 | 39.8 ms | 32.8 % | 1.8 % | 1.6 | 0.2 % | 0.0 |
| after | P2 (1.3x slower) | | 43.0 | 37.5 ms | 28.8 % | 1.0 % | 0.4 | 0.1 % | 0.0 |

| Run | Player (machine) | Speed | Emulation ms/field | New frames shown /s | New-frame interval p95 | > 25 ms | > 50 ms | Waits /s | Waiting | Underruns /min | Pace | CPU load |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| h_b13 | P1 (1.0x) | 0.781x | 15.9 | 45.2 | 41.1 ms | 25.2 % | 3.1 % | 38.1 | 21.8 % | 1.1 | - | 3%-1% |
| h_b13 | P2 (1.3x) | 0.782x | 20.4 | 33.6 | 57.9 ms | 48.0 % | 8.8 % | 5.2 | 0.4 % | 2.2 | - | 3%-1% |
| h_n13 | P1 (1.0x) | 0.741x | 16.4 | 44.1 | 38.1 ms | 30.9 % | 1.2 % | 1.4 | 0.1 % | 0.0 | 0.79 | 0%-90% |
| h_n13 | P2 (1.3x) | 0.740x | 20.8 | 44.1 | 36.0 ms | 25.9 % | 0.4 % | 0.5 | 0.0 % | 0.0 | 0.79 | 0%-90% |
| i_n13 | P1 (1.0x) | 0.744x | 15.9 | 44.3 | 38.4 ms | 31.1 % | 1.6 % | 1.7 | 0.3 % | 0.0 | 0.8 | 8%-3% |
| i_n13 | P2 (1.3x) | 0.744x | 20.3 | 44.2 | 36.2 ms | 27.3 % | 0.6 % | 0.5 | 0.3 % | 0.0 | 0.81 | 8%-3% |
| i_b13 | P1 (1.0x) | 0.892x | 13.8 | 52.1 | 34.9 ms | 17.0 % | 1.2 % | 23.9 | 11.5 % | 0.0 | - | 15%-20% |
| i_b13 | P2 (1.3x) | 0.891x | 17.7 | 38.7 | 46.7 ms | 43.4 % | 3.6 % | 2.7 | 0.2 % | 2.5 | - | 15%-20% |
| k_b13 | P1 (1.0x) | 0.820x | 15.3 | 47.7 | 38.5 ms | 23.3 % | 2.3 % | 35.6 | 17.7 % | 0.0 | - | 76%-77% |
| k_b13 | P2 (1.3x) | 0.820x | 19.6 | 32.8 | 53.6 ms | 57.5 % | 6.6 % | 1.3 | 0.0 % | 0.0 | - | 76%-77% |
| k_n13 | P1 (1.0x) | 0.735x | 16.6 | 43.6 | 39.6 ms | 31.7 % | 1.8 % | 1.5 | 0.1 % | 0.0 | 0.78 | 74%-2% |
| k_n13 | P2 (1.3x) | 0.735x | 20.8 | 43.8 | 37.5 ms | 27.4 % | 1.3 % | 0.4 | 0.0 % | 0.0 | 0.78 | 74%-2% |
| l_n13 | P1 (1.0x) | 0.683x | 17.3 | 40.6 | 43.4 ms | 38.8 % | 3.0 % | 1.8 | 0.5 % | 0.0 | 0.75 | 5%-44% |
| l_n13 | P2 (1.3x) | 0.683x | 21.8 | 40.6 | 40.8 ms | 34.5 % | 2.0 % | 0.4 | 0.2 % | 0.0 | 0.74 | 5%-44% |
| l_b13 | P1 (1.0x) | 0.766x | 16.2 | 44.3 | 40.8 ms | 30.2 % | 2.4 % | 40.3 | 22.7 % | 0.0 | - | 48%-94% |
| l_b13 | P2 (1.3x) | 0.765x | 21.0 | 30.4 | 53.8 ms | 61.2 % | 7.9 % | 0.8 | 0.0 % | 0.0 | - | 48%-94% |
| m_b13 | P1 (1.0x) | 0.797x | 15.4 | 46.6 | 38.8 ms | 24.2 % | 2.3 % | 35.4 | 19.2 % | 0.0 | - | 22%-20% |
| m_b13 | P2 (1.3x) | 0.797x | 20.1 | 31.8 | 53.2 ms | 59.9 % | 7.4 % | 1.3 | 0.1 % | 0.0 | - | 22%-20% |
| m_t13 | P1 (1.0x) | 0.715x | 16.7 | 42.5 | 39.3 ms | 31.5 % | 1.3 % | 1.5 | 0.1 % | 0.0 | 0.75 | 43%-54% |
| m_t13 | P2 (1.3x) | 0.715x | 21.5 | 42.5 | 37.2 ms | 29.1 % | 0.5 % | 0.4 | 0.0 % | 0.0 | 0.76 | 43%-54% |

### 5.2 One player's machine 1.6x slower

Means over three pairs:

| | Player | Speed | New frames shown /s | New-frame interval p95 | > 25 ms | > 50 ms | Waits /s | Waiting | Underruns /min |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| before | P1 (fast) | 0.678x (0.596-0.722) | 40.0 | 46.0 ms | 42.9 % | 4.3 % | 35.6 | 34.8 % | 0.0 |
| before | P2 (1.6x slower) | | 26.3 | 61.6 ms | 73.9 % | 19.5 % | 1.2 | 0.1 % | 0.0 |
| after | P1 (fast) | 0.683x (0.609-0.735) | 40.7 | 42.2 ms | 42.7 % | 2.4 % | 1.6 | 0.1 % | 0.0 |
| after | P2 (1.6x slower) | | 40.7 | 39.1 ms | 36.2 % | 1.1 % | 0.3 | 0.0 % | 0.0 |

| Run | Player (machine) | Speed | Emulation ms/field | New frames shown /s | New-frame interval p95 | > 25 ms | > 50 ms | Waits /s | Waiting | Underruns /min | Pace | CPU load |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| i_b16 | P1 (1.0x) | 0.722x | 13.9 | 42.8 | 41.8 ms | 34.6 % | 3.1 % | 35.9 | 32.4 % | 0.0 | - | 10%-5% |
| i_b16 | P2 (1.6x) | 0.723x | 22.3 | 28.0 | 58.0 ms | 68.5 % | 12.3 % | 3.0 | 0.3 % | 0.0 | - | 10%-5% |
| i_n16 | P1 (1.0x) | 0.735x | 13.7 | 43.8 | 39.4 ms | 35.6 % | 1.3 % | 1.5 | 0.1 % | 0.0 | 0.78 | 17%-0% |
| i_n16 | P2 (1.6x) | 0.735x | 21.1 | 43.8 | 36.5 ms | 24.1 % | 0.4 % | 0.4 | 0.0 % | 0.0 | 0.78 | 17%-0% |
| k_n16 | P1 (1.0x) | 0.706x | 14.0 | 42.1 | 39.3 ms | 39.1 % | 1.9 % | 1.6 | 0.1 % | 0.0 | 0.75 | 17%-1% |
| k_n16 | P2 (1.6x) | 0.706x | 22.0 | 42.0 | 37.5 ms | 30.0 % | 0.7 % | 0.3 | 0.0 % | 0.0 | 0.75 | 17%-1% |
| k_b16 | P1 (1.0x) | 0.716x | 13.5 | 41.9 | 45.6 ms | 36.6 % | 3.6 % | 39.1 | 37.0 % | 0.0 | - | 6%-6% |
| k_b16 | P2 (1.6x) | 0.716x | 22.5 | 28.1 | 57.3 ms | 65.7 % | 12.1 % | 0.4 | 0.0 % | 0.0 | - | 6%-6% |
| m_t16 | P1 (1.0x) | 0.609x | 16.1 | 36.3 | 48.0 ms | 53.5 % | 4.0 % | 1.7 | 0.2 % | 0.0 | 0.64 | 65%-27% |
| m_t16 | P2 (1.6x) | 0.609x | 25.5 | 36.2 | 43.4 ms | 54.5 % | 2.1 % | 0.3 | 0.0 % | 0.0 | 0.64 | 65%-27% |
| m_b16 | P1 (1.0x) | 0.596x | 16.8 | 35.4 | 50.6 ms | 57.4 % | 6.3 % | 31.9 | 34.9 % | 0.0 | - | 24%-34% |
| m_b16 | P2 (1.6x) | 0.596x | 27.1 | 22.8 | 69.4 ms | 87.4 % | 34.2 % | 0.3 | 0.1 % | 0.0 | - | 24%-34% |

### 5.3 Equal machines, other conditions

| Run | Player (machine) | Speed | Emulation ms/field | New frames shown /s | New-frame interval p95 | > 25 ms | > 50 ms | Waits /s | Waiting | Underruns /min | Pace | CPU load |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| k_a13 | P1 (1.0x) | 0.841x | 13.8 | 49.9 | 35.0 ms | 23.1 % | 0.7 % | 2.8 | 0.1 % | 0.0 | 0.91 | 4%-3% |
| k_a13 | P2 (1.3x) | 0.841x | 17.6 | 50.0 | 33.2 ms | 17.1 % | 0.3 % | 1.5 | 0.0 % | 0.0 | 0.91 | 4%-3% |
| j_a13 | P1 (1.0x) | 0.583x | 20.1 | 34.7 | 51.2 ms | 54.6 % | 5.7 % | 2.7 | 0.9 % | 0.0 | 0.63 | 74%-3% |
| j_a13 | P2 (1.3x) | 0.583x | 25.8 | 34.7 | 47.9 ms | 55.8 % | 3.7 % | 1.2 | 0.4 % | 0.0 | 0.62 | 74%-3% |
| n_t13 | P1 (1.0x) | 0.679x | 17.7 | 40.4 | 49.0 ms | 34.9 % | 4.5 % | 1.8 | 0.1 % | 0.0 | 0.77 | 81%-100% |
| n_t13 | P2 (1.3x) | 0.679x | 22.6 | 40.4 | 47.8 ms | 31.3 % | 3.8 % | 0.3 | 0.1 % | 0.0 | 0.77 | 81%-100% |
| n_b13 | P1 (1.0x) | 0.481x | 27.5 | 28.4 | 62.5 ms | 76.0 % | 14.4 % | 25.1 | 17.2 % | 0.0 | - | 100%-88% |
| n_b13 | P2 (1.3x) | 0.481x | 33.9 | 19.0 | 85.3 ms | 97.5 % | 51.9 % | 0.2 | 0.0 % | 0.0 | - | 100%-88% |
| h_b10 | P1 (1.0x) | 0.873x | 17.4 | 44.6 | 45.6 ms | 23.6 % | 3.7 % | 28.9 | 3.8 % | 2.5 | - | 5%-53% |
| h_b10 | P2 (1.0x) | 0.872x | 17.3 | 43.5 | 47.0 ms | 28.5 % | 3.9 % | 4.1 | 0.7 % | 1.2 | - | 5%-53% |
| h_n10 | P1 (1.0x) | 0.911x | 15.7 | 54.0 | 30.9 ms | 9.2 % | 0.2 % | 0.6 | 0.0 % | 0.0 | 1 | 68%-76% |
| h_n10 | P2 (1.0x) | 0.912x | 15.9 | 54.0 | 30.6 ms | 9.3 % | 0.2 % | 0.6 | 0.0 % | 0.0 | 1 | 68%-76% |
| c1_b10 | P1 (1.0x) | 0.975x | 14.2 | 51.9 | 34.8 ms | 13.6 % | 0.9 % | 1.8 | 0.2 % | 0.0 | - | 12%-28% |
| c1_b10 | P2 (1.0x) | 0.975x | 14.3 | 54.4 | 33.3 ms | 9.4 % | 0.7 % | 7.0 | 0.2 % | 1.4 | - | 12%-28% |
| c1_n10 | P1 (1.0x) | 0.928x | 14.5 | 54.0 | 32.1 ms | 10.8 % | 0.6 % | 0.5 | 0.1 % | 0.0 | 1 | 30%-14% |
| c1_n10 | P2 (1.0x) | 0.928x | 14.6 | 52.8 | 34.1 ms | 12.0 % | 1.1 % | 0.6 | 0.0 % | 0.0 | 1 | 30%-14% |

- **Equal machines** (`h_*10`: the machine was busy, 53-76 % at the ends; `c1_*10`: the
  first tuning): the pace stays 1 and nothing changes but the waits (1.8-29 a second
  before, 0.5-0.6 after) and the smoothness under load (`h`: p95 31 ms and 0.2 % over
  50 ms after, against 46-47 ms and 3.7-3.9 % before).
- **Auto delay** (`k_a13`, a quiet machine): both pages measured a 0.5-0.6 ms round
  trip and chose D = 2; 0.841x, 50 new frames a second on both, p95 33-35 ms, 0.3-0.7 %
  over 50 ms, two waits a second, input latency 48 ms (D = 3 runs: 77 ms at their
  pace). A different D makes the inputs land on other frames, so the game's own path
  (and its cost) differs from the D = 3 runs (another guest line, equal to native's
  D = 2 line: section 9); its speed is not comparable with theirs. `j_a13` is the same
  game with the machine busy (74 % at its start) and the overlay on (its Copy stats text
  is in section 3).
- **The machine fully loaded** (`n_*13`: another session's build at 80-100 % during
  both runs; one pair): before, the slower page showed 19 new frames a second with half
  of its intervals over 50 ms; after, 40 a second with 3.8 % over 50 ms, and the game ran
  at 0.68x instead of 0.48x.

### 5.4 What it costs, and why

- **The slower machine's frames:** 33.5 -> 43.0 new frames a second at 1.3x and 26.3 ->
  40.7 at 1.6x; intervals over 50 ms (three refreshes and more) 6.9 -> 1.0 % and 19.5 ->
  1.1 %; p95 53 -> 38 ms and 62 -> 39 ms. Before, it showed 64-72 % of the fields it ran
  (two-field batches); after, 99 %.
- **The faster machine:** waits 35 -> 1.6 a second (the share of time waiting 19-35 % ->
  0.1-0.2 %), intervals over 50 ms 2.3 -> 1.8 % and 4.3 -> 2.4 %; it shows every field
  it runs (it did before too, but in the slower one's rhythm, as its inputs came).
- **Sound:** no underruns after in any run (0.2-0.9 a minute before at 1.3x).
- **Speed:** at 1.6x the same (0.678x before, 0.683x after: +2 %, -1 %, +2 % per pair).
  At 1.3x 11 % slower (0.811x -> 0.724x; per pair -5, -17, -10, -11, -10 %). The pacing
  leaves the slower machine idle 4-10 % of the time (busy 90-93 % against 95-96 % before:
  the 2 % margin, the steps, and fields that start late and are not all caught up), and
  in these pairs its fields cost 2-15 % more than in the paired run (the faster machine's
  too, by as much, which points at the machine's load at the time rather than the pacing;
  at 1.6x the difference was -6 to +3 %). At 1.6x the old chase lost as much: the slower
  page's fields cost it 2-6 % more there, and its two-field batches showed a picture for
  only two thirds of them. At 1.3x the old chase could reach full speed in the lighter scenes, which an
  even pace at 98 % of the measured capacity gives up. `paceMargin=X` (development) sets
  the margin; 1.0 was tried once with the previous tuning (0.780x against 0.851x at 0.98
  in the same round) and was not better.

### 5.5 Solo

Solo games keep Q2's pacing (the chase); what Q5 adds there is the GPU's timing on the
render thread, the frames-shown counter and the overlay (off by default). CLK2's CPU vs CPU
session, paced, fields 8,000-10,400 (40 s of play), pinned to the performance cores, two
interleaved rounds; Q2's `measure.py` (the engine's presents) and `audio.py`:

| Run | Machine | Speed | Host ms per field | New frames shown /s | Present interval p95 | > 25 ms | Underruns /min | CPU load |
|---|---|---:|---:|---:|---:|---:|---:|---|
| before (o) | this PC | 1.000x | 12.22 | 57.9 | 27.6 ms | 7.2 % | 0.5 | 11-43 % |
| after (o) | this PC | 0.997x | 12.51 | 58.5 | 26.9 ms | 6.5 % | 0.0 | 73-4 % |
| after (p) | this PC | 1.000x | 12.85 | 56.7 | 21.8 ms | 3.8 % | 2.1 | 48-77 % |
| before (p) | this PC | 1.000x | 12.98 | 58.0 | 26.0 ms | 5.6 % | 1.0 | 57-57 % |
| after (o) | 1.6x slower | 0.715x | 23.24 | 26.9 | 65.7 ms | 71.4 % | 1.0 | 0-25 % |
| before (o) | 1.6x slower | 0.822x | 19.73 | 29.7 | 48.8 ms | 71.3 % | 0.5 | 35-33 % |
| before (p) | 1.6x slower | 0.757x | 21.30 | 28.5 | 53.6 ms | 70.9 % | 0.0 | 18-57 % |
| after (p) | 1.6x slower | 0.801x | 20.75 | 30.1 | 51.9 ms | 63.7 % | 0.5 | 59-40 % |

No difference beyond the machine's own (the slowed runs varied 0.72x-0.82x within each
build). All eight ended at the same guest line (67,465,866,125 instructions, MEM1
`b0263b80...`). An earlier interleaved A/B of the GPU timing alone (`gpuTimer=0` against
on, four runs) gave 11.5-12.5 ms per field either way.

## 6. The input delay, chosen from the link

`src/delay.ts`, `src/netplay.ts`.

- The lobby's delay choice now starts at **Auto** (2 to 6 remain as manual choices). The
  room keeps the creator's choice as before; Auto is `delay: 0` (the server accepts 0 to
  10 already, so it needed no change; `lobby_protocol.d.ts` says so).
- In an Auto game, once the data channel is open, each page sends 20 pings 50 ms apart
  (`{"mvp":"ping","v":1,"id":k}`), the other page answers each at once (`pong`), and after
  600 ms more it picks D from the round trips: the median round trip halved, plus 10 ms
  (the page-to-Worker hop and a field's run), plus the one-way jitter (half the spread
  from the fastest to the 90th-percentile round trip), divided by 18 ms (a frame of the
  game's pace) and rounded up, between 2 and 6. That is E5's rule of thumb (E5_LOG
  section 4). The page then starts its engine session with that D; the session takes the
  larger of the two proposals, as before, so both games use one D, agreed in the HELLO
  before the game starts. Without answers it takes 3.
- The status line says "Measured the connection: round trip 2 ms, jitter 0 ms: input
  delay 2." Manual choices skip the measurement.
- Results: on this PC (two browsers, one machine) both pages measured round trips of
  0.5-0.6 ms (jitter 0.3-0.8 ms) and chose D = 2, in each of three games (`j_a13`,
  `k_a13`, `l_a16`); both pages' guest lines and window hashes were identical and equal to
  native `mvp-netplay solo --delay 2` (section 9). With the even pacing, D = 2 on this
  local link: 50 new frames a second on both pages, two waits a second, an input latency
  of 48 ms (77 ms with D = 3 at its pace; section 5.3). The rule gives 4 on E5's simulated
  40-50 ms one-way link (`tests/delay.test.mjs`); a real internet link was not tried
  here.

## 7. Picture Auto and the GPU

`gfx/wgpu/gpu_timer.h/.cpp`, `gfx/wgpu/gpu.cpp` (the feature request), the renderer's and
the window's render passes, `wasm/render_side.cpp`, `src/autoscale.ts`, `src/main.ts`.

- **The GPU's time.** The first attempt measured how long the GPU queue took to finish a
  present's work (`onSubmittedWorkDone`). On the RTX 4070 SUPER at 2x it read 13 ms on
  average, 38 % of frames over 14 ms: in Chrome that latency follows the GPU process's
  schedule and the display, not the GPU's work, and Auto would have stepped a fast GPU
  down. Replaced by timestamp queries: the browser engine asks for the device feature
  `timestamp-query` when the adapter has it (Chrome does, rounding to 100 us); every
  render pass (the EFB passes, EFB copies, the scan-out mix, the present, and the
  development load) writes a timestamp at its start and end; at each present the stamps
  are resolved into a buffer that is read back without waiting (three read-backs in
  flight at most), and the sum of the passes' durations is that frame's GPU time. A pass
  still open when the render thread presents is submitted first (the pass is split in
  two; what is drawn is the same).
- **Auto's ceiling** (`AutoScale`): from the reports twice a second, the GPU's busy share
  (the mean GPU time of the frames timed, times every frame presented, over the wall
  time: under a heavy load the read-backs come late and some frames go untimed). Busy more than 85 % in every report for 2 s: one scale
  down. Up again only after a hold (20 s, doubling when a step up is undone within a
  minute, at most 5 minutes) and when the next scale should fit: busy share x ((s + 1) /
  s)^2 under 60 % for 5 s. Only for the Auto setting, only with timestamps; the log says
  "picture: Auto lowered to at most 2x (the GPU was busy 97% of the time at 3x)".
- **Testing it** with `gpuLoad=N` (development): N extra render passes per present over
  a texture of the picture's internal size, each sampling it 25 times per pixel, so the
  load grows with the square of the scale, as a heavier scene or a weaker GPU would.
  Results, Chrome 155, a 2x device pixel ratio (`--dpr 2`: Auto picks 3x by size), the
  boot, intro and menus to field 2,400, paced, single runs:

  | Run | Scale | GPU ms per frame | GPU busy | Speed | New frames shown /s | Present p95 |
  |---|---|---:|---:|---:|---:|---:|
  | no load, Auto (picks 2x at dpr 1) | 2x | 1.2 | 7 % | 1.00x | - | - |
  | `gpuLoad=100`, Auto | 3x | 12.0 | 70 % | 1.00x | - | - |
  | `gpuLoad=150`, 3x fixed | 3x | 18.2 | 98 % | 1.00x | 52.5 | 28.6 ms |
  | `gpuLoad=150`, Auto | 3x, then 2x after 3.8 s | 7.9 at 2x | 48 % | 1.00x | 58.3 | 29.5 ms |

  Auto stepped down once ("picture: Auto lowered to at most 2x (the GPU was busy 98% of
  the time at 3x)") and stayed at 2x: 3x would need about 108 % (48 % x 2.25), over the
  60 % that a step up needs. On this GPU the game itself costs 1-2 ms a frame at 2x, so
  without the artificial load Auto never steps down here. The step-up path and the hold's
  doubling are covered by `tests/autoscale.test.mjs` (not seen live: the load is fixed for
  a run). Chrome 155 offers `timestamp-query` without flags, headed and headless
  (checked); Firefox and Safari were not checked, and without the feature Auto stays as
  Q1 made it.

## 8. The note for a slow machine

`src/main.ts`, `static/index.html`.

A small note under the toolbar, with a close button, once the game has run 15 s past its
first frame (not paused, the overlay's numbers covering at least 2.5 s):

- solo, when the game runs below 0.93x and this machine's capacity is below full speed:
  "This computer is running the game at 0.85× speed. On a laptop, plugging in and
  choosing the Best performance power mode helps.";
- online, when this side sets a pace below 0.93: "This computer is running the game at
  0.70× speed, so the online game runs at that pace for both players. On a laptop, ...".

It hides itself 10 s after the condition ends; dismissed, it stays away for the visit.
The faster player is not told to change anything; their status line says which computer
sets the pace. Results: in the online game `j_a13` the slower page showed "This computer is
running the game at 0.60× speed, so the online game runs at that pace for both players.
On a laptop, plugging in and choosing the Best performance power mode helps." and the
faster page showed nothing. Solo at a simulated 1.6x slower machine it said "This computer
is running the game at 0.64× speed. On a laptop, plugging in and choosing the Best
performance power mode helps." (`o_n16`); at full speed it did not appear.

## 9. Exactness

The pacing, the statistics, the delay choice and the picture scale are host-side: they
change when fields run, what is shown and how, never what a field computes. Checked:

- **Guest lines, solo** (Chrome 155, unpaced, the final build's engine), equal to WEB.md's
  CLK2 checkpoints, threaded and single-threaded (`threads=0`):

  | Run | Guest line (both engines) |
  |---|---|
  | 700 fields, no input | 4,547,010,125 instructions, time base 473,646,888, 360 polls, MEM1 `53221c4a...` |
  | 2,400 fields, START x4 | 15,574,026,125, 1,622,294,388, 1,882 polls, MEM1 `4285f135...` |
  | 3,600 fields, `menus` | 23,357,802,126, 2,433,104,388, 2,747 polls, MEM1 `40a7692d...` |

  CLK2's CPU vs CPU session to field 10,400 (paced, solo runs of section 5.5) gave
  67,465,866,125 instructions, MEM1 `b0263b80...` with the GPU timing on and off alike.
- **Netplay, paced, mismatched machines:** `netplay_check` with E5c's exhibition script to
  frame 3,300, delay 3, player 2 at `hostSlow=1.6`, two browser instances: both pages
  27,484,589,886 instructions, MEM1 `58f2ab77...`, equal to native `mvp-netplay solo` (as in
  Q2); 55 windows compared, no desync, no gate miss.
- **Every measured game** (48 two-page games to frame 6,000, before and after, at 1.0/1.0,
  1.0/1.3 and 1.0/1.6, every tuning): both pages identical in guest line and all 100
  window hashes, no desync, no gate miss; the 45 with delay 3 all ended at 44,816,368,162
  instructions, MEM1 `992cb7bd...`, the three with Auto (D = 2) at 44,822,998,467, MEM1
  `cf2da995...`, each equal to native `mvp-netplay solo` with that delay.
- The pacing, the reports, the leads and the delay's measurement run in the page and the
  Worker's host loop; the engine's additions are host statistics (`net/driver.h`'s lead,
  `net/session.h`'s arrival times and jitter) and the renderer's timing; nothing reaches
  guest memory or the inputs.

## 10. Tests

- Native (`ctest --preset msvc`): `test_net_session` gains "when the peer's inputs
  arrived, and the round trip's jitter"; `test_net_driver` gains "the input lead:
  negative on the side that waits, positive on the other". All 47: 46 passed,
  `local_disasm` skipped as always (no dtk listing); the local ones with the disc include
  `local_netplay_lockstep` and `local_netplay_lossy`.
- Emscripten (`ctest --preset emscripten`, the final build): all 40: 39 passed,
  `local_disasm` skipped; the local ones with the disc include `local_lockstep`,
  `local_boot_twice`, `local_input_start` and `local_audio_determinism`.
- The web shell (`test_web_shell`, node --test on the compiled shell): 87 of 87, with
  four new files:
  - `tests/pacer.test.mjs` (9): capacity (windows, quick up, slow down, the mean not the
    peaks), the proposal (margin, steps, lowered at once, raised after room for two steps
    for 1.5 s), the common pace and stale or older reports, the phase step and its slew
    limit, the schedule (pace changes keep the position, debts, shifts), and two players
    simulated in lockstep on one clock: a 1.45x and a 0.70x machine run evenly at 0.68 with
    intervals within 2 ms of the period and no waits (the earlier full-speed schedule:
    the faster side waits about 40 times a second); the phase settles with equal leads
    from offsets of -60, 0 and +45 ms over a jittery link; back to full speed when the
    slower machine keeps up again and down at once when it cannot; a 30 ms + 15 ms
    jitter link at delay 4.
  - `tests/delay.test.mjs` (5): the rule on round trips from a local link (2) to E5's
    40-50 ms one way (4) and beyond; outliers; the bounds; too few answers.
  - `tests/autoscale.test.mjs` (6): a GPU that cannot keep up at 3x (down to 2x in about
    two seconds, and it stays); a step counts only once the renderer draws at it; a
    fast GPU (3x kept); down to 1x step by step and back up after the hold; the hold
    doubling when a step up is undone; the size's limit and no reports.
  - `tests/netstats.test.mjs` (4): the tracker's numbers (speed, emulation per field,
    frames shown, waiting, underruns, GPU) and the pause; the rows and notes (who sets
    the pace, who waits, solo); the Copy stats text; a peer's summary as received.

## 11. Tools

- `netplay_check.mjs`: `--params`, `--params1`, `--params2`, `--host-slow1/2 N[@F]`,
  `--audio`, `--trace-present`, `--fast-until F`, `--measure-from/to`, `--separate`,
  `--affinity HEXMASK`, `--delay auto`; the summary's `measured` (per page) and
  `statsWindows`, and each page's Copy stats text (with `stats=1`), note and measured link.
- `browser_check.mjs`: `--dpr N`; the GPU's time, the scale and frames shown per window;
  the summary's `hint`, `autoScale` and picture/WebGPU-error log lines.
- Page parameters (development): `gpuLoad=N`, `gpuTimer=0`, `paceMargin=X`, `stats=1`.
- Engine exports: `mvp_net_lead`, `mvp_batch_end`, `mvp_gpu_load`, `mvp_gpu_timing`.
- Session scratch (not committed): `nrun.sh` (a two-page run), `campaign*.sh`,
  `table.py` (the tables here), `wins.py` and `gpuwin.py` (per-window views), `exact.sh`
  (the WEB.md checkpoints), `srun.sh` (a solo run), `sim.mjs` (the lockstep simulation).

## 12. Commits

On `engine/q5-netpace`, oldest first (not pushed; the lead integrates):

- `818a2afbd` engine(net): the input lead, the peer's input arrival times and the round
  trip's jitter
- `b78ddb763` engine(web): netplay_check measures mismatched machines
- `abc486172` engine(web): online games run evenly at the slower machine's pace
- `0e5a4116c` engine(web): the input delay chosen from the measured link
- `f587fc62a` engine(web): the statistics overlay, Copy stats, and a note for a slow
  machine
- `cb54c0533` engine(gfx): the GPU's time from timestamp queries; Picture Auto steps down
  for a busy GPU
- `56dada522` engine(web): the pace follows the scene both ways; one field at a time
  online; a steadier slow-machine note
- `eda752332` engine(web): browser_check --dpr; the checks record the GPU's time, the note
  and the overlay text
- `e4c2ce4f2` engine(web): online fields start at their own slot times
- this log, NETPLAY.md and WEB.md (the docs commit), then a merge of origin/main
  (`b34b45245`, a docs-only commit about mods: nothing to rebuild or re-measure)

## 13. Decisions to record (for the lead)

- **Q5-N1, even lockstep pacing:** online, both players run at one common pace: each
  side proposes its measured capacity (a field's time over its smoothed host cost per
  field) times 0.98, in steps of 0.01, at most full speed; both take the lower proposal
  (reports four times a second as text messages on the data channel). The paced loop
  runs one field per slot at that pace; a wait for the other player moves the schedule
  on; a side behind its schedule catches up one field at a time (at most two fields);
  each side moves its schedule toward equal input leads (`net/driver.h`). Solo games keep
  the full-speed chase (Q2-P1). Host timing only. Trades some speed for evenness
  (section 5).
- **Q5-N2, input delay Auto:** the lobby's default; 20 pings over the data channel and E5's
  rule (D x 18 ms >= half the median round trip + 10 ms + the one-way jitter), 2 to 6;
  the room's delay 0 means Auto; a manual choice overrides it; the session still takes
  the larger proposal.
- **Q5-N3, the data channel's text messages:** `pace` (4 Hz), `stats` (1 Hz), `ping`/`pong`
  (before an Auto game) join Q3's `pad`; JSON with `v: 1`; display and host timing only;
  the engine never sees them; the wire protocol stays version 1.
- **Q5-W1, the statistics overlay:** the Stats button and the backquote key (refused as a
  game binding), off by default and kept per browser; Copy stats for field reports.
- **Q5-W2, the note for a slow machine:** dismissible; shown when this computer runs below
  0.93x for 5 s (solo), or sets an online pace below it.
- **Q5-G1, the GPU's time and Auto:** the browser engine requests `timestamp-query` where
  the adapter has it and times its render passes; Picture Auto steps one scale down
  while the GPU is busy over 85 % for 2 s and back up after a hold (20 s, doubling when
  undone within a minute) when the next scale should stay under 60 %. Fixed settings are
  never changed. The queue-latency measure (`onSubmittedWorkDone`) is not a GPU time in
  Chrome and is not used.
- **Q5-T1, measuring two machines on one PC:** two browser instances (`--separate`), pinned
  to the performance cores, and nothing else heavy running (a build on the efficiency
  cores at low priority still halved the pages' speed).

## 14. Next

1. **Field data from the owner.** Play online desktop + ThinkPad as before, press Stats
   (or the ` key) on both, then Copy stats on each and paste both into the chat. That
   says which machine sets the pace, its capacity, the GPU's time at its scale, the round
   trip and the delay Auto chose, and whether anyone still waits.
2. **The slower machine's capacity is the limit** for both players; Q4's speed work raises
   it directly. Even pacing costs some of it (section 5.4): 11 % at a 1.3x slower partner
   in these runs, none at 1.6x. Fields run one at a time cost the slower machine a little more
   than two-field batches; a way to keep the render thread busy between single fields
   would win that back.
3. **Below full speed, a 60 Hz display shows frames for one or two refreshes:** at 0.8x a
   quarter of the intervals are 33 ms however even the pacing. Snapping the pace to 1,
   3/4, 2/3 of the refresh rate would make the cadence regular at the price of speed;
   worth trying only if the owner finds the alternation visible.
4. Picture Auto on the real 780M: whether 3x is GPU-bound there, and Firefox's and
   Safari's timestamp queries (not checked).
5. Rollback (after K5) would hide the input delay; the pacing and the lead measurement
   would carry over.

## 15. Q5b: winning back the speed (roles)

Branch `engine/q5b-pace` (from origin/main `ad92bce12`, Q5 merged; later merged with Q4's
`10118e111`). The lead's follow-up: at a 1.3x slower partner Q5 had measured 0.81x -> 0.72x
(section 5.1). The task was to win that speed back without losing the evenness: at 1.0 vs 1.3,
at least 0.80x with the slower page's intervals over 50 ms at most 2 % and the faster page's
waits low; no regression at the second scenario (1.6x, and 2x once Q4 had landed).

### 15.1 Where the time went

New statistics first. Every stats window (`worker.ts`) now carries:

- the schedule time the paced loop gives up: after a wait, when behind by more than its debt,
  and when far behind;
- the time spent spinning.

The pace state also carries the role, the trim and the schedule's own pace. `q5b.py` in the
scratch reads them. With these numbers and single-page experiments:

- **The slower machine sat idle.** Q5 left it busy 91-92 % of the wall time; the chase kept it
  at 95.7 %. The idle came from four places:
  - the 2 % margin under its measured capacity;
  - proposals in steps of 0.01, raised only after half a second with room for two steps;
  - fields that started late and were not all caught up (about 3 % of the time was given up
    as debt);
  - stretches where its schedule was slower than it could have run, because the scene got
    lighter faster than the smoothed cost followed.

  The simulation (`sim2.mjs` in the scratch: the section 10 model with per-field costs that
  vary as the game's do) shows the same, at 94 % busy at Q5's pace.
- **Fields run one at a time cost little or nothing more than the chase's two-field batches.**
  This contradicts what section 5.4 suggested. Single-page runs at a simulated 1.6x slower
  machine (CLK2's game, interleaved, a quiet machine) gave:
  - the chase: 18.8 and 19.9 ms per field;
  - one field per batch back to back (`paceTune=fixed:1`): 19.9, 20.1, 19.9 and 20.0 ms;
  - switching off the GPU's timestamps (`gpuTimer=0`) did not help: 20.8 and 20.5 ms.

  The 6.5 % dearer fields of section 5.1 came mostly from the machine's load at the time. In
  this session's quietest round, Q5's fields cost what the chase's did (18.3-18.5 ms).
- So the lever is the slower machine's idle time.

### 15.2 Roles

`src/pacer.ts`. Roles are on by default; `paceTune=roles:0` restores Q5's common pace.

- **The setter.** Once one machine's capacity is 3 % below the other's for half a second, that
  machine sets the pace. Its schedule runs 15 % past its own capacity. It is therefore always a
  little behind, and runs each field as soon as the last is done: one field at a time, every
  frame shown, never idle, at whatever its machine can do from one scene to the next.
- **The follower.** The other machine follows it:
  - its schedule runs at the capacity the setter reports;
  - it moves its schedule toward equal input leads by itself, correcting the whole difference
    at Q5's net speed (the setter does not move);
  - it trims its pace by the phase difference that remains: an integral term, at most 4 %,
    held while it runs at full speed.
- **Agreement.** Player 1 decides the roles from the two capacities and reports its own in its
  `pace` message (`role`); player 2 takes the opposite one, so the two always agree. A first
  version let each side decide; in the simulation two equal machines could then both set the
  pace for seconds.
- **Scope.** Machines within 3 % of each other keep Q5's common pace. The roles last through
  stretches where both machines run at full speed.
- **Copy stats** says which role this side has, its schedule's pace and the trim.
- **Rejected: the follower aiming 12 ms early** (`followEarlyMs`, kept as an option). The idea
  was that the setter would never wait for the follower's inputs, but the setter waited about
  once a second either way. At delay 2 and over the jittery link the follower could not get
  that early: its trim wound up to the 4 % bound (`z2`, `z3`, `v2`, `v3`). At delay 2 it waited
  4.2-4.4 times a second, against 1.9 without the bias (`g1_ra13`). (The message of commit
  `68ae10d0f` says "0.3 without": that game ran at delay 6, see 15.3.)
- **Rejected: counting the loop's own lateness in the cost per field** (`v1`-`v3`). It made no
  difference and was dropped.

Host timing only, as before: the roles decide when fields run, never what they compute.

### 15.3 Measurements on the engine before Q4

**Setup.** Two pages on this PC as in section 2: E5c's innings script, delay 3, fields
4,300-6,900, `--separate --affinity FFFF`, player 2 at `hostSlow=1.3` or `1.6`.

**Variants**, rotated in each round:
- the chase: origin/main before Q5 (`site_base`);
- Q5: `paceTune=roles:0`;
- Q5b: the default.

The `x1`/`x2` roles runs used the first roles build (each side deciding, 1 s hold, no roles
at full speed); `x3` and later used the final one.

**Conditions.** The machine was mostly quiet; runs started only below 75 % load (the load at
each run's start and end is in the table).

**How to read "busy".** It is the slower page's run time over the wall time: how much of its
machine the pacing uses. Speed = busy x 16.68 ms / its cost per field. The cost per field moved
5-10 % from run to run with the machine's load, which moves the speed with it. The busy share
does not move with the load: it is the pacing's own number.

1.0 vs 1.3:

| Variant | Runs | Speed (range) | Slower page: busy | new frames/s | p95 | > 25 ms | > 50 ms | Faster page: waits/s (time waiting) | Underruns/min |
|---|---:|---|---:|---:|---:|---:|---:|---:|---:|
| chase | 3 | 0.852x (0.828-0.866) | 95.7 % | 36.5 | 48.3 ms | 44.8 % | 3.9 % | 37.4 (19.1 %) | 0.4 |
| Q5 | 5 | 0.796x (0.768-0.832) | 91.3 % | 47.3 | 33.9 ms | 19.0 % | 0.3 % | 1.5 (0.1 %) | 0 |
| **Q5b** | 5 | **0.837x (0.782-0.869)** | 94.5 % | 49.7 | 31.8 ms | 13.8 % | 0.3 % | 2.2 (0.1 %) | 0 |

1.0 vs 1.6:

| Variant | Runs | Speed (range) | Slower page: busy | new frames/s | p95 | > 25 ms | > 50 ms | Faster page: waits/s (time waiting) | Underruns/min |
|---|---:|---|---:|---:|---:|---:|---:|---:|---:|
| chase | 2 | 0.684x (0.681-0.687) | 96.5 % | 26.4 | 60.2 ms | 73.2 % | 17.8 % | 35.0 (33.5 %) | 0 |
| Q5 | 2 | 0.653x (0.650-0.656) | 92.9 % | 38.9 | 39.6 ms | 44.7 % | 0.8 % | 1.6 (0.2 %) | 0.5 |
| **Q5b** | 2 | **0.671x (0.669-0.672)** | 96.3 % | 39.8 | 39.8 ms | 40.8 % | 1.1 % | 2.3 (0.2 %) | 0 |

Per run (the slower page, the faster page's waits):

| Run | Variant | CPU load | Speed | Slower page: ms/field | busy | new frames/s | > 50 ms | Faster page: waits/s |
|---|---|---|---:|---:|---:|---:|---:|---:|
| `x1_b13` | chase (before Q5) | 0-16 % | 0.866x | 18.46 | 95.7 % | 36.8 | 3.5 % | 34.3 |
| `x1_q13` | Q5 | 9-5 % | 0.832x | 18.33 | 91.3 % | 49.5 | 0.2 % | 1.3 |
| `x1_r13` | Q5b roles | 6-1 % | 0.855x | 18.32 | 93.8 % | 50.9 | 0.3 % | 2 |
| `x2_r13` | Q5b roles | 5-2 % | 0.858x | 18.39 | 94.5 % | 50.9 | 0.2 % | 2.2 |
| `x2_b13` | chase (before Q5) | 12-18 % | 0.861x | 18.55 | 95.6 % | 38 | 3.9 % | 43.3 |
| `x2_q13` | Q5 | 8-15 % | 0.768x | 19.99 | 91.9 % | 45.7 | 0.3 % | 1.4 |
| `x3_q13` | Q5 | 28-0 % | 0.788x | 19.38 | 91.5 % | 46.8 | 0.2 % | 1.5 |
| `x3_r13` | Q5b roles | 1-13 % | 0.782x | 20.31 | 95.1 % | 46.4 | 0.3 % | 2.2 |
| `x3_b13` | chase (before Q5) | 2-18 % | 0.828x | 19.29 | 95.7 % | 34.8 | 4.2 % | 34.7 |
| `y1_r13` | Q5b roles | 35-17 % | 0.820x | 19.31 | 94.9 % | 48.7 | 0.3 % | 2.2 |
| `y1_q13` | Q5 | 8-5 % | 0.808x | 18.75 | 90.6 % | 47.9 | 0.2 % | 1.7 |
| `y2_q13` | Q5 | 10-13 % | 0.783x | 19.52 | 91.4 % | 46.5 | 0.4 % | 1.4 |
| `y2_r13` | Q5b roles | 7-2 % | 0.869x | 18.05 | 93.9 % | 51.6 | 0.2 % | 2.2 |
| `x4_q16` | Q5 | 23-3 % | 0.656x | 23.64 | 92.9 % | 39.1 | 0.8 % | 1.7 |
| `x4_r16` | Q5b roles | 4-9 % | 0.669x | 24.05 | 96.5 % | 39.8 | 1.1 % | 2.3 |
| `x4_b16` | chase (before Q5) | 26-19 % | 0.687x | 23.50 | 96.6 % | 27 | 16.2 % | 35.1 |
| `x5_b16` | chase (before Q5) | 40-3 % | 0.681x | 23.62 | 96.4 % | 25.8 | 19.3 % | 35 |
| `x5_r16` | Q5b roles | 12-0 % | 0.672x | 23.87 | 96.2 % | 39.8 | 1.2 % | 2.3 |
| `x5_q16` | Q5 | 12-23 % | 0.650x | 23.87 | 92.9 % | 38.7 | 0.8 % | 1.5 |

**Other conditions**, Q5b against Q5 at 1.0 vs 1.3, single runs:

| Condition | Variant | Speed | Slower page: new frames/s | > 50 ms | Faster page: waits/s (time waiting) |
|---|---|---:|---:|---:|---:|
| Delay Auto (both pages chose 2) | Q5 | 0.792x | 47.1 | 0.4 % | 2.6 (0.1 %) |
| Delay Auto (both pages chose 2) | Q5b | 0.827x | 49.1 | 0.4 % | 4.4 (0.1 %) |
| Simulated internet: 30 ms ± 10 ms each way, delay 4 | Q5 | 0.792x | 47.2 | 0.2 % | 3.2 (0.5 %) |
| Simulated internet: 30 ms ± 10 ms each way, delay 4 | Q5b | 0.871x | 52.0 | 0.2 % | 6.4 (1.1 %) |
| Equal machines (1.0 vs 1.0) | Q5 | 0.934x | 55.5 | 0.2 % | 0.5 |
| Equal machines (1.0 vs 1.0) | Q5b | 0.962x | 56.8 | 0.2 % | 0.7 |

Notes:
- These Q5b runs still had the follower aiming 12 ms early, which wound up its trim at delay 2
  and on the jittery link (section 15.2).
- With the final build (no early bias), over the same simulated link (`u2_rn13`): 0.908x, the
  faster page waiting 6 times a second (1.1 % of the time). That is the link's jitter at
  delay 4.
- **One Auto game chose delay 6 on the local link** (`u1_ra13`). Player 1's page was busy for
  half a second while it measured: its pings came back up to 500 ms late. The session took
  the larger proposal (6 against player 2's 2).
- **The fix** (`delay.ts`, `netplay.ts`): each page measures twice (two rounds of 20 pings,
  one second more before an Auto game) and proposes the lower of the two rounds' delays. A
  stall shows in one round, a real link's latency and jitter in both.
- **Checked on the final build:**
  - the local link gave 2 on both pages (`g1_ra13`);
  - 30 ms ± 10 ms each way gave 3 and 4 (the rounds: 3/4 and 4/4), so the session used 4
    (`g2_rn13`);
  - `tests/delay.test.mjs` replays the stalled measurement.

### 15.4 After Q4's merge

Q4 made the emulation about 20 % faster: at `hostSlow=1.3` the game runs at full speed solo
(Q4_LOG). The scenarios became:
- 1.0 vs 1.3: does it run at full speed online, and does the pace stay at 1.0?
- 1.0 vs 2: the old-laptop case, now about 0.8x solo, in place of 1.6.

The chase on this engine is the same build with `paceTune=chase:1`: online games pace as solo
ones (development).

**The quiet rounds.** These are the runs whose machine load stayed under about 20 % at both
ends. The other runs swung with other sessions' builds and benchmarks; they are all in the
per-run table below.

| Scenario | Variant | Runs | Speed | Slower page: busy | new frames/s | p95 | > 25 ms | > 50 ms | Faster page: waits/s |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| 1.0 vs 1.3 | chase | `a4`, `f1` | 0.984x, 0.984x | 81 % | 55.6 | 31 ms | 9.4 % | 0.4 % | 5.1, 18.6 |
| 1.0 vs 1.3 | Q5 | `f1` | 0.955x | 81 % | 55.6 | 29 ms | 9.9 % | 0.2 % | 1.0 |
| 1.0 vs 1.3 | **Q5b** | `a1`, `f1` | **0.976x, 0.975x** | 79 % | 57.4 | 26 ms | 6.5 % | 0.2 % | 0.8 |
| 1.0 vs 2 | chase | `a5`, `e1`, `f3` | 0.772x, 0.775x, 0.777x | 98.8 % | 30.6 | 54 ms | 60 % | 8.0 % | 42 |
| 1.0 vs 2 | Q5 | `a5`, `f3` | 0.757x, 0.744x | 93.1 % | 44.6 | 34 ms | 21.7 % | 0.3 % | 1.8 |
| 1.0 vs 2 | **Q5b** | `a5`, `e1`, `f3` | **0.756x, 0.775x, 0.751x** | 95.6 % | 45.2 | 33 ms | 20.3 % | 0.3 % | 2.0 |

**1.0 vs 1.3: full speed, except where this machine cannot keep up.**
- The pace sat at 1.0 for 91-92 % of the time (`a1_r13`, `f1_r13`) and went back to 1.0
  after each dip.
- The dips are the innings demo's heavy stretch around fields 6,120-6,300 (about three
  seconds). There the 1.3x machine runs at 0.72-0.80x whatever the pacing: the chase dips the
  same way (`f1_c13`, `a4_c13`). The CPU-vs-CPU session that Q4 measured solo has no such
  stretch.
- Over the whole measured stretch: Q5b 0.975-0.976x, the chase 0.984x, Q5 0.955x.
- Against the chase, Q5b's slower page has more new frames, the shorter p95 and fewer
  intervals over 25 ms, and the faster page waits under once a second instead of 5-19 times.

**1.0 vs 2 (the old laptop):**
- **Speed:** Q5b 0.751-0.775x against the chase's 0.772-0.777x, the same in one round and
  2-3 % below in two.
- **Smoothness:** the slower page shows 45 new frames a second instead of 31, with 0.3 %
  of its intervals over 50 ms instead of 8 %. The faster page waits twice a second
  instead of 42 times.
- **Where the last 2-3 % goes:**
  - The chase's slower page is busy 98.8 % of the time, Q5b's 95.6 %. Between two
    one-field batches the Worker spends about 0.45 ms on its other messages and task hops
    (`lateMs`): 2 % at 21 ms a field.
  - One field per batch costs 1.4 % more per field than the chase's mix of one and two
    (solo at 2x, Q4's engine, interleaved: 18.34 and 18.53 ms against 18.09 and 18.26 ms).
- **Two things tried that did not help:**
  - running two one-field batches per task without a hop (`paceTune=burst:2`, `e1`/`e2`:
    the same late time per field, 0.43-0.60 ms);
  - starting fields on the animation frames at full speed instead of on the slot timers
    (`p1`/`p2`: intervals over 25 ms 10.9 % against 9.9 % at 1.3x, 5.3 % against 6.0 % for
    equal machines).

  Both were removed. What is left is the price of presenting every field.

**Presents over 25 ms at 1.3x, and a field of run-ahead.**
- Measured: online at 1.3x, the slower page's new-frame intervals over 25 ms are 6.3-6.7 %
  in the quiet rounds (over 50 ms 0.2 %). Two equal full-speed machines on this PC give
  5.3-6.0 % (`p3`): most of them come from presenting two browsers on one GPU and display,
  not from the slower machine.
- Q4's suggestion was one field of run-ahead. The pacing alone cannot give it: a frame is
  shown at the first display refresh after its batch ends, so starting fields earlier moves
  the frames earlier without evening their intervals.
- It needs the render thread to hold a finished frame until its slot's refresh: a display
  queue one frame deep. Because the game copies every field into the same XFB address
  (0x00888080 in the presentation trace), that queue needs a copy of the scan-out per frame.
- It would add up to a field (16.7 ms) of input latency, solo and online.
- Not done here. It is in the render thread (P2 and Q4's code); worth it only if the owner
  sees hitches at the laptop.

Per run:

| Run | Variant | CPU load | Speed | Slower page: ms/field | busy | new frames/s | p95 | > 25 ms | > 50 ms | Faster page: waits/s |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `a1_r13` | Q5b roles | 12-15 % | 0.976x | 13.40 | 78.4 % | 57.5 | 25.9 ms | 6.3 % | 0.2 % | 0.8 |
| `a1_q13` | Q5 | 9-37 % | 0.832x | 16.80 | 83.6 % | 48.8 | 36.6 ms | 17.4 % | 1.4 % | 1.5 |
| `a1_c13` | chase | 69-69 % | 0.950x | 16.14 | 91.9 % | 47.9 | 39.3 ms | 21.8 % | 1.8 % | 31.5 |
| `a4_c13` | chase | 7-17 % | 0.984x | 13.72 | 81.0 % | 55.4 | 31.2 ms | 9.3 % | 0.4 % | 5.1 |
| `a4_r13` | Q5b roles | 45-71 % | 0.917x | 15.32 | 84.1 % | 53.8 | 29.9 ms | 12.3 % | 0.3 % | 1.5 |
| `a4_q13` | Q5 | 85-7 % | 0.902x | 15.09 | 81.5 % | 53.4 | 29.6 ms | 11.9 % | 0.7 % | 1.5 |
| `p1_t13` | Q5b roles | 37-66 % | 0.927x | 15.67 | 87.0 % | 54.9 | 27.5 ms | 9.7 % | 0.2 % | 1.4 |
| `p2_t13` | Q5b roles | 34-24 % | 0.942x | 14.97 | 84.5 % | 55.3 | 29.1 ms | 10.2 % | 0.3 % | 1.3 |
| `f1_c13` | chase | 0-0 % | 0.984x | 13.62 | 80.3 % | 55.7 | 31.6 ms | 9.4 % | 0.4 % | 18.6 |
| `f1_r13` | Q5b roles | 8-8 % | 0.975x | 13.57 | 79.3 % | 57.2 | 26.7 ms | 6.7 % | 0.2 % | 0.8 |
| `f1_q13` | Q5 | 9-12 % | 0.955x | 14.20 | 81.2 % | 55.6 | 28.9 ms | 9.9 % | 0.2 % | 1 |
| `f2_r13` | Q5b roles | 0-71 % | 0.749x | 20.67 | 93.1 % | 44.5 | 38.1 ms | 27 % | 1 % | 1.7 |
| `f2_q13` | Q5 | 90-20 % | 0.934x | 15.10 | 84.5 % | 54.8 | 28.7 ms | 9.4 % | 0.2 % | 1.3 |
| `f2_c13` | chase | 84-57 % | 0.961x | 15.62 | 90.0 % | 51 | 36.3 ms | 16 % | 1.6 % | 30.9 |
| `a2_q20` | Q5 | 31-0 % | 0.557x | 27.34 | 91.2 % | 33.2 | 44.1 ms | 68.7 % | 1.7 % | 2.6 |
| `a2_r20` | Q5b roles | 1-6 % | 0.665x | 24.18 | 96.4 % | 39.6 | 40.6 ms | 37.2 % | 1.2 % | 2.4 |
| `a2_c20` | chase | 2-28 % | 0.755x | 21.85 | 98.9 % | 29.5 | 56.8 ms | 61.9 % | 11.4 % | 40.1 |
| `a3_c20` | chase | 76-85 % | 0.493x | 33.61 | 99.4 % | 18.9 | 87.4 ms | 93.1 % | 51.1 % | 29.5 |
| `a3_q20` | Q5 | 94-27 % | 0.700x | 22.21 | 93.2 % | 41.7 | 35.6 ms | 33 % | 0.3 % | 2.2 |
| `a3_r20` | Q5b roles | 24-0 % | 0.764x | 20.82 | 95.3 % | 45.3 | 33.6 ms | 19.6 % | 0.4 % | 1.9 |
| `a5_r20` | Q5b roles | 12-8 % | 0.756x | 21.11 | 95.6 % | 45 | 33.5 ms | 21.5 % | 0.3 % | 1.9 |
| `a5_c20` | chase | 7-4 % | 0.772x | 21.38 | 98.8 % | 31.1 | 54.2 ms | 57 % | 7.9 % | 39.7 |
| `a5_q20` | Q5 | 0-7 % | 0.757x | 20.54 | 93.2 % | 45 | 33.8 ms | 20.1 % | 0.2 % | 1.7 |
| `e1_r20` | Q5b roles | 2-3 % | 0.775x | 20.64 | 95.8 % | 46.1 | 32.2 ms | 17.9 % | 0.3 % | 1.9 |
| `e1_c20` | chase | 3-1 % | 0.775x | 21.31 | 98.9 % | 29.5 | 53.6 ms | 65.6 % | 8.1 % | 42.3 |
| `e2_c20` | chase | 1-59 % | 0.670x | 24.61 | 98.7 % | 26.7 | 73.2 ms | 64.1 % | 17.6 % | 35.9 |
| `e2_r20` | Q5b roles | 11-34 % | 0.671x | 23.72 | 95.5 % | 39.9 | 39.3 ms | 38.3 % | 1.2 % | 2.4 |
| `f3_q20` | Q5 | 1-9 % | 0.744x | 20.88 | 93.0 % | 44.1 | 34.5 ms | 23.3 % | 0.3 % | 1.8 |
| `f3_c20` | chase | 4-15 % | 0.777x | 21.22 | 98.8 % | 31.2 | 54.4 ms | 56 % | 8 % | 45.4 |
| `f3_r20` | Q5b roles | 6-5 % | 0.751x | 21.18 | 95.3 % | 44.6 | 34 ms | 21.6 % | 0.4 % | 2.3 |

(`p1_t13`/`p2_t13` ran the final pacing; `a*` the same behaviour as the final; `e1`/`e2` the build
that also carried the two experiments of this section, both inactive below full speed and at
`burst:1`; `f*` the final build.)

### 15.5 Exactness and tests

The roles, the delay's two rounds, the statistics and the development switches are host-side.
They decide when fields run and which delay is proposed, never what a field computes.

**Guest lines, solo**, final build, Chrome 155, unpaced, threaded and single-threaded. All six
equal WEB.md's checkpoints:

| Fields | Instructions | Time base | Polls | MEM1 |
|---|---:|---:|---:|---|
| 700 | 4,547,010,125 | 473,646,888 | 360 | `53221c4a...` |
| 2,400 | 15,574,026,125 | 1,622,294,388 | 1,882 | `4285f135...` |
| 3,600 | 23,357,802,126 | 2,433,104,388 | 2,747 | `40a7692d...` |

**Every Q5b two-page game.** 81 games to frame 6,000, at 1.0/1.0, 1.0/1.3, 1.0/1.6 and 1.0/2,
on both engines, every variant. In each, both pages had identical guest lines and all 100
window hashes, with no desync and no gate miss. By delay:

| Delay | Games | Instructions | MEM1 |
|---:|---:|---:|---|
| 3 | 67 | 44,816,368,162 | `992cb7bd...` |
| 2 | 6 | 44,822,998,467 | `cf2da995...` |
| 4 | 7 | 44,933,331,331 | `b6d747fe...` |
| 6 | 1 (`u1_ra13`) | 44,817,092,629 | `55f983ce...` |

Each line equals native `mvp-netplay solo --delay D` on the merged engine.

**Tests**, final build:

| Suite | Result |
|---|---|
| Native (`ctest --preset msvc`) | data-free 35 of 35; local 13: 12 passed, `local_disasm` skipped as always (`local_netplay_lockstep`, `local_netplay_lossy` and Q4's `local_hle_native` included) |
| Emscripten (`ctest --preset emscripten`) | 41: 40 passed, `local_disasm` skipped |
| Web shell | 90 of 90 |

New shell tests:
- `tests/pacer.test.mjs`: the roles; the follower's trim; player 1 deciding; the early-bias
  option; two simulated players with fields that vary (Q5b faster than Q5, still even).
- `tests/delay.test.mjs`: the stalled measurement and the two rounds.
- `tests/netstats.test.mjs`: the role in Copy stats.

### 15.6 Decisions to record (for the lead)

- **Q5b-N1, roles:**
  - Online, once one machine's capacity is 3 % below the other's for half a second, that
    machine sets the pace and runs free: its schedule runs 15 % past its capacity, one field
    at a time.
  - The other follows at the capacity the setter reports. It is phase-locked to equal input
    leads, with an integral trim of at most 4 %.
  - Player 1 decides the roles and reports them; player 2 takes the opposite one.
  - Machines within 3 % keep Q5-N1's common pace.
  - Host timing only. `paceTune=roles:0` (development) restores Q5-N1.
- **Q5b-N2, Auto delay measured twice:** two rounds of 20 pings; each page proposes the lower of
  the two rounds' delays (amends Q5-N2). Auto games take a second longer to start.
- **Q5b-T1, comparing pacing variants:** compare them by the slower page's busy share (run time
  over wall time) as well as by speed. On this shared PC the cost per field varies 5-10 %
  between consecutive runs.

### 15.7 Next

1. **Field data from the owner,** as in section 14. Copy stats now also says which computer
   sets the pace and the follower's trim.
2. **At the old laptop (2x) the price of every frame shown is 2-3 % of speed.** It comes from
   the Worker's per-batch work and messages (about 0.45 ms a field) and the dearer one-field
   batches. A cheaper batch boundary (fewer messages between the page, the Worker and the
   render thread per field) would close it. That is mostly outside the pacing.
3. **A display queue one frame deep** in the render thread would hide single slow fields
   (section 15.4), at up to a field of input latency. Worth it only if the owner sees hitches
   at the laptop.
4. **Solo play** keeps Q2's chase. The roles' "run free, one field at a time" is, solo, Q2's
   `slowMotion=1`. Whether to make that the default below full speed is the owner's call on
   feel: every frame shown at 2-3 % less speed.
