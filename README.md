> this project is driven entirely by Claude, also known as 'Decomp-slop'\
> This project is purely output focused and cares not of the journey.\
> Thank you\
> -Human

# mvp2005-decomp: MVP Baseball 2005 in the browser

MVP Baseball 2005 for the Nintendo GameCube (`GV4E69`, USA, rev 0), brought
to the web. The goals, in priority order (owner, 2026-10-14):

1. **The game runs in a web browser.**
2. **Online multiplayer in the browser.** Matchmaking runs on the owner's
   server; each player's game runs locally in their own browser.
3. **New rosters and teams are easy to add.**

Above all, it is built on a foundation that creates no tech debt: mods never
need rework because the decompilation moved on.

This is a personal project, and the repository is public. It holds source,
tooling and docs only, never game data. Every player brings a disc image dumped
from their own copy, and everything derived from it stays in their browser.

## How it works

Two tracks feed one product ([`docs/ROADMAP.md`](docs/ROADMAP.md)):

- **The engine** (`engine/`, [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)).
  The browser runs the *retail* game code. A translator turns the PowerPC code
  of the player's own `main.dol` into WebAssembly. A runtime supplies the rest:
  memory, OS, disc, controllers, timing, audio, and a GX-to-WebGPU renderer.
  - Netplay keeps two browsers in lockstep on controller inputs over WebRTC.
  - The owner's VPS runs the lobby, signaling and a TURN relay.
- **The knowledge track** (`src/`, `include/`, `config/`). A matching
  decompilation: C/C++ that the original compilers turn back into a
  byte-identical `main.dol` (SHA-1 `da6becdbea614d03c4b5eae9f8a0fb08e0a184dd`).
  It is aimed at what the engine and the mods need:
  - the SDK functions the engine replaces;
  - everything that can make two netplay runs differ;
  - the roster and team data;
  - the main loop.

  [`tools/goals/targets.py`](tools/goals/targets.py) measures those targets.

**Mods depend only on three contracts that never change:**
- the retail binary's code and addresses;
- the game's own data formats (roster and team packs patch the database tables
  the game reads);
- the engine's versioned APIs.

They never depend on decomp source, so decomp progress can't break them.

## Status

| Track | Where it stands (2026-10-14) | Next |
|---|---|---|
| Engine | Not started | E0: survey the GameCube recompilers, then a native boot spike |
| Knowledge | 87.4% of functions matched; the targets the engine needs are mostly matched but not yet named (below) | K1: name the SDK functions the game calls |

| Knowledge target | Functions | Named | Matched | Done |
|---|---:|---:|---:|---:|
| K1 SDK boundary | 299 | 143 | 251 | 48% |
| K2 roster data | 60 | 4 | 56 | 7% |
| K3 determinism | 192 | 0 | 112 | 0% |
| K4 main loop | 81 | 1 | 70 | 1% |

The milestones (E0-E7, K1-K5), their exit criteria and the ordered next steps
are in [`docs/ROADMAP.md`](docs/ROADMAP.md).

### Decompilation progress

![Decompilation progress: percent of code matched, a treemap of units shaded by match percent, and progress over time](docs/progress/progress.svg)

The card above is live. This table is a snapshot of `main` at `d0a166c2`
(2026-10-14):

| Measure | Done | Total | Share |
|---|---:|---:|---:|
| Functions matched | 21,109 | 24,158 | 87.4% |
| Code bytes matched | 3,075,304 | 5,896,176 | 52.2% |
| Code built from source (units linked from our C/C++) | 386,148 | 5,896,176 | 6.5% |
| Units built from source | 234 | 847 | 27.6% |
| Data bytes matched | 12,710 | 1,369,010 | 0.9% |

- **Matched** means objdiff scores the function 100% against the original
  object code. **Built from source** means the whole unit (every function and
  its data) links from our source, and the final DOL still has the retail hash.
- Of the 2.82 MB of code still unmatched, 2.50 MB has no source yet. The engine
  doesn't need it, because it runs the retail code. The decomp covers it as the
  targets reach it.

| Source folder | What it is | Functions matched | Code matched |
|---|---|---:|---:|
| `src/game/` | Game code in main `.text` | 12,451 / 14,246 | 49.7% of 3.31 MB |
| `src/fe/` | Front-end / franchise UI (`.frontend_codeoverlay`) | 3,132 / 3,408 | 55.2% of 960 KB |
| `src/db/` | Database / roster / trade (`.database_codeoverlay`) | 1,520 / 1,779 | 50.2% of 494 KB |
| `src/ealib/`, `src/ealibs/` | EA shared libraries (EAGL, RealCore, AV/VP6, ...) | 1,368 / 1,665 | 50.4% of 469 KB |
| `src/animation/` | Animation choosers and goals (one retail unity TU) | 967 / 1,057 | 46.6% of 239 KB |
| `src/dolphin/`, `src/sdk/` | Nintendo Dolphin SDK | 696 / 839 | 79.8% of 175 KB |
| `src/snd/` | EA SND 9.02.04 + csis | 437 / 480 | 69.6% of 92 KB |
| `src/gamelib/` | EA gamelib (allocator, datafile, animlib) | 359 / 402 | 49.4% of 85 KB |
| `src/libc/`, `src/libm/`, `src/libgcc/`, `src/libsn/` | SN runtime: newlib, fdlibm, libgcc, libsn | 179 / 273 | 82.0% of 70 KB |

