# V5 log: the render thread's CPU

Side session `claude-side-render`, branch `engine/render-speed` (from origin/main `31aa85fc`,
origin/main `251a64fb` with V3 merged in). The owner, 2026-10-21: "Optimize optimize optimize, I
am still dropping frames on some devices, do whatever you absolutely can to make this run as
efficiently as possible." This log: the threaded engine's render thread (the GX processor and the
WebGPU renderer), made cheaper on the CPU without changing a bit of what it draws.

## Status

| What | State |
|---|---|
| Fast vertex path: loader programs, transform plan, GP memory windows | **Done** (`f2b0c4ee`) |
| Per-draw work: XXH64, hashed caches, packing by layout, indices in place | **Done** (`40f92137`) |
| Constant-size zeroing, unrolled attribute decoders | **Done** (`4f25767b`) |
| XF matrix loads: cached matrices dropped once per load | **Done** (`6fe6007c`) |
| The render thread sleeps on its ring without a 1 ms poll first | **Done** (`de893fdb`) |
| Render thread, dev PC, the bat scene | **5.17 / 5.21 -> 4.27 / 4.28 ms a field (-18%)** |
| Render thread, 4 efficiency cores | **10.27 / 10.29 -> 8.14 / 8.14 ms a field (-21%)** |
| The browser's CPU (all its processes), dev PC / 4 efficiency cores | **1.23 / 1.18 -> 1.00 / 0.99 cores (-17%)**; **1.84 / 1.84 -> 1.70 / 1.67 (-8%)** |
| 4 efficiency cores at 120 fps (V3's in-betweens) | **14.15 / 14.25 -> 11.89 / 11.81 ms a field (-17%)** (before the last two commits) |
| Exactness | Renderer digests equal in every field (interp off and on), final pictures byte-equal, `test_vertex_exact` (new) bit for bit |

## 1. How it was measured

**The scene** is V1's: the save state `braves-dodgers-9th-bat` (Dodger Stadium, top of the 9th,
the MLB 2026 pack, the ABS challenge on; the batter never swings), Chrome 155 headed through
Playwright, paced, from field 136,700 to 140,200 (58 s). A copy of the lead's `run_scene.sh`
pointed at this worktree; every timing run held the measurement lock (`mlock.py`), baseline and
change interleaved (base, new, base, new), the baseline a build of origin/main from the same tree.

**Render thread busy** is the thread's working time per field (`render.busy_ms` of the status; its
waits for records are not counted). Reported as the run's mean and, to see through other agents'
builds on the PC, the median of its 0.5 s windows (`busy.py`). The two agreed within 0.1 ms in
undisturbed runs.

