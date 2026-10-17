# Q6 log: the emulation thread, faster again

Side session `Q6`, branch `engine/q6-speed` (from origin/main `10118e11`: Q4 merged, F1).
Same machine and tools as Q4 (i7-14700K, Windows 11, Chrome 155 headed through
Playwright, Node 24 for the translated code alone). The targets: 4 E-cores at 1.0x,
hostSlow 2 at 0.95x or better (stretch 1.0x), and the laptop stand-in (hostSlow 1.3) at
about 10 ms of emulation per field or less. The owner approved, under F1: dropping NI's
flush and canonicalizing NaNs only where bits leave the registers (lever 1), paired
singles as SIMD with WebAssembly SIMD as a page requirement (lever 2), MEM1 at a fixed
place (3), more native functions under Q4-N1 (4), and the GX parse off the emulation
thread (5). Lever 6 (run-ahead in the pacing) was Q5b's.

**Read this first: the human-pitching reference line changed.** Lever 1 changes what the
guest computes for results below the normal range, and the human-pitching session hits
one: its line at 9,000 fields is now MEM1 `caec614c...` (was `58631fe2...`). WEB.md's
checkpoints at 700, 2,400 and 3,600 fields and the CPU vs CPU line at 11,600 fields are
unchanged. WEB.md section 4 has the re-recorded lines (and section 12 here).

**Read this second: what was not done as approved.** NaNs are still made canonical after
every result, not only where bits leave the registers (section 2 says why: the game
copies integer data through FP registers). Lever 2 measured no gain and is not in the
engine, so WebAssembly SIMD is not a page requirement (section 5). The GX parse itself
stays on the emulation thread; what moved off it is the waiting for the render thread
(section 9).

**In short** (section 11; Chrome, paced, interleaved against origin/main): 4 E-cores,
human pitching **1.000x** and CPU vs CPU **0.993x** (before 0.87-0.93); hostSlow 2
**0.970-0.985x** (before 0.89-0.90; the 1.0x stretch is not reached); hostSlow 1.3
**9.6-10.1 ms** of emulation per field (before 11.0-12.1). The emulation thread alone
(Node): -14.4 %. Every guest line is unchanged except human pitching's (lever 1); renderer
digests, netplay and a whole game check out (section 12).

## 1. Where the time went (at the start)

Node (`mvp-boot-wasm`, the translated code with the runtime, no renderer), CPU vs CPU,
pinned pairs on two efficiency-core clusters, the emulation thread's CPU time per field
of play: 10.62 / 10.78 ms (Q4's merged engine). A browser profile early in Q6 (profiling
build, paced CPU vs CPU, after levers 1, 3 and 4) put about 54 % of the busy emulation
thread in translated code and 43 % in the engine; of the engine, roughly half was the GX
pipeline (the pipe's bursts, `GxFifo`, `GpReads` and its marks, the commands into the
ring) and the rest the native skinning code, safe points and the interpreter.

Paced browser sessions (Q4's merged engine, this PC): hostSlow 2 at 0.90x in both
matches, 4 E-cores at 0.92-0.94x, hostSlow 1.3 at full speed with 11.0-11.3 ms of
emulation per field. The sessions' own statistics showed what the profile cannot: on the
slower machines a large part of the emulation thread's time per field was spent
*waiting* for the render thread (write waits): 1.9-2.8 ms per field on the E-cores,
0.8-1.1 ms at hostSlow 2 (section 9).

## 2. Lever 1: the fast FP model without NI's flush to zero

**What changed.** Under F1 the fast model (Q4) did one IEEE double operation per step,
rounded single results through `float`, made every NaN result the default NaN and then
flushed results below the normal range of their precision to a signed zero (the Gekko's
NI mode, which the game runs with). The flush was a compare and a mask after every
floating-point result, in `fp::fast_result` and in the emitter's result chain. It is gone:
results below the normal range stay the IEEE subnormals the host computed.

**NaNs stay canonical per result.** The approval allowed canonicalizing only where FP
bits leave the registers (stores, `mffs`, compares). That does not work for this game:
`lfd`/`stfd`, `lfs`/`stfs` and `psq_l`/`psq_st` move bit patterns, and the game copies
integer data through FP registers (`fn_803D9918` copies display data into the GX pipe
with `lfd`/`stfd`; a word of -1 is a NaN pattern). A store cannot tell a NaN it must keep
(loaded) from one an operation made (whose payload differs between hosts), so a
store-time rule would either corrupt the copies or let host payloads through. The check
per result is cheap: it cost about 1 % (Node, pinned pairs, play: 10.55/10.57 with it,
10.33/10.57 without), the flush about 5 % (11.92/11.35 -> 11.18/10.96).

**Why every host still computes the same bits (x86 and ARM).** Each step is one IEEE 754
binary64 operation in round-to-nearest-even (add, subtract, multiply, divide, square
root), or a conversion between binary64 and binary32 in round-to-nearest-even; IEEE 754
fixes the result of each of these exactly for every operand, including subnormal operands
and results, when no flush-to-zero or denormals-are-zero mode is on. WebAssembly
specifies exactly these operations with subnormals and without such modes, on every host
(V8 on x86-64 uses SSE2 scalar instructions with MXCSR's FTZ/DAZ clear; on ARM64 it uses
the scalar FP unit with FPCR.FZ clear). Native builds run with FTZ/DAZ off (the default
for MSVC, clang and gcc on x86-64 and ARM64 without fast-math). The two things IEEE
leaves to the implementation are NaN payloads and the sign of a NaN (x86 produces the
"default NaN" 0xFFF8..., ARM 0x7FF8..., and payload propagation differs); every NaN
result is replaced by 0x7FF8000000000000 before anyone sees it, so those never show.
Fused multiply-add is never used (multiply, then add, as two rounded steps; the emitter
emits `f64.mul` then `f64.add`, and the C++ builds compile with `-ffp-contract=off
-fno-fast-math`, or `/fp:precise` with MSVC, engine/CMakeLists.txt). So the guest sees
the same bits on every host and in both engines (native C++ and translated wasm).

**Effect on guest lines.** WEB.md's checkpoints at 700, 2,400 and 3,600 fields and CPU vs
CPU at 11,600 fields do not change (no subnormal result reaches anything those runs
hash). Human pitching at 9,000 fields changes: MEM1 `58631fe2` -> `caec614c` (same
instruction count and time base). WEB.md's reference lines are re-recorded (section 12).

Tests: `test_fp` gains the fast model's rules (200,000 random operand sets, every
operation, both precisions; subnormal results kept, NaNs canonical); `test_sem`'s NI case
runs in the exact model, which still flushes.

