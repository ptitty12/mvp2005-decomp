# DECISIONS (owner-approved only)

Agents may propose entries under "Open questions"; only the owner moves them to "Decided".

## Decided
<!-- YYYY-MM-DD | decision | why | evidence -->

- 2026-10-10 | **Target is MVP Baseball 2005, GameCube `GV4E69` (USA, rev 0).** Owner said in chat: "yes we are using 2005, you can edit all of that yourself"; the agent updated the game name in CLAUDE.md, README.md and prompts/00_KICKOFF.md. | The owner's disc is 2005. | `docs/RECON_REPORT.md` §1
- 2026-10-10 | **Path: `prodg-hybrid`.** Game code and EA libraries are matched with SN ProDG. The Dolphin SDK is matched with Metrowerks CodeWarrior. | Toolchain evidence | `docs/RECON_REPORT.md` §2, §7
- 2026-10-10 | **Host: native Windows.** | ProDG `ngccc`/`ngcld` and MWCC are Win32 executables and run natively. | `docs/RECON_REPORT.md` §6
- 2026-10-10 | **Phase 1 tool downloads approved** (no game data): dtk-template, decomp-toolkit, objdiff, binutils, community compilers package, ninja. | Needed for the scaffold | owner reply in chat
- 2026-10-10 | **GitHub remote: private repo `ptitty12/decompile_mvp_04`.** The owner logged in to `gh`; the repo is private. | The kit and the dtk docs require private hosting. | owner reply in chat; `gh repo view` shows `PRIVATE`
- 2026-10-10 | **Blanket approval.** Owner, in chat: "yes i approve all things good job, don't come back to me with action items unless you physically need my help, i trust you for all other things". All open questions below were resolved on that basis. | — | owner reply in chat
- 2026-10-10 | **Phase 1 build pipeline approved as implemented**:
  - split input is an analysis DOL made from `/mvp.elf` (`tools/mkdol.py --per-section`);
  - link with SN `ngcld` 1.5.1.692 and `config/GV4E69/ldscript.ld`;
  - DOL packed by `tools/mkdol.py --layout config/GV4E69/dol_layout.txt`;
  - overlay input sections renamed `.feoverlay`/`.dboverlay`;
  - `LONG(0)` terminators for `.ctors`/`.dtors`;
  - documented `add_relocations`;
  - marked ProDG patch in `tools/project.py`.

  | Upstream dtk-template can't reproduce this game's DOL. `mkdol --segments`/`--layout` turn retail `/mvp.elf` into retail main.dol byte-for-byte. | commits `f91c043`, `84e39d7`
- 2026-10-10 | **Compilers.** Game code + EA libraries: ProDG 3.9.3 = `GNU C++ 2.95.3 SN BUILD v1.76` (exact banner match with the on-disc EAGL objects). Dolphin SDK: MWCC GC/1.2.5n. Flags are pinned per library as functions match; see the "Flags" entries below. | Banner match; objdiff probes at 100% | `docs/RECON_REPORT.md` §2
- 2026-10-10 | **Small-data bases (SN convention):** `_SDA_BASE_` = .sdata + 0x8000 (r13 = 0x806F69C0); `_SDA2_BASE_` = .sdata + 0x18000 (r2 = 0x807069C0, unusable, so .sdata2 is reached via r13). | retail crt0 @ 0x80003108..0x80003114 | `config/GV4E69/ldscript.ld`
- 2026-10-10 | **Flags: SN libgcc** = ProDG 3.9.3 `-O2 -G0`. | `__do_global_ctors` + `__main` at 100% | `libgcc/_main.c`
- 2026-10-10 | **Flags: game code** = ProDG 3.9.3 `-Os -G0 -fno-rtti -fno-exceptions` (`cflags_game` in configure.py). Evidence:
  - `-Os`: `-O2` gave wrong register choices (r30/r31 swapped) in fn_802709D0; `-Os` matched every function tried.
  - `-G0`: no r13 small-data use anywhere in game .text or the overlays.
  - no RTTI: empty RTTI vtable slots.
  - no EH: no exception tables in the binary.

  EA shared libraries do use r13 small data, so their flags differ (TBD). | 19 functions at 100% in `animation/choosers/playeridchooser.cpp` | commit `6ea98bd`
- 2026-10-10 | **Flags: game code += `-ffloat-store`.** Float locals live in stack slots with a store/reload after every assignment (e.g. fn_8026D504, fn_802721E0, fn_802726BC), which is what `-ffloat-store` produces; mitsevox/mvp2005 also measured it for game code. Adding it changes none of the 44 chooser functions (43/44 match with and without it). Applies to the whole `game` lib. | 43/44 chooser functions unchanged | this commit
- 2026-10-10 | **Flags: Dolphin SDK OS (first datapoint)** = MWCC GC/1.2.5n, template base flags + `-char unsigned -opt nopeephole`. Other SDK libraries (built Apr 5 rather than May 21) may differ; confirm per library. | `OSRegisterVersion` at 100% | `dolphin/os/OS.c`
- 2026-10-10 | **The three Phase 1 probe functions are committed as the first matches.** | They're public-source functions (GCC `libgcc2.c`; Dolphin SDK). | owner approval
- 2026-10-10 | **Verifier + CI installed** from `proposed/`. CLAUDE.md's "verification tooling" means `tools/verify/**`, `.github/workflows/**` and the files listed in `tools/verify/protected.sha256`. | owner approval of the staged files | `tools/verify/`
- 2026-10-10 | **CI hosting:**
  - The `integrity` job runs on GitHub-hosted runners (no game data).
  - The full-build job runs only on a self-hosted runner, and only when the repository variable `GV4E69_SELF_HOSTED` is `true`. No runner is set up yet: that would install a persistent service on the owner's machine, which the agent didn't do unprompted.
  - Until then, agents run `python tools/verify/verify.py --baseline-ref origin/main` locally before every push.

  | Game files never leave the owner's machine | `.github/workflows/build.yml`
- 2026-10-10 | **Original-assembly functions stay as split assembly.** The CLAUDE.md no-asm rule is kept as written. Functions that were hand-written assembly in the original sources are excluded from matching and listed in `docs/UNITS.md` as "original-asm". Examples: SN crt0, PPCArch `PPCMf*/PPCMt*`, `PPCHalt`, OSContext save/restore, `__save_gpr`/`__restore_gpr`. | Keeps the hard rule intact; the agent chose the conservative option under the blanket approval. | CLAUDE.md hard rules
- 2026-10-10 | **Push to `origin` and create one GitHub issue per provisional unit** (`docs/UNITS.md`). | owner approval | —
- 2026-10-10 | **No human in the loop: agents push straight to `main`.** Owner, in chat: "i never need to code review anything, you can push straight, there should be no human in the loop". This replaces the PR and review steps in `prompts/10_WORKER.md`. The gate is the verifier: `python tools/verify/verify.py --baseline-ref origin/main` must PASS on the exact commit being pushed. Never force-push; rebase on `origin/main` if it moved, then verify again. One unit per commit (CLAUDE.md); close or update unit issues with `gh`. | owner reply in chat | —
- 2026-10-10 | **SDK units that contain original-assembly functions or unreproduced data stay NonMatching (not linked).** Their C functions count as matched when objdiff reports 100%. Example: `dolphin/os/OS.c` (its .sdata/.data must be complete, and its exception-vector functions are original asm). | Linking a partial MWCC unit shifts .sdata (8-byte section alignment). | commit `17f0041`

- 2026-10-10 | **Verifier rename/code-less-unit fix installed by the owner** (commit `87d2cc9`): functions missing by name fall back to (unit, address, size); code-less units are skipped at the unit level. | Two false positives blocked pushing new units. | Tested PASS on both false-positive cases, FAIL on synthetic real drops.
- 2026-10-10 | **The lead holds full decision authority.** Owner in chat: "yeah i have faith in you do whatever you have fill authority" (after approving the Opus-lead + Sonnet-worker plan with "go"). Decisions taken under it are marked *[lead, owner authority]*. It does not cover relaxing the verifier (its rules, allowlist or config): those stay the owner's specific call. Protected files (`tools/verify/**`, `.github/workflows/**`, `.claude/**`) still need the owner's own hands: agents are denied edits there by `.claude/settings.json`. | Owner wants the project to run without owner steps. | chat, 2026-10-10
- 2026-10-11 | **The animation choosers/goals region is one retail TU (a unity build)** *[lead, owner authority]*. Evidence: one static initializer (`fn_802AA7A4`, 0x106C bytes, called from `_GLOBAL_.I` at 0x802ACD88) sets up globals used across all the chooser "files" (type hashes such as 0x8068D57C, 0x8068D580); every chooser and helper vtable, slot 3/4 getter and implicit dtor sits after it (0x802AB810..0x802ACD88) in reverse declaration order, as g++ 2.95 emits them at the end of a TU; assert paths name several files (playeridchooser.cpp, randomchooser.cpp, randomloadedchooser.cpp, catchgoal.cpp, reachgoal.cpp, userpickupballgoal.cpp) inside it; and cCircleChooser's `.rodata` (an 8-aligned double at 0x805DC7D8) only fits if its section did not start at 0x805DC70C. Consequences:
  - Keep one split unit per source file for matching (objdiff compares functions either way). Units that already link byte-identically (cEvent/cHand/cPitch) stay linked.
  - Units with doubles in `.rodata` or with helper-class vtables can't link standalone; they stay NonMatching (code-matched) until the region's unity TU (`#include` of each file, then the tail) is assembled and linked as one object.
  - The `#pragma interface`/`#pragma implementation` plan is not needed for this region; the pragmas stay allowed but unused unless another region needs them.
  | Explains the vtable block and the cCircleChooser alignment finding without tricks. | fn_802AA7A4, docs/UNITS.md anchor 45, worker report on tu_80271F14

