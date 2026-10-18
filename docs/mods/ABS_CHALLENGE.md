# ABS challenge mod: scoping

An optional gameplay mod for the browser engine: one toggle that

1. hides the circle that marks where a pitch crossed the zone,
2. makes the home-plate umpire miss some ball/strike calls (an overall rate set by a slider,
   with misses weighted toward the edges of the zone),
3. lets the batting side challenge a called strike and the fielding side challenge a called
   ball within a short window (ABS style: 2 per team, kept when the challenge succeeds),
4. cuts to a replay of the pitch with the circle shown, then upholds or overturns the call.

Written 2026-10-17 by `claude-side-abs`: the design (§1-§12) against `main` at `0bfab3ac`,
then the native implementation (§0). No decomp source changes.

**Owner decision (2026-10-17):** the replay uses route B, rewinding with engine save states
(§7). It is recorded here as a proposed DECISIONS entry (§12), because only the lead edits
DECISIONS.md.

**Evidence tags** used for every claim:
- **V**: read in the split assembly or the matched C at the address given.
- **T**: seen in a traced native run (§3.10).
- **I**: inferred from structure, call patterns or timing, not proven.
- **U**: unknown. Every U is listed in §10 with how to settle it.

Names stay `fn_`/`lbl_` placeholders. The roles given here are claims with their evidence,
not renames.

---

## 0. Implementation status (2026-10-17)

The mod runs natively (`mvp-play --mod abs`), in the browser (the Gameplay mods panel or
`?mod=abs:...`) and in browser netplay. It is not deployed.

| Stage | State | Evidence |
|---|---|---|
| S1 hook API core, hide the circle | **Done** | Mod off: the guest line after 12,000 frames equals a build without the sites (84,818,557,199 instructions, MEM1 `c1bccfad...`). Mod on: at frame 7062 the circle is gone and the rest of the frame is identical (captures) |
| S2 umpire model | **Done** | Calls are rewritten at `fn_801BB338`, and the misses reproduce an independent model of the RNG exactly. The edge constants come from 96 called pitches (84 CPU vs CPU, 12 batter). Over that sample, 8/20/30 % give 8/20/29 % at edge 3, with 83 % of misses within 0.15 of the edge. An independent check over 300+ pitches is still to do |
| S3 save states | **Done** | `mvp-play --rewind-test`: poll 3000 -> 3300 (menus) and 8000 -> 8900 (in play) replay to identical guest lines. Snapshot 7-9 ms, restore 2.6-6 ms native; in Chrome at most 4.3 and 2.9 ms. Two snapshots per pitch since 2026-10-17 (review length below) |
| S4 challenge and review | **Done** | See below |
| S5 browser | **Done; not yet judged by feel** | Chrome 155, `browser_check.mjs --fast`: the overturn run below ends at the native guest line (MEM1 `4d94b897...`); without the challenge it ends elsewhere (`6b141e63...`). The browser build forces the single-threaded engine while a mod is on |
| S6 netplay | **Done (prototype input)** | Two Chrome pages through the local lobby (`netplay_check.mjs`, D = 3): with a challenge by player 1, both pages end at the native reference (MEM1 `b7363882...`), with all 116 hash windows equal and no desync. Different settings are refused at the handshake: "the players chose different gameplay mod settings (mod.abs.miss: player 1 30, player 2 20)". The challenge is still L+R+Z on the game pad (§7.6) |
| S7 polish | Not started | CPU challenges, replay speed, commentary cue, the threaded browser engine |

**S4 test runs** (miss 30, edge 0, seed 1, P1 batting and never swinging):
- **Upheld.** A correct called strike (frame 6063) was challenged. The re-run ended with a
  MEM1 hash identical to live play, so the whole rewind is deterministic. One challenge left.
- **Overturned.** A ball (x = 1.46) called strike three (frame 7036) was challenged:
  - the replay shows the circle outside the zone under "STRIKEOUT!";
  - the re-run made the call a ball;
  - the plate appearance continued, and both challenges were kept.

**Code:**
- `engine/runtime/mod_hooks.{h,cpp}`: the site registry.
- `engine/runtime/save_state.{h,cpp}`: save states, with snapshot hooks in `Machine`,
  `Hardware`, `SystemHooks` and `YieldingHooks`.
- `rt::g_poll_gate` in `hle`.
- `engine/mods/gameplay/`: `api.h`, `host`, `timeline`, `abs`, plus `tests/test_gameplay.cpp`.
- `mvp-play` options:
  - `--mod abs[:k=v,...]` and `--mod-seed`;
  - `--capture-review DIR`;
  - `--rewind-test F:N`.
- Browser: `engine/web/wasm/mvp_web.cpp` and `net_web.cpp` (the `mvp_mod_*` exports, mod
  settings in the handshake, the session seed), `engine/web/src/mods-panel.ts`,
  `engine/web/src/abs-scene.ts` (the challenge scene, §4.6), and the Worker and page.
- Netplay: `net/protocol.cpp` names the differing settings in a refusal.

**Owner feedback, 2026-10-17** (first play in the browser):
- **An announcement the moment someone challenges** (§4.6). The engine waits 1.8 s before the
  review starts.
- **The verdict's strike zone is 1.5 times larger** (§4.6).
- **Challenges reset at every new game, Restart Game included** (§4.3). A restarted game
  used to keep the last game's counters.

**Owner feedback, second round, 2026-10-17** (§4.6):
- **The callout:** instead of the announcement card, a red callout in the middle names who
  called for the challenge ("ECKSTEIN / HAS CALLED FOR A CHALLENGE"), for 1.5 s.
- **The replay plays at half speed,** evenly: 30 frames a second, each for two refreshes.
- **The verdict card:** only "CALL OVERTURNED" or "CALL STANDS", the zone, and the team's
  challenges left ("Cardinals: 2 challenges left").

**Known limits:**
- **Threaded browser engine:** mods need the single-threaded engine. The render thread's
  GX state is not saved yet.
- **Challenge input:** L+R+Z on the game pad (keyboard Q+E+Space). The game sees it too;
  no side effect was seen in the tests (U6).
- **Netplay pauses:** a review pauses the netplay frames (both pages re-run locally).
  The netplay test's longest input latency was 12 s. Whether that came from the review or
  from start-up was not measured (R2).
- **Not judged by feel:** reviews have not been played by a human in a paced browser game.

**Review length (2026-10-17: the late snapshot, §7.2).**

**Before:** a review rewound from the snapshot at the new-pitch site, so it ran headless
through the whole pitch choice and delivery before the replay. The overturn test needed 320
frame polls: 3.3 s native, 4.8 s in Chrome.

**Now:** the mod also takes a snapshot during the delivery (S_L), timed to land 3 polls
before the replay's first frame (release - `pre`). The timeline keeps both and rewinds from
the later one that fits. The replay is unchanged: it still opens `pre` polls before the
release, windup included.

From the timeline's "review: host time" line:

| Run | Rewinds from (was) | Seek (was) | Replay | Re-run |
|---|---|---|---|---|
| Overturn, native headless | poll 6969 (6652) | 3 polls, 31 ms (320 polls, 3.3 s) | 1.0 s | 0.7 s |
| Overturn, Chrome `--fast` | the same | 46-48 ms (4.8 s) | 1.5 s | 1.0 s |
| Netplay script, native | poll 6332 (5990) | 3 polls, 33 ms (345 polls) | 0.9 s | 0.6 s |
| Upheld (a team's first pitch), native | poll 5930 | 85 polls, 0.9 s | 1.2 s | 0.7 s |

The guest lines are unchanged:
- **Overturn:** native and Chrome both end at MEM1 `4d94b897`.
- **Netplay:** two Chrome pages at D = 3 both end at `b7363882`, with 116 windows equal and
  no desync.
- **Mod off, with the new registry site:** MEM1 `c1bccfad`, 84,818,557,199 instructions.
- **Upheld:** the re-run equals live.

**How often the late snapshot fits** (native, every pitch of two games with the mod on: 13
in the batter game, 26 in CPU vs CPU over 24,000 polls):
- **33 of 39 pitches:** the rewind would be 2-4 polls.
- **Each team's first pitch (3):** 78-86 polls, from the 62-poll guess (about 1.2 s in
  Chrome).
- **A delivery shorter than the team's last one (3):** 165-180 polls, from the new-pitch
  snapshot (about 2.6 s in Chrome). The scheduled S_L is dropped at the release in this case.

**The cost: a second snapshot per pitch.** A snapshot takes at most 3.7-4.3 ms in Chrome
(the longest of 24 in each paced run). Restores take at most 2.9 ms.

Measured in headed, paced Chrome 155 over polls 7,000-12,000 of the batter game: 10 pitches,
9 late snapshots. The runs were interleaved new, old, new, old, comparing the same polls; the
old build takes no snapshot there. The last column is the largest present interval among the
two presents after each late-snapshot poll: median (max) over the 9.

| Machine | Build | Rendered fps | Present p95 | > 25 ms | > 50 ms | At the late-snapshot polls |
|---|---|---|---|---|---|---|
| this PC | new (2 runs) | 59.7 / 59.5 | 27.9 / 28.5 ms | 10.5 / 10.6 % | 0 / 0 % | 19.4 (24.0) / 18.9 (26.8) ms |
| this PC | old (2 runs) | 59.5 / 58.8 | 28.6 / 29.4 ms | 10.7 / 12.4 % | 0 / 0 % | 18.9 (30.6) / 20.6 (26.8) ms |
| hostSlow 1.3 | new | 47.4 | 36.2 ms | 24.7 % | 0 % | 19.3 (39.9) ms |
| hostSlow 1.3 | old | 46.9 | 36.1 ms | 24.9 % | 0 % | 16.9 (35.1) ms |

Reading it:
- **On this PC the extra snapshot is not measurable.** The frames around it fall within the
  run's own spread, and the same polls without a snapshot spread just as much.
