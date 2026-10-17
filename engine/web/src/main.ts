// The page (E2, E2c): where the disc comes from, the canvas, input, sound, and
// what the engine Worker reports.
//
// The disc: on load the page asks for /config.json (disc.ts). On the owner's
// private deployment it names the server's image and the game starts without a
// picker: the first visit downloads the image once into this browser's storage
// (disc-store.ts, P1) and every start reads it from there, locally; a server that
// has no disc yet gets a note that its operator must upload one. Anywhere else
// the player picks their own image, which is handed to the Worker as a File (a
// local handle, never uploaded). The page itself holds no engine state.
//
// URL parameters (development and the local browser check, tools/browser_check.mjs):
//   script=TEXT     an mvp-input script instead of live input (newlines as %0A)
//   fields=N        stop after N VI fields; frames=N: at the poll of frame N
//   fast=1          run unpaced, as fast as possible
//   render=0        no WebGPU at all
//   audio=0         never mix audio (the guest is the same either way)
//   cache=0         neither use nor fill the browser's cache of the translation
//   audioLatency=MS the worklet's buffer target (default 60)
//   discBlock=KIB, discBlocks=N, discRun=N, discAhead=N   the server image's block cache
//   traceDisc=1     record the picked image's reads (offsets and sizes) for tuning that cache
//   ioLog=1         log every guest DVD read with its host timing (P1 measurements)
//   inputLog=1      at the stop, report the inputs the guest consumed as an mvp-input script (Q3 tests)
//   store=0         read the server's image over the network instead of keeping it in this browser
//   storeSpare=MB   free space the download must leave (tests of the storage-quota error)
//   threads=0       the single-threaded engine even where the page is cross-origin isolated (P2)
//   bitmaps=0       threaded: the render thread draws into the page's canvas instead of sending ImageBitmaps
//   digest=1        a renderer digest per VI field (window.mvp.digest; digestDetail=F1-F2: per draw)
//   rtTune=US,MS,KIB,SYNC,COPIES   the render thread's waiting (fence poll, record poll, publish step, sync fences,
//                           copies of what commands read instead of write waits)
//   slowMotion=1    a machine that cannot keep up runs the game evenly in slow motion, every frame shown (Q2)
//   audioRing=0     the sound goes to the worklet as messages even where the page could share memory (Q2)
//   hostSlow=N, hostSlowFrom=F   a machine N times slower, simulated from VI field F on (Q2: the engine's
//                   threads spin N - 1 times their working time; host timing only)
//   gpview=0        the renderer reads guest memory as it is, without the data-cache model (Q1 comparisons)
//   scale=1|2|3     the internal resolution, instead of the Picture setting (Q1)
//   gpuLoad=N       development (Q5): extra GPU work per present, in proportion to the picture's pixels
//                   (N passes), to try the Picture setting's Auto on a GPU that cannot keep up
//   gpuTimer=0      development (Q5): the render passes are not timed on the GPU (Auto then goes by size alone)
//   paceMargin=X    development (Q5): the online pace's margin over this machine's capacity (default 0.98)
//   paceTune=K:V,.. development (Q5b): the pacer's options (pacer.ts) and the paced loop's (worker.ts)
//   stats=1         the statistics overlay shown from the start (Q5; otherwise the Stats button or `)
// The page publishes its state as `window.mvp` for the browser check.

import { AudioOut, type AudioState } from './audio.js';
import { AutoScale } from './autoscale.js';
import { findServerDisc } from './disc.js';
import { DownloadError, canStore, downloadDisc, progressText, storedDisc, type DiscOffer } from './disc-store.js';
import { ControlsPanel } from './controls.js';
import { HostInput } from './input.js';
import { Online, type OnlineState } from './netplay.js';
import {
  StatsTracker, copyText, describeMachine, overlayNotes, overlayRows, parsePeerStats, type OverlayContext,
  type StatsSummary,
} from './netstats.js';
import { PackPanel, type PacksState } from './packs.js';
import type { CacheReport, DiscSource, FromWorker, NetStatus, Pad, PaceState, RunOptions, Status, ToWorker } from './protocol.js';

// The server's image in this browser's storage (P1): found there, or downloaded now.
interface DownloadState {
  state: 'checking' | 'waiting' | 'downloading' | 'stored' | 'done' | 'error';
  done: number;
  total: number;
  bytesPerS: number;
  ms?: number;           // the download's duration
  fetched?: number;      // bytes this download fetched
  resumedFrom?: number;  // bytes already stored when it started
  kind?: string;         // the error's kind (DownloadError)
  error?: string;
}

interface PageState {
  source?: 'file' | 'url' | 'stored';
  download?: DownloadState;
  startedAt: number;           // performance.now() when the disc was chosen (picked, or named by the server)
  phases: Record<string, number>;
  translate?: unknown;
  disc?: unknown;
  gfx?: { ok: boolean; message: string };
  firstFrameMs?: number;       // from the start to the first frame on the canvas
  firstFrameField?: number;
  stats?: { status: Status; window: { fields: number; ms: number; wallMs: number } };
  stopped?: { status: Status; guestLine: string };
  discTrace?: string;
  inputLog?: string;           // the inputs the guest consumed, as an mvp-input script (inputLog=1, Q3)
  ioLog?: string;              // every guest DVD read: "field at_ms offset size ms" lines (ioLog=1)
  digest?: string;             // P2: the renderer digest, "field digest draws" lines (digest=1)
  error?: { stage: string; message: string };
  progress?: { text: string; step: number; steps: number };
  cache?: CacheReport;
  audio?: AudioState;
  audioAttachedAtField?: number;
  paused: boolean;
  pad?: Pad;                   // port 0 (player 1, or the online player)
  pads?: Pad[];                // every port the page drives, as last sent (Q3)
  controls?: () => Record<string, unknown>;  // the Controls panel's state (Q3)
  log: string[];
  audioProbe: () => { rms: number; peak: number } | null;
  net?: OnlineState;  // online play (E5, netplay.ts)
  packs?: PacksState; // roster packs (E6, packs.ts)
  pace?: PaceState;   // Q5: the pace (pacer.ts), as the Worker reports it
  overlay?: { shown: boolean; local: StatsSummary; peer: StatsSummary | null; text: string };  // Q5 (netstats.ts)
  hint?: { shown: boolean; text: string; dismissed: boolean };  // Q5: the note for a machine that cannot keep up
  autoScale?: { ceiling: number; steps: unknown[]; lastBusy: number | null };  // Q5: the Picture's Auto and the GPU
}

