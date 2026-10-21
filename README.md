> this project is driven entirely by Claude, also known as 'Decomp-slop'\
> This project is purely output focused and cares not of the journey.\
> Thank you\
> -Human

# MVP Baseball 2005 in the browser

MVP Baseball 2005 for the Nintendo GameCube (`GV4E69`, USA, rev 0), playable in a
web browser: alone, against a friend in another browser, or with a modern roster.

The browser runs the **retail game's own code**. The player's disc image is
checked, its `main.dol` is translated from PowerPC to WebAssembly on the
player's machine, and an engine written for this project stands in for the
GameCube: memory, the Nintendo SDK's OS, disc, controllers, timing, audio, and
a GX-to-WebGPU renderer.

This is a personal project in a public repository. It holds source, tooling and
docs only. It holds no game data, and nothing derived from the disc, not even the
translated code.

## What it is, and what it is not

| It is | It is not |
|---|---|
| A static recompiler (PowerPC to WebAssembly) plus a hardware and SDK runtime, built for one game | A general GameCube emulator. It refuses any disc but GV4E69 rev 0 |
| The retail binary, running unchanged, with mods applied through hooks at fixed retail addresses | A source port. The game's own C/C++ is not needed to play, and mods never build against it |
| Deterministic: every player's browser computes the same game from the same inputs | Bit-exact to a real GameCube. The owner chose "runs right, and the same for both players" (DECISIONS F1, 2026-10-16) |
| Backed by a matching decompilation that names things and proves how they work | A finished decompilation. That stopped being the goal on day four (below) |

## Where it stands (2026-10-21)

**Playable.** A full nine-inning game runs in Chrome, Edge and Firefox at full
speed on a desktop (i7-14700K, RTX 4070 SUPER), with sound, controllers and the
in-game memory card.

- **Engine** (`engine/`):
  - The whole DOL translates in the browser in about half a second, then is cached in the browser.
  - In a real game with the ABS challenge on, the emulation thread needs about 7.5 ms per 16.7 ms field on the dev PC. A simulated machine twice as slow, or Chrome held to four efficiency cores, still runs at full speed with 60 new pictures a second.
  - The renderer runs on its own thread (about 3.8 ms a field on the dev PC). The picture can be drawn at 1x, 2x or 3x the console's resolution, and Auto picks one for the GPU.
  - **120 fps** on displays of 100 Hz or more: the Motion setting (Auto by default) draws an in-between picture between each two game frames, with every moving object halfway. The game itself still runs at 60 Hz, so online play is unaffected.
- **Online play:** two browsers in lockstep over WebRTC.
  - The owner's VPS runs the lobby, signaling and a TURN relay, deployed with Dokploy.
  - Input delay is chosen from the measured link.
  - When one machine is slower, both run at a shared pace instead of stuttering.
- **Rosters and teams:** roster and team packs (format v1, `docs/MODDING.md`) patch the game's own database tables.
  - Packs are checked and hashed, and both online players must use the same set.
  - The server can host packs.
  - A full MLB 2026 pack (all 30 clubs and their minor leagues, real headshots and 2026 stat lines) was built locally from public data. Its pipeline is on a side branch, not on main, and the built pack is never committed: the headshots are copyrighted.
- **Gameplay mods:** a versioned hook API with one shipped mod, the ABS challenge (`docs/mods/ABS_CHALLENGE.md`).
  - The umpire misses some calls, and either side can challenge them.
  - A challenge cuts to a rewound half-speed replay, then the call is upheld or overturned.
- **Save states:** F2 saves and F4 loads, in the browser or from a file.
  - The admin's states live on the server.
- **The site:**
  - The page copies MVP 2005's own gray menu look.
  - Anyone can play with their own disc image.
  - A private player link gives friends the owner's disc. It is never public.
  - An admin link gives the owner the disc page, save states and a memory card kept on the server.

What's still rough is tracked in `PROGRESS.md` and the logs in `docs/engine/`. The
main gap is the weakest machines: one about four times slower than the dev PC still slows the game down.

## How it works

