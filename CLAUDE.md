# CLAUDE.md — MVP Baseball 2005 (GameCube, GV4E69): decomp + browser engine

You are working on a personal project built on MVP Baseball 2005 for GameCube
(`GV4E69`, USA rev 0), using a disc image the owner dumped from their own copy.
The GitHub repository, `ptitty12/mvp2005-decomp`, is public (owner, 2026-10-13), so
everything pushed is published: source, tooling and docs only, never game data.
Re-read this file whenever a prompt tells you to, and whenever you are unsure.

## What we are building (owner, 2026-10-14)

In priority order:
1. **The game runs in a web browser.**
2. **Online multiplayer in the browser.** The owner's VPS runs matchmaking;
   each player's game runs locally in their own browser.
3. **New rosters and teams are easy to add.**

Above all: a foundation without tech debt. Mods must never need rework because
the decomp moved on. A 100% matching decomp is welcome, but it is not the goal.

How we get there: [`docs/ROADMAP.md`](docs/ROADMAP.md) has the milestones and next
steps. [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) has the engine design. In short:

- **Engine track (`engine/`).** The browser runs the *retail* game code, translated
  from PowerPC to WebAssembly on the player's machine. A runtime replaces the
  console hardware and the Nintendo SDK. Milestones E0-E7.
- **Knowledge track (`src/`, `include/`, `config/`).** The matching decomp, aimed at
  what the engine and the mods need: SDK names, determinism sources, roster/team
  data, the main loop. Milestones K1-K5. `python tools/goals/targets.py` measures them.
- **Mods depend only on three stable contracts:** the retail binary's code and
  addresses, the game's own data formats, and the engine's versioned APIs. Never
  on decomp source.

## Choosing work

1. Work that moves the current milestone in `docs/ROADMAP.md` §5 ("Next steps") comes first.
2. Knowledge work is chosen by target: `targets.py --list K<n> --todo` and
   `--units K<n>`. A target function is done when it is matched *and* has an
   evidence-backed name and types (for K1: the name).
3. Paused: the near-miss tail and big-function passes outside the K targets.
   General matching runs only on token budget the K and E work doesn't need. The
   weekly usage cap is the binding limit.

## Source of truth, in priority order

1. The original binary. For decompiled code: the verifier's PASS/FAIL. For
   engine code: its tests, and on the owner's machine the retail game's behavior.
2. `docs/DECISIONS.md` (owner-approved decisions, including those the owner
   delegated to the lead).
3. This file, then `docs/ROADMAP.md` and `docs/ARCHITECTURE.md`.
4. Ghidra/REA/m2c pseudocode: a HINT, often wrong about types and signatures.
5. Comments in the repo, including your own previous comments. Comments are
   claims, not evidence. Never accept a deviation because a comment justifies it.

## Hard rules for everyone (violating any of these means the work is rejected)

- NEVER commit or publish game data, or anything derived from the disc:
  - disc images, DOL/REL files;
  - extracted, decompressed or converted assets: textures, audio, models, overlay binaries;
  - translated or recompiled game code;
  - byte dumps, memory dumps, save states, and screenshots or recordings of the game.

  `orig/` and `build/` stay gitignored, and the public repo never contains any of it.
  **Private deployment (owner, 2026-10-14):** the owner's own access-controlled
  deployment may serve the owner's disc image to the owner and invited players who
  own the game (behind a password, never a public URL). Translated code is still
  produced in each player's browser, and nothing game-derived is served anywhere else.
- NEVER circumvent copy protection, download game files, or fetch ROMs/ISOs.
- NEVER modify, skip, weaken, or special-case the verification tooling, its
  config, the CI workflow, or `.claude/settings.json`. If you believe the
  verifier is wrong, STOP and write the case in `docs/DECISIONS.md` under
  "Open questions for owner".
- Mods and mod formats depend only on the three contracts above. Never ship a
  mod, pack or tool that needs decomp source, placeholder names or struct layouts
  from `include/` to work.
- Never copy GPL code (Dolphin, among others) into the repo. Reading it is fine.
  Vendored code must be MIT, BSD, zlib, Apache-2.0 or CC0, recorded in `docs/REUSE.md`.
- Do not invent names with false confidence. Use `fn_80XXXXXX` / `lbl_80XXXXXX`
  until evidence (strings, call patterns, SDK symbols, body matches against
  public SDK source) supports a real name, and record the evidence in the
  commit message.

## Rules for the knowledge track (`src/`, `include/`, `config/`)

The decomp proves its understanding by byte-identical code. These rules keep that
proof honest:

- A function is DONE (matched) only when the verifier says it matches.
  Readable-but-unmatched code is NOT matched, no matter how plausible it looks.
- NEVER use inline assembly, `asm` blocks, naked functions, `__declspec(naked)`,
  raw byte arrays standing in for code, object patching, or linker tricks to
  force a match.
- NEVER edit generated assembly or split output by hand. Change the split
  config and regenerate instead.
- NEVER mark a unit or function complete without a passing verifier run from
  the current commit.