const $ = <T extends HTMLElement>(id: string) => document.getElementById(id) as T;
const picker = $<HTMLInputElement>('disc');
const pickerBox = $<HTMLElement>('picker');
const stage = $<HTMLElement>('stage');
const canvas = $<HTMLCanvasElement>('screen');
const statusEl = $<HTMLElement>('status');
const progressEl = $<HTMLProgressElement>('progress');
const hintEl = $<HTMLElement>('hint');
const phasesEl = $<HTMLElement>('phases');
const statsEl = $<HTMLElement>('stats');
const logEl = $<HTMLElement>('log');
const errorEl = $<HTMLElement>('error');
const fullscreenBtn = $<HTMLButtonElement>('fullscreen');
const soundBtn = $<HTMLButtonElement>('sound');
const pauseBtn = $<HTMLButtonElement>('pause');
const pictureSel = document.getElementById('picture') as HTMLSelectElement | null;
const startBox = $<HTMLElement>('start-box');
const streamBox = $<HTMLElement>('stream-box');
const playBtn = $<HTMLButtonElement>('play');

const params = new URLSearchParams(location.search);
const options: RunOptions = {
  script: params.get('script') ?? '',
  stopFields: Number(params.get('fields') ?? 0) || 0,
  stopFrames: Number(params.get('frames') ?? 0) || 0,
  fast: params.get('fast') === '1',
  render: params.get('render') !== '0',
  audio: params.get('audio') !== '0',
  cache: params.get('cache') !== '0',
  traceDisc: params.get('traceDisc') === '1',
  tracePresent: params.get('tracePresent') === '1',
  fastUntil: Number(params.get('fastUntil') ?? 0) || 0,
  pacing: params.get('pacing') === 'raf' ? 'raf' : 'chase',
  slowMotion: params.get('slowMotion') === '1',
  ioLog: params.get('ioLog') === '1',
  inputLog: params.get('inputLog') === '1',
  threads: params.get('threads') !== '0',
  digest: params.get('digest') === '1',
  digestDetail: params.has('digestDetail') ? (params.get('digestDetail') ?? '').split('-').map(Number) : undefined,
  rtTune: params.has('rtTune') ? (params.get('rtTune') ?? '').split(',').map(Number) : undefined,
  audioRing: params.get('audioRing') !== '0',
  hostSlow: Number(params.get('hostSlow') ?? 0) || undefined,
  hostSlowFrom: Number(params.get('hostSlowFrom') ?? 0) || 0,
  gpView: params.get('gpview') !== '0',
  gpuLoad: Number(params.get('gpuLoad') ?? 0) || undefined,
  gpuTimer: params.get('gpuTimer') !== '0',
  paceMargin: Number(params.get('paceMargin') ?? 0) || undefined,
  paceTune: params.get('paceTune') ?? undefined,
  discCache: {
    blockKib: Number(params.get('discBlock') ?? 0) || undefined, maxBlocks: Number(params.get('discBlocks') ?? 0) || undefined,
    maxRun: Number(params.get('discRun') ?? 0) || undefined,
    ahead: params.has('discAhead') ? Number(params.get('discAhead')) : undefined,
  },
};
const audioLatencyMs = Math.min(500, Math.max(20, Number(params.get('audioLatency') ?? 60) || 60));

let audio: AudioOut | null = null;
const state: PageState = {
  startedAt: 0, phases: {}, paused: false, log: [],
  audioProbe: () => audio?.probe() ?? null,
};
(window as unknown as { mvp: PageState }).mvp = state;

// Roster packs (E6): picked before the game starts, applied by the engine to the disc.
const packsEl = document.getElementById('packs');
const packPanel = packsEl ? new PackPanel(packsEl) : null;
if (packPanel) state.packs = packPanel.state;

// Online play (E5): the lobby panel, when the page comes from the netplay server.
const onlineEl = document.getElementById('online');
const online = onlineEl ? new Online(onlineEl, params) : null;
if (online) state.net = online.state;
// The lobby opens once the game can start (after the one-time download): a game
// joined earlier would make the other player wait past the session's timeout.
let onlineReady: Promise<void> | null = null;
const initOnline = (): Promise<void> => (onlineReady ??= online ? online.init().catch(() => {}) : Promise.resolve());
const storeDisc = params.get('store') !== '0';
const storeSpare = params.has('storeSpare') ? Number(params.get('storeSpare')) * 1048576 : undefined;

const hasWebGPU = 'gpu' in navigator;
const hasOffscreen = typeof canvas.transferControlToOffscreen === 'function';

function setStatus(text: string): void {
  statusEl.textContent = text;
}

function log(text: string, error = false): void {
  state.log.push(text);
  if (state.log.length > 2000) state.log.splice(0, state.log.length - 2000);
  const line = document.createElement('div');
  line.textContent = text;
  if (error) line.className = 'err';
  logEl.append(line);
  while (logEl.childElementCount > 500) logEl.firstElementChild?.remove();
}

