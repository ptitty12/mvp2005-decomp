# V1 log: frames that reach the screen, and the ABS challenge on the threaded engine

Side session `claude-side-perf`, branch `engine/perf-120` (from origin/main `ffaa4b61`).
The owner, 2026-10-21: "Optimize optimize optimize, I am still dropping frames on some
devices, do whatever you absolutely can to make this run as efficiently as possible",
and "I would love the ability to play at 120 fps on devices that are capable".
Chrome 155 headed through Playwright on the dev PC (i7-14700K, RTX 4070 SUPER, the
owner's 3440x1440 display at 59-60 Hz). Other agents of this session shared the PC
for part of the time; every timing run held the measurement lock (below).

Later work of the same session has its own logs: V3 (120 fps, in-between frames),
V4 (the emulation thread), V5 (the render thread).

## Status

| # | What | State |
|---|---|---|
| V1 | Count what reaches the screen, per display refresh; a picture queue | **Done** (`31aa85fc`, on main) |
| V2 | Gameplay mods (the ABS challenge) in the threaded engine | **Done** (`caf24f56`, on main) |
| - | README: what the project is now, and how it got here | **Done** (`adf6f2a9`, on main) |

## 1. How it was measured

The scripted sessions of earlier logs (CLK2's `game_125`, `human_125`) were lost with an
old scratch folder, so every number here comes from one scene anyone can rebuild: the
save state `braves-dodgers-9th-bat` (save-state-files: top of the 9th at Dodger Stadium,
the MLB 2026 pack, the ABS challenge on; player 1 bats and never swings, so the CPU keeps
pitching). It loads at field 136,350; each run plays 58 s paced from field 136,700.

```
node engine/web/tools/browser_check.mjs --browser chrome --headed --server-disc --disc <CISO> \
  --states <dir with the .mvpstate> --mock-packs <dir with mlb-2026.zip> \
  --params "state=braves-dodgers-9th-bat" --profile <profile dir> --fields 140200 \
  --viewport 1600x1000 [--affinity F000000 | --host-slow 1.3@200]
node engine/web/tools/display_stats.mjs <out>/summary-chrome.json --from 136700
```

`--host-slow N@200`: the Worker's own count of fields run starts at 0 after a state
loads, so `@F` must be a small number here (with `@136400` the engine never slowed).

**What reaches the screen** (`src/presenter.ts`, new): the page counts, at each of its
animation frames (one per display refresh), how many new pictures arrived since the last
one. A picture stays on screen from the refresh that showed it until the next one replaces
it: `held[k]` counts pictures that stayed k refreshes; `dropped` counts pictures replaced
before any refresh showed them. This is the closest the page can get to "frames the player
saw" (the compositor shows the canvas's latest picture at each refresh). `window.mvp.display`,
recorded in every browser_check stats window; `tools/display_stats.mjs` sums a run.

**A measurement lock.** Several agents measured on this PC at once. Every timing run went
through a lock file (`mlock.py` in the session's scratch folder: first come, first served,
25 min staleness), so no two paced sessions overlapped.

## 2. V2: the ABS challenge forced the single-threaded engine

With any gameplay mod on, `worker.ts` loaded `mvp-web.js` (the single-threaded engine:
"mods: single-threaded"), because the mods' timeline saved and restored the GX processor's
state only inline (ABS_CHALLENGE.md S7). The owner plays with the ABS challenge, so every
device ran the GX processor and the renderer on the emulation thread. This was the largest
cost found.

**The change** (`caf24f56`):
- `render_thread`: records `kViewSave` / `kViewLoad` (a slot). The render thread saves and
  restores its GX processor (`RenderSide::save_view` / `load_view`) at those points of the
  stream, exactly where the inline renderer does; a load moves the GP snapshot generation on
  both sides, as `kViewImport` does. Nothing waits on a save.
- `Timeline::Tool::replacing_memory()`, called before a restore replaces guest memory: the
  threaded tool calls `render_memory_replaced()` (the render thread finishes reading memory
  as it was and its copies end), as save-state file loads already did.
- `mvp_present()` during a review's headless legs ends the batch (`render_batch_end`).
- `worker.ts` loads the threaded engine with mods on; `mvp_boot` no longer refuses.

**Exactness.** The `braves-dodgers-9th-close-call` state with L+R+Z tapped every 20 frames
(three challenged reviews: rewind, half-speed replay, re-run), unpaced to field 141,000:
`threads=0` and threaded end at the same guest line, 914,600,154,125 instructions, MEM1
`bd9daa73...`. Renderer digests (`--digest`) agree in all 2,522 fields of a run through the
reviews. Two browsers online (`netplay_check.mjs`, ABS on, Play Now from power-on to frame
9,000, both threaded) stayed in lockstep with equal guest lines; that run did not reach a
review (player 1 was still to pitch), so reviews online are covered by the single-player
check only. `test_gameplay`, `test_mods`, `test_web_shell` pass.

**Measured** (the batting state, 58 s each):

| Machine | Engine | Speed | Emulation ms/field | Render thread ms/field | Pictures shown/s | Lost/s |
|---|---|---:|---:|---:|---:|---:|
| dev PC | single-threaded (before) | 0.999x | 10.85 | - | 56.4 | 3.39 |
| dev PC | threaded (after) | 1.000x | 7.63 | 5.24 | 59.9 | 0.07 |
| hostSlow 1.3 (the ThinkPad's stand-in) | single-threaded | 0.998x | 13.98 | - | 57.3 | 1.40 |
| hostSlow 1.3 | threaded | 0.999x | 9.75 | 5.14 | 60.0 | 0.19 |
| hostSlow 2 | threaded | 0.998x | 14.90 | 5.15 | 59.2 | 0.10 |
| 4 E-cores | single-threaded | 0.792x | 20.06 | - | 29.4 | 0 |
| 4 E-cores | threaded | 1.003x | 13.13 | 10.46 | 59.6 | 0.17 |

On 4 efficiency cores the game went from 0.79x speed with 29 pictures a second to full speed
with 60. The timeline's snapshots and reviews also got cheaper (a review's re-run 840 -> 600 ms
of host time).

## 3. V1: how pictures reach the screen

The threaded engine's render thread posts one ImageBitmap per present, at most one per page
refresh (P2); the page hands it to its `bitmaprenderer` canvas when the message arrives. The
single-threaded engine posts from the Worker after each batch.

`presenter.ts` has two modes (URL `present=now|queue`):
- `now`: as before.
- `queue`: pictures wait in arrival order; each refresh shows the oldest. A pair that arrives
  within one refresh shows on two refreshes instead of losing the first; a queue that holds two
  or more for half a second drops one (bounded delay). With `present=queue` in the threaded
  engine the render thread shows every present at once (no refresh count) and every field of a
  two-field catch-up batch is presented (`mvp_present_every_field`).

Measured on the dev PC (the batting state; `mean wait` is the extra delay in refreshes):

| Engine | Mode | Lost/s | Held 2+ refreshes | Mean wait |
|---|---|---:|---:|---:|
| single-threaded | now | 3.39 (5.7%) | 6.1% | 0 |
| single-threaded | queue | 0.00 | 0.1% | 0.37-0.52 |
| single-threaded, hostSlow 1.3 | now | 4.84 (8.1%) | 8.9% | 0 |
| single-threaded, hostSlow 1.3 | queue | 0.00 | 0.1% | 0.37 |
| threaded | now | 0.07 (0.1%) | 0.2% | 0 |
| threaded | queue | 0.21 (0.3%, trims) | 0.0% | 1.11 |
| threaded, 4 E-cores | now | 0.17 (0.3%) | 0.7% | 0 |
| threaded, 4 E-cores | queue | 0.22 (0.4%) | 0.1% | 1.55 |
| threaded, hostSlow 2 | now | 0.10 (0.2%) | 1.2% | 0 |
| threaded, hostSlow 2 | queue | 0.29 (0.5%) | 0.2% | 1.05 |

So: the single-threaded engine's pictures arrive unevenly and the queue fixes that
completely; the threaded engine's are already even, and a queue only adds a refresh of
delay. **Defaults:** `now` for the threaded engine, `queue` once the Worker reports that it
loaded the single-threaded one. The queue is also what the in-between frames of V3 need.

## 4. Other findings

- **Profile of the single-threaded engine in this scene** (Chrome trace, unnamed build): engine
  39%, idle 28%, translated code 26%, one engine function alone 16.5% (`wasm-function[940]`,
  unnamed). V4 profiles the threaded engine with names.
- **The timeline's snapshot** (`rt::save_state`, every pitch with the ABS challenge on) takes
  3-8 ms of one field: buffers are reused already; the cost is copying MEM1 and ARAM. On a
  machine near its limit it costs one late picture per pitch. Not changed.

## 5. Commits

- `adf6f2a9` docs: README says what the project is now.
- `caf24f56` engine(web,mods): gameplay mods in the threaded engine.
- `31aa85fc` engine(web): what reaches the screen, counted per refresh; a picture queue.
- (this log, `tools/display_stats.mjs`): see `git log`.
