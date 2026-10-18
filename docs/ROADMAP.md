# Roadmap

This file says what the project is for, how we get there, and what to work on next.
It replaces `docs/PHASES.md` (owner, 2026-10-14). The engine design is in
[`ARCHITECTURE.md`](ARCHITECTURE.md). The rules are in [`../CLAUDE.md`](../CLAUDE.md).

## 1. The goal (owner, 2026-10-14)

In priority order:

1. **MVP Baseball 2005 runs in a web browser.**
2. **Online multiplayer in the browser.** The owner's VPS runs matchmaking. Each
   player's game runs locally in their own browser.
3. **New rosters and teams are easy to add.**

Above all, build on a foundation that creates no tech debt. Mods must never need
rework because the decomp moved on. A 100% matching decomp is welcome, but it is not
the goal. Function matters, novelty doesn't. Later wishes, after these three: deeper
gameplay mods, 60 fps, widescreen and higher resolution.

**Done looks like this:** a friend opens the owner's site and picks their own disc
image. They join a lobby, choose a roster pack, and play a full exhibition game
against another player in another browser. On the owner's private deployment the disc image comes from the private server (reached through its private link)
(owner, 2026-10-14); otherwise nothing derived from the disc ever
leaves their machine.

## 2. The strategy

Two tracks feed one product.

- **Engine track (E milestones, `engine/`).** The browser runs the *retail* game
  code. A translator turns the PowerPC code of the player's own `main.dol` into
  WebAssembly. A runtime supplies everything the GameCube hardware and the
  Nintendo SDK did: memory, OS, disc, controllers, timing, audio, and a GX-to-WebGPU
  renderer. This needs no more decompiled code to run the game, so the browser goal
  doesn't wait on the decomp.
- **Knowledge track (K milestones, `src/`, `include/`, `config/`).** The decomp is
  how we understand the game. It names the SDK functions the engine replaces, finds
  everything that can make two runs differ, and maps the roster and team data. It
  also gives the hook points readable names. Byte-identical matching remains the way
  a decompiled function is proven correct, but we now choose *which* functions by
  what the engine and the mods need.

Mods depend only on three contracts that never change (details in
`ARCHITECTURE.md` §2):

1. **The retail binary.** Its code and addresses are fixed forever.
2. **The game's own data formats.** Roster and team packs are patches to the
   database tables and files the game reads.
3. **The engine's mod and netplay APIs**, which we own and version.

Mods are never written against decomp source. So decomp progress, renames
included, can never break a mod. It only makes the game easier to understand.

## 3. Where we are (2026-10-14)

- **Determinism and main loop** (`docs/DETERMINISM.md`, 2026-10-14):
  - **Gameplay already runs at 60 Hz on the console.** The simulation ticks in 1/60 s steps, with up to 3 logic ticks per rendered frame to catch up, so "60 fps" means holding 60 Hz in the browser.
  - The inventory has 65 nondeterminism sources, plus requirements R1-R17 for the engine.
- **Roster data** (`docs/ROSTERS.md`, `tools/rosters/`, 2026-10-14):
  - The 18 database tables in `database.big` are **plain-text tables** (RefPack-compressed). The tools round-trip them byte for byte.
  - Rows are keyed by a djb2 hash of "Last,First" for players and of the 3-letter team code for teams.
  - The player arrays hold 3,250, with 2,921 used. Teams are fixed at 126 (30 MLB, 2 All-Star, 90 minor-league, 4 specials), so new teams replace slots until code hooks raise the limit.
- **Engine:** E0a and **E0b are done.** The retail code, translated by our own translator, boots natively and deterministically to the frame loop (`docs/engine/E0B_LOG.md`). **E1 is done:** the GX command stream renders through Dawn/WebGPU natively (loading screen, EA SPORTS logo, intro movie, title screen; `docs/engine/E1_LOG.md`). **E3 in progress: natively the engine reaches an exhibition game** (first pitch and first ball in play; `docs/engine/E3A_LOG.md`), and audio plays. **E2 is done: the game boots in Chrome, Edge and Firefox** (`docs/engine/WEB.md`). Before that, E2 groundwork showed in-browser translation is viable. The whole DOL translates in 0.33 s, runs at 87% of native speed, and gives the same SDK-log hash as native (`docs/engine/E2_SPIKE.md`). Next: the E2 browser page (a Worker, the disc picker, a WebGPU canvas, measurements on Chrome, Firefox and Safari) and E3 (menus, the first match, audio).
- **Decomp** (main `d0a166c2`): 21,109 / 24,158 functions matched (87.4%);
  3,075,304 / 5,896,176 code bytes (52.2%); 234 of 847 units built from source. The
  live card is in `README.md`.