const NO_WEBGPU = 'This browser has no WebGPU, so the game cannot be drawn. Use a current Chrome or Edge ' +
                  '(113 or newer), Firefox 141 or newer on Windows, or Safari 26 or newer, with hardware ' +
                  'acceleration on.';

function showError(stage: string, message: string): void {
  state.error = { stage, message };
  const reason =
    stage === 'disc' ? `This image cannot be used. The engine runs MVP Baseball 2005 for GameCube, USA (GV4E69), ` +
                       `revision 0 only. (${message})`
    : stage === 'server' || stage === 'download' ? message
    : stage === 'no-disc' ? 'This server has no game disc yet. Only the site\'s operator can add it, on the Server disc ' +
                            'page (the link at the bottom of this page). Reload this page afterwards.'
    : stage === 'webgpu' ? message
    : stage === 'packs' ? `The roster packs cannot be used with this disc, so the game did not start: ${message}. ` +
                          'Reload the page to change the packs.'
    : stage === 'netplay' ? `The online game did not start: ${message.replace(/^cannot play: /, '')}`
    : stage === 'gfx' ? `WebGPU is present, but the game cannot draw: ${message}. Check that hardware acceleration ` +
                        `is on, or try another browser.`
    : `The engine stopped (${stage}): ${message}`;
  errorEl.textContent = reason;
  errorEl.hidden = false;
  progressEl.hidden = true;
  if (stage !== 'gfx') setStatus(stage === 'no-disc' ? '' : 'Stopped.');
}

function showPhases(): void {
  const names: Record<string, string> = {
    engine: 'engine module', gpu: 'WebGPU device', disc: 'disc check (DOL SHA-1)', 'symbols-fetch': 'symbols.txt',
    translate: 'translate + compile', boot: 'boot', 'gpu-wait': 'wait for WebGPU',
  };
  const rows = Object.entries(state.phases).map(([k, v]) => `${names[k] ?? k}: ${v.toFixed(0)} ms`);
  const t = state.translate as Record<string, number> | undefined;
  if (t) rows.push(`  (${t.cached ? 'from this browser\'s cache: unpack' : `CFG ${t.cfg_ms.toFixed(0)} ms, emit`} ` +
                   `${t.emit_ms.toFixed(0)} ms, compile ${t.compile_ms.toFixed(0)} ms, ` +
                   `instantiate ${t.instantiate_ms.toFixed(0)} ms, module ${(t.module_bytes / 1048576).toFixed(1)} MB)`);
  if (state.cache?.storedMs !== undefined) rows.push(`translation cached in this browser (${state.cache.storedMs.toFixed(0)} ms)`);
  if (state.firstFrameMs !== undefined) rows.push(`first frame: ${state.firstFrameMs.toFixed(0)} ms after the start`);
  phasesEl.textContent = rows.join('\n');
}

function showSound(s: AudioState): void {
  state.audio = s;
  const labels: Record<AudioState['state'], string> = {
    off: 'Sound: off', 'waiting-for-gesture': 'Sound: off', starting: 'Sound: starting', running: 'Sound: on',
    suspended: 'Sound: paused', failed: 'Sound: failed', muted: 'Sound: muted',
  };
  soundBtn.textContent = labels[s.state];
  soundBtn.title = s.error ?? 'Turn the sound on or off (M)';
  updateHint();
}

function updateHint(): void {
  const waiting = options.audio && (!state.audio || state.audio.state === 'waiting-for-gesture');
  hintEl.hidden = !(worker && waiting && !state.error);
  hintEl.textContent = 'Click the game or press any key to turn on the sound.';
}

let worker: Worker | null = null;
let pendingAudioPort: MessagePort | null = null;
// Controllers and keyboard (Q3): ports 0 and 1 are players 1 and 2 (online: only this page's player, port 0).
const input = new HostInput(window, (pad: Pad, port: number) => {
  if (port === 0) state.pad = pad;
  (state.pads ??= [])[port] = pad;
  send({ type: 'pad', port, pad });
});
const controls = new ControlsPanel($<HTMLElement>('controls'), input, {
  chip: document.getElementById('pad-chip'), details: document.getElementById('controls-details') as HTMLDetailsElement | null,
  summary: document.getElementById('controls-summary'),
});
state.controls = () => controls.snapshot();
if (online) {
  online.onChange = () => controls.setOnline(online.state.player ?? null);
  controls.onChange = () => online.setController(controls.localController());
}
document.getElementById('pad-chip')?.addEventListener('click', () => {
  const d = document.getElementById('controls-details') as HTMLDetailsElement | null;
  if (d) {
    d.open = true;
    d.scrollIntoView({ block: 'nearest' });
  }
});

function send(msg: ToWorker, transfer: Transferable[] = []): void {
  worker?.postMessage(msg, transfer);
}

if (options.audio) {
  audio = new AudioOut((port) => {
    if (worker) send({ type: 'audio-port', port }, [port]);
    else pendingAudioPort = port;
  }, audioLatencyMs, showSound);
  showSound(audio.info);
} else {
  soundBtn.disabled = true;
  showSound({ state: 'off' });
}

// The first user gesture starts audio (the browsers' autoplay policy). Later
// gestures resume a context the browser suspended.
const gesture = () => audio?.unlock();
for (const type of ['pointerdown', 'keydown', 'touchend'])
  window.addEventListener(type, gesture, { capture: true });

