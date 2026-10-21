# V6 log: Chrome's GPU process, and the uploads

Side session `claude-side-gpu`, branch `engine/gpu-batch` (from `engine/perf-120` at `c40594c7`:
V1, V2, V3, V5; V4 merged in at `7ab11c0b`). The owner, 2026-10-21: "still dropping frames on
some devices": make the game run as efficiently as possible. This session's part: the CPU that
**Chrome's GPU process** spends running our WebGPU work (V5_LOG section 6b: about 2 ms a field,
17 render passes a frame, a submit and three `WriteBuffer`s per pass), without changing a pixel.
Same PC and tools as V5 (i7-14700K, RTX 4070 SUPER, Windows 11, Chrome 155 headed through
Playwright); every timing run held the measurement lock (`mlock.py`).

## Status

| # | Change | Commit | What it did |
|---|---|---|---|
| B | One command encoder per batch; clears drawn inside the open pass; EFB copies, XFB copies, tall copies and the scan-out copy in the batch; presents, scan-out mixes and the GPU timer's resolve in the batch's submit; the blit's bind groups kept | `0f2e22a3` | submits 17 -> 3 a field; GPU process main thread 170 -> 102 ms a second |
| R | The in-between replay reads the live frame's indices, constants and unmoved vertices | `ba7c8487` | replays upload 64% of the vertex bytes and no indices or constants; render thread -7% with interp on (E-cores) |
| G | One bind group per draw (the TEV constants join the textures' group); 16-bit indices | `4d8e9ddc` | ~217 fewer wire commands and 0.19 MB less upload a field; GPU process main thread about -4% |
| S | Uploads through staging buffers mapped for writing (`staging=0`: queue writes) | `764bb2ef`, `29ad7a71` | render thread -6% (dev PC) to -10% (E-cores); GPU process about the same |
| - | `browser_check --trace-cats` (the GPU process's Dawn events in traces) | `c7649289` | tool |
| - | Writes in 256 KiB / 1 MiB pieces; whole 4 MiB staging buffers | tried, dropped | section 4 |

**In short** (section 5: the V4 baseline against this branch, interleaved). Chrome's GPU
process **-33% on the dev PC, -41% on 4 efficiency cores** (0.19 -> 0.13 and 0.30 -> 0.18
cores; its main thread 170 -> 103-109 ms a second, submits 1,018 -> 179 a second), the
browser's CPU in all -12% / -14%, and the render thread -12% / -17% (4.27 -> 3.75 and 8.10 ->
6.69 ms a field); with in-between frames on (`interp=1&fakeHz=120`) GPU process -31% / -30%,
browser -11% / -14%, render thread -16% / -19%. The emulation thread gets a little more room on
the E-cores (11.8 -> 11.4 ms a field).

Pictures: renderer digests equal in every field, the final picture and 30-59 grabbed pictures
at each of three points (frames and in-betweens) byte-identical, with `interp=0` and with
`interp=1&fakeHz=120`, at every commit (section 6).

## 1. How it was measured

- **The scene**: V1's, as in V5 (`braves-dodgers-9th-bat`, paced, fields 136,700 to 140,200,
  58 s). A copy of the lead's `run_scene.sh` (scratch `v6/`) with this worktree, its own Chrome
  profile and output folder. Baselines are copies of the site built from the commit before
  (the V4 merge `dfd8f97a` for the final table), so each pair compares two sites, interleaved
  base, new, base, new.
- **The browser's CPU by process** (`cpu_sample.ps1`, `cpu_run.sh`, `cpu_show.py`): every
  second, each `chrome.exe` of the run's profile, split by command line into the GPU process
  (`--type=gpu-process`), the renderer processes (the page and its workers) and the rest;
  cores over the last 44 s of the paced part. V5's meter, with the split added.
- **Chrome traces** (`--trace 137500:6000 --trace-cats gpu.dawn,disabled-by-default-gpu.dawn`,
  `gpu_proc2.mjs`): the GPU process's main thread (`CrGpuMain`), busy time and self time per
  event in ms per second of trace, with Dawn's own events (`Queue::Submit`,
  `D3D12Device::SignalFence`, `Queue::Tick`). A 6 s trace's busy time moves by about +-4% from
  run to run; small changes took 4 runs each.