```
 player's disc image (picked file, or the owner's server for player-link holders)
   │  identity check: GV4E69 rev 0, main.dol SHA-1 da6becdb...
   ▼
 translator ── main.dol ─► one WebAssembly module (cached in the browser's private storage)
   │
   ▼
 emulation thread (Worker)                              render thread (pthread)
   translated game code + runtime:                        GX state machine
   memory, OS/SDK HLE, disc, VI/AI/DSP/PE  ──GX ring──►   (CPU transform + TEV)
   models, a virtual clock, the input queue               → WebGPU (EFB, copies, XFB)
   │            │                                            │ ImageBitmap per frame
   │            └─ audio ring ─► AudioWorklet (drift resampler, time-stretch)
   ▼                                                         ▼
 lockstep netplay (WebRTC data channel)                 page (canvas, input, panels)
```

- **Translator** (`engine/translator/`): from the CFG of `main.dol`, using
  `config/GV4E69/symbols.txt` for function boundaries. It writes one WebAssembly
  module (or C++ for native builds). Guest registers live in locals, with resume
  labels so that threads and host yields can unwind and re-enter.
- **Runtime** (`engine/runtime/`): the machine, the IPL environment, HLE for SDK
  functions, the hardware models the game reads (VI, PI, CP/PE, AI, DSP/AX, DVD,
  EXI memory card), a virtual clock (1.25 cycles per instruction), and save states.
- **Renderer** (`engine/gfx/`): GX commands to draws (CPU vertex transform, TEV to
  WGSL), then Dawn/WebGPU natively and in the browser.
- **Web shell** (`engine/web/`): the page, the Worker, the threaded engine, audio,
  controllers, the pacing, the disc download, packs and mods panels.
- **Server** (`engine/server/`): TypeScript. Lobby, signaling, TURN config, the
  disc for player-link holders, packs, the admin's states and memory card. Deploy
  guide: `docs/engine/DEPLOY_DOKPLOY.md`.
- **Determinism** is an invariant. Host time and host input reach the game only
  through the per-frame input queue and the virtual clock. Every change is checked
  against recorded "guest lines" (instruction count and a hash of main memory at
  fixed fields), natively and in the browser.

Design and decisions: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md). The browser
build: [`docs/engine/WEB.md`](docs/engine/WEB.md). Netplay:
[`docs/engine/NETPLAY.md`](docs/engine/NETPLAY.md).

## How it got here

The project did not start out to build an engine. It started as a classic
matching decompilation and changed course when the owner's real goals were
written down.

| Dates (2026) | What happened |
|---|---|
| Oct 10 | **A matching decomp.** Recon of the disc. The toolchain was identified: SN ProDG 3.9.3 (GCC 2.95.3) for the game and EA libraries, Metrowerks CW 1.2.5n for the Dolphin SDK. A dtk-template scaffold rebuilt a byte-identical DOL from split assembly. The plan was to finish the decomp, then port the source (Phase 3) |
| Oct 10-13 | **An agent fleet matched most functions.** An Opus lead, up to 13 Sonnet workers, Codex agents and a permuter farm reached 87% of functions but 52% of code bytes in four days. The long tail was big, float-heavy game code. The weekly usage cap, not difficulty, set the pace |
| Oct 14 | **The pivot.** The owner set the real goals: play in a browser, play online, add rosters easily, with no tech debt. A source port needs 100% of the code in portable C, and mods written against decomp source would break whenever it moved. So the engine track began: run the retail binary. The decomp became the knowledge track (SDK names, determinism sources, roster formats) and the near-miss tail was paused. The same day, our own translator booted the DOL natively, Dawn drew the title screen, and the game booted in Chrome |
| Oct 15 | A full nine-inning game, native and deterministic. Lockstep netplay and the VPS server, deployed by the owner. Then the owner's first play test: "unplayable, way too choppy" |
| Oct 15-17 | **Making it feel right.** Q1-Q7 and P1/P2: menu flicker, sound that stretches instead of cutting out, controllers with a guided setup, the disc downloaded once instead of streamed, the renderer on its own thread, multi-second freezes traced to an idle-skip bug (CLK2), emulation 30% faster, pacing between unequal machines online, a frozen block after a scale change |
| Oct 16 | **F1:** deterministic across players matters, bit-exactness to the console doesn't. That unlocked a faster floating-point model |
| Oct 17-20 | **Mods and polish.** The hook API and the ABS challenge, the MLB 2026 roster pipeline, packs on the server, save states, the in-game memory card, an open site with player and admin links, MVP 2005's menu look, thin-line fixes at 2x/3x |
| Oct 21 | **Speed and 120 fps.** The ABS challenge had been forcing the slow single-threaded engine; it now runs threaded like everything else (four efficiency cores: 0.79x and 29 pictures a second, now full speed and 60). The render thread got about 20% cheaper. In-between frames give 120 fps on high-refresh displays. The page counts exactly which pictures reach the screen (`docs/engine/V1_LOG.md`, `V3_LOG.md`, `V5_LOG.md`) |