function onWorker(msg: FromWorker): void {
  switch (msg.type) {
    case 'log':
      log(msg.text, msg.error);
      break;
    case 'progress':
      state.progress = { text: msg.text, step: msg.step, steps: msg.steps };
      setStatus(msg.steps ? `${msg.text} (${msg.step}/${msg.steps})...` : `${msg.text}...`);
      progressEl.hidden = false;
      if (msg.steps) progressEl.value = msg.step / msg.steps;
      else progressEl.removeAttribute('value');
      break;
    case 'phase':
      state.phases[msg.name] = msg.ms;
      if (msg.name === 'translate') state.translate = msg.detail;
      showPhases();
      break;
    case 'packs':
      packPanel?.showMounted(msg.report);
      log(`roster packs: set ${msg.report.set_hash}` + (msg.report.ok
        ? `, result ${msg.report.result_hash ?? ''} (${(msg.report.files ?? []).join(', ') || 'nothing patched'})`
        : ', not applied'), !msg.report.ok);
      break;
    case 'disc':
      state.disc = msg.info;
      log(`disc: ${msg.info.title} (${msg.info.game} rev ${msg.info.revision}, ${msg.info.image}), main.dol ${msg.info.dol_sha1}`);
      break;
    case 'cache':
      state.cache = { ...state.cache, ...msg.report };
      if (msg.report.error) log(`translation cache: ${msg.report.error}`, true);
      showPhases();
      break;
    case 'audio':
      state.audioAttachedAtField = msg.attachedAtField;
      break;
    case 'gfx':
      state.gfx = { ok: msg.ok, message: msg.message };
      log(`WebGPU: ${msg.ok ? 'ready on ' : ''}${msg.message}`, !msg.ok);
      if (!msg.ok && options.render) showError('gfx', msg.message);
      break;
    case 'first-frame':
      state.firstFrameMs = performance.now() - state.startedAt;
      state.firstFrameField = msg.fields;
      progressEl.hidden = true;
      setStatus('Running. Enter = START, X = A, Z = B, arrows = D-pad; or a gamepad. F = fullscreen, M = sound, P = pause.');
      showPhases();
      updateHint();
      break;
    case 'stats': {
      if (msg.window.digest) state.digest = (state.digest ?? '') + msg.window.digest;
      const { digest: _d, ...win } = msg.window;
      state.stats = { status: msg.status, window: win };
      onStats(msg.status, msg.window);
      const s = msg.status, w = msg.window;
      const perField = w.fields ? w.ms / w.fields : 0;
      const a = state.audio?.worklet;
      statsEl.textContent =
        `field ${s.fields}, frame ${s.frames}, guest ${s.guest_s.toFixed(1)} s | ` +
        `${w.fields} fields in ${(w.wallMs / 1000).toFixed(2)} s (${(w.fields / (w.wallMs / 1000)).toFixed(1)}/s), ` +
        `${perField.toFixed(2)} ms per field | draws ${s.draws ?? 0}, GPU errors ${s.gpu_errors ?? 0}` +
        (s.scale ? `, picture ${s.scale}x` : '') +
        (a ? ` | sound ${(a.fill / a.inRate * 1000).toFixed(0)} ms buffered, ${a.underruns} underruns` : '') +
        (s.net ? ` | server reads ${s.net.requests} (+${s.net.prefetched} ahead), waited ${s.net.blocked_ms.toFixed(0)} ms` : '') +
        (s.io ? ` | disc reads ${s.io.reads}, ${s.io.ms.toFixed(0)} ms in all, longest ${s.io.max_ms.toFixed(0)} ms` : '');
      break;
    }
    case 'stopped':
      state.stopped = { status: msg.status, guestLine: msg.guestLine };
      state.discTrace = msg.discTrace;
      state.ioLog = msg.ioLog;
      state.inputLog = msg.inputLog;
      setStatus(`Stopped: ${msg.status.why ?? ''}`);
      log(msg.guestLine);
      break;
    case 'error':
      showError(msg.stage, msg.message);
      break;
    case 'net-send':
      online?.send(msg.data);
      break;
    case 'net':
      netStatus = msg.status;
      online?.engineStatus(msg.status, msg.windows, msg.diagnostic);
      break;
    case 'pace':  // Q5: for the other player, and the overlay
      state.pace = msg.state;
      online?.sendPace(msg.report, msg.state);
      break;
  }
}