## 3. Lever 3: MEM1 at a fixed place for the memory helpers

Every load and store the translated code does not inline goes through an in-module
helper (`rd8/16/32`, `wr8/16/32`, `rd64`, `wr64`, `lfs`, `stfs`). Before, each walked
the page table: page index, entry load, null check, then the access. The runtime now
gives the module MEM1's host address (`env.mem1`: `{base, limit}`, `rt::set_wasm_mem1`,
set by `mvp_web` and `mvp-boot` once the machine exists; `WasmMem1Layout`). Each helper
is a small front: an access at 0x80000000 + t with t < limit (limit = MEM1's size less
7, so all 8 bytes are inside) goes to base + t, one subtraction and one compare; any
other address tail-calls the page-table path, unchanged (`rd32.page`, ...). The fronts
are small enough that V8 inlines them into hot callers. MEM1 is mapped at 0x80000000
linearly by `Machine`, so the bytes are the same either way; limit 0 (no machine given)
keeps the old path. The write fronts keep the render thread's write check
(`gp_check`). `test_wasm_lockstep` runs its synthetic program against the interpreter
with the path on, in both emitter modes.

Node, pinned pairs, CPU per field of play: 10.24/10.30 -> 10.00/10.05 with the path
inside the helpers, 9.92/9.79 with the fronts split off (about 4 % in all). Guest line
unchanged.

## 4. Lever 4: more of the game's hot code natively (Q4-N1)

