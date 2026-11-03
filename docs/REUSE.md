# Reuse map: public code we can port instead of writing

> Every reference source checked since, with its verdict: `docs/REFERENCE_SOURCES.md` (2026-10-13).
>
> What was used (as of 2026-10-13): mitsevox/mvp2005 sources were ported unit by unit (Dolphin SDK,
> libc, libm, libsn, SND; commits say "ported from mitsevox/mvp2005"), its EA reference names became
> `config/GV4E69/refnames/`, and Dolphin SDK bodies came from adonis-singh/re4. Unit counts below
> (126 units) are the 2026-10-10 provisional split; there are 847 units now.

Researched 2026-10-10 for `docs/SPEEDUPS.md` item 6. **Verified** facts were
checked directly (gh api, the repo's README). Everything else was **reported**
by a research agent and must be checked before anyone relies on it. CLAUDE.md
still applies to anything ported: no `asm` bodies, our verifier decides, and
the source goes in DECISIONS.md.

> **Correction:** `docs/RECON_REPORT.md` §5 says no public decompilation of MVP
> exists. One does: see below.

## 1. A public decomp of this exact game: mitsevox/mvp2005

**Verified:**
- https://github.com/mitsevox/mvp2005: public, **CC0-1.0**, created
  2026-09-29, last push 2026-10-01.
- Targets `GV4E69` with the same retail `main.dol` SHA-1, `da6becdb…84dd`,
  and links with SN `ngcld` like we do. It has 262 C/C++ files under `src/`.
- Its README claims **1,127 / 24,150 functions (4.67%)** and **253,740 /
  5,896,192 code bytes (4.30%)**. 227 units are configured, 226 marked
  matching, and the build reproduces `main.dol` exactly.
- `orig/` holds only a placeholder; there's no game data in it.

**Reported (unverified):**
- What the matching units cover:

  | Area | Units | Location |
  |---|---|---|
  | Dolphin SDK | 96 | |
  | libc | 65 | |
  | libm | 33 | |
  | libgcc | 12 | |
  | libsn | 3 | |
  | EA SND | 16 | `src/snd/cmn/*` |
  | geomlib | 1 | `geomgroup.cpp` |
  | EAGL | 1 | `eagl/viewport.cpp` (2 functions) |

- It links SDK pieces our unit list lacks: vm, vmbase, DebuggerDriver and
  amcstubs.
- `config/GV4E69/filemap.tsv` gives TU boundaries and object names, built from
  FIFA05/UEFA SN link maps. It also has `symbols.txt`, `linked_names.tsv`,
  `name_sources.tsv` and `refnames/*.tsv`. The `refnames` tables were derived
  from other EA games' debug files (see its `docs/reference-builds/`).
- Its measured flags:

  | Code | Flags |
  |---|---|
  | game | `-O2 -G0 -ffloat-store -fno-strength-reduce` |
  | geomlib | `-Os -G0 -ffloat-store` |
  | EAGL | `-O2 -G0` |
  | SDK | `-char unsigned` throughout; signed char for dvd, mtx, CARDOpen and CARDRename; EXIBios `-O3,p`; `-opt nopeephole` only on `__ppc_eabi_init.c` |

  These differ from our DECISIONS, where game code is `-Os` and OS has
  `nopeephole` (resting on one function). Re-check per library.

**How to use it is the owner's decision** (open question):
- port its matched units into this repo,
- move our work onto a private copy of it,
- or work independently.

## 2. EA shared libraries (55 of our 126 units)

All reported. Libraries found in the DOL:

| Library | Approx. address range | Evidence |
|---|---|---|
| EAGL (`C:/mvp2004/libraries/GC/eagl`) | 0x803D5000–0x803EB2AC (17 ctor units) | Deprecation messages say "removed in EAGL 4.09.00", so ours is older than 4.09 |
| gamelib / animlib / corelib | 0x80364000–0x8038C000 (26+ units) | No version string |
| AV / rcmp, VP6, MAD codecs | 0x8038BC58–0x803A4094 | |
| EA SND V9.02.04 (Dec 23 2004) | 0x803A424C–0x803B9794 | Version string |
| SPCH V3.18.01 (Oct 13 2004); csis | | Version strings |
| RealCore (realmem, realfile, realsystem, realmath, realfont, realshape, realmemcard) | 0x803EB2AC–0x80402CAC | |
| SN libsn | | v62 |

Public sources:

| Source | Licence | Covers | Confidence |
|---|---|---|---|
| mitsevox/mvp2005 | CC0 | SND `src/snd/cmn/*` (16 units, matched), `eagl/viewport.cpp` | Very high (same DOL) |
| dbalatoni13/nfsmw | CC0, ProDG 3.9.3 | `src/Speed/Indep/Libs/snd/9/` (about 29 units matching for a later SND build); `src/Speed/Indep/Src/EAGL4Anim/` (16 of 46 files have content, not marked matching). realcore, csis and LibSN are empty stubs there. | High for SND, medium for EAGL anim |
| lifewillbeokay/moh-rising-sun | CC0, ProDG 3.8.1 | `src/eagl/` (state, tar, transform, tevstage) and `src/eagl_anim/`, older EAGL | Low to medium |

No public source was found for gamelib/animlib, the EAGL core render path,
VP6/MAD, SPCH, or the RealCore C bodies. cobanov/nfsu2-decomp is an empty
scaffold.

## 3. Dolphin SDK (13 units)

All reported. Build strings, all `(0x2301)`:

| Library | Build string |
|---|---|
| AI | Apr 5 2004 04:15:02 |
| AR | Apr 5 2004 04:15:03 |
| ARQ | Apr 5 2004 04:15:04 |
| AX | Apr 5 2004 04:15:05 |
| CARD | Apr 5 2004 04:15:35 |
| DSP | Apr 5 2004 04:15:32 |
| DVD | Apr 5 2004 04:14:51 |
| EXI | Apr 5 2004 04:14:14 |
| GX | Apr 5 2004 04:14:28 |
| PAD | Apr 5 2004 04:14:49 |
| SI | Apr 5 2004 04:14:16 |
| VI | Apr 7 2004 04:13:59 |
| OS | May 21 2004 09:28:09 (Patch 1) |

Best public matches:
- **mitsevox/mvp2005:** all 13, already matching on this exact DOL.
- **emoose/re4:** every date matches, 100% matched. The tools are CC0; the
  source is marked as the original owners' IP.
- **PrimeDecomp/echoes:** CC0. GX and OS have our exact dates; no AX.
- **doldecomp/dolsdk2004:** no licence. GX and VI are still in progress.

## 4. libc, libm, libgcc and libsn

All reported:
- mvp2005 has these adapted to MVP's libsn v62 (`src/libc`, `src/libm`,
  `src/libgcc`, `src/libsn`).