**`mvp-gx-bench`** (P1/P2's synthetic stream of skinned strips) gained flags for this work:
`--ref` (the reference vertex path, the one before V5), `--gp` (arrays read through a GpMemory whose
entries are copies, as on the render thread), `--dark` (color 0 lit by no light: the game's setup,
see below). Its output hashes are unchanged by everything here (`34cc0ea9fe288a0f` default,
`43b07a43647c4c7c --alpha-lit`, `9fdf6d06781f9f48 --no-skin`, `d33a1eed9cd0e077 --no-light`,
natively and under Node).

**Profiles.** `build_prof.cmd`'s named threaded build, plus `-O1` at link
(`-DCMAKE_EXE_LINKER_FLAGS=--profiling-funcs -O1`): Binaryen's `-O2`+ inliner otherwise folds every
function called once back into its caller (the whole GX path showed as `GxProcessor::command`).
`MVP_PROF_NOINLINE` (gx_processor.cpp, renderer.cpp) keeps the parts out of line in such builds
(`MVP_WEB_NAMES` and clang only; nothing otherwise). Chrome trace (`--trace 137500:8000`) and
`trace_summary.mjs`; the render thread is the DedicatedWorker whose top is `_do_futex_wait`.

## 2. Where the render thread's time went (before)

Per field in play, dev PC, origin/main: 5.2 ms of work. 7,616 primitives, 46,417 vertices
(6.1 a primitive), 565 pixel-state snapshots, 7,051 draws with the previous draw's state, 7,338
appended to the previous WebGPU draw, 269 WebGPU draws; 2.2 MB of vertices and 0.37 MB of indices
uploaded (the `gx` object now in the render status, kept per window by browser_check).

What the vertices are (a census build, not committed): every attribute indexed (3.2 indexed reads
a vertex, all through `GpMemory::view`), 39% of vertices in a color channel that is lit but has no
light enabled (ambient times material), no light factors at all, 1.3 texture coordinate generators
a vertex, all with post matrices, 0.4 a vertex normalized before the post matrix; half the
vertices carry a matrix index (skinning).

The named profile (inline build, then `-O1`): `GxProcessor::command` 90% of the working time;
inside it the transform 1.45 ms, the loader 0.79 (with `GpMemory::view` 0.41), primitive and
command dispatch 0.69; `Renderer::draw` 0.47 plus texture lookups 0.27 (palettes hashed on every
lookup), packing 0.15; `writeBuffer` 0.6; EFB copies, flushes and presents the rest. The renderer
hashed 2.1 MB a field: texture memory re-checked after each texture cache invalidation (1.4 MB),
EFB copy memory (0.4), palettes (0.1), pipeline configurations (0.2).

## 3. Kept

### 3.1 The fast vertex path (`f2b0c4ee`)

- **Loader programs.** `vertex_format()` also compiles the format into steps (one per read, in the
  vertex's byte order: type, array, size, destination, count, scale, decoder); `vertices()` runs
  them for a primitive's vertices, with the input vertex zeroed once per primitive (every vertex
  writes the same fields; a read outside memory zeroes its fields, as the reference's fresh vertex
  has them).
- **A transform plan.** `setup_transform` (rebuilt when XF registers change, as before) also lists
  the color passes and each light factor once (a light with its diffuse and attenuation functions,
  the reuse the P1 transform already did), and decodes the texture coordinate generators.
  `transform_fast` follows the plan with the reference's arithmetic, operation for operation. The
  normal matrix is skipped when no light factor needs the normal (the game's lit channels have no
  lights).
- **GP memory windows.** `GpMemory::window()`: the largest range around a read, inside its 4 KiB
  page, where `view()` returns base + address (memory as it is, or copies); a version, bumped by
  every change of what `view()` returns where (flushes, activations, copies and their ends, writes
  without the cache, fields, attach), invalidates windows. The loader keeps one per array.
- The reference path (`load_vertex` + `transform`) stays, public, for tests and tools;
  `set_reference_vertices(true)` makes primitives take it.

Bench under Node (`--gp --dark`): 64.9 -> 48.0 ns a vertex; default lighting `--gp`: 80.7 -> 67.6.
Scene, dev PC: 5.22 / 5.27 -> 4.82 / 4.99 ms a field.

### 3.2 Per-draw work (`40f92137`)

- `hash_bytes` is XXH64 (xxHash, BSD-2): 20 GB/s in the browser against 8 GB/s for the
  multiply-mix loop before (`hb.cpp` in the session's scratch: 376 B to 128 KB buffers). Its values
  only ever meet values of the same function (cache keys, change checks), so nothing else moves.
- `textures_`, `tmem_textures_`, `samplers_` are unordered maps (keys already mixed; never iterated).
- `pack_vertices`: the packing loop as a helper, with the common layouts as template constants.
- `GxProcessor::primitive`: the index list sized once and written in place; the default matrix
  indices unpacked only when the CP registers changed (small local changes; the function's shape is
  V3's).

Scene, dev PC: 5.21 / 5.21 -> 4.53 / 4.55 ms (with 3.1).

### 3.3 Zeroing and decoders (`4f25767b`)

`transform_fast` zeroed the unused color channel and texture coordinates with variable sizes:
`memory.fill`, a call in V8. A switch gives each case a constant size (stores). The loader decodes
the common shapes (u8 x2/x3, s8 x3, u16 x2, s16 x2/x3, f32 x2/x3, RGBA8) unrolled. Bench:
46.4 -> 43.0 ns a vertex.

### 3.4 XF matrix loads (`6fe6007c`)

A load that stays inside the position/texture or the normal matrices (direct or indexed: the game's
matrix uploads, 42,300 GX commands a field in all) clears the cached matrices it touches in one pass
instead of word by word. Bench: 43.3 / 43.2 -> 42.3 / 41.8 ns a vertex (a 12-word load every two
strips).

### 3.5 No record poll before the render thread sleeps (`de893fdb`)

While a run batch was active the render thread polled 1 ms for records before sleeping on the
ring's futex. The profile showed the poll as 15% of its wall time (`emscripten_get_now` mostly), not
counted as busy but burning a core. The browser's CPU over the last 43 s of the scene (every Chrome
process of the run, sampled each second: `cpu_run.sh`), alternated:

| | poll 1 ms | poll 0.1 ms | no poll |
|---|---:|---:|---:|
| dev PC | 1.24 / 1.30 cores | 1.08 / 1.12 | 1.02 / 1.10, 1.02 / 1.02 |
| 4 efficiency cores, 120 fps | 2.08 / 2.07 | - | 1.97 / 1.98 |

Speed, render busy and pictures the same in every pair, on 4 and 2 efficiency cores too (speed
0.994-1.001x either way); emulation time the same within noise (13.0-13.9 ms on efficiency cores;
7.3-7.5 on the dev PC, perhaps +0.1 ms without the poll: the publishes wake a sleeping thread).
`rtTune=-1,1` brings the poll back.

### 3.6 Together (origin/main `251a64fb` against `de893fdb`, interleaved, mlock)

| Machine | base r1 | base r2 | new r1 | new r2 | change |
|---|---:|---:|---:|---:|---:|
| Render thread, dev PC (i7-14700K) | 5.17 | 5.21 | 4.27 | 4.28 | -18% |
| Render thread, 4 efficiency cores (`--affinity F000000`) | 10.27 | 10.29 | 8.14 | 8.14 | -21% |
| Browser CPU, dev PC (cores) | 1.23 | 1.18 | 1.00 | 0.99 | -17% |
| Browser CPU, 4 efficiency cores (cores) | 1.84 | 1.84 | 1.70 | 1.67 | -8% |

Render thread busy ms per field (run means; medians 5.15 / 5.20 -> 4.26 / 4.28 and 10.24 / 10.26 ->
8.15 / 8.12). Game speed 0.998-1.001x in every run; emulation 7.3 -> 7.4-7.5 ms a field on the dev PC,
13.0 -> 13.15 on efficiency cores (medians). (Interp on, 4 efficiency cores: section 7.)

## 4. Tried, not kept

- **WebAssembly SIMD for the matrix products** (`simd4.h`: four lanes, wasm SIMD128 / SSE / scalar,
  lane-wise IEEE only, so bit-exact; position and texture/post matrices by columns): exact
  (`test_vertex_exact` natively and under Node, bench hashes), and no faster: 46.6 / 47.0 / 47.1 ns
  a vertex with it against 47.0 / 46.1 / 46.6 without. Each product is a short dependency chain and the scalar
  rows already run side by side; the splats and lane extracts cost what the vector arithmetic saves
  (as Q6 found on the emulation side). So the gfx library is NOT compiled with `-msimd128`, and
  SIMD is not a page requirement. The patch is in the session's scratch (`v5/simd_attempt.patch`,
  `v5/simd4.h`).
- **`__restrict` on transform_fast's references**: slower (45.2 against 43.3 ns a vertex).
- **A hand-written loader for the bench's exact format** (no steps, no switch): 42.1 / 43.0 against
  43.4 / 43.1 ns a vertex. The step loop costs nothing measurable; what is left is the reads and
  conversions themselves, so no per-format loaders were generated.
- **DrawCall::verts without zeroing on growth** (an allocator that default-initializes): 42.5 /
  43.1 against 43.4 / 43.1 ns a vertex; not worth changing a type V3's code shares.
- **Four vertices at a time (SoA, wasm SIMD)**, as a standalone microbenchmark of the game's
  transform (position, projection, viewport, the color pass pair, one generator with a post matrix;
  bit-identical results): 9.7 -> 8.3 ns a vertex. Gathering the inputs and scattering the outputs
  costs most of what four lanes save; not worth a rewrite of the transform.
- **The transform templated on the plan's counts** (generators, passes as constants): 42.4 / 41.8
  -> 41.6 / 41.1 ns a vertex with the game's lighting, nothing with lights. Dropped.
- **The parts of the transform, by elimination** (bench, game lighting, ns a vertex, for the
  record): loading 16, per-primitive work 8, position and projection 5, color passes 4.6, two
  texture coordinate generators with post matrices 7, zeroing 2.6 (then removed: 3.3).
- **A SIMD hash** (xxh32-style, two v128 accumulators): 9-12 GB/s, below scalar XXH64's 12-21.
- (The record poll was first measured here without a CPU meter: on 4 efficiency cores speed
  0.997 / 1.000x with it, 0.999 / 1.001x without; on 2 (`C000000`) 0.994 / 0.997x with, 1.000 /
  1.000x without. The CPU meter then showed what it costs: section 3.5, kept.)

## 5. Exactness

- `test_vertex_exact` (new, engine/gfx/tests): 400 random streams through both paths, compared bit
  for bit: every VCD/VAT combination (direct and indexed, every component and color format, NBT
  with one and three indices, matrix indices, invalid format values), random XF state (matrices,
  normal and post matrices, lights of every diffuse and attenuation function, channels, texgens of
  every type and source, projections), XF/CP/BP writes between primitives, and memory read through a
  GpMemory with snapshots, copies, copy ends, writes without the cache and fields between
  primitives (the windows). 490,000 vertices, natively and under Node. It catches a reassociated
  specular term and a missing window invalidation (both tried).
- `mvp-gx-bench` hashes unchanged (all four, native and Node).
- Renderer digests (`--digest`), origin/main against this branch, fields 1-139,000 of the scene:
  3,010 fields, 0 differ, at each step; with in-between frames on (`interp=1&fakeHz=120`): 3,009
  fields, 0 differ. The final picture of each run (field 139,000) is byte-identical to the
  baseline's (the digest covers what the renderer is given; the picture covers what it draws).
- From power-on through the intro, the title and the front end's menus (an input script:
  `menus.input`, START/A/B/stick taps at fixed frame polls; unpaced, 3,600 fields): renderer
  digests of origin/main and this branch, 3,594 common fields, 0 differ; the final picture (the User
  Profiles screen) byte-identical. (Live `--keys` presses land on different fields from run to run:
  use a script.)
