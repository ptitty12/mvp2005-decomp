# PROGRESS

## Snapshot (update each session)
- Direction (owner, 2026-10-14): **browser play, browser netplay, easy roster/team mods**, on a foundation without tech debt. Plan: `docs/ROADMAP.md`. Engine track: not started, E0 next. Knowledge track: the decomp, prioritized by targets K1-K5 (`tools/goals/targets.py`; K1 48%, K2 7%, K3 0%, K4 1% done on 2026-10-14).
- Target: MVP Baseball **2005**, GameCube `GV4E69` (USA, rev 0). `main.dol` SHA-1 `da6becdbea614d03c4b5eae9f8a0fb08e0a184dd`
- Build: `python configure.py && ninja` rebuilds a byte-identical `build/GV4E69/main.dol` from compiled source plus remaining dtk-split assembly (24,158 functions, 5,896,176 code bytes).
- Toolchain:
  - game + EA libs: SN ProDG 3.9.3, `GNU C++ 2.95.3 SN BUILD v1.76`. Flags by library/unit are recorded in `configure.py` and `docs/DECISIONS.md`.
  - Dolphin SDK: MWCC GC/1.2.5n and GC/2.0, with evidence-backed per-library/unit flags.
  - linker: SN ngcld 1.5.1.692 + `config/GV4E69/ldscript.ld`; DOL packed by `tools/mkdol.py --layout`