- **Render thread**: the status's `render.busy_ms` per field (`busy.py`: mean, and the median
  of 0.5 s windows). The render status's `gx` object gained `submits`, `staged`, `unstaged` and
  `staging_mb` (render_side.cpp), the `interp` object `upload_bytes` and `frame_bytes`.
- **Exactness** (`exact_pair.sh`): the baseline site and the new one, each with `--digest` and
  `--grab 137000:60,138200:60,139100:60` to field 139,400; `digest_cmp.py` compares the digests
  field by field; the final pictures by MD5; `grab_cmp.py` aligns the two runs' grabbed
  pictures by content (a grab starts wherever the poll saw its field) and compares every
  overlapping picture's decoded pixels.

## 2. Where the GPU process's time went (at the start)

Trace of `c40594c7` (V5), 6 s: the GPU process's main thread busy **169.6 ms a second**:

| Event (self time) | ms/s | per second |
|---|---:|---:|
| `Queue::Submit` (Dawn: recording the D3D12 command lists, ExecuteCommandLists) | 64.3 | 1,012 |
| `WebGPUDecoderImpl::HandleDawnCommands` (the wire's commands, Dawn's validation and the `WriteBuffer` copies) | 40.0 | 1,673 |
| Skia / the compositor (`SkiaOutputSurfaceImplOnGpu::*`, swap chain present, image access) | ~25 | 60 |
| `CommandBufferService:PutChanged`, scheduler, `D3D12Device::SignalFence`, ticks, polling | ~25 | |

17 submits a field: each draw pass (7.5 a field, split by every clear and EFB copy), each clear
(own pass, own bind group, own buffer write), each EFB-to-texture conversion, the XFB copy, the
scan-out copy, the scan-out mix, the present and the GPU timer's resolve; three `WriteBuffer`s
per draw pass. A submit cost the GPU process about 40 us on its own.

## 3. Kept

### 3.1 One command encoder per batch (`0f2e22a3`)

- The renderer keeps one `CommandEncoder` open (a *batch*); draw passes on the EFB, the EFB's
  conversions to copy textures, XFB copies, P2's tall copies and the scan-out copy all go into
  it, in order. The batch's vertices, indices and constants grow through it (base vertex,
  first index and dynamic offsets point into them) and `flush()` writes each once and submits.
- **Clears** are a draw inside the open pass (their own pass and submit before). Within a pass
  draws land in order, so the draws before see nothing of it and the draws after see it, as
  with a pass of its own. Their constants, like the conversions', are slots of the batch's
  constants, bound by dynamic offset: no bind group or buffer write per clear or copy.
- **Submit points**: a VI field (after its scan-out copy), a present, a read-back, the
  in-between replay (interp on: the XFB copy), and a texture upload over contents the batch
  samples. That last one is now decided by draw serials: each draw that resolves its textures
  marks them; a flush records the last draw it submitted (not a draw still resolving its
  textures: its commands come after). The pass serial before could miss a texture an earlier
  stage of the same draw had marked before a flush in a later stage.
- **Presents**: the scan-out and in-between mixes and the blit are drawn into the batch, with
  the GPU timer's resolve, and submitted with it before the picture is taken as an ImageBitmap:
  one submit per picture. The blit's bind groups are kept per source view (holding the view)
  and its parameters written only when they change.
- **V3's record**: one data set per batch, ops naming draw ranges (clears and copies inside a
  batch replay in order); the replay binds clear and conversion constants by dynamic offset.

Trace: busy 169.6 -> 102.0 ms a second, `Queue::Submit` 1,012 -> 179 a second (64.3 -> 30.6
ms), `HandleDawnCommands` self 40.0 -> 26.0. Passes 7.5 -> 5.5 a field (the clears).