function startEngine(disc: DiscSource): void {
  if (worker) return;
  const stored = disc.kind === 'file' && !!disc.stored;
  state.source = stored ? 'stored' : disc.kind;
  state.startedAt = performance.now();
  errorEl.hidden = true;
  state.error = undefined;  // e.g. a download error, when the player chose to play without keeping the data
  streamBox.hidden = true;
  picker.disabled = true;
  pickerBox.hidden = disc.kind === 'url' || stored;
  startBox.hidden = true;
  setStatus(stored ? 'Starting the game...' : disc.kind === 'file' ? `Reading ${disc.file.name}...`
                                                                   : 'Loading the disc from the server...');
  worker = new Worker(new URL('./worker.js', import.meta.url), { type: 'module' });
  worker.onmessage = (ev: MessageEvent<FromWorker>) => onWorker(ev.data);
  worker.onerror = (ev) => showError('worker', ev.message || 'the engine Worker failed to start (out of memory?)');
  const dpr = Math.min(window.devicePixelRatio || 1, 2);
  const width = Math.round(canvas.clientWidth * dpr), height = Math.round(canvas.clientHeight * dpr);
  let off: OffscreenCanvas | null = null;
  let frames: MessagePort | undefined;
  if (vsync && options.render && options.threads !== false && params.get('bitmaps') !== '0') {
    // P2, cross-origin isolated (the threaded engine): the page keeps the canvas
    // and shows each frame the render thread sends as an ImageBitmap. A Worker
    // started by a Worker cannot keep an OffscreenCanvas of the page on screen
    // steadily (its frames reach the screen only between its tasks, and the
    // render thread waits inside long tasks for the emulation); the page's main
    // thread is idle and shows each bitmap at the next refresh.
    canvas.width = width;
    canvas.height = height;
    const ctx = canvas.getContext('bitmaprenderer');
    if (ctx) {
      const chan = new MessageChannel();
      chan.port1.onmessage = (ev: MessageEvent<ImageBitmap>) => ctx.transferFromImageBitmap(ev.data);
      frames = chan.port2;
    }
  } else if (hasOffscreen && options.render) {
    canvas.width = width;
    canvas.height = height;
    off = canvas.transferControlToOffscreen();
  }
  // In an online game (E5) the Worker boots only after the handshake with the other player.
  const netplay = online?.netOptions();
  // The roster packs (E6), in the panel's order; the set can no longer change.
  const packs = packPanel?.current() ?? [];
  packPanel?.lock();
  const transfer: Transferable[] = packs.flatMap((p) => p.files.map((f) => f.data));
  if (off) transfer.unshift(off);
  if (frames) transfer.unshift(frames);
  options.scale = pictureScale();
  sentScale = options.scale;
  send({ type: 'start', disc, canvas: off, width, height, options: { ...options, netplay, packs },
         vsync: vsync ? (vsync.buffer as SharedArrayBuffer) : undefined, frames }, transfer);
  if (pendingAudioPort) {
    send({ type: 'audio-port', port: pendingAudioPort }, [pendingAudioPort]);
    pendingAudioPort = null;
  }
  if (online && netplay) {
    online.onDatagram = (data) => send({ type: 'net-recv', data }, [data]);
    online.onPace = (report) => send({ type: 'pace-peer', report });  // Q5
    online.whenOpen().then((delay) => send({ type: 'net-open', delay })).catch(() => {});
    window.addEventListener('pagehide', () => send({ type: 'net-quit' }));
  }
  input.poll();
  new ResizeObserver(() => {
    const w = Math.round(canvas.clientWidth * dpr), h = Math.round(canvas.clientHeight * dpr);
    if (w > 0 && h > 0) send({ type: 'resize', width: w, height: h });
    applyPicture();  // Auto follows the picture's size (fullscreen)
  }).observe(canvas);
  updateHint();
}

picker.addEventListener('change', () => {
  const file = picker.files?.[0];
  if (file) startEngine({ kind: 'file', file });
});

// ---- the picture (Q1): the internal resolution ----
//
// The renderer draws at 1x, 2x or 3x the console's 640 x 448 pixels and scales
// the result to the canvas (an area filter when larger, sharp bilinear when
// smaller). Auto draws at least as many pixels as the canvas shows, up to 3x;
// a software adapter stays at 1x. The choice is kept in this browser.
const PICTURE_KEY = 'mvp.picture.v1';
let sentScale = 0;
const autoScale = new AutoScale();  // Q5: Auto's ceiling from the GPU's time
let softwareAdapter = false;
void (async () => {
  try {
    const gpu = (navigator as unknown as { gpu?: { requestAdapter(o?: object): Promise<unknown> } }).gpu;
    const a = await gpu?.requestAdapter({ powerPreference: 'high-performance' });
    const info = (a as unknown as { info?: { isFallbackAdapter?: boolean } } | null)?.info;
    softwareAdapter = !!(info?.isFallbackAdapter ?? (a as unknown as { isFallbackAdapter?: boolean } | null)?.isFallbackAdapter);
    if (softwareAdapter) applyPicture();
  } catch {
    // no WebGPU: nothing to choose
  }
})();
function pictureSetting(): string {
  const fromUrl = params.get('scale');
  if (fromUrl && ['1', '2', '3'].includes(fromUrl)) return fromUrl;
  try {
    const v = window.localStorage.getItem(PICTURE_KEY);
    if (v && ['auto', '1', '2', '3'].includes(v)) return v;
  } catch {
    // storage blocked: the default
  }
  return 'auto';
}
function pictureScale(): number {
  const v = pictureSetting();
  if (v !== 'auto') return Number(v);
  if (softwareAdapter) return 1;
  return Math.min(autoScale.ceiling, sizeScale());  // Q5: no higher than the GPU keeps up with
}
// Q1: Auto by the picture's size alone: at least as many pixels as the canvas shows, up to 3x.
function sizeScale(): number {
  const dprNow = Math.min(window.devicePixelRatio || 1, 2);
  const h = canvas.clientHeight * dprNow;  // the picture's height on screen, in device pixels
  return Math.min(3, Math.max(1, Math.ceil(h / 448 - 0.1)));
}
function applyPicture(): void {
  const s = pictureScale();
  if (!worker || s === sentScale) return;
  sentScale = s;
  send({ type: 'scale', scale: s });
}
if (pictureSel) {
  pictureSel.value = pictureSetting();
  if (params.get('scale')) pictureSel.disabled = true;
  pictureSel.addEventListener('change', () => {
    try {
      window.localStorage.setItem(PICTURE_KEY, pictureSel.value);
    } catch {
      // storage blocked: this visit only
    }
    applyPicture();
    canvas.focus();
  });
}

// ---- Q5: the statistics overlay, the note for a slow machine, Auto and the GPU ----
//
// netstats.ts measures this side from the Worker's reports; online, the summary goes to the other player
// once a second (a text message on the data channel) and theirs comes back. The overlay shows both, live,
// over the picture (the Stats button or the ` key; off by default, the choice kept in this browser), and
// Copy stats puts the same as text on the clipboard for a chat. Display only.
const OVERLAY_KEY = 'mvp.overlay.v1';
const overlayEl = $<HTMLElement>('overlay');
const overlayBtn = $<HTMLButtonElement>('stats-toggle');
const tracker = new StatsTracker();
let netStatus: NetStatus | null = null;
let peerStats: StatsSummary | null = null, peerStatsAt = 0;
let lastStatsSent = 0;
let gpuName: string | null = null;
const isolated = (self as unknown as { crossOriginIsolated?: boolean }).crossOriginIsolated === true;

