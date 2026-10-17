# Q7 log: the block in the top-left corner

Side session `Q7`, branch `engine/q7-topleft` (from origin/main `7ab8e2ca`). Chrome 155
headed on this PC (i7-14700K, RTX 4070 SUPER, Windows 11), Playwright, the owner's CISO picked
as a file; Q6 benchmarked on the same PC during the session. Captures stay in the session
scratch (never committed).

The owner, on the deployed build: "there seem to be some issues in the top left corner with the
graphics where a sizable block is not updating or is in the wrong spot or something".

## 1. Status

| # | Task | State |
|---|---|---|
| 1 | Reproduce | **Done**: after any mid-session change of the internal resolution (Picture setting, Auto following a resize or fullscreen, Auto's GPU step-down), the top-left 128 x 128 block of the game's frame (console pixels) stays frozen at its content from the moment of the change; it shows when the camera moves (a ball in play). The game's other render-to-texture effects freeze with it |
| 2 | Root cause | **Done**: the renderer's bind-group cache was keyed by texture-view addresses, and in the browser (emdawnwebgpu) a released view's address is reused by the next view; a scale change replaces every EFB-copy texture, so the draws that sample the new copies found the bind groups of the old ones (section 3) |
| 3 | Fix | **Done**: cache entries hold references to their views and samplers; a replaced copy's entries are dropped (section 4) |
| 4 | Regression checks | **Done**: `test_gfx_render` (a copy redone across scale changes is the one sampled) and `browser_check --compare` (the scale-change check, section 5) |
| 5 | Exactness, tests | Section 6 |

## 2. What was tried

All in Chrome 155 headed (Playwright), the threaded engine unless noted; CDP screenshots and
screencasts of the page; `browser_check` gained `--actions F:KIND[:ARG]` (resize, picture,
fullscreen, stats at a field) for this.

| Run | Scenario | Result |
|---|---|---|
| a_base | CLK2's `game_125` to 9,100 fields, Auto (2x at dpr 1), shots at 9 fields | Fine; guest line `7ed924af` (Q1's) |
| b_pic | the same, Picture 3/1/2/3 in the menus, 1/3/2/1/Auto in play | Stills fine (the camera was the at-bat view at each change, see below) |
| c_dig | CPU vs CPU to 11,600, threaded and single-threaded, renderer digests | 0 of 11,600 fields differ; `d8c30e9a` (Q1's) |
| d_intro | no input, paced, 4,300 fields, screencast | No frozen cells (`stale.py`) |
| e_rs | dpr 1.5 (Auto 3x), paced play: window 600x800 and back, fullscreen | Stills fine at each change |
| g_long | CPU vs CPU to 33,600, unpaced, screencast of 16,000 frames | No seam at the top-left block (`seam.py`), scale fixed all along |
| k_load | dpr 1.5, `gpuLoad=150` (Auto steps 3x -> 2x at field ~7,000), paced, screencast | **Reproduced**: from the first ball in play after the step-down, the top-left 128 x 128 block shows the at-bat view while the rest of the frame follows the ball |
| m_bug | `game_125`, Auto 2x, Picture 3x at field 7,000, stop at 8,250 | **Reproduced**: the top-left block shows the at-bat camera's light tower and wall, cut off at x = 128, y = 128, over the outfield view |
| native | `mvp-play` 1x and 3x captures of the same fields, a draw log of fields 24,300-24,301 | 1x and 3x agree; the draw log shows the game's per-frame save and restore of that block (section 3) |

Other leads checked and ruled out on the way:
- the threaded and single-threaded renderers get identical commands in every field (digests);
- copies sampled at another address or size than copied (`copy_inside`, new: 0; size misses 0);
- the ImageBitmap path when the page's canvas keeps its first size (Chrome stretches the bitmap to
  the element: checked with a test page);
- the statistics overlay (Q5) is the only other block at the top left (a text table; not shown
  unless Stats is on);
- the ABS challenge scene (not deployed; mods force the single-threaded engine).

## 3. Root cause