The old phase plan is kept as history in [`docs/ROADMAP.md`](docs/ROADMAP.md) §6.

## The decompilation (knowledge track)

The decomp is how the engine work understands the game. It names the SDK functions
the runtime replaces, finds everything that could make two netplay runs differ, maps
the roster tables, and gives mod hook points readable names. Byte-identical matching
is still how a decompiled function is proven correct.

| Measure (main, 2026-10-14; unchanged since the pivot) | Done | Total | Share |
|---|---:|---:|---:|
| Functions matched | 21,141 | 24,158 | 87.5% |
| Code bytes matched | 3,091,588 | 5,896,176 | 52.4% |
| Units built entirely from source | 234 | 850 | 27.5% |

![Decompilation progress: percent of code matched, a treemap of units shaded by match percent, and progress over time](docs/progress/progress.svg)

The knowledge targets (`python tools/goals/targets.py`): K1 SDK boundary 99% named;
K2 roster data, K3 determinism and K4 main loop are mostly matched, but most of
their functions are still unnamed.

**Toolchain.** Game code and EA libraries: SN ProDG 3.9.3 (`GNU C++ 2.95.3 SN BUILD
v1.76`, `-Os -G0 -fno-rtti -fno-exceptions -ffloat-store`; EA libraries mostly
`-O2`). Dolphin SDK: Metrowerks CodeWarrior GC/1.2.5n (GC/2.0 for VM and the debugger
driver). Link: SN `ngcld` 1.5.1.692 with `config/GV4E69/ldscript.ld`. The build uses
dtk-template, decomp-toolkit and objdiff, patched for ProDG (`tools/project.py`). The
game shipped with no symbols or map.

**Building the decomp.** Put a disc image or the extracted `mvp.elf` in
`orig/GV4E69/` (gitignored), then run `python configure.py` and `ninja` on native
Windows (ProDG and MWCC are Win32 programs). The build checks the result against the
retail SHA-1 `da6becdbea614d03c4b5eae9f8a0fb08e0a184dd` and writes the objdiff
report.

**What counts as matched** is decided by tools, not agents. `tools/verify/verify.py`
checks protected files, the retail hash, the hard rules (no inline asm, naked
functions or `.incbin`) and score regressions. `tools/match/check_literals.py`
catches a "100%" function that loads the wrong constant or calls the wrong target.

**Building the engine:** [`docs/engine/WEB.md`](docs/engine/WEB.md) §1 (browser)
and [`docs/engine/E0B_LOG.md`](docs/engine/E0B_LOG.md) (native). Toolchain versions are in
[`docs/engine/TOOLCHAIN.md`](docs/engine/TOOLCHAIN.md).

## Who does the work

AI agents write every line of code and docs. The owner dumped the disc from their
own copy, sets the goals, makes the decisions that are theirs
(`docs/DECISIONS.md`), deploys, plays, and never runs commands.

- **Claude Code:** Opus 5.5 leads and builds the engine in side sessions, Sonnet 5.5
  workers match and name functions, and Fable 5.1 takes hard functions when needed.
- **Codex** agents work under the same rules (`AGENTS.md`).
- **Coordination:** `tools/coord.py` keeps a shared roster, unit claims, an inbox and
  one integration lock (`docs/COORDINATION.md`).

## Rules, in short

The full rules are in [`CLAUDE.md`](CLAUDE.md).

- **No game data:**
  - Nothing from the disc, and nothing derived from it, is committed or published: assets, translated code, memory dumps, screenshots, recordings.
  - The owner's own deployment may serve the owner's disc to player-link holders.
  - It may serve the owner's save states to the owner alone.
- Mods depend only on three stable contracts: the retail binary's addresses, the
  game's data formats, and the engine's versioned APIs. Never on decomp source.
- Engine: determinism is an invariant. Every component has tests that need no game
  data.
