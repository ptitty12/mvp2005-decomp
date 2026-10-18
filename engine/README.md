# engine/

The browser engine: it runs the retail MVP Baseball 2005 code, translated from
PowerPC to WebAssembly on the player's machine, with a runtime that replaces the
GameCube hardware and the Nintendo SDK. It also holds netplay, roster/team packs
and the VPS services.

- Design: [`../docs/ARCHITECTURE.md`](../docs/ARCHITECTURE.md). The planned layout is §11.
- Milestones and next steps: [`../docs/ROADMAP.md`](../docs/ROADMAP.md) (E0-E7).
- Rules: [`../CLAUDE.md`](../CLAUDE.md), "Rules for the engine track" and the hard
  rules for everyone.

**Status (2026-10-14):** E0b (native boot spike) done; E0c (unwinds, controller
input, AX voices) done; E2 groundwork done (the engine builds for WebAssembly,
and the WebAssembly emitter boots the DOL with the native SDK-log hash); E2 done
(the player picks their disc in Chrome, Edge or Firefox, the page translates it
and the title screen renders through WebGPU: `web/`); E5 netplay in progress
(lockstep in `net/`, the VPS server in `server/`, two browsers in lockstep through
the lobby: [`../docs/engine/NETPLAY.md`](../docs/engine/NETPLAY.md),
[`../docs/engine/E5_LOG.md`](../docs/engine/E5_LOG.md)). Build instructions and
measurements:
[`../docs/engine/E0B_LOG.md`](../docs/engine/E0B_LOG.md),
[`../docs/engine/E0C_LOG.md`](../docs/engine/E0C_LOG.md),
[`../docs/engine/E2_LOG.md`](../docs/engine/E2_LOG.md),
[`../docs/engine/E2_SPIKE.md`](../docs/engine/E2_SPIKE.md) and
[`../docs/engine/WEB.md`](../docs/engine/WEB.md) (the browser build).

Three rules matter most here:

- **Nothing derived from the disc is ever committed or published.** (The owner's
  private deployment, reached only through its private link, may serve the owner's own disc image.) That covers
  translated code, extracted files and memory dumps. The player's browser
  produces them from the player's own disc; development builds produce them
  under `build/`.
- **Determinism:** host nondeterminism reaches the game only through the input
  queue and the virtual clock.
- **Every component has tests that need no game data.**