**What the game does in every frame of play** (`mvp-play --draw-log`, field 24,300): after the
stadium, it copies the EFB's top-left 128 x 128 to a texture at `0x00B58280` (no clear), draws
the ball's shadow map into that corner (a black fill and gray meshes, scissor 0,0-128,128),
copies it to `0x00B48260`, then draws `0x00B58280` back over the corner (z always, no blend):
a save and restore of the block. It also copies a 64 x 64 corner to `0x01386100` (sampled by
about 3,000 draws a field: the players' second texture) and 512 x 448 at half scale to
`0x01797180` (the stadium's video board, P2's tall copy).

**What the renderer did.** `Renderer::draw` caches the texture bind group of each set of
eight views and eight samplers in `bind_groups_`, keyed by the handles' addresses (E1,
`efe9feea`). The comment said a cached group keeps its views alive, so an address is never
reused while the entry exists. That holds in native Dawn (a bind group references its views);
it does not in the browser: emdawnwebgpu's C++ handle of a view is freed when its last
reference goes (`webgpu.cpp`, `RefCounted::Release` then `emwgpuDelete`), the JavaScript bind
group only holds the JavaScript view, and the next `CreateView` can return the same address.

A change of the internal resolution (Q1, `40ef87f2`) recreates every EFB-copy texture at the
new size in `copy_to_texture` (and the tall copies): the old view is released and the new view,
created right after, gets the freed address. The draws that sample the new copy then find the
old bind group, which samples the old texture, which nothing writes any more. So from the first
frame at the new scale:
- the restore draws the block as it was at the change: **a frozen 128 x 128 block in the top-left
  corner** (it matches the at-bat view, so it shows only when the camera moves);
- the shadow map, the players' second texture and the video board freeze the same way (the
  player shading differs from a run at a fixed scale).

It lasts until the cache passes 4,096 entries and is cleared. Native builds never show it,
which is why `mvp-play` captures at 1x and 3x agree.

**When it happens to the owner.** Picture Auto changes the scale mid-session (Q1: by the
canvas size, so toggling fullscreen on the desktop goes 2x <-> 3x; Q5-G1 `cb54c053`: one step
down when the GPU is busy, likely within seconds of play on the ThinkPad's 780M), and so does
choosing a Picture value during a game. Before Q1 the cache could only alias when the game
copied another size to the same address, which is rare. A fixed Picture setting (not Auto)
chosen before the game starts avoids it until the fix is deployed; reloading the page clears it.

## 4. The fix

`gfx/wgpu/renderer.h/.cpp` (renderer only; nothing reaches guest memory):
- each `bind_groups_` entry (`BindEntry`) holds references to the views and samplers in its key,
  so no address in a key can belong to another object while the entry exists, on any WebGPU
  implementation;
- `forget_view()` drops the entries of a copy's view when the copy texture is replaced (a new
  size or scale), dropped (its memory changed) or its tall texture rebuilt, so the old textures
  are freed rather than kept by the cache;
- the open pass forgets which group it last bound whenever the cache frees groups (its
  4,096-entry clear, `forget_view()`): `emit_pending()` skips a bind by address, and a freed
  group's address could come back for the next group.

