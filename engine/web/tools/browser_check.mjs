// Local browser check for E2/E2c (needs the player's disc; never run in CI):
// serves the site, opens it in a real browser with Playwright, gives it the
// disc image (through the page's file input, or from the mock deployment's
// /config.json with --server-disc), and records the startup phases, the cold
// start (start -> first frame), frame times, canvas screenshots, audio levels,
// and the determinism line.
//
//   node engine/web/tools/browser_check.mjs --browser chrome|msedge|chromium|firefox|webkit
//        [--disc PATH] [--out DIR] [--script FILE] [--fields N] [--frames N] [--fast]
//        [--shots F1,F2,...] [--keys F1:KEY,F2:KEY,...] [--headed] [--timeout SECONDS] [--no-render]
//        [--js-flags FLAGS] [--tag NAME]
//        [--server-disc [--login] [--latency MS] [--rate MBPS]] [--audio | --no-audio] [--audible]
//        [--audio-latency MS] [--pad F:button:I,F:axis:I:V,...] [--runs N] [--clear-cache] [--no-cache]
//        [--hide F:MS] [--trace-disc] [--disc-cache PARAMS] [--hold MS] [--real-server MAIN_JS [--disc-dir DIR]]
//        [--throttle RTT_MS,MBIT] [--io-log] [--clear-disc] [--store-off] [--profile DIR]
//        [--threads 0] [--no-isolation] [--digest] [--cast-size WxH]   (P2; see below)
//        [--host-slow N[@F]] [--affinity HEXMASK] [--load N]   (Q2; see below)
//
//   --disc     the image (default: $MVP_DISC). It goes to the page's file input
//              locally (or, with --server-disc, to the mock server on 127.0.0.1)
//   --out      where screenshots and the summary go (default build/engine/web-check);
//              keep it under build/: screenshots are game-derived, never committed
//   --script   an mvp-input script file (runtime/input.h) for the run
//   --packs    roster packs (E6), comma-separated pack folders or .zip files, in set
//              order: given to the page's Roster packs panel before the disc
//   --server-packs  roster packs the server offers, comma-separated file names
//              (e.g. mlb-2026.zip): chosen from the panel's "Packs on this server"
//              list, as a player does, after any --packs (needs --real-server and
//              --packs-dir)
//   --fields / --frames   stop there and print the guest line (MEM1 SHA-1)
//   --fast     unpaced (as fast as possible); default paced to 59.94 fields/s
//   --shots    VI fields at which to screenshot the canvas (approximately: the
//              page reports progress twice a second)
//   --grab     F:N,... (V3): from VI field F, the next N pictures the render thread sends
//              (in-between pictures too), as PNGs (game-derived: keep them under build/)
//   --keys     live keyboard input: press KEY (a KeyboardEvent key name, e.g.
//              Enter, x, ArrowDown; KEY+KEY holds several together, e.g. q+e+Space for the
//              ABS challenge) for 150 ms (--hold MS) once the run reaches field F
//   --js-flags V8 flags for Chrome/Edge/Chromium (experiments, e.g. --no-wasm-tier-up)
//   --tag      added to the output file names (summary-BROWSER-TAG.json, ...)
//   --server-disc  the mock deployment (serve.mjs --disc): the page finds the
//              image in /config.json and loads it without the picker. --login:
//              the mock's API wants the access cookie (expect the no-access error);
//              --latency, --rate: a slow server
//   --real-server  E5's server (engine/server, built: dist/src/main.js) instead of
//              the mock, with a throwaway access key (the check opens the player
//              link) and the disc configured
//   --packs-dir with --real-server: the server's packs directory (MVP_PACKS_DIR): the
//              zips in it are the packs the page offers in its dropdown
//   --disc-dir with --real-server: the server's disc directory (MVP_DISC_DIR, as the
//              Docker deployment: DIR/GV4E69.img is the uploaded image) instead of
//              MVP_DISC_IMAGE
//   --admin    with --real-server: a throwaway admin key too, and the check opens the admin
//              link instead of the player link (the browser is the server's admin)
//   --server-states DIR  with --real-server and --admin: the admin's save states on the server
//              (MVP_STATES_DIR); ?state=NAME (--params) loads one of them
//   --throttle Chrome's network conditions (CDP) on the page and its Workers: a round
//              trip of RTT_MS and MBIT Mbit/s each way, e.g. 60,100 (Chromium browsers)
//   --site     the site directory to serve (default build/engine/emscripten/web/site)
//   --clear-disc  run 1 starts without the server image kept in the browser (P1)
//   --store-off   the page reads the server's image over the network (store=0) instead
//              of downloading it once into the browser's storage
//   --interrupt-download PCT  reload the page once when the download reaches PCT %
//              (it must resume: the summary's download.resumedFrom)
//   --fallback after a download error, press "Play without keeping the game data"
//   --params Q more page parameters (e.g. storeSpare=100000000 for the quota error)
//   --states DIR  save-state files for the page's state=NAME (serve.mjs --states; local only)
//   --mock-packs DIR  with --server-disc: the mock server offers DIR's pack zips (serve.mjs --packs)
//   --profile  a browser profile directory on disk (persistent: what a player's
//              browser keeps between visits, and its real storage quota) instead of a
//              fresh in-memory context
//   --io-log   log every guest DVD read with its host timing (ioLog=1) into
//              OUT/io-*.txt and summarize the waits in the summary
//   --audio    click the page first (the user gesture that starts audio), then
//              sample the worklet's output through the page's AnalyserNode tap.
//              The browser's output is muted (--audible: not muted)
//   --no-audio the page never mixes (audio=0), for the determinism comparison
//   --pad      a fake standard gamepad (navigator.getGamepads replaced before the
//              page loads): at field F press button I for 150 ms, or hold axis I
//              at V for 300 ms
//   --runs N   N runs in one browser context (one profile): run 1 is cold, later
//              runs find the translation cached; --clear-cache empties the cache
//              first; --no-cache: cache=0
//   --hide     at field F, report the tab hidden for MS ms (the pause), then shown
//   --trace-disc  record the picked image's reads (offsets and sizes, no data)
//              into OUT/disc-trace-*.txt, to tune the server image's cache
//   --disc-cache  that cache's settings as page parameters (discBlock=KIB&
//              discBlocks=N&discRun=N&discAhead=N)
//   --host-slow N[@F]  (Q2) a machine N times slower, simulated from VI field F on: the engine's threads
//              spin N - 1 times their working time (hostSlow=N&hostSlowFrom=F) and Chrome's CPU throttling
//              slows the page's main thread N times (DevTools throttling reaches only the main thread)
//   --affinity HEXMASK  (Q2, Windows) the browser's processes run only on these logical processors (e.g.
//              four efficiency cores, a weaker CPU), set once the page has loaded and again at the first frame
//   --load N   (Q2) N busy processes (one thread each) at normal priority for the whole run
//   --actions F:KIND[:ARG],...  (Q7) host-side changes in the middle of a run, once it reaches field F:
//              resize:WxH (the page's viewport), picture:auto|1|2|3 (the Picture setting, as a player
//              chooses it), fullscreen (the Fullscreen button: on, or off again), stats (the overlay)
//   --compare PNG  (Q7) compare the final picture with PNG (another run's -final.png, same viewport and
//              --dpr) over --compare-region x0,y0,x1,y1 in the console's 640 x 448 pixels (default
//              0,0,128,128: the block the game saves and restores in every frame of play); the run fails
//              when the region's mean difference per channel exceeds --compare-max (default 3). The
//              scale-change check: the same script at a fixed scale (--params scale=3) to field F, then
//              with --actions F0:picture:3 to F and --compare that run's -final.png (WEB.md)
//
// Exit status: 0 when every run reached its first frame (and its stop, if any),
// 1 otherwise, 77 without a disc.