- **At hostSlow 1.3** (Q2's stand-in for the owner's laptop):
  - the frame holding the snapshot was 1.4-4.8 ms longer at 8 of the 9 polls;
  - once in 9 that made a 37 ms frame where the old build had 18 ms: one late frame during
    the delivery, about 0.3 s before the release.
  - hostSlow slows the emulation, not the copy, so on the laptop itself the copy will take
    somewhat more than 4 ms.
- **Neither machine is smooth here, with or without the snapshot:** 10-12 % and 25 % of
  presents are over 25 ms.
- **Against that,** the rewind before a replay drops from 4.8 s to under 0.05 s in Chrome,
  for any pitch after a team's first with an unchanged delivery.
- **The challenge scene (§4.6)** no longer flashes its card over a rewind this short. The
  Replay tag names who challenged.

## 1. Verdict

**Feasible, and every gameplay hook falls on a small, well-defined retail function. The
engine is not ready for it yet.** Two engine features are missing and come first:
- a **hook API v1** that binds host code to retail addresses (§5);
- **save states** at the frame boundary (§6).

Both are owned, versioned engine contracts (ARCHITECTURE §2). Both also serve other work:
rollback netcode, quick save in the browser, and tests that start from a mid-game state.

| Part | Mechanism | Retail sites | Confidence |
|---|---|---|---|
| 1. Hide the circle | Before-hook on the circle draw clears its show flag (`hud+0x90`) while the mod is on and no review is running | `fn_802F893C` (V, T, seen in captures) | High. The flag rose 2 fields before the call, and the circle is on screen from then until the next pitch (T) |
| 2. Umpire misses | After-hook on the ball/strike decision: rulebook truth, then an engine-RNG miss weighted by edge distance | decision `fn_801BBC9C` (V, T) | High. One call per pitch that reaches the catcher; the result feeds the normal commit path (T) |
| 3. Challenges | Engine input during a window after the call; per-team counters in mod state | the call `fn_801BB1E0` (T), new pitch `fn_801BAFA4` (T) | High for the logic. Medium for netplay: input v1 has no spare bit (§7.6) |
| 4. Review replay | Route B: restore the per-pitch snapshot, present the pitch with the circle shown, then re-run to the challenge frame with the verdict forced | release `fn_801896E8` (T, I) | Medium. Costs depend on snapshot speed and headless speed in the browser (R1, R2) |

The game also ships **its own umpire-error machinery** (§3.8):
- a randomized "variable strike zone" for balls and strikes, **off** in a default Play Now
  game (T);
- **blown out/safe calls at the bases**, **on** by default (T);
- CPU manager arguments, which **explicitly skip ball and strike calls** (V).

None of it gives a challenge or a replay, and the variable zone has no rate control. So the
mod brings its own miss model and can optionally reuse the game's zone as the "umpire" (§4.1).

## 2. Settings

All are match settings: they join the netplay handshake (§5.6). As built, `mvp-play`
takes them as `--mod abs:key=value,...`, for example `--mod abs:miss=20,edge=0`.

| Key | Values | Default | Notes |
|---|---|---|---|
| (enabled) | `--mod abs` | off | The toggle |
| `hide` | 0 / 1 | 1 | Hide the post-pitch circle in play (shown in reviews) |
| `miss` | 0-30 % of called pitches | 8 | The owner floated 20 %. With edge weighting it is the *mean* over called pitches (§4.1) |
| `edge` | 0 (uniform) to 4 (edges only) | 3 | 0 gives the owner's flat model |
| `umpire` | 0 engine / 1 game | 0 | 1 uses the game's own variable zone (§3.8) as the umpire; `miss` and `edge` are then unused |
| `challenges` | 0-5 per team | 2 | Kept when the call is overturned |
| `window` | 30-180 ticks (1/60 s of game time) | 90 | Must close before the next pitch begins (about 1.7 s after the call, T: §3.10) |
| `pre` | 0-60 frame polls | 15 | Replay lead-in before the release |
| `post` | 15-120 frame polls | 45 | How long the replay holds after the call |

Planned, not built:
- a replay speed (presentation only);
- CPU challenges in games against the CPU (S7).

## 3. Code map (retail `main.dol`, GV4E69 rev 0)

### 3.1 The objects and how to reach them

| Object | Reached from | What it is | Tag |
|---|---|---|---|
| **P** (`Unk801B7FB0_P`) | `A+0x14`; `T+0x4` | Game context. `+0x554` is the gameplay PRNG (`fn_8014D8AC`, DETERMINISM RN2), `+0x65C[13]` the on-field actors (`fn_801C1638`), `+0x6F8` the game state **G**, `+0x700` the play coordinator **T**, `+0x704` the event bus | V (`include/game/tu_801B7FB0.h`, uses in `tu_801BAC74.cpp`) |
| **T** (`Unk8017A930_T`) | `P+0x700`; `A+0x10` | Play coordinator. It notifies `+0x3A4` (batter side; swing state at `+0x3A4 -> +0x28C`), `+0x3B0` (ball/pitch, `Unk8022FE34`), `+0x3B4` (fielders), `+0x3B8`, `+0x3BC` (the umpire **A**). `+0x404` holds cManagerArgumentTunables | V (`tu_8017A930.cpp` `fn_8017B5F4`, `fn_8017B6C0`; `tu_80177C34.cpp:888`) |
| **A** (`Unk801B7FB0_Ai`, vtable `0x8065F9F8`) | `T+0x3BC` | Plate-umpire AI. `+0x8` umpire state **B**, `+0xC` umpire actor (animations), `+0x10` T, `+0x14` P, `+0x24` a flag rolled in `fn_801BB338`, `+0x28/+0x2C/+0x30/+0x34` the variable zone's top, bottom, left and right, `+0x68` cBlownCallTunables | V; the values were seen in a run (T) |
| **B** | `A+0x8` | Umpire state (0x34 bytes, made by `fn_801B9800`). `+0x20` = "the variable-zone call differs from the rulebook", `+0x28` = umpire phase (3 at a call) | V |
| **G** (`Unk80069F0C`) | `P+0x6F8` | Game state. `+0x10` the **options** words (§3.6), `+0x14` -> the count block (§3.5). Event dispatch is `G::fn_8006A2C4` | V; in the trace the count block sits at G+0x80 (T) |

### 3.2 From the pitch reaching the catcher to the call (V, T)

```
fn_801DF7A0  (pitch arrival; role I)                       0x801DFDE4: bl fn_8017B5F4
  fn_8017B5F4(T, flag)   broadcast "pitch at the plate" to umpire, m3B8, fielders, ball, batter
    fn_801BB1E0(A, flag) the umpire handles the pitch                        (B->m28 = 3)
      fn_801BBC9C(A, &r)       ball/strike decision -> r (1 = strike)       <- hook site
      fn_801BB338(A, flag, r)  a roll that may set A->m24 (only when flag == 0, r == 0)
      if fn_801D0294(T->m3A4 + 0x28C) /* swung */ || r:
          if strikes == 2 && (B->m18 || B->m1C) && (!P->m65C[10] /* I: 1st base empty */ || outs == 2):
              fn_8017B994(T); fn_801BDB38(A)        dropped third strike, event 0x55
          else fn_801BD878(A)                       strike
      else fn_801BDA38(A)                           ball
```

- **Swing test** `fn_801D0294(s)`: returns 1 when the word at `s+4` is 1, 2, 4, 6 or 7, else 0
  (V, `0x801D0294`). The mod reads that word directly.
- `fn_801BB1D0` sets `B->m1C` when the catch type (`+0x78C` of the arrival object) is not
  0xB, 0x11 or 0xD (V, `0x801DFDBC`): the ball was not caught cleanly.
- **Trace (§3.10):** one pitch per `fn_8017B5F4`, and every `fn_801BB1E0` led to exactly one
  count change: 11 of 11 in run A (6 balls, 5 strikes). Fouls (`fn_8006DDF4`, event 0x45) and
  balls in play never pass through `fn_801BB1E0`.

### 3.3 The decision, `fn_801BBC9C(A, int* out)` (V; `0x801BBC9C`-`0x801BBED0`, asm only)

1. **Location:** `fn_8022EA60(&v, T->m3B0)` returns the two floats at `(T->m3B0)->m4 + 0x28`
   (x) and `+0x2C` (y) (V, `0x8022EA60`, 7 instructions). They are the plate crossing in
   zone-normalized units.
2. **Rulebook call:** `*out = (|x| <= 1.0 && |y| <= 1.0)` (constant `lbl_805CFD08` = 1.0f).
3. **Variable zone**, only when `options+0x148 != 0 && options+0x74 == 0`:
   - The call is redone against the per-game zone `A->m30 <= x <= A->m34`,
     `A->m2C <= y <= A->m28`. That zone is rolled once per game in `fn_801BACA0` from the game
     PRNG: top 0.92-1.15, bottom -(0.92-1.15), right 0.90-1.20, left -(0.90-1.20) (V, matched C,
     `tu_801BAC74.cpp`; T: rolled at field 4246, lr `0x8017AE3C`).
   - `B->m20` is set when the zone call differs from the rulebook.
4. **When they differ**, a keep threshold comes from a rating, `G::fn_8006C038(actor->m704)`
   for on-field actor 0 (`fn_801C1638(P, 0)`; which player that is, I: pitcher or catcher).
   Above 0.9 gives 90, above 0.8 gives 70, above 0.6 gives 50, else 25. Then a game PRNG roll
   `fn_8014D8AC(P->m554, 100)` at source line 0x6BC decides:
   - a zone **strike** on a rulebook ball stands only if the roll is at or under the threshold;
   - a zone **ball** on a rulebook strike always stands, and the roll only clears `B->m20`.
     That looks like an original bug, and we preserve it.
5. **Debug print:** if `B->m20` is still set, `fn_80303CA0("Variable Strike Zone Call", ...)`
   runs (string `0x805CFCEC`).

**Consequence for the mod:** with the variable zone off (the default, T), the umpire is
exactly the rulebook. Every miss then has to come from the mod. With it on, the game's own
call already draws from the gameplay PRNG. The mod must therefore call the original and
override the result afterwards, so the game's random stream is consumed exactly as without
the mod (§4.1).