- One translation unit (or one function) per commit:
  `match(<unit>): <function> [N/M funcs]`. Two exceptions, both verified like any
  other commit:
  - **Name commits**: `name(<unit>): <old> -> <new> (<evidence>)`. They rename
    symbols in `symbols.txt` and the source, with evidence.
  - **Contract commits** (2026-10-14): `contract(<symbol>): <what> [<units>]`. When
    one call contract (a signature, return type or shared struct) is wrong, fix the
    shared header and every unit that uses it in one commit. Never use one to bundle
    unrelated matches.
- Mirror the original structure, bugs included. The decomp describes retail code;
  fixes and improvements belong in mods.
- Struct layouts are shared truth. Changing a struct used by matched functions
  means rebuilding and re-verifying everything that uses it; if any previously
  matched function breaks, revert. Record each struct's evidence (offsets
  observed, functions that use it) in a comment above the definition.
- SDK code (Dolphin OS, GX, MSL, etc.) is matched against public decomp work
  where the version matches. Note the source in DECISIONS.md.

### The knowledge loop (one function at a time)

1. Pick the next function from a K target (`tools/goals/targets.py`) or your batch.
2. Gather evidence: assembly (split output), `twins.py`, m2c/Ghidra drafts,
   callers/callees, strings, struct usage elsewhere.
3. Write C/C++ for that ONE function. Build and diff (`tools/match/diff_unit.py`).
4. Iterate until it matches. Typical levers: types, signedness, statement order,
   temporaries, loop shape, inlining, `const`, struct layout.
5. If stuck after ~8 genuine attempts: park it (original asm stays linked), log
   what you tried, and move on.
6. Name what you understood, with evidence. Commit, then update `PROGRESS.md`.

## Rules for the engine track (`engine/`)

- `engine/` holds our own code: translator, runtime, renderer, audio, virtual disc,
  mods, netplay, web shell and VPS services (layout in `docs/ARCHITECTURE.md` §11).
- **Determinism is an invariant.** Host nondeterminism (wall time, audio device
  timing, network timing, file-load latency, host threads) may reach guest state
  only through the input queue and the virtual clock. Code that breaks this is a bug.
- **The engine reads nothing from the decomp side** (owner, 2026-10-27). Nothing in
  `engine/` or `deploy/` reads `config/`, `src/`, `include/` or the decomp's tools.
  The decomp's metadata comes only from the pinned copy in `engine/decomp_pin/`,
  changed only with `engine/tools/sync_decomp_pin.py` (its README says when and how).
- **Every component has tests that need no game data**, and they run in CI.
  Tests that need the disc run only where it exists, skip cleanly elsewhere, and
  never write their outputs into the repo.
- Engine APIs (mod-pack format, hook API, netplay protocol) are versioned. A
  breaking change needs a new major version and a migration note in DECISIONS.
- Decisions D1-D8 are in `docs/ARCHITECTURE.md` §9. Record each new one in
  DECISIONS.md when it settles.
- Commit format: `engine(<component>): <summary>`. One component per commit where
  practical.

## Context hygiene

- Work one translation unit (knowledge) or one component (engine) per session where possible.
- Don't paste whole disassembly listings into notes; reference addresses.
- When a function is matched, you no longer need its asm or pseudocode in
  context. Prefer compacting early over carrying stale detail.

## Reporting

At the end of every session, append to `PROGRESS.md`: date, track and
unit/component, what was done (functions matched or named, milestones moved),
blockers, and anything the owner must decide. Be honest: report failures and
unmatched functions plainly.

## The owner never runs commands (owner, 2026-10-11)

- Never hand the owner a shell command, script or manual procedure to run.
  Agents run every command themselves.
- If an action is physically blocked (the permission rules in
  `.claude/settings.json` deny agents edits to `tools/verify/**`,
  `.github/workflows/**` and `.claude/**`), do not work around the block.
  Record it once under "Open questions for owner" in `docs/DECISIONS.md`
  and keep working on everything else.
- "Stop and ask" below means: record the question in `docs/DECISIONS.md`,
  mention it once in your report, and continue with other work.

## Working alongside Codex and other agents (owner, 2026-10-13)

Claude and Codex agents work on this repo at the same time and coordinate
through `tools/coord.py`. Follow `docs/COORDINATION.md`:
- register at the start of a session (`claude-lead`, `claude-w<N>`,
  `claude-side-<topic>`), run `who`, and read your `inbox`;
- claim units before editing them, and release them when they're merged;
- push to main only inside `python tools/coord.py run-locked <you> --purpose
  "..." -- <gate and push>`, docs-only and engine pushes included. The gate is
  `tools/team/integrate.py` (`--allow engine/ --allow docs/` for engine and doc commits);
- message Codex agents with `send <you> codex "..."` or by name.
`claude-lead` settles conflicts over units, shared files and integration order.

## When to stop and ask

- The verifier disagrees with something you're confident about.
- A rule above seems to block necessary progress.
- You'd need to change build flags, compiler version, or split config for many
  units at once.
- A design decision would change one of the three mod contracts or a versioned
  engine API.
- Anything involving copy protection, distribution, or publishing (hosting
  included).
