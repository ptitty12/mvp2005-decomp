# The browser build (E2, E2c)

How the engine runs in a browser: the page gets the disc image (the player picks
their own, or, on the owner's deployment with its player link, the browser downloads the
owner's image from the server once and keeps it: P1), translates
the game to WebAssembly inside the browser
(or loads the translation it cached on an earlier visit), draws the frames on a
canvas through WebGPU, plays the sound through an AudioWorklet, and reads the
keyboard and controllers (`docs/engine/Q3_LOG.md`). Nothing game-derived is ever sent from the browser.
Working logs and evidence: `docs/engine/E2_LOG.md` (E2: browser boot) and
`docs/engine/E2C_LOG.md` (E2c: sound, gamepad, the disc from the server, caching,
polish) and `docs/engine/P1_LOG.md` (P1: the disc downloaded once into the browser;
remote-read stalls measured). Toolchain versions: `docs/engine/TOOLCHAIN.md`.

## 1. Build and serve (agents)

Everything below is run by agents; the owner never runs commands.

1. **Tools once per checkout:** `npm ci` in `engine/web` (TypeScript and
   Playwright, pinned in `package.json` and `package-lock.json`; `node_modules/`
   is ignored by git), with the SDK's Node: `..\tools\emsdk.cmd npm ci` from
   `engine/web`. Browser builds for Firefox and WebKit:
   `node node_modules/playwright/cli.js install firefox webkit` with
   `PLAYWRIGHT_BROWSERS_PATH=%USERPROFILE%\tools-ext\playwright-browsers`.
   Chrome and Edge are the installed system browsers.
2. **Configure and build** (from `engine/`):
   `tools\emsdk.cmd cmake --preset emscripten`, then
   `tools\emsdk.cmd cmake --build --preset emscripten -j 8 --target mvp-web`
   (about 20 s, plus the threaded engine below: its own tree,
   `build/engine/emscripten/mt`, a few minutes the first time; `-DMVP_WEB_MT=OFF`
   skips it; the whole preset builds the tests and, with `MVP_DISC`, the
   C++-emitted dev path too). The preset finds emdawnwebgpu under
   `%USERPROFILE%\tools-ext\emdawnwebgpu\<Dawn release>\emdawnwebgpu_pkg`
   (`MVP_EMDAWNWEBGPU_DIR` overrides it).
3. **The site** lands in `build/engine/emscripten/web/site/`:

   | File | What |
   |---|---|
   | `index.html`, `style.css`, `controls.css` | the page (`engine/web/static/`) |
   | `main.js`, `worker.js`, `protocol.js`, `input.js`, `gamepad.js`, `gamepad-profiles.js`, `pad-manager.js`, `controls.js`, `controls-keys.js`, `controls-rebind.js`, `controls-store.js`, `audio.js`, `audio-worklet.js`, `resampler.js`, `disc.js`, `disc-store.js`, `disc-download.js`, `module-cache.js`, `packs.js`, `packs-core.js` | the shell, compiled from `engine/web/src/*.ts` by `tsc` |
   | `mvp-packs.js`, `mvp-packs.wasm` (0.6 MB) | the roster-pack checker for the page's panel (E6): `engine/mods` alone, loaded only when packs are picked |
   | `mvp-web.js`, `mvp-web.wasm` (0.6 MB) | the engine: translator, runtime, interpreter, renderer, mixer (`engine/web/wasm/`) |
   | `mvp-web-mt.js`, `mvp-web-mt.wasm` | the threaded engine (P2): the same sources for wasm threads, the renderer on its own thread (section 2); loaded where the page is cross-origin isolated |
   | `build-id.json` | the SHA-256 of `mvp-web.wasm` (`engine`), of `mvp-web-mt.wasm` (`engine_mt`) and of `symbols.txt` (the key of the browser's translation cache) |
   | `symbols.txt` | `config/GV4E69/symbols.txt`: repository metadata (function boundaries and names), not game data (ARCHITECTURE.md §4) |
   | `licenses/` | notices for the third-party code compiled into the engine (Emscripten, emdawnwebgpu) |

4. **Serve:** `node engine/web/tools/serve.mjs` (`--port N`, default 8000;
   `--root DIR`). It serves that directory only, on 127.0.0.1, GET and HEAD
   only, no listings, `Cache-Control: no-store`, and (P2) COOP `same-origin`,
   COEP `require-corp` and CORP `same-origin`, so the page is cross-origin
   isolated and runs the threaded engine; `--no-isolation` leaves them out (the
   single-threaded fallback). **`--disc PATH`** (local
   tests only) mocks the owner's deployment (E5 builds the real one in
   `engine/server/`): `GET /config.json` answers `{"disc": "/api/disc/image"}`,
   and `/api/disc/image` serves PATH to HEAD and to ranged GETs (206,
   `Cache-Control: private, no-store`); `--login` makes `/api/*` answer 401
   without the cookie `mvp_access=ok`; `--latency MS` and `--rate MB/S` slow it
   down. `--packs DIR` (with `--disc`) adds `"packs": "/api/packs"` to the config
   and offers DIR's pack zips there, as the deployment's pack store does.
   `--states DIR` serves save-state files for the page's `?state=NAME` from
   `/states/NAME.mvpstate` (local only: states are game-derived).
5. **Check in real browsers** (local only: it needs the disc):
   `node engine/web/tools/browser_check.mjs --browser chrome|msedge|chromium|firefox|webkit`
   with `--disc PATH` (default `MVP_DISC`), `--script FILE` (an `mvp-input`
   script), `--fields N` / `--frames N` (stop there and print the guest line),
   `--fast` (unpaced), `--shots F1,F2` (canvas screenshots at those VI fields),
   `--keys F:KEY,...` (live key presses; `--hold MS`), `--headed`, `--js-flags`,
   `--tag`; and (E2c) `--server-disc` (the mock deployment instead of the file
   input; `--login`, `--latency`, `--rate`), `--audio` (click first, then sample
   the worklet's output through the page's analyser) or `--no-audio`,
   `--audio-latency MS`, `--pad F:button:I,F:axis:I:V` (a fake standard gamepad
   behind `navigator.getGamepads()`), `--runs N` with `--clear-cache` (cold and
   warm starts in one profile) or `--no-cache`, `--hide F:MS` (the tab hidden for
   MS ms), `--trace-disc` (the picked image's read offsets and sizes),
   `--disc-cache PARAMS`, `--packs P1,P2` (E6: roster pack folders or zips through the
   page's panel, before the disc), and `--real-server MAIN_JS` (E5's server, built,
   instead of the mock: a throwaway access key, the check opens the private
   link, the disc configured; with the lobby there the check presses Play;
   `--disc-dir DIR` gives it the Docker deployment's disc directory instead of a
   file). P1 adds `--throttle RTT_MS,MBIT` (Chrome's network conditions through
   CDP, which also slow the page's Workers), `--io-log` (every guest DVD read with
   its host wait, summarized), `--clear-disc`, `--store-off` (read the server's
   image over the network as before P1), `--interrupt-download PCT`, `--fallback`,
   `--params Q`, `--profile DIR` (a profile on disk) and `--site DIR`; P1-gfx adds
   `--fast-until F` (unpaced to field F, then paced), `--trace-present` and
   `--screencast F:N`; P2 adds `--threads 0` (the single-threaded engine on an
   isolated page), `--no-isolation`, `--digest` (the renderer digest of every field,
   written to `digest-BROWSER-rN.txt`) and `--cast-size WxH` (small JPEG screencast
   frames that keep up at 60 fps); the summary says whether the page was
   cross-origin isolated and whether the render thread drew; Q2 adds `--host-slow N[@F]`
   (a machine N times slower, simulated from field F: the engine's threads spin N - 1
   times their working time and DevTools throttles the main thread, `hostSlow=N`,
   `hostSlowFrom=F`), `--affinity HEXMASK` (the browser on chosen logical processors)
   and `--load N` (busy processes), and the worklet's gap, stretch and target
   statistics in every window (Q5: also the GPU's time, the scale and the frames
   shown, and the summary's `hint`, `autoScale` and picture log); Q7 adds
   `--actions F:KIND[:ARG],...` (host-side changes once the run reaches field F:
   `resize:WxH`, `picture:auto|1|2|3`, `fullscreen` on and off, `stats`) and
   `--compare PNG` with `--compare-region x0,y0,x1,y1` (console pixels, default
   the top-left 128 x 128 block) and `--compare-max N` (the final picture against
   another run's, failing above N; the scale-change check below);
   `netplay_check.mjs` (Q5) takes `--params1/2`, `--host-slow1/2 N[@F]` (one
   player's machine slower), `--audio`, `--trace-present`, `--fast-until F`,
   `--measure-from/to`, `--separate` (two browser instances, each with its own GPU
   process), `--affinity HEXMASK` and `--delay auto`, and its summary's `measured`
   gives each page's game speed, emulation ms per field, new frames per second, the
   new-frame interval p95 and share over 25 ms, waits per second and the share of
   time waiting, underruns per minute and the pace (Q5b: each stats window also
   carries the schedule time the paced loop gave up, by reason, and the time due
   fields waited for the loop itself, and the pace state its role; the page
   parameter `paceTune=key:value,...` sets the pacer's options, e.g. `roles:0`
   for Q5's common pace, and the loop's: `debt`, `gateDebt`, `spin`, `fixed`,
   and `chase:1` for the pacing before Q5 online); `netplay_check.mjs
   --server-disc` runs E5's two-page check the deployment's way. It starts the server, opens the page with Playwright,
   and writes `summary-BROWSER[-TAG].json` and the screenshots to `--out`
   (default `build/engine/web-check`; screenshots are game-derived and stay under
   `build/`). It lists every request the page made. Chrome and Edge run with
   `--mute-audio` and Firefox with its volume at 0 unless `--audible`.
   **Controllers** (Q3): `node engine/web/tools/pad_check.mjs --browser chrome|msedge|firefox`
   drives the Controls panel with fake gamepads (`tools/fake_pads.mjs`: standard and
   non-standard pads, hot-plug, a made-up GameCube adapter that needs the guided setup,
   two players, the lobby line with a scripted lobby server) and needs no game data;
   `node engine/web/tools/pad_game_check.mjs --disc PATH [--net]` plays the real game's intro
   and menus on fake pads, has the engine report the inputs the guest consumed (`inputLog=1`),
   replays that log with no gamepad and requires the same guest line, and with `--net` runs two
   pages through E5's server with player 2 on a pad (`docs/engine/Q3_LOG.md` section 4).
   **The scale-change check** (Q7; run it when the renderer's EFB copies, caches or scale
   handling change): CLK2's `game_125` script (the exhibition, then CPU vs CPU) unpaced to
   field 8,250, first at a fixed scale (`--fast --fields 8250 --params scale=3 --tag ref`),
   then from Auto with the Picture changed in the at-bat view and the ball in play by the
   stop (`--fast --fields 8250 --actions 7000:picture:3 --compare OUT/chrome-ref-r1-final.png`).
   The game saves and restores the EFB's top-left 128 x 128 in every frame of play, so a
   copy texture left over from the old scale shows there as a frozen block: the check
   fails above a mean difference of 3 (Q7_LOG.md section 5 has the numbers).
6. **Unit tests** (data-free, in `ctest --preset emscripten`): `test_web_shell`
   runs `node --test` on `engine/web/tests/*.test.mjs` against the compiled
   shell (the gamepad mapping and the Controls layers, the audio resampler, the
   disc's server contract, the one-time download's manifest check); `test_web_cache` tests the server image's block cache and the
   translation cache's blob. `mvp-web-cache-sim TRACE` (development) replays a
   `--trace-disc` trace through the block cache over a modelled network.

## 2. Architecture

```
 page (main thread)                          engine Worker (module)
 ------------------                          ----------------------
 /config.json -> {"disc", "discHash", ...}: the image, downloaded once into OPFS (disc-download Worker),
   or a picked File  ----------------------> Module.mvpDisc (FileReaderSync: local reads)
   (no OPFS, or store=0) -------------------> Module.mvpDiscUrl (Range XHRs, block cache)
 <canvas> -- transferControlToOffscreen ---> specialHTMLTargets['#mvp-canvas']
 keyboard, Gamepad API -- pad state -------> mvp_set_pad (latched until the next frame poll)
 requestAnimationFrame -- tick (fallback) --> paced loop: mvp_run_fields(n), mvp_present()
 status, progress, log, errors <-- messages - phases, stats, first frame, guest line
 AudioContext (after the first gesture)       OPFS: the translated module (DOL SHA-1 + build id)
   AudioWorkletNode <==== ring in shared memory (Q2; isolated pages) ==== the mixer, as it mixes
                    <==== MessageChannel (no isolation) ==== each batch's frames (Int16Array, transferred)
     drift resampler 32 -> 48 kHz, time-stretch when behind (Q2)
     -> gain -> speakers; -> AnalyserNode (level probe)
```

- **The page** (`src/main.ts`) holds no engine state. On load it asks for
  `/config.json` (`src/disc.ts`): with a `disc` URL the server's image is used
  and no picker is shown (P1: downloaded once into this browser and read from
  there, below); a deployment whose operator has not uploaded the disc yet shows
  that the operator must upload it (no picker, no upload in the main flow; the
  Server disc page is a small "site operator" link at the bottom); anywhere else
  it shows the picker and hands the Worker the picked `File`. With the server's image and E5's lobby on the page (the
  deployment: `docs/engine/NETPLAY.md`), a Play button starts a game alone, and
  creating or joining an online game starts the engine as soon as the page has
  its player number (the Worker then waits for the other player's handshake);
  without the lobby the game starts at once. It also gives the Worker the canvas as an `OffscreenCanvas` and the
  controller state, shows the status line and a progress bar while loading
  (posted by the Worker, also from inside the translation), the phases, the
  frame statistics and errors, and publishes them as `window.mvp` for the
  browser check. URL parameters for tests: `script`, `fields`, `frames`,
  `fast=1`, `render=0`, `audio=0`, `cache=0`, `audioLatency=MS`, `traceDisc=1`,
  `discBlock`/`discBlocks`/`discRun`/`discAhead`, and (P1) `ioLog=1`, `store=0`,
  `storeSpare=MB`.
- **The look** (owner, 2026-10-17: "the same theme as MVP2005 baseball ... the gray pallet"). The page
  copies the game's own front end, with colors sampled from the native engine's capture of the main
  menu (the values are listed at the top of `static/style.css`):
  - the game on the left in a silver frame, with a gunmetal bar under it. The bar's buttons look like
    the game's button prompts: a round key (F, M, P, `) then small capitals;
  - a menu column on the right: the Game panel (status, progress, Play), Play online, Roster packs,
    then Gameplay mods, Controls and Engine details folded into one frame like the Main Menu. A
    closed item is a gray bar; an open one is the chosen item's maroon with gold lettering and a
    baseball bullet;
  - panel headings are silver bars with a slanted blue tab, and the title is a red script outlined in
    white on a silver swoosh.
  The screen keeps to the window's height, and below 1020 px the column moves under the game. Fonts
  are the machine's own: Bahnschrift, and Brush Script MT or Segoe Script for the title. The page
  loads no fonts or images. The server's own pages (the Server disc page and the "private server"
  page) use the same theme from `engine/server/src/theme.ts`.
- **The Worker** (`src/worker.ts`) loads `mvp-web.js` (an Emscripten ES module,
  `-sENVIRONMENT=worker -sMODULARIZE -sEXPORT_ES6`) and calls its exports in
  order:
  1. `mvp_gfx_init`: the WebGPU adapter and device are requested first
     (asynchronous, `Gpu::create_async`), so they arrive while the DOL is
     translated;
  2. the disc: `mvp_open_disc` for a picked file (a `vfs::Source` whose reads
     call `FileReaderSync.readAsArrayBuffer(file.slice(...))`), or
     `mvp_open_disc_url` for the server's image (below). Either way CISO or ISO,
     then the **identity check**: `vfs::open_disc(..., require_retail=true)`: game
     id GV4E69, revision 0, and the SHA-1 of `main.dol` (`da6becdb...`). Anything
     else stops here with "This image cannot be used. The engine runs MVP Baseball
     2005 for GameCube, USA (GV4E69), revision 0 only." and the reason (for
     example `main.dol SHA-1 056ef58c... is not GV4E69 rev 0`, or `GCM: bad disc
     magic at 0x1C`);
  2b. (E6) the roster packs the page handed over, if any: checked, applied to the
     tables of this disc (`engine/mods`, `wasm/packs_web.cpp`) and mounted on the
     virtual disc (`vfs/overlay.h`); an error stops here with the reason (below);
  3. the translation: from this browser's cache if it has one for this DOL and
     engine build (`mvp_translate_cached`, below), else `mvp_translate`:
     `symbols.txt` (fetched from the site) -> CFG -> one WebAssembly module
     (`translator/wasm_emit`) -> synchronous `new WebAssembly.Module` (allowed for
     any size in a Worker) -> instance in the runtime's own function table
     (`runtime/wasm_host`). The CFG is freed afterwards; the module is kept only
     until it is cached;
  4. `mvp_boot`: the machine, the IPL environment, the hardware model, the input
     queue (a script for tests, else live input), `YieldingHooks`;
  5. waits for the renderer (the GX processor must see the command stream from
     the first command), then runs.
- **The run loop.** `rt::run()` returns at every VI field (a *host yield*,
  `runtime/host_yield.h`): the VI field observer asks for a yield, the end of
  that safe point unwinds every native frame the way a thread switch does, and
  the next `run()` continues at the guest PC, re-entering functions at their
  resume labels. The safe point the yield interrupted is not repeated (the first
  safe point after a yield at the same PC and instruction count returns at once),
  so the guest executes exactly the same instructions with and without yields.
  `mvp-play --yield` checks this natively, `local_play_determinism` runs it, and
  the browser's guest lines below match native ones.
- **Pacing.** Each animation frame (the Worker's own `requestAnimationFrame` where
  it exists, else a tick message from the page's) runs as many whole VI fields as
  the wall clock asks for at 59.94 fields/s, at most 4 per frame; if the game falls
  more than 8 fields behind (the intro movie, a slow stretch) the debt is dropped
  instead of racing. Then `mvp_present()` draws the field's frame. (Q2) In the
  threaded engine, while the host time per field (smoothed over about a second) is a
  whole field or more, every batch is one field with its present, back to back, and
  the clock runs at most two fields ahead: on a machine that cannot keep up the game
  runs evenly in slow motion with every frame shown (the sound stretches to match)
  instead of in two-field jumps. (Q5) In an online game the schedule runs at the
  two players' common pace (`src/pacer.ts`; NETPLAY.md section 1, "Pacing between
  unequal machines"): 1 when both machines keep up, else the slower machine's
  sustainable pace, one field per slot on both sides (each started at its slot's
  time by a timer, not at the next animation frame), a wait for the other player
  moving the schedule on instead of being caught up, and the schedule nudged
  toward equal input leads so neither side waits; (Q5b) once one machine is
  clearly slower, it runs each field as soon as the last is done and the other
  follows it (NETPLAY.md, "Roles"). A batch that ran no field (an
  online game waiting at a frame poll) ends without presenting again. `fast=1` runs
  fields back to back for 40 ms per task instead (measurements). **Wall time
  decides only how many fields run, never what happens inside one**: guest state
  depends on the disc and the per-frame pad inputs alone.
- **Pause.** A hidden tab, the Pause button or P stops the Worker's loop and
  suspends the AudioContext; on return the pacing clock restarts from the field
  it stopped at. Only the host loop stops: the guest sees no wall time, so a run
  with a pause gives the same guest line as one without.
- **Input.** The keyboard and every connected controller are read on the main
  thread (the Gamepad API is not available in Workers), at every animation frame
  and whenever a key changes (`src/input.ts`, `src/pad-manager.ts`), into one
  GameCube pad per port: players 1 and 2 offline; online only this page's player,
  as the page's port 0, which the engine puts on the player's own port
  (NETPLAY.md section 1). A port's pad is the keyboard (if the keyboard is set to
  drive that port) merged with the controller assigned to it. The pads go to the
  Worker when they change; the engine applies them at the game's next frame poll
  (E0c's input queue); a button pressed since the previous poll counts as pressed
  at that poll even if already released, because the game polls once per frame of
  its own, which can be several fields apart. Per-frame inputs are the only host
  input that reaches the guest: with `inputLog=1` the engine reports exactly what
  the guest consumed as an `mvp-input` script, and replaying that script with no
  gamepad gives the same guest line (`tools/pad_game_check.mjs`).
- **Controllers and the Controls panel** (Q3; layers in `src/gamepad.ts`,
  `gamepad-profiles.ts`, `controls-store.ts`, `pad-manager.ts`, `controls-keys.ts`,
  `controls-rebind.ts`; the panel is `src/controls.ts`). The default mapping is for
  the Gamepad API's standard layout (Xbox, PlayStation, most pads in Chrome and Edge):
  bottom face button = A, right = B, left = X, top = Y, either bumper = Z,
  triggers = L/R (analog 0-255, the digital L/R click at 90 % of the travel, a
  trigger's `value` counts, not the browser's early `pressed`), Start = START,
  D-pad, left stick = main stick, right stick = C-stick. Sticks: a radial dead zone
  (15 %, adjustable) with the rest scaled to +-100, and the GameCube's octagonal
  gate (the four directions and the four diagonals reach the full distance, 92 %
  between them; a switch turns it off). Nintendo-named pads (vendor 057e, standard
  layout) get A/B and X/Y swapped to their labels; an Xbox pad without the standard
  mapping gets the Linux joystick order; any other pad starts from a guess (A B X Y
  on buttons 0-3, a D-pad on buttons 12-15 or on a hat axis) and the panel offers the
  guided setup, which asks for each of the sixteen controls in turn (press the button,
  or move the stick or trigger; a D-pad reported as one "hat" axis, triggers on axes
  resting at -1 and the analog axis of a GameCube pad's trigger are recognised).
  GameCube adapters are recognised by name and vendor (Mayflash 0079:1843 and 1844,
  Nintendo 057e:0337) and always offered the guided setup: their raw layouts differ
  between firmware, mode, OS and browser. **Which pad plays which port:** a pad that
  has been *used* (a button pressed or a stick moved: browsers list idle pads, and a
  GameCube adapter lists its four sockets whether or not a controller is in them)
  takes the next free port set to Auto, and keeps it while it stays connected; each
  port can instead be set to a model of pad (the n-th one of that model) or to none;
  the keyboard drives player 1 unless set to player 2 or nobody. Online, the panel
  shows one row, "You (player N)", and the pad of whichever port has one plays.
  Pads can be plugged and unplugged at any time (Chrome shows a pad only after its
  first press and only to the page that has the focus: the panel says so). **The
  panel** (under "Controls", and a "Controller: ..." chip in the toolbar that opens
  it, which also opens by itself for a pad whose layout it does not know): the pads
  found with their layout and port; who plays; a live picture and numbers of what
  the game will get from the pad or keyboard being tested; the binding of every
  control with a Set button ("press the button for A": the press never reaches the
  game, Esc cancels) and Set up this controller (all sixteen in turn, Skip, Back,
  Cancel); the dead zone, the octagon and the trigger click; the keyboard's keys the
  same way (a key another control uses swaps with it; F, M, P, Esc and the
  browser's keys are refused); a vibration test (`vibrationActuator`; the game's own
  rumble calls are not exposed yet, `Q3_LOG.md` section 7). Everything is stored per
  browser in `localStorage["mvp.controls.v2"]`, per controller *model* (vendor and
  product id, else its name), checked field by field when read; the E2c mapping
  (`mvp.gamepad.mapping.v1`) is read once for pads without their own. "Press A to
  play" starts the game from the deployment's Play button.
- **Sound** (`src/audio.ts`, `src/audio-worklet.ts`, `src/resampler.ts`, engine
  `wasm/web_audio.h`). The AX mixer of E3b observes the DSP's frames in the
  Worker exactly as natively (read-only: E3b-MIX1). Browsers start audio only
  after a user gesture, so the page creates the AudioContext on the first
  pointer press, key press or touch (a hint says so; gamepad buttons do not
  count as a gesture), adds the worklet and gives one end of a MessageChannel to
  the worklet and the other to the Worker; the Worker then attaches the mixer.
  **The frames' way (Q2):** on a cross-origin isolated page they go through a ring
  in shared memory (`wasm/web_audio.h` `AudioRing`), which the worklet drains at
  every render quantum: the threaded engine's mixer writes into a ring in the
  engine's own (shared) memory as it mixes, inside the run; the single-threaded
  engine's Worker copies each run batch into a SharedArrayBuffer ring of its own.
  Without isolation (or `audioRing=0`) the Worker posts each batch's frames (16-bit
  stereo at the AI's rate, 32 kHz; the buffer transferred) through the channel, as
  E2c did. The worklet's drift resampler is the native one's design
  (`audio/drift.h`): it reads at 32000 / the context's rate (usually 48 kHz) times a
  ratio within +-0.5 %, steered by its smoothed fill toward a 60 ms target, with
  cubic (Catmull-Rom) interpolation. **When the emulation runs behind (Q2)** the
  buffer would empty over and over (sound on, off, on, off): once it falls well
  under the target the input is time-stretched with its pitch kept (contiguous
  stretches, and a jump back to the best-matching point near the speed's ideal
  track, cross-faded over 4 ms), at a speed a PI controller on the fill sets (down
  to a quarter speed), until the input keeps up. Dry (a stall), it fades out, plays
  again once half the target is back and raises its target 20 ms (up to 200 ms; 10
  ms back per quiet 30 s). More than 4 x the target, it skips down to it; full, it
  drops. An AnalyserNode taps its output (`window.mvp.audioProbe()`); M or the Sound
  button mutes. Nothing goes back from the worklet to the Worker (Q2_LOG.md).
- **The render thread** (P2: `wasm/render_thread.h`, `wasm/render_side.h`,
  `gfx/render_ring.h`, `gfx/gp_reads.h`; P2_LOG.md). Where the page is
  cross-origin isolated (the server's and `serve.mjs`'s headers) and `threads=0`
  is not given, the Worker loads `mvp-web-mt.js` instead: the same engine with a
  shared memory and one pthread that owns the WebGPU device and runs the GX
  processor and the renderer. The emulation copies every GX command, VI field and
  present into a 16 MiB ring in shared memory and goes on; a guest write to
  memory the render thread has still to read for a command already issued (the
  vertex arrays, palettes, textures and other sources `gfx::GpReads` finds in the
  commands, tracked per 64 bytes) waits until the render thread has read it, so
  guest state and everything drawn are the same as inline. The render thread
  shows at most one frame per display refresh (the page counts its animation
  frames in shared memory) and posts each frame to the page as an ImageBitmap,
  which the page's `bitmaprenderer` canvas shows. Without isolation, or with
  `threads=0`, `mvp-web.js` renders inline in the Worker as before. URL
  parameters: `threads=0`, `bitmaps=0` (draw into the page's transferred canvas
  instead), `digest=1` / `digestDetail=F1-F2` (the renderer digest), `rtTune=...`.
- **The disc from the server, kept in the browser** (P1: `src/disc-store.ts`,
  `src/disc-download.ts`; the server's `src/disc_hash.ts`). The server advertises
  its image in `/config.json` with `discHash` (the image hash: SHA-256 over the
  SHA-256 of each 8 MiB piece), `discSize` and `discManifest` (the piece list,
  `/api/disc/manifest?v=`); it hashes the image once after an upload or at the first
  start (0.6 s for the 1.38 GB CISO on this PC with the file cached; a small VPS reads the disk first) and keeps the result next to it
  (`GV4E69.manifest.json`); meanwhile `/config.json` says `discPreparing` and the
  page asks again every 2 s. The first visit downloads the whole image into the
  origin private file system: a dedicated Worker (only Workers get synchronous
  OPFS handles, which write each piece in place) checks the manifest against the
  advertised hash, checks `navigator.storage.estimate()` and sizes the file to
  the whole image before fetching anything (a small quota or a full disk fails
  at once, with a message), then fetches 4 pieces at a time by Range GETs, checks
  each piece's SHA-256 before writing it, and flushes and records the written
  pieces about once a second (`mvp-disc/<hash>.part.json`), so a reload continues
  where it stopped; the last write is `<hash>.ok`. The page shows "Downloading
  the game data from the server, once: 34% (470 of 1,316 MB, 11.8 MB/s, about
  72 s left)". A Web Lock keeps a second tab waiting instead of downloading
  twice; `navigator.storage.persist()` asks the browser to keep the copy. The
  engine then gets the stored file as a `File` and reads it exactly like a picked
  file (FileReaderSync): every guest DVD read is local and synchronous. Later
  visits find `<hash>.ok` and start at once; a different image hash (the operator
  uploaded another image) downloads again and removes the old copy first. The
  identity check (GV4E69 rev 0, the DOL's SHA-1) still runs on every start. The
  lobby (E5) opens only once the copy is complete. On a download error the page
  says what to do (free space and a normal window, open the player link again,
  reload to resume) and offers to play with the image read from the server as before.
- **The disc from the server, read over the network** (the fallback, and servers
  without `discHash`; `src/disc.ts`, engine `wasm/web_disc.*`,
  `wasm/block_cache.*`). The deployment's contract (E5): `GET /config.json`
  `{"disc": "<url>"}`; the URL is on the same origin (anything else is refused)
  and answers HEAD with the size and a ranged GET with 206, both with
  `Cache-Control: private, no-store`; API calls from a browser without access
  answer 401 (the page then says to open the player link again). The Worker checks
  the image with a HEAD and a
  one-byte ranged GET (a server ignoring Range would send 1.4 GB per read), then
  the engine reads it by synchronous Range XHRs (a guest DVD read completes
  inside the call; the browser sends the access cookie with them) through a block
  cache: 256 KiB blocks, 64 MiB, least recently used out; a miss fetches all the
  blocks its read needs (up to 32) or, on sequential misses, a run growing to 4;
  a sequential stream also asks for the next 8 blocks ahead, which the Worker
  fetches asynchronously between batches. Failed reads retry twice; a read that
  still fails stops the run with the reason. Nothing is kept after the page
  closes. Over the internet this stalls the game (P1, §3).
- **The drive's reads, timed** (P1, `wasm/mvp_web.cpp` `TimedSource`): every
  guest DVD read the drive model makes is timed on the host clock; `mvp_status`
  reports `io` (reads, host ms, the longest, and how many took longer than a
  field, 50 ms and 100 ms) and the page shows it in the stats line; `ioLog=1`
  keeps a per-read log (field, time, offset, size, wait).
- **Roster packs** (E6; `src/packs.ts`, `src/packs-core.ts`, engine `wasm/packs_web.*`;
  format: `docs/MODDING.md`). The Roster packs panel, above the disc picker, takes pack
  folders (a directory picker) and `.zip` files, in the order the player puts them (Up,
  Down, Remove). Each change is checked at once by `mvp-packs.js`, the engine's pack
  code without the rest of the engine: every pack's name, id, version and hash, the set
  hash, and every problem with its MODDING.md code and place. The packs must be picked
  before the disc (picking the disc starts the game). At the start the panel locks and
  hands the files to the Worker, which applies them to the player's disc before the
  translation (`packs` phase: the example pack takes about 280 ms in Chrome) and reports
  the result (result hash, patched files) back to the panel; errors that need the disc
  (an unknown player, a broken roster) stop the start with "The roster packs cannot be
  used with this disc ..." and the first problems. The set hash goes into the netplay
  handshake (`NETPLAY.md` §3). The packs never leave the browser; the patched tables
  live only in the engine's memory.
  **Server packs** (the deployment): when `/config.json` names `"packs"` (the server's
  `MVP_PACKS_DIR`, `engine/server/src/pack_store.ts`), the panel adds a **Packs on this
  server** list from `GET /api/packs`. Choosing one downloads its zip from
  `/api/packs/<file>` (an ETag lets the browser revalidate instead of
  downloading again) and adds it exactly as a picked zip, so the checks, the order and the
  netplay set hash are the same. The page remembers the server packs a player chose (in
  this browser's `localStorage`) and adds them again on the next visit if the server
  still has them. The owner uploads and removes them on the Server disc page
  (`DEPLOY_DOKPLOY.md` §8). `browser_check.mjs --real-server ... --packs-dir DIR
  --server-packs FILE` chooses them from the list as a player does.
- **Save states** (2026-10-19; `src/states.ts`, engine `mvp_state_*`, `runtime/state_file.h`).
  F2 saves the game where it is and F4 goes back to the last state saved or
  loaded (also the toolbar's Save state / Load state, and the Save states panel:
  load a `.mvpstate` file, download the saved one). The engine saves and loads at
  the next frame poll in live play (the poll gate, as a review does), offline only,
  in either engine (2026-10-20: the threaded one through its render thread, whose
  GX state is exported and imported by ring records, `render_view_export` /
  `render_view_import`; a load first does what a guest write over all of memory
  does, `render_memory_replaced`); a load also restores the renderer's GX state,
  starts a fresh mixer and restarts the mods' timeline (its snapshots belonged to
  the run before). The last state is kept in this browser (IndexedDB
  `mvp-states`), so F4 works after a reload once the same game runs.
  `?state=NAME` starts from `/states/NAME.mvpstate` on the same site: the page
  reads its header first, takes its gameplay mods (unless `mod=` is given: the
  same mods, other settings allowed) and its roster packs from the server's list,
  and the Worker loads it at the first frame poll, after checking that the mounted
  packs' result hash is the state's. A file is about 50 MB (MEM1, ARAM, the VM
  region). **The admin's states on the server** (2026-10-20; owner request,
  DECISIONS): when `/config.json` says `admin` with `states`, the panel's **On the
  server** part lists them (Load into the running game; Start from it = `?state=`),
  every F2 also copies the save to the admin's slot there (`quick`, gzipped with
  `CompressionStream` before the POST), F4 in a browser without a save of its own
  takes that slot, and **Save to the server** keeps one under a name. The server
  stores them gzipped and serves them with `Content-Encoding: gzip`
  (`engine/server/src/state_store.ts`); only the admin can list, read or write them.
  `browser_check.mjs --real-server ... --admin --server-states DIR` tests this.
- **The memory card** (2026-10-20; `src/card.ts`, engine `mvp_card_*`,
  `runtime/memcard.h`). Slot A holds a 1019-block card, the EXI flash card the
  SDK's CARD library drives (not HLE: the game's own SDK code runs on it), so the
  game saves profiles, seasons and settings from its own menus. The page gives
  the engine this browser's card (IndexedDB `mvp-card`) before boot, or none for
  a new card the engine formats as the game would. The Worker sends the card back
  (`'card'`) once the game's writes have settled for 60 fields, and at the end of
  a run. The Memory card panel downloads it and takes a card file back before the
  game starts. No card goes in for an online game, or for a test run (`script=`,
  `fields=`, `frames=`) unless `card=1`; `card=0` takes it out. **The admin's card
  on the server** (`/config.json` says `card`; `engine/server/src/card_store.ts`):
  the admin's browser starts from the server's card (or its own, when it has saves
  the server lacks on the same version) and PUTs each settled card, gzipped, with
  If-Match on the version it came from. When another browser has saved since, the
  server answers 412 and this browser keeps its card to itself and says so. Save
  states hold the card's transfer state, not its contents (state file v2). The
  browser summary's `card` (inserted, writes, sent) shows it.
- **The translation cache** (`src/module-cache.ts`, engine `wasm/module_cache.*`).
  After the first frame the Worker serializes the translator's output (the wasm
  binary and the tables the runtime fills from it, about 52 MB) and writes it to
  the origin private file system (`mvp-translated/<key>.bin`, then a `.ok`
  marker), keyed by the DOL's SHA-1 and `build-id.json`, removing older entries.
  The next visit reads it straight into the engine's heap and loads it with
  `mvp_translate_cached` instead of fetching `symbols.txt` and translating; a
  missing, incomplete or unloadable entry means translating as before. The entry
  is derived from the disc and stays in the player's browser (ARCHITECTURE.md §3).
- **Renderer.** The E1 renderer, unchanged, on emdawnwebgpu (Emscripten's
  `webgpu.h` from the same Dawn release as the native build). Only the edges
  differ: `Gpu::create_async` (no blocking `WaitAny` in the browser), `Window`
  on a canvas found by selector instead of an SDL window, no explicit `Present`
  (the browser shows the canvas when the task returns), and read-backs are
  native only. One renderer fix applied to both: the EFB pass's depth clear value
  is set (browsers reject webgpu.h's NaN default even when the load op is Load).
  Fullscreen (the button, F, or a double click) shows the canvas at 4:3 on black.
  Q1 (`docs/engine/Q1_LOG.md`): the renderer reads guest memory as the GP sees
  it after the CPU's data cache (`gfx/gp_memory.h`: lines the game flushes are
  snapshot until the next VI field; the threaded engine sends the changed lines
  through the ring), which ended the front end's flashing; it draws at 1x, 2x
  or 3x the console's pixels (the page's **Picture** setting: Auto, 1x, 2x,
  3x, kept in `localStorage["mvp.picture.v1"]`; Auto draws at least as many
  pixels as the canvas shows, up to 3x, and follows fullscreen; a software
  adapter stays at 1x), shows the 640 x 448 frame at 4:3 (the game's 3D
  cameras are 4:3), scales it to the canvas with an area filter (larger) or
  sharp bilinear (smaller), and samples textures with the game's LOD bias.
  Above 1x, 2D vertices go to whole console pixels (Q8, `docs/engine/Q8_LOG.md`),
  so menus and the HUD cover the pixels they cover at 1x. (Q5)
  Auto also respects the GPU: the renderer times its render passes on the GPU
  with timestamp queries where the browser has them (`gfx/wgpu/gpu_timer.h`;
  Chrome does, rounded to 100 us), and while the GPU is busy more than 85 % of
  the time for two seconds Auto steps one scale down (`src/autoscale.ts`); it
  steps up again only after a hold (20 s, doubling when a step up had to be
  undone within a minute) and while the next scale should fit (the busy share
  times the pixel ratio under 60 %) for five seconds. A fixed setting is never
  changed. URL parameters: `scale=1|2|3` (instead of the setting), `gpview=0` (no
  data-cache model, for comparisons), `gpuLoad=N` (development: N extra render
  passes per present over a texture of the picture's internal size, 25 samples
  a pixel, as a GPU that cannot keep up). (Q7) A change of scale replaces every
  EFB-copy texture; the renderer's texture bind-group cache, keyed by handle
  addresses, now holds references to its views and drops a replaced copy's
  entries: in the browser a released view's address is reused at once, and before
  Q7 the game's top-left 128 x 128 block (saved and restored by the game in every
  frame of play), its shadows and the video board stayed frozen after any mid-game
  change of scale (`docs/engine/Q7_LOG.md`).
- **The statistics overlay** (Q5; `src/netstats.ts`). The **Stats** button in the
  toolbar or the backquote key (left of 1; refused as a game binding) shows,
  over the picture, live numbers for this computer and, online, for the other
  player's: game speed, emulation ms per field and the speed the machine could
  run at, frames shown per second, the share of time waiting for the other player
  (and waits per second), sound underruns per minute, the picture's scale and the
  GPU's ms per frame; online also the round trip and its jitter, the input delay,
  and which computer sets the pace. Off by default; the choice is kept per
  browser (`localStorage["mvp.overlay.v1"]`; `stats=1` in the URL shows it).
  **Copy stats** puts the same as text on the clipboard, with both machines'
  threads, GPU and browser, for the owner to paste into a chat. Online each page
  sends its numbers once a second as a text message on the data channel; the
  engine never sees them. A dismissible note under the toolbar appears when this
  computer runs the game below 0.93x for a while (solo), or sets an online pace
  below it: "This computer is running the game at 0.85x speed. On a laptop,
  plugging in and choosing the Best performance power mode helps."
- **Errors** say what to do: no WebGPU (which browsers have it), a GPU that
  cannot be used, the wrong disc (above), no access to the server, a server that does not
  answer Range requests, a failed engine Worker.
- **What the server sends:** the files of §1 (the page, the shell, the engine,
  `build-id.json`, `symbols.txt`; `symbols.txt` not on a warm start), and on the
  owner's deployment `/config.json`, the manifest and the image (on the first visit
  only, P1). The browser check records
  every request; no run made a request outside the local origin.

## 3. Measurements

Machine: Intel Core i7-14700K, 64 GB, NVIDIA GeForce RTX 4070 SUPER, Windows 11.
Other agents shared the machine. Disc: the owner's CISO (1.38 GB). Chrome and
Edge: the installed browsers, headless (Playwright `channel`). Firefox: Playwright's
build, **headed** (headless Firefox gets no WebGPU adapter). WebKit: Playwright's
Windows build, which has no `navigator.gpu` at all, so it ran without rendering.

**Startup, E2** (2026-10-14; ms, the page's clock; a picked file, no cache). Runs:
the `menus` input script (START at frames 280, 430, 580, 730; DOWN at 1000; A at
1060: intro, licence screen, title, memory card dialog, main menu with the
profile dialog), 3,600 VI fields, paced (WebKit: unpaced).

| Phase | Chrome 155 | Edge 155 | Firefox 157 | WebKit 27.2 |
|---|---:|---:|---:|---:|
| engine module (fetch, compile, instantiate 0.6 MB) | 6 | 8 | 8 | 10 |
| disc check (CISO index, FST, DOL + SHA-1) | 27-30 | 32 | 25 | 36 |
| `symbols.txt` fetch (3.9 MB, localhost) | 14-17 | 15 | 13 | 14 |
| parse symbols | 40-52 | 42 | 40 | 46 |
| CFG | 145-158 | 159 | 139 | 157 |
| emit (49.5 MB module) | 274-293 | 284 | 323 | 273 |
| compile (`new WebAssembly.Module`) | 16-21 | 17 | 149-157 | 42-45 |
| instantiate | 1 | 1 | 0-1 | 2 |
| boot | 4 | 4 | 4 | 5 |
| WebGPU adapter + device (overlaps translation) | 571-599 | 658 | 715 | none |
| **cold start: pick -> first frame on the canvas** | **668-792** | **1,085** | **823-860** | (game code runs 0.6 s after the pick) |

**Cold and warm start, E2c** (start -> first frame, ms; 4 runs in one profile, the
cache cleared before the first; "server": the mock deployment on localhost):

| | Chrome, picker | Chrome, server | Firefox, picker | Firefox, server |
|---|---:|---:|---:|---:|
| cold (translated) | 698 | 740 | 1,120 | 1,026 |
| warm (from the cache) | 280-330 | 344-363 | 399-439 | 380-418 |
| translation phase, cold / warm | 491 / 76-114 | 514 / 103-116 | 641 / 174-182 | 658 / 172-189 |

Warm, the translation phase is reading the 52 MB entry, unpacking it and compiling
(Firefox's compile alone is 122-133 ms); the first frame then waits for the WebGPU
device. Writing the entry after the first frame takes 110-410 ms. A warm start also
leaves the engine's memory at 146-155 MB instead of 213-220 MB (no translator peak).

**Frame time** (host milliseconds per VI field inside `rt::run` and the renderer,
averaged over half-second windows; achieved fields per second in brackets; E2):

| Scene (VI fields) | Chrome | Edge | Firefox | WebKit (no rendering, unpaced) |
|---|---:|---:|---:|---:|
| boot loading screen (0-580) | 5.8 (59.9/s) | 5.4 (58.1/s) | 3.4 (59.8/s) | 3.5 |
| intro movie (600-1270) | 79 (12/s) | 79 (12/s) | 54 (18/s) | 96 |
| licence screen (1300-1660) | 0.6 (60/s) | 0.4 (60/s) | 0.3 (60/s) | |
| title, "PRESS START" (1940-2150) | 1.4 (60/s) | 1.2 (60/s) | 1.2 (60/s) | |
| memory card dialog (2180-2500) | 3.9 (60/s) | 3.6 (60/s) | 2.8 (60/s) | 2.9 |
| main menu + profile dialog (2730-3600) | 12.9 (60/s) | 12.9 (60/s) | 8.1 (60/s) | 11.3 |

- Everything but the intro movie runs at full speed (59.94 fields/s) in every
  browser with WebGPU. The movie is decoded by game code on the CPU, mostly FP and
  paired-single work that still goes through the interpreter (`rt.exec`, E2_SPIKE
  §6 item 3); it runs at 20-30% of real time, as natively (`/Od`: 18%).
  *Since E2d the movie runs at full speed too: next table.*
- The main menu costs 12.9 ms per field in Chrome, 11.6 ms of it in game code
  (the same run with `render=0`): the renderer adds about 1.3 ms there. Chrome
  with V8's tier-up off (`--js-flags --no-wasm-tier-up`, Liftoff only) gives the
  same numbers, so TurboFan is not the difference to Firefox, whose code is
  30-35% faster on the movie and the menu.
- Unpaced (Chrome): the loading screen runs at 3.4 ms per field, about 5x real time.
- The engine's WebAssembly memory is 213 MB once running (`heap_mb` in the
  status, Chrome). Memory never shrinks, so this is the translator's peak (the CFG
  and the 49.5 MB module, freed after instantiation and reused by the heap) plus
  the machine (24 MB MEM1, 16 MB ARAM). The compiled code lives outside it, in
  the browser's own memory.

**Frame time, E2d** (2026-10-14; `docs/engine/E2D_LOG.md`): the wasm emitter now
computes floating point, paired singles, quantized loads and stores and
`lmw`/`stmw` inline. The same `menus` run with E2c's client and sound on
(`browser_check.mjs --audio --no-cache`), against `origin/main` before E2d
(`45de368e`) on the same machine the same evening. Host ms per field (achieved
fields/s); unpaced (`--fast`): times real time.

| Scene (VI fields) | Chrome before | Chrome E2d | Firefox before | Firefox E2d |
|---|---:|---:|---:|---:|
| boot loading screen (0-580) | 3.9 (59.8/s) | 3.3 (59.8/s) | 2.3 (60/s) | 2.8 (59.8/s) |
| intro movie (600-1270) | 60.4 (15.8/s) | **7.0 (60/s)** | 53.7 (17.8/s) | **7.2 (60/s)** |
| licence screen (1300-1660) | 0.31 (60/s) | 0.25 (60/s) | 0.31 (60/s) | 0.28 (60/s) |
| title, "PRESS START" (1940-2150) | 0.96 (60/s) | 0.93 (60/s) | 1.09 (60/s) | 1.02 (60/s) |
| memory card dialog (2180-2500) | 2.5 (60/s) | 1.8 (60/s) | 2.6 (60/s) | 2.1 (60/s) |
| main menu + profile dialog (2730-3600) | 9.1 (60/s) | **5.0 (60/s)** | 8.1 (60/s) | **5.6 (60/s)** |
| audio underruns during the movie | 107 | 0 | 106 | 0 |
| unpaced: intro movie | 0.25x | **2.26x** | (0.07x, a disturbed run) | **2.14x** |
| unpaced: main menu | 1.79x | **3.31x** | (0.62x, a disturbed run) | **2.96x** |
| unpaced: boot loading screen | 4.51x | 5.42x | 2.75x | 3.51x |

- The intro movie now runs at 1.0x real time paced and about 2.2x unpaced in both
  browsers, with no sound underruns; the main menu takes 5.0-5.6 ms of the 16.7 ms
  field.
- Translation: the module is 62.1 MB (was 49.5); emit 340-360 ms in Chrome (was
  275-295), V8's lazy compile 19-20 ms, Firefox's compile 180-240 ms (was 122);
  cold start in Chrome 760-810 ms (was 685-730), Firefox 1.08-1.4 s.
- Firefox numbers are from a headed window on a machine other sessions shared;
  their spread is wider than Chrome's.

**Sound** (E2c; details in E2C_LOG.md): sound trails the picture by about
80-115 ms (the worklet's 60 ms target, its fill averaging 60-74 ms at full speed;
the context's 10 ms base latency in Chrome and 32 ms output latency in Firefox;
up to one field of batching). Over the `menus` run: no underruns after the intro
movie in Chrome, one in Firefox; 112-114 during the movie, which decodes below
real time. A 45 or 30 ms target underruns in the menus (16 and 21 times). The
mixer costs about 2 % of `rt::run` time.

**The server image** (E2c): on localhost the first 800 fields fetch 35 MB for
16.7 MB of guest reads and wait 165 ms. Simulated links (the first minute's read
trace through the block cache): the guest waits 1.8 s in all at 15 ms / 300
Mbit/s, 3.5 s at 30 ms / 200 Mbit/s (the longest stall 124 ms), 8 s at 50 ms /
100 Mbit/s, almost all of it during the boot loading screen and the load before
the licence screen.

**The server image over the internet (P1, 2026-10-15; details in P1_LOG.md).** The
real server with the Docker deployment's disc directory, Chrome 155 with CDP network
conditions on the page and its Workers, E3a's path into the match as an input
script (E3A_LOG.md §2, "Menu path"; to the poll of frame 5,200: 13,071 VI fields,
the first pitch), paced, sound on. Every
guest DVD read timed (2,094 reads, 31.2 MB):

| | read wait in all | reads > 50 ms | > 100 ms | longest | worst second | during play (fields 10,500+) |
|---|---:|---:|---:|---:|---:|---:|
| before, 60 ms / 100 Mbit/s | 18.6 s | 169 | 126 | 209 ms | 1,047 ms of reads | 2.9 s, 29 reads > 50 ms |
| before, 80 ms / 50 Mbit/s | 27.3 s | 173 | 173 | 352 ms | 1,036 ms | 4.2 s, 29 > 50 ms |
| after (kept in the browser), 60 ms / 100 Mbit/s | 0.86 s | 0 | 0 | 1.7 ms | 112 ms | 0.09 s, 0 > 16.7 ms |
| before, 60 ms / 100 Mbit/s, machine loaded (20 busy threads and a disk writer) | 20.4 s | 170 | 127 | 1,416 ms | 1,417 ms | 3.0 s, 29 > 50 ms |
| after, the same load | 1.90 s | 0 | 0 | 5.0 ms | 237 ms | 0.18 s, 0 > 16.7 ms |

The one-time download took 112 s at 60 ms / 100 Mbit/s (12.3 MB/s, the link's
rate; 114 s with the machine loaded), 57 s at 40 ms / 200 Mbit/s, 223 s at 80 ms /
50 Mbit/s, 4.8 s on localhost. The disc check before the boot fell from 1,240 ms to
24 ms. The guest line at frame 5,200 is the same either way (`MEM1 8d79a7b5...`).

**The threaded engine (P2, 2026-10-15; P2_LOG.md section 4).** Chrome 155 headed,
after CLK2, paced, 60 s of CPU-vs-CPU play and 40 s of human pitching, against
origin/main `6ed1c7a9`: speed 0.97-0.98x -> 1.000x; the Worker's (now the
emulation thread's) time per field 14.7-14.8 -> 11.4 ms (CPU vs CPU) and 13.7 ->
10.9 ms (human); composited frame intervals p95 38-50 ms with 9-13% over 25 ms
-> 20-21 ms with 0.5-0.8%; new frames shown 51-55 -> 58-59 a second. The render
thread works 4.7-6.0 ms per field.

**Q6 (2026-10-17; Q6_LOG.md section 11).** Chrome 155 headed, paced, the same two
sessions, interleaved against origin/main (Q4's engine with Q5b's shell), up to three
rounds. Four efficiency cores: human pitching 0.87-0.91x -> **1.000x**, CPU vs CPU
0.91-0.93x -> **0.993x**, new frames shown 34-44 -> 55-57 a second, composited p95 46-52
-> 21-23 ms. hostSlow 2: 0.89-0.90x -> **0.970-0.985x**, 36-42 -> 49-52 new frames a
second. hostSlow 1.3 (the laptop's stand-in): full speed before and after, emulation 11.0-12.1
-> **9.6-10.1 ms** per field. This PC: 8.7-10.4 -> 7.4-7.9 ms per field. The emulation
thread alone (Node) is 14 % faster; on the slower machines most of the gain is that the
emulation no longer waits for the render thread (Q6's copies, Q6_LOG.md section 9).

## 4. Determinism

The browser's guest line (`mvp_guest_line`: instructions, time base, frame
polls, SHA-1 of MEM1; the same line as `mvp-play`'s) after a fixed number of VI
fields equals the native one for the same input script:

| Run | Guest line | Matches |
|---|---|---|
| 700 fields, no input | 2,845,943,115 instructions, time base 474,323,852, 94 polls, MEM1 `eef70ba3...` | native `mvp-play` (with and without `--yield`), Chrome, Firefox, WebKit; Chrome from the server image with sound and a 3 s hidden-tab pause at field 304 (E2c) |
| 2,400 fields, START at 280/430/580/730 | 9,737,828,100, 1,622,971,350, 971 polls, MEM1 `4df06af1...` | native (with and without `--yield`), Chrome |
| 3,600 fields, the `menus` script | 14,602,688,100, 2,433,781,350, 2,122 polls, MEM1 `95f5c597...` | native, Chrome (paced, unpaced, Liftoff only, no rendering), Edge, Firefox, WebKit; **with sound playing** (the mixer attached at field 0, the worklet consuming), Chrome and Firefox from the server image, Chrome with 30 and 45 ms audio buffers and unpaced; Chrome without the mixer (E2c) |

Three different WebAssembly engines (V8, SpiderMonkey, JavaScriptCore), paced
or not, with or without rendering, with or without sound, read from a file or a
server, paused or not, with yields at every field: the same guest state as the
native build that does not yield.

**Save-state files (2026-10-19).** A state saved by native `mvp-play` at frame poll
2,000 (`--state-save 2000:FILE`, no input) and loaded into a fresh process
(`--state-load FILE`) reaches frame 3,000 with the line of the run that never
stopped (22,097,623,848 instructions, time base 2,301,835,817, MEM1 `57bd399e...`);
so does Chrome loading the same file (`browser_check.mjs --states DIR --params
state=NAME --frames 3000`), and Chrome again after F2 at frame 2,051 and F4 later
(back to 2,051, then on to 3,000).

**Since E3a and E2d (2026-10-14):** the native engine takes the floating-point
unavailable exception (E3A_LOG.md), which changed the guest state once, and the
wasm emitter takes it the same way (E2D_LOG.md §3). The lines above are pre-E3a
values; the current ones, native `mvp-play --no-render` (translated code at `/Od`)
and the browsers with E2d's emitter, with sound on, unpaced:

| Run | Guest line | Matches |
|---|---|---|
| 700 fields, no input | 2,845,943,121 instructions, time base 474,323,853, 94 polls, MEM1 `73e856d4...` | native, Chrome 155, Firefox 157; Chrome from the image kept in the browser (P1) |
| 2,400 fields, START at 280/430/580/730 | 9,737,828,100, 1,622,971,350, 966 polls, MEM1 `c70ca6c5...` | native, Chrome, Firefox; Chrome from the kept image (P1) |
| 3,600 fields, the `menus` script | 14,602,688,100, 2,433,781,350, 2,117 polls, MEM1 `5e150b6c...` | native, Chrome (paced and unpaced), Firefox (paced and unpaced); `mvp-boot-wasm` under Node; Chrome from the kept image, unpaced (P1) |

**Since CLK2 (2026-10-15):** the virtual CPU charges 1.25 cycles per instruction and
list walks are no longer idle-skipped (CLK2_LOG.md), so every line changes again
(the values above are CLK1's). Native `mvp-play --no-render` at `/O2`:

| Run | Guest line | Matches |
|---|---|---|
| 700 fields, no input | 4,547,010,125 instructions, time base 473,646,888, 360 polls, MEM1 `53221c4a...` | native, Chrome 155 (unpaced) |
| 2,400 fields, START at 280/430/580/730 | 15,574,026,125, 1,622,294,388, 1,882 polls, MEM1 `4285f135...` | native |
| 3,600 fields, the `menus` script | 23,357,802,126, 2,433,104,388, 2,747 polls, MEM1 `40a7692d...` | native at `/O2`, at `/Od` and with `--yield`; Chrome 155 (unpaced) |

**The threaded engine (P2):** the same lines in Chrome 155 threaded (700 fields
with sound, 2,400 and 3,600 fields, paced and unpaced), single-threaded on an
isolated page and without isolation, and in Firefox 157 threaded (700 and 3,600);
two threaded pages in a netplay game agree with native `mvp-netplay solo`; the
renderer digests of the threaded and the inline engine agree in every field of
both match sessions (P2_LOG.md section 3).

**Q6 (2026-10-17): the human-pitching line changed; the lines were re-recorded.**
Q6 stopped modeling NI's flush to zero in the fast floating-point model (the owner's
F1; Q6_LOG.md section 2), which changes what the guest computes for results below the
normal range. The three checkpoints above are unchanged (re-recorded: native, and
Chrome 155 threaded and single-threaded, paced, without isolation). The two match
sessions' lines, which earlier logs carried (the input scripts of P2's CPU vs CPU and
human-pitching sessions):

| Run | Guest line | Matches |
|---|---|---|
| CPU vs CPU, 11,600 fields | 75,249,642,125 instructions, time base 7,838,504,388, 10,544 polls, MEM1 `d8c30e9a...` (unchanged) | native; Chrome 155 threaded (paced and unpaced) and single-threaded (unpaced) |
| Human pitching, 9,000 fields | 58,384,794,125, 6,081,749,388, 8,054 polls, MEM1 **`caec614c...`** (before Q6: `58631fe2...`) | native; Chrome 155 threaded (paced and unpaced) and single-threaded (unpaced) |

The renderer digests of the threaded engine (with Q6's copies, Q6_LOG.md section 9) and
the inline engine agree in every field of both sessions; for CPU vs CPU they also equal
the digests before Q6. Two threaded pages in a netplay game agree with native
`mvp-netplay solo` (`2a8de818...`, unchanged).

## 5. Limits and next steps

- **Safari is not measured.** Playwright's WebKit on Windows has no WebGPU; the
  engine itself runs there (same guest line), and the page now says the browser
  has no WebGPU. A Mac is needed for Safari with WebGPU.
- **Firefox headless** gets no WebGPU adapter; headed works.
- **Speed:** since E2d the intro movie runs at full speed (about 2.2x real time
  unpaced) and the main menu uses about a third of the frame budget in Chrome and
  Firefox (§3, "Frame time, E2d"). What is left and what was tried:
  `docs/engine/E2D_LOG.md` §5.
- **Remote play:** done in P1: the image is downloaded once into the browser
  (DECISIONS 2026-10-14 allows it) and read locally; the game never waits on the
  network. The first visit waits for the download (about 1.4 GB: 112 s at 100
  Mbit/s). Reading over the network remains only as the fallback.
- **Gamepads** were tested with unit tests, fake `getGamepads()` objects in
  Chrome, Edge and Firefox (the panel, hot-plug, ports, the guided setup) and
  the real game on fake pads (replay-equal, online too), not yet with a real
  controller: the profiles for non-standard pads (`Q3_LOG.md` section 2) are the
  documented layouts, not hardware-tested, and GameCube adapters need their
  guided setup once.
- **The real server** (E5, `engine/server/`) was tested locally (Chrome: login,
  the disc from `/config.json`, sound, the cache; and an online game of two
  pages that both read the server's image, in lockstep with no desync), not yet
  on the VPS.
- Before hosting on the VPS: serve the site over **HTTPS** (WebGPU, AudioWorklet
  and the origin private file system exist only in secure contexts; localhost
  counts as one), with `.js` as `text/javascript` (the worklet is a module),
  `.wasm` as `application/wasm` and `.json` as JSON; serve `licenses/` with the
  engine, and decide the production headers (a Content-Security-Policy, caching)
  and compression.
