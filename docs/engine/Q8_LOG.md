# Q8 log: thin lines at 2x and 3x (fullscreen)

Side session `Q8`, branch `engine/gfx-pass` (from origin/main `f784990cb`). Chrome 155 headed on
this PC (i7-14700K, RTX 4070 SUPER, Windows 11, one 3440 x 1440 display at 59 Hz), Playwright;
native `mvp-play` captures. Captures stay in the session scratch (never committed).

The owner, 2026-10-20: "there are some weird yellow lines when i go full screen, pleaes do a full
pass on graphics to make sure they are in order". PROGRESS listed the same thing as an open item
on 2026-10-17: "thin one-pixel lines across the score bug's base-runner diamond at 3x, natively too".

## 1. Status

| # | Task | State |
|---|---|---|
| 1 | Reproduce | **Done**: in the pitching view, a one-pixel line in the dirt's color across the dark inset of the score bug's base-runner diamond, at 2x and 3x (native captures; the browser runs the same renderer); not at 1x |
| 2 | Root cause | **Done**: 2D edges between pixel centers (section 3) |
| 3 | Fix | **Done**: above 1x, 2D vertices go to whole console pixels (section 4) |
| 4 | The rest of the pass | **Done**: section 2; nothing else found that is the renderer's |

## 2. The pass

Why fullscreen: on this display the fullscreen canvas is 1920 x 1440 CSS pixels (4:3 of the
height), so Auto draws at 3x and the 1920 x 1344 frame reaches the screen almost pixel for pixel.
Windowed, the canvas is 960 x 720 and Auto draws at 2x, scaled down by the area filter: the same
lines are there but averaged with their neighbours.

| Checked | How | Result |
|---|---|---|
| The page in fullscreen | style.css (`#stage:fullscreen`, the canvas's outline), the overlays over the stage | Nothing drawn over the picture; no outline |
| The present (area filter, sharp bilinear) | window.cpp's blit at 1440 x 1080 and 1920 x 1440 canvases | Correct at both (ratio 1.33 area filter; 1.0 x 0.93 plain bilinear) |
| Fullscreen at start and mid-game, threaded and single-threaded | `browser_check --viewport 3440x1440` and `1920x1080`, `--actions F:fullscreen`, the ABS states (`--states`, `--mock-packs`) | Scale changes cleanly (Q7's fix holds); screenshots as native |
| 1x against 3x, every scene at hand | native captures of the same fields at 1x and 3x: the Play Now path (title, menus, team select, line-up, Fenway play), the `braves-dodgers-9th-bat` and `-pitch` states (Dodger Stadium, MLB 2026 pack, pitching view, a home run, replays) | A detector for lines only the 3x frame has (below) |

The detector compares each 3x frame with the 1x frame of the same field: runs of 8 or more console
pixels along one third of a console row (or column) that differ strongly from 1x while the other
two thirds agree, where the 1x picture is flat across the line (so that a 3D edge drawn finer at 3x
does not count). Before the fix it found the mini-map line in 76 of 125 pitching-view frames and
seams along the letter quads of the team-select titles; after it, neither. What it still finds is
3D detail drawn finer at 3x (chalk lines, wall tops, players' edges).

Seen and left alone (the game's own, at 1x too, or correct at 3x):
- **Distant foul poles and flag poles** are thinner than a console pixel. At 1x they fall between
  pixel centers and show as yellow dots or not at all; at 2x and 3x they are continuous yellow
  hairlines. That is the geometry drawn at the higher resolution, not a seam.
- The batting view's hot zone: gaps between its cells and a seam down the middle of the colored
  column, at 1x as at 3x.
- A black band at the right edge for a few fields when the camera changes at Fenway: at 1x too.

## 3. Root cause

Draw log of field 137,400 of the pitch state (`mvp-play --draw-log` with every vertex). The score
bug draws the mini-map's inset like this:

1. scissor from (478, 55);
2. a black quad from (478.08, 55.44) to (577.28, 101.38), writing depth (the inset's backing);
3. a grass-colored quad over the same rectangle;
4. the inset's little 3D field, perspective, clipped by the scissor;
5. the diamond frame over it all (a texture with see-through parts).

A pixel is drawn when its center is inside. At 1x row 55's center is 55.5, inside the backing quad
(from 55.44): the row is black. At 3x the row's top third is sampled at 55.17, outside the quad but
inside the scissor (165-167 at 3x): the 3D field draws its dirt there, and the frame's see-through
part shows it as a line (at 2x: 55.25, the same). Any 2D edge between a pixel center and a pixel
boundary does this at 2x and 3x; the letter quads' seams on the team-select screen were the same
thing.

## 4. Fix

`Renderer::draw` (gfx/wgpu/renderer.cpp): above 1x, a vertex with w exactly 1 (the transform's
orthographic output, the game's 2D) goes to the whole pixel `ceil(v - 0.5)` in x and y before
upload, as Dolphin's "vertex rounding" does. For a horizontal or vertical edge at v that is the
boundary of the pixels it covers at 1x (a center exactly on it counts for a top or left edge, as
the rasterizer's rule has it), so at any scale a 2D rectangle covers whole console pixels, the ones
it covers at 1x; slanted 2D edges (the score bug's diamonds) move by less than half a console
pixel. 1x is drawn unchanged. Perspective draws (the 3D world) are untouched. Rendering only: nothing reaches
guest memory.

## 5. Checks

| Check | Result |
|---|---|
| `test_gfx_render`: "2D edges between pixel centers cover whole console pixels (Q8)" | Passes; fails at 2x and 3x without the fix (8 checks), passes at 1x either way |
| msvc ctest, the renderer's and the play tests (12: render ring, png, TEV, texture decode, GX processor, GP reads and memory, hook, GPU, render, `local_play_determinism`, state file) | 12/12 |
| emscripten ctest, the renderer core and the web shell (png, TEV, texture decode, GX processor, GP reads and memory, hook, web cache, web shell) | 9/9 |
| Native 3x and 2x of the pitch state, field 137,400 | The line's row (165 at 3x, 110 at 2x): (101, 81, 46) and (101, 81, 46) before, (8, 9, 19) and (7, 9, 22) after, dark as at 1x |
| Q7-T1, the scale-change check (`game_125`, 8,250 fields, `scale=3` against Picture 3 at 7,000) | Passes: region mean 0.00, whole picture 0.00 (limit 3) |
| Chrome, the pitch state at 3440 x 1440, fullscreen from field 136,380, threaded engine (cross-origin isolated), A pressed through the at-bat | 3x on a 1920 x 1440 canvas; the mini-map's dark insets clean in the fielding view; GPU errors 0 |
| The threaded page's canvas keeps its 960 x 720 attributes in fullscreen (the ImageBitmaps are 1920 x 1440): does that blur it? | No: a round trip of the screenshot through 960 x 720 removes half its detail (0.52 of the gradient energy left), as for the single-threaded page (0.58) |