function overlayWanted(): boolean {
  if (params.get('stats') === '1') return true;
  try {
    return window.localStorage.getItem(OVERLAY_KEY) === '1';
  } catch {
    return false;
  }
}
function showOverlay(on: boolean): void {
  overlayEl.hidden = !on;
  overlayBtn.setAttribute('aria-pressed', on ? 'true' : 'false');
  if (on) renderOverlay();
}
function toggleOverlay(): void {
  const on = overlayEl.hidden !== false;
  try {
    window.localStorage.setItem(OVERLAY_KEY, on ? '1' : '0');
  } catch {
    // storage blocked: this visit only
  }
  showOverlay(on);
}
overlayBtn.addEventListener('click', toggleOverlay);
showOverlay(overlayWanted());

function overlayContext(): OverlayContext {
  const local = tracker.summary(describeMachine(gpuName));
  const n = netStatus;
  return {
    online: !!online?.state.player, player: online?.state.player ?? null, local, peer: peerStats,
    peerAgeMs: peerStats ? performance.now() - peerStatsAt : null,
    rttMs: n ? (n.rttRecent ?? n.rttAvg) : null, jitterMs: n?.rttJitter ?? null, delay: n ? n.delay : null,
    pace: state.pace ?? null, isolated,
  };
}

function renderOverlay(): void {
  const c = overlayContext();
  const table = $<HTMLTableElement>('overlay-table');
  const head = c.online ? ['', `You (P${c.player})`, `P${c.player === 1 ? 2 : 1}`] : ['', 'This computer'];
  const rows = overlayRows(c);
  const tr = (cells: string[], th: boolean) => {
    const r = document.createElement('tr');
    cells.forEach((v, i) => {
      const cell = document.createElement(th || i === 0 ? 'th' : 'td');
      cell.textContent = v;
      r.append(cell);
    });
    return r;
  };
  const thead = document.createElement('thead');
  thead.append(tr(head, true));
  const tbody = document.createElement('tbody');
  for (const r of rows) tbody.append(tr(c.online ? [...r] : [r[0], r[1]], false));
  table.replaceChildren(thead, tbody);
  $<HTMLElement>('overlay-notes').textContent = overlayNotes(c).join(' ');
  state.overlay = { shown: !overlayEl.hidden, local: c.local, peer: c.peer, text: copyText(c, new Date()) };
}

$<HTMLButtonElement>('overlay-copy').addEventListener('click', () => {
  const text = copyText(overlayContext(), new Date());
  const done = () => {
    const el = $<HTMLElement>('overlay-copied');
    el.hidden = false;
    setTimeout(() => (el.hidden = true), 1500);
  };
  if (navigator.clipboard?.writeText) navigator.clipboard.writeText(text).then(done, () => fallbackCopy(text, done));
  else fallbackCopy(text, done);
});
function fallbackCopy(text: string, done: () => void): void {
  const ta = document.createElement('textarea');
  ta.value = text;
  document.body.append(ta);
  ta.select();
  try {
    if (document.execCommand('copy')) done();
  } finally {
    ta.remove();
  }
}

// The note for a machine that cannot keep up: once the game has run a while (not paused) below full speed
// because of this computer (solo: slower than the game; online: this side sets a pace below full speed).
// Dismissed, it stays away for this visit.
const HINT_AFTER_MS = 15000, HINT_SPEED = 0.93, HINT_HOLD_MS = 5000;
const hintBox = $<HTMLElement>('slow-hint');
let hintDismissed = false, hintShownSince = 0, hintSlowSince = -1;
$<HTMLButtonElement>('slow-hint-close').addEventListener('click', () => {
  hintDismissed = true;
  hintBox.hidden = true;
  if (state.hint) state.hint.dismissed = true;
});
function updateSlowHint(local: StatsSummary): void {
  if (hintDismissed || state.firstFrameMs === undefined || state.paused || tracker.seconds() < 2.5) return;
  const now = performance.now();
  if (now - state.startedAt - (state.firstFrameMs ?? 0) < HINT_AFTER_MS) return;
  // Slow over the last seconds (the overlay's numbers, not one moment's pace), and because of this computer:
  // solo when it could not run faster; online when it sets the pace (its proposal is the lower one).
  const p = state.pace;
  const online = !!p && !!online0?.state.player;
  const speed = local.speed;
  const ours = online ? !(p!.peerProp !== null && p!.peerProp < p!.prop) : local.cap !== null && local.cap < 1;
  const slow = speed !== null && speed < HINT_SPEED && ours;
  if (!slow) {
    hintSlowSince = -1;
    if (!hintBox.hidden && now - hintShownSince > 10000) hintBox.hidden = true;
    return;
  }
  if (hintSlowSince < 0) hintSlowSince = now;
  if (now - hintSlowSince < HINT_HOLD_MS && hintBox.hidden) return;  // a few seconds, not a heavy moment
  const text = `This computer is running the game at ${speed!.toFixed(2)}× speed` +
               (online ? ', so the online game runs at that pace for both players' : '') +
               '. On a laptop, plugging in and choosing the Best performance power mode helps.';
  $<HTMLElement>('slow-hint-text').textContent = text;
  if (hintBox.hidden) hintShownSince = now;
  hintBox.hidden = false;
  state.hint = { shown: true, text, dismissed: false };
}
const online0 = online;