- emoose/re4 has newlib 1.8.2 and fdlibm, built with SN v1.79, but they match
  under v1.76.
- crt0, ppcdown, proview and tealeaf were original asm and stay as split asm,
  per DECISIONS.

## 5. Recommendations

1. **Port mvp2005's matched units:** runtime, SDK, SND and geomlib, about
   1,100 functions. Re-verify each with `tools/verify/verify.py`, and strip
   or exclude anything with `asm`. This is the fastest single gain available:
   about 16 of our unit issues almost outright, with the 4.7% that's already
   public.
2. **Use mvp2005's filemap for TU boundaries and EA object names**, recording
   the source as evidence. Whether names that came from other games' debug
   files count as acceptable evidence is the owner's call.
3. **Use nfsmw's EAGL4Anim and moh-rising-sun's EAGL as starting points** for
   the matching EAGL pieces. Expect version differences.
4. **Expect no outside help for gamelib (about 29 units) or game code.** That's
   where `tools/scout/` and the matching effort go.
5. **Check mvp2005's progress regularly** while the owner decides: it's active
   and ahead of us on infrastructure.

## 6. Vendored into `engine/`

Code copied into the engine tree. Only MIT, BSD, zlib, Apache-2.0 or CC0 code may
appear here (CLAUDE.md). Each entry keeps its license file next to the code and is
pinned to the exact upstream commit it was fetched from.

