# V4 log: the emulation thread, faster again

Side session `claude-side-emu`, branch `engine/emu-speed` (from origin/main `31aa85fc`: V1, V2).
The owner, 2026-10-21: "Optimize optimize optimize, I am still dropping frames on some
devices, do whatever you absolutely can to make this run as efficiently as possible."
This session's part: the **emulation thread** (translator, runtime, the emulation side of
the threaded engine). Other sessions of the same day had the render thread (V5) and the
in-between frames (V3). Same PC and tools as Q4/Q6 (i7-14700K, Windows 11, Chrome 155
headed through Playwright, Node 24 for the translated code alone); every timing run held
the measurement lock (`mlock.py`, V1 section 1).

## Status

| # | Lever | Commit | Gain |
|---|---|---|---|
| L1 | The native skinning code's completing pipe stores skip `Hardware::write` | `0873cc3d` | Node about -0.7 % |
| T1 | The runtime's addresses and MEM1 as constants in the translated module | `fc1305e2` | Node -5.1 %; Chrome (with L1) -8 % |
| FS | Calls reach functions without resume labels; resumes take resumable copies | `1a57afb3` | Node -5.6 %; Chrome -3.7 % |
| LP | Loops as wasm loops in functions without resume labels | `5b0b3518` | Node about -2 %; Chrome within noise |
| GR | GpReads gets only the commands it looks at | `130c4d78` | within noise |
| SN | Snapshots copy ARAM and the card by pages; statistics stay put | `201a0012` | per-pitch snapshot about 6.5 -> 1.4 ms (Chrome) |
| SP | Snapshot slots made with the machine; state files load into a slot | `27ad53c9` | longest snapshot 6.5 -> 3.7 ms |
| HC | Heap code (the game's run-time generated code) translated while the game runs | `7466736f` | Node -2.1 %; interpreted instructions -85 % |
| NC | The native skinning blend makes NaNs canonical where values leave | `100208b0` | Node about -1.3 % |
| CW | The translation cache written 4 MiB per task | `c79659ec` | no stop of the game for the whole write on a cold start |
| - | Fixed Cpu address, inline MEM1 loads, an entry table for indirect calls in the module, V8 flags | tried, dropped | section 8 |

**In short.** Chrome threaded, the ABS scene paced 58 s (section 9, the final build against
origin/main): emulation per field on this PC 7.69 -> 6.70 ms, hostSlow 1.3 9.89 -> 8.65,
hostSlow 2 15.12 -> 13.41, 4 efficiency cores 13.54 -> 11.88: -11 to -13 % everywhere, and
on the two slower configurations far fewer pictures held over a refresh (hostSlow 2 2.5 % ->
0.6 %, 4 E-cores 3.3 % -> 0.8 %; the E-cores' speed 0.993x -> 1.000x). Unpaced, the same
scene's emulation goes from 7.52 / 7.56 to 6.48-6.57 ms per field. The per-pitch
snapshot of the ABS challenge, the longest field of play on a slow machine, goes from about
6.5 ms to 1.4 ms on this PC (3.7 ms for a slot's first). Every guest line is unchanged
(section 10).

## 1. How it was measured

- **The scene** (V1 section 1): the save state `braves-dodgers-9th-bat` (top of the 9th, the
  ABS challenge on, the MLB 2026 pack; player 1 bats and never swings). `run_scene.sh` in the
  session's scratch folder (a copy of the lead's with this worktree, its own Chrome profiles
  and output folder). Unpaced (`--fast`) from the state to field 139,500 for A/B (about 20 s
  of play, ms per field from field 136,700); paced 58 s for the final table. The
  translation cache never hit between runs (the mock server's port changes, so the origin
  and its storage do); `--runs 2` checks the cached path.
- **Node** (`mvp-boot-wasm`, the translated code with the runtime, no renderer): a tree of
  its own (`build/engine/em-node`, `-DMVP_DISC=...`, only that target), a "Play Now" input
  script from power-on (START at polls 400, 700, 1300; DOWN 1460; A 1520; DOWN 1960; A 2020,
  2160, 2400, 3200, 3400; then A every 100 frames: player 1 pitches from frame ~3,500),
  thread CPU per frame between the time-at marks of frames 4,000 and 7,500. Pinned pairs as
  Q4-M1: two runs at once on the E-core clusters `F0000` and `F00000`, swapped for the second
  pair. Toggles in one build where possible (`MVP_WASM_FIXED=0`, `MVP_WASM_FAST=0`,
  `MVP_WASM_LOOPS=0`, `MVP_WASM_HEAP=0`), so the pairs compare code paths, not builds. The two
  clusters differ by up to 3 % (other sessions' load); single pairs under 2 % are noise.
- **Profiles**: the named profiling build (`build_prof.cmd`: `-DMVP_WEB_NAMES`,
  `--profiling-funcs`), paced, `--trace 137500:10000`, `trace_summary.mjs`, and a call-tree
  script for inclusive times under a function (`tree.mjs`, scratch).
- **V8's code**: `node --print-wasm-code-function-index=N` prints Liftoff's and TurboFan's
  machine code of one function (N from `wasm-dis` of `mvp-xlat emit-wasm --names`; `--fast`
  and `--loops` give this session's module).

## 2. Where the time went (at the start)

Chrome trace, named build, paced, 10 s from field 137,500 (about 600 fields). The emulation
thread was busy 5,009 ms (49 %): translated code 54 % of that, the engine 43 %.

- **Translated code** 2,700 ms: the memory helpers 954 ms (`h.wr32` 267, `h.rd32` 199, `h.lfs`
  140, `h.stfs` 61 ...), the game's functions 1,929 ms (`fn_80369878` 143, `fn_803E3C08` 92,
  `fn_803BD358` 49 ..., a long tail).
- **The GX path** about 1,300 ms: `Hardware::gp_consume` 749 ms inclusive (the observer's
  batches for the render thread 450, of which `GpReads::command` 240; `GxFifo` 150), the
  pipe's bursts from translated code (`rt.wgp_store`) 539, the native skinning loop's stores
  through `Hardware::write` (164 self) and `Stores::word` (149).
- **The native skinning loop** (`hle_fn_803EA3EC`) 897 ms inclusive, its arithmetic
  (`run_803EB000`) 331 self.
- `rt.call_indirect` 112, the interpreter (code the game writes at run time) about 120, the
  guest's interrupt handler inside safe points 348 (guest work).

What the translated code cost beyond the guest's work, read from V8's machine code:
- every MEM1 access went through four dependent loads before the data (the instance's
  imported-globals table, `env.mem1`'s address, its limit, its base);
- every function with a call or a loop (19,000 of 24,000) was one wasm `loop` around its
  whole body with a `br_table` dispatcher (for resume labels): every call entered through the
  `br_table`, every local was a phi of that loop, and TurboFan zeroed about 20 spill slots at
  each entry (38,000 calls per field of play, counted with the entry hook in Node);
- V8's wasm calls have no callee-saved registers: every value live across a helper call is
  spilled around it. That is the price of helper calls (section 8).

## 3. The translated code

**L1** (`0873cc3d`). `Hardware::wgp_store` appended inline only while a store left the burst
unfinished; the completing ones (and every `psq_st` half that reached into the next burst)
went through `Hardware::write`. With `fast` set (the access log full, as for the module's
`rt.wgp_store`) it takes `write()`'s pipe path directly: count, end the poll detector's run,
set c.pc and c.ticks, `wgp_write`. `fp::double_to_single_ftz` is inline. Exact: `write()`
does the same for a pipe store once nothing is logged; `local_hle_native` compares every
store's PC and ticks with the interpreter. Node 9.330 / 9.308 -> 9.205 / 9.299.

**T1, fixed addresses** (`fc1305e2`). The browser's engine emits its module in its own
process, so the addresses the module read through imported globals (`g_unwinding`,
`g_resume`, `g_entry_hook`, the pipe's state, `ppc::g_gp_pages`, MEM1's view) are known
there: `WasmEmitOptions::fixed` (`xlat::WasmFixed`) puts them in the code as constants. A
MEM1 access is a compare with a constant limit and a load or store whose offset immediate is
MEM1's base; a gp_pages check is one load with the table's address as its offset. MEM1 sits
in static storage in `mvp_web` (`Machine(u8* storage)`), the same address in every run of a
build; `rt::load_wasm_module` refuses a module whose fixed addresses are not the runtime's (a
cached module of another build is retranslated); `set_wasm_mem1` aborts if a machine's MEM1
moves away from a loaded module's. The cache's blob carries the addresses (format 3). Node
`MVP_WASM_FIXED=0 / 1`: 9.362 / 9.401 -> 8.763 / 9.045 (-5.1 %). Chrome (with L1): 7.52 /
7.56 -> 6.89 / 6.95.

**FS, fast entries** (`1a57afb3`, `WasmEmitOptions::fast_entries`). A single-entry function
with resume labels is emitted twice. The **fast** one is what calls and the entry table
reach: no resume labels, so an unwinding return (`c.pc` not the return address, or
`g_unwinding`) leaves instead of resuming in the frame, and it needs the dispatcher only for
its loops or jump tables. The **slow** copy is the function as before, registered only for
resume entries (`rt::g_resume_labels`). Native frames hold no guest state, so where an
unwind ends changes nothing the guest sees (runtime/native.h): the unwinds a frame used to
end in place now reach the dispatcher, which re-enters the slow copies (Node, 8,547 fields:
unwinds 66K -> 137K, resume entries 353K -> 701K, in-frame resumes 71K -> 0; interpreted
instructions the same). The module doubles (58 -> 112 MB; emit 0.39 -> 0.67 s in Chrome;
the cache's blob 115 MB). So that the engine's heap does not grow with it, `emit_wasm`
writes the bodies straight into the module's code section (its size patched as a 5-byte
LEB128); `load_wasm_module` can keep the JavaScript copy it compiles from as
`Module.mvpModuleBytes`, so the translation frees its own bytes at once and the Worker writes
the cache from that copy (header and bytes as two parts); a cached blob is compiled in place
(`deserialize_module_tables`). Chrome's heap at the end of a run: 685 MB before, 822 MB with
the copies alone, 685 MB with these, cold start and cache hit alike. Node `MVP_WASM_FAST=0 /
1`: 9.018 / 8.889 -> 8.518 / 8.397 (-5.6 %). Chrome, interleaved: 6.85 / 6.92 -> 6.59 / 6.67.

**LP, structured loops** (`5b0b3518`, `WasmEmitOptions::structured_loops`). About 3,000 fast
functions still had the dispatcher for their loops, among them most of the hottest
(`fn_80369878`, `fn_803E3C08`, `fn_8036CC08`, `fn_803802A0`, the interrupt handler
`fn_80435460`). A segment that later segments branch back to now starts a wasm `loop` that
runs to the last of them, where the loops nest and nothing enters one but at its start
(`FuncGen::plan_loops`); forward branches stay blocks that end before their target, opened at
the level of the loop that holds it (`emit_region`). 1,973 of the 2,950 became structured;
the rest (overlapping loops, a loop entered at its condition) keep the dispatcher. Node
`MVP_WASM_LOOPS=0 / 1`: 8.205 / 8.290 -> 7.915 / 8.223; Chrome within the noise.

**HC, heap code** (`7466736f`). The game writes code into its heap and calls it through CTR
every frame: 26 generated routines in play (calls to the game's functions with
lis/ori/mtctr/bctrl and the results stored, 150 to 5,800 instructions each, all called from
one site, 0x803A7BC8). The interpreter ran them: about 17,000 instructions a field in the ABS
scene. Now `rt::call_indirect` asks `rt::g_heap_code_hook` (`rt::set_wasm_heap_code`) before
it interprets. On a target's third call the code reachable from it (branches followed;
within 64 KiB of MEM1; no direct calls out of it) is translated as a module of its own: a
one-section DOL image of those bytes, the translator's CFG, the emitter with
`WasmEmitOptions::heap_code` (no resume labels, no entry prologue) and the CFG's back edges
and idle spins cleared, so there are no safe points in it, exactly as in the interpreter,
which has none outside the program. It is called while its bytes are the same as when it was
translated (a memcmp per call); other bytes there are interpreted again and translated anew
later. At most one translation per 8 ms of host time (when, never what, the guest runs). No
blob changed or wrote into itself in play (checked with a temporary byte comparison around
every interpreted call). Node `MVP_WASM_HEAP=0 / 1`: 8.192 / 8.058 -> 7.960 / 7.946 (-2.1 %);
interpreted instructions 76.7 M -> 11.5 M; 26 translations, 14 ms in all, the longest 1.4 ms.

## 4. The native skinning blend (NC, `100208b0`)

`fn_803EB000`'s native blend applied `fp::fast_result` to every multiply and multiply-add
(about a hundred per call, 1,700 calls a field): the single rounding and a check that makes
any NaN the default one. The check now happens where a computed value leaves for guest state
(the FPRs it writes and the pipe's `psq_st` words). Every step is still one IEEE operation
and the single rounding, and each gives a NaN for a NaN operand and never turns a NaN into a
number, so a value is a NaN exactly where the model's is, and at the exits it gets the same
bits. Loaded values (the one-matrix path's rows) keep their bits. `local_hle_native` (the
original from `main.dol` in the interpreter against the native code, random operands with
NaNs and infinities): 22,000 trials, no difference. Node about -1.3 % on each cluster.

## 5. The GX path

`GR` (`130c4d78`): `render_gx_batch` called `GpReads::command` for every command of a push;
for most (the skinning's XF loads, NOPs) it does nothing. `GpReads::reads_something(op)`
names the ones it reads, and the batch skips the rest. Within the noise in Chrome.

Not changed (measured or reasoned, section 8): batching the translated code's bursts until
the next safe point (the next safe point is usually one burst away; every store, MMIO access
and safe point would have to flush first).

## 6. The ABS challenge's snapshots (SN `201a0012`, SP `27ad53c9`)

The gameplay mods' timeline takes a snapshot before every pitch: Machine (MEM1, the VM
region) and Hardware member by member. Hardware's 16 MiB of ARAM, the card's 8 MiB image
(when a card is in) and its statistics (the access log, 2 MB; the access counts; the pipe's
first megabyte) were copied whole each time.

- **By pages.** `Hardware::save_into` and `restore_from` copy ARAM and the card's image by
  pages (4 KiB; the card's 8 KiB sectors): the AR DMA (the one place ARAM is written) and the
  card's programs and erases stamp what they write with the current epoch; every save starts
  a new one; a snapshot taken from this machine before gets only the pages stamped since its
  own epoch (all of them when it came from elsewhere: a state file, a whole copy). A restore
  brings back only the pages written since that snapshot and stamps them new to the other
  one. The statistics and the bookkeeping stay with their object (set aside around the
  member-wise copy).
- **Made with the machine.** A slot's first snapshot still allocated and touched about 50 MB
  (6.5 ms in Chrome on this PC, the longest field of every run). `Timeline::prepare` (mvp_web
  and mvp-play, with mods on) makes both slots' buffers as the machine is built; the browser
  reads a save-state file straight into a slot (`Timeline::slot_for_load`) instead of a
  second copy of the machine, so the heap's high-water mark stays at 685 MB.

Chrome (the batting scene to field 139,500, 15 snapshots): before, about 6.5 ms each; after,
the first of each slot 3.7 ms at most, the rest about 1.4 ms, mean 2.0 ms. Native mvp-play:
mean 3.3 -> 2.1 ms, longest 7.0 -> 3.7 ms. Restores in a review 1.6-3.6 ms. The timeline
reports the snapshots' total time now (`snapshot_ms_total`; mvp-play prints the mean).

## 7. The translation cache (CW, `c79659ec`)

The Worker writes the translated module to OPFS once after the first frame of a cold start,
with a synchronous access handle on the engine's thread: the game stopped for the whole write
(now 115 MB). `writeFile` writes 4 MiB per task and yields between pieces.

## 8. Tried and dropped

| What | Measured | Verdict |
|---|---|---|
| The Cpu struct at a fixed address too (a constant instead of the parameter) | Node 8.710 -> 8.861 (one clean pair); module 58 -> 70 MB | slower, larger |
| Integer loads from MEM1 inline (constant base and limit, the page path as the else) | Node 8.397 / 8.080 -> 8.130 / 8.072; module +25 MB | no clear gain |
| Indirect calls through an entry table in linear memory and `call_indirect` in the module (7,000 a frame) | Node three pairs: mean 8.020 -> 8.106 | no gain (as Q4 found) |
| The native loop's psq_st words appended as one pair (`Hardware::wgp_store_pair`) | Node 8.023 / 7.799 -> 7.893 / 7.871 | within noise |
| V8's wasm inlining budget raised (`--wasm-inlining-budget=60000 --wasm-inlining-factor=10`) | Node 9.304 / 9.174 -> 9.536 / 9.353 | slower: V8's own limits are right |
| `--no-wasm-inlining` | inconclusive | - |
| Resume labels off, resumes in the interpreter (no slow copies) | Node 5.375 -> 7.397 ms per frame (single runs); interpreted 77 M -> 1,058 M | far slower: resumed frames run interpreted to their end |
| A pipe check in the store fronts | not built: one return_call saved per pipe store against a compare on every MEM1 store | - |
| Batching the translated code's bursts until the next safe point | not built (section 5) | about 2 % at best |

## 9. Results

Chrome 155 headed, the batting scene, 58 s paced from field 136,700, base (origin/main
`31aa85fc`) and this branch interleaved; "hostSlow N" is Q2's simulated slower machine
(`--host-slow N@200`), "4 E-cores" the browser on four efficiency cores (`--affinity
F000000`). Emulation ms per field inside `mvp_run_fields` (with hostSlow, its spin
included); render thread ms per field; pictures held over more than one refresh.

| Config | Build | Speed | Emulation ms/field | Render ms/field | Held >1 refresh | Lost/s |
|---|---|---:|---:|---:|---:|---:|
| this PC | base | 1.000 | 7.70 | 5.28 | 0.2 % | 0.09 |
| this PC | V4 (round 1, `7466736f`) | 0.999 | 6.80 | 5.14 | 0.1 % | 0.00 |
| hostSlow 1.3 | base | 0.998 | 9.86 | 5.25 | 0.2 % | 0.03 |
| hostSlow 1.3 | V4 (round 1) | 0.998 | 8.83 | 5.12 | 0.2 % | 0.09 |
| hostSlow 2 | base | 1.000 | 15.09 | 5.20 | 1.5 % | 0.14 |
| hostSlow 2 | V4 (round 1) | 1.000 | 13.57 | 5.09 | 0.6 % | 0.14 |
| 4 E-cores | base | 0.998 | 13.30 | 10.42 | 0.7 % | 0.21 |
| 4 E-cores | V4 (round 1) | 1.003 | 11.94 | 10.22 | 0.9 % | 0.45 |

Round 2, the final build (`c79659ec`) first, then base:

| Config | Build | Speed | Emulation ms/field | Render ms/field | Held >1 refresh | Lost/s |
|---|---|---:|---:|---:|---:|---:|
| this PC | V4 | 1.001 | 6.70 | 5.12 | 0.1 % | 0.02 |
| this PC | base | 1.003 | 7.69 | 5.26 | 0.2 % | 0.07 |
| hostSlow 1.3 | V4 | 1.002 | 8.65 | 5.07 | 0.3 % | 0.07 |
| hostSlow 1.3 | base | 1.001 | 9.89 | 5.22 | 0.2 % | 0.03 |
| hostSlow 2 | V4 | 1.001 | 13.41 | 5.11 | 0.6 % | 0.14 |
| hostSlow 2 | base | 0.998 | 15.12 | 5.20 | 2.5 % | 0.33 |
| 4 E-cores | V4 | 1.000 | 11.88 | 9.80 | 0.8 % | 0.31 |
| 4 E-cores | base | 0.993 | 13.54 | 10.56 | 3.3 % | 0.31 |

Reading it: the emulation thread needs 11-13 % less on every configuration. On this PC and the
laptop's stand-in (hostSlow 1.3) the game ran at full speed before too, with nothing held;
the margin under the 16.7 ms field grows by 1.0-1.2 ms. On hostSlow 2 and the four E-cores,
where the emulation used most of the field, the pictures held over a refresh fall to a
quarter and the E-cores reach full speed. The render thread is unchanged (its share is
V5's). Every run's guest line at field 140,200: 909,410,970,125 instructions, MEM1
`3777d8c7...`.

## 10. Exactness

- **The batting scene** at field 139,500: 904,870,434,125 instructions, time base
  94,257,336,888, MEM1 `bcf1d407...`: origin/main in Chrome, every commit in Chrome
  (threaded; cold start and cache hit), native `mvp-play --state-load` after each runtime
  change. Paced to field 140,200 (every final run, base and V4): 909,410,970,125 /
  `3777d8c7...`.
- **Reviews** (the close-call state with L+R+Z every 20 frames: three reviews, six restores)
  to field 141,000: 914,600,154,125 instructions, MEM1 `bd9daa73...` (V1's line), native, and
  Chrome threaded and single-threaded after the snapshot changes.
- **From power-on** in Chrome (the final build), threaded and single-threaded: 700 fields
  4,547,010,125 / `53221c4a...`, 2,400 fields (START at 280/430/580/730) 15,574,026,125 /
  `4285f135...` (WEB.md's lines).
- **Netplay**: `netplay_check` (two Chrome pages through the server, the E5c script, 3,200
  frames, delay 3, both threaded): both pages 26,835,905,005 / `2a8de818...` (native
  `mvp-netplay solo`'s line), no desync.
- **Renderer digests** (`--digest`, every field, the batting scene to field 138,000):
  threaded = single-threaded = origin/main's threaded in all 1,999 common fields.
- **Node** at frame 7,600 of the Play Now script: 55,447,992,226 instructions, MEM1
  `d263da26...`, every build and every toggle.
- **Tests**: `test_wasm_lockstep` (the synthetic program with fixed addresses, fast entries
  and structured loops; modules for other addresses refused; a program of loop shapes; code
  in a data section called and changed between calls), `test_web_cache` (the blob's fields),
  `test_gp_reads` (the commands GpReads ignores report nothing), `test_boot_hw` (snapshots by
  pages hold what whole copies hold, through DMA writes, card programs and erases, two slots,
  restores and a state file), `local_hle_native`. ctest msvc 50 (2 skip as before),
  emscripten 38.

## 11. What is left, and how to resume

- The render thread spends about 12 % of its samples in `performance.now()` and 8 % in futex
  waits (`render_thread.cpp`'s `spin_for_data()` polls up to 1 ms after catching up; sent to
  claude-side-render and the lead; not changed here).
- The GX path is still about a quarter of the busy emulation thread (`GpReads`, `GxFifo`, the
  observer's batches, the bursts); `rt.call_indirect` about 3 % (an entry table in the module
  did not help).
- Snapshots: MEM1 (24 MiB) and the VM region (8 MiB) are still copied whole; the VM region is
  written only through the page-table paths, which could stamp pages like ARAM.
- To resume: the scratch folder's `emu/` has the scripts (`run_scene.sh`, `run_boot.sh`,
  `final.sh`, `nrun.sh`, `ab.sh`, `abenv.sh`, `build_*.cmd`); the Node tree is
  `build/engine/em-node`, the profiling tree `build/engine/prof-mt`.