function onStats(s: Status, w: { fields: number; ms: number; wallMs: number; cap?: number }): void {
  const audioW = state.audio?.worklet;
  tracker.push({
    t: performance.now(), fields: s.fields, windowFields: w.fields, windowMs: w.ms, presentsNew: s.presents_new ?? null,
    gpu: s.gpu ? { n: s.gpu.n, ms: s.gpu.ms } : null, cap: w.cap ?? null, scale: s.scale ?? null,
    threaded: s.threaded ?? null, stalls: netStatus?.stalls ?? null, stallMs: netStatus?.stallMs ?? null,
    underruns: audioW ? audioW.underruns : null, paused: state.paused,
  });
  gpuName ??= state.gfx?.ok ? state.gfx.message : null;
  const local = tracker.summary(describeMachine(gpuName));
  // Online: this side's numbers for the other player, once a second.
  const now = performance.now();
  if (online?.state.channel === 'open' && now - lastStatsSent >= 1000) {
    lastStatsSent = now;
    online.sendStats({ ...local, player: online.state.player ?? null } as unknown as Record<string, unknown>);
  }
  if (!overlayEl.hidden) renderOverlay();
  updateSlowHint(local);
  // Auto and the GPU: a ceiling for the picture's scale while the GPU cannot keep up.
  // Only with the GPU's time from timestamp queries, and only for the Auto setting.
  if (s.gpu?.timed && s.scale && pictureSetting() === 'auto' && !state.paused) {
    const before = autoScale.ceiling;
    autoScale.update(now, { n: s.gpu.n, ms: s.gpu.ms, dropped: s.gpu.dropped }, s.scale, sizeScale());
    state.autoScale = { ceiling: autoScale.ceiling, steps: autoScale.steps, lastBusy: autoScale.lastBusy };
    if (autoScale.ceiling !== before) {
      log(`picture: Auto ${autoScale.ceiling < before ? 'lowered' : 'raised'} to at most ${autoScale.ceiling}x ` +
          `(the GPU was busy ${(100 * (autoScale.lastBusy ?? 0)).toFixed(0)}% of the time at ${s.scale}x)`);
      applyPicture();
    }
  }
}
if (online) {
  online.onPeerStats = (m) => {
    const p = parsePeerStats(m);
    if (!p) return;
    peerStats = p;
    peerStatsAt = performance.now();
  };
}

// ---- pause, fullscreen, sound ----

let userPaused = false;
function applyPause(): void {
  const paused = userPaused || document.hidden;
  if (paused === state.paused) return;
  state.paused = paused;
  send({ type: 'pause', paused });
  audio?.setPaused(paused);
  pauseBtn.textContent = userPaused ? 'Resume' : 'Pause';
  if (worker && !state.error && state.firstFrameMs !== undefined)
    setStatus(paused ? (userPaused ? 'Paused. P or the Resume button continues.' : 'Paused (tab hidden).') : 'Running.');
}
document.addEventListener('visibilitychange', applyPause);
pauseBtn.addEventListener('click', () => {
  userPaused = !userPaused;
  applyPause();
});

function toggleFullscreen(): void {
  if (document.fullscreenElement) void document.exitFullscreen();
  else void stage.requestFullscreen?.({ navigationUI: 'hide' }).catch((e: unknown) => log(`fullscreen: ${e}`, true));
}
fullscreenBtn.addEventListener('click', toggleFullscreen);
canvas.addEventListener('dblclick', toggleFullscreen);
document.addEventListener('fullscreenchange', () => {
  fullscreenBtn.textContent = document.fullscreenElement ? 'Exit fullscreen' : 'Fullscreen';
  canvas.focus();
});

soundBtn.addEventListener('click', () => {
  if (!audio) return;
  if (audio.info.state === 'waiting-for-gesture' || audio.info.state === 'starting') return;  // this click unlocked it
  audio.setMuted(!audio.isMuted());
});

window.addEventListener('keydown', (e) => {
  if (e.repeat || e.ctrlKey || e.altKey || e.metaKey) return;
  if (e.target instanceof HTMLInputElement || e.target instanceof HTMLButtonElement) return;
  if (e.code === 'Backquote') toggleOverlay();
  else if (e.code === 'KeyF') toggleFullscreen();
  else if (e.code === 'KeyM' && audio && audio.info.state !== 'waiting-for-gesture' && audio.info.state !== 'starting')
    audio.setMuted(!audio.isMuted());
  else if (e.code === 'KeyP') {
    userPaused = !userPaused;
    applyPause();
  }
});

// P2: the display's refreshes, for the threaded engine's render thread, which
// presents once per refresh (a Worker of a Worker has no requestAnimationFrame of
// its own): a counter in shared memory, bumped and notified at every animation
// frame. Only where the page is cross-origin isolated (else no shared memory, and
// the single-threaded engine presents from its own animation frames).
const vsync = (self as unknown as { crossOriginIsolated?: boolean }).crossOriginIsolated === true &&
              typeof SharedArrayBuffer !== 'undefined' ? new Int32Array(new SharedArrayBuffer(4)) : null;

function tick(now: number): void {
  input.poll();
  if (!startBox.hidden && !input.capture && input.manager.confirmPressed()) playBtn.click();  // "press A to play" with a controller
  if (vsync) {
    Atomics.add(vsync, 0, 1);
    Atomics.notify(vsync, 0);
  }
  send({ type: 'tick', now });
  requestAnimationFrame(tick);
}
requestAnimationFrame(tick);

// ---- where the disc comes from ----