- `ctest` (msvc) gfx tests: 11 of 11 (`test_gfx_render` draws through Dawn natively); the emscripten
  preset's gfx tests (Node): 8 of 8.

## 6. The GPU

Status `gpu.ms` per frame, the scene, this branch: 0.72 ms at Picture 1x, 1.87 at 2x, 2.40 at 3x
(RTX 4070 SUPER), 12.7 timed passes a field. The CPU side (4.2 ms a field) is the larger cost here.
Nothing cheap was found that cuts GPU time without changing the picture: the passes load and store
the EFB because the game continues it across copies; the depth format is the exactness one.

## 6b. Not done (ideas, with what stands in the way)

- **Four vertices at a time** (SIMD across vertices, SoA): the one way left to cut the transform's
  arithmetic (1.2 ms a field now). Skinned vertices take different matrices per lane (gathers), and
  primitives average 6 vertices; a large rewrite with an uncertain payoff after 4.
- **DrawCall::verts without value-initialization** (an allocator that default-initializes): the
  vector grows and shrinks with every primitive and zero-fills each growth (~3 MB a field;
  ~0.1 ms). Changes DrawCall's type, which V3's code and the tests share.
- **16-bit indices**: 0.19 MB a field less to upload (~0.04 ms); touches emit_pending and V3's replay.
- **Merging same-state primitives into one DrawCall** in the processor: would cut most per-draw
  overhead, but changes what the digest folds and V3's per-primitive identities.