| Interleaved, against the V4 merge | base | new |
|---|---|---|
| GPU process, dev PC (cores; 3 pairs) | 0.172 / 0.194 / 0.192 | 0.126 / 0.109 / 0.134 |
| Browser, dev PC | 0.944 / 1.016 / 0.951 | 0.906 / 0.908 / 0.874 |
| Render thread, dev PC (ms a field) | 4.18 / 4.20 / 4.23 | 4.13 / 4.03 / 4.04 |
| GPU process, 4 E-cores (`--affinity F000000`) | 0.300 / 0.268 | 0.190 / 0.190 |
| Browser, 4 E-cores | 1.516 / 1.535 | 1.348 / 1.389 |
| Render thread, 4 E-cores | 7.63 / 8.02 | 7.38 / 7.49 |
| GPU process, 4 E-cores, interp on | 0.389 / 0.419 | 0.311 / 0.291 |
| Browser, 4 E-cores, interp on | 1.821 / 1.941 | 1.704 / 1.716 |
| Render thread, 4 E-cores, interp on | 11.45 / 11.46 | 10.73 / 10.80 |

The emulation thread on the E-cores went 11.7 -> 11.3 ms a field too (less contention).

### 3.2 The replay reads the live frame (`ba7c8487`)

While a frame is recorded (interp on) the live buffers fill from offset 0 for the whole frame
(each flush writes what its batch added), so at the frame's XFB copy everything it drew is
still there; the replay is submitted right after and the next frame writes after it. The
replay binds the live index buffer and the live constants (draws, clears and conversions: the
record keeps their slots), and the live vertex buffer for every draw without a vertex at a
halfway position; the moved draws' spans of the frame's vertex bytes go into the replay's own
buffer at the same offsets (runs joined over gaps under 4 KiB). It uploaded the whole frame
again before (vertices, indices, constants), with a bind group per clear and per copy. In the
bat scene the replays uploaded 64% of their frames' vertex bytes. A frame whose data overflows
the buffers gets no in-between (as an oversized frame did before).

4 E-cores, interp on, against 3.1: render thread 10.77 / 10.78 -> 10.08 / 10.04 ms a field, GPU
process 0.315 / 0.317 -> 0.273 / 0.295 cores, browser 1.76 / 1.71 -> 1.63 / 1.65.

### 3.3 One bind group per draw, 16-bit indices (`4d8e9ddc`)

A census of `emit_pending` (a temporary build) in the bat scene: 267 WebGPU draws a field, 266
changing the texture bind group, 217 the constants' offset, 45 the pipeline, 6 the scissor; no
two consecutive draws with the same pipeline and textures (so draws cannot be merged further
without changing what they draw). An experiment setting each group twice put a wire command at
about 0.3 us of the GPU process's main thread.

- The TEV shader's constants move into the textures' group as binding 16 (a declaration change
  in tev_wgsl.cpp; the code that reads them is the same): one `SetBindGroup` with its dynamic
  offset per draw instead of two. The replay's substitute groups carry the live constants too.
- Indices are 16-bit (0.37 -> 0.19 MB a field). A run of one state that would pass 65,536
  vertices starts a new WebGPU draw (a GX primitive has at most 65,535); a new test draws 76,800
  vertices of one state and fails without that split. Writes stay whole 4-byte words (an odd
  batch is padded with an index no draw reads).

Traces, 4 each, interleaved: busy 100.7 / 104.8 / 108.9 / 108.2 -> 100.8 / 96.3 / 100.4 / 108.7
ms a second (about -4%); render thread 4.08 / 4.08 / 4.08 / 4.09 -> 4.05 / 4.02 / 4.03 / 4.03.

### 3.4 Staging buffers (`764bb2ef`)

The render thread's profile (trace, `writeBuffer` self time): 0.6 ms a field for 2.6 MB, 0.25 ms
per MB. An experiment writing the vertices twice cost the render thread 0.4 ms and the GPU
process's main thread 13.5 ms a second for 2.2 MB more a field: the uploads were the largest
WebGPU cost left on both sides. Splitting the writes into 256 KiB or 1 MiB pieces changed
nothing measurable.

A batch's uploads now go into a buffer mapped for writing (`MapWrite | CopySrc`,
`WriteMappedRange`: one copy into the mapped memory), are copied on the GPU into the live
buffers by a command buffer submitted just before the batch's, and the buffer is mapped again
(`MapAsync`, resolved once the GPU is done with it). The replay's moved spans go the same way.
The pool grows on demand to 32 MiB at most (26-32 MiB in the bat scene: the maps come back
several batches later); a buffer is mapped again for a little more than its last use, because
the GPU process copies a buffer's whole mapped range at unmap (whole 4 MiB buffers cost it about
25% more). Uploads under 256 KiB, a full pool, or the page parameter `staging=0` take queue
writes (`mvp_gpu_staging`).