async function init(): Promise<void> {
  if (options.render && !hasWebGPU) {
    pickerBox.hidden = true;
    showError('webgpu', NO_WEBGPU);
    return;
  }
  setStatus('Looking for the disc...');
  const found = await findServerDisc(location.href);
  // The Docker deployment's Server disc page (the operator uploads the image there): a small link at the bottom.
  if (found.kind !== 'error' && found.discPage) {
    $<HTMLAnchorElement>('server-disc-link').href = found.discPage;
    $<HTMLElement>('operator').hidden = false;
  }
  if (found.kind === 'error') {
    pickerBox.hidden = true;
    showError('server', found.message);
    return;
  }
  if (found.kind === 'none') {
    if (found.discPage) {
      // The deployment, before its operator uploaded the disc: players never pick or upload one.
      pickerBox.hidden = true;
      setStatus('');
      showError('no-disc', '');
      return;
    }
    log(`disc: pick one (${found.why})`);
    await initOnline();
    pickerBox.hidden = false;
    setStatus('Pick your disc image to start.');
    return;
  }
  if (found.preparing) {
    // The server is hashing a new image (once after an upload or an update): ask again shortly.
    setStatus('The server is preparing the game data (once after an update)...');
    progressEl.hidden = false;
    progressEl.removeAttribute('value');
    setTimeout(() => void init(), 2000);
    return;
  }
  log(`disc: from the server (${new URL(found.url).pathname})`);
  const streamed: DiscSource = { kind: 'url', url: found.url };
  if (found.offer && storeDisc && canStore()) {
    const kept = await keepDisc(found.offer, streamed);
    if (kept) await ready(kept);
    return;
  }
  await ready(streamed);
}

// The server's image in this browser's storage: the copy kept from an earlier
// visit, else downloaded now with a progress bar. On failure the error says what
// to do, and the player may still play with the image read from the server.
async function keepDisc(offer: DiscOffer, streamed: DiscSource): Promise<DiscSource | null> {
  const total = offer.size;
  const have = await storedDisc(offer.hash, total);
  if (have) {
    state.download = { state: 'stored', done: total, total, bytesPerS: 0 };
    log(`disc: the game data stored in this browser (${offer.hash.slice(0, 12)}...)`);
    return { kind: 'file', file: have, stored: true };
  }
  state.download = { state: 'checking', done: 0, total, bytesPerS: 0 };
  progressEl.hidden = false;
  progressEl.removeAttribute('value');
  setStatus('Getting ready to download the game data...');
  try {
    const r = await downloadDisc(offer, (p) => {
      state.download = { state: p.phase, done: p.done, total: p.total, bytesPerS: p.bytesPerS };
      if (p.phase === 'waiting') {
        setStatus('Another tab of this browser is downloading the game data; waiting for it...');
        return;
      }
      if (p.phase === 'checking') return;
      progressEl.value = p.done / p.total;
      setStatus(`Downloading the game data from the server, once: ${Math.floor((100 * p.done) / p.total)}% ` +
                `(${progressText(p.done, p.total, p.bytesPerS)}). Later visits start at once.`);
    }, storeSpare);
    state.download = { state: 'done', done: total, total, bytesPerS: r.ms ? (r.fetched * 1000) / r.ms : 0, ms: r.ms,
                       fetched: r.fetched, resumedFrom: r.resumedFrom };
    progressEl.hidden = true;
    log(`disc: downloaded ${Math.round(r.fetched / 1048576)} MB in ${(r.ms / 1000).toFixed(1)} s` +
        (r.resumedFrom ? ` (resumed at ${Math.round(r.resumedFrom / 1048576)} MB)` : '') + '; kept in this browser');
    return { kind: 'file', file: r.file, stored: true };
  } catch (e) {
    const err = e instanceof DownloadError ? e : new DownloadError('storage', String(e));
    state.download = { ...(state.download ?? { done: 0, total, bytesPerS: 0 }), state: 'error', kind: err.kind, error: err.message };
    const mb = `${Math.round(total / 1048576).toLocaleString('en-US')} MB`;
    const text: Record<DownloadError['kind'], string> = {
      quota: `This browser cannot keep the game data (${mb}): ${err.message}. Free some disk space, and use a normal ` +
             'window (not a private or incognito one), then reload the page.',
      login: 'You are not logged in (or your login expired). Log in on this site, then reload the page.',
      changed: 'The game disc on the server was replaced during the download. Reload the page to download the new one.',
      network: `The game data could not be downloaded (${err.message}). Reload the page: the download continues where it stopped.`,
      corrupt: `The downloaded game data did not match the server's checksums (${err.message}). Reload the page to try again.`,
      storage: `This browser could not store the game data (${err.message}). Reload the page to try again, or use another browser.`,
    };
    showError('download', text[err.kind]);
    if (err.kind !== 'login' && err.kind !== 'changed') {
      // Still playable: the engine reads the image from the server (it may pause while the game loads).
      streamBox.hidden = false;
      $<HTMLButtonElement>('stream').addEventListener('click', () => {
        streamBox.hidden = true;
        void ready(streamed);
      }, { once: true });
    }
    return null;
  }
}

// The disc is ready: the lobby opens (E5) or the game starts.
async function ready(disc: DiscSource): Promise<void> {
  await initOnline();
  if (online?.state.available) offerStart(disc);
  else startEngine(disc);
}

// With the lobby (E5) on the page and the disc on the server: Play starts a
// game alone at once; creating or joining an online game starts the engine as
// soon as this page has its player number (the Worker then waits for the
// other player's handshake before it boots).
function offerStart(disc: DiscSource): void {
  startBox.hidden = false;
  errorEl.hidden = true;
  state.error = undefined;
  setStatus('The game is ready. Press Play, or create or join an online game.');
  playBtn.addEventListener('click', () => startEngine(disc), { once: true });
  const waitForRoom = () => {
    if (worker) return;
    if (online?.netOptions()) startEngine(disc);
    else setTimeout(waitForRoom, 250);
  };
  waitForRoom();
}

void init();