- **writeBuffer** (0.6 ms a field, 2.6 MB): proportional to the bytes; the layout is the picture's.
- **The GPU process** (Chrome's, not ours): its main thread spends ~2 ms a field on the dev PC
  executing the WebGPU command stream (`CommandBufferService:PutChanged`, 975 ms of an 8 s trace):
  ~270 WebGPU draws, their state changes, 7.5 passes and 2.6 MB of uploads a field. Fewer passes
  (clears drawn inside the open pass, EFB copies batched) or 16-bit indices would cut it; they live in
  flush/efb_copy/clear_efb and V3's replay, so they are left for whoever owns those.

## 7. In-between frames on (4 efficiency cores)

V3's 120 fps mode costs the render thread most on slow cores (V3_LOG: +3.66 ms a field on 4
efficiency cores). The same interleaved pair with `interp=1&fakeHz=120`, `--affinity F000000`:

| | r1 | r2 |
|---|---:|---:|
| origin/main `251a64fb` | 14.15 ms (0.998x) | 14.25 ms (0.995x) |
| this branch `4f25767b` | 11.89 ms (1.004x) | 11.81 ms (1.001x) |

Render thread busy per field (-17%); 119-121 pictures a second in every run; emulation 14.0 / 14.2
-> 13.6 / 13.6 ms a field.

## 8. How to resume

Scripts in the session's scratch folder, `v5/`: `build_web.cmd`, `build_native.cmd TARGETS`,
`build_prof_o1.cmd` (the named build at -O1), `run_scene.sh TAG "params" [args]`,
`time_pair.sh TAG ROUNDS [args]` (BASESITE / NEWSITE), `digest_pair.sh TAG "params"`,
`digest_cmp.py`, `busy.py`, `gpu.py`, `bench_pair.sh` / `bench_ab.sh` / `bench_exp.sh` (Node benches),
`ctest_native.cmd`, `cpu_sample.ps1` + `cpu_run.sh` + `final_pair.sh` (runs with the browser's CPU). A baseline site is a copy of `build/engine/emscripten/web/site` built from
origin/main (`git checkout <rev> -- engine/`, build, copy, `git checkout HEAD -- engine/`).

## 9. Commits (branch `engine/render-speed`, not pushed)

- `f2b0c4ee` engine(gfx): a fast vertex path for the render thread
- `9acbe90a` merge origin/main (V3)
- `40f92137` engine(gfx): cheaper per-draw work on the render thread
- `4f25767b` engine(gfx): constant-size zeroing, unrolled decoders
- `e72b5357` docs: this log
- `6fe6007c` engine(gfx): XF matrix loads drop cached matrices once per load
- `08a48c7e` engine(web): the render status's gx counts include commands, passes, copies, clears
- `de893fdb` engine(web): the render thread sleeps on its ring without polling first