### 3.4 Commit: count, strikeout, walk (V, T)

Both outcome handlers post on the event bus with `fn_8037893C(P+0x704, id, payload, 0, 0)`.
Dispatch is synchronous: the handler reads the updated count right after posting (V). The
trace shows the count handler running in the same field, inside `G::fn_8006A2C4` (T, lr
`0x8006A454`/`0x8006A484`). So **the post is the commit point**.

| Event | Posted by | Handled by (`G::fn_8006A2C4` case) | Effect | Tag |
|---|---|---|---|---|
| 0x41 strike | `fn_801BD878` (`0x801BD8CC`) | `fn_8006DF00` | strikes++ (`+0x1CCA`) | V, T |
| 0x40 strikeout | `fn_801BD878` when strikes >= `*(int*)0x8061B9C4` (it points to `0x8061B9C0`, static 3) | no case 64 in G's switch; its subscriber is unknown (U7) | then `fn_801BC660(A, ..., 6, 1)`, umpire anim `fn_801BFC4C`/`fn_801BFBAC` | V; T (strike three at field 16152 ended the half-inning) |
| 0x48 ball | `fn_801BDA38` (payload: the batter and pitch info) | `fn_8006DE84` | balls++ (`+0x1CCB`) | V, T |
| (walk) | `fn_801BDA38` when balls > 3 | `fn_8017B6C0(T, 0, 0)`: notifies all actors, posts **0x3A** (plate-appearance result, G case 58 `fn_8006DAD0`) | umpire anim `fn_801BFC8C` | V (no walk in the traces) |
| 0x45 foul | elsewhere | `fn_8006DDF4` | fouls++ (`+0x1CCC`); strikes++ only while strikes <= 1, else `+0x1CCD`++ | V, T |
| 0x55 dropped third strike | `fn_801BDB38` | case 85, `fn_8006E710` | umpire anim `fn_801BFC1C` | V; meaning I |
| 0x3D (I: pitch released) | `fn_801896E8` (`0x80189B80`) | G case 61, `fn_8006DD64` (per-pitch stat) | about 39 fields before the call; three AI callers read the pitch location in the same field | T, I |
| 0x3E (I: ball reaches the catcher) | `fn_80188FC4` (`0x8018911C`) | U | 2 fields before the call | T, I |
| 0x31 (I: new pitch) | `fn_8017B238` (`0x8017B3F4`), right after `fn_801BAFA4` | U | about 1.7 s after each call | T, I |

### 3.5 Where the count lives (V, T)

The count block is `*(G+0x14)`. It holds two 0xE64-byte team records, then:

| Offset | Size | Meaning | Tag |
|---|---|---|---|
| `+0x1CC8` | u8 | inning, 0-based | T (0 through the 1st, 1 when the top of the 2nd began at field 23039) |
| `+0x1CC9` | u8 | outs (`== 2` is tested for the dropped-third-strike rule) | V, T |
| `+0x1CCA` | u8 | strikes | V, T |
| `+0x1CCB` | u8 | balls | V, T |
| `+0x1CCC` | u8 | fouls in this plate appearance (saturates at 0xFF) | V, T |
| `+0x1CCD` | u8 | fouls with two strikes | V |
| `+0x1CD0` | int | batting side: 1 = away (top), 0 = home | T (flipped from 1 to 0 when the third out came at field 16152) |

Run D (§3.10) read these bytes at every call. Each change matched the traced event: a foul
gave strikes 0 -> 1, a ball gave balls 0 -> 1, an out reset the count with outs +1, and a
called strike three reset everything and flipped `+0x1CD0`.

### 3.6 Option words read by this code (`G+0x10` -> options)

| Offset | Read by | Value in a default Play Now game (Pro) | Meaning | Tag |
|---|---|---|---|---|
| `+0x50` | the pitcher's cursor gate (`fn_802F83E4`) | 2 | cursor style: 0 off, 1 no fade, 2 fade (I from the draw code); not the circle | V, T |
| `+0x6C`, `+0x70` | the pitcher's cursor gate (indexed by `+0x1CD0 == 0`) | 0 for P1's team, -1 for the CPU | controller slot per team (index 0 = home) | V, T, I |
| `+0x74` | variable zone | 0 | must be 0 for the zone | V, T |
| `+0x140` | `fn_801BB338` (index into 5-entry tables) | 2 | difficulty level | V, T, I |
| `+0x148` | variable zone | **0** | variable strike zone on/off | V, T |
| `+0x14C` | blown base calls (`fn_801BCFA8`, `fn_801BE6BC`) | **1** | blown out/safe calls on/off | V, T |

Which front-end setting writes each word is not traced (U5).

### 3.7 The post-pitch marker

**Tunables.** The "Cursors" group is registered by `fn_802FBC7C` (V):

| Tunable | Stored at |
|---|---|
| `PitchLocRed`, `Green`, `Blue`, `Alpha` | tunable object `+0x44`, `+0x48`, `+0x4C`, `+0x50` |
| `PitchLocScaleX` | global `0x8068E148` |
| `PitchLocScaleY` | global `0x8068E14C` |
| `PitchLocFadeTime` | global `0x8068E150` |
| `PitchSize1ScaleX/Y` | globals `0x8068E154`, `0x8068E158` |

The names are the strings at `0x805E7088`-`0x805E70F0`.

**Readers.**

| Global | Read by | Tag |
|---|---|---|
| ScaleX/Y `0x8068E148`/`0x8068E14C` | `fn_802F893C`, the **circle**: one draw through `fn_802F3ABC` | V |
| | `fn_802F83E4`, the pitcher's cursor: two draws | V |
| PitchSize1 `0x8068E154` | `fn_802F893C`, second block | V |
| FadeTime `0x8068E150` | `fn_802F4B68` (HUD update: clamps the pitcher-side timer `+0x74`) | V |
| | `fn_802F4FC8` (HUD tick: sets `+0x28` while `+0x7C < FadeTime`) | V |
| | `fn_802F83E4` (alpha = 1 - clamp(`+0x74`)/FadeTime in its mode 3) | V |

**The HUD object** is `Unk802F42A4`, the hitting/pitching interface: one instance,
`0x8128CC00` in every run (T). It holds the batter pointer `P->m6CC` and the location of
§3.3. Its tick `fn_802F4FC8` calls two draws per frame poll in play (T: 140 calls each in
140 fields):
- `fn_802F83E4` at `0x802F52C0`;
- `fn_802F893C` at `0x802F52C8`.

**The circle where the pitch crossed is `fn_802F893C`'s first block** (V, T, seen in
captures):
- **Setter:** `fn_802F4F40` (`0x802F4F40`) runs from the HUD update. While the batter
  `P->m6CC` has `+0x38C == 2`, it copies the batter's `+0x3C0` (3 floats: the crossing
  point) to `hud+0x94`. It sets **`hud+0x90 = 1`** once the batter's `+0x3F4` reaches
  `fn_80190F0C(batter)`.
- **Clear:** `fn_802F4B68` clears `hud+0x90` when the pitcher-side mode `hud+0x70` is not
  0 or 4 (`0x802F4DE8`).
- **The draw:** `fn_802F893C` tests `hud+0x90` at `0x802F8964`-`0x802F897C`. When it is set,
  it draws one marker at `hud+0x94` scaled by PitchLocScaleX/Y (`0x802F8A64`), then jumps to
  `0x802F8BD8`.
- **The second block** (`0x802F8BD8` on) is gated by `hud+0x80` and more conditions. It maps
  the normalized location (`fn_8022EA60`) to the world, stores it at `hud+0x84` and draws a
  second marker scaled by PitchSize1. What it shows is unknown (U1).
- **Run F (P1 batting, never swinging):**
  - `hud+0x94` was filled at field 10777, right after the release at 10776.
  - `hud+0x90` went 0 -> 1 at 10813; the call came at 10815 (a ball, count 1-0 -> 2-0).
- **Run E's captures** on the same timeline:
  - 10785: no circle;
  - 10830: a small white circle high and outside, next to the strike-zone corner brackets
    and an "87 MPH / Slider" readout;
  - 10905: the circle still shown, 11 fields before the next pitch begins.
- **Run G (CPU vs CPU):** the flag rose one field before the call and cleared when the next
  pitch began, so no controller is needed for the circle (T).
- **So the circle stays up through the whole span a challenge window would cover.**

**`fn_802F83E4` is the pitcher's cursor, not the circle.**
- Gate (all must hold, V):
  - `hud+0x68 != 0` and global `0x8061B898 != 0` (it reads 1, T);
  - the controller slot `options+0x6C[+0x1CD0 == 0]` is not -1;
  - `options+0x50 != 0`;
  - mode `hud+0x70 != 0`;
  - `hud+0xCC == -1`;
  - not (`options+0x50 != 1` and mode == 4).
- Its mode stays 0 in CPU vs CPU and when a human bats (T, runs B and E).
- With a human pitcher it is 1, then 2, during pitch selection (T, run C).

**Ways to hide the circle, best first:**

| # | How | Notes |
|---|---|---|
| a | **Before-hook on `fn_802F893C`** writing `hud+0x90 = 0` (r3 + 0x90) while the mod hides the circle | Skips exactly the circle block (V). In the two HUD units the flag is written only by the setter `fn_802F4F40`, the clear in `fn_802F4B68` and two init/reset writes (`0x802F4414`, `0x802F45F0`), and read only by `fn_802F893C`; other units were not searched. It is re-set every update, so the hook clears it every frame. The second block and the rest of the HUD are untouched. During the review presentation the hook does nothing, so the circle shows |
| b | Replace-hook `fn_802F893C` and return | Also drops the second block and anything else it draws (U1); only if (a) fails |
| c | Write `options+0x50 = 0` | Affects the pitcher's cursor, not the circle (gate of `fn_802F83E4`); no |
| d | Write `PitchLocFadeTime = 0` | No: `fn_802F83E4` divides by it, and the circle does not read it |