It is a trade: the render thread does less, the GPU process the same or a little more (it
copies the data once either way and handles the maps):

| Interleaved, against 3.3 | base | new |
|---|---|---|
| Render thread, dev PC (ms a field) | 4.00 / 4.02 / 4.26 | 3.76 / 3.74 / 3.88 |
| GPU process, dev PC (cores) | 0.122 / 0.119 / 0.132 | 0.141 / 0.142 / 0.127 |
| Browser, dev PC | 0.84 / 0.90 / 0.95 | 0.89 / 0.91 / 0.89 |
| Render thread, 4 E-cores | 7.36 / 7.41 | 6.70 / 6.68 |
| GPU process, 4 E-cores | 0.196 / 0.180 | 0.190 / 0.200 |
| Browser, 4 E-cores | 1.40 / 1.40 | 1.39 / 1.37 |
| Render thread, 4 E-cores, interp on | 9.95 / 9.93 | 8.75 / 8.96 |
| GPU process, 4 E-cores, interp on | 0.289 / 0.266 | 0.279 / 0.299 |
| Browser, 4 E-cores, interp on | 1.67 / 1.62 | 1.59 / 1.59 |

(The rows come from the change's sizing variants, which differ only in how big the buffers
are.) Kept for the render thread, the busiest thread after the emulation; `staging=0` turns it
off if the GPU process or the memory (up to 32 MiB of mapped buffers) matter more on a device.

## 4. Tried, not kept

- **Writes in pieces** (256 KiB, 1 MiB): no change on either side (the wire's chunking is not
  what costs).
- **Whole 4 MiB staging buffers** (any buffer takes any batch): the GPU process +25%, the pool
  at its 32 MiB limit; the unmap copies the whole mapped range.
- **Staging only large uploads** (to hold fewer buffers): both batches of a field (at the VI
  field and at the present) are over 1 MB in play, so a threshold changes nothing there; it
  stays at 256 KiB (smaller uploads take queue writes).

## 5. Together

The V4 merge (`dfd8f97a`, engine/perf-120) against `764bb2ef` (the last change measured;
`29ad7a71` after it only adds a fallback), interleaved base, new, base, new, every set under the
measurement lock (`final_all.sh`). Cores are the browser's processes over the last 44 s of the
paced scene; render thread and emulation are ms of work per field (run means).

| Configuration | | V4 merge | V6 | change |
|---|---|---|---|---:|
| Dev PC (3 pairs) | GPU process | 0.199 / 0.193 / 0.188 | 0.128 / 0.123 / 0.135 | -33% |
| | browser, all processes | 1.015 / 0.964 / 1.017 | 0.897 / 0.859 / 0.885 | -12% |
| | render thread | 4.27 / 4.27 / 4.27 | 3.76 / 3.75 / 3.75 | -12% |
| | emulation (median) | 6.82 / 6.79 / 6.82 | 6.78 / 6.79 / 6.74 | |
| 4 E-cores, `--affinity F000000` (2 pairs) | GPU process | 0.302 / 0.302 | 0.179 / 0.176 | -41% |
| | browser | 1.579 / 1.573 | 1.354 / 1.362 | -14% |
| | render thread | 8.10 / 8.09 | 6.64 / 6.73 | -17% |
| | emulation (median) | 11.73 / 11.81 | 11.44 / 11.35 | -3% |
| Dev PC, interp on (2 pairs) | GPU process | 0.236 / 0.241 | 0.160 / 0.170 | -31% |
| | browser | 1.107 / 1.092 | 0.949 / 1.006 | -11% |
| | render thread | 5.71 / 5.72 | 4.77 / 4.79 | -16% |
| 4 E-cores, interp on (2 pairs) | GPU process | 0.404 / 0.395 | 0.288 / 0.268 | -30% |
| | browser | 1.828 / 1.920 | 1.637 / 1.584 | -14% |
| | render thread | 10.59 / 11.48 | 9.01 / 8.76 | -19% |
| | emulation (median) | 12.04 / 12.06 | 11.75 / 11.54 | -3% |