- **Knowledge targets.** From `python tools/goals/targets.py` at `d0a166c2`. A K1
  function is done when it has its real name. K2-K4 functions are done when they're
  matched *and* named.

| Target | Functions | Named | Matched | Done |
|---|---:|---:|---:|---:|
| K1 SDK boundary: SDK functions that game/EA code calls | 299 | 143 → **295** | 251 | 48% → **99%** |
| K2 roster data: database load/save path | 60 | 4 | 56 → 57 | 7% |
| K3 determinism: time reads, `rand`/`srand`, async callbacks | 192 → 201 | 0 → 2 | 112 → 137 | 0% |
| K4 main loop: `main` and two levels of callees | 81 | 1 | 70 → 73 | 1% |

Updated at the end of 2026-10-14 (main `22d10e4c`, 21,141 functions matched).
- K1 is done apart from OSInit/DVDInit (they need data splits), `__init_user` and one function with no evidence.
- K3 and K4 now need names more than matches. Several K3 functions read `mftb` directly, which has no C form under our rules, so they can't match; for those, "done" means named and documented in `docs/DETERMINISM.md`.

Most of the code the engine needs is already matched. What's missing is
**names and types**: for example, 251 of the 299 K1 functions match, but 156 of them
are still `fn_` placeholders. That is cheap work with a big payoff.

## 4. Milestones

Each milestone has a hard exit you can check. Don't advance on vibes.

### Engine track