The card updates itself. A scheduled task on the owner's PC runs
`tools/progress_bot.py` every 2 minutes; it builds `main`, redraws the card,
verifies and pushes a `docs: update progress card` commit. The
**[live fleet dashboard](https://github.com/ptitty12/mvp2005-decomp/tree/dashboard)**
shows matching speed, each worker's activity and the merge backlog.

## The decompilation (knowledge track)

### Toolchain

Found in recon (`docs/RECON_REPORT.md`):

- **Game code and EA libraries:** SN Systems ProDG 3.9.3 (`GNU C++ 2.95.3 SN
  BUILD v1.76`). Game code uses `-Os -G0 -fno-rtti -fno-exceptions
  -ffloat-store`; the EA libraries mostly use `-O2`. Flags are pinned per
  library, and sometimes per unit, with evidence in `docs/DECISIONS.md`.
- **Dolphin SDK:** Metrowerks CodeWarrior GC/1.2.5n (GC/2.0 for VM and the
  debugger driver), with per-file flags such as `-opt nopeephole`.
- **Link:** SN `ngcld` 1.5.1.692 with a static linker script
  (`config/GV4E69/ldscript.ld`). `tools/mkdol.py` packs the DOL.

Built on dtk-template, decomp-toolkit and objdiff, patched for ProDG
(`tools/project.py`). The game has no symbols or map.

### Building

Put a disc image (`.iso`, `.ciso`, `.rvz`, ...) or the extracted `mvp.elf` in
`orig/GV4E69/` (gitignored), then:

```sh
python configure.py
ninja
```

`ninja` rebuilds `build/GV4E69/main.dol` from compiled source plus the
remaining split assembly, checks it against `config/GV4E69/build.sha1`, and
writes the objdiff report `build/GV4E69/report.json`. The host is native
Windows: ProDG and MWCC are Win32 programs. The engine has no build yet (E0).

### What counts as matched

The verifier decides, not any agent:

- `tools/verify/verify.py` checks four things:
  - protected-file integrity;
  - the retail SHA-1;
  - the hard rules (no inline asm, naked functions or `.incbin`; only approved pragmas);
  - no score drops against a baseline.
- `tools/match/check_literals.py` catches a "100%" function that loads the
  wrong float, string, global or call target. `tools/team/integrate.py` refuses
  commits that add one.
- CI runs the integrity check on GitHub. The full build runs only on the
  owner's machine, because game files never leave it.

## Who does the work

AI agents write every line of code and docs. The owner dumped the disc, sets
the goals, approves the decisions that are theirs (`docs/DECISIONS.md`), and
never runs commands.

- **Claude Code:**
  - an Opus 5.5 **lead** plans batches from the knowledge targets, owns shared
    headers and config, and merges and verifies;
  - Sonnet 5.5 **workers** match and name functions in their own worktrees;
  - **side sessions** build the engine and the tools;
  - Fable 5.1 takes hard functions "when needed".
- **Codex** agents work under the same rules (`AGENTS.md`).
- **Coordination:** `tools/coord.py` keeps a shared roster, unit claims, an
  inbox and one integration lock (`docs/COORDINATION.md`).
- **Background jobs** run as scheduled tasks on the owner's PC: the progress
  bot, the dashboard and the permuter farm.

## Rules, in short

The full rules are in [`CLAUDE.md`](CLAUDE.md). The most important:

- Never commit or publish game data, or anything derived from the disc (the owner's
  private deployment, reached only through its private link, may serve the owner's own disc image):
  - assets;
  - translated game code;
  - memory dumps, screenshots and recordings.
- Mods depend only on the three contracts above, never on decomp source.
- Decomp: no inline asm in any form, and no tricks that force a match. One unit
  per commit, plus name commits and contract commits. The verifier decides.
- Engine: determinism is an invariant. Every component has tests that need no
  game data.
- No GPL code copied in (Dolphin is a reference, not a source).
- Agents never change the verifier, CI or `.claude/`.

## Repository layout

| Path | Contents |
|---|---|
| `engine/` | The browser engine: translator, runtime, renderer, audio, virtual disc, mods, netplay, web shell, VPS services. Starts at E0 |
| `src/` | Decompiled C/C++, one file per split unit (`game/tu_<addr>.cpp` until the original file name is known) |
| `include/` | Shared headers. Struct comments record their evidence |
| `config/GV4E69/` | dtk config (`splits.txt`, `symbols.txt`), linker script, DOL layout, retail hash, EA reference names and datafile schemas |
| `configure.py`, `tools/project.py` | Build generator (dtk-template with the ProDG pipeline) |
| `tools/goals/` | `targets.py`: the knowledge targets and their progress |
| `tools/verify/` | The verifier and its protected-file hashes. Owner-only |
| `tools/match/` | Matching helpers ([README](tools/match/README.md)) |
| `tools/scout/`, `tools/swarm/`, `tools/team/` | Unit packets and renaming, merge drivers, integration and the lead's batch scripts |
| `tools/recon/` | Disc and binary analysis ([README](tools/recon/README.md)) |
| `tools/*.py` | `coord.py`, `progress_bot.py`, `progress_chart.py`, `fleet_dashboard.py`, `mkdol.py`, dtk-template helpers |
| `docs/` | Roadmap, architecture, decisions, patterns, logs and research |
| `prompts/` | Agent briefs |
| `orig/`, `build/` | Disc files and build output. Gitignored, never committed |

## Docs

| File | What's in it |
|---|---|
| [`docs/ROADMAP.md`](docs/ROADMAP.md) | The goal, the strategy, milestones with exit criteria, next steps |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Engine design: translator, runtime, renderer, audio, virtual disc, mods, netplay, decisions |
| [`CLAUDE.md`](CLAUDE.md) | The rules every agent follows |
| [`AGENTS.md`](AGENTS.md) | Entry point for Codex and other non-Claude agents |
| [`PROGRESS.md`](PROGRESS.md) | Snapshot and session log, newest last |
| [`HANDOFF.md`](HANDOFF.md) | The lead's hand-off: state, how to resume, queues, scripts |
| [`docs/DECISIONS.md`](docs/DECISIONS.md) | Decisions (toolchain, flags, splits, policy, direction) and open questions for the owner |
| [`docs/COORDINATION.md`](docs/COORDINATION.md) | The Claude/Codex coordination protocol (`tools/coord.py`) |
| [`docs/GAME_DATA.md`](docs/GAME_DATA.md) | EA datafile, database tables, schedules and archive formats |
| [`docs/PATTERNS.md`](docs/PATTERNS.md) | Source patterns that produced byte-identical matches with ProDG/g++ 2.95 |
| [`docs/NEARMISS.md`](docs/NEARMISS.md) | Parked near-misses and what was tried |
| [`docs/SPEEDUPS.md`](docs/SPEEDUPS.md) | Speed-up ideas, observations and what was tried |
| [`docs/REUSE.md`](docs/REUSE.md), [`docs/REFERENCE_SOURCES.md`](docs/REFERENCE_SOURCES.md) | Public code we can use, and every outside source checked, with verdicts |
| [`docs/RECON_REPORT.md`](docs/RECON_REPORT.md), [`docs/RECON_CHECKLIST.md`](docs/RECON_CHECKLIST.md) | Recon report: disc, sections, toolchain evidence, subsystems (2026-10-10 snapshot) |
| [`docs/UNITS.md`](docs/UNITS.md) | First-pass unit anchors (2026-10-10 snapshot; the live split is `config/GV4E69/splits.txt`) |
| [`docs/SWARM_SESSION_2026-10-13.md`](docs/SWARM_SESSION_2026-10-13.md), [`docs/audits/`](docs/audits/), [`docs/notes/`](docs/notes/) | Session reports, audits and per-function notes |
| [`docs/dtk/`](docs/dtk/) | decomp-toolkit / dtk-template reference docs |

## Credits and references

- [decomp-toolkit](https://github.com/encounter/decomp-toolkit),
  [dtk-template](https://github.com/encounter/dtk-template) and
  [objdiff](https://github.com/encounter/objdiff) by encounter. The template's
  license is in `docs/dtk/LICENSE-dtk-template`.
- [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) and
  [m2c](https://github.com/matt-kempster/m2c), used from external clones.
- [mitsevox/mvp2005](https://github.com/mitsevox/mvp2005) (CC0): ported
  Dolphin SDK, libc, libm, libsn and SND sources, and EA reference names.
- [adonis-singh/re4](https://github.com/adonis-singh/re4): Dolphin SDK
  bodies (dolsdk2004) and ProDG compiler notes.
- [dbalatoni13/nfsmw](https://github.com/dbalatoni13/nfsmw): prior art for
  ProDG matching on an EA GameCube title.

MVP Baseball 2005 is a trademark of its owners. This project is not
affiliated with EA, Nintendo or MLB. It contains no game data and hosts
nothing derived from it: every player supplies their own disc.