Game speed 0.997-1.002x in every run; pictures in and shown per second unchanged.

Traces (6 s, interleaved): the GPU process's main thread busy 170.7 / 170.3 -> 108.6 / 103.4 ms
a second; `Queue::Submit` 1,018 -> 179 a second, 67.0 -> 33.2 ms; `HandleDawnCommands` self
41.9 -> 26.1 ms; `D3D12Device::SignalFence` 4.3 -> 1.2 ms. What is left there: recording the
D3D12 commands of 267 draws a field (about 30 ms a second), the wire's commands and the
uploads' copies (about 26), the compositor showing the page (about 25: Skia, the swap chain,
the ImageBitmap's image access), Chrome's scheduler (about 17).

## 6. Exactness

- Renderer digests (`--digest`), baseline against each commit's build, fields 1-139,400 of the
  scene: 3,410 fields compared, 0 differ, with `interp=0` and with `interp=1&fakeHz=120`, at
  every commit.
- The final picture of each run byte-identical to the baseline's.
- Grabbed pictures (`--grab`, the pictures the render thread sends, frames and in-betweens): at
  three points, every overlapping picture's pixels identical (30-60 per point; 8 and 12 twice,
  where the poll started the two runs 24-26 fields apart).
- `test_gfx_render` 19 of 19 natively (Dawn D3D12; a new case for the 16-bit split), the
  emscripten gfx tests 8 of 8 (Node), `test_tev_wgsl`, `test_gx_processor`,
  `test_vertex_exact` (msvc), `test_web_shell`.

## 7. Not done (ideas, with what stands in the way)

- **Uploads that repeat**: in the bat scene 99% of the index bytes and 41% of the vertex bytes
  of a frame repeat the frame before's (by draw, a census build); with positions apart, 46% of
  the other attributes repeat even while the camera moves. A content-addressed ring of the live
  buffers would skip them, at the cost of hashing every draw's bytes (about 0.13 ms a field) and
  a second vertex stream (the replay writes positions in place today).
- **The VI field's submit**: two batches a field (at the VI field and at the present). One per
  present would save a submit (about 40 us of the GPU process) but give the GPU the frame's work
  later.
- **Per-draw work in the GPU process** (`Queue::Submit` about 30 ms a second now): proportional
  to the 267 draws a field, each with its own textures; fewer texture bindings per group (8
  always bound, mostly 1-2 used) might make each cheaper; untried.
- **The compositor** (~25 ms a second of the GPU process): presenting each picture as an
  ImageBitmap to the page's `bitmaprenderer` canvas (V1's presenter); not this session's part.

## 8. How to resume

Scratch folder `v6/` (in the lead's scratch): `build_web.cmd`, `build_native.cmd TARGETS`,
`run_scene.sh TAG "params" [args]` (SITE), `final_pair.sh TAG ROUNDS [args]` (BASESITE,
NEWSITE, PARAMS), `cpu_run.sh` / `cpu_sample.ps1` / `cpu_show.py` (the browser's CPU by
process), `exact_pair.sh TAG "params"` with `digest_cmp.py` and `grab_cmp.py`, `trace_set.sh TAG
SITE...` with `gpu_proc2.mjs` (the GPU process's threads), `gxshow.py` (the status's gx counts
per field), `final_all.sh`. Baseline sites: `site-base4` (the V4 merge), `site-b*` per step.

## 9. Commits (branch `engine/gpu-batch`, not pushed)

- `c7649289` engine(web): browser_check --trace-cats
- `0f2e22a3` engine(gfx): one command encoder per batch, clears inside the pass, presents in the batch's submit
- `7ab11c0b` merge engine/perf-120 (V4)
- `ba7c8487` engine(gfx): the in-between replay reads the live frame's indices, constants and unmoved vertices
- `4d8e9ddc` engine(gfx): one bind group per draw, 16-bit indices
- `764bb2ef` engine(gfx): uploads through staging buffers mapped for writing
- `29ad7a71` engine(gfx): staging falls back to queue writes if a mapped write is refused