import { spawn, spawnSync } from 'node:child_process';
import { randomBytes } from 'node:crypto';
import { existsSync, mkdirSync, readFileSync, statSync, writeFileSync } from 'node:fs';
import { join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { chromium, firefox, webkit } from 'playwright';

const here = fileURLToPath(new URL('.', import.meta.url));
const repo = resolve(here, '..', '..', '..');
const argv = process.argv.slice(2);
const opt = (name, fallback) => {
  const i = argv.indexOf(name);
  return i >= 0 && i + 1 < argv.length ? argv[i + 1] : fallback;
};
const flag = (name) => argv.includes(name);

const browserName = opt('--browser', 'chrome');
const disc = opt('--disc', process.env.MVP_DISC ?? '');
const out = resolve(opt('--out', join(repo, 'build', 'engine', 'web-check')));
const scriptFile = opt('--script', '');
const fields = Number(opt('--fields', '0'));
const frames = Number(opt('--frames', '0'));
const fast = flag('--fast');
const headed = flag('--headed');
const render = !flag('--no-render');
const timeoutS = Number(opt('--timeout', '600'));
const jsFlags = opt('--js-flags', '');
const tag = opt('--tag', '');
const realServer = opt('--real-server', '');  // E5's server: engine/server/dist/src/main.js
const serverDisc = flag('--server-disc') || !!realServer;
const login = flag('--login');
const latency = opt('--latency', '');
const rate = opt('--rate', '');
const audio = flag('--audio');
const noAudio = flag('--no-audio');
const audible = flag('--audible');
const audioLatency = opt('--audio-latency', '');
const runs = Math.max(1, Number(opt('--runs', '1')));
const clearCache = flag('--clear-cache');
const noCache = flag('--no-cache');
const traceDisc = flag('--trace-disc');
const discParams = opt('--disc-cache', '');  // e.g. discBlock=256&discRun=8&discAhead=8
// P1 (development): the presentation trace in each stats window; unpaced up to a field, then paced;
// the composited page as Chrome shows it (CDP screencast, Chromium only) from field F, N frames.
const tracePresent = flag('--trace-present');
// P2: --threads 0: the single-threaded renderer even where the threaded engine could run (threads=0);
// --no-isolation: the server sends no cross-origin isolation headers (the page cannot share memory, so
// the Worker falls back to the single-threaded engine); --digest: the renderer digest per VI field
// (digest=1) into OUT/digest-LABEL-rN.txt, to compare the threaded and the inline renderer.
const threadsOff = opt('--threads', '') === '0';
const noIsolation = flag('--no-isolation');
const digest = flag('--digest');
const fastUntil = Number(opt('--fast-until', '0'));
const screencast = opt('--screencast', '') ? opt('--screencast', '').split(':').map(Number) : null;
// P2: --cast-size WxH: screencast frames as small JPEGs, acknowledged before they are written (default: PNGs of
// the whole page, acknowledged after). Chrome sends the next frame only after the acknowledgement, so large PNGs
// cap the capture below 60 frames per second on a busy machine; small JPEGs keep up (timing measurements).
const castSize = opt('--cast-size', '') ? opt('--cast-size', '').split('x').map(Number) : null;
// --trace F:MS (Chromium): a Chrome performance trace (V8's sampling profiler in every thread, the GPU
// process's tasks) from field F for MS ms, written to OUT/trace-LABEL.json (summarize: trace_summary.mjs).
const traceAt = opt('--trace', '') ? opt('--trace', '').split(':').map(Number) : null;
const discDir = opt('--disc-dir', '');
const asAdmin = flag('--admin');
const serverStates = opt('--server-states', '');
const throttle = opt('--throttle', '') ? opt('--throttle', '').split(',').map(Number) : null;
const ioLog = flag('--io-log');
const clearDisc = flag('--clear-disc');
const storeOff = flag('--store-off');
const profile = opt('--profile', '');
const interruptAt = Number(opt('--interrupt-download', '0'));
const fallback = flag('--fallback');
const pageParams = opt('--params', '');
const statesDir = opt('--states', '');
const mockPacks = opt('--mock-packs', '');
const label = tag ? `${browserName}-${tag}` : browserName;
const chromiumArgs = ['--enable-unsafe-webgpu', ...(audible ? [] : ['--mute-audio']),
                      ...(jsFlags ? [`--js-flags=${jsFlags}`] : []),
                      // --chrome-args 'A B': more switches (e.g. --disable-gpu-vsync --disable-frame-rate-limit,
                      // animation frames as fast as the browser can, like a high-refresh display)
                      ...opt('--chrome-args', '').split(' ').filter(Boolean)];
const packPaths = opt('--packs', '').split(',').filter(Boolean).map((p) => resolve(p));
const serverPacks = opt('--server-packs', '').split(',').filter(Boolean);
const packsDir = opt('--packs-dir', '');
const shots = opt('--shots', '').split(',').filter(Boolean).map(Number).sort((a, b) => a - b);
// V3: --grab F:N,...: from field F, the next N pictures the render thread sends (in-between pictures
// included, threaded engine with ImageBitmaps), as OUT/LABEL-rR-grab-F-K-{between|frame}.png.
const grabs = opt('--grab', '').split(',').filter(Boolean).map((g) => g.split(':').map(Number))
  .sort((a, b) => a[0] - b[0]);
const keys = opt('--keys', '').split(',').filter(Boolean).map((k) => {
  const [f, key] = k.split(':');
  return { field: Number(f), key };
}).sort((a, b) => a.field - b.field);
const pads = opt('--pad', '').split(',').filter(Boolean).map((p) => {
  const [f, kind, index, value] = p.split(':');
  return { field: Number(f), kind, index: Number(index), value: value === undefined ? 1 : Number(value) };
}).sort((a, b) => a.field - b.field);
const compareRef = opt('--compare', '');  // Q7: a reference picture for the final one
const compareRegion = opt('--compare-region', '0,0,128,128').split(',').map(Number);
const compareMax = Number(opt('--compare-max', '3'));
const actions = opt('--actions', '').split(',').filter(Boolean).map((a) => {  // Q7
  const [f, kind, arg = ''] = a.split(':');
  return { field: Number(f), kind, arg };
}).sort((a, b) => a.field - b.field);
const hide = opt('--hide', '') ? opt('--hide', '').split(':').map(Number) : null;
const holdMs = Number(opt('--hold', '150'));  // how long --keys and --pad buttons stay down
// Q2: weaker machines, simulated.
const hostSlow = opt('--host-slow', '') ? opt('--host-slow', '').split('@').map(Number) : null;
const affinity = opt('--affinity', '');
const loadProcs = Number(opt('--load', '0'));

if (!disc || !existsSync(disc)) {
  console.error('browser_check: no disc image (--disc or MVP_DISC)');
  process.exit(77);
}
mkdirSync(out, { recursive: true });

// The server, on a free port: serve.mjs (with --server-disc its mock of the
// deployment), or with --real-server E5's server (engine/server, built) with a
// throwaway access key and the disc configured.
// --site DIR: another build's site (e.g. a profiling build with function names).
const site = resolve(opt('--site', join(repo, 'build', 'engine', 'emscripten', 'web', 'site')));
const accessKey = randomBytes(18).toString('base64url');
const adminKey = randomBytes(18).toString('base64url');
let server;
if (realServer) {
  server = spawn(process.execPath, [realServer], {
    env: { ...process.env, MVP_ACCESS_KEY: accessKey, MVP_SITE: site, MVP_PORT: '0', MVP_HOST: '127.0.0.1',
           ...(discDir ? { MVP_DISC_DIR: resolve(discDir) } : { MVP_DISC_IMAGE: disc }),
           ...(packsDir ? { MVP_PACKS_DIR: resolve(packsDir) } : {}),
           ...(asAdmin ? { MVP_ADMIN_KEY: adminKey } : {}),
           ...(serverStates ? { MVP_STATES_DIR: resolve(serverStates) } : {}) },
    stdio: ['ignore', 'pipe', 'inherit'],
  });
} else {
  const serverArgs = [join(here, 'serve.mjs'), '--port', '0', '--root', site];
  if (noIsolation) serverArgs.push('--no-isolation');
  if (statesDir) serverArgs.push('--states', resolve(statesDir));
  if (serverDisc) {
    if (mockPacks) serverArgs.push('--packs', resolve(mockPacks));
    serverArgs.push('--disc', disc);
    if (login) serverArgs.push('--login');
    if (latency) serverArgs.push('--latency', latency);
    if (rate) serverArgs.push('--rate', rate);
  }
  server = spawn(process.execPath, serverArgs, { stdio: ['ignore', 'pipe', 'inherit'] });
}
const base = await new Promise((resolveUrl, reject) => {
  server.stdout.on('data', (d) => {
    const m = /(?:serving|listening on) (http:\/\/[^\s;]+)/.exec(String(d));
    if (m) resolveUrl(m[1].endsWith('/') ? m[1] : `${m[1]}/`);
  });
  server.on('exit', (code) => reject(new Error(`server exited ${code}`)));
});

const launchers = {
  chrome: () => chromium.launch({ channel: 'chrome', headless: !headed, args: chromiumArgs }),
  msedge: () => chromium.launch({ channel: 'msedge', headless: !headed, args: chromiumArgs }),
  chromium: () => chromium.launch({ headless: !headed, args: chromiumArgs }),
  firefox: () => firefox.launch({
    headless: !headed,
    firefoxUserPrefs: {
      'dom.webgpu.enabled': true, 'dom.webgpu.workers.enabled': true, 'gfx.webgpu.ignore-blocklist': true,
      ...(audible ? {} : { 'media.volume_scale': '0.0' }),
    },
  }),
  webkit: () => webkit.launch({ headless: !headed }),
};
if (!launchers[browserName]) {
  console.error(`browser_check: unknown browser ${browserName}`);
  process.exit(2);
}

// A fake standard gamepad (Playwright cannot emulate one): the page's
// navigator.getGamepads returns window.__fakePad, which the check edits.
function fakeGamepad() {
  const pad = {
    id: 'browser_check fake pad (STANDARD GAMEPAD)', index: 0, connected: true, mapping: 'standard', timestamp: 0,
    buttons: Array.from({ length: 17 }, () => ({ pressed: false, touched: false, value: 0 })),
    axes: [0, 0, 0, 0],
  };
  window.__fakePad = pad;
  Object.defineProperty(navigator, 'getGamepads', { value: () => [pad, null, null, null], configurable: true });
}

// The tab hidden or shown, as the page sees it.
function setHidden(hidden) {
  Object.defineProperty(document, 'hidden', { value: hidden, configurable: true });
  Object.defineProperty(document, 'visibilityState', { value: hidden ? 'hidden' : 'visible', configurable: true });
  document.dispatchEvent(new Event('visibilitychange'));
}

// The guest's DVD reads (--io-log): "field at_ms offset size ms" lines. Totals, and the
// reads that kept a field waiting (> 16.7 ms), > 50 ms and > 100 ms, before and after
// the first frame, with the longest ones.
function summarizeIo(text, firstFrameField) {
  const rows = text.trim().split(/\r?\n/).filter(Boolean).map((l) => l.split(' ').map(Number));
  const part = (rs) => ({
    reads: rs.length, mb: +(rs.reduce((n, r) => n + r[3], 0) / 1048576).toFixed(1),
    waitMs: +rs.reduce((n, r) => n + r[4], 0).toFixed(0), maxMs: +Math.max(0, ...rs.map((r) => r[4])).toFixed(1),
    over16: rs.filter((r) => r[4] > 1000 / 59.94).length, over50: rs.filter((r) => r[4] > 50).length,
    over100: rs.filter((r) => r[4] > 100).length,
  });
  const longest = [...rows].sort((a, b) => b[4] - a[4]).slice(0, 10)
    .map((r) => ({ field: r[0], offset: r[2], size: r[3], ms: r[4] }));
  return { all: part(rows), beforeFirstFrame: part(rows.filter((r) => r[0] < firstFrameField)),
           afterFirstFrame: part(rows.filter((r) => r[0] >= firstFrameField)), longest };
}

// --affinity (Windows): the logical processors the browser's processes may use: every chrome.exe (or
// msedge.exe) descending from this check's process (Playwright starts the browser as its child; the
// renderer, GPU and utility processes are the browser's). Returns how many were set.
function setAffinity(mask) {
  const ps = `$m=[IntPtr]0x${mask}; $all=Get-CimInstance Win32_Process; $ids=@(${process.pid}); ` +
    `do { $more=@($all | Where-Object { $ids -notcontains $_.ProcessId -and $ids -contains $_.ParentProcessId } | ` +
    `ForEach-Object { $_.ProcessId }); $ids+=$more } while ($more.Count -gt 0); $n=0; ` +
    `foreach ($p in ($all | Where-Object { $ids -contains $_.ProcessId -and $_.Name -match '^(chrome|msedge)[.]exe$' })) ` +
    `{ try { (Get-Process -Id $p.ProcessId).ProcessorAffinity=$m; $n++ } catch {} }; $n`;
  const r = spawnSync('powershell', ['-NoProfile', '-Command', ps], { encoding: 'utf8' });
  const set = Number(String(r.stdout).trim().split(/\s+/).pop());
  console.log(`affinity 0x${mask}: ${set} browser processes`);
  return { mask, processes: set };
}

async function runOnce(context, run) {
  const summary = { run };
  const page = await context.newPage();
  const consoleErrors = [];
  page.on('console', (m) => {
    if (m.type() === 'error') consoleErrors.push(m.text());
  });
  page.on('pageerror', (e) => consoleErrors.push(String(e)));
  // Only the local server's own files: the disc goes in through the file input
  // or from the mock deployment on the same origin.
  const requests = [];
  page.on('request', (r) => requests.push(r.url()));
  if (pads.length) await page.addInitScript(fakeGamepad);
  if (throttle) {
    // Page-level conditions also slow the page's dedicated Workers' requests (Chrome 155, checked).
    const cdp = await context.newCDPSession(page);
    const bytesPerS = (throttle[1] * 1e6) / 8;
    await cdp.send('Network.enable');
    await cdp.send('Network.emulateNetworkConditions',
                   { offline: false, latency: throttle[0], downloadThroughput: bytesPerS, uploadThroughput: bytesPerS });
  }

  const q = new URLSearchParams();
  if (scriptFile) q.set('script', readFileSync(scriptFile, 'utf8'));
  if (fields) q.set('fields', String(fields));
  if (frames) q.set('frames', String(frames));
  if (fast) q.set('fast', '1');
  if (!render) q.set('render', '0');
  if (noAudio) q.set('audio', '0');
  if (noCache) q.set('cache', '0');
  if (audioLatency) q.set('audioLatency', audioLatency);
  if (traceDisc) q.set('traceDisc', '1');
  if (tracePresent) q.set('tracePresent', '1');
  if (threadsOff) q.set('threads', '0');
  if (digest) q.set('digest', '1');
  if (fastUntil) q.set('fastUntil', String(fastUntil));
  if (ioLog) q.set('ioLog', '1');
  if (hostSlow) {
    q.set('hostSlow', String(hostSlow[0]));
    if (hostSlow[1]) q.set('hostSlowFrom', String(hostSlow[1]));
  }
  for (const [k, v] of new URLSearchParams(discParams)) q.set(k, v);
  for (const [k, v] of new URLSearchParams(pageParams)) q.set(k, v);
  if (storeOff) q.set('store', '0');
  // Open the player link (or the admin link), as a player would (on a page that starts nothing: the server
  // answers with the cookie and goes on to /healthz), and empty the browser's caches before the game page loads.
  if (realServer && run === 1) {
    await page.goto(asAdmin ? `${base}healthz?admin=${adminKey}` : `${base}healthz?key=${accessKey}`);
    if (page.url() !== `${base}healthz`) throw new Error(`the ${asAdmin ? 'admin' : 'player'} link went to ${page.url()}`);
  }
  if (run === 1 && (clearCache || clearDisc)) {
    if (!realServer) await page.goto(`${base}style.css`);
    await page.evaluate(async ([translation, disc]) => {
      const root = await navigator.storage.getDirectory();
      if (translation) await root.removeEntry('mvp-translated', { recursive: true }).catch(() => undefined);
      if (disc) await root.removeEntry('mvp-disc', { recursive: true }).catch(() => undefined);
    }, [clearCache, clearDisc]);
  }
  const t0 = Date.now();
  await page.goto(`${base}?${q}`);
  summary.webgpu = await page.evaluate(() => 'gpu' in navigator);
  summary.crossOriginIsolated = await page.evaluate(() => self.crossOriginIsolated === true);  // P2
  if (affinity) summary.affinity = setAffinity(affinity);
  // The user gesture that starts audio: a real click on the page.
  if (audio) await page.mouse.click(5, 5);
  // Roster packs (E6): through the panel's inputs, as a player picks them, before the disc.
  for (const [i, p] of packPaths.entries()) {
    await page.setInputFiles(statSync(p).isDirectory() ? '#packs-folder' : '#packs-zip', p);
    await page.waitForFunction((n) => window.mvp.packs?.count === n && window.mvp.packs.report, i + 1, { timeout: 30000 });
  }
  // The server's packs: from the panel's dropdown (downloaded from the server behind the private link).
  for (const [i, file] of serverPacks.entries()) {
    await page.waitForSelector('#packs-server', { timeout: 30000 });
    await page.selectOption('#packs-server', file);
    await page.waitForFunction((n) => window.mvp.packs?.count === n && window.mvp.packs.report, packPaths.length + i + 1,
      { timeout: 120000 });
  }
  if (packPaths.length || serverPacks.length) {
    summary.packsPanel = await page.evaluate(() => window.mvp.packs.report);
    console.log(`packs: set ${summary.packsPanel.set_hash}, ${summary.packsPanel.issues.length} issue(s) before the disc`);
  }
  if (!serverDisc) await page.setInputFiles('#disc', disc);
  else {
    // With the lobby on the page (E5), the server's disc waits for Play (or a room).
    // P1: the first visit downloads the image into the browser first (a progress bar).
    let lastPct = -1, interrupted = false;
    for (;;) {
      const s = await page.evaluate(() => ({ source: window.mvp.source, error: window.mvp.error, download: window.mvp.download,
                                             ready: !document.getElementById('start-box')?.hidden,
                                             stream: !document.getElementById('stream-box')?.hidden }));
      if (s.error && fallback && s.stream && !summary.downloadError) {
        summary.downloadError = { ...s.error, download: s.download };
        console.log(`download error shown: ${s.error.message}`);
        await page.click('#stream');
        continue;
      }
      if (s.source || s.error || s.ready || Date.now() > t0 + timeoutS * 1000) {
        summary.download = s.download ?? null;
        summary.readyMs = Date.now() - t0;
        break;
      }
      const d = s.download;
      if (d?.state === 'downloading' && d.total) {
        const pct = Math.floor((100 * d.done) / d.total);
        if (interruptAt && !interrupted && pct >= interruptAt) {
          interrupted = true;
          summary.interruptedAt = d.done;
          console.log(`reloading the page at ${pct}% of the download`);
          await page.reload();
          continue;
        }
        if (pct >= lastPct + 10) {
          lastPct = pct;
          console.log(`download ${pct}% (${(d.done / 1048576).toFixed(0)} MB, ${(d.bytesPerS / 1048576).toFixed(1)} MB/s)`);
        }
      }
      await page.waitForTimeout(250);
    }
    if (summary.download) console.log(`disc in the browser: ${JSON.stringify(summary.download)}, ready ${summary.readyMs} ms after load`);
    if (await page.locator('#play').isVisible()) await page.click('#play');
  }

  const deadline = t0 + timeoutS * 1000;
  const mvp = () => page.evaluate(() => {
    const s = window.mvp;
    return { source: s.source, phases: s.phases, translate: s.translate, disc: s.disc, gfx: s.gfx,
             firstFrameMs: s.firstFrameMs, firstFrameField: s.firstFrameField, stats: s.stats, stopped: s.stopped,
             error: s.error, cache: s.cache, audio: s.audio, audioAttachedAtField: s.audioAttachedAtField,
             paused: s.paused, pad: s.pad, probe: s.audioProbe(), discTrace: s.discTrace,
             packs: s.packs?.mounted ?? null, ioLog: s.ioLog, digest: s.digest,
             hint: s.hint ?? null, autoScale: s.autoScale ?? null,
             log: s.log.filter((l) => /^picture: |WebGPU error|^timeline: review: host time|^mvp: |^motion: /.test(l)).slice(0, 20),
             stateLog: s.log.filter((l) => /^state: /.test(l)).slice(0, 40),
             card: s.card ?? null, cardLog: s.log.filter((l) => /^card: /.test(l)).slice(0, 20),
             display: s.display ? s.display() : null,  // V1: what reached the screen (presenter.ts)
             motion: s.motion ?? null };  // V3: the Motion setting and Auto's decision
  });
  const windows = [];
  const audioSamples = { n: 0, nonsilent: 0, maxRms: 0, maxPeak: 0, firstNonsilentField: null };
  let lastWindow = '', state = await mvp(), shotIndex = 0, keyIndex = 0, padIndex = 0, hidden = null, grabIndex = 0;
  let actionIndex = 0;
  summary.actions = [];
  summary.keysPressed = [];
  summary.padEvents = [];
  // --screencast F:N: every frame Chrome composites (it sends one only when the page changed), with the
  // wall time and the last field the page reported, until N frames.
  let cast = null;
  const castFrames = [];
  const startCast = async () => {
    cast = await page.context().newCDPSession(page);
    cast.on('Page.screencastFrame', (f) => {
      if (castSize) cast.send('Page.screencastFrameAck', { sessionId: f.sessionId }).catch(() => undefined);
      if (castFrames.length < screencast[1]) {
        const i = castFrames.length;
        const path = join(out, `${label}-r${run}-cast-${String(i).padStart(5, '0')}.${castSize ? 'jpg' : 'png'}`);
        writeFileSync(path, Buffer.from(f.data, 'base64'));
        castFrames.push({ i, t: f.metadata.timestamp, field: lastField });
      }
      if (!castSize) cast.send('Page.screencastFrameAck', { sessionId: f.sessionId }).catch(() => undefined);
    });
    await cast.send('Page.startScreencast', castSize
      ? { format: 'jpeg', quality: 50, maxWidth: castSize[0], maxHeight: castSize[1], everyNthFrame: 1 }
      : { format: 'png', everyNthFrame: 1 });
    console.log(`screencast started at field ${lastField}`);
  };
  let lastField = 0;
  let throttled = null, affinityAgain = false;  // Q2
  // --trace: the browser-wide tracing session (it records the Worker's thread too).
  let tracing = null, traceStart = 0;
  const startTrace = async () => {
    tracing = await page.context().browser().newBrowserCDPSession();
    await tracing.send('Tracing.start', {
      transferMode: 'ReturnAsStream',
      traceConfig: { includedCategories: ['disabled-by-default-v8.cpu_profiler', 'v8.execute', 'v8', 'gpu',
                                          'devtools.timeline', 'disabled-by-default-devtools.timeline', 'toplevel'] },
    });
    traceStart = Date.now();
    console.log(`trace started at field ${lastField}`);
  };
  const stopTrace = async () => {
    const done = new Promise((r) => tracing.once('Tracing.tracingComplete', r));
    await tracing.send('Tracing.end');
    const { stream } = await done;
    const chunks = [];
    for (;;) {
      const r = await tracing.send('IO.read', { handle: stream, size: 1 << 22 });
      chunks.push(r.base64Encoded ? Buffer.from(r.data, 'base64') : Buffer.from(r.data));
      if (r.eof) break;
    }
    await tracing.send('IO.close', { handle: stream });
    const path = join(out, `trace-${label}-r${run}.json`);
    writeFileSync(path, Buffer.concat(chunks));
    summary.trace = { path, fromField: traceAt[0], toField: lastField, ms: Date.now() - traceStart };
    console.log(`trace written at field ${lastField}: ${path}`);
  };
  while (Date.now() < deadline) {
    state = await mvp();
    if (state.error) break;
    if (state.stats && JSON.stringify(state.stats) !== lastWindow) {
      lastWindow = JSON.stringify(state.stats);
      const w = state.audio?.worklet;
      const st = state.stats.status;
      windows.push({ field: st.fields, frame: st.frames, at: Date.now() - t0, ...state.stats.window,
                     audio: w ? { fill: w.fill, underruns: w.underruns, skipped: w.skipped, target: w.target,
                                  gaps20: w.gaps20, gapMs: w.gapMs, silent: w.silentFrames, out: w.outFrames,
                                  ratio: w.ratio, stretch: w.stretch, stretched: w.stretched, splices: w.splices,
                                  speed: w.speed, transport: w.transport, ringDropped: w.ringDropped,
                                  inFrames: w.inFrames } : undefined,
                     // P2, threaded: the render thread's working time and the emulation's fence waits so far (ms)
                     renderBusyMs: st.render?.busy_ms, fenceMs: st.thread?.fence_ms, fences: st.thread?.fences,
                     gpu: st.gpu, scale: st.scale, presentsNew: st.presents_new,  // Q5
                     writeWaitMs: st.thread?.write_wait_ms, writeWaits: st.thread?.write_waits,
                     gx: st.gx,  // V5: the GX path's and the renderer's counts so far
                     display: state.display });  // V1: the page's counts so far (presenter.ts)
    }
    const field = state.stats?.status.fields ?? 0;
    lastField = field;
    if (screencast && !cast && field >= screencast[0]) await startCast();
    if (hostSlow && !throttled && field >= (hostSlow[1] || 0)) {
      throttled = await page.context().newCDPSession(page);
      await throttled.send('Emulation.setCPUThrottlingRate', { rate: hostSlow[0] });
      console.log(`host slowed ${hostSlow[0]}x at field ${field}`);
    }
    if (affinity && !affinityAgain && state.firstFrameMs !== undefined) {
      affinityAgain = true;
      summary.affinity = setAffinity(affinity);
    }
    if (traceAt && !tracing && field >= traceAt[0]) await startTrace();
    if (tracing && !summary.trace && Date.now() - traceStart >= traceAt[1]) await stopTrace();
    if (cast && castFrames.length >= screencast[1] && !summary.screencast) {
      await cast.send('Page.stopScreencast').catch(() => undefined);
      summary.screencast = castFrames;
      console.log(`screencast: ${castFrames.length} frames`);
    }
    if (state.probe) {
      ++audioSamples.n;
      audioSamples.maxRms = Math.max(audioSamples.maxRms, state.probe.rms);
      audioSamples.maxPeak = Math.max(audioSamples.maxPeak, state.probe.peak);
      if (state.probe.rms > 1e-4) {
        ++audioSamples.nonsilent;
        audioSamples.firstNonsilentField ??= field;
      }
    }
    while (keyIndex < keys.length && field >= keys[keyIndex].field) {
      const { key } = keys[keyIndex++];
      const chord = key.split('+');  // KEY+KEY: held together (e.g. q+e+Space, the ABS challenge's L+R+Z)
      for (const k of chord) await page.keyboard.down(k);
      await page.waitForTimeout(holdMs);
      for (const k of chord.reverse()) await page.keyboard.up(k);
      summary.keysPressed.push({ key, field });
      console.log(`pressed ${key} at field ${field}`);
    }
    while (padIndex < pads.length && field >= pads[padIndex].field) {
      const ev = pads[padIndex++];
      const set = (on) => page.evaluate(([e, down]) => {
        const p = window.__fakePad;
        if (e.kind === 'button') p.buttons[e.index] = { pressed: down, touched: down, value: down ? 1 : 0 };
        else p.axes[e.index] = down ? e.value : 0;
        p.timestamp = performance.now();
      }, [ev, on]);
      await set(true);
      await page.waitForTimeout(ev.kind === 'button' ? holdMs : Math.max(300, holdMs));
      const during = await page.evaluate(() => window.mvp.pad);
      await set(false);
      summary.padEvents.push({ ...ev, at: field, pageSaw: during });
      console.log(`gamepad ${ev.kind} ${ev.index} at field ${field}: page pad ${JSON.stringify(during)}`);
    }
    while (actionIndex < actions.length && field >= actions[actionIndex].field) {  // Q7
      const a = actions[actionIndex++];
      if (a.kind === 'resize') {
        const [w, h] = a.arg.split('x').map(Number);
        await page.setViewportSize({ width: w, height: h });
      } else if (a.kind === 'picture') {
        await page.selectOption('#picture', a.arg);
      } else if (a.kind === 'fullscreen') {
        // On: the toolbar's button (a user gesture). Off: the page's own way out (the button is
        // under the fullscreen stage then).
        if (await page.evaluate(() => !!document.fullscreenElement)) await page.evaluate(() => document.exitFullscreen());
        else await page.click('#fullscreen');
        await page.waitForTimeout(300);
      } else if (a.kind === 'stats') {
        await page.click('#stats-toggle');
      } else {
        throw new Error(`--actions: unknown action ${a.kind}`);
      }
      const canvasBox = await page.evaluate(() => {
        const c = document.getElementById('screen');
        return { css: [c.clientWidth, c.clientHeight], attr: [c.width, c.height], dpr: window.devicePixelRatio,
                 fullscreen: !!document.fullscreenElement };
      });
      summary.actions.push({ ...a, at: field, canvas: canvasBox });
      console.log(`action ${a.kind}${a.arg ? ` ${a.arg}` : ''} at field ${field}: canvas ${JSON.stringify(canvasBox)}`);
    }
    if (hide && !hidden && field >= hide[0]) {
      const before = (await mvp()).stats?.status.fields ?? 0;
      await page.evaluate(setHidden, true);
      const t = Date.now();
      await page.waitForTimeout(hide[1]);
      const mid = await mvp();
      await page.evaluate(setHidden, false);
      hidden = { atField: before, ms: Date.now() - t, fieldsWhileHidden: (mid.stats?.status.fields ?? 0) - before,
                 paused: mid.paused, audio: mid.audio?.state };
      console.log(`hidden for ${hidden.ms} ms at field ${before}: ${hidden.fieldsWhileHidden} fields ran meanwhile`);
    }
    while (grabIndex < grabs.length && field >= grabs[grabIndex][0]) {  // V3
      const [at, n] = grabs[grabIndex++];
      const pics = await page.evaluate((k) => window.mvp.grab ? window.mvp.grab(k) : null, n);
      if (!pics) {
        console.log(`grab at field ${field}: no pictures (not the threaded engine with ImageBitmaps)`);
        continue;
      }
      summary.grabs ??= [];
      summary.grabs.push({ at, field, kinds: pics.map((p) => (p.between ? 'between' : 'frame')) });
      pics.forEach((p, k) => {
        const path = join(out, `${label}-r${run}-grab-${at}-${String(k).padStart(3, '0')}-${p.between ? 'between' : 'frame'}.png`);
        writeFileSync(path, Buffer.from(p.url.slice(p.url.indexOf(',') + 1), 'base64'));
      });
      console.log(`grab at field ${field}: ${pics.map((p) => (p.between ? 'B' : 'F')).join('')}`);
    }
    while (shotIndex < shots.length && field >= shots[shotIndex]) {
      const path = join(out, `${label}-r${run}-field-${String(field).padStart(5, '0')}.png`);
      await page.locator('#screen').screenshot({ path });
      console.log(`screenshot at field ${field}: ${path}`);
      ++shotIndex;
    }
    const wantStop = fields || frames;
    if (wantStop ? state.stopped
                 : (state.firstFrameMs !== undefined && shotIndex >= shots.length && grabIndex >= grabs.length && keyIndex >= keys.length &&
                    padIndex >= pads.length && actionIndex >= actions.length && (!hide || hidden) && (!audio || audioSamples.nonsilent > 0) &&
                    (!screencast || summary.screencast) && (!traceAt || summary.trace))) break;
    await page.waitForTimeout(200);
  }
  if (cast && !summary.screencast) summary.screencast = castFrames;
  if (tracing && !summary.trace) await stopTrace();
  if (state.discTrace) writeFileSync(join(out, `disc-trace-${label}-r${run}.txt`), state.discTrace);
  delete state.discTrace;
  if (state.digest) writeFileSync(join(out, `digest-${label}-r${run}.txt`), state.digest);
  delete state.digest;
  if (state.ioLog) {
    writeFileSync(join(out, `io-${label}-r${run}.txt`), state.ioLog);
    summary.io = summarizeIo(state.ioLog, state.firstFrameField ?? 0);
  }
  delete state.ioLog;
  if (state.stopped || state.firstFrameMs !== undefined) {
    const path = join(out, `${label}-r${run}-final.png`);
    if (compareRef) await page.waitForTimeout(500);  // Q7: the stop field's picture reaches the canvas
    const shot = await page.locator('#screen').screenshot({ path }).catch(() => null);
    if (compareRef && shot) {  // Q7
      summary.compare = await comparePictures(page, shot, readFileSync(compareRef), compareRegion);
      summary.compare.reference = compareRef;
      console.log(`compare with ${compareRef}: region mean ${summary.compare.region.toFixed(2)}, worst cell ` +
                  `${summary.compare.worst.diff.toFixed(1)} at ${summary.compare.worst.x},${summary.compare.worst.y}, ` +
                  `whole picture ${summary.compare.whole.toFixed(2)} (limit ${compareMax})`);
    }
  }
  Object.assign(summary, {
    source: state.source, phases: state.phases, translate: state.translate, disc: state.disc, gfx: state.gfx,
    firstFrameMs: state.firstFrameMs, firstFrameField: state.firstFrameField, cache: state.cache,
    windows, final: state.stopped?.status ?? state.stats?.status ?? null, guestLine: state.stopped?.guestLine ?? null,
    packs: state.packs,
    error: state.error ?? null, consoleErrors: consoleErrors.slice(0, 20), hidden,
    hint: state.hint, autoScale: state.autoScale, pictureLog: state.log, stateLog: state.stateLog,  // save states  // Q5; and a gameplay mod review's timing
    card: state.card, cardLog: state.cardLog,  // the memory card (card.ts)
    audio: { page: state.audio, attachedAtField: state.audioAttachedAtField, samples: audioSamples },
  });
  const origin = new URL(base).origin;
  summary.foreignRequests = requests.filter((u) => !u.startsWith(origin) && !u.startsWith('blob:') && !u.startsWith('data:'));
  summary.requests = [...new Set(requests.map((u) => u.replace(origin, '').replace(/\?.*$/, '?...')))];
  if (serverDisc && !realServer) summary.mockServer = await (await fetch(`${base}_stats`)).json();
  await page.close();
  const ok = state.firstFrameMs !== undefined || (!render && state.stopped);
  summary.ok = ok && !state.error && (!(fields || frames) || !!state.stopped) &&
               (!compareRef || (!!summary.compare && summary.compare.region <= compareMax));
  return summary;
}

// Q7 (--compare): this run's final picture against a reference PNG of the same size (another run's
// final screenshot, e.g. the same script at a fixed scale), decoded and compared in the page. The
// region is in the console's pixels (the 640 x 448 frame fills the 4:3 canvas). Returns the mean
// absolute difference per channel (0-255) over the region and over the whole picture, and the
// region's worst 16 x 16 cell.
async function comparePictures(page, shot, reference, region) {
  return page.evaluate(async ([a64, b64, r]) => {
    const decode = async (b64s) => {
      const bytes = Uint8Array.from(atob(b64s), (ch) => ch.charCodeAt(0));
      return createImageBitmap(new Blob([bytes], { type: 'image/png' }));
    };
    const [a, b] = await Promise.all([decode(a64), decode(b64)]);
    const w = a.width, h = a.height;
    const pixels = (bm) => {
      const c = new OffscreenCanvas(w, h);
      const g = c.getContext('2d');
      g.drawImage(bm, 0, 0, w, h);  // the reference at this picture's size
      return g.getImageData(0, 0, w, h).data;
    };
    const pa = pixels(a), pb = pixels(b);
    const diff = (x0, y0, x1, y1) => {
      let sum = 0, n = 0;
      for (let y = y0; y < y1; ++y)
        for (let x = x0; x < x1; ++x) {
          const i = (y * w + x) * 4;
          sum += Math.abs(pa[i] - pb[i]) + Math.abs(pa[i + 1] - pb[i + 1]) + Math.abs(pa[i + 2] - pb[i + 2]);
          n += 3;
        }
      return n ? sum / n : 0;
    };
    const sx = w / 640, sy = h / 448;
    const px = (cx, cy) => [Math.round(cx * sx), Math.round(cy * sy)];
    const [x0, y0] = px(r[0], r[1]), [x1, y1] = px(r[2], r[3]);
    let worst = { diff: 0, x: r[0], y: r[1] };
    for (let cy = r[1]; cy < r[3]; cy += 16)
      for (let cx = r[0]; cx < r[2]; cx += 16) {
        const [ax, ay] = px(cx, cy), [bx, by] = px(Math.min(cx + 16, r[2]), Math.min(cy + 16, r[3]));
        const d = diff(ax, ay, bx, by);
        if (d > worst.diff) worst = { diff: d, x: cx, y: cy };
      }
    return { size: [w, h], referenceSize: [b.width, b.height], region: diff(x0, y0, x1, y1), whole: diff(0, 0, w, h),
             worst, regionConsole: r };
  }, [Buffer.from(shot).toString('base64'), Buffer.from(reference).toString('base64'), region]);
}

// --load N: busy processes for the whole run (killed at the end).
const loaders = [];
for (let i = 0; i < loadProcs; ++i)
  loaders.push(spawn(process.execPath, ['-e', 'for(;;){}'], { stdio: 'ignore' }));
const all = { browser: browserName, tag: tag || null, jsFlags: jsFlags || null, headed, fast, fields, frames,
              script: scriptFile || null, serverDisc, login, latency: latency || null, rate: rate || null, runs: [] };
let rc = 1;
let browser;
try {
  // --viewport WxH (default 1100x1100): the page's size, e.g. a full-screen window on this machine's display.
  const [vw, vh] = opt('--viewport', '1100x1100').split('x').map(Number);
  let context;
  if (profile) {
    // A profile on disk: what the browser keeps between visits, with its real storage quota.
    if (!['chrome', 'msedge', 'chromium'].includes(browserName)) throw new Error('--profile: Chromium browsers only');
    context = await chromium.launchPersistentContext(resolve(profile), {
      channel: browserName === 'chromium' ? undefined : browserName, headless: !headed, args: chromiumArgs,
      viewport: { width: vw, height: vh } });
    browser = { close: () => context.close() };
    all.version = context.browser()?.version() ?? null;
  } else {
    browser = await launchers[browserName]();
    all.version = browser.version();
    // --dpr N (Q5): the display's device pixel ratio (a high-DPI laptop screen: Auto picks a higher scale)
    context = await browser.newContext({ viewport: { width: vw, height: vh }, deviceScaleFactor: Number(opt('--dpr', '1')) });
  }
  for (let run = 1; run <= runs; ++run) {
    const s = await runOnce(context, run);
    all.runs.push(s);
    console.log(`run ${run}: first frame ${s.firstFrameMs?.toFixed(0)} ms, cache ${JSON.stringify(s.cache)}, ` +
                `guest line ${s.guestLine}`);
  }
  rc = all.runs.every((s) => s.ok) ? 0 : 1;
} catch (e) {
  all.exception = String(e?.stack ?? e);
} finally {
  await browser?.close();
  server.kill();
  for (const l of loaders) l.kill();
}
writeFileSync(join(out, `summary-${label}.json`), JSON.stringify(all, null, 2));
console.log(JSON.stringify(all, (k, v) => (k === 'windows' || k === 'requests' ? undefined : v), 2));
process.exit(rc);