| Path | Upstream | License | Pinned commit | Used for |
|---|---|---|---|---|
| `engine/third_party/doctest/doctest.h`, `LICENSE.txt` | [doctest/doctest](https://github.com/doctest/doctest) v2.5.3 | MIT | `2d0a9359a60c51affe2a9bebb1be1dca47868151` | Unit-test framework for every data-free engine test |
| `engine/third_party/pureikyubu/` (`gekko.cpp`, `gekko.h`, `gekkoc.cpp`, `gekkoc.h`, `gekkodec.cpp`, `gekkodec.h`, `gekkodisasm.cpp`, `gekkodisasm.h`, `gqr.h` from `src/`; `pch.h`, `stubs.cpp` from `testing/gekko_bench/`; `LICENSE`) | [emu-russia/pureikyubu](https://github.com/emu-russia/pureikyubu) | CC0-1.0 | `0c29dca5cfa13d9126a137b9ebf95f13821413cb` (the last commit where the benchmark's `pch.h` and the core sources agree) | Test oracle only: `test_ppc_fuzz_pk` runs random integer/branch/CR/XER programs on its interpreter and ours. Not linked into the engine. Changes: `pch.h` includes our `pk_jit_stub.h` when the recompiler is not built; `pk_jit_stub.h`, `pk_wasm_shim.h` (no x86 cycle counter in the WebAssembly build, E2) and `CMakeLists.txt` are ours |
| `engine/third_party/zstd/` (`lib/common/`, `lib/compress/`, `lib/decompress/`, `lib/zstd.h`, `lib/zstd_errors.h`, `LICENSE`) | [facebook/zstd](https://github.com/facebook/zstd) v1.5.5 | BSD-3-Clause | `63779c798237346c2b245c546c40b72a5a5913fe` (the v1.5.5 release) | The RVZ disc container stores its chunks and its tables as zstd streams (`vfs/rvz.cpp`, `vfs/rvz_pack.cpp`). Built from source for the host and for WebAssembly; the x86-64 assembly decompressor (`huf_decompress_amd64.S`) is not included, and `ZSTD_MULTITHREAD` is off |

### Read, not copied: the RVZ container (`engine/vfs/`)

The RVZ disc container's compressor is vendored (the `third_party/zstd` row in §6);
its layout was implemented from documentation. No code was copied:

| Source | License | Read for |
|---|---|---|
| Dolphin's `docs/WiaAndRvz.md` | documentation only (the Dolphin project is GPL-2.0+; only its format document was read, and no code or constant was copied from `WIABlob.cpp`) | The container layout: `wia_file_head_t`/`wia_disc_t` fields, `rvz_group_t`, the `RVZ\x1` magic, the compression enum, the version rules, the `data_offset % 0x8000` rounding, and the RVZ packing PRNG |

`tests/test_rvz.cpp` builds its images in memory from a synthetic pattern, so no RVZ
file, and nothing derived from a disc, is committed. Dolphin itself could not be run
to cross-check a file: no Dolphin build is available here and the independent Rust
`rvz` crate reads Wii images only.

### Read, not copied: the renderer (`engine/gfx/`, E1)

Nothing is vendored for the renderer; Dawn and SDL3 are external SDKs (`docs/engine/TOOLCHAIN.md`).
These were read for hardware facts and design, and no code was copied:

| Source | License | Read for |
|---|---|---|
| [encounter/aurora](https://github.com/encounter/aurora) at `51797212b9283e91cc03c273854d63b2421d8490` | MIT | Design of the TEV-to-WGSL generator, pipeline cache and register decoding (`lib/gx/shader.cpp`, `regs.cpp`, `gx.cpp`: BP cull encoding and front face); its texture converter was judged too tied to its own buffers to vendor |
| Dolphin (VideoCommon) | GPL-2.0+ (read only) | Hardware behaviour: CMPR's 5/8-3/8 interpolation, indirect texturing (formats, bias, matrix scale, wraps), the TEV fixed-point lerp and compare modes, fog and lighting formulas, EFB copy register fields |
| YAGCD and the Dolphin SDK's GX macros | public documentation | Register layouts and command lengths |

### Compiled in, not vendored: the browser build (`engine/web/`, E2)

Nothing new is copied into the tree. The browser engine (`mvp-web.js`/`.wasm`)
compiles in third-party code from SDKs outside the repository
(`docs/engine/TOOLCHAIN.md`); the build copies their notices into the served
site's `licenses/` directory:

| Code | License | In the served engine as |
|---|---|---|
| emdawnwebgpu, Dawn release `v20261013.193955` (`webgpu.cpp`, `webgpu_cpp.h`, `library_webgpu*.js`) | BSD-3-Clause (Dawn & Tint authors); its JS bindings under Emscripten's MIT/NCSA | `webgpu.h` for the renderer (`licenses/dawn-emdawnwebgpu.txt`) |
| Emscripten 6.0.12 runtime and system libraries (JS glue, libc, libc++, libc++abi) | MIT/NCSA; libc++ Apache-2.0 with LLVM exception; musl MIT | everything (`licenses/emscripten.txt`) |

Dev tools only, never served: TypeScript 6.0.3 (Apache-2.0) compiles the shell;
Playwright 1.64.0 (Apache-2.0) drives the local browser check.

### Read, not copied: audio (`engine/audio/`, E3b)

Nothing is vendored for audio; SDL3 is the external SDK `engine/gfx/` already uses
(`docs/engine/TOOLCHAIN.md`). These were read for behaviour, and no code or data was copied:

| Source | License | Read for |
|---|---|---|
| Dolphin (DSPHLE `UCodes/AX.h`, `AX.cpp`, `AXStructs.h`, `AXVoice.h`, master as of 2026-10-14) | GPL-2.0+ (read only) | AX microcode behaviour: command numbers and arguments, the 2004 microcode's mixerCtrl bits, per-millisecond voice processing (SRC, envelope, LPF, mix with ramps), aux send/return and the Dolby Pro Logic II commands 0x13/0x10, output clamping. The DSP ROM's polyphase coefficients were not used: `audio/mixer.cpp` designs its own |
| The game's own SDK and EA SND code (`config/GV4E69` symbols, dtk assembly of `dolphin/ax` and `snd/`) | the retail binary (read, as for the decomp) | The command list the CPU builds (fn_804181DC), the PB fields it reads back (fn_80419568), the studio/depop ramp (fn_804190A4), the aux buffer rotation (fn_80417F50), EA's mixerCtrl and ramp use (SNDDRV_DSPMixerUpdate) and its aux A callback (SNDDRV_audiocallback) |

### Netplay and the VPS server (`engine/net/`, `engine/server/`, E5)

Nothing is vendored into the repository. The server has one runtime dependency,
installed by npm from the pinned `engine/server/package-lock.json` and copied into
the deploy package (`engine/server/tools/package.mjs`) with its own license file:

| Code | License | Used as |
|---|---|---|
| [ws](https://github.com/websockets/ws) 8.22.0 | MIT | The WebSocket server for the lobby and signaling (`engine/server/src/server.ts`) |
| TypeScript 6.0.3, `@types/node` 24.19.2, `@types/ws` 8.18.2 | Apache-2.0 / MIT | Development only (compiling and type-checking the server) |

| Source | License | Read for |
|---|---|---|
| xxHash's XXH64 description ([Cyan4973/xxHash](https://github.com/Cyan4973/xxHash), `doc/xxhash_spec.md`) | BSD-2-Clause | `engine/net/state_hash.cpp` implements XXH64 from the published algorithm (no code copied); its reference vectors are in `test_net_protocol` |
| coturn's documentation ("TURN REST API", `use-auth-secret`) | BSD-3-Clause | The credential format `engine/server/src/turn.ts` derives and the `deploy/turnserver.conf.template` settings |

The Docker deployment (`deploy/`) vendors nothing either; it builds on pinned
upstream images and downloads (by digest or SHA-256, in `deploy/Dockerfile` and
`deploy/coturn/Dockerfile`):

| Piece | License | Used as |
|---|---|---|
| `emscripten/emsdk:6.0.12` image | MIT/NCSA (Emscripten), others as shipped | Build stage only (not in the runtime image) |
| `node:24.22.0-trixie-slim` image | MIT (Node.js), Debian packages | Runtime and server build |
| Ninja 1.13.1 release binary | Apache-2.0 | Build stage only |
| `coturn/coturn:4.18.0` image | BSD-3-Clause | The STUN/TURN service, unmodified (an entrypoint script is added) |