Diagnostics: `copy_inside` (status JSON, `mvp-play`'s summary): textures that start inside an
EFB copy's memory but not at its address (0 in play); `mvp-play --draw-log` prints each copy's
destination stride.

## 5. Regression checks

- `test_gfx_render`, "an EFB copy redone at a new scale is the one sampled (Q7)": six frames
  that each copy a colored square and draw it back, across 1x -> 2x -> 2x -> 3x -> 1x -> 2x;
  every frame must show its own color. Native Dawn could not alias before either; the test
  pins the behavior the browser now has too.
- **The scale-change check** (`browser_check --compare`, WEB.md): the same script at a fixed
  scale, then with the scale changed mid-game, both stopped at the same field; the final pictures
  must agree in the top-left block. The worker now presents the batch that stops the run, so
  the final picture is the stop field's (it showed the batch before).

The check, Chrome 155, `game_125` unpaced to field 8,250 (a ball in play, the camera on the
outfield), viewport 1100 x 1200 at dpr 1 (Auto: 2x), the same guest line in every run
(`MEM1 57692a1f...`):

| Run | Scale | Top-left block (mean, worst 16 x 16 cell) | Whole picture | Result |
|---|---|---:|---:|---|
| reference | `scale=3` from the start | - | - | - |
| fixed engine | Auto 2x, Picture 3x at field 7,009 | 0.00 (0.0) | 0.00 | pass: pixel-identical |
| engine before the fix (same worker) | Auto 2x, Picture 3x at field 7,000 | 32.81 (65.6) | 1.88 | fails |

The desktop's case, fullscreen: viewport 1600 x 1300 at dpr 1, so Auto is 2x in the window
(canvas 960 x 720) and 3x in fullscreen (1600 x 1200); the Fullscreen button at field ~7,000,
stop at 8,250 in fullscreen; the reference the same with `scale=3`:

| Run | Top-left block (mean, worst cell) | Whole picture | Result |
|---|---:|---:|---|
| fixed engine | 0.00 (0.0) | 0.00 | pass |
| engine before the fix | 33.38 (65.9) | 1.91 | fails |

Before the fix the final picture shows, in the top-left 128 x 128, the at-bat view's light
tower and wall, cut off at x = 128 and y = 128 over the outfield (the "SE'S" sign split at the
block's edge); the batter and pitcher shading differs too. After it, the picture is the
reference's.

(more in section 6)

## 6. Exactness and tests

The changes are host-side (the renderer's caches and the worker's last present): nothing reaches
guest memory or the inputs. On the final build (`0d933e45`), Chrome 155, unpaced:

| Run | Guest line | Engines |
|---|---|---|
| 700 fields, no input | 4,547,010,125 instructions, time base 473,646,888, 360 polls, MEM1 `53221c4a...` | threaded, single-threaded |
| 2,400 fields, START x4 | 15,574,026,125, 1,622,294,388, 1,882 polls, MEM1 `4285f135...` | threaded, single-threaded |
| 3,600 fields, `menus` | 23,357,802,126, 2,433,104,388, 2,747 polls, MEM1 `40a7692d...` | threaded, single-threaded |
| CPU vs CPU (`game_125`), 11,600 fields | 75,249,642,125, 7,838,504,388, 10,544 polls, MEM1 `d8c30e9a...` | threaded, single-threaded |
| `netplay_check`, E5c's script to frame 3,300, delay 3 | 27,484,589,886, 2,862,978,113, 3,301 polls, MEM1 `58f2ab77...` on both pages (the `--expect` line), 55 windows equal, no desync | two threaded pages |

All equal to WEB.md's CLK2 lines and Q1's match line. Every run of sections 2 and 5 also
ended on the same guest line as its pair (`57692a1f...` at 8,250, `4521313...` at 9,300), with
and without the fix, whatever the scale changes.

The step-down scenario of section 2 (`k_load`, dpr 1.5, `gpuLoad=150`, Auto 3x -> 2x at field
~7,000, paced) on the final build: the strongest seam at the top-left block's edges over 2,359
composited frames is 3.5 (a replay's picture-in-picture), against 11.3 before (the frozen block
during the fly ball).

Tests:
- `ctest --preset msvc`: 49, 48 passed, `local_disasm` skipped as always (the local ones with the
  disc included: `local_play_determinism`, `local_boot_twice`, ...); `test_gfx_render` 10 cases,
  104 assertions, with the new case.
- `ctest --preset emscripten`: 42, 41 passed, `local_disasm` skipped.
- The web shell (`test_web_shell`, node --test): 91 of 91.

## 7. Commits

On `engine/q7-topleft` (from origin/main `7ab8e2ca`), oldest first; not pushed:

- `7ee758d9` engine(gfx): texture bind groups hold their views; a replaced EFB copy's groups go
  with it (the fix, `test_gfx_render`'s case, `copy_inside`, the draw log's stride)
- `76e99cad` engine(web): the scale-change check: `browser_check --actions` and `--compare`; the
  stopping batch is shown
- `0d933e45` engine(gfx): the pass binds its texture group again after the cache frees groups
  (`emit_pending()` skips a bind by address; a freed group's address could come back)
- this log and WEB.md (docs)

Scratch tools (not committed): `run.sh`, `natcap.sh`, `exact.sh`, `chain.sh`, `sheet.py`,
`tlsheet.py`, `stale.py` (frozen cells in a screencast), `seam.py` (a seam at the top-left
block's edges), `cmpscale.py`, `copyuse.py` and `order.py` (copies and draws in a draw log).

## 9. Also seen (not fixed here)

- At 3x, thin one-pixel lines cross the score bug's base-runner diamond (field ~17,264 and
  ~24,300 of `game_125`), natively too: an edge of something the game draws in console pixels
  that 3x resolves. Small; worth a look with a draw log of those fields.
- The base-runner diamond's empty-base panels and the second-base panel's crowd at field 24,300
  are the same at 1x and 3x, natively and in the browser; whether the console draws them the
  same way is not known.

## 8. Decisions to record (for the lead)

- **Q7-G1, the renderer's object caches:** a cache keyed by WebGPU handle addresses must hold
  references to those handles (in the browser a released handle's address is reused at once).
  The texture bind-group cache does; a replaced EFB-copy texture's entries are dropped.
- **Q7-T1, the scale-change check:** `browser_check --compare` of a run that changes the
  Picture setting mid-game against one at a fixed scale, top-left 128 x 128 block, mean
  difference at most 3; run it when the renderer's copies, caches or scale handling change.
