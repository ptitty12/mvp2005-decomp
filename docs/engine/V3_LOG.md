# V3 log: 120 fps on displays that can show it (in-between frames)

Side session `claude-side-interp`, branch `engine/interp-120` (from `engine/perf-120` at
`31aa85fc6`). Chrome 155 headed on the dev PC (i7-14700K, RTX 4070 SUPER, Windows 11, one
display at 60 Hz), Playwright through `engine/web/tools/browser_check.mjs`; the in-game scene is
the lead's: the `braves-dodgers-9th-bat` and `-pitch` save states (MLB 2026 pack, ABS mod),
fields 136,700 to 140,200 (58 s of play) unless said otherwise. Every timing run held the
measurement lock (`mlock.py`). Pictures stayed in the session scratch (game-derived).

The owner, 2026-10-21: "I would love the ability to play at 120 fps on devices that are capable."

## 1. Status

| # | Task | State |
|---|---|---|
| 1 | Geometry identity (`DrawCall::ident`) | **Done** (`gx_processor.cpp`; off unless asked for) |
| 2 | Frame record in the renderer | **Done** (`interp.cpp`, hooks in `renderer.cpp`); the live frames unchanged (section 4) |
| 3 | Matching and the in-between replay | **Done**: private EFB, copies and picture; scene cuts, gaps, 30 fps: none made |
| 4 | Presentation (threaded engine) | **Done**: the in-between, then the frame; the page queues them (`presenter.ts`) |
| 5 | Motion setting, Auto | **Done**: Auto / 120 fps / 60 fps, `interp=0|1` (`motion.ts`) |
| 6 | Tests | **Done**: 7 GPU cases in `test_gfx_render`, 1 in `test_gx_processor`, `presenter` and `motion` shell tests |
| 7 | Browser checks | **Done**: digests equal on and off; pictures during a pitch and balls in play; display stats at 120 Hz (emulated); cost on the dev PC and on 4 efficiency cores |
| - | Single-threaded engine, native `mvp-play` | Not interpolated (by design: section 6) |

## 2. Design

**Identity.** `GxProcessor::primitive` hashes, per draw, what stays the same when the game draws
the same object again: the opcode and vertex count, the VCD and the format's VAT, the array base of
every indexed attribute, and the first vertex's bytes as the stream holds them (its indices, or its
direct data). Matrices are not in it: a moving model keeps its identity, and its clip-space
positions are its matrices applied to the same vertices. Skinned players are blended into XF
matrices on the CPU, so they keep theirs too. 2D quads whose positions the CPU writes each frame
(direct data that changes) get a new identity and are simply drawn where they are now.