- Decomp: no inline asm in any form, and no tricks that force a match. The verifier
  decides.
- No GPL code copied in. Dolphin is a reference, not a source.
- Agents never change the verifier, CI or `.claude/`.

## Repository layout

| Path | Contents |
|---|---|
| `engine/` | The engine: translator, runtime, renderer, audio, virtual disc, mods, netplay, web shell, server ([README](engine/README.md)) |
| `deploy/` | Docker compose for the owner's VPS (Dokploy) |
| `src/`, `include/` | Decompiled C/C++, one file per split unit, and shared headers |
| `config/GV4E69/` | dtk config (`splits.txt`, `symbols.txt`), linker script, retail hash, EA reference names, datafile schemas |
| `configure.py`, `tools/project.py` | The decomp's build generator |
| `tools/` | Matching helpers, recon, roster tools (`rosters/`), knowledge targets (`goals/`), the verifier (`verify/`, owner-only), coordination and fleet scripts |
| `docs/` | Roadmap, architecture, decisions, engine logs (`docs/engine/`), modding, game data, patterns |
| `orig/`, `build/` | Disc files and build output. Gitignored, never committed |

## Docs

| File | What's in it |
|---|---|
| [`docs/ROADMAP.md`](docs/ROADMAP.md) | The goals, the two tracks, milestones E0-E7 and K1-K5, history |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Engine design and decisions D1-D8 |
| [`docs/engine/`](docs/engine/) | One log per engine milestone and quality pass (E0-E6, Q1-Q8, P1-P2, CLK2), plus WEB.md, NETPLAY.md, DEPLOY_DOKPLOY.md |
| [`docs/MODDING.md`](docs/MODDING.md), [`docs/mods/`](docs/mods/) | Roster and team pack format v1; gameplay mods |
| [`docs/DETERMINISM.md`](docs/DETERMINISM.md), [`docs/ROSTERS.md`](docs/ROSTERS.md), [`docs/GAME_DATA.md`](docs/GAME_DATA.md) | What can make runs differ; the roster database; EA data formats |
| [`docs/DECISIONS.md`](docs/DECISIONS.md) | Every decision, with open questions for the owner |
| [`PROGRESS.md`](PROGRESS.md) | Session log, newest last |
| [`CLAUDE.md`](CLAUDE.md), [`AGENTS.md`](AGENTS.md), [`docs/COORDINATION.md`](docs/COORDINATION.md) | Agent rules and coordination |
| [`docs/PATTERNS.md`](docs/PATTERNS.md), [`docs/NEARMISS.md`](docs/NEARMISS.md), [`docs/SPEEDUPS.md`](docs/SPEEDUPS.md) | Matching know-how from the decomp phase |
| [`docs/RECON_REPORT.md`](docs/RECON_REPORT.md), [`docs/REFERENCE_SOURCES.md`](docs/REFERENCE_SOURCES.md), [`docs/REUSE.md`](docs/REUSE.md) | Recon, every outside source checked, and what code we may reuse |

## Credits

- [decomp-toolkit](https://github.com/encounter/decomp-toolkit),
  [dtk-template](https://github.com/encounter/dtk-template) and
  [objdiff](https://github.com/encounter/objdiff) by encounter (template license in
  `docs/dtk/LICENSE-dtk-template`).
- [Dawn](https://dawn.googlesource.com/dawn) and emdawnwebgpu (WebGPU), [Emscripten](https://emscripten.org/),
  [SDL3](https://www.libsdl.org/) (native window and audio). Notices for code compiled into the
  browser engine ship in its `licenses/` folder.
- [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) and
  [m2c](https://github.com/matt-kempster/m2c), used from external clones.
- [mitsevox/mvp2005](https://github.com/mitsevox/mvp2005) (CC0): ported SDK, libc,
  libm, libsn and SND sources, and EA reference names.
- [adonis-singh/re4](https://github.com/adonis-singh/re4): Dolphin SDK bodies and
  ProDG notes. [dbalatoni13/nfsmw](https://github.com/dbalatoni13/nfsmw): prior art
  for ProDG matching on an EA GameCube title.

MVP Baseball 2005 is a trademark of its owners. This project is not affiliated
with EA, Nintendo or MLB. It contains no game data and hosts nothing derived
from it. Every player supplies their own disc.