### 3.8 The game's own umpire-error machinery: wired, and when it fires

| Piece | Wired? | Fires when | Evidence |
|---|---|---|---|
| **Variable strike zone** (balls/strikes; debug string "Variable Strike Zone Call" `0x805CFCEC`) | Yes, behind `options+0x148` | Never in a default Play Now game (`+0x148 = 0`). When on, only on pitches between the rulebook zone and that game's rolled zone, with the keep/revert roll of §3.3 | V `fn_801BBC9C`; T `+0x148 = 0` |
| **cBlownCallTunables** (`0x805CFEB8`; class `Unk801C0334`, hash `0xD1E132A4`) | Yes, **on by default** | Per-base out/safe calls. `fn_801BCFA8` (`0x801BD130`) and `fn_801BE6BC` (`0x801BE930`) roll `rand * k < prob(base)` when `options+0x14C != 0`, then mark the call blown (`fn_80255784`/`fn_802554BC`). Not ball/strike | V; T (`+0x14C = 1`; `fn_801C04E0` called 39 times in run A) |
| ↳ its fields | — | `+0x4` TimeWindowSecs, `+0x8` TagProbability, `+0xC..+0x18` First/Second/Third/Home probability (`fn_801C04E0(base)`); instance at `A+0x68` | V (`tu_801BDD60.cpp`) |
| **cManagerArgumentTunables** (`0x805CF858`; class `Unk801B64B0`, hash `0x575DF764`) | Yes, for **out/safe calls only** | `fn_8017CF14` runs for both teams after every play. It returns early for strike (0x41) and ball (0x48) events. Otherwise it uses `ProbabilityOnBlownCalls` (`+0xC`) when `fn_80324E38` says the call was blown, else MinimumCloseCallTimeDelta (`+0x4`) and ProbabilityOnCloseCalls (`+0x8`). It needs a score gap of 3 or less and that team not having argued (`team+0x110`) | V `0x8017CFF0`-`0x8017D0DC`; T (44 calls in run A, never an argument) |
| **`pitch_ump_bad_call`** (`0x805A7CE4`) | Registered | A commentary event. Its {name, id} pair is at `0x8061735C`; `fn_800418C4` copies it to the handle `0x806870DC`, used by `fn_8034808C` and `fn_80065968`. What plays it is not found (U3) | V |
| **"replay showing blown call by ump"** (`0x805AD4FC`) | Registered | Entry 113 of the constraint-name table `0x80618D00`, which `fn_80057778` loads with `EventCategoryConstraints.csv`. Its evaluator was not found (U3). The game shows replays itself, so this constraint is plausible for its blown base calls | V; T (replays seen) |

**Bottom line:** ball/strike misses exist in the code but are off by default. Blown base
calls are on. Nothing in the game argues or reviews balls and strikes. The commentary
*names* for a blown call exist, but their trigger is unknown, so v1 does not depend on them.

### 3.9 The game's replay system (survey)

- **`cScriptReplay`**:
  - Script class name `0x805C8424`, hash `0x2916F5EA`. Matched C:
    `src/game/tu_801547AC.cpp:300-430`, `tu_8016F9A8.cpp:29-31`.
  - Its tunables: Seek/UseEvent/Event, DeltaTime (-5..5 s), PlaybackSpeed (-2..2), and a
    spline speed curve.
  - `fn_cslot11` seeks a *recorded* track (`actor+0x50`; bounds `fn_80326DF0`/`fn_80326E64`,
    in 1/60 s) to an event-relative time and drives a playback actor (`+0x44` playing,
    `+0x4C` time, `+0x50` rate).
- **It is driven by data-driven camera scripts** ("linearscript", "funclist"). The command
  list at `0x805C7728` includes `cScriptReplay`, `cScriptStopSavingReplay`,
  `cSimToEndOfPlay` and `cPlayManagerArgumentScript` (V).
- **`REPLAYENTER`/`REPLAYEXIT`** (`0x805B94E0`/`0x805B94EC`): presentation event names, used
  in `fn_800BCF98` (V; not analysed further).
- **It is a playback of recorded poses, not a re-simulation.**
- **It fires in normal play:** `fn_cslot11` ran 6 times in run A (T).
- **The PRNG reseed path** (`fn_801A0DE0`/`fn_801A41D0`, DETERMINISM RN2) is a game
  restart, not this replay.

### 3.10 Native-runner confirmation

**Runner.** `mvp-play` from the `engine/clk2` build (`msvc-o2`, built 2026-10-15, CPI 2.000),
copied into a session scratch directory so nobody's build tree was locked.
- Script: CLK2's `game_125.txt`. It goes through the menus, pauses at the first pitch and
  switches to CPU vs CPU. Variants are noted per run.
- Tracing: `--trace-entry` with `--trace-peek`. A peek offset added to r3 wraps mod 2^32,
  so in a deterministic run it can read any absolute address.
- Traces, dumps and captures stayed local. They are game-derived and are never committed.

| Run | Script, frames | Traced | Result |
|---|---|---|---|
| A | CPU vs CPU, 24,000 frames (433 s of guest time) | the umpire path, count handlers, blown-call accessor, manager-argument check, replay player | 26 pitches, 11 reached `fn_801BB1E0`. Each gave exactly one of `fn_801BDA38`->`fn_8006DE84` (6) or `fn_801BD878`->`fn_8006DF00` (5). The zone was rolled once at field 4246. Replays fired 6 times. No walk and no dropped third strike |
| B | same, fields 14,300-14,800 | every event post, the pitcher's cursor draw (`fn_802F83E4`) and the HUD update, the location reads | The pitch timeline below; the cursor mode stays 0 in CPU vs CPU |
| C | P1 pitching (A every 100 frames), 9,000 frames | marker object fields, umpire, release | The pitcher's interface states (§3.7); nothing drawn after the call |
| D | CPU vs CPU, 24,000 frames | option words and count bytes, by absolute peeks at each call | §3.5 and §3.6 |
| E | P1 batting and never swinging (`game_125` with P1 moved to the away team in Select Controller), frames to 13,000, captures every 15 fields from field 10,500 | marker object, umpire, release | Every pitch was called (8 releases, each with a call 39 fields later); the circle shows in the captures from just after each call until the next pitch (§3.7) |
| F | same as E, fields 10,760-10,900 | both HUD draws, with 30 words of the HUD object | `hud+0x90` rose 2 fields before the call; `hud+0x94` was filled at release (§3.7) |
| G | CPU vs CPU, fields 14,500-14,700 | the circle draw with `hud+0x90/+0x94` | `hud+0x90` rose at 14572, one field before the call at 14573, and cleared at 14676 when the next pitch began: the circle is shown even when the CPU bats |

**One pitch** (run B, VI fields):

| Field | What happens |
|---:|---|
| 14418 | umpire reset `fn_801BAFA4` and event 0x31 |
| 14472 | events 0x59/0x5A from the ball (`fn_802321BC`/`fn_80232254`) |
| 14534 | **release**: event 0x3D (`fn_801896E8`), location read by `0x80189AE4`, `0x801CD978` and `0x801D0CA0` |
| 14571 | event 0x3E (`fn_80188FC4`) |
| 14573 | **call**: `fn_801BBC9C`, strike, event 0x41, strikes++ |
| 14674 | next pitch: `fn_801BAFA4` and event 0x31 |

Over run A the pitch reached the plate 200-400 fields after the reset (CPU pitchers). The
next reset always came 101 fields (about 1.7 s) after the call. Release to call was 39
fields for every called pitch in runs B, C and E (10 pitches).

**The delivery, `fn_802321BC` (event 0x5A) to the release** (T, 2026-10-17: `mvp-play`
traces of `0x801BAFA4`, `0x802321BC`, `0x801896E8` and the frame poll, from field 5000, in
three games; then the same games with the mod logging each release):
- **It repeats pitch after pitch over long runs,** within a frame poll or two:
  - **mod off (traces):**
    - batter game (CPU pitching to P1, who never swings): 143-144 polls for all 12 pitches;
    - CPU vs CPU: 144-145 (6 pitches), 135-137 (5), 74-75 (4);
    - run B: 62 fields.
  - **mod on (the umpire's misses change the games):**
    - batter game: 143-144 (8), then 62 (5);
    - CPU vs CPU over 24,000 polls: 144-145 (6), 135-137 (5), 74-75 (10), 144-145 (3),
      62-63 (2).
- **It also changes within one team's stint,** not only when the other team takes the
  field. Mod on: 144 -> 62 in the batter game; 135 -> 75 and 144 -> 63 in CPU vs CPU.
  - The cause is not known. Guesses: a pitching change, or the stretch with runners on.
  - Not tested yet. The one peek at the on-field actor slots `P+0x65C[10..12]` (I: runners;
    all 0) was in the mod-off batter game, whose delivery never changed.
- **A human pitcher's delivery varies** (63 and 144 polls in two pitches with A pressed every
  100 frames). The release waits for the pitching meter.
- The pitch choice before it (new pitch to `fn_802321BC`) takes 44-270 polls.

**A new game: the umpire's set-up `fn_801BACA0`** (V: `src/game/tu_801BAC74.cpp`; T,
2026-10-17):
- **What it does:** it clears the umpire's state and rolls the per-game variable zone (§3.3).
- **When it runs:** traced with the batter game's script, then the pause menu's Restart Game.
  The menu items are Replay, Pitch/Swing Analysis, Roster Options, Game Options, Box Score,
  Select Controller, Restart Game and Quit; a confirmation follows, Cancel or Ok.
  - It ran at field 3863, when the first game started.
  - It ran again at field 7639, just after the restart was confirmed. The new game's first
    pitch (top of the 1st, 0-0, 0 out) came at field 9433.
- **With the mod** (`challenges=1`):
  - a challenge used in the first game left 0;
  - after Restart Game the mod logged "new game: 1 challenge(s) per team";
  - the new game's first called strike could be challenged.
  - The build before the fix refused that challenge ("no challenges left").