**Record** (`Renderer`, while on): from one XFB copy to the next, each draw pass's vertex, index and
constant bytes (moved out of the live buffers after their upload, no copy) and its encoded draws
(pipeline, texture bind group with its views, constant offset, scissor, index range, base vertex),
the clears and EFB-to-texture copies in order (with the copy's target as it was), and per DrawCall
the identity, where its vertices went and their clip-space positions before the 2D snap. If the
XFB copy that starts a frame does not clear color and depth, the private EFB starts as a GPU copy
of the live one.

**Match** at the XFB copy of frame N, if N-1 was recorded and ended exactly one VI field before:
the k-th draw of an identity in N pairs with the k-th draw of that identity in N-1 (a hash table of
N-1's identities, each with its draws in order; same vertex count). Where an identity's draws are
far apart in that order (a first vertex more than 0.05 of the screen's width away, 0.1 in NDC, or
another vertex count), they pair by nearest instead, but only if that moves them less than half as
far in all: the game drew them in another order. A crowd of identical billboards that all moved in
a pan stays in order (a test covers both). Draws whose positions did not change at all are
skipped; a draw whose sampled vertices moved more than 0.25 of the screen's width is drawn where it
is now. **No in-between** when under 60 % of the frame's vertices matched or the matched 3D
vertices moved 0.1 of the screen's width (0.2 NDC) on average (a scene cut), or when nothing moved
at all (a menu, a pause: the in-between would be the frame; the page shows it twice as long).

**Replay**: each matched vertex at the mean of its two clip-space positions (all four components:
exactly the frame drawn with the mean of the two frames' matrices, since the transform is linear in
them; a vertex that crosses the camera plane stays where it is now; Q8's 2D snap applied again
above 1x), into a private EFB (color and depth at the current scale), private textures for the
EFB copies the frame makes (draws after such a copy bind the private one: substitute bind groups,
made when needed), and a private picture (three kept, by frame serial). The frame's uploads go
into private buffers at per-pass offsets, the whole replay is one command buffer. Decoded textures
are sampled as they are at the end of frame N; the statistics count when that differs from what
the frame sampled (`retextured`: a texture re-decoded after the frame used it; `early_copies`: a
copy sampled before the frame redid it): 0 and 0 in every run here.

**Present** (`RenderSide::present`, threaded engine): when the picture to show is a new frame N
whose in-between was made from the frame shown last, the in-between goes first, then N, each as an
ImageBitmap; the in-between as `{bitmap, between: true}`. Where the scan-out (P1) mixes the last
three fields (pixels the game alternates at 30 Hz), the in-between is mixed with the two frames
before it the same way, so those pixels look alike in both pictures. While on, the render thread
shows every present at once and the emulation presents every field (`mvp_present_every_field`'s
path); the page shows the pictures from its queue (V1's `queue` mode), one per refresh. The page's
presenter counts in-betweens apart (`display.between`), drops the oldest in-between first when the
queue must be trimmed, and skips in-betweens in `now` mode and on displays measured under 90 Hz.
The presenter's refresh rate is now the count over the last 60 refreshes (the moving average of
1000/dt was biased by uneven intervals).

**When.** The Motion select next to Picture: Auto (default), 120 fps, 60 fps (its tooltip explains
in-between frames); kept per browser; `interp=0|1` overrides it. Auto (`src/motion.ts`) needs
the threaded engine, a display measured at 100 Hz or more (off again under 90 Hz for 2 s), and room to spare over a 2 s
window for 3 s: render thread at most 8.5 ms per field, GPU at most 40 % busy, the game at 0.98x
or better. On, it turns off when the render thread passes 13.5 ms per field, the GPU 80 % or the
game drops under 0.95x for 3 s, and waits 30 s before trying again (doubling up to 5 minutes when
it had come on less than a minute before). The GPU threshold is under the Picture setting's Auto
(85 %): in-betweens go before the resolution does. Allowed online (presentation only); the same
rules apply.

**Determinism.** Nothing here writes guest memory or reaches the emulation: the record and the
replay live in the renderer, the setting reaches the engine only as a render-thread record and the
`every field presents` flag (which changes what is presented, not what runs). Netplay lockstep is
unaffected by either player's setting.

## 3. Files

- `engine/gfx/draw.h`: `DrawCall::ident`. `engine/gfx/gx_processor.*`: `set_identify`, the hash.
- `engine/gfx/wgpu/interp.cpp` (new): record, match, replay, `between()`.
  `renderer.h/.cpp`: the hooks (draw, emit, flush, clear, copy, XFB copy, scan field), the depth
  EFB's CopySrc usage, `ensure_mix_pipe()` (the scan-out mix's pipeline, shared).
- `engine/web/wasm/render_side.*`: `set_interp`, the two-picture present, `interp` in the status.
  `render_thread.*`: `kInterp`, presents shown at once while on. `mvp_web.cpp`: `mvp_interp`.
- `engine/web/src/motion.ts` (new): the setting and Auto. `presenter.ts`: in-betweens. `main.ts`:
  the select, Auto, `fakeHz`, `window.mvp.grab`. `worker.ts`, `protocol.ts`, `mvp-web.d.ts`,
  `static/index.html`. `tools/browser_check.mjs`: `--grab`, `motion` in the summary.
- Tests: `engine/gfx/tests/test_gfx_render.cpp` (7 cases), `test_gx_processor.cpp` (1),
  `engine/web/tests/presenter.test.mjs` (3 more), `motion.test.mjs` (new, 5).

## 4. Exactness: the frames themselves

- `test_gfx_render` "the frames themselves are the same with them on and off": 7 frames with EFB
  copies at 2x, the XFB and the whole EFB read back after each: byte for byte equal, the same draws
  and encoded draws.
- Renderer digests (`digest=1`, one line per VI field: state, vertices, textures, texture memory,
  copies) over the bat scene: interp off against interp on (`d0`/`d1`), against interp on with
  grabs (`i1`), and against Auto switching on mid-run with two Picture changes (`a1`): **4,210
  fields compared each, 0 different**.

## 5. Measurements

### Pictures

`--grab F:N` saved the pictures the render thread sent: always in-between, frame, in-between, frame
(`BFBF...`) in play; frames only on static screens and transitions (no new frame, or a cut).
For each in-between B between frames P and N, the mean absolute difference (0-255, RGB):

| Scene | d(P,N) | d(P,B) | d(B,N) |
|---|---|---|---|
| At bat, pitcher set (bat state, 137,000) | 0.40 | 0.23 | 0.22 |
| Batter intro, camera move (138,200) | 7.56 | 4.67 | 4.46 |
| Ball in play, fielder and ball (pitch state, 140,700) | 9.14 | 6.83 | 6.39 |
| Fast camera after contact (138,460) | 17.53 | 15.09 | 15.51 |
| Crowd in a pan (139,100) | 7.28 | 5.83 | 5.67 |

Symmetric everywhere: the in-between sits between the two frames. Looked at: the batter's swing,
the pitcher, the ball in flight (between its two positions), the fielder and his name label, the
crowd and stairs in a pan, the HUD (unchanged), the transition wipes (the moving panels halfway; a
fading logo at the frame's opacity, since colors and constants are not interpolated). No garbage,
no doubled objects. The speckles around fast-moving players are in the frames too: P1's scan-out
mix taking background-covered-background over three fields for an alternation.

### Display (120 Hz emulated: `fakeHz=120`, interp on)

| Run | pictures in /s | shown /s | in-betweens shown /s | held 1 refresh | dropped | underruns /s |
|---|---|---|---|---|---|---|
| Dev PC (`h2`) | 119.7 | 119.5 | 59.3 | 99.7 % | 12 in 58 s | 0.5 |
| 4 efficiency cores (`e1`, `--affinity F000000`) | 119.4 | 117.3 | 57.3 | 98.4 % | 126 in 58 s | 2.6 |
| The same without in-betweens (`e0`) | 59.8 | 59.8 | 0 | 17 % (2-3 refreshes each, uneven) | 10 | 0 |

`fakeHz` drives the page's presenter from a timer, so these are the queue's numbers at 120 Hz; the
monitor here composites at 60 Hz, so the screen itself could not be looked at at 120 Hz.
`--disable-gpu-vsync --disable-frame-rate-limit` was tried first: animation frames came at 117 a
second but in bursts (two within 2 ms, then ~15 ms), which a real 120 Hz display does not do.

In-betweens made: 3,857 of 3,877 recorded frames (bat scene: 14 gaps, 5 cuts by match, 1 other);
4,993 of 5,009 (pitch scene with balls in play: 12 gaps, 3 cuts).

### Cuts (tuning the thresholds)

Every matched frame by its mean 3D move (NDC; under 0.01 / 0.02 / 0.05 / 0.1 / 0.2 / 0.4 / more)
and by its matched share (under 0.6 / 0.8 / 0.9 / 0.95 / 0.99 / more), from the status JSON's
`shift_hist` and `match_hist`, with the fields of the cuts (`events`):

| Run | move | matched share | cuts (field: share, move) |
|---|---|---|---|
| bat scene (`v1`) | 3469 / 163 / 219 / 8 / 0 / 1 / 1 | 4 / 4 / 100 / 342 / 3405 / 6 | 137,714: 0.16, 0.30; 138,042: 0.52, 0.07; 140,103: 0.10, 0.41 |
| pitch scene, balls in play (`v2`) | 3948 / 614 / 334 / 57 / 2 / 3 / 0 | 5 / 29 / 668 / 899 / 3351 / 6 | 137,279: 0.57, 0.34; 137,633: 0.16, 0; 138,287: 0.35, 0.35; 139,992: 0.34, 0.34 |

Play stays under 0.1 (the camera chasing a ball in play reaches 0.10-0.11 in two frames, made);
camera cuts are at 0.3-0.4 and also match under 60 %, so either test alone would catch them here;
the 0.2 move threshold sits in the gap. A white flash in a transition (a full-screen quad changing
color: the same geometry) is not a cut by either test: its in-between is the flash itself, one
refresh early, which nobody can tell. A grab of 120 pictures across the cut at 137,714 (bat scene,
which is deterministic without input) shows B/F alternation up to it, frame, frame at it (the
transition's new panels: 16 % matched), and B/F again after; across the white flash of the same
transition the in-between is the flash. A frame where nothing moved at all counted once in the bat
scene (`still`).

### Cost (bat scene, Picture Auto = 2x, 58 s)

| | render thread ms/field | GPU ms/field | game speed |
|---|---|---|---|
| Dev PC, off (`p0`) | 5.15 | 1.40 | 1.000x |
| Dev PC, on (`h2`) | 6.52 (+1.37) | 1.88 (+0.48) | 0.997x |
| 4 efficiency cores, off (`e0`) | 9.89 | 1.33 | 1.001x |
| 4 efficiency cores, on (`e1`) | 13.55 (+3.66) | 2.01 (+0.68) | 0.998x |
| Dev PC, Auto on this 60 Hz monitor (`auto60`) | 5.22 | 1.48 | 1.002x (Auto stayed off: nothing recorded) |

Per frame with an in-between (dev PC): 1.09 ms of the render thread (0.38 matching, the rest the
replay's uploads and encoding) for ~7,600 draws, ~46,000 vertices, ~270 replayed WebGPU draws.
A Chrome profile (profiling build, `--trace`) put the rest in `writeBuffer` (the frame's ~2.3 MB of
vertices, indices and constants a second time). The matching was 1.47 ms per frame first
(sorting every draw by identity); the hash-table walk, sampled move measurement and skipping still
draws took it to 0.38.

## 6. Not done, limits

- **Colors and constants** are not interpolated: a fade shows the frame's opacity in its in-between.
  Neither are textures (their contents at the end of the frame), EFB copies sampled taller than copied
  (P2's tall copies: the live texture), and anything the CPU rewrites per frame as direct vertex
  data (drawn where it is now).
- **Single-threaded engine**: no in-betweens (it draws into a canvas of the page; the frame right
  after would replace the in-between). The setting has no effect there; Auto stays off.
- **Native `mvp-play`**: not wired (its window presents once per field).
- **Displays between 90 and 120 Hz, or 144 Hz**: two pictures per frame still; at 144 Hz about one
  picture in six holds two refreshes (no third in-between).
- **Real 120 Hz screens** were not available here: the display numbers come from `fakeHz`.
- **Cost of the upload**: the replay uploads the frame's bytes again. Writing the live passes at
  frame offsets in one buffer would let the replay reuse them and upload only moved vertices; not
  done (it changes the live path's buffer use).

## 7. How to resume

- Build: `build_web.cmd` as in WEB.md 1 (the threaded engine is in `mvp-web-mt`); native tests:
  `engine/tools/msvc.cmd cmake --build --preset msvc --target test_gfx_render test_gx_processor`.
- Scene: `browser_check.mjs --server-disc --states DIR --mock-packs DIR --params
  "state=braves-dodgers-9th-bat&interp=1&fakeHz=120" --fields 140200 --digest --grab F:N`.
  The status JSON's `interp` has the counters (frames, made, gap, cut_match, cut_shift, other,
  shown, reordered, retextured, early_copies, match, shift, ms, ms_match, ranges, vertices,
  replayed); the summary's `display.between` and `motion`.
- Tuning: `Renderer::interp_tune` (cut thresholds) and `DEFAULT_MOTION` in `motion.ts`.

## 8. Commits (branch `engine/interp-120`, not pushed)

- `engine(gfx)`: in-between frames in the renderer (identity, record, match, replay, tests).
- `engine(web)`: the Motion setting, the two-picture present, Auto, presenter counts, `--grab`, `fakeHz`.
- `engine(web)`: `--grab` numbers with three digits.
- `engine(gfx)`: no in-between for a frame where nothing moved. `engine(web)`: `still` in the status.
- `engine(web)`: the select's label shortened.
- `docs`: this log; WEB.md (section 1's `--grab`, section 2's Motion).

The owner's monitor is 59 Hz: Auto stays off there, so nothing changes for him until he plays on a
120 Hz display (or picks 120 fps, which then only costs: the page skips in-betweens under 90 Hz).
PROGRESS.md is left to the lead (shared file).