| # | Milestone | Exit |
|---|---|---|
| E0a | **Recompiler survey.** Evaluate the GameCube recompilers and runtimes that exist (DolRecomp/GCRecompiler + ModernGekko, GekkoAOT, gcn-recomp, sp00nznet's ww, RingOut) on: license, language, Gekko coverage (paired singles, FPU rounding), whether the output can target WebAssembly, and hooks for HLE, time and interrupts. | `docs/engine/E0_SURVEY.md` exists, and decisions D1-D2 (`ARCHITECTURE.md` §9) are recorded in `docs/DECISIONS.md` |
| E0b | **Boot spike (native).** On the dev machine, translate `main.dol` (output stays in `build/`) and run it with stub HLE. | The game runs from `__start` through `main` into its frame loop and reaches the first VI retrace. A log lists every SDK call by name |
| E1 | **First pixels (native).** GX command stream to WebGPU (Dawn); disc reads through the VFS. | The EA logo and the title screen render recognizably |
| E2 | **Browser boot.** The player picks their disc image (ISO or CISO); translation runs in the browser; WebGPU renders. | Title screen in Chrome from a cold load, with no game-derived bytes fetched from any server |
| E3 | **Playable single player.** Gamepad API and keyboard input, audio, virtual memory card (browser storage), 59.94 Hz pacing. | A full 9-inning exhibition against the CPU: no crash, sound plays, steady frame pacing |
| E4 | **Determinism.** Input recorder and replayer; a hash of main memory every frame. | A recorded 3-inning game replays with identical hashes on two runs, and on two different machines/browsers |
| E5 | **Netplay.** Lobby, signaling and TURN relay on the owner's VPS; lockstep from power-on; input-delay setting; desync detector. | Two players on different networks finish an exhibition game |
| E6 | **Roster packs.** Pack format, loader and an authoring CLI; the pack's hash is part of the lobby handshake. | A pack that changes a team's roster is chosen in the lobby, and both players play with it |
| E7 | **Team packs.** Replace a team slot's name, abbreviation, roster, logos, uniforms and load screen. | A custom team, in a replaced slot, plays a netplay game |
| Later | Extra team slots (code hooks; needs the K2 limit inventory), rollback netcode, widescreen, higher internal resolution, spectators | Planned when E7 is done |

### Knowledge track

| # | Milestone | Exit |
|---|---|---|
| K1 | **SDK boundary named.** Give each SDK function the game calls its real name, proven by a body match against public SDK source (dolsdk2004 via adonis-singh/re4, the CC0 mitsevox port) or an SDK string. | ≥ 98% of K1 functions named in `symbols.txt`, with evidence in the commit messages |
| K2 | **Roster and team data path.** Where the tables sit inside `/data/database/database.big`, their record layouts on GameCube, the loader and saver functions, and every hard-coded team count or table capacity. | `docs/ROSTERS.md`; a `tools/rosters/` reader/writer that round-trips the player's own `database.big` byte-exactly (checked locally, never committed); ≥ 90% of K2 done |
| K3 | **Determinism inventory.** Every time-base read, random-number seed, and async or callback path that can feed game state. | `docs/DETERMINISM.md` lists each source with its address and how the engine controls it; K3 ≥ 90% done; E4 passes |
| K4 | **Main loop and frame pacing.** Name and type the frame loop. Confirm whether gameplay renders at 30 or 60 fps. | K4 ≥ 90% done; the frame rate is recorded in `docs/DETERMINISM.md` |
| K5 | **Game-state map (later).** Where simulation state lives, for save-state sync and rollback. | Planned after E5 |

**Dependencies:**
- E0b needs the K1 names for OS, DVD, VI and PAD.
- E4 needs K3.
- E6 needs K2.
- E7 needs K2's asset naming.
- E1-E3 need no new decomp.

## 5. Next steps, in order

Updated at the end of 2026-10-14.

1. ~~**K1 naming sweep**~~ 99% done (K1a: 94 names; K1b: 58 names plus dtk's cascade). Left: OSInit and
   DVDInit, which need data splits for fstload and EXIBios (see the K1b commit), and `__init_user`.
2. ~~**E0a survey**~~ Done: we write our own translator (D1), with resumable guest threads (D4).
3. ~~**E0b boot spike**~~ Done 2026-10-14: boots to the frame loop, deterministic. Follow-ups: fewer unwinds per thread switch,
   controller input (R14), AX voice state (R8).
4. **K2-K4 worker batches.** First round done: +32 matches (k_01..k_05) and 355 evidence-backed names
   (names-01). Next: a second names pass over the remaining EA reference names (`config/GV4E69/refnames/`),
   then batches from `python tools/goals/targets.py --units K3` (and K2, K4). K3 functions that read `mftb`
   can't match; document them instead.
5. ~~**K2 tools and pack format**~~ Done:
   - `tools/rosters/`;
   - `docs/ROSTERS.md` (§12 team slot assets);
   - pack format v1 (`docs/MODDING.md`, `tools/rosters/pack.py`).

   The engine implements the same spec at E6.
6. **After E0b item 6:**
   - the in-browser compile spike (E2 risk #2): install Emscripten, emit WebAssembly for the whole DOL, measure;
   - E1 renderer groundwork: vendor Aurora's TEV/WGSL and texture decoders (MIT).
7. **Show the new progress** (enabler): K-target and E-milestone progress on the progress card and the dashboard.

**Paused** (owner direction, 2026-10-14): the near-miss tail (95-99.99% functions
outside K targets), Fable hard-function passes on non-target functions, and size-ranked
p-batches that touch no K target. The permuter farm uses CPU only, so it keeps
running. General matching resumes only with token budget the K and E work doesn't
need. The weekly usage cap is the binding limit, so spend it on what moves a milestone.

## 6. History: phases 0-2 (2026-10-10 to 2026-10-13)

The original plan was matching decomp first, then a source port. It ran as:

| Phase | Result |
|---|---|
| 0: Recon | Done 2026-10-10 (`docs/RECON_REPORT.md`) |
| 1: Scaffold, a byte-identical DOL from split assembly | Done 2026-10-10 |
| 2a: Toolchain lock-in on SDK and runtime units | Done: compilers and flags pinned per library |
| 2b: Game code | 87% of functions and 52% of code bytes matched by 2026-10-13. It continues as the knowledge track |
| 3: Source port (native, then WASM) | Replaced on 2026-10-14 by the engine track. A source port needs all of the code and data in portable C first, and mods written against the decomp source would break whenever it changed |