- **Quit, then a new game from the main menu,** was not traced. I: it starts a game the same
  way as the first one, which runs `fn_801BACA0` (field 3863).

### 3.11 Names for the page: the challenger and the team

T, 2026-10-17: MEM1 dumps of the batter game (Cardinals at Red Sox), taken at polls 6000,
7030 and 7700. A temporary dump option of `mvp-play`, not committed; the dumps stayed local.
- **`P+0x65C[0..12]`, the on-field actors, are in scorebook order.** Each slot is an actor
  pointer. At all three polls: 0 Schilling (pitcher), 1 Varitek (catcher), 2-8 the other
  Red Sox fielders, ending 3 Bellhorn, 4 Mueller, 5 Renteria, 6 Ramirez, 7 Damon,
  8 Nixon. Slot 9 is the batter (Eckstein), and 10-12 were 0 (I: the runners, §3.2).
- **An actor's last name** is a NUL-terminated string at `+0x360`, in a 32-byte buffer
  (copy at `+0x3D4`; the rest of the buffer is uninitialized heap fill). No first name was
  found near the actor. The batter's actor changes with each plate appearance; Walker's
  name is at the same offset.
- **The team's nickname:** each team record of the count block (`*(G+0x14)` + team x 0xE64,
  §3.5) points at `+0xB0` to the team's strings. The nickname is at `+0x14`: "Red Sox" for
  record 0 (home) and "Cardinals" for 1 (away). Next to it are the short name ("Bos"), the
  abbreviation ("BOS") and the location ("Boston").
- **Checked with the mod** in two games:
  - the overturn test: "Eckstein", Cardinals, challenging a strike;
  - the two-player script: "Varitek", Red Sox, challenging a ball.
- The mod reads these only for the page: the strings never reach game state, and an
  unreadable chain gives "" (the page then shows the team).

## 4. Mod behaviour

### 4.1 Truth and the umpire model

**Truth** is the game's rulebook test, `|x| <= 1 && |y| <= 1` on the location of §3.3, which
is exactly what ABS means.

**The umpire's call** is rewritten in a before-hook on `fn_801BB338`, the function
`fn_801BB1E0` calls right after the decision (as built, `engine/mods/gameplay/abs.cpp`):
- `fn_801BB338` has exactly one caller (`0x801BB258`).
- It is entered with the game's call in r5. `fn_801BB1E0` keeps that call in its stack slot
  `r1+8` and reads it again after the swing test (`0x801BB244`, `0x801BB270`, V).

```
on fn_801BB338(A, flag, r):                  # before its first instruction
  swing = word(T->m3A4 + 0x28C + 4) in {1,2,4,6,7}     # = fn_801D0294, T = A+0x10
  if swing: return                           # swinging strikes are not umpire calls
  truth = |x| <= 1 && |y| <= 1               # x, y = (T->m3B0)->m4 + 0x28 / +0x2C
  if abs.umpire == game: call = r5           # the game's own call (its variable zone, if on)
  else:
    d = max(|x|, |y|) - 1                    # signed L-inf distance to the boundary
    call = truth XOR (rng(0) < P_miss(|d|))
  if a review is re-running: call = its verdict          # §7.2 (the draw still happens)
  r5 = call; [r1+8] = call; record {frame, x, y, truth, call} in mod state
```

**The miss probability** is `P_miss(|d|) = min(1, k * w(|d|))`:
- `w = exp(-|d|/sigma)`, with sigma from `abs.edge`; edge 0 gives w = 1, the owner's flat
  model.
- `k` is calibrated per sigma so that the mean of `P_miss` over called pitches equals
  `abs.miss`, using the locations of called pitches in traced games.
- One unit of x is half the zone width and one unit of y half the zone height, so "near the
  edge" scales with the batter's zone.
- **It is computed in integers:**
  - `w` is a 256-entry Q16 table built by repeated Q32 multiplication with
    `exp(-1/(256 sigma))`, compared against a 32-bit RNG draw.
  - That avoids libm `exp`, whose last bits differ between the native and wasm builds.
  - The location arithmetic is exact in double.

**Why a before-hook on the callee rather than wrapping `fn_801BBC9C`:**
- **Unwinds.** A guest thread switch inside a hooked function unwinds the host frames
  (runtime/native.h). Code placed after `call_original` in a replace-hook would then be
  skipped for that pitch. A before-hook never calls the original, so nothing can be skipped.
- **The game's random stream.** The game still makes its own decision first, drawing from the
  gameplay PRNG when the variable zone is on. So the game's random stream is unchanged up to
  the first call that differs.
- **The game sees the call as its own.** `fn_801BB338` and the strike/ball handlers see the
  umpire's call exactly as if the game had made it, including strikeouts, walks and the
  dropped-third-strike rule.
- **`B->m20`** is left as the game set it. No reader outside the umpire was found (U4).

### 4.2 Hiding the circle

Hide method (a) of §3.7, a before-hook on `fn_802F893C` that writes `hud+0x90 = 0`. It is
active whenever `abs` is on, except during the review presentation, where the flag is left
alone so the game draws the circle itself.

Hiding is required, not cosmetic. The circle appears 2 fields before the call and stays up
until the next pitch begins, so without the hook every challenge window would show the
answer.

### 4.3 Challenge rules

- **Eligible:** a *called* pitch (no swing), within `abs.window` of game time after the call
  (counted in game ticks, the 60 Hz counter `Owner::m4C` of DETERMINISM §1.1, not in frame
  polls, whose rate varies with load). A called strike can be
  challenged by the batting team, a called ball by the fielding team.
- **Teams:** `+0x1CD0` gives the batting side, and `options+0x6C[team]` gives the controller
  port per team (index 0 = home, T). In netplay that maps each player's port to a team.
- **Counters:** each team starts every game with `abs.challenges`. A challenge is spent only
  when the call stands.
  - The mod refills them at the umpire's per-game set-up (`fn_801BACA0`, site since
    2026-10-17). That set-up also runs on the pause menu's Restart Game (§3.10).
  - Before that, a restarted game kept the last game's counters (owner, 2026-10-17).
- **Verdict:** known the instant the challenge is made (truth was recorded at the call).
  The review is presentation.
- **The window closes** at the next new-pitch signal (`fn_801BAFA4`) or when the time runs
  out, whichever comes first.

### 4.4 State machine (host side, per guest timeline)

```
IDLE --fn_801BAFA4 (new pitch)--> request snapshot S_P at the next frame poll
     --fn_802321BC (windup W)----> schedule S_L at W + D - pre - 3 (D: this fielding
                                   team's last delivery, else 62)
     --event 0x3D (fn_801896E8)--> record release frame R; drop S_L if still pending;
                                   learn D = R - W for the fielding team
     --fn_801BBC9C call----------> record call frame C, truth, call; WINDOW (if called pitch)
WINDOW --challenge input, eligible--> REVIEW(K = this frame)       --timeout/new pitch--> IDLE
REVIEW: restore S_L (or S_P if S_L came after R-pre); headless to R-pre; snapshot S_R into
        that slot; present R-pre .. C+M (circle shown, game audio muted, replay speed);
        restore S_R; re-run headless R-pre .. K with forced_call = verdict; spend the
        challenge if upheld; IDLE at frame K+1 (live)
```

### 4.5 Mod state

Mod state is a fixed-size plain block, at most 256 bytes, with no pointers. It holds:
- the RNG state (4 x u32) and the two challenge counters;
- the phase and the window deadline (in ticks);
- R, C, K and the recorded call {x, y as raw bits, truth, call, swing};
- `forced_call` and statistics;
- for the late snapshot: this pitch's first snapshot poll and windup frame, the pending
  request, the fielding team, and the learned delivery per team;
- the batter's and the catcher's actors at the last call, whose names the page shows (§3.11).

Save states copy the block, and the netplay state hash includes it (§5.7, §6.2).

### 4.6 UI

**As built (browser):** an HTML layer over the game, never game state.

**The challenge scene** (`engine/web/src/abs-scene.ts`, 2026-10-17, reworked twice with the
owner's feedback the same day):
- **The moment a challenge is made: the callout.**
  - Who called for it, in the Replay tag's red look but large and in the middle of the
    picture: "ECKSTEIN / HAS CALLED FOR A CHALLENGE". The batter challenges a called strike,
    the catcher a called ball (§3.11).
  - The engine waits 1.5 s (`ANNOUNCE_MS`) before the review starts, so the callout can be
    read. The page passes the time as `modIntroMs`, and the engine takes it through
    `mvp_mod_intro`.
  - During the wait the run is stopped at the frame poll before the review, as when it
    waits for the other player online. Nothing reaches the guest.
  - The callout stays while the review rewinds (usually about 50 ms; up to about 2.6 s when
    the late snapshot misses, §0).
- **During the replay (Presenting): half speed** (owner, 2026-10-17), with only a small tag
  naming the team ("Replay · Cardinals challenge").
  - The paced loop runs the replay's fields at half the pace (`Schedule.setSlow`,
    `worker.ts`), and the engine starts the replay in a batch of its own, so it is paced
    that way from its first frame.
  - Each game frame stays on screen for exactly two animation frames: 30 new frames a
    second, evenly spaced. The game simulates in whole ticks, so true 60 fps slow motion
    would need it to draw frames between its own ticks, which it does not.
  - Measured (paced Chrome, the overturn test): the replay took 3.63 s for 110 frames.
    Intervals between frames: p5 31.8, median 33.4, p95 34.4, max 39.3 ms; only the two
    around the switch (26.6 and 39.3 ms) were off. Live play afterwards was back at
    60 presents a second.
- **During the re-run (Rerunning): the verdict card.**
  - "CALL OVERTURNED" or "CALL STANDS", and nothing under it (owner, 2026-10-17).
  - Where the pitch crossed, drawn in the zone. The zone is 1.5 times its first size (owner:
    it was hard to see): about 275 px wide, capped at 40 % of the window's height.
  - The challenges the team has left, with its nickname: "Cardinals: 2 challenges left". No
    "Back to the game" line (owner).

  It fades out 2.2 s after play resumes.