- As of main `68d25430` (2026-10-13 22:53 CDT, the progress bot's verified build):
  - Functions matched: **21,104 / 24,158 (87.36%)**; code matched: **3,073,968 / 5,896,176 bytes (52.13%)**.
  - Source-linked: **234 of 847 units, 386,148 code bytes (6.55%)**, plus 13,826 data bytes. Matched functions in unlinked units still use extracted assembly in the final DOL.
  - Left: 3,054 functions / 2.82 MB. The 853 functions over 1 KB hold 1.86 MB of it, and 507 functions sit at 95-99.99%.
- Last audit: 2026-10-13, retail DOL hash + literal checks + unmodified verifier PASS; per-commit integration gate required before push.

## Session log (newest last)
<!-- YYYY-MM-DD | agent/model | unit | matched N, abandoned M | blockers | owner decisions needed -->
- 2026-10-10 | Claude Opus 5.5 | Phase 0 recon (no unit) | matched 0, abandoned 0 | Blocked on owner approval. Disc is 2005, not 2004. Toolchain is ProDG GCC 2.95.3 rather than CodeWarrior. No symbols; `/mvp.elf` is stripped. Missing on host: ninja, Ghidra, JDK, REA; `gh` not logged in. | Decide: target game, path (`prodg-hybrid` recommended), host (native Windows recommended), Phase 1 tool downloads, GitHub remote
- 2026-10-10 | Claude Opus 5.5 | Phase 1 scaffold (no unit) | matched 0 committed (3 probe functions 100% in a scratch clone), abandoned 0 | None blocking. Notes:
  - Upstream dtk-template can't build this game, so a ProDG pipeline was added (analysis DOL from `/mvp.elf`, ngcld + static linker script, mkdol layout packing).
  - Verified from a clean clone of `f91c043`: `main.dol: OK`. Final check from a clean clone of `4a1d8de`, with the staged verifier installed as the owner would: `VERIFY PASS` (integrity, hard rules, retail hash, regressions vs HEAD~1).
  - The verifier/CI is staged in `proposed/`, because the agent is denied the target paths. It was tested with 6 pass/fail cases.
  - GitHub issues were not created; that needs the owner's OK.
  - Ghidra/JDK/REA are still not installed (not needed yet).
  | Decide:
  - install the verifier;
  - CI hosting (self-hosted runner recommended);
  - commit the probes?;
  - original-asm SDK functions policy;
  - push + create unit issues;
  - review the Phase 1 pipeline choices.
- 2026-10-10 | Claude Sonnet 5.5 (W3) | ealib/tu_803663B0, 803666EC ... 8036E65C (12 units) | matched 24 funcs (12x __static_initialization_and_destruction_0 + 12x _GLOBAL_.I), setup fns (0x2b0-0xc40) not attempted | none | none
- 2026-10-10 | W4 (Sonnet 5.5) | ealib/tu_8036FEA0 .. tu_80376598 (issues #66-#76) | matched 22 funcs (11x static-init pair), 0 abandoned; 44 large funcs (0x224-0x8x0 param-table ctors/creators) not attempted | none | none
- 2026-10-10 | W5 (Claude Sonnet 5.5) | ealibs/tu_803D9FE8.cpp (#91), ealibs/tu_803D7ED4.cpp (#88/#89 range) | matched 20 (5/6 and 15/16), parked 2 (fn_803DA038 mr r11 near-miss after 6 variants; fn_803D8094 not attempted) | none | EA lib flags -Os -G0 provisional
- 2026-10-10 | Claude Sonnet 5.5 (W1) | dolphin/ai/AI.c (#108) | matched 6 (152 B), attempted 12, parked 6 (AIInitDMA, AIStartDMA, AIStopDMA, AISetStreamPlayState, AISetStreamVolLeft/Right: RMW on constant register base does not reproduce lis+addi base) | unmatched also: AIInit, AISetDSPSampleRate, AISetStreamSampleRate, __AI_SRC_INIT (large), 3 handlers (not attempted) | none
- 2026-10-11 | Claude Opus 5.5 (lead) + Sonnet workers | choosers (tu_80271748/940/C64 linked; tu_80271F14, tu_80272B20), swarm integration, team pipeline | main: 46 -> 142 functions at 100% (W1/W3/W4/W5 merged 74; cCircleChooser 10/11; cLinearChooser 12/13; cEvent/cHand/cPitch 18 linked earlier), every pushed merge verified per commit | Notes:
  - Owner approved the team plan (Opus lead, Sonnet workers; 7 running at session end) and gave the lead full authority; CLAUDE.md now says the owner never runs commands.
  - Flags: game code += -ffloat-store; EA libs switched -Os -> -O2 on W4's evidence (DECISIONS).
  - Found: the animation choosers/goals region is one unity TU (DECISIONS); helper-class units are code-matched but can't link standalone.
  - MISTAKE: cc5d94b (cMatrixChooser setup) reached main with a failing build: the push was gated on a pipe's exit code, not verify.py's. Fixed forward in 9f743e2 (verified against f065b4a). Since then every lead push goes through `tools/team/integrate.py --push` (lead mode), which gates on verify.py per commit.
  - Parked: see docs/NEARMISS.md (cCircleChooser slot 17 at 97.9%, others).
  | Owner: verify.py reads every `#pragma` allowlist entry as a comment (DECISIONS open question). That one-line fix is in tools/verify, which only the owner can edit; W2's 80 SDK functions wait on it.
- 2026-10-11 | Claude Opus 5.5 (lead) + up to 12 Sonnet/Opus workers | checkpoint | main: 341 -> 2052 functions at 100% since the last entry; every merge verified per commit (skeleton-only setup batches verified at HEAD) | Notes:
  - All main game .text (0x80003100..0x80340000) is now in units: real TUs where boundaries are known (7 early TUs), code-only units elsewhere (data extern). Unity-TU regions (animation choosers/goals, 0x8000F598..0x8006902C) are split per placeholder file.
  - Families found: trace wrappers (2 families, ~800 functions, generated), event/chooser/warp class boilerplate, static-dtor (__tcf_N) stubs, setter families. Recipes in docs/PATTERNS.md.
  - Hard idioms cracked: `>?` clamps, Vec3 copies inside inlines, global/local-alloc priority (docs/PATTERNS.md).
  - Incidents: a merge tool bug (stale pad name) and duplicate global __tcf_N names each broke the link checksum before push; both caught by the gate, fixed (tools updated), nothing bad reached main.
  - Not linkable yet: unity-TU parts; real TUs pending linkonce/rodata-pool questions (DECISIONS).
  | Owner: verify.py allowlist bug still open (DECISIONS); W2 (80 SDK functions) waits on it.
- 2026-10-11 | Claude Opus 5.5 (hard-case matcher, work/opus-families) | game/tu_8007FF8C, tu_80084910, tu_80089894, tu_8008CE64, tu_80037250 | matched 25: the vcall family (10, pass-through arguments to fn_80071E2C), fn_80080374, fn_800393B0 and fn_80081BA8 (same mechanism), and the CSV loader family (12, pointer locals) | Trace wrappers family B with K != 0 and N == 0 (17 functions) stay open: mechanism found (sched1 adjust_priority), but no zero-cost source; see docs/NEARMISS.md | none
- 2026-10-11 | Claude Opus 5.5 (lead) + 12 Sonnet + up to 3 Opus workers | checkpoint | main: 3,507 -> 7,750 functions at 100% (32% of all 24,158; 13.7% of code bytes) since the last checkpoint. Ranges 00-18 integrated; 19-25 running; second passes of 03/04/05/13 started.
  - Opus families rounds 1-2: vcall family, CSV loaders, roster walk, passthrough args, callee return type and sched2, cr7 scc (docs/PATTERNS.md "Hard-case families" 1 and 2).
  - Found a verifier gap: function scores ignore relocation targets (DECISIONS open question). New `tools/match/check_literals.py` and an integrate.py gate; 104 false matches on main are being fixed (work/false-matches).
  - New lead tools: tools/match/passthru.py and check_literals.py; worker brief has quick tips.
  - Next: ~10,400 functions sit in unsplit regions (0x803401A8-0x803663B0, .feoverlay, .dboverlay, EA/SDK tail). An Opus setup delegate is splitting them into code-only units.
  - Incidents: a usage limit cut off all workers once (resumed from their transcripts, nothing lost); the progress bot races lead doc pushes (rebase and rerun).
  | Owner: two open questions in DECISIONS (verify.py pragma allowlist; relocation-target checking). Neither blocks the team.

### 2026-10-11 checkpoint (lead, after range-33 / Opus families 3)
- main: 13,362 functions at 100% (of 24,158; 55%); code bytes about 23%. The remaining work is dominated by large functions.
- Merged this stretch:
  - game ranges 28, 29, 30 and pass2-04;
  - fe ranges 33-36;
  - db ranges 41 and 42 (first 3 units);
  - clone.py (repo-wide twin cloner, about 150 clones);
  - Opus families 3: two-vptr widget ctors solved, plus the callee `void*` return rule.
- Running:
  - fe ranges 37-40;
  - db ranges 42 (units 4-7) and 43-46;
  - pass-2 groups g16 and g17;
  - Opus families 4 (dead-compare switches, frame-offset locals, callee-saved permutations);
  - Opus tail setup (split about 3,500 EA-lib/SDK functions; flags survey; tu_80362FB4 -ffloat-store question).
- Queued: 31 pass-2 groups (about 120 small unmatched functions each), then large-function strategy, then the tail ranges.
- Blockers, owner-only (recorded in DECISIONS): the verify.py `#pragma` allowlist bug, which blocks W2's SDK branch; relocation-blind scoring, covered by the check_literals gate.
- Known unresolved names, not mismatches:
  - fn_803567AC: tu_80134A60 declares it as a free function; it is now `__11Unk803567AC`.
  - fn_8037B424, now `_._11Unk8037B3E8`.
  - fn_80090880: callers pass 2 arguments; use the cast recipe.
- 2026-10-11 | Claude Opus 5.5 (tail-setup delegate, work/tail-setup) | main .text tail 0x8036F44C..0x8043FDB8 | setup only: 254 code-only units (95 EA-lib, 103 runtime, 56 SDK), 0 functions matched; tu_80362FB4/tu_80365C10 moved to cflags_ealib (fn_8036344C exact there) | none | Flags per library in DECISIONS (tail entry); unconfirmed: AV, libgcc DI helpers, sdk/ pieces, dvd/mtx/exi deviations. 503 functions are port jobs from port/mvp2005 and match/W2-pad.
- 2026-10-12 | Claude Opus 5.5 (scheduled audit) | audit of origin/main `a45298cc` | read-only. **19,329 / 24,158 functions at 100%**: 80.0% of functions, 39.83% of code bytes. `verify.py --baseline-ref 5cb09b2b` gave VERIFY PASS. Full report: `docs/audits/2026-10-12.md`.
  - Clean: no cheating hits, no verifier or settings changes since the owner's installs, no game data in git history.
  - Checks passed: 10/10 random matched functions byte-identical with relocations masked; 25/25 commit `[N/M]` claims equal objdiff; `check_literals.py` found 0 mismatches over 69,494 references (1,348 unresolved names).
  - Issues:
    - Only 64/786 units are linked, and the verifier's score ignores relocations.
    - 3 owner-only verifier questions are pending.
    - 39 match commits mix units.
    - The PROGRESS snapshot above is stale.
    - The main checkout's `main` has diverged, with 2 unpushed progress-bot commits.
  - Progress bot: healthy since 2026-10-11 12:20 (7 checkout failures before that); history.csv is current.
- 2026-10-12 | Claude Opus 5.5 (lead session 2, from 10:28 CDT) | fleet coordination | **19,329 -> 19,681 functions at 100%** by 11:00 (81.5%; 41.8% of code bytes).
  - The first lead session ended at 10:20 and its 12 workers died with it. Their committed branches (p3-28, p4-22..30, p4-33) were all integrated (+345 functions, verify PASS each). Uncommitted leftovers are kept as `git stash` entries named `leftover work/<batch>`.
  - Lead kit committed (`tools/team/lead/`). Worker helper scripts made portable: they hardcoded other worktrees' paths and could have edited the wrong worktree. integ.sh no longer reports a `[remote rejected]` push as merged.
  - EXI split: EXIUart.c (`dolphin/exi/tu_804286AC.c`, 4/4) and the GXInit.c head (`dolphin/gx/tu_80428C64.c`, 3/3) were split off EXIBios.c.
  - `ealib/tu_80394744`: moved to cflags_ealib_sda_nsr instead of a re-split. Its last function is already matched there, so a split would be a verify regression.
  - HANDOFF section 7 items 6 (OSExceptionVector labels: none are mis-typed in symbols.txt) and 7 (relocname fixes: all 19 already 100%) needed no change. 228 merged stale branches were deleted.
  - Running: 13 Sonnet workers on p4_00..p4_12; 1 Opus worker on hard-case families 5, 7, 8, 9 (round 7). Queued: p4_13..p4_21, p4_32 (no commits last time), then p5 (`tools/team/lead/gen_p5.py`: 0% functions of 0x400-0x800, plus small ones in units no p4 batch covered).
  - Blockers: none new. The owner-only questions in DECISIONS are unchanged.
- 2026-10-12 | Claude Opus 5.5 (lead session 2, update 15:10 CDT) | fleet | **19,681 -> 20,118 functions at 100%** (83.3%) since the 11:00 entry; +789 this session.
  - All 13 p4 batches p4-00..p4-12 merged: 18-46 functions each (median about 30). A usage-limit cutoff at 10:55 stopped every worker; they were resumed at 13:15 with SendMessage and kept their context.
  - Lead fixes:
    - **p4-00 header merge:** two batches added the same three method declarations, and the build failed; fixed on the branch.
    - **p4-06 address literals:** two string-address tables were written as integer literals, because check_literals compared unrelocated pointer words. The functions now name the symbols, and check_literals resolves pointers to externs (repo-wide still 0 mismatches). The worker prompt forbids address literals.
    - **Worker helpers:** several (hill.py, rv.py, Q21, perm.py) wrote variants into the shared tools folder, and 295 such files were committed by mistake, then removed. They now write to $QDIR, and rv.py --apply keeps CRLF.
  - Running (13 Sonnet workers): p4-13..p4-21, p4-32b (Dolphin OS/VM, a rerun), p5-18, p5-19, p5-22. The Opus hard-case round 7 is held. The pass-5 queue (28 batches, 0% functions up to 0x800) is ordered in tools/team/lead/queue/p5_order.txt.
  - Open families from reports:
    - SumRow byte-sum loop temp registers (fn_80166E40, fn_800493D4, fn_80118770, fn_801609EC);
    - the inlined stack Unk803597C4 record ctor prologue (fn_80163390 and 3 more);
    - fn_80097318 plus 4 siblings (one shape).
  - Blockers: none new. Functions that need `mftb` stay unmatched (inline-asm-only, existing rule).
- 2026-10-12 | Claude Opus 5.5 (lead session 2, evening, 17:52-20:15 CDT) | fleet | **20,341 -> 20,582 functions at 100%** (85.2%); +241 since the 16:00 stop.
  - The fleet restarted at 17:52 when the usage window reset: 13 fresh Sonnet workers on p5 batches (0% functions up to 0x800). Every branch was merged with VERIFY PASS: p5-02 (+14), 03 (+11), 04 (+4), 08 (+19), 09 (+36), 11 (+30), 13 (+21), 14 (+20), 16 (+11), 17 (+19), 20 (+10), 21 (+13), 23 (+12), 24 (+13), 26 (+8). About 40 near-misses (95-99.99%) were committed with notes.
  - Pass 6 (`tools/team/lead/gen_p6.py`, 46 batches): every 0% function up to 0x800 in free units. Units of the p4 batches cut short at 16:00 come first. Running: p5-00/01/05/06/07/10/12/25/27 and p6-00..03.
  - Patterns: PATTERNS.md "Pass 5" section. Notably, an object with an empty inline destructor reserves an unreferenced stack slot, which explains many spare-frame-byte near-misses.
  - Incident: one worker killed every python.exe on the machine (taskkill /IM). Workers re-ran their jobs; the prompts forbid it now.
  - Walls (unchanged rules): `mftb`/`mtspr`, paired-single (psq_) kernels, a literal data word after `bl`, and g++ virtual-base destructor machinery are not attempted.
  - Blockers: none new. The owner-only questions in DECISIONS are unchanged.
- 2026-10-12 | Claude Opus 5.5 (lead session 2, update 21:40 CDT) | fleet | **20,582 -> 20,703 functions at 100%** (85.7%); +362 since 17:52.
  - p5 is finished: every p5 batch has been worked once and merged. Late merges: p5-00 (+7), 01 (+5), 05 (+5), 06 (+7), 07 (+9), 10 (+8), 12 (+11), 25 (+4), 27 (+36, the dvd.c state machine).
  - p6 so far: p6-00 (+4), 01 (+1), 02 (+4), 03 (+1), 04 (+2), 05 (+4), 06 (+8), 35 (+7). The first p6 batches (the cut-p4 game units around 0x801C-0x8022) are Vec/float-temporary heavy, with few small functions. `queue/p6_order.txt` is now ranked by the count of functions <= 0x200. The dolphin p6 batches (mostly hand-written code) are last.
  - New owner question in DECISIONS: dolphin/dvd/tu_804227D4 merges dvdfs.c and dvd.c, and fn_804227D4 is defined at the end of the file to stop inlining.
  - Tooling:
    - integ.sh rewords `wip(` near-miss commits to `match(` on the worker branch before merging (31 older `wip(` commits are already on main).
    - rv.py scores a directory, so there is no command-line length limit.
    - The p6 prompt has the near-miss commit format and the background-job rule.
  - Recurring unsolved shapes:
    - `slwi r0` vs `r11` index temps in `in->m8[id]` loops;
    - `cmpwi` before `li r3,4` in `c ? k : 4`;
    - goal-pointer copies (`mr r28,r27`) removed by cse-skip-blocks;
    - `mr r0,r3` before `cmpwi` on a call result.
  - Usage: the 5 h window was 92% used at 21:36 and the weekly limit 77%. The fleet restarts at the 22:50 window reset.

### 2026-10-12 evening (enablers: fleet dashboard, tooling only)

- Unit: none. Functions matched: 0 (tooling session).
- Added `tools/fleet_dashboard.py`. It reads git only (no build, no model calls) and reports:
  - verified and worker-claimed code and functions per hour;
  - near-miss share of commits;
  - per-worker activity, the merge backlog and sessions;
  - the functions closest to 100% in the progress bot's last build.
- `--watch` runs in a terminal. `--publish` replaces the single orphan commit on the `dashboard` branch.
  - A new scheduled task, "MVP decomp live dashboard", runs it every minute from the worktree `decompile_mvp_04-dash`. That worktree must stay.
- Pushed the two progress-bot commits that were only on the local `main` checkout (2-minute poll, rebase on a push race).
- Verify: PASS at 3738240c (20,730 functions, no regressions).
- Reading of session 3 (17:57 onward):
  - Verified code per hour held steady against session 2 (about 55-70 KB/h vs about 63 KB/h).
  - Functions per hour fell, because the remaining functions average about 860 B against about 140 B for matched ones.
  - The near-miss share of commits rose from about 4% at 18:00 to 40-60% after 21:00.
- Owner question added to DECISIONS: the GitHub repo is public.
- Later the same evening (owner approved tool downloads; lead request):
  - Added `tools/match/permute.py` and `tools/match/cxx2c.py`, which run decomp-permuter on one near-miss.
    - decomp-permuter lives at github.com/simonlindholm/decomp-permuter @8556c81, cloned in tools-ext. Its venv has toml and Levenshtein.
    - C units run on the trimmed unit.
    - C++ units run on a C rendering. Each candidate's body is spliced back into the original unit and compiled with g++.
    - Setup refuses any function whose base does not compile to the unit's own code.
  - Coverage: 51 of 60 random C++ near-misses at >= 95% set up. The refusals were constructors, static initializers and bodies the C parser can't read.
  - The lead runs a near-miss batch (queue nm_00) with it.
  - Verify: PASS at e2f3bda2 and 7dce9440.
  - Owner decisions passed on, not decided here:
    - extra usage (a cost decision);
    - a verify rule that keys regressions by address (behind the verifier lock).
- 2026-10-13 | Claude Opus 5.5 (lead session 2, final, 22:50-00:25 CDT) | fleet | **20,740 -> 20,869 functions at 100%** (86.4%). Session 2 total: 19,329 -> 20,869 (+1,540).
  - New near-miss pass with decomp-permuter tooling (tools/match/permute.py, from the enablers session, now for C and C++):
    - 7 batches in about 75 minutes: nm-00 +23 (includes the AI.c flags fix below), nm-01 +7, nm-02 +7, nm-03 +10, nm-04 +16, nm-05 +7, nm-06 +7;
    - the best yield per agent-hour of the session.
    - Nearly every match came from hand edits guided by compiler dumps (`cc1plus -dg -dl` register priorities, `-fno-gcse` A/B); permuter candidates mostly cheated (dropped calls). See PATTERNS "Near-miss pass".
  - Pass 6 final round: p6-07 +7, p6-10 +9, p6-13 +14, p6-14 +12, p6-26 +3, p6-28 +7.
  - dolphin/ai/AI: `cflags_dolphin_ai` no longer has `-opt nopeephole`, which makes every compiled AI.c function byte-identical (DECISIONS 2026-10-13).
  - Process: p6-10's branch had worker scratch files committed; the lead removed them from the branch history before merging. integ.sh now rewords `wip(` commits.
  - Stopped at 00:10 on the owner's instruction:
    - all agents stopped; leftovers stashed; orphaned permuter runs killed;
    - session crons deleted and the `mvp-decomp-lead-resume` scheduled task disabled;
    - the permuter farm, dashboard and progress bot (no tokens) keep running.
    - Resume steps are in the HANDOFF "Session 2 status" block.
  - Blockers: none new. Owner questions are unchanged (verify keyed by unit name blocks splits; public repo; extra usage; other-platform discs).


- 2026-10-13 | Codex + one focused lightweight audit | fe/tu_80488CDC.cpp; matching helpers | fn_8048A2AC matched (204 bytes), unit 42 -> 43/55; local baseline 20,794 -> 20,795/24,158 functions | Full verifier PASS against the pre-change build, retail main.dol hash unchanged, no regressions; check_literals: 261 references, 0 mismatches/unresolved. A bounded 24-order search found the final four-store ordering. Added reusable store_orders.py (scratch-only, bounded search) and fixed permute.py's missing executable flags on standalone overlay targets: fn_8048A2AC's baseline search score was 5100 against an empty target, now 10 against 51 instructions. Added empty-target/error guards and 8 synthetic helper tests, including real PowerPC assembly/disassembly. Existing fleet integration and farm were active; work stayed in an isolated Codex worktree. No owner action needed. Remaining efficiency findings: farm ledger invalidation depends only on unit source, and original-code/linking issues remain as documented.

- 2026-10-13 | Codex | matched-twin follow-up: snd/tu_803A9204, fe/tu_8051E878, ealib/tu_803D2B20, game/tu_8011243C, game/tu_801A57C8 | Five further verified matches after fn_8048A2AC: iSNDrandom (264 B), fn_8051F784 (196 B), fn_803D2B64 (92 B), fn_8011489C (208 B), fn_801A7CBC (128 B). Session total: 6 functions / 1,092 bytes. Each source commit: retail SHA1 retained, no regressions, no literal/target mismatches. game/tu_801A57C8 retains six unresolved names in other existing functions. | Shortcut: clone --scan also finds existing near-misses, although normal clone mode skips their definitions; manually adapt those matched twins before random searches. Tried 8 bounded search-loop variants for fn_80097380; none matched, no speculative source retained. Also tried volatile zero-pool declarations for fn_803A94BC; worse, discarded. No owner action needed.

- 2026-10-13 | Codex + focused lightweight linking audit | Source-link promotion / helpers | 18 fully matched units now source-linked locally: complete units 64 -> 82; complete code 15,464 -> 39,664 bytes (+24,200). All promotions retain retail SHA1 and pass the unmodified verifier without regressions, one unit per commit. Includes EXIBios (6,620 B), game/tu_80144CF0 (2,920 B), db/tu_805A1A9C (2,920 B), fe/tu_80528B78 (2,028 B), and fourteen smaller units. | Added read-only link_candidates.py: allocated section layout plus real linked-symbol imports, not fuzzy score alone. A larger game/tu_800C2F90 source-link trial failed on unresolved canonical/mangled call names; reverted its config, no speculative source kept. Source naming established _exit at 0x80413F3C and __div2i at 0x80406CC0 from observed runtime calls/stub destinations; their original code remains untouched. No owner action needed.

- 2026-10-13 | Codex | Vector norm | fn_80402A90 matched (60 B), unit ealib/tu_804027DC 29 -> 30/34. Session matching total: 7 functions / 1,152 bytes. A reused float temporary reproduced the target load/operation schedule; direct expression and six load-order probes did not. Full verifier PASS, no regressions or literal/target discrepancies.
- 2026-10-13 | Codex | Source-link follow-up | 18 further units, +58,988 linked code bytes: packed-tag helper correction unlocked 15 trace-wrapper units (+45,344 B); canonical strncpy unlocked fe/tu_80464090 (+5,136 B); canonical memcpy plus per-unit -fno-builtin unlocked game/tu_803543FC (+5,744 B) and game/tu_80353930 (+2,764 B). Session source-link gains: 36 units / 83,188 B; complete units 64 -> 100, code 15,464 -> 98,652 B. Every promotion retained retail SHA1, passed the unmodified verifier without regressions and passed literal/target checks. The shared packed-tag header correction rebuilt all seven users before promotion. Source changes and config remain one unit per commit; final per-commit integration gate also required. | Shortcut: fix one evidence-backed declaration shared by many exact objects before spending time on isolated near-misses. No owner action needed.

- 2026-10-13 | Codex | Matched twins and C import follow-up | Three further exact functions: fe/tu_804C931C fn_804CA2F0 (56 B, switch cases 4/5), ealib/tu_80394744 fn_80394CE0 (264 B, additive generator twin with unsigned state view), game/tu_800034A0 static initializer (3,556 B, 13 normalized global vectors). Session matching gains: 10 functions / 5,028 code bytes, plus 156 B matched BSS. Initializer names corrected to the compiler-generated static initializer and wrapper; verifier accepts same-unit/address/size renames without regressions. Actual target point values retained; plain explicit assignments failed, genuine C++ global initialization matched all 889 instructions.
- 2026-10-13 | Codex | Csis/allocator source-link sweep | 13 more verified source-linked units (+65,808 B): two shared C linkage fixes unlock ten units; three local deallocator-call repairs unlock another three. Every source-linked promotion retains retail DOL SHA1 and passes literal checks and the unmodified verifier. Session source-link gains: 49 units / 148,996 B; complete units 64 -> 113, linked code 15,464 -> 164,460 B. New clone.py fix also copies separate inline member definitions when it copies the declaring class, including their dependencies; 4 new synthetic tests, 12 helper tests pass. Large renderer fn_803EA500 now reaches the correct 426 instructions in scratch but still fails the strict pre-check on its unresolved GXWGFifo address; left unapplied. Two multi-initializer functions in game/tu_80174318 remain parked after a bounded scratch probe. No owner action needed.

- 2026-10-13 | Codex | newlib ctype data identity | Identified _ctype_ at 0x8060E9F0 (257 bytes, including the EOF sentinel), replacing the previous interior label at 0x8060E9F1. Confirmed zero sentinel and ASCII control/space/digit/uppercase/lowercase/hex masks in retail data. Three non-libc consumers now address masks through _ctype_ + 1; all passed the unmodified verifier without regressions. A trial retaining an overlapping interior label was rejected by the splitter and reverted; the final table has one proper object name and no alias. All 531 checked references across affected consumers resolve with 0 mismatches. Seven libc units now source-link (+1,236 B): strtol, strtoul, strcasecmp, strlwr, tolower, isdigit, isspace. Session source-link gains: 56 units / 150,232 B; complete units 64 -> 120, linked code 15,464 -> 165,696 B. Final batch integration required. Unused old declarations in two game headers remain; the three actual consumers use the canonical table. No owner action needed.

- 2026-10-13 | Codex | Data ownership and unused SDK bodies | 49 more verified source-linked units / 51,144 code bytes since db183aa6. Complete units 120 -> 169; linked code 165,696 -> 216,840. Read-only data_candidates.py locates complete compiled sections using code relocation pairs, canonical sized global objects, initialized pointers and unique aligned whole-section matches. No zero-block or ambiguous search guesses. Synthetic helper suite: 27 tests PASS. Both ProDG halfword relocations and MWCC SDA21 halfword-vs-word locations are handled without changing target addresses or relocation kinds.
  - Proper section ownership unlocks private libm/newlib constants and BSS, including vfprintf, vfscanf, vfiprintf, fopen and errno. Several old anonymous pool extents crossed proven boundaries and were corrected with compiled/retail-byte evidence. DVD's 11-byte pool includes five actual zero padding bytes in its split. ef_log remains blocked by a shared/external pool reference and stays extracted.
  - Removed unused SDK definitions outside assigned target ranges, or made required private helpers static inline: GXTexture, GXTev, GXGeometry, GXBump, Padclamp, dvdidutils, EXIUart and ARQ. Every retained target function is unchanged. No target functions moved between units.
  - atexit now compiles as C++ with a C ABI export and calls the existing GC allocation wrapper fn_8000E5BC(int), whose mangled name is the retail call target. FSasync's globals received canonical reference-backed names at relocation-proven addresses, resolving other extracted callers without aliases.
  - Accepted two farm matches after scratch confirmation and the full acceptance gates: fe/tu_8046F770 fn_804707B8 (132 B, zero/one scheduling), db/tu_805721F8 fn_80573FB8 (548 B, weighted-sum operand order). Session gains now 12 functions / 5,708 code bytes; source-link gains from original session baseline: 105 units / 201,376 code bytes. Total source-linked units 64 -> 169. No owner action needed; final per-commit integration gate follows.

- 2026-10-13 | Codex + bounded Luna scratch audits | compiler-aware clones, GX FIFO family, canonical calls | matched 4 new functions / 1,976 B; source-linked 17 more units / 47,116 code B; 15 partial units retain prior scores | Each source commit passed retail hash, literal checks and unmodified verifier. New functions: OS fn_80438704 (32 B), DB fn_80594810 (112 B), FE fn_80463C64 (128 B), renderer fn_803EA500 (1,704 B). Source promotions: game/tu_80344180, game/tu_80042DBC, dolphin/vm/tu_8043D960, snd/cmn/salloc, game/tu_80360C18, and 12 GX setup units from 803666EC through 8036F44C. The shared typed FIFO view reproduces lis 0xCC01 with signed displacement for both 4- and 8-byte register unions. Retail control flow proved two calls belonged inside the mB0 condition in seven related renderer sources; whole-DOL trials caught the masked branch error. Natural global construction recovers F44C's 20-byte BSS node and ctor entry; two compiler initializer names were corrected at unchanged addresses/sizes. The clone search now preserves local-label topology and compares resolved internal branch destinations; bad/fixed real controls and 69 synthetic helper tests pass. Compiler selection is shared with MWCC C handling; farm fingerprints now include headers, flags and helper code on future starts, with exact trial restoration. Existing farm was not restarted. Parked: D9C34 cannot source-link alone because its hand-coded startup wrapper lacks the owned ctor entry and its initialization body lies in the preceding placeholder unit. DB's two remaining near misses did not improve in bounded probes. D8234's nearer fn_803D83BC keeps its old symbol view; DA4EC's helper change regressed fn_803DBBE0 and was rejected. No owner decision needed. Per-commit integration gate pending before push.


- 2026-10-13 | Codex + bounded Luna scratch audits | ABI/import recovery and near misses | Seven new exact functions / 2,992 code bytes since 88de073d; fifteen more source-linked units / 24,960 code bytes. Complete units 186 -> 201, linked code 263,956 -> 288,916 B; exact functions 20,885 -> 20,892 and matched code 2,995,596 -> 2,998,588 B. All source commits pass retail SHA1, literal checks and the unmodified verifier without regressions; per-commit integration follows.
  - New functions: game fn_8012C2BC (508 B), fn_8022A71C (536 B), fn_8001DEC0 (404 B), Cpu::fn_80238934 (268 B), fn_80245730 (248 B), ealib fn_803F28A4 (460 B) and fn_803F2A70 (568 B). The three control-flow repairs retain retail null handling and branch destinations. The last two reproduce load-before-store ordering with explicit real-value locals.
  - Six fully matched game units source-link after actual runtime/C++ declarations replace unresolved placeholders: 803414A4, 803560F8, 80360168, 8035688C, 80352E64 and 8035726C. Typed placement construction and existing deallocators preserve destructor work. Eight text-only CARD consumers source-link using existing exported fn_/lbl_ symbols and proper SDK parameter types; no provider aliases or metadata renames were needed.
  - game/tu_8007A4DC now source-links 4,664 B. Retail callers establish float results and incoming arguments omitted from three wrapper declarations; all sixty functions keep their offsets, sizes, instruction words and local branch destinations. Three method manglings were corrected at unchanged addresses/sizes. Provider-specific C overloads and pointer casts stay local, preserving other header users. Shared header changes are limited to the corrected wrapper methods.
  - fn_80090880 explicitly forwards the incoming localization ID to fn_80402CAC. First migrated the already-linked FE caller onto the table-manager header, then corrected the provider signature and symbol metadata in its own commit, then removed the temporary function-pointer cast. Every intermediate commit passed. This unlocked tu_8035726C without mixed-unit commits or aliases.
  - Store-order helper now accepts decimal/exponent float literals while rejecting expressions and repeated fields; 70 helper tests PASS. A read-only scratch scan found 100 near misses within two instruction words with identical local branch paths. Parked after bounded probes: CARDInit extra instruction; tu_8007DB74 still needs argument tracing; fn_802B2C98's 120 float-store orders did not match; reciprocal stack-slot and several index/minimum forms did not improve. Existing farm remains running. Session gains from the original baseline: +23 exact functions / 10,676 code B; +137 source-linked units / 273,452 code B. No owner action needed.

- 2026-10-13 | Codex + two bounded Luna audits | Return layouts, wrapper ABI and helper checks | Four further exact functions / 1,004 B; two source-linked units / 9,940 B since 48722645. Exact functions 20,892 -> 20,896, matched code 2,998,588 -> 2,999,592 B; source-linked units 201 -> 203 and code 288,916 -> 298,856 B. All source changes retain retail SHA1, pass literal/target checks and the unmodified verifier without regressions. Per-commit integration required before push.
  - fn_8001EE28 (396 B) uses the callee's actual eight-byte pair instead of a twelve-byte buffer; Unk80159F28::fn_801653B0 (320 B) uses the actual Pair8 return; fn_803D74A0 (76 B) uses the actual five-word result. Callee definitions/retail accesses establish each extent; every instruction and local branch matches, with actual objdiff 100%.
  - game/tu_8007DB74 source-links 5,668 B after canonical provider imports and explicit incoming arguments omitted from six reconstructed signatures. Metadata renames preserve addresses/sizes; no aliases. game/tu_8007CAC4 source-links 4,272 B after real provider types and nine void setter return corrections. Both current shared declaration views were corrected; all users rebuilt. This also makes the existing caller fn_800E25E0 (212 B) exact. Status-returning providers remain int.
  - New read-only frame_candidates.py narrows searches to equal-length instruction streams with only stack-relative offset changes, different frames and equal internal branch paths. Clone checks now cover all executable sections, including objects with both text and overlays. 77 helper tests PASS; synthetic fixtures prove mixed-section scanning and reject changed overlay branches. Current game objects had no mixed-section gap.
  - Parked without source edits: fn_cslot11 has no evidence for changing its Vec3 extent; fn_8058C3C4's seven bounded signature/local/known-zero variants did not improve its seven-slot schedule mismatch. The fn_8035AB44 destructor in-charge forwarding issue remains parked. Existing farm continues. Session gains from original baseline: +27 exact functions / 11,680 matched code B; +139 source-linked units / 283,392 code B. No owner action needed.

- 2026-10-13 | Codex + three bounded Luna workers | Farm matches and final near-miss audit | fn_803F1204 (208 B) and fn_803F113C (200 B) from the running farm, plus same-family fn_803F12D4 (204 B), now match exactly. Replaced arbitrary permuter names with initialized active state locals; storing -active and active retains retail constant scheduling and the original values. Word streams, branch paths and named imports were checked before full hash/literal/unmodified-verifier acceptance. Session gains: +30 exact functions / 12,292 matched code B; +139 source-linked units / 283,392 compiled code B. Total exact functions 20,899 / 24,158; matched code 3,000,204 / 5,896,176 B. Source linking remains 203 units / 298,856 code B / 8,062 data B.
  - Expanded to three lightweight agents at the owner's request; tasks use disjoint functions and scratch candidates. fn_803F0FCC improved five -> three differing words with the same new mechanism, still unmatched. Three fn_803F108C variants regressed. No new evidence supports the unusual absolute constant forms in fn_803F13A0. Shared docs/NEARMISS.md now consolidates failed bounded probes to prevent repeated work across agents. 77 helper tests PASS; protected verification files untouched. Final per-commit integration remains required before push.

- 2026-10-13 | Codex + three bounded Luna workers, resumed at owner's request | Seven more source-linked units, +10,184 code B / +56 data B since 9ff739dd. Complete units 203 -> 210; compiled code 298,856 -> 309,040 B; compiled data 8,062 -> 8,118 B. Exact-function count remains 20,899; these are source-link gains in already-matched functions. Every accepted change passes retail SHA1, literal checking and the unmodified verifier; per-commit integration follows.
  - DVD error code unit (408 B): use the RTC type header instead of internal __os.h, eliminating five irrelevant startup declarations. AMC stubs (40 B): omit the out-of-split AMC_IsStub definition. OSRegisterVersion (44 B): reference the existing four-byte format object, preserving the target's small-data reference without claiming ownership at a misaligned new allocation.
  - DVD queue (504 B + 32 B BSS): omit the two out-of-split diagnostic/queue functions and their private debug table, and mark the internal pop helper inline. All five retail function streams and local branches stay exact; paired relocations anchor the real queue array at 0x806DE1F8.
  - Double/float ceil (344/196 B + 16/8 B data): mutable huge objects and x+huge arithmetic reproduce the exact instructions and writable small-data blocks. The former const definition produced an unreferenced read-only duplicate with ambiguous ownership; no pool was placed by guessing. Public reference licenses preserved.
  - game/tu_800C2F90 (8,648 B): canonical C/C++ import types plus guarded real deallocation retain all sixty functions and 152 branch edges. Before linking it, corrected fn_802B96A4's actual three-integer arguments and result in its provider TU; its 32-byte code is unchanged. Metadata rename Fv -> Fiii preserves address and size, with no aliases or final mismatched function-pointer call. All 334 consumer references resolve and compare exactly.
  - New compiler lead: -mps-float enables ProDG __ps_t and paired-single builtins. Native lvalues fold offsets, but fn_803A3D4C remains seven words off because quantization/register encodings differ; no flags or unmatched SIMD source were accepted. Indexed builtin probes returned success without producing ELF, so their API acceptance is unproven. Scratch remains under build/agent-event.

- 2026-10-13 | Codex + three Luna workers, continued | libm/ef_log source-linked (640 code B / 116 data B): export its existing first coefficient as lbl_806F0000, which an external extracted table already references. Other coefficients remain private and unchanged. Source/target code, 20 local branch destinations and both data blocks are exact; full retail hash, literals and verifier pass. The initial wrong scratch path was rejected and restored before the correct reference was used.
  - fe/tu_804FF6B4 source-linked (3,112 code B): bind string calls to the actual strcpy/strncpy providers and TEAMNAME/TEAMCITY to their existing data symbols. All 27 functions and local edges match; 129 references check with zero mismatches/unresolved names. Full hash and verifier pass. Two independent worker audits agreed on the import addresses.
  - Read-only import_candidates.py pairs matched source/retail REL24 call sites and lists canonical exported provider names. It distinguishes conflicting destinations and unresolved imports, preserving all aliases as leads rather than selecting a guessed ABI. Seven new synthetic regression tests pass; matching-helper suite is 84/84.
  - Source-linked totals are now 212 units / 312,792 code B / 8,234 data B; exact functions remain 20,899. The table v4 argument repair is parked under the one-TU commit rule; no conditional class definitions or mismatched function-pointer calls were accepted.

- 2026-10-13 | Codex + three Luna workers, continued | Three new exact functions, +952 matched code B: fn_801B58D4 (368 B, recover C-linkage unsigned RNG return and label/scale evaluation order); fn_801A2158 (224 B, return inline clamp expression directly and assign it to the caller's existing float local); fn_801F522C (360 B, declare actual eight-byte output scratch before the vector copies, preserving the target's existing float data references). Every candidate passes retail hash, literal checks and the unmodified regression verifier. Exact functions 20,899 -> 20,902; matched code 3,000,204 -> 3,001,156 B.
  - fe/tu_804B38C4 source-linked (4,164 B, 36 functions): real C singleton-constructor entry, correct C++ zero-argument function linkage, strncpy, and named original literals. All 16 shared-header users compile without word or local-edge changes across 617 functions; actual full build and regression verification pass. 237 references check with zero mismatch/unresolved names.
  - fe/tu_804B985C source-linked (6,324 B, 28 functions): original string/table objects plus canonical sprintf/strncpy. Stack-local 30-element arrays remain local and initialize from the existing exact 120-byte tables via memcpy; all 59 local branches and 192 checked references agree. Full hash and verifier pass. Source-linked totals now 214 units / 323,280 code B / 8,234 data B.
  - Bounded unsuccessful trials are preserved under build/ and logged in NEARMISS.md. Workers continue on two disjoint front-end ranges plus a read-only stack-slot scanner; the existing permuter farm remains active. This is an integration checkpoint, not the end of authorized work.

- 2026-10-13 | Codex + three Luna workers, continued | fe/tu_804B7C98 source-linked: 7,108 code B and the real 8-byte integer-to-float bias pool. The only two retail references to lbl_805F7CA0 come from this unit; paired relocations uniquely anchor its exact aligned pool, with no external alias needed. Correct typed UiS C++ calls and strncpy plus original string/float symbols retain all fifty function streams/local branches. The actual build passes retail SHA1, 229 reference checks (zero mismatch/unresolved), and the unmodified regression verifier. Total source-linked: 215 units / 330,388 code B / 8,242 data B; exact functions remain 20,902.
  - Prior five source commits passed per-commit integration and reached main at 148f23ce. The permuter farm remains active (129 ledger entries, four matches already accepted); no new farm match was available at this checkpoint. Workers continue on the database candidate and read-only scanners.

- 2026-10-13 | Codex + three Luna workers, continued | fn_80562BC8 (db/tu_8055F730, 620 B) now exact: remove the first temporary-list pointer alias and access the real local directly. This makes the final vptr store frame-relative, as in retail. Original loop-variable scopes are preserved; all other 23 compiled function bodies and relocation signatures remain unchanged. All 155 instructions, twelve local branches, thirty checked references, full hash and unmodified verifier pass. Totals: 20,903 exact functions, 3,001,776 matched code B (50.91%).
  - New read-only stack_candidates.py finds same-frame slot differences and explicitly labels provisional relocation mismatches when requested. It found the fn_801D14A0 and fn_801C4C3C leads; both remain unaccepted after bounded trials/ABI audits. Six new synthetic tests include all three executable sections, changed branch conditions/destinations, malformed stack operands and call/data binding differences. Matching-helper suite: 90/90. Tool commit is separate from game source and still uses the normal integration gates.

- 2026-10-13 | Codex + three Luna workers, continued | B7 source-link, DB exact match and stack scanner passed per-commit integration and reached main at 3661cb68. Added read-only literal_pair_map.py: pair equal-sized matched functions across all executable sections, require equal relocation kinds, skip executable symbol targets, validate source/retail extents, preserve alternate names and mark incomplete prefix checks. Seven new synthetic tests cover mixed sections, unpaired/mistyped references, executable targets, negative/oversized/truncated reads and alias ambiguity. Helper suite 97/97. The mapper recovered the B7 bias-pool lead before that unit's full acceptance gates.
  - Read-only global reference audit: 90,594 checked, 0 mismatches, 1,104 unresolved names. Unresolved names remain exclusions, not proof that all bindings are settled. A paired-call scan of 224 high-score call-site leads found no evidence-backed prototype repair beyond already accepted work; remaining names/ABI equivalence alone did not justify changes.

- 2026-10-13 | Codex continuation | db/tu_80596D88.cpp | matched fn_80597A5C (20 bytes), 43/47 unit functions | Retail caller fn_8059053C stores returned r3 at +0x2C after allocation; correct pointer result and return self reproduce all five words. Strict references, full build, retail hash, literal check and unmodified verifier PASS. fn_80596D88: eight return/order forms improve ABI correctness but retain store-order differences; parked. | none
- 2026-10-13 SW02: db/tu_80563954.cpp, matched fn_80564C9C (0xA4; 41 instructions), 1/1 functions attempted across 8 scored source forms; unit 14/32 -> 15/32. Reconstructed id/float-threshold/counter comparison including unordered float behavior. Caller fn_804C931C region at 0x804C9BBC supplies f1/f2 and r3-r7, confirming the argument shape. check_literals: 67 references, 0 mismatch/unresolved. Unmodified verify.py --baseline against unchanged origin/main 6d282adf report: PASS, no regressions, retail SHA1 da6becdbea614d03c4b5eae9f8a0fb08e0a184dd. No blockers or owner decisions.
- 2026-10-13 | Codex swarm04 | game/tu_8026C414 | matched fn_8026C864 (152 B), six bounded source forms | inline owned-pool cleanup preserves derived-vptr addressing; 38/38 instructions, 78 literal/call references checked with zero mismatches/unresolved; retail SHA-1 and unmodified verifier PASS against swarm baseline 6d282adf, no regressions (20903 -> 20904) | none
- 2026-10-13 | Codex swarm05 | fe/tu_804BB110 fn_804BB334 now exact (+156 code B; 35/40 unit functions). Inline rectangle/record resets with an indexed outer loop reproduce the retail induction and store order; five bounded variants. All 39 instructions match, 175 literal/import references check with zero mismatches or unresolved names, and the unmodified verifier passes against swarm baseline 6d282adf with retail SHA1 and zero regressions. No protected tooling/config changes.
- 2026-10-13 | Codex swarm06 | ealib/tu_803FFD9C | matched fn_804006FC (80 B), unit 36 -> 37/41 exact functions | Conventional first-match callback removal matches all 20 instructions using a while loop and break. Bounded eight-form scratch probe: for/return, do/return and indefinite-loop forms failed; counter-search followed by a separate clear also failed. Accepted form was attempt 7. Full build, 119 literal/call references (zero mismatches/unresolved), retail SHA-1 and unmodified verifier PASS against swarm baseline 6d282adf, zero regressions (20903 -> 20904). fn_804004D0 deferred without attempts to release the worker slot. No owner action needed.

- 2026-10-13 | Codex swarm07 | game/tu_802E6464 | fn_802E7FA8 matched (+268 code bytes; 30/40 unit functions). Two conventional vector/radius forms yielded 67/67 instructions. Full build, retail SHA-1, 144 reference checks with no mismatches and unmodified current-commit verifier passed; 20 unresolved names were preexisting unrelated references. Coordinator independently verified the integration.
- 2026-10-13 | Codex swarm10 | fe/tu_804EA2E4 | fn_804EADD4 matched (+268 code bytes; 50/52 unit functions). Two conventional forms using existing FeP4TabX layout; 264 reference checks with zero mismatches/unresolved, full build, retail SHA-1 and unmodified current-commit verifier passed. Coordinator independently verified the integration.
- 2026-10-13 | Codex matching swarm | Twelve distinct workers in waves, three active beside the lead maximum; six exact functions / 1,088 code bytes accepted. No new source-linked units. All other variants remained scratch-only, with concrete parks recorded in docs/NEARMISS.md. Full worker/results/evidence report: docs/SWARM_SESSION_2026-10-13.md. Project now 20,910/24,158 exact functions and 3,002,884/5,896,176 matched code bytes (50.93%); these totals include concurrent work by the existing agent. All workers finished with clean tracked state and no worker-owned jobs. No owner action needed.

- 2026-10-13 | Codex + three Luna workers | GAME/EA/DB/FE return and lifetime recovery | matched five functions / 720 code bytes: fn_8001ED28 (88), fn_8001F3B8 (68), lbl_Owner::fn_803DA038 (96), fn_8057CFA8 (220), fn_804DF4D4 (248). Real callers prove integer/node/list results; corrected actual callee declarations and all shared-header users rebuilt. FE message-pointer materialization belongs inside each handled branch. Separate caller declaration/preparation commits preserve one-TU history. All strict binding/local-edge checks, full retail SHA1, literal checks and unmodified verifier PASS; per-commit integration queued. | none
- 2026-10-13 | Codex + Luna shortcut worker | SDK GXAttr, CARDStatEx, dsp_debug, dsp_task | source-linked four units / 8,280 code bytes and 900 target data bytes including alignment padding. Removed only source revision functions absent from target, corrected canonical imports, and bound GX small tables / DSP state to existing real globals. GX owns its eight exact compiler jump tables; DSP owns its exact format-string pool while shared state stays external to avoid a real section-order cycle. All full hashes/literals/regressions PASS. | none

- 2026-10-13 | Codex + Luna helper worker | tools/match/link_candidates.py | added per-function section-relative placement diagnostics across all allocated executable sections; reports missing/extra/moved/ambiguous functions and requires identical placement for ready counts. Identical alias entries are retained; conflicting locations and zero extents remain provisional. 109 helper tests PASS, actual DA038 five moved functions detected, full retail hash and unmodified verifier PASS. | none

- 2026-10-13 | Codex integration | rebased continuation over the concurrent six-function / 1,088-byte swarm pass, preserving both source changes and both bounded-attempt logs. Fresh full build retains retail SHA1; combined snapshot 20,915 exact functions and 3,003,604 matched code bytes. | none
- 2026-10-13 | Codex persistent swarm16 | ealib/tu_803FFD9C | fn_80400690 exact (+108 B;38/41 funcs), conventional duplicate-check then first-empty callback insertion.27/27instructions,126 reference checks0mismatch/unresolved,full retail hash and unmodified baseline verifierPASS vs9a1bb0c5. Four bounded forms; first matched. Work continues by owner instruction; no batch stop condition.

- 2026-10-13 | Codex persistent swarm16 | ealib/tu_803FFD9C | fn_803FFEB8 exact (+84B;39/41funcs). Retail at803FFECC forwards incoming h in r3 tofn804000C8; prior source passed0 and emitted an extra li. Provider and caller evidence confirm handle normalization.21/21instructions,128referencechecks0mismatch/unresolved,fullhash+unmodified verifierPASSvs9a1bb0c5. Work continues.

- 2026-10-13 | Codex persistent swarm16 | ealib/tu_803FFD9C | fn_8040024C exact (+112B;40/41funcs). Naturally pretested while(msg!=2), with initialmsg0, reproduces retail per-call local address and queue-base register without changing behavior.28/28instructions,133referencechecks0mismatch/unresolved,fullretailhash/unmodifiedverifierPASSvs9a1bb0c5. Four sourceforms. fn804004D0 parked after8typedcopy/initializer/helper forms, best24vs20instructions (clock lifetime/r30/frame); caller80148590 provesunsignedclockreturn, no speculative source retained.
- 2026-10-13 | Codex persistent swarm14 | fe/tu_80516990 fn_805170C0 exact (+320 code B; 42/47 unit functions), four conventional variants. Two-side extraction/sort/reinsert uses a four-element local matching the provider's four-entry side records. Reuse the sort counter for reinsertion and load the next player before incrementing its index to preserve retail register allocation. Canonical provider signed count and unsigned item index recover hoisted call arguments; float score result comes from the retail floating-return path. All80words match, 206references have zero mismatch/unresolved, full build/retailSHA1/unmodified verifier pass against9a1bb0c5. No protected changes. Three other targets were restored and parked in NEARMISS.md.

- 2026-10-13 | Codex persistent swarm14 | fe/tu_804CB430 fn_804CE2AC exact (+348 code B; 39/45 unit functions), two source forms. Existing 40-byte seven-slot DB index and two returned IDs construct the four-word state result at +0x688. Passing that result by value into the inline setter preserves retail's temporary copy and all four reloads; manual consecutive copies lost three reloads to CSE. Actual fn_800B555C state argument is explicitly forwarded to its predicate. All87words match, 140references have zero mismatch/unresolved, full build/retailSHA1/unmodified verifier pass against9a1bb0c5. Failed fn_80454BB0 prefix probes restored and parked. No owner action.

- 2026-10-13 | Codex persistent swarm14 | fe/tu_804E02E0 fn_804E1020 exact (+480 code B; 38/41 unit functions), first source form. Recovered title screen states0..3, explicit idle cases8/9 and default transition, original signed timer thresholds, virtual widget updates and lazy singleton calls. The named-child result from fn_800ED2B0 is explicitly forwarded into fn_800CFB38 with its actual pointer argument. All120words match, 224references have zero mismatch/unresolved, full build/retailSHA1/unmodified verifier pass against9a1bb0c5. Two other bounded FE probes restored and parked; no owner action.

- 2026-10-13 | Codex persistent swarm14 | fe/tu_804FE34C fn_804FE6B8 exact (+500 code B; 41/43 unit functions), three compiling forms. Recovered roster callbacks, 40-int temporary-array construction/cleanup, secondary selection control and selected-team/player state writes. An inline getter for the secondary control preserves the retail pointer adjustment before vptr loads; source store order reproduces the array header and selection resets. Explicit incoming default argument and zero arguments in cases0/1 preserve the existing call stream. All125words match, 140references have zero mismatch/unresolved, full build/retailSHA1/unmodified verifier pass against9a1bb0c5. No owner action.

- 2026-10-13 | Codex persistent swarm14 | fe/tu_804FE34C fn_804FEE04 exact (+528 code B; 42/43 unit functions), two source forms. Recovered the selected-menu string comparisons, prompt state fields and popup branch with the canonical localization import. Writing the three integer prompt fields before the label pointer reproduces retail's label-first schedule and zero-field reuse. Correct int result preserves existing callers that ignore it. All132words match, 176references have zero mismatch/unresolved, full build/retailSHA1/unmodified verifier pass against9a1bb0c5. No owner action.
- 2026-10-13 | Codex persistent swarm13 | game/tu_8026C414 fn_8026C65C: reconstructed the transform/vector bases, six 0x20-byte entry initializers and embedded transforms/pool using existing layouts. Candidate matches all 64 instructions (+256 code bytes; 53/58 functions). Full build, retail SHA1, 98 literal/symbol references and unmodified verifier against the 9a1bb0c5 baseline pass with no regressions. No protected changes.

- 2026-10-13 | Swarm continuation: fe/tu_804FE34C source-linked (43/43 functions, 4,968 code bytes). Repaired local calls to the canonical strcpy and fn_80090880__FP11Unk80090880i providers; section footprint, function offsets and sizes match retail, all 211 literal references resolve, and the unmodified verifier passes with no regressions against the committed combined baseline. Full retail SHA1 remains da6becdbea614d03c4b5eae9f8a0fb08e0a184dd. No shared headers, flags, aliases or verification tooling changed.

- 2026-10-13 | Swarm continuation: fe/tu_804E02E0 fn_804E0BAC +120 matched bytes, all 41/41 functions now exact. Correct local timer return type matches provider fn_800D876C; conventional zero-field initialization order recovers the remaining stores. Eight bounded source forms, 30/30 instructions, 299 literal references fully resolved, unmodified verifier and full retail SHA1 pass against committed 91d7c79f baseline. Source linking remains separate: this unit emits an extra 176-byte read-only pool and has two older missing imports.
- 2026-10-13 | Codex persistent swarm13 | game/tu_8026C414 fn_8026C75C: adjacent constructor forwards the additional owner flag to fn_80328234 and reuses the verified transform/entry initialization. Candidate matches all 66 instructions (+264 code bytes; 54/58 functions). Full build, retail SHA1, 118 literal/symbol references and unmodified verifier against the 9a1bb0c5 baseline pass with no regressions. No protected changes.

- 2026-10-13 | Codex persistent swarm13 | game/tu_8026C414 fn_8026C414: reconstruct state-record comparisons and event 127/128/129 posting. Retail retains the old-state==10 comparison in CR7; this fixes the last arm lost in the m2c draft. Existing owner/player layouts and canonical event destructor imports preserve all 135 instruction words (+540 code bytes; 55/58 functions). Full build, retail SHA1, 140 literal/symbol references and unmodified verifier against the 9a1bb0c5 baseline pass with no regressions. No protected changes.
- 2026-10-13 | Codex lead + three Luna workers | continued SDK/SND source ownership | source-linked 13 more units, +34,844 code bytes and +3,344 data bytes including verified padding; no new function matches in this batch | OSMessage, CARDRdwr, locale, SISamplingRate, GXFrameBuf, CARDRead, Pad, DSP core, libsn dummy, CARDUnlock, AX, SIBios and saems now compile into the final retail-identical DOL. Separate one-TU preparations recover AX conversion-helper call names and CARD mount callback visibility. Same-address metadata recovers __cvt_fp2unsigned and the C-compatible SI command-state object name. Private compiler pools are owned only after exact bounds/bytes and all-consumer audits; shared state remains bound to its existing real providers. Full hash, literal check and unmodified regression verifier pass after every acceptance. Global function total remains 20,915/24,158; 232 source-linked units contain 373,512 code and 12,486 data bytes. Per-commit cold integration required before push. | none

- 2026-10-13 15:15 CDT | Sole-integrator merge batch: all finished initial branches plus matching batches16/17/19, matching-fe-db-a and root-fe-batch01; 50 additional exact functions /12,724 code bytes versus e0f1b437. Initial33-function handoff passed every gate before upstream main moved; consolidated batch includes17 newly verified matches. Required stock full retail, regression and literal gate runs on the final merge before publication. Six matching workers continue in disjoint four-unit batches; no new source linking. One-line near misses preserved; no owner decisions.

- 2026-10-13 15:45 CDT | Sole-integrator matching batch: verified ready batches20/22/25 and matching-ea-b, seven additional exact functions /2,584 code bytes versus the merge base. Worker full builds, zero literal mismatches and final committed-HEAD regression gates pass; stock integration repeats retail/literal/regression checks on the combined merge before push. Six matching workers continue in disjoint four-to-six-unit batches. One shared Claude-owned roster ABI blocker is recorded in DECISIONS; no new source linking or protected-tool changes.

- 2026-10-13 16:15 CDT | Sole-integrator matching batch: cleaned batches21/24/27 plus matching-ea-b3 add ten exact functions /4,592 code bytes; three notes-only handoffs add no function count. Canonical pointer/integer boundaries and direct DB calls replace rejected incompatible drafts; their invalid commits stay on local recovery refs. Worker full builds, zero literal mismatches and final committed-HEAD regression gates pass. FE retains two pre-existing unresolved constructor imports. Stock integration gates the combined branch before push. Six matching workers remain active; no new source linking or protected-tool changes. Claude-owned roster/float-wrapper ABI blockers remain parked.

- 2026-10-13 16:45 CDT | Sole-integrator matching batch: ready23/29/30/31 plus root SDK/SND/FE handoffs add eleven net exact functions /5,564 code bytes; notes-only handoffs add no count. One local date match already present upstream is excluded. DB virtual provider contracts audited against concrete vtables; new caller follows canonical integer boundary after correction. Worker full builds, zero literal mismatches and final committed-HEAD verifier gates pass; stock integration verifies the combined merge before push. Six workers continue in fresh four-unit matching batches, no new source linking.

- 2026-10-13 17:15 CDT | Sole-integrator matching batch: coordinator DB/SND batches26/35, root sound batch08, matching-sdk-c3 and local32 add five net exact functions /1,168 code bytes. Local34 adds only a required near-miss note: both reported code gains already existed upstream, whose exact bodies are preserved. Candidate selection now filters the published main report as well as each warm branch. Full builds, zero literal mismatches and final committed-HEAD worker verifiers pass; stock integration gates the combined merge before push. DB parser retains four pre-existing unresolved names. Six workers continue fresh four-unit batches; no new source linking. Further Claude-owned return/callback contract blockers are recorded in DECISIONS.

- 2026-10-13 17:45 CDT | Sole-integrator matching batch: A5, C5/C6, local36/local37/local39 and root09 add ten net exact functions /2,200 code bytes. Duplicate local37 FE definition is excluded and the existing main body is preserved. Canonical DB record declarations move unchanged to a minimal shared header: all 758 source objects remain byte-identical; approved DB float declaration agrees with its existing provider. These header preparations and root38 near-miss notes add no functions. Full worker builds, zero literal mismatches and final committed-HEAD verification pass; stock integration gates the combined batch before push. Existing unresolved names remain unchanged. Six workers continue matching in disjoint four-unit packets; no new source linking.
- 2026-10-13 | Claude Opus 5.5 ("REA library usage" side session) + 3 Fable 5.1 / 5 Sonnet 5.5 agents | research-driven speed-ups; big-function test; RE4 SDK reuse | matched 16 functions / 17.1 KB on main: fn_800D69E8, fn_8048C9F4, fn_8059E708 (Fable tests, ef7ce69f); fn_80343B44, fn_80343DE4, fn_803E628C (twins test, 6330ed05); __ARChecksize, PackArgs, GetFontCode, ParseStringS, __OSModuleInit, DVDGetCurrentDir, __OSInitAudioSystem, __OSStopAudioSystem, __GXAbort, GXAbortFrame (RE4 SDK ports, aa9bfc7b). Parked: __OSUnhandledException (17 register slots), __OSMaskInterrupts/__OSUnmaskInterrupts (need nopeephole). A Sonnet control also matched fn_8048C9F4 (not merged, duplicate). | Notes:
  - Tools: tools/match/twins.py (look-alikes, callee prototypes, substitution tables), tools/match/linkcheck.py (3,515 callee names that won't link; the lead is fixing include/ in batches), progress card shows built-from-source. PATTERNS.md: RE4 ProDG section and "Loop dumps". DECISIONS.md: RE4 as SDK source.
  - About 12 KB of SDK matches need splits along the original files with -opt nopeephole (tu_804330D0, GXInit.c, OSTime.c, OSRtc/OSReset tails); handed to the lead, who started OSTime.c.
  - Blockers: the push race between integrators (verify 6-20 min; one batch verified 3 times before landing), SPEEDUPS #19. rename.py needs the .cpp suffix and hung 12 min on ar.c. sort_functions.py moved type definitions below their first use on game/tu_80341B78.
  - Usage: weekly all-models 86% -> 90% for this session and its 8 agents; Fable weekly 2% -> 5%.
  | Owner: (1) whether Fable 5.1 goes beyond test cases (3/3 matched; Sonnet matched the shared one too); (2) codeless asm question (DECISIONS); (3) shared integration lock including Codex (SPEEDUPS #19); (4) any other MVP/EA builds with symbols (SPEEDUPS #7)

- 2026-10-13 18:15 CDT | Sole-integrator matching batch: clean A6, A7, B6, C7 and local41/local42 add eight net exact functions /3,248 code bytes versus published609503da. Root12 and local44 contribute only required near-miss notes. New DB caller signatures agree with existing concrete providers; EA completion flags are shared with retrace/draw-sync callbacks. Full worker builds, zero literal mismatches and final committed-HEAD verifier gates pass. Stock integration gates the combined batch before push. Six workers continue on fresh-main four-unit packets; no new source linking.

- 2026-10-13 18:45 CDT | Sole-integrator matching batch: root13 and C8 add three net exact front-end functions /232 code bytes versus published96cd5f4c. Local45/local48 and coordinator47/51 contribute only required one-line near-miss notes. Full worker builds, zero literal mismatches and final committed-HEAD regression gates pass; existing unresolved imports in unchanged DB units remain disclosed in worker logs. Stock integration gates the combined batch before push. Six workers continue on four-to-six-unit matching packets selected from published-main report and consolidated attempted-address index; no new source linking.

- 2026-10-13 19:15 CDT | Sole-integrator matching batch: root15, coordinator54 and local49/local52/local53 add six net exact functions /2,036 code bytes versus published199b90d8; Notes-only A8/B8/root16 add no count. C9 canonical key/int-output preparation preserves all 64 affected functions, sections and relocations and adds no functions. Binary branch evidence corrects the front-end zero-selector arm; ordinary heap-address temporaries and per-arm DB total stores recover the remaining exact functions. Full worker builds, zero literal mismatches and final committed-HEAD verification pass; stock integration gates the combined batch before push. Six workers continue in disjoint four-unit packets, filtering main and durable attempt records; no new source linking.

- 2026-10-13 19:45 CDT | Sole-integrator matching batch: local56/local58 and corrected A9 add seven exact functions /1,432 code bytes versus published73e1fc3a. Root55/root59/local57/localFE09 contribute only required near-miss notes. Canonical date pointer identity, existing record lifetime and sound buffer arithmetic match real providers; full worker builds, zero literal mismatches and final committed-HEAD verifier gates pass. Stock integration gates and pushes the combined batch under the shared integration lock. Six workers continue fresh four-unit matching packets; no new source linking.
- 2026-10-13 | Claude Opus 5.5 (lead, solo at the owner's request, ~15:15-19:45 CDT; no fleet) | near-misses, SDK splits, link names | main 20,931 -> ~21,060 functions with Codex and the "REA library usage" session working in parallel.
  - Near-misses converted by hand, all merged with VERIFY PASS:
    - snd: fn_803A9CF4, fn_803B02FC, fn_803B3E18, fn_803B6D58, fn_803B75A8, fn_803B3F60 (tu_803B3E18 complete).
    - game: fn_8011AA24, fn_80104798, fn_8010EAF4, fn_8010A284, fn_8010FFD8; fn_8010FE70 raised from 94.2% to 97.8%.
    - Most were wrong callee declarations:
      - a missing pass-through parameter (fn_803AB310(h), fn_8006BC74(p, idx));
      - a C++ declaration of an extern "C" callee with the wrong return type (fn_800DE440, fn_80403DE4);
      - strncpy called under its placeholder name.
    - Others: an int-held table base for the r11 index register, an early return/else to place `li r3,-1`, a plain `while (rem--)` loop.
  - SDK splits, now possible because verify follows moved functions (PR #135):
    - dolphin/os/OSTime.c (0x80438628, -opt nopeephole): __OSGetSystemTime and __OSTimeToSystemTime match.
    - dolphin/gx/GXFifo.c (0x80429FF0); the GXInit.c tail (tu_80428F14) gets -opt nopeephole: __GXInitRevisionBits and GXInit (1,956 B) match.
    - Bodies come from the RE4 decomp's dolsdk2004 sources, ported by the REA session's worker.
  - Link names: C++-linkage declarations of extern "C" callees were made extern "C" across about 205 files, with no codegen change (verify). linkcheck's c++->C group fell from 6,290 to 2,215 relocations. 56 names stay C++ because some files declare them as overloads.
  - Evening, owner "keep solving functions": fn_802B2C98 (float store order, brute force), fn_80319534 (m34 store moved after m40), fn_80391C20 (index array address held as an int: index-first lhzx).
  - Coordination: registered as claude-lead in tools/coord.py (docs/COORDINATION.md); integ.sh now runs integrate.py under `coord.py run-locked claude-lead`; nm-09/nm-10 units claimed.
  - Parked near-misses (5-30 variants each; allocator ties): fn_803A7924, fn_803B7700, fn_803B5B6C, fn_803A6B64, fn_803A94BC, fn_80122570, fn_800EF2FC, fn_80220BFC, fn_8025AC0C (Vec3 slot), fn_802C920C (constant reassociation), fn_80391DC4, fn_803EDD8C, fn_8058DBEC (rec/fl register swap), fn_80392884, fn_802C7EF8.
  - SDK backlog from the RE4 worker (lead-owned config; notes in that session's scratchpad sdk1/sdk2):
    - dolphin/os/tu_804321CC: OnReset fn_80432904 matches with nopeephole; needs a file split.
    - dolphin/os/tu_80435A10: ranges 0x80435BD0..0x80435CE8 and 0x80436154..0x8043643C match with nopeephole; UnlockSram needs OSRtc.c as its own unit.
    - dolphin/os/tu_804330D0 is 8 SDK files with mixed flags (OSCache, OSContext, OSExec Run and OSInterrupt want nopeephole). About 5.7 KB.
    - __GXInitGX (2,360 B) needs GXInit.c's data as TU statics.
    - symbols.txt: lbl_80437270 (end label inside fn_80437254) and the resetFuncRegistered$145 rename.
    - dolphin/ar/ar.c can become Matching (__ARChecksize matched).
  - Tool notes for the owner:
    - sort_functions.py moved type definitions below their first use in game/tu_80341B78 (needs a guard).
    - rename.py hung 12 minutes on dolphin/ar/ar.c (needs a timeout).
    - integrate.py loses push races when verify takes 10-20 minutes and main moves every few minutes; my merges needed up to 6 attempts.

- 2026-10-13 20:15 CDT | Sole-integrator matching batch: root18, local60 and coordinator63 add seven exact functions /908 code bytes versus publishedec45a02a. Retail control-flow evidence corrects front-end unconditional-success returns and request-status equality; big-endian byte access recovers audio clipping. Canonical request-provider declarations preserve all25 other compiled functions in the wrapper unit. Full worker builds, zero literal mismatches and final committed-HEAD verifier gates pass. Stock integration verifies and pushes the combined batch under the shared lock; six workers continue fresh four-unit packets, no new source linking.


- 2026-10-13 20:45 CDT | Sole-integrator matching batch: root19, coordinator65 and local66 add four exact functions /976 code bytes versus publishedae6b3743. Common-refresh control flow, global-state load ordering, array-search pointer lifetime and old-selection virtual dispatch match the retail binary. Local64 and coordinator65 add only required one-line near-miss notes. Full worker builds, zero literal mismatches and final committed-HEAD verifier gates pass. Stock integration gates and pushes the combined batch under the shared lock. Six workers continue fresh four-unit matching packets; no new source linking.
- 2026-10-13 | Claude Opus 5.5 ("REA library usage" side session), closing notes | reference material and coordination | matched none this part; earlier tonight 16 functions / 17.1 KB (see the entry above) | Notes for whoever continues:
  - Tools added tonight (tools/match/README.md): twins.py (look-alikes, callee prototypes, substitution tables, EA names, datafile classes), linkcheck.py (3,515 callee names that won't link; the lead is fixing include/ in batches), refnames.py (EA names from 15 builds, CC0), datafile.py (EA tuning classes), tools/coord.py (Claude/Codex roster, claims, inbox, integration lock; docs/COORDINATION.md, AGENTS.md).
  - Knowledge docs: docs/GAME_DATA.md (datafile format, database tables and enums, schedules, standings, archives), docs/REFERENCE_SOURCES.md (every disc/repo/folder checked, verdicts, what's still worth finding), PATTERNS.md RE4 section + "Loop dumps".
  - With the lead: about 12 KB of SDK matches waiting on nopeephole unit splits (OSTime.c started; tu_804330D0, GXInit.c, OSRtc/OSReset tails; per-section drafts in the session scratchpad, sdk1/split_drafts); linkcheck declaration batches; no size cap on the next Sonnet queue (twins.py rank by callee_cov).
  - Owner decisions recorded tonight: Fable 5.1 when needed; no codeless asm; Claude/Codex coordinate via tools/coord.py; names from related EA builds count as tier-E2 evidence.
  - Best remaining reference hope: MVP Baseball 2004 for GameCube (owner's copy), then demo discs and MVP 06 NCAA (PS2).
  | Owner: nothing pending from this session

- 2026-10-13 21:15 CDT | Final Codex wind-down matching batch: local61, local69 and B12 add five exact functions /2808 code bytes. Source field snapshots, row-base lifetime, native bounded iterators and nested DB scans match retail. ROOT21 adds the provider-grounded shared sound-copy callback contract with zero matching gain. Required one-line near misses from batches61/68/69/71 are retained; obsolete notes for newly exact functions are removed. Worker full Ninja builds, zero literal mismatches and committed-HEAD stock verifiers pass; unchanged inherited allocation-name unresolved imports are disclosed in worker evidence. Combined stock integration gates and pushes under the shared lock. Owner requested a usage-saving stop: no new functions or batches; successful work committed and unsuccessful drafts restored.

- 2026-10-13 | Claude Opus 5.5 ("claude-side-docs" side session) | docs refresh, no unit | matched 0, abandoned 0 | None. Owner asked for the README and docs to be brought up to date, with their note at the top of the README. Changes, docs only:
  - README.md rewritten: the owner's note at the top; current numbers and a per-library table; toolchain, build, gate, who does the work (Claude lead/workers/Fable, Codex, coord.py, scheduled jobs), rules, phase status, layout, docs index and credits. The old setup-kit sections are gone.
  - Status banners on the 2026-10-10 snapshots (RECON_REPORT, UNITS, REUSE, 00_KICKOFF); phase status in PHASES; a "what's left by size" table in SPEEDUPS; the PROGRESS snapshot refreshed.
  - prompts/10_WORKER.md now describes the current flow (coord.py claims, verify, push under run-locked) instead of PRs and GitHub unit issues; 20_AUDIT samples commits instead of PRs.
  - Fixed double-encoded characters in DECISIONS.md and tools/match/README.md. Added status notes, without moving anything to Decided, to three open questions that PR #135 (d71a9076) settled on the verifier side: the pragma allowlist, cross-unit moves, and the dvdfs/dvd split. The card/ax/mtx splits on `staged/sdk-resplit` and the tu_804227D4 split are still not done.
  - "private" wording removed from AGENTS.md and prompts/11_TEAM_WORKER.md (DECISIONS 2026-10-13); HANDOFF.md points at `ptitty12/mvp2005-decomp`. The lead-owned briefs in tools/team/lead/worker_prompt_*.md still say "private"; claude-lead was told.
  - Tool READMEs: check_literals.py, passthru.py, branch_targets.py (match) and units.py (recon) added; the swarm README's done item marked.
  | Owner: nothing new. The 125 still-open provisional unit issues on GitHub are unused (claims live in coord.py); closing them is a possible cleanup.
- 2026-10-13 | Claude Opus 5.5 (lead) + 11 Claude Fable 5.1 workers, 21:28-22:12 CDT | Fable hard-function pass on game/ and animation/ (owner: "go hard with Fable on the harder ones") | 9 functions matched, all merged to main by 23:10. Spun down early: the all-models weekly limit hit 100% at about 22:10.
  - Setup:
    - 10 workers on parked near-misses and big unmatched functions, 3 units each (queue/hard_NN.txt, template tools/team/lead/worker_prompt_hard.md);
    - 1 worker on the provider-contract declaration fixes Codex asked for.
  - Matched:
    - fn_802A1790 (animation/goals/tu_8029EE50), fn_802C0050 (game/tu_802BF974);
    - fn_80249D44 and fn_80248370 (game/tu_80246D04), fn_801852A4 (game/tu_8018423C);
    - fn_801971B8 (game/tu_80194D44), fn_802F35A4 (game/tu_802EFF28);
    - fn_8020C5FC (85.1% -> 100%) and fn_8020C9CC (game/tu_8020ADC4).
    - 6 of 11 workers matched at least one function.
  - Cost and yield: the Fable weekly limit went from 5% to 22% and all-models from 91% to 100%, in about 40 minutes. That is about 1.3 matches per worker-hour on parked functions, where Sonnet's earlier near-miss batches got about 9 on easier ones. Fable showed its value in explanations, not throughput: every unmatched function now has a root cause in its notes, and several general compiler rules came out of it (PATTERNS "Fable hard-function pass").
  - Contract worker (no commits, evidence complete): exact edits for fn_800B7A94/7AB8 (pointer result), fn_8007F198, fn_8007BB28 (float), fn_8007FB40 (unsigned), fn_8007F524, fn_80077D88, fn_80071EF8, fn_8007824C, with symbols.txt renames. fn_803FFA60's exact prototype waits on a Codex caller (db/tu_8053CC9C.cpp:676) that passes an int as the source pointer. Details: docs/notes/fable-2026-10-13/W12.md.
  - Source bug found: fn_8028C6B8 is missing `case 1:` (retail routes m48 == 1 to the case-0 body).
  - Process problems found and fixed:
    - verify fails on a leftover `build/_baseline/wt` from an interrupted run; integ.sh now clears it inside the lock.
    - Two of the lead's own integrations under the same agent name ran at once: `coord.py run-locked` treated a second run under the same name as re-entrant, and the first to finish released the lock while the other was still verifying. Fixed: `run-locked` is no longer re-entrant, so a second run under the same name waits (tested with two overlapping runs in a scratch COORD_DIR). The explicit `lock` command is unchanged.
  - Notes for every function attempted: docs/notes/fable-2026-10-13/.
- 2026-10-14 | Claude Opus 5.5 ("claude-side-goals" side session) | repo reframe around the owner's end goals; no unit | matched 0 | None. The owner set the goals in chat (browser play, browser netplay with VPS matchmaking, easy roster/team mods, no tech debt) and asked for the repo to be rebuilt around them.
  - New: `docs/ROADMAP.md` (replaces `docs/PHASES.md`): the goal, two tracks, three stable mod contracts, milestones E0-E7 and K1-K5 with exits, ordered next steps, and what is paused.
  - New: `docs/ARCHITECTURE.md`: in-browser PowerPC-to-WebAssembly translation of the retail code; the runtime and SDK replacements; GX command stream to WebGPU; audio; virtual disc; roster/team packs as semantic patches; input-lockstep netplay over WebRTC with the VPS for lobby, signaling and TURN; decisions D1-D8.
  - New: `tools/goals/targets.py`, which measures K1-K4 from a build.
  - New: `engine/README.md`.
  - Rewritten: `CLAUDE.md` (goal, tracks, rules for both; name and contract commits) and `README.md`.
  - Updated: `AGENTS.md`, `DECISIONS.md` (three Decided entries; two open contract questions unblocked), the HANDOFF banner, SPEEDUPS, the worker brief, and references to PHASES.md.
  - Findings:
    - most K-target code is matched but unnamed;
    - the roster database is `/data/database/database.big`;
    - the game reloads its code overlays from `/data/ovlay.big` at runtime (same bytes as the DOL);
    - the disc carries two unreferenced, larger overlay builds (`ovlay_d.big`, `ovlay_m.big`), noted in REFERENCE_SOURCES as a possible naming source.
  | Owner: nothing pending. D1 (adopt or write the translator) is decided by the E0a survey.
- 2026-10-14 | Claude Opus 5.5 ("claude-side-k1b" side session) | K1b: SDK names held back by K1a, on branch side/k1b-1014 (not pushed; for the lead to integrate) | matched 0 (21,109 before and after), named 58 K1 functions + 34 more functions and 18 data objects via dtk | K1: 237 -> 295 of 299 named.
  - New tool: `tools/goals/rename_symbols.py` (+ tests). Renames symbols.txt, src/, include/, config.yml relocation targets and the alias table, byte-preserving and idempotent; `--from-dtk` runs dtk's split to a fixpoint and propagates every rename dtk's built-in SDK signatures make. `tools/goals/renames/k1b.tsv` holds the 58 explicit names with evidence; the name commit is regenerated from it with one command, and `renames/k1b_applied.tsv` (all 110 renames) re-run on a later tree renames any merged source that still uses an old name.
  - 12 contract commits made the placeholders' local prototypes agree with the SDK headers before the rename (exception handlers, OSEnableInterrupts, OSSetErrorHandler, DBPrintf, DVDGetCommandBlockStatus, GXSetFogRangeAdj, DVDGetCurrentDiskID, GXSetBreakPtCallback, GXSetDrawSyncCallback, GXInitLightColor by value, GXSetViewportJitter parameter order, OSHeapHandle); every object byte-identical. os/tu_804321CC's typedef AlarmQueue became OSAlarmQueue.
  - Evidence: all 110 renames (58 explicit + 52 by dtk) match mitsevox/mvp2005's name at the same address; 49 of the 58 have a dtk signature, 55 are in RE4's dolsdk2004 symbols.
  - check_literals now resolves dtk's `<name>_<ADDR>` spellings of scope:local symbols (otherwise K1b would have silently dropped 40 checked references).
  - symbols.txt: LastResetEnd, __OSStartTime and __OSLastInterruptTime are single 8-byte OSTime objects (the overlap that broke dtk's split for OSInit/DVDInit).
  - Held back: OSInit and DVDInit (their dtk cascade renames 150 more symbols: 82 functions, 68 data; needs six more contracts, the dvd.c BB2 overlay taken apart, and data splits for the linked fstload and EXIBios units whose statics dtk makes scope:local, including the function-local block$16); __init_user (dtk then names _ctors at 0x80649298, which SN's libgcc already uses for .ctors at 0x805A2BC0); fn_8040405C (no evidence).
  - Note: dtk makes signature-known statics scope:local and writes them as `<name>_<ADDR>` in the split asm when another object uses them. Units that reference such a name and are not linked are fine; linking one later needs its data split into it (GXResetFuncInfo in gx/tu_80428F14 is already in this state).
  | Owner: nothing pending.
- 2026-10-14 (afternoon) | Claude Opus 5.5 ("claude-side-goals", acting lead) + 11 subagents | first day of the goal-driven plan | **matched 21,109 → 21,141 (+32)**, all merged and verified.
  - **K1 (SDK names) 48% → 99%.**
    - K1a: 94 names from mitsevox/mvp2005 (same DOL, CC0) plus RE4 and source-comment evidence.
    - K1b: 58 more, plus dtk's signature cascade (34 functions, 18 data objects) and 12 contract commits that align prototypes with the SDK headers.
    - New tools: `tools/goals/rename_symbols.py` (re-runnable, `--from-dtk`), and check_literals now resolves dtk's `<name>_<ADDR>` local spellings, which checks 5 more references.
  - **Research landed:**
    - `docs/engine/E0_SURVEY.md`: D1 is to write our own translator; D4 is resumable guest threads.
    - `docs/DETERMINISM.md` (K3/K4): **gameplay is natively 60 Hz**; 65 nondeterminism sources; engine requirements R1-R17.
    - `docs/ROSTERS.md` and `tools/rosters/` (K2): the database tables are RefPack-compressed **text**, round-tripped byte-exact; capacities 3,250 players, 1,800 pitchers and 126 teams.
  - **K-target matching:** 5 Sonnet workers (k_01..k_05) matched 32 functions in K2/K3/K4 units. Several K3 functions read `mftb` and can't be matched under the no-asm rule.
  - **Running at the end of the day:** E0b (native boot spike; Opus, worktree `decompile_mvp_04-e0b`, branch `engine/e0b`); the names pass (W1, `work/names-01`); roster/team pack format v1 (`side/packs-1014`); team-slot asset mapping (`side/teamassets-1014`).
  - **Held back:** OSInit and DVDInit names (they need data splits for fstload and EXIBios); `__init_user` (dtk relocates `_ctors`).
  | Owner: nothing pending.
- 2026-10-14 | Claude Opus 5.5 ("claude-side-packs" side session) | E6/E7 foundation: roster and team pack format v1, on branch side/packs-1014 (not pushed; for the lead to integrate) | matched 0 | No milestone exit yet (E6 needs the engine loader); the format and its reference implementation are done.
  - New: `docs/MODDING.md`, the v1 contract. It covers:
    - the manifest and pack layout;
    - 12 operations: player add, edit, remove and rename; roster set, replace and swap; team names and team edit; manager edit and rename (also by team code, so a team pack never names the retail manager); LOC text;
    - `team.names` sets `team.dat` and every LOC string of a team, finding the ids in the player's own `main.dol` through the slot tables of ROSTERS.md §12;
    - asset mappings to existing BIG entries under the file-overridable roots (frontend, models.big, stadium, announcer audio). Rebuilds keep each archive's payload alignment, so the 64- and 128-byte aligned announcer archives now rebuild byte for byte;
    - section 13 (team slots), filled from ROSTERS.md §12: what v1 can replace, and what it can't (home park, tid list, LOC-id tables, uniform letters; load screens are global);
    - static and full validation with stable error codes;
    - composition: a pack may overwrite or use another pack's cells only if it depends on it;
    - canonical serialization and the pack, set and result hashes;
    - the semver policy (minor versions only relax or add);
    - a worked example and open questions for a boot.
  - New: `tools/rosters/pack.py` (validate, apply, hash, new) and `tools/rosters/tests/test_pack.py` (41 tests on a synthetic disc with a synthetic `main.dol`; all 64 roster tests pass). `big.py` gained `payload_alignment()` and an `align` parameter.
  - New: `docs/examples/packs/fictional-league/`, which replaces the Anaheim slot with an invented club (25 players, full depth chart, names, manager).
  - Evidence used for the roster rules: the `roster.dat` loader in `src/db/tu_8053967C.cpp` keeps at most 25 ids per team, at most 3 LR, 4 MR, 2 SU and 1 CP per lineup, and reads pitching roles from the first lineup column only.
  - Local end-to-end on GV4E69, nothing committed:
    - the example applies with 0 errors and 9 warnings;
    - `team.names` hit exactly Anaheim's ids 2010, 2050 and 7210 (both LOC files) and 17618 (FEENG);
    - every untouched retail row, the other 125 rosters and `tstat`'s stored bytes are identical;
    - two runs, and an image versus an extracted directory, give identical bytes;
    - `database.big` grows from 1,010,678 to 1,056,803 bytes.
  | Owner: nothing pending. Boot questions for E2/E6, in MODDING.md §14: a larger, recompressed `database.big`; roster sizes other than 25/22; players with no team.
- 2026-10-14 | Claude Opus 5.5 (`claude-w2`, side session) | K1c: OSInit and DVDInit, on branch work/k1c (not pushed; for the lead to integrate) | matched 0 (21,141 before and after), names held | K1: 295 of 299 named, unchanged.
  - Landed on work/k1c (each commit built with the retail hash; compiled objects byte-identical except where noted; VERIFY PASS and check_literals 0 at the head):
    - six contract commits so the placeholders' prototypes agree with the SDK headers: __OSMaskInterrupts (u32), DVDInquiryAsync (DVDDriveInfo*, DVDCBCallback), OSDefaultExceptionHandler, __OSErrorTable (OSErrorHandler[17]), the three __OSLastInterrupt* variables (volatile: fn_8043402C 98.46% -> 99.53%), EXIImm/EXIDma (EXICallback);
    - dvd.c's BB2 overlay without its `#define BB2` macro; fn_8042515C's parameter renamed so it can't shadow the static autoInvalidation;
    - .bss data splits for the two linked units: fstload.c (bb2Buf, __fstLoad's static block) and EXIBios.c (Ecb);
    - stateBusy's switch table in dvd's split, so that dtk keeps the static's name once it is local;
    - `tools/goals/renames/k1c.tsv` with the evidence for both names.
  - Held: the name commit. The dtk cascade has 152 renames (2 explicit, 150 by dtk: 82 functions, 68 data). It needs two things only the owner or the lead can change:
    - common-symbol modeling for errno.c and eleven ealib units, so that fstload.c and EXIBios.c can own their .sbss statics;
    - four linker-defined symbols in the protected ldscript.ld.
  - Details and options: docs/DECISIONS.md, 2026-10-14 K1c entry.
  - Staged on local branches, not for merge as they stand:
    - `staged/k1c-scommon`: the common-symbol fix and the .sbss splits (VERIFY PASS);
    - `staged/k1c-names`: the generated name commit. It does not link until ldscript.ld defines the four symbols. Its objdiff report has 21,141 matched and no drops, and OSInit goes from 96.5% to 97.4%.
  | Owner: the K1c entry in DECISIONS.md "Open questions for owner".
- 2026-10-14 (evening) | Claude Opus 5.5 ("claude-side-goals", acting lead) + engine agents | engine track | **The game boots, renders and plays sound natively, and boots in the browser.** Everything is on main with gates passing.
  - **E0b:** the retail code is translated by our own translator and boots deterministically to the frame loop.
  - **E0c:** resume labels (0.01% interpreted), the per-frame input queue (the netplay hook), AX voice state.
  - **E1:** GX → Dawn/WebGPU. Loading screen, EA logo, intro, title screen, menus.
  - **E3b:** audio. D5: DSP command lists; mixer plus SDL3; the mixer is read-only.
  - **E2:** a WebAssembly emitter (whole DOL translated in the browser in about 0.5 s, 87% of native speed). The page runs in Chrome, Edge and Firefox, with guest hashes equal to native.
  - **K1c:** ldscript.ld linker symbols, owner-approved; one line of protected.sha256 refreshed. The OSInit/DVDInit names (`staged/k1c-names-v2`, c0e42e51) wait on a verify.py false positive; the fix is an open owner question.
  - **Owner decisions:** the toolchain downloads; the private, password-protected deployment may serve the owner's disc image.
  - **Running at the end of the session:**
    - E3a (native speed `/O2`, into an exhibition game; worktree decompile_mvp_04-e1, branch engine/e3a);
    - E5 (netplay lockstep plus server: login gate, disc route, lobby, signaling; decompile_mvp_04-e5, engine/e5);
    - E2c (browser audio, gamepad, disc from `/config.json` → `/api/disc/image`, caching; decompile_mvp_04-e2, engine/e2c);
    - E2d (wasm emitter FP/psq/lmw inlining and speed; decompile_mvp_04-e2d, engine/e2d).

    Each writes `docs/engine/<id>_LOG.md`, so a fresh session can resume from those logs.
  | Owner: optional, a verify.py fix for name reuse across statics (unblocks OSInit/DVDInit). Later: VPS deploy access.
- 2026-10-14/15 (night) | lead + agents | engine and deploy | On main:
  - **E3a:** natively the engine reaches an exhibition game, through the first pitch and first ball in play. Fixes: the FP-unavailable exception (the movie specks), the write-gather/WPAR fixes, /O2 game code (4-6× faster), FP shortcuts proven equal.
  - **E2c:** browser audio, gamepad, disc from the server, translation cache.
  - **E5/E5b:** lockstep netplay plus the server (password gate, first-run password page, disc route, lobby, WebRTC signaling, TURN).
  - **Deploy:** Dokploy compose (`deploy/docker-compose.yml`, guide `docs/engine/DEPLOY_DOKPLOY.md`). The owner deployed it to mvp.ptaylor.dev, and it boots.
  - **Lead state for resuming:**
    - E2d (wasm emitter: FP/psq/lmw inlining plus E3a's FP-unavailable and WPAR semantics; worktree decompile_mvp_04-e2d, branch engine/e2d) is running. Until it lands, `test_wasm_lockstep` is DISABLED and the browser build doesn't match native in-game.
    - Next wave: E3c (full 9-inning game natively, the E3 exit; worktree -e1, engine/e3c) and E6 (roster packs in the engine; worktree -e2, engine/e6).
    - Integration: `git merge --no-ff engine/<x>` onto origin/main in decompile_mvp_04-docs, record decisions, then `python tools/coord.py run-locked claude-side-goals ... -- python tools/team/integrate.py --push --head-only`.
    - Engine merges that conflict are delegated to a merge agent, which builds and tests both toolchains.
  | Owner: deployments only at milestones (owner, 2026-10-15). Optional: the verify.py fix for OSInit/DVDInit names.
- 2026-10-15 (evening) | lead + agents | engine: quality pass after owner play tests | On main:
  - **Q1 (menu/intro flashing, picture):**
    - The renderer now reads memory as the CPU cache wrote it back (Q1-G1). Menu flicker went to 0.
    - The picture: 2×/3× resolution (Auto), 4:3, better scaling (Q1-G2).
  - **Q2 (sound, weaker machines):**
    - The sound goes through a shared-memory ring and is time-stretched instead of going silent. Underruns per minute: 7.2 → 0.7 on this PC, 251 → 0.5 at a simulated 2× slower machine.
    - Weaker machines are limited by the emulation thread: 0.69× speed at 2× slower.
  - **Q3 (controllers):** Xbox, PlayStation, Switch Pro, GameCube adapters (guided setup) and generic pads; a Controls panel with remapping; per-player controllers online. Rumble from the game is not done.
  - **Running:** Q4, emulation-thread speed for weaker machines (worktree decompile_mvp_04-e2, branch engine/q4-speed).
  | Owner: redeploy once in Dokploy to get Q1–Q3 (players re-translate once). Optional: the other computer's CPU and browser, to target Q4.
- 2026-10-16/17 | lead + agents | engine: speed and online play between unequal machines | On main:
  - **F1 (owner):** the engine need not be bit-exact to the GameCube, only deterministic across players.
  - **Q4:** emulation 11.4-12.5 → 9.2-10.0 ms per field on the dev PC (i7-14700K).
    - Laptop stand-in (hostSlow 1.3) 0.94× → 1.000×; hostSlow 2 0.65× → 0.8×.
    - Levers: GX pipe path, FPR cache, fast FP model, the first native game function.
  - **Q5:** even lockstep pacing; Auto input delay; the Stats overlay with Copy stats; GPU-aware Picture Auto; a slow-machine note.
  - **Q5b:** pacing roles. 1.0 vs 1.3 online now runs at 0.975×, with the pace at 1.0 about 91% of the time; 1.0 vs 2 runs at 0.76× and smooth.
  - **The owner's field test:** desktop + ThinkPad (Ryzen 7 PRO 7840U) online was choppy and slow before Q4/Q5.
  - **Running:** Q6, more emulation speed (worktree decompile_mvp_04-e2, branch engine/q6-speed). Targets: hostSlow 2 ≥ 0.95×, 4 E-cores 1.0×, ≤ 10 ms per field at hostSlow 1.3.
  | Owner: redeploy, then paste Copy stats from both machines after an online game.
- 2026-10-17 | claude-side-packs | engine: server-hosted roster packs (owner request) | On main:
  - The deployment can host roster packs: the owner uploads a pack zip on the Server disc page (`/disc`, new **Roster packs** card: list, Remove, Upload), the server keeps it in `MVP_PACKS_DIR` (Docker: `/data/packs`, same volume and login as the disc), and the game page's Roster packs panel offers it in a **Packs on this server** dropdown. Choosing one adds it exactly like a picked zip (same checks, same netplay set hash); the page remembers it per browser, and the game start waits for a pack download in progress.
  - Server: `engine/server/src/pack_store.ts`, `/api/packs` routes (GET list and zip with ETag; POST/DELETE with the disc page's CSRF token). Tests: 72 server, 91 web; Chrome on the real server: the MLB 2026 pack from the dropdown gives the same result hash as the file (`464d7267...`). Guide: `docs/engine/DEPLOY_DOKPLOY.md` §8.
  | Owner: redeploy in Dokploy, then upload the MLB 2026 zip on the Server disc page.