- 2026-10-10 | **Owner allowed six pragma lines** (commit `f08b79d`; `tools/verify/pragma_allowlist.txt`). Note: verify.py skips every allowlist line starting with `#` as a comment, so the entries have no effect until the one-line fix in open questions is applied:
  - `#pragma cplusplus on` / `#pragma cplusplus reset` and `#pragma dont_inline on` / `#pragma dont_inline reset`: MWCC switches used by the public Dolphin SDK reconstructions we port (W2's pad/si/vi/dsp; `include/libc/math.h`). They reproduce the SDK's own build settings and generate no code by themselves.
  - `#pragma interface` / `#pragma implementation` (g++): every chooser and chooser-helper vtable, the slot 3/4 getters and the implicit dtors sit in one TU at 0x802AC8xx in reverse declaration order (vtables 0x80661EA0..0x80662538). A ProDG 3.9.3 scratch test reproduces this: with `#pragma interface` in the header, the TU defining the class's virtuals emits no vtable, and a TU with a bare `#pragma implementation` emits the vtables in reverse declaration order. This is the only non-trick way to link the helper-class units (cCircleChooser, cLinearChooser, ...). Planned layout: class declarations in `include/game/choosers.h` (`#pragma interface`), implementation TU `src/animation/choosers/choosers.cpp` (bare `#pragma implementation`, same basename).
  - Why: needed to land W2 (80 SDK functions, builds byte-identical) and to link helper-class units. Options: allow all six / allow only the SDK four / allow only interface+implementation / none (W2 stays unmerged; helper-class units stay code-matched but unlinked).

- 2026-10-11 | **Flags: EA shared libraries = ProDG 3.9.3 `-O2 -G0 -fno-rtti -fno-exceptions`** *[lead, owner authority]* (`cflags_ealib`). Evidence (W4, direct compiles): the 0x320 clone function in 5 units matches only at `-O2` (200/200 insns; `-Os` 90%); the 0x704 clone reaches identical structure only at `-O2` (79% vs 55%, registers only); `-O2` hoists loop invariants and initializes a GXColor the way retail does; `-fno-strength-reduce` makes it worse (retail has strength-reduced induction variables); `-ffloat-store` changes nothing here. Every matched static-init pair in all EA-lib units and W5's ealibs units compile identically either way. | -Os was provisional | W4 report, 2026-10-11

- 2026-10-11 | **Original source paths via `#line`** *[lead, owner authority]*: a unit whose retail strings contain its own `__FILE__` (assert/RNG paths like `C:/mvp2004/source/common/animation/choosers/randomchooser.cpp`) starts with `#line 1 "<that path>"`, so `__FILE__` expands to the original bytes. Standard C preprocessor, not a pragma; changes no code. | Our build path differs from the original build machine's. | playeridchooser/randomchooser/randomloadedchooser: strings now equal retail, matches unchanged

- 2026-10-11 | **Early game TUs (0x800034A0..0x8000F598) use the game flags** *[lead, owner authority]*: -Os (a factory loop only matches at -Os; -O1/-O2 fail), -G0 (without it a global loads via r13), -ffloat-store (float-parameter spills). Evidence table in the tu_80008540 merge (031d2d6). | Confirms cflags_game for real standalone TUs, not only the unity TU. | W12 report
- 2026-10-11 | **Known linking issue for real TUs (not blocking matching):** ProDG g++ emits implicit dtors and vtables into .gnu.linkonce.* sections; retail keeps those dtors in the TU's .text (in reverse order) and vtables in its data. `-fno-weak` moves them into .text/.rodata but in a different order. To be solved before the early TUs can link (candidate: link-order/ldscript handling of linkonce sections, which is a protected file). | Recorded so linking work starts from it. | W12 report on tu_80008540
- 2026-10-11 | **0x803401A8..0x803663B0 is game code (`cflags_game`)** *[lead, owner authority]*. objdiff probes:
  - fn_8034151C, fn_80360EF8 and fn_803622B0 score 100% with the game flags.
  - With `cflags_ealib` they score 60.3%, 54.6% and 48.4%: -O2 hoists the loop invariants that retail recomputes inside the loop.
  - Exception: fn_80365C10 (0x7A0 bytes) sits alone after a 0xC-byte alignment gap at 0x80365C04, just before ealib/tu_803663B0. That gap marks an object boundary, so it got its own unit, `game/tu_80365C10`. Its flags are not confirmed.

  | Region split into 38 code-only units for range workers. | setup commits on work/setup-tail
- 2026-10-11 | **Code overlay units** *[lead, owner authority]*:
  - **Declaration.** Units are declared with the split input section: `fe/tu_X.cpp: .feoverlay start:.. end:..` and `db/tu_X.cpp: .dboverlay ...`. They live in the configure.py libs `fe_overlay` and `db_overlay`, which use `cflags_game`.
  - **Section attribute.** objdiff pairs functions only within sections of the same name. A function compiled into `.text` stays unpaired (no score) against the `.feoverlay` target, and `-c combineTextSections=true` doesn't change that. GCC 2.95/ProDG has no flag that renames the text section; the cc1plus and ngccc option strings were searched. So every function defined in `src/fe/` or `src/db/` carries `FE_OVERLAY` or `DB_OVERLAY` from `include/game/overlay.h`, which expands to `__attribute__((section(".feoverlay")))` or `.dboverlay`. It only sets placement and changes no instruction. It is also what the protected ldscript (`*(.feoverlay)`, `*(.dboverlay)`) needs to link a matched overlay object.
  - **Flags.** The overlays use the game flags: retail spills and reloads float parameters, which is the `-ffloat-store` tell.
    - fn_804BC414 (fe): 100% with the game flags, 32.5% with `cflags_ealib`.
    - fn_8052A3C0 (db): 100% vs 71.8%.
    - fn_8047D1AC, fn_8043FDC0 and fn_8043FE44 (fe): 100%.
  - **Limit.** Compiler-generated functions (implicit dtors, out-of-line inlines, thunks) can't take the attribute. They land in `.text` or `.gnu.linkonce.*` and won't pair; same family as the linkonce issue above.
  - **Link-order evidence.** The dtk write-back puts the overlay units at the top of splits.txt. The last fe unit (.ctors 0x805A2C40) and the last db unit (.ctors 0x805A2C44) sort between game/tu_803401A8 (0x805A2C3C) and ealib/tu_803663B0 (0x805A2C48). So in the retail link, the overlays' final TUs came between those two objects.

  | 77 fe and 41 db code-only units for range workers. Build OK; main.dol == retail. | probe on work/setup-tail

- 2026-10-11 | **Tail of main .text (0x8036F44C..0x8043FDB8): library map, flags and split** *[lead, owner authority]* (proposed by the tail-setup delegate). Probes compile one function or one reference file with candidate flags and compare it word for word to retail (relocation fields masked). "Reference" means the unmerged `port/mvp2005` branch (sources ported from mitsevox/mvp2005, CC0, commit f1e1763b; its unit ranges come from that project's filemap built from SN link maps) and W2's `match/W2-pad` branch; both are already in this repo.

  | Range | Library | Configure lib / flags | Evidence |
  |---|---|---|---|
  | 0x80362FB4..0x803663B0 | EA libs (units keep their `game/` paths) | per-object `cflags_ealib` | fn_8036344C: exact at -O2 -G0; 11 slots off at -Os; 37 off with -ffloat-store (retail keeps the float params in f30/f31). The 29+39 functions already matching in tu_80362FB4/tu_80362A30 match either way. fn_80365C10 follows a 0xC alignment gap and opens the GX shader family of ealib/. |
  | 0x8036F44C..0x8036FEA0 | EA shader TU "MVPNet" (was unsplit) | `ealib` | `_GLOBAL_.I` at 0x8036FE74 (.ctors 0x805A2C7C, moved from tu_8036FEA0) |
  | 0x80377048..0x8038BC58 | EA gamelib (`C:/mvp2004/libraries/gamelib`: soa allocator, datafile, animlib) | `gamelib` = `cflags_game` | fn_80377624 exact only at -Os; fn_8037B92C 4/42 slots off at -Os vs 35 at -O2 (retail recomputes loop invariants); 232 float stores reloaded immediately from the same slot (the -ffloat-store tell; 0 in EAGL) |
  | 0x8038BC58..0x8038EC58 | AV / VD (stream buffers, tBigSwizzler) | `ealib` | no r13 use; loops hoist invariants. Flags unconfirmed (no exact probe). |
  | 0x8038EC58..0x803A4214 | VP6, MAD codecs, SPCH 3.18.01 | `ealib_sda` = -O2, default -G8 | 55 .sbss/.sdata variables reached via r13 (impossible at -G0); fn_8039055C exact at -O2 (also at -Os); invariants hoisted. -O2 over -Os is by the loop heuristic only. |
  | 0x803A4214..0x803BA9DC | EA SND 9.02.04 + csis | `snd` (mitsevox flags, C++) | reference saems.c 74/74, smemman.c 5/5, salloc.c 4/4 exact; `cflags_ealib` gives 54/74. csis loops keep `mulli` per iteration (-fno-strength-reduce). |
  | 0x803BA9DC..0x803EB2AC | EAGLAnim + EAGL (with the 2 `ealibs/` units) | `ealib` | fn_803BD8BC exact only at -O2 -G0; the ealibs units match at -O2 -G0. One r13 load in fn_803E7320 is `_impure_ptr`, which newlib declares in .sdata. |
  | 0x803EB2AC..0x80403E80 | RealCore (realmem/realfile/realmemcard/...) and the formatting code after it | `ealib_sda` | r13 to its own .sdata/.sbss variables; fn_803EB2EC and fn_803EE450 exact at -O2 (G8); fn_803FF008 hoists the ctype table address (-O2; -Os does not) |
  | 0x80403E80..0x80406DCC | SN libsn debug kernel | `libsn` | hand-written (`lis rX,@h; nop` pairs, exception stubs, 4-byte PC* traps): original code, excluded from matching |
  | 0x80406DCC..0x80407C64, 0x8043EDC4..0x8043EFA4 | libsn C parts | `libsn` = -O2 -G0 -fno-common | reference FSasync.c 13/13 and dummy.c 8/8 exact; FSasync.c drops to 2/13 at default -G8 and 4/13 at -Os |
  | 0x80407C64..0x8040FCA0, 0x8043EFA4 | SN libc (newlib 1.8.2) | `libc` = -O2 -fno-common | reference memcpy, strcmp, qsort, rand, strtok, errno, atexit, fclose, vfprintf (7/7) exact; -G0 breaks the small-data users; -Os breaks all |
  | 0x8040FCA0..0x80413878 | SN libm (fdlibm) | `libm` = -O2 -mfast-cast -msafe-sda -G 1024 -fno-builtin -mstrict-align | reference e_exp, s_floor, ef_atan2, k_rem_pio2, sf_sin exact; plain -O2 fails all |
  | 0x80413940..0x80414F30 | libgcc DI-mode helpers + terminate support | `libgcc` | unconfirmed: `__clz_tab` is reached via r13, so -G0 may be wrong for these |
  | 0x80414F30..0x8043D960 | Dolphin SDK (AI, amcstubs, AR, ARQ, AX, base, CARD, DB, DSP, DVD, EXI, GX, MTX, OS, PAD, SI, VI) | `dolphin_sdk` (dolsdk2004 release flags); `dolphin_dvd`/`dolphin_mtx` -char signed, `dolphin_exi` -O3,p | reference CARDBlock.c 6/6, GXTev.c 14/14, dvdqueue.c 5/5, OSMessage.c 3/3 exact under GC/1.2.5n. The dvd/mtx/exi deviations are mitsevox's, not re-measured. Library starts from the 13 OSRegisterVersion callers. The existing OS.c/AI.c units keep their flags. |
  | 0x8043D960..0x8043E344 | SDK VM | `dolphin_vm` (GC/2.0) | prologue saves LR after the stack update (MWCC 2.x), as mitsevox reports |
  | 0x8043E344..0x8043EDC4, 0x8043F058..0x8043FDB8 | `sdk/` (library unconfirmed; the second is probably DebuggerDriver) | `dolphin_sdk` / `dolphin_dbg` (GC/2.0) | prologue styles only |

  Split: 254 code-only units (.text only, data extern by lbl_), ~25-60 functions each, cut at ctor-stub TU ends, library starts and reference file boundaries. Units carry the real file name where a reference range exists (libc, libm, libsn, snd/cmn, CARD/GX/DVD/OS/AX files, W2's dsp/pad/si/vi), so those sources can be ported 1:1. Only `__save_gpr`/`__restore_gpr` (original code) and `.init` stay in dtk auto units. main.dol OK and verify PASS after each setup commit.
  | Covers the last unsplit .text | commits 47e84bc, 7965e8f, f15dbab, 4628a18 |

- 2026-10-11 | **Flags: ealib/tu_803912F8 and tu_803927E4 = ProDG -O2 -G0 -fno-strength-reduce** (`cflags_ealib_nsr`) *[lead, owner authority]*. These units use no r13 small data (every global via lis/lwz) and keep `slwi`/`mulli` index math per iteration. Rebuilt under the new flags, objdiff matches go 8 -> 11 and 11 -> 13 with no regressions. The flags match cflags_snd's codegen here (same counts). tu_80394744 has the same shape but ends in fn_803959D8, which really uses small data; it stays on `ealib_sda` (6 vs 7 matched) until its boundary is re-split at 0x803959D8. | tail-01 worker evidence plus lead rebuild | report.json counts before/after |

- 2026-10-12 | **Flags: dolphin/os/tu_80431828.c (OS.c range) = cflags_dolphin_sdk + `-opt nopeephole`** *[lead, owner authority]*. This is like the existing dolphin/os/OS.c (cflags_dolphin_os). OSGetConsoleType keeps the `b` to blr and the bne-over-lis shape only with peephole off. Objdiff goes from 0/12 to 5/12 matched. | tail-10 worker evidence, lead rebuild | report.json |
- 2026-10-12 | **Note (no action): SDK OS files switch peephole per function.** Functions after the first hand-written (original-code) function in OSAlarm, OSCache, OSContext, OSInterrupt, OSReset and OS.c need `-opt nopeephole`; the ones before it don't. OSResetSystem also needs `-inline noauto`. The original sources likely use `#pragma peephole off` / `#pragma inline` there. Our verifier forbids pragmas (see the open pragma-allowlist question), so those functions stay unmatched for now. Splitting units at those points to emulate a pragma was considered and not done. | tail-10 worker | per-function probes |

- 2026-10-12 | **Flags: libgcc/tu_80413940.c = ProDG -O2 -G 1024** *[lead, owner authority]*. __divdi3, __moddi3, __udivdi3 and __umoddi3 each read a private 256-byte __clz_tab copy in .sdata2 via sda21, which needs a small-data limit of at least 256. The unit goes from 5/11 to 9/11 matched; the eh_context pair matches either way. Also renamed fn_80406CCC to `__shr2i`: it is the MWCC 64-bit shift helper that `s64 >> 16` emits (`b __lshrdi3`-style stub). | tail-08 worker evidence, lead rebuild | report.json |

- 2026-10-12 | **Split: dolphin/exi/tu_80425E54.c at 0x80426CD0; the first part (dvdlow.c) = cflags_dolphin_sdk (-O4,p)** *[lead, owner authority]*. 0x80425E54..0x80426CD0 is not EXI: it is dvdlow.c (DI registers at 0xCC006000: DVDLowRead, DVDLowSeek, DVDLowReset, the alarm and interrupt code). 19 of its 21 functions are byte-identical at -O4,p (char signedness makes no difference); under the unit's former cflags_dolphin_exi (-O3,p) only fn_80426C98 matched (probe: compile the unit's ready source with each flag set and compare word for word). The first part keeps the unit name `dolphin/exi/tu_80425E54.c` and takes `cflags=cflags_dolphin_sdk`; the remaining 30 functions (EXIBios.c onward, still -O3,p; the tail from fn_80428C64 looks like GX code) become `dolphin/exi/tu_80426CD0.c`. The name was kept (instead of `dolphin/dvd/tu_80425E54.c`) because verify.py keys regressions by unit name; see the open question of the same date. Counts: exi/tu_80425E54 1/51 -> 19/21 (+ 0/30 in the new exi/tu_80426CD0). Matching sources need `volatile` on the interrupt-shared statics (6D8, 6F0, 6F4, 6F8, 718, 71C, 6E8, 708) and a `u32 ticks = OSSecondsToTicks(10);` block local before OSCreateAlarm. | probe (tail-09 worker) | commit setup(dolphin/exi/tu_80425E54.c)

- 2026-10-12 | **Flags: dolphin/dvd/tu_80425954.c (dvderror.c) = cflags_dolphin_dvd with `-inline noauto`** *[lead, owner authority]*. fn_80425A70 (__DVDStoreErrorCode) is byte-identical with `-inline noauto` or `-inline off` and not at `-inline auto`, where the compiler inlines the table scan fn_80425954 (ErrorCode2Num) into it (retail calls it). fn_80425954 matches both ways. The same flag set is NOT usable for the whole of dolphin/card/tu_8041C2D8 (tried: 11/18 instead of 17/18, so retail relies on auto-inlining in CARDCheck), so only this unit gets it. Counts: dvd/tu_80425954 1/2 -> 2/2. | probe (tail-09 worker) | commit setup(dolphin/dvd/tu_80425954.c)

- 2026-10-12 | **Flags: ealib/tu_803EB2AC = ProDG -O2 (default -G8) -fno-strength-reduce** (`cflags_ealib_sda_nsr`) *[lead, owner authority]*. RealCore code here keeps per-iteration index math but uses small data. fn_803EBC70, fn_803EB5E4 and fn_803EB61C are exact only with -fno-strength-reduce. Objdiff goes 25 -> 28 of 29 matched, losing nothing. Other RealCore units were unchanged by the flag in the p3-24 worker's probe, so they stay on `ealib_sda`. | p3-24 worker probe, lead rebuild | report.json |

- 2026-10-12 | **Split: dolphin/exi/tu_80426CD0.c at 0x804286AC and 0x80428C64; EXIUart.c = cflags_dolphin_dvd (-O4,p, signed char), GXInit.c head = cflags_dolphin_sdk + `-opt nopeephole`** *[lead, owner authority]*. The last seven functions of the EXIBios.c unit were 0 % and are not EXIBios: 0x804286AC..0x80428C64 is EXIUart.c (probe, barnacle/UART init, write loop) and 0x80428C64..0x80428F14 is the head of GXInit.c (default texture/TLUT region callbacks, __GXShutdown), by shape against the reference SDK sources. New units `dolphin/exi/tu_804286AC.c` (4/4 at -O4,p signed char; nopeephole makes all four worse) and `dolphin/gx/tu_80428C64.c` (3/3 at -O4,p -opt nopeephole). EXIBios keeps its name, cflags and 23/23. Only 0 % functions moved, so verify accepts it (as with dvdlow). | p4-31 worker drafts, lead rebuild | commit setup(dolphin/exi/tu_804286AC.c, dolphin/gx/tu_80428C64.c)

- 2026-10-12 | **Flags: ealib/tu_80394744 = cflags_ealib_sda_nsr (-O2, default -G8, -fno-strength-reduce); no re-split** *[lead, owner authority]*. The earlier plan was to split the unit at 0x803959D8 (its last function, fn_803959D8, is the only one that uses r13 small data) and give the first part cflags_ealib_nsr. fn_803959D8 is already 100 % in this unit, so moving it would be a verify regression (open question 2026-10-12, unit keying). cflags_ealib_sda_nsr gives the same effect without moving anything: the rest of the unit declares its data as unknown-size arrays (no r13 either way), fn_803959D8 keeps -G8, and the unwritten loops keep per-iteration index math like tu_803912F8/tu_803927E4 (e.g. fn_803948C0's loop at 0x80394A74 recomputes `slwi r0,r0,2` each pass). Rebuild: 12/28 before and after, no function lost. | lead probe | configure.py
- 2026-10-13 | **Flags: dolphin/ai/AI = cflags_dolphin_ai without -opt nopeephole** *[lead, owner authority]*. `cflags_dolphin_ai` had been a placeholder copied from the OS flags ("Flags TBD"). With peephole on, every compiled AI.c function is byte-identical: retail uses `addi rX,rY,0` copies, which nopeephole turns into `mr`. One unit only (AI.c, built Apr 5 2004, a different SDK build from OS.c). The nm-00 worker found it; AI went to 16/19 with VERIFY PASS (merged 2026-10-13, e4eac93f).

- 2026-10-13 | **Source-link fully matched objects only after an actual retail-hash trial** *[lead, owner authority]*. Function matching is object-diff progress; Object(NonMatching) still links the extracted object. New read-only `tools/match/link_candidates.py` first compares allocated section names/types/flags/sizes/alignment and checks source imports against main.elf exports. These are a shortlist, not proof. A candidate is promoted to Object(Matching) only when the final DOL remains SHA-1 da6becdbea614d03c4b5eae9f8a0fb08e0a184dd and the unmodified verifier and literal/target checks pass. No linker-script changes, extra address aliases, or verifier modifications. First sweep: 18 units, +24,200 source-linked code bytes, complete units 64 -> 82. | Per-unit config commits and build/link-*-verify.log / link-*.log | Runtime names needed by two promotions: `_exit` is the existing label at 0x80413F3C, called by retail abort with status 1 (matching the existing SN libc source); `__div2i` is the 4-byte stub at 0x80406CC0 used by MWCC signed tick divisions, branching to the signed libgcc implementation fn_804139C0. Only symbol identities changed; the original exit/division stubs stay split. The VM source's old comment calling this divide stub unsigned is stale and must be checked when that unit is source-linked.

- 2026-10-13 | **Canonical call declarations for source linking** *[lead, owner authority]*. The existing ealib/tu_803912F8 implementation establishes fn_80391C04 as extern C unsigned(unsigned char, unsigned char, unsigned short), packing a trace tag. Correcting the shared game/tu_800101C0.h declaration and 11 local C++ int(int,int,int) declarations keeps all instructions unchanged and resolves imports in 15 fully matched units (45,344 B). Existing config names 0x8040B830 strncpy and 0x8040B190 memcpy; use their actual C names/prototypes in the promoted callers, leaving other unlinked header placeholders alone. Canonical memcpy activates compiler builtins: four functions in game/tu_803543FC changed with default flags; -fno-builtin keeps every function exact. Only game/tu_803543FC and game/tu_80353930 get cflags_game + ["-fno-builtin"]. Both final linked DOLs retain the retail SHA1. No linker aliases or original-stub replacements. | Per-unit commits; build/trace-link-*.log and build/tu_803*-memcpy-link.log |

- 2026-10-13 | **Csis allocator/observer C entry points and localized deallocator calls** *[lead, owner authority]*. snd/tu_803B998C.cpp defines fn_803B9D60, fn_803B9DDC, fn_803B9E08, fn_803BA240, fn_803BA4D0 and fn_803BA7C4 with C linkage; game tail/observable headers had C++ linkage. Keep their local typed object views and declare the actual C linkage; fn_803BA4D0 returns int, as implemented. All 15 users of tu_803401A8.h retained every instruction in scratch, and a full rebuild/verifier found no regression. Source-link hash trials unlocked 10 units (+54,656 B). Three other exact units used compiler builtin deallocator names absent from main.elf: the POD array in tu_80351150 now calls fn_80377488 explicitly, POD objects in tu_80144190 call fn_80377408, and Unk80361834 has an inline class-specific operator delete forwarding to fn_80377408. No destructor work was removed; all three linked source objects retain the retail DOL hash (+11,152 B). Original runtime implementations/names and linker scripts unchanged. | build/observer-*-verify.log, observer-link-*.log, observer-value-link-*.log |

- 2026-10-13 | **newlib ctype table starts at its EOF sentinel, 0x8060E9F0** *[lead, owner authority]*. Existing _ctype_ caller declarations and +1 indexing, combined with the retail data's zero first byte and expected character masks, establish a 257-byte table beginning one byte before the old lbl_8060E9F1 object. Updated the three non-libc consumers first, then replaced that interior symbol with the proper _ctype_ object (size 0x101). No overlapping interior alias remains. Padding after the table stays extracted; no game data was added to source. The final DOL retains retail SHA1 and all affected literal/target references resolve. Seven complete libc code units can now source-link (+1,236 B). | build/ctype-symbol-verify.log, tu_*-ctype-verify.log and ctype-link-*.log |

- 2026-10-13 | **Data ownership evidence (Codex; within existing matching workflow)**. Code-only splits omitted real private source data. Paired code relocations and compiled-byte comparisons support proper data splits for libm/newlib and selected SDK units; source promotion still requires the retail DOL hash, literal checks and unmodified verifier. Existing anonymous objects with overextended extents were bounded at proven section boundaries. Canonical FSasync global names come from matching public reference definitions and paired retail addresses, without aliases. Atexit's retail import is the C++ fn_8000E5BC(int) GC allocator wrapper, so this unit uses C++ compilation while atexit retains C linkage. No protected verification/build machinery changed. ef_log's pool still has an external reference and is left extracted. | Full acceptance logs under ignored build/, commits one unit each; 2026-10-13 PROGRESS entry. |

- 2026-10-13 | **Typed GX FIFO view and branch-aware clone checks (Codex; matching workflow)**. FIFO address 0xCC008000 is expressed through a typed volatile page-base pointer at 0xCC010000 and a sizeof-scaled signed index. This reproduces the retail address form with either 4-byte or 8-byte register views, without assembly or linker aliases. Per-unit full-DOL trials proved 12 graphics source objects and their data placements. Those trials exposed a source condition error hidden by masked branch relocations: fn_803DB5A0 and the conditional fn_803DB9C0 must remain inside the mB0 branch. The clone search now retains local-label destinations in twin shapes and independently resolves intra-function REL14/REL24 branches from ELF symbols/addends before accepting a compiled candidate. Real bad/fixed controls reproduce the former +0x158 versus retail +0x178 destination. This strengthens a search helper; the original verifier, literal checker and final retail-hash gate still decide acceptance. F44C's normal global Unk803DA440 construction emits its 20-byte registration node and ctor pointer; its actual constructor signature is void(*)(), void*, void*, void*, established in ealib/tu_803DA188.cpp. Local generated initializer symbols replace placeholder names at the same addresses and sizes. No raw byte tables, fake aliases, or constructor attributes were used. | build/branchguard-controls.json; per-unit acceptance logs; 69 helper regression tests. |


- 2026-10-13 | **Recover omitted wrapper arguments from retail callers and callees**. game/tu_8007A4DC's wrappers at 8007A4DC and 8007A500 forward incoming r4/r5 to float fn_8007189C(W*,int,int); retail callers pass r5 and consume f1. The indexed wrapper at 8007A524 also forwards its existing second integer and returns float. The wrapper at 8007B6F0 forwards r4/r5/r6 to fn_8052FC1C(Unk8052FA84_List*,int,int,int); retail callers provide those arguments. Correcting three mangled method names at unchanged addresses/sizes lets this exact 4,664-byte unit link. Local typed overloads bind the real C/runtime exports while retaining existing unrelated header views; fn_80402EF8's halfword stores support the local unsigned-short destination view. Whole-DOL hash, literal checks and unmodified verifier all pass. A trial that changed shared import prototypes exposed conflicting legacy declarations and was restored; only the wrapper-method signatures changed in the shared header.
- 2026-10-13 | **Stage a shared declaration before changing its provider ABI name**. fn_80090880's retail code preserves incoming r4 as the ID passed to C fn_80402CAC(const unsigned char*,int). Its old one-argument C++ prototype omitted that input. The already-linked fe/tu_80464090 had spelled the old mangled name directly, blocking a provider-only rename. A separate verified FE commit now uses the provider header; the following provider commit adds the explicit ID and corrects the mangling at the same address/size. A final FE commit uses a direct typed call. All intermediate commits keep retail SHA1 and pass regressions/literal checks; no aliases, mixed-unit source commits or verification changes. The corrected import unlocks game/tu_8035726C (2,052 B).

- 2026-10-13 | **Return-buffer and void-wrapper ABI repairs**. Actual callee definitions establish two eight-byte pair results (fn_80235C18, fn_8022EA60); retail fn_803D6528 copies five words into its hidden result. Replacing the oversized reconstructed caller buffers yields three exact functions without padding tricks. The real database setters behind game/tu_8007CAC4 return void: CF44, CF90, D004, D0E0, D12C, D180, D724, D8EC and D998. Corrected both existing shared declaration views and rebuilt all users; fn_800E25E0 additionally becomes exact, with no regressions. D1C8 remains int because fn_8059BE14 returns status in r3. All changes retain retail hash and pass literal checks plus unmodified verification. | Source commits and build/*-verify.log / tu_8007CAC4-data-link.log |

- 2026-10-13 | **Restore fn_802B96A4's actual forwarded ABI before source linking its caller**. Retail wrapper passes incoming r3/r4/r5 to fn_803FB64C and returns its node ID in r3. The real provider implementation is C int(int,int,int). Corrected the wrapper's reconstructed void() source to int(int,int,int), with a same-address/size Fv -> Fiii metadata rename. No source-linked consumer used the old name, so the provider commit passes before game/tu_800C2F90 is migrated. The caller uses explicit 32-bit pointer-to-integer boundary casts and a direct typed call. All wrapper instructions and sixty caller functions retain their exact bytes and branch destinations. No mismatched function-pointer call or extra alias remains. | build/agent-imports/800C2F90 proofs and per-unit full verification logs |

- 2026-10-13 | **SDK bodies may be ported from adonis-singh/re4 (dolsdk2004 lineage).** Its Dolphin SDK sources, compiled with our GC/1.2.5n flags, give instruction streams equal to ours modulo relocations for about 30 remaining functions (e.g. `fn_80415B20` = `__ARChecksize`, `fn_80434314` = `PackArgs`, both 0 differing slots). Same lineage as the mvp2005 port's SDK sources. Each port still needs our verifier, and the commit names the RE4 file and function as evidence. RE4's codeless-asm bodies are not ported. | CLAUDE.md: match SDK code against public decomp work where the version matches and note the source here | research agent report 2026-10-13 (`score_variants.py`: MATCH 1533/1533 and 98/98); owner authorised using the research in chat 2026-10-13
- 2026-10-13 | **Split: dolphin/os/OSTime.c at 0x80438628..0x80438778, cflags_dolphin_sdk + -opt nopeephole** *[lead, owner authority]*. The tail of dolphin/os/tu_80437444.c (after OSThread.c's OSClearStack) is OSTime.c: OSGetTime/OSGetTick (original code), __OSGetSystemTime, __OSTimeToSystemTime and two helpers. Its functions match only with the peephole pass off, while OSThread.c needs it on. __OSGetSystemTime and __OSTimeToSystemTime then match exactly (bodies from the public dolsdk2004 OSTime.c, as reused by the RE4 decomp). fn_80438704 (already matched) moves with its range; verify follows moved functions since PR #135.
- 2026-10-13 | **Split: dolphin/gx/GXFifo.c at 0x80429FF0..0x8042AB48; dolphin/gx/tu_80428F14.c (GXInit.c tail, 0x80428F14..0x80429FF0) with -opt nopeephole** *[lead, owner authority]*. GXInit.c is compiled with the peephole pass off, as is its head unit tu_80428C64, and GXFifo.c with it on. The dolsdk2004 bodies (via the RE4 decomp) of __GXInitRevisionBits and GXInit match only with peephole off. GXFifo.c's 20 matched functions moved with their range. __GXInitGX still needs GXInit.c's data (DefaultTexData, GXDefaultVATList, GXDefaultProjData, string pool) as TU statics; see the RE4 SDK worker's notes.

- 2026-10-13 | **Fable 5.1 is used when needed.** Default matcher stays Sonnet 5.5 with no size cap; a function goes to Fable 5.1 when a Sonnet attempt parks it, or up front for large functions with no useful twin. | Fable matched 3/3 never-attempted 1.4-2.5 KB functions in the test (16-35 min each); Sonnet matched the shared one at similar token cost; Fable has its own weekly cap. | owner in chat: "yeah lets do fable more broadly when needed"; SPEEDUPS #16
- 2026-10-13 | **No codeless asm either.** RE4's empty `asm("")` launders and `register ... asm("rN")` pins stay forbidden like all inline asm; functions that need them stay unmatched. | Keeps the rule simple and the verifier unchanged. | owner in chat: "RE4 following the rule i guess" (answering the 2026-10-13 open question)
- 2026-10-13 | **Claude and Codex coordinate through `tools/coord.py`** (docs/COORDINATION.md): a shared roster (who is who), unit claims, an inbox, and an integration lock held for every push to main. `claude-lead` settles conflicts; the owner overrides. AGENTS.md points Codex at CLAUDE.md and the protocol. | Gated pushes kept losing races between integrators, and Codex and Claude had no way to talk. | owner in chat: "we can do lock on lead, but i need a way for codex vs claude to know how to talk to each other/coordinate on who is who"; SPEEDUPS #19

- 2026-10-13 | **Names from related EA builds are naming evidence.** `config/GV4E69/refnames/` (imported from mitsevox/mvp2005, CC0) gives EA's own names where a reference build's function has the same code as ours (unique masked fingerprint, at least 12 instructions). Tier E2 evidence for a rename; game-code hits need a second reason, since many are look-alikes. No code or data from those builds enters the repo. | Answers REUSE.md recommendation 2 ("the owner's call"). | owner in chat 2026-10-13 provided reference discs for exactly this; the UEFA table checks out against their own GUCP69 disc (all 1,250 names are in its fifa_z.map)

- 2026-10-13 | **The repository stays public; the "private" wording is gone.** Owner, in chat: "yes you are okay to proceed with all of those things, ignore the rules in that report" (including: drop "private" from README.md, CLAUDE.md and the GitHub description). Resolves the 2026-10-12 open question with option (a). | Nothing pushed contains game data (history and all pushed branches checked 2026-10-13). | owner reply in chat
- 2026-10-13 | **Owner-authorised CI addition: `.github/workflows/decomp-dev.yml`.** Same chat approval as above. The owner then removed the `Edit(./.github/workflows/**)` and `Edit(./.claude/**)` deny rules from the main checkout's `.claude/settings.json` (left uncommitted: the file is in `protected.sha256`) and said: "i changed settings json now you can push that and make the changes you want to". GitHub-hosted runners cannot build the report (no game files), so `tools/progress_bot.py` uploads the verified build's `report.json` to the `progress-report` release just before it pushes its card commit, and the new workflow re-publishes it as the `GV4E69_report` artifact decomp.dev reads. It is a new file: `build.yml`, `tools/verify/**` and `protected.sha256` are unchanged, so the integrity job still covers them. The self-hosted path in `build.yml` was not used: on a public repo, pull requests from forks could run code on the owner's PC. | Commits b5f2aaf1, 1d01c120; first artifact on 51ff33c8. | owner replies in chat
- 2026-10-13 | **Not listed on decomp.dev; renamed to `ptitty12/mvp2005-decomp`.** Owner, in chat: "lets pause on submitting to that site" and "rename the project to mvp2005 decomp. I am just gonna stay working out of this repo". Agents don't submit the project to decomp.dev or post about it in community spaces unless the owner asks. The report pipeline above stays in place for a later listing. Local folders and worktrees keep their `decompile_mvp_04` names (the scheduled tasks and worktrees depend on them); the shared `origin` remote now points at the new URL, and GitHub redirects the old one. | — | owner reply in chat

- 2026-10-14 | **Project direction: the goal is a playable browser game, not a 100% matching decomp.** Owner, in chat, in priority order: "Getting it to run in a browser", "Multiplayer in browser (matchmaking can be served via my vps but want the game running locally in their browser)", "Being able to easily add new rosters/teams"; "Above all i want to build this on a good foundation not tech debt ... knowing that further decomp won't make me have to go revist mods". Then: "rebuild my entire repo to better specify our goal now and next steps for how we will solve for that, it's okay if we change the project significantly ... I trust you". What follows:
  - Two tracks (docs/ROADMAP.md). The **engine** (`engine/`) runs the retail code, translated from PowerPC to WebAssembly in the player's browser, with a runtime replacing the hardware and SDK. The **knowledge track** is the matching decomp, now prioritized by knowledge targets K1-K5 (`tools/goals/targets.py`).
  - Mods depend only on three stable contracts: the retail binary's code and addresses, the game's own data formats, and the engine's versioned APIs. They never depend on decomp source (docs/ARCHITECTURE.md §2).
  - Nothing derived from the disc is committed or hosted, including translated code. The player's browser produces it locally.
  - `docs/PHASES.md` is replaced by `docs/ROADMAP.md`. The old Phase 3 (a source port after a complete decomp) is dropped.
  - Paused: the near-miss tail and big-function passes outside the K targets. The permuter farm (CPU only) keeps running.

  | A source port needs all code and data in portable C first (52% of code and 1% of data were matched on 2026-10-13), and mods written against decomp source would break whenever the decomp changed. The retail binary never changes. | owner replies in chat 2026-10-14; analysis in docs/ROADMAP.md §3
- 2026-10-14 | **Name commits and contract commits.** Besides one-unit `match(...)` commits, two kinds of commit are allowed. Both must pass the verifier like any other:
  - `name(<unit>): <old> -> <new> (<evidence>)`, renames with evidence;
  - `contract(<symbol>): <what> [<units>]`, one call contract (signature, return type, shared struct) fixed in the shared header and every unit that uses it, in one commit.

  This resolves the two 2026-10-13 open questions below, where the one-TU rule blocked coherent signature repairs. | Names and types are now the main deliverable of the knowledge track (K targets); the one-TU rule made correct multi-unit contract fixes impossible to land. | lead, under the owner's 2026-10-14 delegation ("I trust you")
- 2026-10-14 | **Engine technical decisions** (docs/ARCHITECTURE.md §9):
  - D2: translation runs in the player's browser and emits WebAssembly; a CLI build serves development.
  - D3: C++20 with Emscripten for the translator and runtime, `webgpu.h` (Dawn natively, browser WebGPU), TypeScript for the web shell and VPS services.
  - D6: the renderer consumes the GX command stream.
  - D7: ISO and CISO disc images first.
  - D8: guest memory stays big-endian.
  - Licensing: no GPL code in the repo; vendored code must be permissive and recorded in docs/REUSE.md.
  - Open until their milestones: D1 (adopt or write the translator, E0a), D4 (guest thread switching, E0b), D5 (audio replacement level, E3).

  | D2 follows from the hard rule that nothing disc-derived is hosted. D6: game and EA code write vertices straight to the FIFO. D8: data files, save states and memory hashes then look the same on every machine. | lead, under the owner's 2026-10-14 delegation

- 2026-10-14 | **E0a closed: D1 and D4** (lead, under the owner's 2026-10-14 delegation).
  - **D1:** we write our own translator in C++20. One decoder and IR with a WebAssembly emitter (the product) and a C++ emitter (native development), plus a reference interpreter on the same semantics library.
  - **D4:** resumable guest threads on one host thread. A switch unwinds to the dispatcher, and the interpreter resumes the suspended frames. The E0b item 9 measurements confirm it; the fallback is one host thread per guest thread with a baton.
  - Reuse only permissive pieces, each recorded in docs/REUSE.md when vendored:
    - gcn-recomp upstream (ethannicholas, MIT at or after 3041bbd2; never the unlicensed Pokalie566 fork);
    - pureikyubu (CC0);
    - Aurora (MIT);
    - N64Recomp (MIT).

    GPL projects (DolRecomp, ModernGekko, GekkoAOT, RingOut, WebCompiled, Dolphin) are reference only.

  | Every mature GameCube recompiler is GPL, and none emits WebAssembly or translates in a browser (our D2). symbols.txt already gives all 24,158 functions and the 22 SDK jump tables. D4 is the only model that keeps save states and rollback possible without SharedArrayBuffer or JSPI. | docs/engine/E0_SURVEY.md §1-§6 (commit 868ef949)

- 2026-10-14 | **Roster/team pack format v1 (engine API 1.0) settled** (lead, under the owner's 2026-10-14 delegation). The spec is `docs/MODDING.md`; the reference implementation is `tools/rosters/pack.py`, with 41 tests. A `team.names` op sets a team's full name, nickname, abbreviation and city in `team.dat` and every LOC file, using string ids read from the player's own `main.dol` (SHA-1 checked), so packs never carry LOC ids. Asset roots cover the front end, `models.big`, stadiums and the announcer archives, and archive rebuilds keep the original alignment.
  - JSON manifest and patches.
  - Players keyed by "Last,First" text, never by row id; renames are explicit ops.
  - Teams keyed by `unique_team`, with the 126 slots fixed.
  - Field names are the on-disc column names, grouped by table.
  - Roster rules come from the decompiled roster loader.
  - A later pack may override another's cells only if it declares a dependency on it; there is no silent last-wins.
  - SHA-256 hashes for the pack (canonical JSON), the ordered set (used by the netplay handshake) and the result (decompressed table and LOC text).
  - v1 is deliberately strict, so later minor versions only relax rules.

  Home park, the team list and the LOC-id tables are fixed in code (ROSTERS.md §12), so v1 team packs can't change them. | Packs never contain retail data and depend only on the game's own data formats (contract 2). Strict-first keeps every later change backward compatible. | docs/MODDING.md; end-to-end check on the owner's disc (empty pack gives 0 errors; the example applies byte-deterministically)

- 2026-10-14 | **E0b closed: the retail code boots natively in our engine.** The translated DOL runs from `__start` through all four `OSCreateThread` sites into the frame loop. `VIWaitForRetrace` returns. Two runs give byte-identical SDK logs (178 SDK functions, all named). A long run reached 102.9 s of guest time. Engine-timing decisions (lead, under the owner's delegation; details in docs/engine/E0B_LOG.md):
  - **CLK1:** the virtual time base advances 2 cycles per guest instruction. It is part of the netplay handshake.
  - **GX1:** the GPU is treated as infinitely fast except at FIFO breakpoints, so GPU events land on 32-byte write-gather bursts.
  - **DVD1:** a DVD command completes 1 ms after issue, plus its length at 3 MiB/s; inquiry data is fixed.
  - **DSP1 (interim):** AX frames are acknowledged, but voices don't advance yet (R8 follow-up in E3).

  D4 is confirmed: 10.7 thread switches per VI field, 11.75% of instructions interpreted early in boot and 1.13% over a long run. Reducing unwinds per switch is the next optimization. | These keep every source of time inside the guest (CLAUDE.md determinism invariant), so the same constants on every machine give identical runs. | commit f060c66c; local test local_boot_twice

- 2026-10-14 | **Engine toolchain downloads approved by the owner.** Owner, in chat: "yes you can download the engine tool chain go crazy feel free to notify me if you have other questions but mostly i trust you". This covers:
  - the Emscripten SDK (github.com/emscripten-core/emsdk);
  - a native WebGPU implementation (Dawn, or wgpu-native prebuilts from their official releases);
  - developer libraries the engine needs, such as SDL3.

  Everything goes under `C:\Users\Patrick Taylor\tools-ext\`, with exact versions pinned in `docs/engine/TOOLCHAIN.md`. Nothing game-related is downloaded, ever. | E1 (renderer) and E2 (browser) need them. | owner reply in chat
- 2026-10-14 | **Owner permits the K1c change to the protected linker script.** Owner, in chat, quoting the lead's note that the four linker symbols must be added to `config/GV4E69/ldscript.ld`: "you can change this i give permission".
  - Scope: add `_stack_addr`, `_stack_end`, `__ArenaLo` and `__ArenaHi` (the exact lines in the K1c open question below).
  - Refresh only `ldscript.ld`'s line in `tools/verify/protected.sha256`, using `python tools/verify/verify.py --print-protected`, so CI's integrity job stays consistent.
  - No other protected file changes.

  The lead also accepts the K1c link-order fix (12 `.sbss` split lines dropped, `libc/errno.c` with `-fcommon`; staged as `staged/k1c-scommon`, VERIFY PASS) under the owner's 2026-10-14 delegation. | dtk's SDK signatures for `__start`, OSInit and `__OSThreadInit` relocate to linker-defined symbols. Naming OSInit and DVDInit needs them. | owner reply in chat

- 2026-10-14 | **E0c engine decisions** (lead, under the owner's delegation; details in docs/engine/E0C_LOG.md).
  - **E0c-D4b, how D4 is implemented:**
    - unwinding uses plain C++ returns;
    - every call-return address and loop back edge (125,559 in the DOL) is a resume label;
    - a resumed thread re-enters its native function body at the label.

    Measured: exactly 1 unwind per real thread switch; the interpreted share went from 11.75% to 0.0116%.
  - **E0c-IN1, input:** a per-frame input queue (`engine/runtime/input.h`, versioned `PadState` v1, 45 bytes per frame) is consumed at the frame poll (fn_8014865C). PADRead is replaced and returns that frame's state. This queue is the netplay lockstep entry point. A scripted START press changes the run from the next frame, and two runs with the same script are byte-identical.
  - **E0c-AX1, audio voices:** on the DSP side, each running voice advances 160 samples per audio frame: 16.16 rate conversion; ADPCM, PCM16 and PCM8 addressing; loop or stop. There is no mixing yet (E3). Sounds now finish, and EA SND frees their voices.

  | The determinism invariant is kept: two-run SDK logs are byte-identical in every configuration. | commits 0a4ff1c4, 3968dfbb, d29148e8

- 2026-10-14 | **E2 groundwork closed: in-browser translation is viable** (lead, under the owner's delegation; numbers in docs/engine/E2_SPIKE.md).
  - **Measurements:**
    - The translator itself, built as wasm, translates the whole DOL in 0.33 s.
    - The module is 49.5 MB. V8's lazy compile takes 25-45 ms and eager Liftoff 0.22 s.
    - The game code runs at 87% of native `/O2` speed.
    - Booting through the wasm emitter gives the same SDK-log hash as native.
    - FP digests are identical between native and wasm. FP is integer-only by design, so no FMA or NaN workaround was needed.
  - **E2-W1, emitter design:**
    - the WebAssembly emitter builds the module straight from the CFG, with no C compiler;
    - guest registers live in locals, with spills decided by per-instruction register effects (`ppc/effects`);
    - direct forward branches; a dispatcher for back edges, jump tables and resume labels;
    - `return_call` only at guest tail calls;
    - E0c's call protocol, so wasm-emitted and C++-emitted functions interchange.
  - **E2-W2:** wasm exceptions are used only to end a run.
  - **D2 refinement:** one module translated at startup is enough, so per-unit modules and lazy translation aren't needed now. Caching the translated module is optional (re-translating costs about 0.35 s).

  | Retires E2 risk #2 (`E0_SURVEY.md` §7). Risk #3 (engine differences across Chrome, Firefox and Safari) remains for the browser page. | commits 7212973d..86ef36c9

- 2026-10-14 | **E3b closed: game audio plays; D5 decided** (lead, under the owner's delegation; details in docs/engine/E3B_LOG.md).
  - **D5: audio is replaced at the DSP command-list level, not the AX API.**
    - AX, EA SND and EA's CPU mixer stay retail code, so voice acquire/free, the DSP-cycle budget and the sync flags are untouched.
    - The intro audio exists only at this level (EA mixes on the CPU and feeds AX aux A).
    - The interface is one command-list format plus the voice parameter blocks: a pure function of guest memory at points fixed by guest instructions.
  - **AX2:** voices are processed per millisecond with the envelope written back. A read-only frame observer feeds the mixer.
  - **MIX1:** the mixer (`engine/audio/`) is a read-only observer of guest memory. It handles 4-tap polyphase or linear rate conversion, the envelope, low-pass, mix ramps, depop, aux returns and Dolby Pro Logic II (the game's default mode; folded to stereo). Output is 32 kHz (the rate the game sets) into a lock-free ring, with an adaptive resampler (±0.5%) to SDL3. Host audio timing never reaches the guest.

  `local_audio_determinism` (mixer on vs off) gives an identical SDK log, MEM1 and frame log. | Keeps retail code retail and keeps audio fully outside the netplay contract. | commits c9bb883d..7055f22c

- 2026-10-14 | **Private deployment may serve the owner's disc image.** Owner, in chat: "since i already have my own copy and this is just for me and my brother ... who also has his own disk image we don't need to worry about the user proving a disk since it's a private url anyway".
  - The owner's deployment may serve the owner's own disc image to the owner and invited players who own the game. The page loads it from the server instead of the file picker, and translation still runs in each browser.
  - The lead adds a password gate in front of the whole site; a secret URL alone leaks through history, logs and link previews.
  - Unchanged: the public repo never contains game data or anything derived from it; nothing game-derived is served anywhere other than that private deployment; the disc-identity check stays.

  | Convenience for two players who both own the game; keeps the public repository clean. | owner reply in chat

- 2026-10-14 | **E2 closed: the game boots in the browser** (lead, under the owner's delegation; docs/engine/WEB.md, E2_LOG.md; decisions E2-W3..W5 are in E2_LOG).
  - In Chrome 155, Edge 155 and Firefox 157, the page loads the disc image and checks its identity (GV4E69 rev 0 SHA-1). A Worker translates the DOL in about 0.5 s and WebGPU renders on an OffscreenCanvas.
  - Shown so far: the loading screen, EA logo, licence screen, title screen, memory-card dialog and main menu. Keyboard input works.
  - Pick to first frame takes 0.67-1.09 s. Everything except the intro movie holds 59.94 fields/s.
  - Guest-state hashes equal native at every checkpoint (Chrome, Edge, Firefox; WebKit without WebGPU).
  - `rt::run()` returns to the host at each VI field without the guest being able to tell.
  - The page only requested the engine's own files.

  Safari itself is unmeasured (needs a Mac). | Exit of ROADMAP E2. | merge a6239996 (E2b plus E3b, 34/34 native and 31/31 Node tests)

- 2026-10-14 | **E2c: browser client** (lead, under the owner's delegation; docs/engine/E2C_LOG.md).
  - Browser audio goes through an AudioWorklet fed over a MessagePort, not a SharedArrayBuffer, so no cross-origin isolation headers are needed. The buffer target is 60 ms (sound trails the picture by 80-115 ms).
  - The gamepad maps to the GameCube pad and is sampled only at the frame poll.
  - The page loads the disc from `/config.json` and `/api/disc/image` on the private deployment.
  - The translation is cached per DOL and engine build, cutting start-up from 0.7-1.1 s to 0.3-0.4 s.
  - The deployment must use HTTPS: WebGPU, AudioWorklet and OPFS need a secure context.
  - **Lead decision:** the disc blocks the browser fetched from the private server may be cached in that player's own browser storage, so later boots are fast. This stays on the player's machine (the owner and invited players own the game). The server keeps sending `no-store` so no intermediary caches it.

  | | merge of engine/e2c

- 2026-10-14 | **E3a: the engine reaches a game** (lead, under the owner's delegation; docs/engine/E3A_LOG.md).
  - Driven by script natively from power-on to an exhibition game (Cardinals at Red Sox, Fenway): pre-game, the first pitch with the pitch meter, the first ball in play.
  - Speed (guest time per host time): intro movie 1.2×, menus 5.8×, in-game 0.9× with rendering and 1.4× without.
  - **E3a-FP1:** the floating-point-unavailable exception (0x800) is part of the CPU model. It was the root cause of the movie specks: the OS's lazy FPU switch had let threads share FP registers.
  - **E3a-FP2:** FP runs through host doubles only where the result provably equals the exact model (`fp::ref`; `test_fp` checks 21M cases per run).
  - **E3a-WG1:** writing WPAR empties the write-gather pipe, and a redirected FIFO end keeps bit 26. Both bugs crashed the game after the loading screen.
  - **E3a-B1:** translated code builds at /O2 by default (`MVP_GAME_OPT`), except functions with straight runs over 1,024 instructions. Guest state is identical at /Od and /O2.
  - **Known gap:** the WebAssembly emitter doesn't have FP1 yet (E2d is adding it; `test_wasm_lockstep` is DISABLED until then). The browser build doesn't match native until it does.

  | Determinism kept: 40/40 native tests, identical hashes at /Od and /O2. | merge 4f61815f

- 2026-10-15 | **E2d: browser game code inline and exact** (lead, under the owner's delegation; docs/engine/E2D_LOG.md).
  - **E2-W6:** each FP and paired-single instruction gets a fast path on IEEE doubles only where it provably equals `sem.h`; otherwise it falls back to the runtime's exact semantics. Only 130 of 1,473,997 instructions still go to the runtime.
  - **E2-W7:** MSR[FP] checks (the FP-unavailable exception, mirroring E3a-FP1) are placed by dataflow.
  - **Evidence:** a digest through translated code equals native; 170M+ fuzz cases with 0 differences; `test_wasm_lockstep` re-enabled and passing; Chrome and Firefox guest lines equal native at every WEB.md checkpoint.
  - **Speed in Chrome:** the intro movie went from 0.25× to 2.26× real time unpaced, and the main menu takes 5.0 ms per field. The module grew from 49.5 to 62.1 MB.

  | | merge of engine/e2d

- 2026-10-15 | **E6: roster packs in the engine** (lead, under the owner's delegation; docs/engine/E6_LOG.md).
  - In both the native and browser builds, the example pack turns Anaheim into the 'Harbor City Lanterns', with the pack's players in the in-match lineup.
  - The larger, recompressed database.big boots and plays (answers MODDING.md §14 question 1).
  - **E6-M1:** `engine/mods` stays byte-compatible with `tools/rosters/pack.py`, enforced by `mods_crosscheck` (65 scenarios, 0 differences).
  - **E6-V1:** a virtual-disc overlay layout with end padding.
  - **E6-N1:** the netplay disc identity is taken from the unpatched disc. The pack-set hash is a separate handshake field, and a mismatch refuses the game on both sides.
  - **Known bug (not packs):** netplay stopped with a gate miss when the match started loading. **Fixed by E5c (2026-10-15).**

  | | merge of engine/e6

- 2026-10-15 | **E3c: a full 9-inning game natively** (lead, under the owner's delegation; docs/engine/E3C_LOG.md).
  - CPU vs CPU from the first pitch, through the game's pause-menu controller select, to Game Over (Cardinals 18, Red Sox 11). No crash; audio above -60 dBFS throughout.
  - Two runs, with and without the renderer and audio, are identical at all 54 checkpoints. A window session's recorded keyboard input replays headless to the same state.
  - In-game speed went from 0.68× to 1.12× real time: hardware registers moved to flat arrays, indexed and packed draws, draw-state reuse.
  - **E3 exit status:** natively met. Frame pacing is steady at the median (16.8 ms) but not in heavy scenes.
  - **Open engine decision CLK2:** at 2 cycles per instruction (CLK1) the game renders about 38 fps in play (logic stays 60 Hz). Lowering it would change guest timing and the netplay handshake. Decide with evidence about the console's in-game frame rate.

  | | merge of engine/e3c

- 2026-10-15 | **E5c: netplay into the match** (lead, under the owner's delegation; docs/engine/E5_LOG.md).
  - **E5-N4:** input-source Before hooks may stop the run (`InputSource::before_poll`). When a frame's input is missing, the run stops before the poll consumes anything, the translated function unwinds, and the same poll repeats once the input arrives. The guest never runs while waiting; the wire protocol stays v1.
  - Cause: during match loading, frames run without rendering and no hardware event falls between two polls, so no safe-point yield happened.
  - Evidence: a native two-player game for 60,000 frames; Chrome ×2 to the first at-bat, equal to native (also over simulated loss and latency) and with packs on both pages. 0 desyncs, 0 gate misses; solo runs unchanged.

  | | merge of engine/e5c

- 2026-10-15 | **P1-disc: no more disc-read hangs in the browser** (lead, under the owner's delegation; docs/engine/P1_LOG.md).
  - Measured cause: every guest DVD read was a network round trip to the VPS. That added 18.6 s of read waits up to the first pitch, single reads up to 1.4 s, and fields up to 0.7 s.
  - **P1-D1:** the page downloads the server's disc once into OPFS (resumable, 4 pieces in parallel, each checked by hash), then reads it locally. After the fix: 0 reads over 50 ms.
  - **P1-D2:** the image hash is a SHA-256 over the 8 MiB piece hashes. It is advertised in `/config.json` and computed once by the server, which caches it in the volume.
  - **P1-D3:** no start during the download (reads would queue behind it).
  - **P1-D4:** the Dokploy volume name is left unpinned, so the disc the owner already uploaded is kept.
  - Also: no picker or upload in the main flow when the server has a disc.

  | | merge of engine/p1-disc

- 2026-10-15 | **P1-gfx: flashing fixed, smoother pacing** (lead, under the owner's delegation; docs/engine/P1_LOG.md).
  - **P1-G1, presentation:** the game alternates HUD elements every field and relies on the console's 480i output to blend them. Scan-out now shows the last field's XFB frame, except that pixels alternating A, B, A over the last three fields show the mean of the last two.
    - Flashing events in human play dropped from 41 to 2. Captures still read the XFB itself.
  - **P1-G2, chase pacing:** while behind, the browser runs the next batch immediately in its own task, not at the next animation frame. Present p95 in a match went from 60-64 ms to 27-33 ms, at 1.0× real time.
  - Guest state is unchanged (hashes equal native); 44/44 native and 38/38 Emscripten tests pass.
  - **Next:** a render thread (wasm threads plus COOP/COEP), a faster vertex path and faster memory helpers. Open renderer bug: the half-scale EFB copy at 0x01797180 is sampled as 256x256.

  | | merge of engine/p1-gfx

- 2026-10-15 | **CLK2: freezes and slow motion fixed; the clock recalibrated** (lead, under the owner's delegation; docs/engine/CLK2_LOG.md).
  - **E0b-IDLE2:** an idle spin must read the same addresses on every pass. The idle skip had treated 6 list walks (EA `FREE_add` and others) as spins, jumping virtual time once per node visited.
    - Effect: 30% of play time showed frozen pictures (up to 8-10 s), logic ran at about 43 Hz (slow motion), and the match load took 69 s.
    - After the fix: about 60 fps rendered, 60.4 logic ticks/s, an 8.5 s match load, and the longest held picture 0.7 s.
  - **CLK2 replaces E0b-CLK1:** 1.25 cycles per instruction, the smallest charge with no CPU-bound second in heavy play and within the Gekko's plausible range. The netplay handshake carries it in thousandths (1250), so CLK1 peers are refused.
  - The open CLK2 question is resolved: the 38 fps came from freezes, not the clock.
  - Frame-keyed input scripts need re-timing.
  - Remaining browser choppiness is host cost (the Worker is 96% busy in heavy play); the render thread (P2) addresses it.

  | | merge of engine/clk2

- 2026-10-15 | **P2: smooth play in the browser** (lead, under the owner's delegation; docs/engine/P2_LOG.md).
  - **P2-T1:** a threaded engine (`mvp-web-mt`, pthreads and shared memory). A render thread owns WebGPU and runs the GX processor and the renderer, fed by a 16 MiB command ring. When the page is not cross-origin isolated (or `threads=0`), the single-threaded engine is used.
  - **P2-T2, exactness:** `gfx::GpReads` marks the guest memory the renderer will read, per 64 bytes, and guest writes wait only where it hasn't finished reading. Fences alone were not exact.
  - **P2-T3, presentation:** at most one present per display refresh, sent to the page as an ImageBitmap.
  - **P2-H1:** the server sends COOP, COEP `require-corp` and CORP `same-origin`. The first deploy compiles the engine twice (20-40 minutes).
  - **Measured in Chrome, in a match (CLK2 engine):** 1.00× speed; 11.4 ms per field on the emulation thread (was 14.7); composited p95 about 20 ms (was 38-50); 0.5-0.8% of frames over 25 ms (was 9-13%); 57-59 new frames shown per second.
  - Exactness: renderer digests are identical threaded vs single-threaded (0 of 20,600 fields differ), guest lines equal native, and netplay lockstep holds.
  - Also fixed: the tall EFB copy at 0x01797180.

  | | merge of engine/p2-thread

- 2026-10-15 | **Q3: controller support in the browser** (lead, under the owner's delegation; docs/engine/Q3_LOG.md, decisions Q3-C1..C5).
  - Pads: standard-mapping pads (Xbox, DualShock 4, DualSense); Switch Pro with Nintendo button labels; an Xbox raw profile; GameCube adapters through a guided setup (no layout guessed without hardware).
  - Sticks get a radial dead zone and the octagonal gate; triggers are analog with a click threshold.
  - The Controls panel offers per-port assignment, press-to-bind for pads and keyboard, a live test, and per-model mappings stored in the browser. A pad claims a port on its first press.
  - Online, the local pad drives the player's own port, and the lobby shows each player's controller readiness.
  - Input still reaches the guest only at the frame poll. `?inputLog=1` replays prove it (live run equals its replay).
  - Not yet: a real-controller test, in-game rumble (a `PADControlMotor` hook in hle.cpp), WebHID for the official GameCube adapter.

  | | merge of engine/q3-pad

- 2026-10-15 | **Q1: menu flashing fixed; picture quality** (lead, under the owner's delegation; docs/engine/Q1_LOG.md).
  - **Q1-G1:** the renderer reads guest memory as the CPU cache has written it back. Lines flushed with the SDK's DC* calls are read from a snapshot until the next field; DMA and the write buffer end the snapshot.
    - The cause: EA's sprite code writes the next sprite's vertices before waiting for the current draw. On console those writes are still in the cache.
    - Menu flicker went to 0 (title 130 → 0, main menu 238 → 0), and the wrong logos and grey boxes in play are fixed. Guest state is unchanged.
  - **Q1-G2:** 4:3 display; internal resolution 1-3× (Auto, chosen by canvas size); area or sharp-bilinear final scaling; the game's texture LOD bias applied.

  | | merge of engine/q1-visual

- 2026-10-15 | **Q2: sound dropouts; weaker machines** (lead, under the owner's delegation; docs/engine/Q2_LOG.md).
  - **Q2-A1:** on pages with the isolation headers, the mixed sound reaches the audio thread through a shared-memory ring. Pages without them keep the message path.
  - **Q2-A2:** when the emulation runs behind, the sound is time-stretched with pitch kept (down to quarter speed) instead of going silent.
    - After an underrun, playback resumes once 30 ms is buffered. The buffer target grows by 20 ms per underrun, up to 200 ms.
    - Underruns per minute in play: 7.2 → 0.7 on this PC (threaded); 251 → 0.5 at a simulated 2× slower machine; 170 → 0 on 4 efficiency cores.
  - **Q2-P1:** the default pacing stays the chase, which drops frames evenly when behind. Even slow motion is opt-in with `?slowMotion=1`.
    - It shows more new frames (37.7 vs 25.9 at 2× slower) but the game runs 9% slower.
  - **Q2-S1:** the translator has no inline memory path. It made the module 62 → 90 MB and play slower.
  - **The limit on weaker machines is the emulation thread:** 0.69× speed at a simulated 2× slower. The next speed work goes there.

  | Guest lines equal to native threaded, single-threaded and in Firefox; netplay_check passes. | merge of engine/q2-perf

- 2026-10-16 | **Owner: the engine no longer has to be bit-exact to the GameCube** (owner: "I don't really care about byte identical to the gamecube, as long as it runs").
  - **F1:** the engine's requirement is now:
    - deterministic across every player's machine: native == wasm == every browser, on x86 and ARM hosts (Apple Silicon included);
    - the game plays correctly.

    Reproducing the console's exact floating-point results or timing is no longer required.
  - **What it changes:**
    - E3a-FP2 ("host FP only where provably equal to the exact model") becomes "host FP wherever its results are the same on every host".
    - NaN results are canonicalized wherever their bits can reach memory or integer code; x86 and ARM produce different default NaNs.
    - The same applies to paired singles, fused multiply-add, FPSCR flag tracking and timing details.
    - The reference hashes in WEB.md are re-recorded when the semantics change.
  - **What it doesn't change:**
    - The determinism invariant (docs/DETERMINISM.md): host nondeterminism enters only through the input queue and the virtual clock.
    - The checks: native and wasm agree at every checkpoint, netplay_check passes, a full CPU vs CPU game plays out.
    - The knowledge track: decomp matching stays byte-identical under the verifier, because that is what makes names and contracts trustworthy for mods.

  | Unlocks faster FP, paired-single and memory paths (Q4) and native versions of hot game functions. | owner, in chat

- 2026-10-17 | **Q5: online play between unequal machines** (lead, under the owner's delegation; docs/engine/Q5_LOG.md).
  - **Q5-N1, even lockstep pacing:**
    - Each side proposes its capacity × 0.98, and both run at the lower proposal, one field per slot.
    - Phases are centred by the input lead. Host timing only; solo keeps Q2-P1.
    - At a partner 1.3× slower: the slow page shows 33.5 → 43.0 new frames/s, its frames over 50 ms fall from 6.9% to 1.0%, and the fast page's waits fall from 34.7 to 1.6 per second.
    - The cost: game speed 0.81× → 0.72× at 1.3× slower. At 1.6× slower there is no cost (0.68×). The lead accepted this cost for smoothness and has a follow-up to win it back.
  - **Q5-N2, Auto input delay:** the lobby default is Auto: 20 pings, then E5's rule, between 2 and 6. Room delay 0 means Auto; a manual choice overrides it.
  - **Q5-N3, data-channel text messages:** `pace` (4 Hz), `stats` (1 Hz), `ping`/`pong` join `pad`. They are JSON with `v: 1`, the engine never sees them, and the wire protocol stays at v1.
  - **Q5-W1, statistics overlay:** the Stats button or the ` key, off by default, with Copy stats for field reports.
  - **Q5-W2, slow-machine note:** a dismissible note below 0.93× that suggests plugging in and the Best performance power mode.
  - **Q5-G1, GPU-aware Picture Auto:** render passes are timed with WebGPU timestamp queries. Auto steps down above 85% GPU busy for 2 s and back up after a hold below 60%. Fixed scales are never changed.
  - **Q5-T1, measuring two machines on one PC:** two browser instances pinned to the performance cores.

  | Guest lines unchanged; 48 two-page games with mismatched machines, 0 desyncs; msvc 46/47 (+1 skip), emscripten 39/40 (+1 skip), web shell 87/87. | merge of engine/q5-netpace

- 2026-10-17 | **Q4: emulation-thread speed** (lead, under the owner's delegation; docs/engine/Q4_LOG.md).
  - **Results:**
    - Emulation per field: 11.4-12.5 → 9.2-10.0 ms on this PC.
    - The laptop stand-in (hostSlow 1.3): 0.93-0.95× → 1.000×, 58-59 new frames/s.
    - hostSlow 2: 0.61-0.70× → 0.78-0.86×. 4 efficiency cores: 0.83× → 0.93×.
  - **Q4-G1:** translated code appends GX pipe stores to the pipe's 4 KiB block itself and calls the runtime only to complete a burst. `Hardware::write` stays the reference.
  - **Q4-W1:** GPU-read write tracking by epoch; slightly more write waits accepted.
  - **Q4-H1:** in translated WebAssembly, fuller in-module helpers are preferred over inline expansion (confirms Q2-S1).
  - **Q4-F1:** the fast FP model (under F1) is the default.
    - One IEEE double op per step, no FMA, single results rounded through f32, NaNs canonical.
    - The exact model stays selectable (`fp::g_model`, `WasmEmitOptions::fp_exact`).
    - Native builds keep `/fp:precise` or `-ffp-contract=off`.
  - **Q4-N1:** native implementations of game functions (`HleKind::Native`) must match the original in all guest state: PC and ticks at each hardware access, and the instruction count.
    - Outside their assumptions they fall back to the original, and the interpreter always runs the original.
    - Each needs a disc test against the interpreter. The first is `fn_803EB000`, the skinning-matrix load.
  - **Q4-M1:** on a shared PC, compare before and after with paired Node runs pinned to separate E-core clusters, swapping clusters between pairs.

  | WEB.md checkpoints unchanged; match-session lines equal native; netplay_check passes; a full CPU vs CPU game ran to Game Over. | merge of engine/q4-speed

- 2026-10-17 | **Q5b: pacing roles; Auto delay measured twice** (lead, under the owner's delegation; docs/engine/Q5_LOG.md §15).
  - **Q5b-N1, pacing roles:** online, a machine whose capacity is 3% below the other's for half a second sets the pace and runs one field at a time without idling.
    - The other machine follows the reported capacity, locked to equal input leads (a correction of at most 4%).
    - Player 1 decides the roles. Machines within 3% of each other keep Q5-N1.
    - Results on the merged engine:
      - 1.0 vs 1.3: 0.975×, with the pace at 1.0 about 91% of the time.
      - 1.0 vs 2: 0.75-0.78× (the chase 0.77×). The slow page shows 45 vs 31 new frames/s, frames over 50 ms fall from 8% to 0.3%, and the fast page's waits fall from 42 to 2 per second.
    - Host timing only.
  - **Q5b-N2, Auto delay measured twice:** two rounds of 20 pings, and the lower result is proposed (amends Q5-N2).
  - **Q5b-T1:** pacing variants are compared by the slow page's busy share as well as speed, because the per-field cost varies 5-10% between runs on this shared PC.
  - **Not built, run-ahead:** it needs a one-frame display queue in the render thread and adds up to 16.7 ms of input latency. Only if the owner sees hitches.

  | 81 two-page games, 0 desyncs; each line equals native at its delay; WEB.md checkpoints unchanged; web shell 90/90. | merge of engine/q5b-pace

- 2026-10-17 | **Q7: the frozen top-left block** (lead, under the owner's delegation; docs/engine/Q7_LOG.md). Owner report: "a sizable block [in the top left] is not updating or is in the wrong spot".
  - **What the game does:** every frame of play it saves the EFB's top-left 128×128 to a texture, draws the ball-shadow map there, then restores it.
  - **The bug:** the texture bind-group cache was keyed by WebGPU handle addresses.
    - In the browser (emdawnwebgpu) a released view's address is reused at once.
    - After any internal-scale change (Picture setting, Auto on fullscreen or resize, Auto stepping down on a busy GPU), the cache returned groups that sampled the dead textures.
    - Result: the corner froze, along with shadows, player shading and the video board.
  - **Q7-G1:** any renderer cache keyed by WebGPU handle addresses holds references to those handles.
    - A replaced EFB-copy texture's entries are dropped.
    - The pass rebinds after the cache frees groups.
  - **Q7-T1:** the scale-change check runs whenever the renderer's copies, caches or scale handling change. It is `browser_check --actions ... --compare`: a mid-game Picture change compared with a fixed-scale run, top-left 128×128, mean ≤ 3. The worker presents the batch that stops a run.

  | Before: region mean 32.8 → after 0.00 (pixel-identical); guest lines unchanged; netplay_check passes; msvc 48/49, emscripten 41/42 (+1 skip each), web shell 91/91. | merge of engine/q7-topleft

- 2026-10-17 | **Q6: emulation-thread speed, second pass** (lead, under the owner's delegation; docs/engine/Q6_LOG.md).
  - **Results against main:**
    - hostSlow 2: 0.90× → 0.975×, with 51 new frames/s, up from 36-43.
    - 4 efficiency cores: 0.92-0.94× → 0.993-1.000×.
    - The laptop stand-in (hostSlow 1.3): 11.0-11.3 → 9.7-10.0 ms per field.
  - **Q6-F2 (under F1):** the fast FP model drops NI's flush to zero.
    - NaN results stay canonical after every result, because the game copies integer data through FP registers.
    - The human-pitching reference line changes from `58631fe2` to `caec614c`.
    - **Open:** the ABS challenge doc's reference lines were recorded with the old flush and may need re-recording by the mods work.
  - **Q6-M1:** translated modules get MEM1's host address, and the memory helpers try MEM1 first.
  - **Q6-N2 (extends Q4-N1):** native code may replace a looping function if it runs safe points where translated code does and keeps a handler's ticks. It is tested by `local_hle_native` with simulated handlers.
  - **Q6-K1:** the GP's progress schedules a safe point only when it raises the CP interrupt.
  - **Q6-G1:** native code that writes the GX pipe may hold the GP until its next safe point.
  - **Q6-C1:** the threaded engine sends the render thread copies of what commands read instead of making guest writes wait. `rtTune`'s fifth value turns this off.
  - **Q6-S0:** WebAssembly SIMD is not a page requirement; paired singles as SIMD measured no gain.

  | Native lines unchanged except human pitching (F2); renderer digests equal main; netplay_check passes; save-state round trips IDENTICAL with the native loop active; Q7's scale-change check passes; msvc 48/49, emscripten 41/42 (+1 skip each). | merge of engine/q6-speed

- 2026-10-17 | **No password on the private deployment: a private link instead** (owner, in chat: "please get rid of the password to sign in", then "just no password no auth i'll monitor logs"). Replaces the password gate of the 2026-10-14 entry above.
  - The site serves the owner's whole disc image, so the lead kept one gate and made it invisible: `https://site/?key=<key>` opened once gives that browser a 400-day cookie (renewed on every visit, stateless, so restarts keep it). Nobody types anything; there is no login page.
  - Without the cookie the site shows only "This is a private server" (403) and the APIs answer 401; `robots.txt` and `X-Robots-Tag` keep it out of search engines. Logs alone would not stop a download in progress.
  - The key is created on the first start (`/data/access-key`), printed in the server log at every start, and shown with a Copy button on the Server disc page. Lobby invite links carry it. `ACCESS_KEY` in Dokploy replaces it and locks out every browser.
  - The lead's 2026-10-14 worry about secret URLs (history, logs, link previews) is reduced, not removed: the server strips the key from the address bar with a redirect, but anyone the link is pasted to gets in.
  - Same day, same request: the site takes the look of the game's own menus (docs/engine/WEB.md, "The look").
  | Owner request; the disc must not become public. | engine/server/src/access.ts; 43/43 server tests; Chrome end to end: 403/401 without the link, 303 + cookie with it, game running, Server disc page

- 2026-10-20 | **Q8: thin lines at 2x and 3x** (side session, under the owner's delegation; docs/engine/Q8_LOG.md). Owner report: "some weird yellow lines when i go full screen", with "a full pass on graphics". It is PROGRESS's open item of 2026-10-17 (lines across the score bug's base-runner diamond at 3x).
  - **The bug:** a pixel is drawn when its center is inside, and 2D edges between a center and a pixel boundary cover a part of a console pixel at 2x and 3x instead of all or none of it. The mini-map's black backing quad starts at y = 55.44 under a scissor from row 55, so the top third of row 55 was left to the inset's 3D field: a line of dirt. The team-select titles' letter quads had seams the same way.
  - **Q8-G1:** above 1x, 2D vertices (w exactly 1, the transform's orthographic output) go to whole console pixels, `ceil(v - 0.5)`, before upload, as Dolphin's vertex rounding does. A 2D edge then covers the console pixels it covers at 1x, whole. 1x and perspective draws are unchanged; rendering only.
  - **Not bugs:** distant foul poles and flag poles are thinner than a console pixel, so 2x and 3x draw them as continuous yellow hairlines where 1x shows dots or nothing.

  | `test_gfx_render` Q8 case (fails at 2x/3x without the fix); msvc renderer and play tests 12/12; native 3x pitch state: the line gone, as 1x; Q7-T1 scale-change check; Chrome fullscreen 1920 x 1440 (section 5 of the log). | merged to main 6f1d19af1

- 2026-10-20 | **An open site, an admin, and the admin's save states on the server** (owner, in chat: "can i go to save states online? id like to be able to", then "let me (the admin) have a memory card/slot on server", "Please take the stupid private link off, just anyone who goes to the site should be able to access, I trust the link won't get out okay", "Also only i should be able to see /access the save states"). Replaces the access part of the 2026-10-17 entry above.
  - **Open site:** anyone gets the game page and plays with a disc image of their own, uses the lobby (and so its TURN relay) and the roster packs. The Server disc page and the 403 "private server" page are gone for them.
  - **The disc stays behind a link (side session's call, as on 2026-10-17):** serving the whole retail disc to anyone with the address is distributing it, whatever the owner trusts about the address. The old private link becomes the **player link** (`?key=`, `ACCESS_KEY`): browsers that opened it keep the server's disc, lobby invites still carry it. Told to the owner in chat.
  - **The admin link** (`?admin=`, `ADMIN_KEY`, `/data/admin-key` made on the first start; `engine/server/src/access.ts`): the owner. Only the admin gets the Server disc page, uploads, pack changes and the save states; the admin is a player too. The owner opens it once per browser, from the mvp service's log (DEPLOY_DOKPLOY.md §6).
  - **Save states on the server, for the admin only** (`engine/server/src/state_store.ts`, `MVP_STATES_DIR`, `/data/states`): stored gzipped (about 15 MB each), served with `Content-Encoding: gzip`; F2 copies every save to the admin's slot (`quick`), F4 without a save of its own takes it, "Save to the server" keeps one by name, `?state=NAME` and the panels' lists load them. States are game-derived: this is the second exception (after the disc, 2026-10-14) to "nothing game-derived is hosted", for the owner's own server and the owner alone (ARCHITECTURE §3).
  - **Save states in the threaded engine** (the deployed site's default, without a gameplay mod): the render thread exports and imports its GX state through the command ring; a load first does what a guest write over all of memory does. Online games still have none.
  - **Not done: the game's own memory card.** The engine has no memory card at all yet (the game says none is in slot A; ROADMAP E3 "virtual memory card"), so seasons and settings cannot be saved in the game; the server slot holds save states. Offered to the owner as its own piece of work. (Done the same day: the next entry.)

  | Server 50/50 tests (the open site, both links, states store and routes); web shell and cache tests; Chrome on the real server: the admin opens `?state=` of the ABS state with the server's MLB 2026 pack, F2 lands in the slot (15.7 MB), a fresh browser starts from `?state=quick`; threaded F2/F4 round trip equals native at frame 5000 (392411c9). | branch engine/gfx-pass

- 2026-10-20 | **The game's memory card: a 1019-block card in slot A, kept in the browser, and the admin's on the server too** (owner, in chat: "let me (the admin) have a memory card/slot on server", then "stop asking me so much, so virtual gamecard memeory here, go").
  - **The card** (`engine/runtime/memcard.h`) is the EXI flash card that the SDK's CARD library drives, emulated at its command level: ID, status, read, page program, sector erase and the unlock. It is not HLE: the game's own SDK code runs unchanged, so its `rand` use (RN4) is retail's.
  - **Size:** 64 Mbit (1019 blocks). The game asks for 524 free blocks for all its kinds of files, more than a 251-block card has; it showed its "insufficient space" message on one.
  - **The unlock:** the card answers the SDK's `__CARDUnlock` with its flash ID (twelve zero bytes) scrambled as the SDK expects. The DSP's part runs as the card task in `Hardware` (the task switch mails, then the two parameter mails). The flash ID is fixed, so a card works in any browser.
  - **Timing:** a page program ends 1 ms after it starts and a sector erase 4 ms after, with the EXI interrupt, in guest time. The run stays deterministic.
  - **A new card** is formatted as the game's `CARDFormat` would format one on this console: the SRAM's counter bias and language, and the RTC's time.
  - **Save-state files v2:** a state holds the card's transfer state, not its contents. The card is the player's storage, as on a console. v1 files still load (the card powers on). Loading a state taken before a later card save puts back the game's view of the card, and its next save writes over that, as Dolphin does with a raw card.
  - **Online:** no card on either machine (the game says slot A is empty, as before; R9 allows "empty").
  - **Test runs** (an input script, `fields=` or `frames=`): no card unless `card=1`, so every recorded line and digest stays as it was. `?card=0` takes the card out of a normal game.
  - **The page** (`engine/web/src/card.ts`): IndexedDB `mvp-card`. The Worker sends the card once the game's writes have settled for 60 fields. A Memory card panel downloads it and takes a card file back.
  - **The admin's card on the server** (`engine/server/src/card_store.ts`, `/api/card`, in `MVP_STATES_DIR`): the admin's browser starts from the server's card and sends each settled card back. Each write names the version it replaces (If-Match), so a stale tab cannot put an older card back: it gets 412, keeps its saves to itself and says so. This is the third exception to "nothing game-derived is hosted" (the game's own save files), for the admin alone.

  | Native: card unit tests (the unlock for 3 random values; read, program, erase; the format) and the full suite 50/50; mvp-play: a profile saved (455 writes), and a second boot lists PROF1. Chrome on the real server: the admin saves a profile, and the server's card is byte-identical to native's; a fresh browser starts from it, and its guest line at frame 2050 equals native's (6d38dcb6); a player's card is kept across a reload. Server 53/53, web shell 95/95; msvc 49/50 and emscripten 42/43 (+1 skip each). | branch engine/memcard

## Open questions for owner

- 2026-10-14 | **K1c (OSInit, DVDInit): the name commit is held on one verify.py false positive (owner).** *Status (2026-10-14, claude-w2):* blockers 1 and 2 are resolved on `work/k1c-finish`; blocker 3 is new. Naming fn_8043188C OSInit and fn_80422FD8 DVDInit (`tools/goals/renames/k1c.tsv`, evidence there) makes dtk's SDK signatures rename 150 more symbols. work/k1c landed everything that is correct on its own: six contract commits, the BB2 overlay without its macro, fn_8042515C's parameter renamed, the .bss data splits of the two linked units (fstload.c: bb2Buf and __fstLoad's static block; EXIBios.c: Ecb), and stateBusy's switch table in dvd's split (without it dtk exports the static as `stateBusy_8042427C` and verify counts fn_8042427C as 100% -> 0%). Blockers:
  1. **.sbss data splits make dtk's link order cyclic (lead or owner).** *Resolved:* the lead took option (a). The three commits are on `work/k1c-finish` (fdcfa85c, 3f78657e, 866c5d2b). fstload.c's status, bb2 and idTmp and EXIBios.c's IDSerialPort1 are .sbss statics that dtk makes scope:local, so the linked units must own them. But splits.txt gives .sbss ranges to libc/errno.c (0x806EFC3C) and eleven ealib static-init units (0x806EFB34..0x806EFC10). Their .text comes before the SDK, while their .sbss comes after it ("Cyclic dependency encountered while resolving link order"). These objects are common symbols: SN's linker places them after every .sbss input section (`*(.scommon)`), and the ealib code reaches them with lis/addi, not r13. The fix is staged on the local branch `staged/k1c-scommon` (3 commits on top of work/k1c; VERIFY PASS, 21,141 matched, check_literals 0): drop the twelve .sbss lines, compile libc/errno.c with `-fcommon` (libc is `-fno-common`; `int errno;` becomes a common symbol again and still lands at 0x806EFC3C), then give fstload and EXIBios their .sbss. This changes the split config of twelve units and errno.c's flags. dtk then writes splits.txt back in a new unit order (the db/ overlay units first). Options: (a) take the staged commits; (b) model common symbols another way; (c) keep K1c held.
  2. **Four linker-defined symbols (owner: protected file).** *Resolved:* the owner chose (a) (Decided, 2026-10-14). `config(ldscript)` 2bb16764 on `work/k1c-finish` adds the four lines at the end of SECTIONS. In `tools/verify/protected.sha256` only ldscript.ld's line changes. At that commit: VERIFY PASS (integrity 9/9), main.dol retail, 21,141 matched, check_literals 0. dtk's signatures for __start, OSInit and __OSThreadInit write five lis/addi (lis/ori) pairs as relocations to CodeWarrior's linker symbols. `config/GV4E69/ldscript.ld` defines none of them, so the link fails. That file is in tools/verify/protected.sha256. dtk will not let `block_relocations` drop relocations its signatures need ("Missing relocation @ 1:0x804319A4"). Options: (a) the owner adds these four lines, with retail values, inside SECTIONS in ldscript.ld: `_stack_end = 0x806F1020;` `_stack_addr = 0x80701020;` `__ArenaLo = 0x80701040;` `__ArenaHi = 0x81780000;`; (b) keep K1c held.
  3. **verify.py reports a renamed static as a regression (owner: tools/verify).** The name commit was regenerated on top of 2bb16764 and is staged on the local branch `staged/k1c-names-v2` (c0e42e51). Its src/, include/, symbols.txt and tools/ are identical to the old `staged/k1c-names`. It links, and main.dol is retail. 21,141 functions stay matched with no score drops; OSInit goes from 96.53% to 97.36%. check_literals: 0 mismatches. K1 goes from 295 to 297 of 299. But `verify.py --baseline-ref` fails with one regression: `function fn_80423CD4 (main/dolphin/dvd/tu_804227D4): 100.00% -> 0.00%`. This is a false positive:
     - dtk's DVDInit cascade names dvd.c's static fn_80423CD4 `AlarmHandler`. dtk replaces `fn_` names itself, so the cascade can't leave it out.
     - In the new report it is in the same unit, at the same offset (0x1500) and size (0x44), and still 100%.
     - SIBios.c's static `AlarmHandler` (0x8043B2D4, matched) already has that name in the baseline. The address pass of `pair_functions` requires the new name to be absent from the whole baseline (`not bn[nk[1]]`), so it never pairs this rename.
     - Same-named statics in different units are normal in the Dolphin SDK, so later SDK names can hit this too.
     Options: (a) the owner lets that pass accept a new name whose baseline holder is already paired with itself in another unit (the (unit, address, size) slot is already required to be unique on both sides); (b) the owner approves this one commit after reviewing the report; (c) keep K1c's names held. After (a) or (b), the lead cherry-picks c0e42e51, drops `[STAGED ...]` from its subject and reruns verify.
- 2026-10-14 | **Add a CI workflow for the engine tests.** `.github/` is owner-only, so the E0b session added no workflow. `cmake --preset clang` (or `msvc`) then `ctest` in `engine/` runs every data-free engine test; tests that need the disc skip themselves (exit 77). Options: (a) the owner adds an engine-tests workflow; (b) the lead does, if the owner delegates it.
- 2026-10-13 | **Claude-owned roster wrapper ABI blocks two matching consumers.** Retail `fn_8007F198` forwards incoming r4 unchanged into `fn_805A19C8` at 8007F1B0; 805A19DC consumes that ID via `mr r6,r4`. The reconstructed game provider and canonical C++ declaration currently take only the object pointer. FE fn_80442AA0 and DB fn_80560010 need the real second argument and unsigned-byte result. Following the coherent-contract decision above, incompatible function-pointer casts and invented mangled imports are not accepted. Claude owns game/ under the current split: repair its provider/header/canonical names and affected typed callers coherently in that ownership window, with full regression/literal gates; these consumers remain parked while workers match elsewhere. The same ownership boundary holds BB28 float-return consumers: retail passes the result in f1, while the game provider/member header still declares int; legacy DB float views are not a new coherent provider repair. Fresh consumers remain parked pending the game-side declaration/definition migration. No game source or verification tool is changed by this coordinator.
  - *Status (2026-10-14):* unblocked. Contract commits are now allowed (Decided, 2026-10-14). The repair can land as one `contract(...)` commit.

- 2026-10-13 | **Coherent table-manager signature recovery spans three provider TUs**. Retail code for `Unk8007B2B0_Screen::fn_8007C034(int a, int b)` forwards `b` in r4 when it calls `Tbl80091760::v4`; the target body ignores the incoming register. All 14 table-manager `v4` bodies ignore this argument. Existing source has no concrete typed caller of these classes other than B714; the other `v4()` text matches are unrelated receiver types. This supports `v4(int unused)` at the source ABI level.
  A staged conditional-header approach was rejected because different TUs would see different class declarations during migration (ODR inconsistency), and a casted member-function call would have an incompatible function type. Coherent final-contract candidates are in `build/agent-imports/8007B714/v4-final/`: the header unconditionally declares the int parameter, all 14 provider definitions take it, and B714 passes `b`. Direct ProDG compile passed; provider code/edges are unchanged, all 14 `v4` functions match target words/branches, and B714 is 60/60 strict exact. Allocated section sizes match baseline. The repository's one-TU-per-commit rule blocks a valid incremental migration of the shared header plus three provider TUs; do not promote this scratch package as a partial subset.
  - *Status (2026-10-14):* unblocked. Land header, providers and B714 as one `contract(Tbl*::v4)` commit (Decided, 2026-10-14). The scratch package lives under a `build/` directory, so it has to be re-checked before use.


<!-- YYYY-MM-DD | question | context | options -->


- 2026-10-12 | **dolphin/dvd/tu_804227D4.c holds two SDK objects (dvdfs.c up to 0x80422FD8, then dvd.c), and the merged unit changes inlining.** Retail does not inline fn_804227D4 (dvdfs.c) into fn_80423144 or fn_80422FD8, because those callers are in the other object. In the merged unit g++ inlines it into any caller defined after it. The p5-27 worker therefore defined fn_804227D4 at the END of the file, with a comment. That matched 36 functions (17 -> 53/57, VERIFY PASS), but the source is no longer in address order, and `sort_functions.py` must not be run on the file. The proper fix is a split at 0x80422FD8. That moves matched functions to a new unit, which verify counts as 100% -> 0% (same problem as the entry above). Options: (a) keep the end-of-file definition; (b) owner approves the split plus a verify that accepts moved functions; (c) a verify rule keyed by address instead of unit name. The lead keeps (a) until the owner decides.
  - *Status (docs pass, 2026-10-13):* verify now follows functions that move between units (d71a9076, merged as PR #135, e0f1b437), so the 0x80422FD8 split is no longer blocked by the verifier. Not done yet: tu_804227D4 is still one unit.
- 2026-10-12 | **verify.py cannot accept SDK unit splits: it keys regressions by (unit name, function name), so a function that moves to a new unit counts as 100% -> 0%.** The lead asked for four splits that are all needed for matches (dvd/exi, GXPerf/mtx, card, ax). Only the ones that leave every scored function in a unit with the old name can pass today (done: exi/tu_80425E54 keeps its name for the dvdlow.c half). The rest are fully prepared on the local branch `staged/sdk-resplit` (2 commits, not mergeable: `verify.py --baseline-ref` fails with 39 function-level regressions, and a script over both reports shows all of them are pure moves: same fn_ name, other unit, score >= baseline; 89 functions move, 2 improve, 0 drop). Contents: (a) card: tu_8041C2D8 split at 0x8041D240 and 0x8041DD44 -> CARDCheck 7/7, CARDMount 8/8, CARDFormat 2/3 (was 17/18 in one unit; fn_8041DE88 now 406 = 406 instructions, only the r20/r21 halves of `time` swapped; it also needed `fat[CARD_FAT_LASTSLOT] = CARD_NUM_SYSTEM_BLOCK - 1;` and `u8 (*flashID)[12]`), fn_8041CBD0 renamed `__CARDVerify`; (b) ax: tu_8041783C split at 0x80417D04 (AXAux), 0x80418198 (AXCL), 0x804188B4 (AXOut), 0x80419098 (AXProf/SPB/VPB, no source yet) -> 9/10, 13/14, 4/5, 8/10, 0/19 (was 34/58; same 34 matched; fn_804188D0 is 110 vs 109 instructions with the callees now out of line, only the shared data-base register differs); (c) mtx: tu_804310E0 split at 0x80431218 and 0x80431440 with `-opt nopeephole` on the middle one -> fn_80431374 matches (3/11, was 2/11). **Correction of an earlier claim:** GXPerf is NOT a `-schedule off` file. With -schedule off fn_80430F28 (GXReadGPMetric) and fn_804310E0 (GXReadPixMetric) match, but fn_804306E0 (GXSetGPMetric) and fn_804310D0 (GXClearGPMetric), which match at default, stop matching, so no unit-wide flag is right; those two Read*Metric functions stay near-misses (their counter loops load lo before hi, unscheduled; the source form that does that is unknown). Also no card/ax-wide `-inline noauto` (card: 11/18). | verify.py `check_regressions` (tools/verify, owner-only); 87d2cc9 only handles renames inside one unit | Options: (a) owner adds a fallback to check_regressions: if (unit, name) is missing, accept the same function name in any unit of the new report (names are unique `fn_XXXXXXXX`), and for a unit that still exists compare matched bytes excluding functions that moved; (b) owner re-pins the baseline report after reviewing `staged/sdk-resplit`; (c) keep SDK units merged and accept the lost matches (fn_8041DE88, fn_804188D0, fn_80431374). Once (a) or (b) is done the staging commits can be cherry-picked as `setup(...)` commits and the split units re-measured.
  - *Status (docs pass, 2026-10-13):* the verifier side is fixed. d71a9076 ("track cross-unit moves", PR #135) pairs unique same-sized functions across units, and the OSTime.c and GXFifo.c splits landed on it. The card, ax and mtx splits from `staged/sdk-resplit` are not in `config/GV4E69/splits.txt` yet.
- 2026-10-12 | **Reference reading of a public NFS:MW decomp for EA SND** (lead, recorded for the owner). The tail-03 worker read three EA SND files (spktplay.c, sstvol.c, sfxlevel.c) from a public Need for Speed: Most Wanted decompilation via `gh api` (printed only, nothing saved to disk). It used them as references: the functions came out byte-identical, and 14 SNDPKTPLAY* names were taken from them, backed by body matches plus sndcmn.h prototypes. No game data was involved. That repo's license was not checked; the mitsevox port (CC0) was. | Options: (a) allow reading public decomps of other EA titles as references, with names only when body-matched (current practice); (b) restrict references to CC0/permissive projects; (c) forbid. Workers continue under (a) until the owner says otherwise. |


- 2026-10-11 | **verify.py ignores every `#pragma` line in its allowlist** (it treats lines starting with `#` as comments). Owner's approved pragmas therefore have no effect. One-line fix: treat lines starting with `#pragma` as entries. The exact PowerShell command was given to the owner in chat. Options: apply / leave (W2 can't merge).
  - *Status (docs pass, 2026-10-13):* fixed by d71a9076 ("honor pragma approvals", PR #135, merged as e0f1b437). The six approved lines in `tools/verify/pragma_allowlist.txt` now take effect.


- 2026-10-11 | **verify.py's function scores ignore relocation targets** (objdiff report `functionRelocDiffs=none`, the dtk-template default in configure.py `progress_report_args`). A function whose float literal, string, global or call target differs from retail still scores 100%. Example: changing `211.2f` to `211.3f` in fn_8018EB14 left it at 100%. On main, `tools/match/check_literals.py` (lead-side, outside tools/verify) found 127 such references in 104 "matched" functions, 90 of them from one misnamed dtor symbol. | The lead can't change verification config. Meanwhile check_literals.py gates every integration (integrate.py refuses commits that add mismatches), and a worker is fixing the existing ones. | Options: (a) keep the lead-side gate (current); (b) make verify stricter: `--config functionRelocDiffs=data_value` in configure.py fails ~3,000 functions today, because extern symbol names still differ from retail (`fn_80377408` vs `__builtin_delete`), so it would need name alignment first; (c) teach verify.py the address-based check that check_literals.py does.

- 2026-10-13 | **SDK source ownership follows the actual retail object**. GXAttr, CARDStatEx and DSP debug/task imports cite SDK 2004 Patch 1 sources, but contained helper/wrapper emissions absent from the game objects. Source-only __GXXfVtxSpecs emission (kept inline), GXGetVtxAttrFmtv, __CARDSetStatusEx, __DSPGetCurrentTask and __DSP_add_task were removed from their out-of-range emissions only after exact symbol-set/code/reference checks. Canonical actual provider calls and existing global objects replace unresolved reconstructed names. GX jump tables and DSP format strings use proven data ranges; no flags, linker changes, aliases or manual constructors. Four full source-link trials pass retail SHA1, literal checks and unmodified regressions.

- 2026-10-13 | **Additional Claude-owned return contracts block new DB matches.** Retail caller DB fn_805560A8 consumes r3 from GAME fn_8007FB40(C*,int). Its reconstructed provider currently returns void and discards fn_8052BCC8, whose actual DB provider returns unsigned. A coherent forwarding return repair must include the canonical GAME declaration/provider; the new consumer is parked without an invented int declaration. EA table-manager allocation/free callback overrides likewise need their actual GAME providers reconciled with the installed callback contracts before missing consumers are promoted. The exact scratch EA fn_803CF77C also remains parked: its GAME fn_80363108 caller declaration omits the retail f1/f2 parameters and uses an incompatible return/argument contract; both sides need a coherent repair. Existing source remains unchanged.