The mod feeds the scene through `Host::publish` with two records:
- at the challenge: `{"abs":"challenge",team,call,truth,x,y,left,by,club}`:
  - `by`: the challenger's last name;
  - `club`: the team's nickname;
  - `left`: the challenges the team will have after this one (one is spent when the call
    stands; the verdict is known at the challenge);
- when the review ends: `{"abs":"verdict",team,overturned,call,left,club}`.

The page holds the truth back until after the replay. The banner keeps the other messages.
`mvp-play` prints the records as `ui:` lines.

**Not built:**
- challenges left per team on screen outside a review;
- a "Challenge?" prompt during the window.

## 5. Engine hook API v1

### 5.1 What exists, and the constraints it sets (survey of `engine/`, 2026-10-17)

- **Replacement is fixed at translation time.** `rt::hle_table()` (`runtime/hle.h:36-66`)
  lists Replace and Before entries by symbol name or retail address.
  - The translator emits a stub at each matched function (`translator/emit_cpp.cpp:120-177`;
    wasm `wasm_emit.cpp:3563-3671`).
  - A Replace function gets `GuestFn original` and returns like a `blr` with r3 set
    (`hle.cpp:274-278`).
  - Indices are baked into the translated code, so entries are append-only.
- **Direct calls are static** (`f_X(c)`, wasm `call`). Swapping an entry in the native
  table at run time would catch only indirect calls. Only the emitted stub catches every
  caller.
- **`rt::g_entry_hook`** (`runtime/native.h:57-62`) sees every function entry but is one
  global pointer. When set it costs an indirect call on every guest call. Fine for tracing,
  not for shipping mods.
- **The DOL cannot be patched:** `icbi` over modified text is a hard fault
  (`runtime/sys_hooks.cpp:74-81`). That is good, since mods must not patch code.
- **No host API surface exists** for mods: no RNG, no settings, no f32 helpers
  (`ppc/mem_access.h` has read8-64 and write8-64 only).

### 5.2 Binding model

**v1 has a site registry plus translation-time stubs.**
- The engine ships a registry of hook sites, `engine/mods/sites.h` (proposed). Each entry
  has a retail address, kind (`before` | `replace`), a label (the decomp name, informative
  only) and the API version that introduced it.
- The translator emits an HLE-style stub at each site. It reuses the existing mechanism:
  `hle_table()` appends one Before entry per registry site, each calling the handler bound
  there (`runtime/mod_hooks.{h,cpp}`).
- The registry is part of the engine build, so the build id covers it and translated-code
  caches stay valid.
- **With no mod bound,** a replace site calls `original` and a before site returns: one host
  call per call of that function.
- The ABS sites are called at most once per frame poll (the circle draw) or once per pitch.
  Cost is negligible.
- `mvp-xlat` already refuses an entry that matches 0 or more than 1 function
  (`translator/tools/mvp_xlat.cpp:89-99`). HLE skips functions with secondary entries, so
  S1 must check that none of these sites has one.
- **Mod sites may write guest memory,** unlike the SDK "Before" hooks, whose policy is host
  state only (`hle.h:15`). That is the point of a gameplay mod. It stays deterministic
  because every peer runs the same mod with the same settings and seed.

**v1 sites for this mod:**

| Address | Kind | Use |
|---|---|---|
| `0x801BB338` | before | the umpire's call: rewrite r5 and the caller's `r1+8` (§4.1) |
| `0x802F893C` | before | hide the circle: `hud+0x90 = 0` (§4.2) |
| `0x801BAFA4` | before | new pitch: request snapshot S_P |
| `0x801896E8` | before | release: record R; learn the delivery (§7.2) |
| `0x802321BC` | before | the pitch is chosen and the delivery starts (posts event 0x5A): schedule the late snapshot S_L (§7.2). Added in hook API 1.1 |
| `0x801BACA0` | before | the umpire's set-up for a new game, also on Restart Game (§3.10): refill both teams' challenges (§4.3). Added in hook API 1.2 |

v1 needs only before sites, so no mod code runs "after" the original (see §4.1 on unwinds).
`fn_802321BC` runs once per pitch, for CPU and human pitchers. The CPU's pitch choice
(`fn_80232254`, event 0x59) comes in the same field (V: `src/game/tu_8022FE34.cpp`; T: one
per pitch in 29 traced pitches, §3.10).

**Third-party code mods** bind only registry sites in v1. A site is added by an engine minor
release. v2 can add **runtime binding of any function**. The translator would emit a
per-function flag test at entry (one load and branch, folded into the existing entry test
in wasm), and a bound flag would route through the dispatcher. That costs a few percent of
call overhead even when unused, so it should be measured before it is promised.

**Binding is by address.** The label is decoration: a manifest that names a decomp symbol
but no address is rejected.

### 5.3 Host API (as built: `engine/mods/gameplay/api.h`, version 1.2)

**1.2 (2026-10-17):** a site (`0x801BACA0`, a new game).

**1.1 (2026-10-17):** a site (`0x802321BC`). The host keeps the two latest snapshots instead
of one, and a review restores the later one at or before `present_from`. Both changes are
additions: a 1.0 mod that asks for one snapshot per pitch behaves as before.

**A mod implements `mods::Mod`:**
- `info()`: id, name, version, the registry sites it binds, typed settings and the size of
  its state block.
- `start(Host&)`: once, when the match starts.
- `site(Host&, Cpu&, address)`: at a bound site, before the function's first instruction.
- `frame(Host&, Cpu&)`: at every frame poll, after the poll's input was consumed.
- `review_done(Host&, Cpu&, const Review&)`: when a review it requested has finished.

**The engine gives it `mods::Host`:**