**`fn_803EA3EC`, the skinning loop, natively.** The function around `fn_803EB000` (Q4's
native skinning-matrix blend) loops over n matrices: for each one it calls
`fn_803EB000` and writes two more XF loads (the normal matrix and a second one) into the
write-gather pipe. Now the whole loop runs as host code: its prologue (the frame, the
FPR saves), each iteration (`run_803EB000`, then the two XF loads straight into the
pipe's model, `Hardware::wgp_store`, with the PC and ticks of each original store), and
its epilogue (registers read back from the frame, as the original does). Its safe
points are where translated code has them: at `fn_803EB000`'s entry and at the back
edge's source (before the `bdnz`), every iteration. A handler that runs at one and
returns keeps its ticks (the loop's own tick count is relative from there) and the
loop continues; anything else (a thread switch, an exception) leaves with
`g_unwinding` set, and the dispatcher continues at `c.pc` in the interpreter (a
replaced function's original has no resume labels). The native code falls back to the
original whenever the state is not the one it is written for (FP off, the exact model,
GQR0 not floats, a span not in plain memory).

Two first attempts changed the CPU vs CPU guest line (MEM1 `57fadf1b`, `4ccb3cb8`): the
safe point placed at the loop head instead of the back edge's source, and a handler's
ticks overwritten by the loop's absolute count. Both are fixed. `local_hle_native` now
runs the original from main.dol in the interpreter against the native loop: 2,000
trials, about 9,000 matrices, safe points due at once or every few instructions, a
quarter of them standing for a handler that ran (its time on `c.ticks`); registers,
memory, and every store with its PC and ticks agree. The test catches the tick bug when
it is put back.

Node, pinned pairs: 9.73/9.93 -> 9.59/9.73 (about 2 %). Guest lines unchanged.

**Also written, exact, and dropped (no gain).** `fn_803BD358` (a 4x4 matrix product in
singles) and `fn_80431440` (`PSMTXMultVec`) natively: `local_hle_native` found no
difference in 2,000 random trials each, but play did not get faster (9.61/9.51 against
9.56/9.50): both are short, and the HLE call (leaving the module, the dispatcher's table,
the register file through memory) costs what their bodies save. `fn_80369878` and
`fn_803E3C08` (mixed integer and FP with many calls) were not attempted for the same
reason: their time is spread over calls into translated code, which a native body
would have to call back into.

## 5. Lever 2: paired singles as SIMD: tried, not adopted

Where the paired-single work is hottest, in the native skinning blend (`fn_803EB000`,
Q4), two forms were built (`-msimd128` for that file only) and measured with pinned
pairs: v1, both halves of each paired single in one f64x2 (multiply, add, the rounding
to single and the NaN check lane by lane, the same IEEE steps as the scalar code):
10.04/9.95 against 9.87/9.84 without, slower; v2, the loads too (two big-endian singles
swapped and promoted at once, NaN patterns the scalar way): 9.71/9.53 against 9.67/9.58,
no gain. The blend is short chains of loads, a few multiply-adds and stores; V8's scalar
code keeps both halves in registers, and the lane inserts and extracts to and from the
register file's layout cost what the vector arithmetic saves. The translator's own
paired-single code was not converted: after the native blend, the paired-single
functions left in the profile (`fn_803BD358`, `PSMTXMultVec`, the `psq_l` helper) are
each under 1 % of the emulation thread. So lever 2 is not part of the
engine, and WebAssembly SIMD is NOT a page requirement (the page's checks are
unchanged).

## 6. The write helpers look for the pipe first

With MEM1's stores taken by the fronts, the page-table write helpers' common case is the
write-gather pipe (GX commands written by game code: 0xCC008000, an unmapped page). Its
test (and the `fast` flag) now comes before the page-table walk. Same results
(`test_wasm_lockstep`'s pipe test; guest line unchanged). Node: 9.56/9.63 -> 9.45/9.51
(about 1 %).

## 7. A safe point only when the GP raises the CP interrupt

Every burst the GP read kicked the scheduler (`next_event = now`), so the next
safe-point location ran `rt.safepoint` with nothing to deliver: 6,600 such calls per
field of play (23.8 of 23.8 million in the CPU vs CPU stretch), each working out the
devices' time and every interrupt cause. The GP's progress can raise only the CP
interrupt (a FIFO watermark or the breakpoint, each with its enable); the game has the
overflow enable on but keeps the FIFO drained, so that cause is never up. `gp_run` now
kicks only when it is (`Hardware::cp_cause`, shared with `pi_cause`). Device events and
deliveries keep their own schedule (a safe point with nothing to deliver changes
nothing), and the host yield still comes at the field's own safe point, so the guest sees
the same: every guest line is unchanged, native and wasm. No-op safe points in play:
23.8 million -> 0.29 million. Node: 9.57/9.41 -> 9.24/9.13 (about 3 %).

A narrower first try (kick only when the overflow or underflow enables are set) gained
nothing: the overflow enable is on.

## 8. The GP reads the skinning code's bursts in one piece

Nearly all of the 15,900 bursts a field of play sends through the write-gather pipe
come from the skinning loop (`fn_803EA3EC` around `fn_803EB000`, both native now),
about five per matrix. Each burst went through the whole GP path by itself: the CPU
FIFO's pointers, `GxFifo`'s push and parse, the observer's batch for the render thread
(with its copies and marks), the PE and CP checks.

Now `Hardware::gp_defer` holds the GP from a pass's first store to the safe point before
its back edge (and around the standalone `fn_803EB000`): a completed burst still goes to
memory and moves the CPU FIFO's write pointer, as before; at `gp_defer(false)` the GP
reads everything written, up to the write pointer in one push when no breakpoint is set
and the FIFO does not wrap in between (`gp_run`; the draw-done count adds up the
finishes of one push). No guest code runs inside a pass and nothing in it reads the
GP's state; what the GP raises (the PE token and finish, the CP's pointers and its
interrupt cause, the kick that schedules a safe point) is in place before the next safe
point, which is the first place anything could be delivered, as before.

`test_devices`: a 256 KiB stream of NOPs, XF and CP loads, tokens and finishes through a
1 KiB FIFO that wraps, the GP held over groups of 1-9 bursts on one machine and not on
the other: after every group the same pointers, token, interrupt state, finishes,
commands seen and memory (a finish miscounted inside one push is caught). Guest lines
unchanged (native, Node, Chrome).

Measured: Node, pinned pairs, 9.23/9.26 -> 9.04/9.02 ms per field of play (about 2.3 %);
Chrome unpaced (CPU vs CPU, fields 8,000-11,600, ms per field in `mvp_run_fields`,
interleaved): 7.56/7.53 -> 7.34/7.41 (about 2.3 %). Q6's first try of this (only inside
the loop, before the safe-point change of section 7) had measured nothing in Node; with
the no-op safe points gone the per-burst path was a larger share.

## 9. Lever 5: the GX work off the emulation thread

**What can move, and what cannot.** The emulation thread parses the GX command stream
twice: once in `GxFifo` (command lengths, CP/BP state; this decides the PE token, the
draw-done interrupt and the CP's read pointer, which the guest sees) and once in
`GpReads` (what the render thread will read for each command, so that guest writes there
wait for it). The first must stay where it is: its effects are guest state, and they
must land at the burst that carries them, as they do now, or the guest line changes. The
second exists only so that the render thread reads guest memory as the inline renderer
would; it has to be known before the guest's next write, which can come right after the
burst, so it cannot lag behind on another thread either. What the render thread costs the
emulation thread in play is mostly something else: the **write waits**, where a guest
write meets memory a command the render thread has not finished still reads, and the
emulation thread stops until it has.

Before this lever (paced sessions, the Q6 engine so far):

| Machine | Match | Write waits per field | Time waiting per field | Of the field's emulation time |
|---|---|---:|---:|---:|
| hostSlow 1.3 | CPU vs CPU / human pitching | 1.20 / 1.75 | 0.12 / 0.28 ms | 1 / 3 % |
| hostSlow 2 | CPU vs CPU / human pitching | 1.39 / 1.78 | 0.82 / 1.08 ms | 5 / 7 % |
| 4 E-cores | CPU vs CPU / human pitching | 1.46 / 2.00 | 1.87 / 2.77 ms | 12 / 18 % |

On the E-cores the render thread itself needs 9-10 ms a field, so it is often still drawing
the previous frame when the game starts writing the next one's vertex data; the
emulation thread then waits up to several milliseconds.

**The design: copies instead of waits.** When a command that reads a vertex array, an
indexed XF load, a palette (TLUT load) or a TMEM preload goes into the ring, the
emulation thread now sends the render thread a copy of what it reads (whole 64-byte
entries, a `kShadow` record placed before the command's own record) and tags those
entries in `ppc::g_gp_pages` as copied, instead of marking them with the command's
epoch. The render thread keeps the copy (`GpMemory::shadow`, at the same address in a
buffer of its own), and every memory read it makes goes through `GpMemory::view`, which
takes the copy where there is one. A guest write to a copied entry (every write path runs
the check: the interpreter's stores, the translated stores, the native code's stores, the
pipe's bursts, DMA) ends the copy instead of waiting: a `kShadowEnd` record goes into the
ring before the write happens. Commands issued before the write come before that record,
so the render thread reads them from the copy, which holds memory as it was when they
were issued; commands after it read memory (marked as before) or a new copy.

When an entry is copied:
- No snapshot (Q1's GP view of the data cache) over it, or one that memory still matches:
  the GP's view of the entry is then memory as it is, and stays equal to the copy until
  the guest writes there (a later flush snapshots the same bytes; a field's end makes the
  GP read memory, which holds the same bytes). An entry whose snapshot memory has moved
  on from (the game wrote after flushing) is marked as before. Entries entirely under
  snapshots need nothing (the snapshot travels in the ring), as before.
- An entry that an earlier command still reads from memory (marked, not finished) is
  copied too; its mark is kept beside it, and the guest's next write there still waits
  for that command (only for it, and only once).
- Textures and EFB-copy ranges are marked as before (large, rarely written; no write
  waited for them).

Why this is exact: the renderer's input is the same as the inline renderer's for every
command. A copy is taken from memory when its command is issued, with no guest code
running in between, and stays identical to memory (as the GP sees it) until a guest write
there, which ends it first; so for every command issued while it lasts, the copy holds
what the inline renderer would have read at that command. Guest state cannot depend on
any of it (the render thread only reads). Checked with the renderer digests (section
11): threaded with copies, threaded without (`rtTune=...,0`), and single-threaded agree
field by field.

Measured: paced sessions of one site with copies on and off (`rtTune=-1,-1,0,-1,0`),
interleaved; speed, the emulation thread's time per field (with the ×N spin of the
simulated machines), new frames shown per second, presents over 25 ms:

| Machine | Match | Copies on | Copies off |
|---|---|---|---|
| 4 E-cores | CPU vs CPU | **0.991x**, 14.03 ms, 54.1/s, 7.2 % | 0.962x, 15.54 ms, 47.9/s, 18.3 % |
| 4 E-cores | human pitching | **1.000x**, 13.38 ms, 56.1/s, 7.6 % | 0.946x, 16.07 ms, 45.5/s, 23.6 % |
| hostSlow 2 | CPU vs CPU | **0.968x**, 15.69 ms, 47.3/s, 21.7 % | 0.931x, 16.80 ms, 44.8/s, 22.2 % |
| hostSlow 2 | human pitching | **0.975x**, 15.10 ms, 49.0/s, 22.6 % | 0.930x, 16.42 ms, 45.4/s, 24.8 % |
| hostSlow 1.3 | CPU vs CPU | 1.000x, **10.17 ms** | 1.000x, 10.45 ms |
| hostSlow 1.3 | human pitching | 1.000x, **9.83 ms** | 1.000x, 10.15 ms |

Write waits in play fell to 0.00-0.02 a field. The copies are small: 72 MB over the whole
11,600-field CPU vs CPU run (about 6 KB a field), 50-100 ends a field; the render thread's
own time per field did not change (10.3 ms on the E-cores either way).

How it got there: a first version copied only entries with no snapshot over them and no
command still reading memory there. It removed two thirds of the waits but not their
time (4 E-cores, human pitching: 0.953x against 0.943x without; hostSlow 2: 0.959x
against 0.932x): the long waits remained. Counting why the remaining waits' entries were
marked rather than copied (a debug build) found 154,000 of 164,000 on entries with one of
their two cache lines under a snapshot (an array's edge next to a flushed one) and the
rest on entries an earlier command still read. The version above copies both kinds.

## 10. Tried and dropped (measured, reverted)

Node, pinned pairs, CPU per field of play (each pair against the build it was added to):

| What | With | Without | Verdict |
|---|---:|---:|---|
| Paired singles as f64x2 in the native blend (v1) | 10.04 / 9.95 | 9.87 / 9.84 | slower |
| The same with vector loads (v2) | 9.71 / 9.53 | 9.67 / 9.58 | no gain |
| Native `fn_803BD358` and `PSMTXMultVec` (exact) | 9.61 / 9.51 | 9.56 / 9.50 | no gain |
| Runs of `stfd`/`lfd` as one helper call | 9.49 / 9.38 | 9.47 / 9.44 | no gain (noise) |
| Pipe stores forced inline in hot callers | 9.50 / 9.52 | 9.52 / 9.46 | no gain |
| The GP reading the native loop's bursts in one piece, first try (before section 7) | 9.46 / 9.45 | 9.49 / 9.50 | no gain then; kept later (section 8: 2.3 %) |
| Kick only with the FIFO's watermark enables (narrower than section 7) | 9.91 / 9.91 | 9.97 / 9.94 | no gain |
| No NaN check after results (the most the approved store-time form could save) | 10.33 / 10.57 | 10.55 / 10.57 | about 1 %; the store-time form is not exact here (section 2) |

Also not done: a JIT for code the game writes at run time (about 15,000 interpreted
instructions a field, small thunks in the heap; about 2.5 % of Node's play time); it
would need the emitter and a module loader in the page.

## 11. Results (the merged engine against origin/main)

**Emulation alone.** Node (`mvp-boot-wasm`, CPU vs CPU from power-on to the poll of frame
10,544), two runs at once pinned to two efficiency-core clusters, twice with the clusters
swapped; the emulation thread's CPU time per field of play (frames 7,300-10,544):
origin/main (Q4) 10.70 / 10.66 ms, merged Q6 9.12 / 9.17 ms (**-14.4 %**); whole runs 108.2 /
108.2 s -> 93.8 / 94.9 s. Node has no render thread, so the copies (section 9) are not in
this number. Chrome, threaded, unpaced CPU vs CPU (fields 8,000-11,600, ms per field in
`mvp_run_fields`): 7.34 / 7.41 with every lever, 7.56 / 7.53 without section 8's.

**Match sessions.** Chrome 155 headed with sound, the match scripts (CPU vs CPU: unpaced to
field 8,000, measured paced 8,000-11,600; human pitching: 6,600-9,000), each pair
interleaved before/after, three rounds (round 2 ran each pair before-first; round 3 repeated
the configurations disturbed in rounds 1 and 2). Before is origin/main (`3c566aaf`: Q4's
engine with Q5b's shell), after the merged branch (`a2f433bf`). "hostSlow N" is Q2's
simulated slower machine (each engine thread spins N - 1 times its working time, plus
Chrome's CPU throttling); hostSlow 1.3 stands in for the owner's laptop. "4 E-cores": Chrome
pinned to four efficiency cores (`--affinity F000000`). Emulation ms/field is the host time
per field inside `mvp_run_fields` (with hostSlow, the spin included); rendered fps is presents
per second, new frames/s the presents that showed a new game frame; p95 and "> 25 ms" are
over the intervals between presents and between composited frames (the screencast).

Other work on this PC disturbed some runs (another session's MSVC build at up to 20 cores,
and a busy Chrome tab): round 1's 4 E-cores CPU vs CPU pair and hostSlow 4 human-pitching
pair, round 2's hostSlow 2 CPU vs CPU "before" and its hostSlow 2 human-pitching pair. They
are in the table as measured (the outliers: 0.827/0.692, 0.281/0.266, 0.687, 0.664/0.551) and
were repeated in round 3. A native whole game ran pinned to cores 16-19 during rounds 1-3.

| Config | Match | Build | Game speed | Emulation ms/field | New frames/s | Rendered fps | Present p95 ms | Presents > 25 ms | Composited p95 ms | Composited > 25 ms | Underruns/min |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| this PC | CPU vs CPU | before | 1.000 / 1.000 | 8.8 / 10.4 | 59.5 / 57.9 | 59.9 / 59.6 | 25 / 25 | 9.4 % / 5.7 % | 20.6 / 21.6 | 0.6 % / 2.7 % | 0.0 / 0.0 |
| this PC | CPU vs CPU | after | 1.000 / 1.000 | 7.7 / 7.8 | 59.5 / 59.5 | 59.9 / 59.9 | 25 / 25 | 4.2 % / 7.1 % | 23.0 / 21.4 | 2.5 % / 2.0 % | 0.0 / 0.0 |
| this PC | human pitching | before | 1.000 / 1.000 | 8.7 / 8.7 | 59.2 / 59.2 | 59.9 / 59.9 | 25 / 25 | 6.2 % / 4.7 % | 21.8 / 20.9 | 1.2 % / 1.5 % | 0.0 / 0.0 |
| this PC | human pitching | after | 1.000 / 1.000 | 7.4 / 7.9 | 59.6 / 57.5 | 59.9 / 59.5 | 25 / 25 | 4.5 % / 6.7 % | 21.9 / 21.7 | 1.1 % / 2.8 % | 0.0 / 0.0 |
| hostSlow 1.3 | CPU vs CPU | before | 1.000 / 1.000 | 11.3 / 12.1 | 59.2 / 57.8 | 59.9 / 58.7 | 24 / 27 | 3.8 % / 7.1 % | 21.2 / 23.9 | 1.0 % / 4.3 % | 0.0 / 0.0 |
| hostSlow 1.3 | CPU vs CPU | after | 1.000 / 1.000 | 9.9 / 10.1 | 59.0 / 59.3 | 59.9 / 59.9 | 25 / 26 | 3.6 % / 7.8 % | 21.2 / 21.3 | 0.7 % / 2.0 % | 0.0 / 0.0 |
| hostSlow 1.3 | human pitching | before | 1.000 / 1.000 | 11.0 / 11.2 | 59.5 / 59.6 | 59.9 / 59.9 | 25 / 26 | 5.1 % / 8.0 % | 21.2 / 21.5 | 1.0 % / 2.4 % | 0.0 / 0.0 |
| hostSlow 1.3 | human pitching | after | 1.000 / 1.000 | 9.6 / 9.8 | 59.6 / 59.3 | 59.9 / 59.9 | 25 / 25 | 3.7 % / 5.5 % | 21.9 / 21.5 | 1.5 % / 1.9 % | 0.0 / 0.0 |
| hostSlow 2 | CPU vs CPU | before | 0.893 / 0.687 | 18.0 / 23.9 | 36.2 / 26.9 | 36.5 / 27.1 | 47 / 65 | 49.1 % / 69.9 % | 53.0 / 82.1 | 51.1 % / 68.9 % | 0.9 / 0.0 |
| hostSlow 2 | CPU vs CPU | after | 0.976 / 0.970 | 15.3 / 15.5 | 50.6 / 49.1 | 51.5 / 49.6 | 37 / 38 | 19.1 % / 21.1 % | 31.8 / 34.1 | 7.3 % / 12.3 % | 0.0 / 0.0 |
| hostSlow 2 | human pitching | before | 0.903 / 0.551 / 0.894 | 17.5 / 30.2 / 17.8 | 41.7 / 20.8 / 40.4 | 42.1 / 20.9 / 40.9 | 45 / 78 / 46 | 31.4 % / 92.6 % / 34.4 % | 52.1 / 103.0 / 53.0 | 32.1 % / 88.9 % / 34.6 % | 0.0 / 0.0 / 0.0 |
| hostSlow 2 | human pitching | after | 0.985 / 0.664 / 0.978 | 14.7 / 25.0 / 15.1 | 51.5 / 24.3 / 49.0 | 52.1 / 24.4 / 49.3 | 35 / 65 / 36 | 19.1 % / 81.1 % / 26.3 % | 34.7 / 84.4 / 35.7 | 14.6 % / 78.2 % / 20.0 % | 0.0 / 0.0 / 0.0 |
| hostSlow 4 | CPU vs CPU | before | 0.400 / 0.438 | 41.6 / 38.0 | 15.4 / 16.7 | 15.5 / 16.9 | 115 / 106 | 97.6 % / 96.7 % | 135.2 / 129.4 | 96.4 % / 95.0 % | 0.4 / 0.0 |
| hostSlow 4 | CPU vs CPU | after | 0.545 / 0.478 | 30.6 / 34.8 | 20.4 / 18.4 | 20.6 / 18.4 | 78 / 94 | 97.8 % / 96.1 % | 109.6 / 130.5 | 91.3 % / 90.9 % | 0.6 / 0.0 |
| hostSlow 4 | human pitching | before | 0.266 / 0.439 | 62.5 / 37.9 | 10.4 / 16.8 | 10.4 / 16.9 | 185 / 102 | 100.0 % / 98.4 % | 186.2 / 130.5 | 97.9 % / 94.6 % | 73.3 / 0.0 |
| hostSlow 4 | human pitching | after | 0.281 / 0.513 | 59.2 / 32.5 | 10.9 / 19.5 | 10.9 / 19.5 | 148 / 108 | 99.9 % / 92.6 % | 140.0 / 121.1 | 88.9 % / 89.2 % | 17.5 / 0.0 |
| 4 E-cores | CPU vs CPU | before | 0.692 / 0.909 / 0.932 | 23.7 / 17.4 / 16.9 | 27.4 / 38.0 / 44.1 | 27.6 / 38.5 / 44.9 | 75 / 47 / 42 | 63.8 % / 35.6 % / 25.5 % | 83.3 / 50.9 / 46.0 | 65.1 % / 34.8 % / 21.7 % | 0.7 / 0.0 / 0.0 |
| 4 E-cores | CPU vs CPU | after | 0.827 / 0.993 / 0.993 | 18.9 / 13.8 / 13.7 | 37.6 / 55.2 / 54.5 | 38.7 / 56.7 / 56.7 | 59 / 23 / 28 | 33.3 % / 4.9 % / 8.6 % | 64.6 / 21.5 / 21.5 | 29.3 % / 1.0 % / 1.1 % | 3.4 / 0.0 / 0.0 |
| 4 E-cores | human pitching | before | 0.911 / 0.871 / 0.901 | 17.1 / 18.8 / 17.6 | 41.8 / 33.6 / 40.8 | 42.0 / 33.8 / 41.3 | 44 / 51 / 45 | 30.3 % / 53.6 % / 32.3 % | 50.9 / 52.4 / 51.2 | 31.2 % / 53.6 % / 33.7 % | 0.0 / 0.0 / 0.0 |
| 4 E-cores | human pitching | after | 0.997 / 1.000 / 1.000 | 13.6 / 13.4 / 13.2 | 54.8 / 55.9 / 57.2 | 55.4 / 58.0 / 58.0 | 28 / 21 / 21 | 6.1 % / 3.4 % / 3.1 % | 22.8 / 21.3 / 21.3 | 4.1 % / 2.8 % / 2.7 % | 1.6 / 0.0 / 0.0 |

Reading it, against the targets:

- **4 E-cores, 1.0x:** human pitching **1.000x** in the two undisturbed rounds (0.997 in the
  disturbed one), before 0.87-0.91; CPU vs CPU **0.993x** in both undisturbed rounds, before
  0.91-0.93. New frames shown 54.5-57.2 a second (before 33.6-44.1), presents over 25 ms
  3-9 % (before 26-54 %), composited p95 21-23 ms (before 46-52). CPU vs CPU stays 0.7 % under
  full speed: a few windows still need more than the field's 16.7 ms (p90 16.6-16.7 ms).
- **hostSlow 2, 0.95x or better:** **0.970-0.985x** in every undisturbed run (before
  0.89-0.90), new frames 49-52 a second (before 36-42). The 1.0x stretch goal is not reached:
  the emulation needs 14.7-15.5 ms of the 16.7 ms field there, with peaks over it.
- **hostSlow 1.3 (the laptop), about 10 ms of emulation per field:** **9.9 / 10.1 ms** CPU vs
  CPU, **9.6 / 9.8 ms** human pitching (before 11.0-12.1), full speed in all of them.
- **This PC:** full speed before and after; emulation 7.4-7.9 ms per field (before
  8.7-10.4); presents and composited frames unchanged within the run-to-run spread.
- **hostSlow 4:** 0.48-0.55x (before 0.40-0.44) where undisturbed: better, still far from
  full speed, as expected for a machine four times slower.

Where the time goes now (a profile of the copies build, paced CPU vs CPU at this PC's
speed): the emulation thread is busy about half of each field; of that, translated code is
56 %, the engine 41 %. The GX pipeline (the pipe's bursts, `GxFifo`, the commands into the
ring with `GpReads`, copies and marks) is still about a fifth of the busy time, the native
skinning code (with its stores into the pipe) a sixth.

## 12. Exactness (the merged engine, `a2f433bf`)

**Guest lines, native** (`mvp-play --no-render`, MSVC): WEB.md's checkpoints, 700 fields
`53221c4a...`, 2,400 `4285f135...`, 3,600 `40a7692d...` (unchanged); CPU vs CPU at
11,600 fields 75,249,642,125 instructions, MEM1 `d8c30e9a...` (unchanged); human
pitching at 9,000 fields 58,384,794,125 instructions, MEM1 `caec614c...` (changed by
section 2, from `58631fe2...`; same instruction count and time base). Each Q6 commit
that changes the emulation was checked against these before it was made; the copies (the
render side only) against the browser's lines in their own A/B sessions.

**Guest lines, Chrome** (the merged site): 700 fields threaded and single-threaded with
sound, 2,400 and 3,600 threaded and single-threaded, 3,600 paced with sound and on a page
without cross-origin isolation: the native lines above, every one. The match sessions'
lines (section 11) too, in every session, before and after.

**Node** (`mvp-boot-wasm`, to the poll of frame 10,544): MEM1 `5725e2e2...`, before every
lever and after each one.

**Renderer digests** (`--digest`, every VI field, unpaced): CPU vs CPU: the merged engine
threaded (with copies), the merged engine single-threaded, Q4's threaded and Q4's
single-threaded agree in every field (11,600, or 11,598 where a threaded run stopped two
fields before its render thread reported the last two); human pitching: threaded and
single-threaded agree in all 8,995 common fields, and with the copies build's two runs.
The copies alone were checked the same way before their commit, also against the threaded
engine with copies turned off.

**Netplay.** Native `mvp-netplay solo` (3,200 frames, delay 3, the E5c script):
26,835,905,005 instructions, MEM1 `2a8de818...`, the same as before Q6. `netplay_check`
(two Chrome pages through the server, both threaded): both pages equal it, no desync.

**Tests.** MSVC: 35 tests and the 12 local ones with the disc (`local_disasm` skips: no
dtk listing here). Emscripten: 32 tests (with the web shell's) and 8 local ones. All
pass. New: `test_fp` (the fast model's rules without the flush), `test_wasm_lockstep`
(the MEM1 path), `local_hle_native` (the native loop with simulated handlers),
`test_gp_memory` (copies), `test_devices` (the GP held over bursts).

**A whole game.** Native `mvp-play --headless` on the merged engine (`a2f433bf`) with the CPU
vs CPU script from power-on, pinned to four efficiency cores, frames captured every 7,200
fields: played to **Game Over** by field 230,400, Cardinals 12, Red Sox 10 in 10 innings,
the same result and box score as Q4's game; no crash, and the captures look as they should
(skinned players included). Its MEM1 hashes (every 3,600 fields) equal Q4's game to frame
61,200 and differ from 64,800 on (the same instruction counts there): section 2's change
reaching guest state in a long game, as expected, without changing how the game went.

## 13. Commits, proposals, what is left

Commits on `engine/q6-speed` (not pushed):

| Commit | What |
|---|---|
| `73d7849e` | the fast FP model without NI's flush to zero (lever 1) |
| `fd9c8024` | the memory helpers try MEM1 first, at its own host address (lever 3) |
| `4be54002` | `fn_803EA3EC`, the skinning loop, natively (lever 4) |
| `1e8bf506` | the write helpers look for the write-gather pipe first |
| `2618ecc0` | a safe point only when the GP raises the CP interrupt |
| `c3d5c442` | copies of what commands read instead of write waits (lever 5) |
| `8ef9646e` | merge of origin/main (Q5b), no conflicts |
| `a2f433bf` | the GP reads the skinning code's bursts in one piece |
| (this log) | Q6_LOG.md, WEB.md's re-recorded lines and Q6's numbers |

Proposed DECISIONS entries (for the lead; docs/DECISIONS.md not edited):

- **Q6-F2** (under F1): the fast FP model does not model NI's flush to zero; results
  below the normal range are IEEE subnormals, the same on every host. NaN results stay
  canonical per result, not only where bits leave the registers (the game moves integer
  data through FP registers). The human-pitching reference line changes (`58631fe2` ->
  `caec614c`); WEB.md re-recorded.
- **Q6-M1**: translated modules get MEM1's host address (`env.mem1`); each memory helper
  is a front with a MEM1 path and the page-table path behind it.
- **Q6-N2** (extends Q4-N1): native code may replace a game function with a loop and safe
  points if it runs them where translated code does (callee entries, the back edge's
  source), keeps a handler's ticks, and leaves to the interpreter for anything else;
  `local_hle_native` tests such code with simulated handlers. Short functions are not
  worth it (`fn_803BD358`, `PSMTXMultVec`: exact, no gain).
- **Q6-K1**: the GP's progress schedules a safe point only when it raises the CP interrupt.
- **Q6-G1**: native code that writes the GX pipe may hold the GP until its next safe point
  (`Hardware::gp_defer`); the GP then reads all of it in one push.
- **Q6-C1**: the threaded engine sends the render thread copies of what commands read of
  vertex arrays, XF loads, palettes and preloads, ended ahead of the guest's next write
  there, instead of making that write wait (`rtTune`'s fifth value turns it off).
- **Q6-S0**: WebAssembly SIMD is not required (lever 2 measured no gain); the page's
  requirements are unchanged.

What is left, by size:

- The GX path on the emulation thread is still about a fifth of its busy time in play
  (the pipe's bursts, `GxFifo`, `GpReads`, the commands into the ring). Larger steps:
  sending a display list once and calling it by reference while its memory is unchanged
  (the copies' end records already say when it changes), and caching `GpReads`' ranges
  per display list with it.
- The run-time code the game writes into its heap (about 15,000 interpreted instructions
  a field) could be translated in the page; about 2.5 % in Node.
- `fn_80369878` and `fn_803E3C08` (each about 1 %) spend their time in calls into other
  translated code; native versions would have to call back.