| Service | What it does |
|---|---|
| guest memory | `r8/r16/r32/rf32`, `w8/w16/w32`. MEM1 and the VM region only; anything else stops the run with a reason |
| `rng(stream)` | 4 streams of xoshiro128\*\* per mod, seeded from the match seed (§5.5) |
| `setting(key)` | Validated values |
| `state()` / `state_as<T>()` | The mod's plain-data block |
| `frame()`, `input()` | The poll count and this poll's controller input |
| `log()` | A developer log line |
| `notify()` | A short message for the player (the page's banner; `mvp-play` prints it) |
| `publish()` | A small JSON record for the mod's own part of the page, such as the ABS challenge scene (§4.6); `mvp-play` prints it. Never game state |
| timeline | `phase()`, `request_snapshot()` (the two latest are kept), `snapshot_frame()` (the latest), `request_review(Review)` (from the latest kept snapshot at or before `present_from`), `review()` (§7.2) |

**`mods::GameplayHost` (`host.h`) is the engine side:**
- It enables mods with `id:key=value,...` settings.
- It produces the canonical handshake string (`mod.<id>=<version>;mod.<id>.<key>=<value>;...`,
  sorted).
- It binds sites through `runtime/mod_hooks.h`.
- It saves and loads every mod's state and RNG streams as one blob, for save states and the
  netplay state hash.

**`mods::Timeline` (`timeline.h`) carries out snapshots and reviews at frame polls.** It has
two save-state slots. Live, each snapshot replaces the older one. A review restores the slot
it rewinds from, then saves the replay's first frame into that same slot, so two slots are
enough. A tool gives it three callbacks:
- presenting on or off;
- saving its host-side views with a slot;
- restoring them.

`mvp-play` and the browser engine (`mvp_web.cpp`) implement those.

**Rules:**
- **Deterministic code only:** no host clock, no host randomness, no floating point that can
  differ between builds (no libm transcendental functions; integer or table math instead),
  and no allocation that changes behaviour.
- **State lives only in `state`.** The engine owns that block, snapshots it and hashes it.
  A mod keeps nothing in statics.
- **A callback must not unwind the guest** (no guest thread switch inside `on_call`).
  `call_original` is the only way to run guest code, and arbitrary guest calls from host
  code are not in v1.
- **Writes to guest memory are allowed** but go through `gp_check` like translated stores
  (`ppc/mem_access.h:23-34`).

### 5.4 Where mod code runs

- **v1 (this mod):** a first-party mod compiled into the engine (C++, native and wasm) and
  written only against `hook_api.h`. The engine build id already covers its code; its
  enablement and settings go in the settings string.
- **v2:** third-party code mods as **WebAssembly side modules** inside a pack. They import
  only `Host`, have no other imports and no shared memory, so they are sandboxed and
  deterministic. The pack's content hash joins the pack-set hash in HELLO.
- That needs a `code` section in the pack format (engine API 1.x -> 1.(x+1), `MODDING.md` §11).

### 5.5 Randomness

- **The match seed:** the HELLO `seed` field already exists, and player 1's proposal is used
  (`net/session.cpp:171`). It is unused today: hosts leave it 0 (`net_web.cpp:98-109`).
  - v1 makes player 1's host fill it from `crypto.getRandomValues`.
  - Offline, the engine draws one at game start and stores it in the input recording's
    header (E4), so recordings replay identically.
- **Streams:** `seed_stream = SHA-256("mvp-mod-rng/1" || match_seed || mod_id || stream)[0..16]`
  feeds xoshiro128\*\*.
- **State** sits in the mod state block (§4.5), so rewinds restore it.
- **The game's own PRNG is never used by mods.**

### 5.6 Settings and the handshake

- **The manifest declares typed settings** with ranges. The engine validates them and
  writes them canonically into the HELLO settings string as `mod.<id>=<version>;` plus
  `mod.<id>.<key>=<value>;` entries.
  - The string is sorted and compared exactly (`net/protocol.cpp:267-289`), with a limit of
    512 bytes (`protocol.h:53`).
  - ABS needs about 90 bytes.
- **A difference refuses the game** with a reason ("different mod settings: abs.miss"), like
  any other settings mismatch.
- **Mid-match changes are not allowed** in v1.

### 5.7 Netplay hash, recorder and replays

- **The state hash** (`net/state_hash.cpp`) folds in every mod state block and the rewind
  epoch (§7.5).
- **The E4 recorder** records the match seed, the mod list and the settings. A recording made
  with a mod replays only with the same mod build.

### 5.8 Versioning and tests

- Hook API semver is separate from the pack format until code packs exist, then joined.
- Minor versions add sites and services; a major version changes signatures.
- **Disc-free tests:** memory-bounds refusal, settings canonicalization, RNG stream vectors,
  and a state-block round trip.
- **Disc tests (local only):**
  - Mod off: the guest line equals a build without the sites, which proves zero impact.
  - Mod on: two runs with the same seed give identical guest lines.

## 6. Save states v1

### 6.1 When

- **Only between `rt::run()` calls.** Mid-run, guest frames live on the host stack and, in
  wasm, in locals (`native.h:19-21`, `wasm_emit.h:15-18`). Yields are refused inside
  interrupt handlers (`host_yield.cpp:17, 32`).
- **The natural point is the frame poll:** a Before hook at `0x8014865C` calls
  `YieldingHooks::stop_before`. Netplay already stops there.
- **A request** (`request_snapshot`) made in a hook mid-frame is carried out at the next
  frame poll.

### 6.2 Contents (from the engine survey; file:line in `engine/`)

| Part | Size | Notes |
|---|---|---|
| `ppc::Cpu`: GPR, FPR (ps0/ps1), CR, LR, CTR, XER, FPSCR, MSR, PC, ticks, reserve, SR, SPR | ~5 KB | Skip the host pointers (`mem`, `hooks`, `env`); recompute `next_event` (`ppc/cpu.h:99-120`) |
| MEM1 | 24 MiB | `Machine::mem1_` (`runtime/machine.h:19-20, 53`) |
| VM region RAM | 0-32 MiB (8 by default) | `Machine::extra_ram_`; recreate the mapping (`hle.cpp:81-90`) |
| ARAM | 16 MiB | `Hardware::aram_` (`hw.h:306`) |
| Virtual clock: TB offset, decrementer value, base, fire time, pending, CPI | <100 B | `SystemHooks` (`sys_hooks.h:73-89`) |
| Device registers | 36 KiB | `Hardware::regs_` (`hw.h:283-284`) |
| Device state | small | VI, AI, DSP HLE (mailboxes and pending output), AI DMA, DI (incl. completion time), EXI + SRAM + RTC, PE (`hw.h:173-306`) |
| Write-gather pipe | 56 B | `WgpState` (`hw.h:78-85`) |
| GX FIFO parser | <2 KiB + pending | `gx_fifo.h:97-122`. **It affects the guest:** draw-done and token interrupt timing depend on it (`hw.cpp:287-300`) |
| Input | small | `InputQueue` and the script cursor (`input.h:115-162`); `g_frame_polls` (`hle.cpp:148`) |
| `YieldingHooks` armed state | small | `host_yield.h:52-58` |
| Renderer GX shadow: BP, XF memory and registers, VAT/VCD, arrays, TEV, TLUT 512 KiB, TMEM 1 MiB | ~1.6 MiB | Host only (`gfx/gx_processor.h:173-203`), but needed to render the first frame after a restore |
| Mod state blocks | <= 4 KiB each | §5.3 |

**Not included**, because they are rebuilt or are not state:
- the page table (derived from BATs, MSR and the VM mapping);
- translated code and tables (keyed on the DOL only);
- statistics and logs;
- the audio mixer's shadow (it ramps from silence);
- GPU textures, which are invalidated by bumping the renderer epoch (`renderer.h:43`);
- **EFB contents and EFB-copy textures:** they exist only on the GPU. The first frame after
  a restore can show a stale copy, for example a shadow texture, until the game redraws it
  (R4).

**Never committed, never uploaded:** a save state is game-derived (ARCHITECTURE §3).

### 6.3 Restore procedure

1. Drain the render thread (`render_sync`) in the multi-threaded web build.
2. Copy the blocks back and re-derive the page table.
3. Reset the runtime globals to their idle values (`g_unwinding`, `g_resume`, `g_yield`,
   exception depth).
4. Reset `next_event`, bump the texture epoch and flush the audio output ring (fade in over
   about 100 ms).
5. Set the input queue to the snapshot's poll; later inputs come from the ring.
6. Continue at `Cpu.pc`, which is always a function entry or a resume label at a frame poll.

### 6.4 Size and cost budget

- **Size:** about 50 MiB raw. Two slots are preallocated, with no allocation at run time.
- **ARAM** is written only by DMA, which the hardware model sees (ARQ/AR registers and DSP
  DMA). Dirty-block tracking can copy only the changed 64 KiB blocks, often a small fraction.
- **MEM1** is copied whole: no cheap write tracking exists for translated stores.
- **Budget, to be measured in S3/S5:**
  - snapshot p99 at most 4 ms native and at most 8 ms in Chrome;
  - restore at most 6 ms native and at most 12 ms in Chrome.
- **Two snapshots per pitch** (2026-10-17, §7.2):
  - S_P lands in the dead time after the new-pitch signal, 200-400 polls before the plate.
  - S_L lands during the delivery, about 0.3 s before the release. It is never taken after
    the release, so never with a pitch in flight.
  - Their frame-time cost in a paced Chrome game is in §0 ("Review length").
- The owner judges by feel, so report the frame-time spike at the snapshot, not an average.

### 6.5 Tests (exits in §8)

- **Round trip:** snapshot at poll F, run N polls, restore, run N polls. The guest line
  (ticks, TB, polls, MEM1 SHA-1) must equal the first pass, at 100 random F in a full game,
  including during a load (DVD in flight) and with audio playing.
- **Netplay:** both peers restore and re-run and reach the same hash.

## 7. Replay route

### 7.1 Decision: route B, engine save states (owner, 2026-10-17)

Route A, the game's own replay, was surveyed (§3.9) and rejected:
- It plays recorded poses through camera scripts, so we would have to author or reuse a
  script.
- The circle is HUD state, not part of the recording.
- Nothing in it can change the committed call.

Route B reuses one general engine feature (save states). It shows the pitch exactly as it
happened and gives an exact, deterministic way to apply the corrected call.

### 7.2 How route B applies a verdict

The brief's "restore the post-call snapshot, then apply the corrected call" **cannot work
directly.** By the time a challenge arrives:
- the call has been committed (strikes or balls incremented, §3.4);
- it may have ended the plate appearance (strike three or ball four, with runners advanced
  and event 0x3A posted);
- it may have ended the half-inning (T: field 16152).

Patching bytes in that state would have to undo all of that. So the verdict is applied by
**re-running the pitch with the call forced**:

```
live:     ...S_P(new pitch)...W(windup)...S_L.R(release)....C(call)....K(challenge)
review:   restore S_L (S_P if S_L came after R-pre) -> headless to R-pre -> snapshot S_R
          -> PRESENT R-pre .. C+M   (render; circle shown; guest audio muted; replay pace)
          -> restore S_R -> headless R-pre .. K with forced_call = verdict
          -> live again at K+1
```

- **Upheld:** the re-run reproduces the original timeline bit for bit (same inputs, same
  RNG state). In debug builds this doubles as a free determinism check: the MEM1 hash at K
  must equal the hash recorded live at K.
- **Overturned:** the game makes the correct call at C, and everything after it is
  consistent: count, outcome, runners, stats and commentary.
- **The presentation is discarded,** so it may write guest memory freely, because S_R is
  restored afterwards.
  - With the hide hook idle, the game draws the circle itself, whoever bats: the flag rises
    in CPU vs CPU too (T, run G). No forcing is needed.
- **S_P is taken at the new-pitch signal,** in dead time. A review from it alone must first
  run headless through the whole pitch choice and delivery: 320 polls in the overturn test.
- **S_L, the late snapshot (2026-10-17), keeps that rewind short.**
  - At the windup (`fn_802321BC`, §5.2) the mod schedules S_L to land 3 polls before
    R - pre. It predicts R from the delivery this fielding team's pitcher threw on the last
    pitch (§3.10), or 62 polls before the team's first pitch.
  - The host keeps both snapshots. A review restores S_L when it is at or before R - pre,
    else S_P.
  - S_L is not taken after the release: a request still pending then is dropped, so it never
    stalls a pitch in flight.
  - Content is unchanged: the replay still starts at R - pre.
  - The two slots are reused (§5.3), so no third 50 MiB slot is needed.
- **Options weighed, and why not:**
  - **A snapshot at the release, with the replay starting there.** It would lose the windup
    lead-in (`pre`), and its stall would land on the pitch in flight.
  - **A fixed number of polls after the new pitch.** The pitch choice takes 44-270 polls
    (§3.10), so no fixed number lands near the release.
  - **A periodic ring.** Every snapshot is a frame-time spike. A learned delivery needs only
    one extra per pitch.
- **When S_L misses** (T over 39 pitches in two native games, §0):
  - **A team's first pitch:** the 62-poll guess lands early, a rewind of about 80 polls.
  - **A delivery shorter than the team's last one:** S_L is dropped, and the review rewinds
    from S_P.
- **`pre`** is a short lead-in, about 0.3 s, so the replay opens on the delivery. **M** is
  how long the replay holds on the circle after the call, about 1 s at replay speed.
- **Re-run length:** about (R - pre .. K) = 0.65 s of flight + up to 1.5 s of window + pre.
  Natively that is about 1-1.5 s of headless host time (runs went 1.4-2.4x real time with
  no renderer on this build). It sits behind the "Reviewing..." card.

### 7.3 Inputs during the review

- Guest inputs for polls S_P..K come from the input ring. In netplay both peers have
  identical rings, because those inputs were exchanged in lockstep.
- Input sampled during the presentation is not delivered to the guest. Only "skip replay",
  which is local, is read.
- Inputs for K+1..K+D (the input delay) were already exchanged and apply normally after the
  review.

### 7.4 Audio and commentary during the review

- **Presentation:** guest audio output is muted at the mixer, which is a read-only observer
  (D5), so muting changes no game state. The host plays its own authored replay sting and
  can keep a host-side crowd bed. AX state advances in guest memory but is discarded with
  S_R.
- **Re-run:** headless and silent. Live audio resumes at K+1 with a fade-in, because the
  commentary engine may be mid-line at that point (R5).
- **The overturned call is not re-voiced** (the umpire's "Strike three!") because C has
  passed. A later option can render from just before C live after an overturn, so the
  umpire visibly makes the right call.
- **Playing the game's own cue** `pitch_ump_bad_call` on an overturn needs the commentary
  trigger path (U3). That is S7, not v1.

### 7.5 Lockstep netplay

- **The challenge is a controller input,** so it is in the lockstep input of poll K on both
  peers. Both enter REVIEW at the same poll, and the rewind and re-run are deterministic.
- **The presentation may run at different wall-clock speeds,** or be skipped by one player.
  Lockstep re-synchronizes at poll K+1, where the faster peer waits.
- **Keepalive:** the engine must keep servicing the network during a long headless re-run.
  Run it in slices of one VI field with `request_yield`, so INPUT keepalives (every 20 ms)
  continue and the session does not time out.
- **Desync hash:** the rewind bumps an epoch that is folded into the hash chain.
  - Hash records for re-run polls are not exchanged; checking resumes at K+1.
  - Records already compared on the old timeline stay valid, because they matched.

### 7.6 The challenge input

`PadState` v1 has no spare bit. Unknown button bits and reserved flag bits are rejected on
the wire (`runtime/input.cpp:53-56`, `net/protocol.cpp:94-97`). There are two ways:

1. **Recommended: input v2 with an engine-buttons byte.** It is never delivered to the
   guest, which is masked at PADRead as today.
   - Mapped to buttons a GameCube pad lacks: gamepad Back/Select (or L3), keyboard `C`.
   - It is a protocol major version (old peers cannot parse it), cheap in practice because
     build ids must match anyway (NETPLAY.md "Versioning").
   - The E4 recorder stores it too.
2. **Native prototype only: a GameCube combo** during the window (for example Z+L), read by
   the mod. The guest sees it too, so its side effects at that moment must be checked (U6).

## 8. Staged plan

Each stage ends with a check anyone can run, and later stages build on earlier ones.

| Stage | Work | Exit |
|---|---|---|
| **S0** | This document | Owner OK; the proposed entries (§12) taken by the lead |
| **S1** Hook API core + hide the circle (native) | `hook_api.h`, the site registry and dispatcher, guest-memory and settings services, frame callback; ABS part 1 | `mvp-xlat` accepts every §5.2 site (each matches one function, no secondary entries). With the mod off, the guest line equals a build without sites. In a P1-batting, never-swinging run (run E's script) the circle appears in captures with the mod off and not with it on, at the same fields. Disc-free unit tests pass |
| **S2** Umpire model (native) | After-hook at `0x801BBC9C`, engine RNG and seed, the edge table, stats | Over at least 300 called pitches (CPU vs CPU, several seeds) the measured miss rate is within ±2 points of the slider at 0, 8 and 20 %; with edge 3 at least 80 % of misses have \|d\| < 0.15. Same seed: identical guest lines; different seed: different. Every override shows the matching event and count change in a trace |
| **S3** Save states (native) | Snapshot/restore at the frame poll, two slots, ARAM dirty tracking | 100 random round trips over a full game give identical guest lines, including during a load. Snapshot p99 <= 4 ms and restore <= 6 ms on the owner's machine. Captures show at most one stale frame after a restore |
| **S4** Challenge + review (native) | Challenge input (prototype combo), window, state machine, presentation and re-run | A scripted challenge of a known miss is overturned, and the count bytes after the review equal truth. An upheld challenge gives a MEM1 hash at K equal to the live hash at K. Counters change only on upheld calls. Overturned ball four and overturned strike three each continue the plate appearance |
| **S5** Browser | wasm build, overlay UI, snapshot cost in Chrome | Snapshot p99 <= 8 ms in Chrome on the owner's machine. A full review works in the browser. The owner plays an inning and judges it by feel (report spikes, not averages) |
| **S6** Netplay | Settings in HELLO, the real seed, input v2 engine buttons, hash epoch, keepalive during re-run | Two browsers on different networks finish a game with at least 3 challenges (both outcomes): no desync, the same verdicts on both screens, a mismatched setting refused with a reason |
| **S7** Polish | CPU challenges, slow-motion tuning, the commentary cue on overturn (needs U3), a per-game ABS stats line | Owner's call |

## 9. Risks

| # | Risk | Mitigation |
|---|---|---|
| R1 | Snapshot copy (about 50 MiB) causes a visible hitch every pitch in the browser | Take it in the dead time after the new pitch; ARAM dirty tracking; measure in S5; drop to snapshotting only when a challenge is still available |
| R2 | Headless re-run is slow in the browser (in-play poll rate is already below 60, E3C) | Short re-runs (S_P re-taken near release); slice and hide behind the review card; measure in S5 |
| R3 | A hook site has a secondary entry, so HLE skips it | S1 exit. Otherwise bind the nearest single-entry caller (for example `fn_801BB1E0` with the swing test inline) |
| R4 | EFB copies are GPU-only, so the first frame after a restore is stale | Accept one frame, or hide it under the review card's transition |
| R5 | Audio continuity after the review: commentary resumes mid-line | Fade in; later, an option to resume rendering from just before C |
| R6 | Clearing `hud+0x90` hides more than the circle, or a second path also draws it (the second block, U1) | S1 captures with the hook on and off; fall back to hide method (b) |
| R7 | Determinism holes that only rewinds expose (host state outside §6.2) | The upheld path's hash check (S4) catches them on every challenge in debug builds |
| R8 | Pad v2 is a protocol major version | Cheap while build ids must match; schedule with E5 maintenance |
| R9 | Calibration: "20 %" means different things flat and edge-weighted | Show the measured rate in the stats line; calibrate per edge setting (S2) |

## 10. Unknowns

| # | Unknown | How to settle |
|---|---|---|
| U1 | What the second block of `fn_802F893C` (from `0x802F8BD8`, PitchSize1-scaled, gated by `hud+0x80` and more) draws; what draws the "87 MPH / Slider" readout | S1: captures with the circle hook on; a trace of the second block's gate |
| U2 | The meaning of events 0x3D, 0x3E, 0x31, 0x59/0x5A (release, catch, new pitch, windup?) | Captures at those fields; a release trace with a human pitcher |
| U3 | What plays `pitch_ump_bad_call`, and how the constraint "replay showing blown call by ump" is evaluated | Trace the cue handle `0x806870DC`'s users and the constraint evaluator during a blown base call |
| U4 | Readers of `B->m20` outside the umpire TUs | A memory-watch tool or exhaustive xref; v1 leaves it untouched |
| U5 | Which front-end options write `options+0x50/+0x148/+0x14C` | Toggle them in Game Options in a scripted run and re-peek |
| U6 | Side effects of a GameCube button combo during the window (prototype input) | S4 trace |
| U7 | Subscribers of events 0x40 (strikeout), 0x31 and 0x3E: none has a case in G's switch (`tu_8006902C.cpp`) | Trace the bus dispatch `fn_8037893C` into its subscribers |
| U8 | Browser snapshot and re-run costs | S5 measurements |

## 11. What this mod does not touch

- **No decomp source changes.** Decomp source stays byte-identical, and mods never depend on
  it (ARCHITECTURE §2).
- **No translator semantics changes.** It only needs appended `HleEntry`s for the registry
  sites.
- **No game data changes.**
- Every binding is a retail address. The decomp names here are labels.

## 12. Proposed entries for the lead's shared files

**DECISIONS.md (owner decision):**
> 2026-10-17, owner: the ABS challenge mod reviews a challenged pitch by rewinding with
> engine save states (route B in `docs/mods/ABS_CHALLENGE.md` §7), not with the game's own
> replay system.

**DECISIONS.md (lead decisions to make):**
- Hook API v1 binds registry sites through appended HLE entries (§5.2). Runtime binding of
  any function is deferred to v2, pending a cost measurement.
- Save states v1 at the frame poll, with the contents of §6.2.
- Netplay input v2 adds a host-only engine-buttons byte (protocol major version).
- The HELLO seed becomes live: player 1 proposes 64 random bits, and the recorder stores
  the seed.
- Hook API 1.1 (2026-10-17, §5.3): a minor version under §5.8's rule. It adds the site
  `0x802321BC`, and the host keeps two snapshots instead of one, so a review can rewind
  from the later one (§7.2). Both are additions; 1.0 mods behave as before.
- Hook API 1.2 (2026-10-17): the site `0x801BACA0` (a new game, §3.10). An addition.

**ROADMAP.md:**
- Engine track: add "E8 Mods with code: hook API v1 and save states (exit: ABS challenge
  S1-S4 natively)", and "E9 ABS challenge in the browser and netplay (exit: S5-S6)".
- The K5 game-state map becomes partly unnecessary for rollback: whole-machine save states
  (§6) do not need it.

**PROGRESS.md:**
> 2026-10-17, claude-side-abs: scoped the ABS challenge mod (`docs/mods/ABS_CHALLENGE.md`).
> Mapped the umpire's ball/strike decision (`fn_801BBC9C`), the commit through events
> 0x41/0x48 to the count at `*(G+0x14)+0x1CCA/+0x1CCB`, the post-pitch circle (`fn_802F893C`,
> flag `hud+0x90`) and the game's blown-call machinery; confirmed with seven traced native runs
> and captures. No code changed.
> Next: hook API v1 + hide the circle (S1).
>
> 2026-10-17, claude side session (ABS fast rewind): an ABS review now rewinds from a late snapshot
> the mod takes during the delivery, so the headless seek before the replay fell from 4.8 s to
> 46 ms in Chrome (overturn test). Hook API 1.1 (site `0x802321BC`, two snapshots kept).
> Guest lines unchanged (`c1bccfad`, `4d94b897`, netplay `b7363882`). The extra snapshot's
> frame-time cost is in §0.
