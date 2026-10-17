// Two-browser netplay check for E5 (needs the player's disc; never run in CI).
// Starts the netplay server (engine/server, built with `npm run build`) with a
// throwaway password in front of the built site, opens two isolated browser
// contexts (two players with their own cookies), logs both in, lets player 1
// create a game and player 2 open the invite link, gives both pages the disc
// through their file inputs, and runs both games in lockstep with the same
// mvp-input script (each page drives only its own port from it). Both stop at
// the poll of frame N and report their guest line (MEM1 SHA-1) and their
// state-hash windows; they must be identical, with zero desyncs.
//
//   node engine/web/tools/netplay_check.mjs [--browser chrome|msedge|chromium|firefox] [--browser2 NAME]
//        [--disc PATH] [--script FILE] [--frames N] [--delay D|auto] [--fast] [--headed]
//        [--expect "guest: ..."] [--out DIR] [--tag NAME] [--timeout SECONDS] [--no-render]
//        [--steady-from FRAME] [--netsim LATENCY,JITTER,LOSS]
//        [--packs P1,P2,...] [--packs2 P1,...|none] [--expect-reject] [--server-disc]
//        [--params Q] [--params1 Q] [--params2 Q] [--host-slow1 N[@F]] [--host-slow2 N[@F]]
//        [--audio] [--trace-present] [--fast-until F] [--measure-from F] [--measure-to G]   (Q5)
//
//   --expect   a guest line both pages must also match (e.g. the native
//              `mvp-netplay solo` run of the same script, frames and delay)
//   --steady-from  measure input latency, frame rate, stalls and bandwidth from
//              this frame on (default N/2: past the boot and the intro movie)
//   --netsim   a slow network: each page delays its outgoing datagrams by LATENCY
//              ms plus up to JITTER ms and drops a fraction LOSS (both directions)
//   --packs    roster packs (E6) for both players: pack folders or .zip files, in set
//              order, through each page's Roster packs panel before the disc
//   --packs2   player 2's packs instead (`none`: no packs)
//   --expect-reject  the two pack sets differ on purpose: pass when both pages are
//              refused at the handshake with the roster-packs reason, and neither boots
//   --server-disc  the deployment's way (P1): the server serves the disc, each page
//              downloads it into its own browser storage before its lobby opens, and
//              the engines start on creating and joining the game (no file inputs;
//              no packs)
//   --delay    the input delay player 1 creates the game with (default 3); `auto`: the
//              lobby measures the link and both pages pick it (Q5)
//
// Q5, pacing measurements (mismatched machines):
//   --params Q / --params1 Q / --params2 Q   more page parameters for both pages / player 1's / player 2's
//   --host-slow1 N[@F], --host-slow2 N[@F]   that player's machine N times slower from VI field F on
//              (hostSlow=N&hostSlowFrom=F; Chromium: its page's main thread throttled N times too)
//   --audio    click each page first (the gesture that starts its sound): the worklet's underruns count
//   --trace-present  every present each page made (tracePresent=1), for the frame intervals
//   --fast-until F   both pages unpaced to VI field F, then paced (fastUntil=F)
//   --measure-from F, --measure-to G   the measured stretch, in VI fields (default: after --fast-until)
//   --separate the two pages in two browser instances (each with its own GPU process, as on two machines)
//   --affinity HEXMASK  (Windows) the browsers' processes only on these logical processors, set once both
//              pages have loaded and again when both run (e.g. FFFF: this PC's performance cores)
//   The summary's `measured` has per page: game speed, emulation ms per field, new frames shown per
//   second, the new-frame interval p95 and its share over 25 ms, waits per second and the share of the
//   time waiting for the other player, audio underruns per minute, and the pacing (Q5) as reported.
//
// Writes netplay-BROWSER[-TAG].json to --out (default build/engine/web-check).
// Exit status: 0 when both pages stopped at frame N with identical guest lines
// and windows and no desync, 1 otherwise, 77 without a disc.

import { spawn, spawnSync } from 'node:child_process';
import { randomBytes } from 'node:crypto';
import { existsSync, mkdirSync, readFileSync, statSync, writeFileSync } from 'node:fs';
import { join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { chromium, firefox } from 'playwright';

const here = fileURLToPath(new URL('.', import.meta.url));
const repo = resolve(here, '..', '..', '..');
const argv = process.argv.slice(2);
const opt = (name, fallback) => {
  const i = argv.indexOf(name);
  return i >= 0 && i + 1 < argv.length ? argv[i + 1] : fallback;
};
const flag = (name) => argv.includes(name);

const browserName = opt('--browser', 'chrome');
const browserName2 = opt('--browser2', browserName);  // player 2's browser (mixed-browser games)
const disc = opt('--disc', process.env.MVP_DISC ?? '');
const out = resolve(opt('--out', join(repo, 'build', 'engine', 'web-check')));
const scriptFile = opt('--script', '');
const frames = Number(opt('--frames', '1200'));
const delay = opt('--delay', '3');  // a number, or `auto` (Q5: the lobby measures the link and picks it)
const fast = flag('--fast');
const headed = flag('--headed');
const render = !flag('--no-render');
const expect = opt('--expect', '');
const netsim = opt('--netsim', '');  // "latency,jitter,loss" on each page's outgoing datagrams
const tag = opt('--tag', '');
const packsList = (v) => (v === 'none' ? [] : v.split(',').filter(Boolean).map((p) => resolve(p)));
const packs1 = packsList(opt('--packs', ''));
const packs2 = argv.includes('--packs2') ? packsList(opt('--packs2', '')) : packs1;
const expectReject = flag('--expect-reject');
const serverDisc = flag('--server-disc');
const timeoutS = Number(opt('--timeout', '900'));
const site = resolve(opt('--site', join(repo, 'build', 'engine', 'emscripten', 'web', 'site')));
const serverMain = join(repo, 'engine', 'server', 'dist', 'src', 'main.js');
const label = `netplay-${browserName}${browserName2 !== browserName ? `-${browserName2}` : ''}${tag ? `-${tag}` : ''}`;
// Q5: per-page parameters, simulated slower machines, sound, the presentation trace, the measured stretch.
const pageParams = [opt('--params1', ''), opt('--params2', '')].map((p) => `${opt('--params', '')}&${p}`);
const hostSlows = [opt('--host-slow1', ''), opt('--host-slow2', '')].map((v) => (v ? v.split('@').map(Number) : null));
const audio = flag('--audio');
const tracePresent = flag('--trace-present');
const fastUntil = Number(opt('--fast-until', '0'));
const measureFrom = Number(opt('--measure-from', String(fastUntil)));
const measureTo = Number(opt('--measure-to', String(Number.MAX_SAFE_INTEGER)));
const separate = flag('--separate');
const affinity = opt('--affinity', '');

if (!disc || !existsSync(disc)) {
  console.error('netplay_check: no disc image (--disc or MVP_DISC)');
  process.exit(77);
}
if (!existsSync(serverMain)) {
  console.error('netplay_check: build the server first (npm ci && npm run build in engine/server)');
  process.exit(2);
}
mkdirSync(out, { recursive: true });
const script = scriptFile ? readFileSync(scriptFile, 'utf8') : '';

// The server: a throwaway password, port 0, no disc route (the pages use the file input),
// or with --server-disc the disc configured.
const password = randomBytes(18).toString('base64url');
const server = spawn(process.execPath, [serverMain], {
  env: { ...process.env, MVP_PASSWORD: password, MVP_SITE: site, MVP_PORT: '0', MVP_HOST: '127.0.0.1',
         MVP_DISC_IMAGE: serverDisc ? disc : '' },
  stdio: ['ignore', 'pipe', 'inherit'],
});
const base = await new Promise((resolveUrl, reject) => {
  server.stdout.on('data', (d) => {
    const m = /listening on (http:\/\/\S+);/.exec(String(d));
    if (m) resolveUrl(m[1]);
  });
  server.on('exit', (code) => reject(new Error(`server exited ${code}`)));
});

// Two pages run at once: neither may be throttled as a background or occluded page (headed windows
// can cover each other).
const chromiumArgs = ['--enable-unsafe-webgpu', '--disable-features=WebRtcHideLocalIpsWithMdns',
                      '--disable-renderer-backgrounding', '--disable-backgrounding-occluded-windows',
                      '--disable-background-timer-throttling', ...(audio ? ['--mute-audio'] : [])];
const launchers = {
  chrome: () => chromium.launch({ channel: 'chrome', headless: !headed, args: chromiumArgs }),
  msedge: () => chromium.launch({ channel: 'msedge', headless: !headed, args: chromiumArgs }),
  chromium: () => chromium.launch({ headless: !headed, args: chromiumArgs }),
  firefox: () => firefox.launch({
    headless: !headed,
    firefoxUserPrefs: { 'dom.webgpu.enabled': true, 'dom.webgpu.workers.enabled': true, 'gfx.webgpu.ignore-blocklist': true,
                        'media.peerconnection.ice.obfuscate_host_addresses': false,
                        ...(audio ? { 'media.volume_scale': '0.0' } : {}) },
  }),
};

// --affinity (Windows): every chrome.exe / msedge.exe descending from this process on the given logical
// processors (as browser_check.mjs does). Returns how many processes were set.
function setAffinity(mask) {
  const ps = `$m=[IntPtr]0x${mask}; $all=Get-CimInstance Win32_Process; $ids=@(${process.pid}); ` +
    `do { $more=@($all | Where-Object { $ids -notcontains $_.ProcessId -and $ids -contains $_.ParentProcessId } | ` +
    `ForEach-Object { $_.ProcessId }); $ids+=$more } while ($more.Count -gt 0); $n=0; ` +
    `foreach ($p in ($all | Where-Object { $ids -contains $_.ProcessId -and $_.Name -match '^(chrome|msedge)[.]exe$' })) ` +
    `{ try { (Get-Process -Id $p.ProcessId).ProcessorAffinity=$m; $n++ } catch {} }; $n`;
  const r = spawnSync('powershell', ['-NoProfile', '-Command', ps], { encoding: 'utf8' });
  const set = Number(String(r.stdout).trim().split(/\s+/).pop());
  console.log(`affinity 0x${mask}: ${set} browser processes`);
  return set;
}

// The measured stretch of one page (Q5): its stats windows from field `from` to `to`.
const q = (v, p) => (v.length ? v[Math.min(v.length - 1, Math.floor(v.length * p))] : null);
function measure(windows) {
  const w = windows.filter((x) => x.field >= measureFrom && x.field <= measureTo).slice(1);  // the first may straddle the start
  if (w.length < 3) return { note: 'not enough windows' };
  const fields = w.reduce((n, x) => n + x.fields, 0), ms = w.reduce((n, x) => n + x.ms, 0);
  const wall = w.reduce((n, x) => n + x.wallMs, 0);
  const first = w[0], last = w[w.length - 1];
  const seconds = (last.at - first.at) / 1000;
  const r = {
    fromField: first.field, toField: last.field, seconds: +seconds.toFixed(1),
    speed: +(fields / 59.94 / (wall / 1000)).toFixed(3),
    emulationMsPerField: +(ms / fields).toFixed(2),
  };
  const presents = w.flatMap((x) => x.trace ?? []).filter((t) => t.k === 'p');
  const fresh = presents.filter((t) => t.new);
  if (fresh.length > 2) {
    const d = fresh.slice(1).map((t, i) => t.ms - fresh[i].ms).sort((a, b) => a - b);
    const span = fresh[fresh.length - 1].ms - fresh[0].ms;
    r.newFramesPerSecond = +((1000 * d.length) / span).toFixed(1);
    r.newFrameIntervalMedian = +q(d, 0.5).toFixed(1);
    r.newFrameIntervalP95 = +q(d, 0.95).toFixed(1);
    r.newFrameIntervalP99 = +q(d, 0.99).toFixed(1);
    r.over25 = +((100 * d.filter((x) => x > 25).length) / d.length).toFixed(1);
    r.over50 = +((100 * d.filter((x) => x > 50).length) / d.length).toFixed(1);
    r.presentsPerSecond = +((1000 * (presents.length - 1)) / (presents[presents.length - 1].ms - presents[0].ms)).toFixed(1);
  }
  const net0 = first.net, net1 = last.net;
  if (net0 && net1 && seconds > 0) {
    r.waitsPerSecond = +((net1.stalls - net0.stalls) / seconds).toFixed(1);
    r.waitingPct = +((100 * (net1.stallMs - net0.stallMs)) / (seconds * 1000)).toFixed(1);
    r.pollStopsPerSecond = +((net1.pollStops - net0.pollStops) / seconds).toFixed(1);
    r.framesPerSecond = +((net1.frame - net0.frame) / seconds).toFixed(1);
    r.rttAvg = net1.rttAvg;
    r.delay = net1.delay;
  }
  const a0 = first.audio, a1 = last.audio;
  if (a0 && a1 && seconds > 0) {
    r.underrunsPerMinute = +((60 * (a1.underruns - a0.underruns)) / seconds).toFixed(1);
    r.gaps20PerMinute = +((60 * ((a1.gaps20 ?? 0) - (a0.gaps20 ?? 0))) / seconds).toFixed(1);
    r.audioSpeed = a1.speed;
  }
  const p = w.map((x) => x.pace).filter(Boolean);
  if (p.length) {
    const rates = p.map((x) => x.rate).sort((a, b) => a - b);
    r.pace = { rateMedian: q(rates, 0.5), rateMin: rates[0], rateMax: rates[rates.length - 1], last: p[p.length - 1] };
  }
  return r;
}

const summary = { browser: browserName, browser2: browserName2, netsim: netsim || null, tag: tag || null, frames, delay, fast, render, script: scriptFile || null,
                  packs: [packs1, packs2], expectReject, params: pageParams, hostSlow: hostSlows, fastUntil,
                  measure: [measureFrom, measureTo], site };
let rc = 1;
const browsers = [];
try {
  browsers.push(await launchers[browserName]());
  if (browserName2 !== browserName || separate) browsers.push(await launchers[browserName2]());
  summary.version = browsers.map((b) => b.version());
  const pages = [];
  for (const who of ['p1', 'p2']) {
    const browser = who === 'p2' ? browsers[browsers.length - 1] : browsers[0];
    const ctx = await browser.newContext({ viewport: { width: 1000, height: 1000 } });
    const page = await ctx.newPage();
    const errors = [];
    page.on('console', (m) => {
      if (m.type() === 'error') errors.push(m.text());
    });
    page.on('pageerror', (e) => errors.push(String(e)));
    const requests = [];
    page.on('request', (r) => requests.push(r.url()));
    // Log in through the form, as a player would (to a page that starts nothing).
    await page.goto(`${base}/login?next=%2Fhealthz`);
    await page.fill('input[name=password]', password);
    await Promise.all([page.waitForURL(`${base}/healthz`), page.click('button[type=submit]')]);
    pages.push({ who, page, errors, requests, windows: [], lastWindow: '', throttled: false });
  }
  const query = (k) => {
    const qs = new URLSearchParams();
    if (script) qs.set('script', script);
    qs.set('frames', String(frames));
    if (fast) qs.set('fast', '1');
    if (!render) qs.set('render', '0');
    if (netsim) qs.set('netsim', netsim);
    if (tracePresent) qs.set('tracePresent', '1');
    if (fastUntil) qs.set('fastUntil', String(fastUntil));
    const hs = hostSlows[k];
    if (hs) {
      qs.set('hostSlow', String(hs[0]));
      if (hs[1]) qs.set('hostSlowFrom', String(hs[1]));
    }
    for (const [key, v] of new URLSearchParams(pageParams[k])) if (key) qs.set(key, v);
    return qs;
  };

  const [a, b] = pages;
  await a.page.goto(`${base}/?online=create&delay=${delay}&${query(0)}`);
  // With --server-disc each page first downloads the disc (its lobby opens afterwards).
  const lobbyMs = serverDisc ? timeoutS * 1000 : 15000;
  await a.page.waitForFunction(() => window.mvp?.net?.invite, null, { timeout: lobbyMs });
  const invite = await a.page.evaluate(() => window.mvp.net.invite);
  summary.invite = invite.replace(/room=[a-z0-9]+/, 'room=<id>');
  await b.page.goto(`${invite}&${query(1)}`);
  await b.page.waitForFunction(() => window.mvp?.net?.player === 2, null, { timeout: lobbyMs });
  // The user gesture that starts each page's sound (Q5: underruns are measured).
  if (audio) for (const p of pages) await p.page.mouse.click(5, 5);
  // Roster packs (E6), through each page's panel, before the disc starts the engine.
  for (const [k, list] of [packs1, packs2].entries()) {
    for (const [i, p] of list.entries()) {
      await pages[k].page.setInputFiles(statSync(p).isDirectory() ? '#packs-folder' : '#packs-zip', p);
      await pages[k].page.waitForFunction((n) => window.mvp.packs?.count === n && window.mvp.packs.report, i + 1,
                                          { timeout: 30000 });
    }
  }
  summary.packSets = await Promise.all(pages.map((p) => p.page.evaluate(() => window.mvp.packs?.setHash ?? null)));
  console.log(`pack sets: ${summary.packSets.join(' / ')}`);
  const t0 = Date.now();
  if (serverDisc) summary.download = await Promise.all(pages.map((p) => p.page.evaluate(() => window.mvp.download)));
  else await Promise.all(pages.map((p) => p.page.setInputFiles('#disc', disc)));
  if (affinity) summary.affinity = { mask: affinity, processes: [setAffinity(affinity)] };
  let affinityAgain = !affinity;

  const read = (p) => p.page.evaluate(() => {
    const s = window.mvp;
    return { phases: s.phases, firstFrameMs: s.firstFrameMs, stats: s.stats, stopped: s.stopped, error: s.error,
             packs: s.packs?.mounted ? { ok: s.packs.mounted.ok, set: s.packs.mounted.set_hash,
                                         result: s.packs.mounted.result_hash, files: s.packs.mounted.files } : null,
             net: s.net && { ...s.net, windows: undefined, diagnostic: s.net.diagnostic } , windows: s.net?.windows,
             worklet: s.audio?.worklet ?? null, pace: s.pace ?? null, gfx: s.gfx ?? null,
             overlay: s.overlay?.text ?? null, hint: s.hint ?? null, link: s.net?.link ?? null };
  });
  const deadline = t0 + timeoutS * 1000;
  let states = [];
  const timeline = [];
  const samples = [[], []];  // per page: {t, status, transport} while running
  let lastT = 0;
  while (Date.now() < deadline) {
    states = await Promise.all(pages.map(read));
    const now = Date.now();
    for (const [k, st] of states.entries()) {
      const p = pages[k];
      if (st.net?.status?.state === 'running' && !st.stopped)
        samples[k].push({ t: now, status: st.net.status, transport: st.net.transport });
      // Each stats window once (the page posts one about twice a second), with what the session and
      // the worklet said at the time (Q5).
      if (st.stats && JSON.stringify(st.stats) !== p.lastWindow) {
        p.lastWindow = JSON.stringify(st.stats);
        const n = st.net?.status, w = st.worklet;
        p.windows.push({ field: st.stats.status.fields, frame: st.stats.status.frames, at: now - t0, ...st.stats.window,
                         net: n ? { frame: n.frame, stalls: n.stalls, stallMs: n.stallMs, pollStops: n.pollStops,
                                    rttAvg: n.rttAvg, delay: n.delay } : undefined,
                         audio: w ? { underruns: w.underruns, gaps20: w.gaps20, speed: w.speed, fill: w.fill } : undefined,
                         thread: st.stats.status.thread ? { lagMs: st.stats.status.thread.lag_ms, writeWaitMs: st.stats.status.thread.write_wait_ms,
                                                            fenceMs: st.stats.status.thread.fence_ms } : undefined,
                         renderBusyMs: st.stats.status.render?.busy_ms, presents: st.stats.status.presents,
                         pace: st.pace ?? undefined });
      }
      const hs = hostSlows[k];
      if (hs && !p.throttled && (st.stats?.status.fields ?? 0) >= (hs[1] || 0) && (k === 0 ? browserName : browserName2) !== 'firefox') {
        p.throttled = true;
        const cdp = await p.page.context().newCDPSession(p.page);
        await cdp.send('Emulation.setCPUThrottlingRate', { rate: hs[0] });
        console.log(`${p.who}: host slowed ${hs[0]}x at field ${st.stats?.status.fields ?? 0}`);
      }
    }
    if (!affinityAgain && states.every((st) => st.firstFrameMs !== undefined)) {
      affinityAgain = true;
      summary.affinity.processes.push(setAffinity(affinity));  // the render threads' and GPU's processes too
    }
    if (now - lastT >= 5000) {
      lastT = now;
      const row = { s: Math.round((now - t0) / 1000) };
      for (const [k, st] of states.entries()) {
        const n = st.net?.status;
        row[pages[k].who] = n ? { state: n.state, frame: n.frame, field: st.stats?.status.fields, stalls: n.stalls,
                                  stallMs: n.stallMs, rtt: n.rttAvg, rate: st.pace?.rate }
                              : { fields: st.stats?.status.fields };
      }
      timeline.push(row);
      console.log(JSON.stringify(row));
    }
    const done = states.every((st) => st.stopped && st.windows !== undefined);
    const failed = states.some((st) => st.error || (st.net?.status && ['desynced', 'rejected', 'disconnected'].includes(st.net.status.state)
                                                    && !st.stopped));
    const bothRefused = states.every((st) => st.net?.status?.state === 'rejected' && st.error);
    if (done || (failed && (!expectReject || bothRefused))) break;
    await new Promise((r) => setTimeout(r, tracePresent ? 150 : 500));
  }
  // Let the final reports arrive.
  await new Promise((r) => setTimeout(r, 1500));
  states = await Promise.all(pages.map(read));
  // Each page's last frame (game-derived: under build/ only).
  if (render) {
    for (const p of pages)
      await p.page.locator('#screen').screenshot({ path: join(out, `${label}-${p.who}-final.png`) }).catch(() => undefined);
  }
  summary.wallSeconds = (Date.now() - t0) / 1000;
  summary.timeline = timeline;
  summary.pages = states.map((st, k) => ({
    who: pages[k].who,
    phases: st.phases,
    error: st.error ?? null,
    packs: st.packs,
    gfx: st.gfx,
    overlayText: st.overlay,  // Q5: what Copy stats gives (with stats=1)
    hint: st.hint,
    link: st.link,            // Q5: the link measured for an Auto delay
    guestLine: st.stopped?.guestLine ?? null,
    why: st.stopped?.status.why ?? null,
    threaded: st.stopped?.status.threaded ?? null,  // P2: the render thread drew (the threaded engine)
    net: st.net ? { ...st.net, diagnostic: st.net.diagnostic ? st.net.diagnostic.slice(0, 4000) : undefined } : null,
    windowsCount: (st.windows ?? '').split('\n').filter(Boolean).length,
    consoleErrors: pages[k].errors.slice(0, 20),
    foreignRequests: pages[k].requests.filter((u) => !u.startsWith(base) && !u.startsWith('blob:') && !u.startsWith('data:')),
  }));
  // Q5: the measured stretch of each page, and the stats windows themselves (without the trace).
  summary.measured = pages.map((p) => ({ who: p.who, ...measure(p.windows) }));
  summary.statsWindows = pages.map((p) => p.windows.map(({ trace: _t, ...w }) => w));
  // Steady state: from the first sample at or after frame steadyFrom to the last one.
  const steadyFrom = Number(opt('--steady-from', String(Math.floor(frames / 2))));
  summary.steady = samples.map((list, k) => {
    const first = list.find((x) => x.status.frame >= steadyFrom);
    const last = list[list.length - 1];
    if (!first || !last || last === first) return { who: pages[k].who, note: 'not enough samples' };
    const dt = (last.t - first.t) / 1000;
    const d = (f) => f(last) - f(first);
    const latN = d((x) => x.status.latencySamples);
    return {
      who: pages[k].who,
      fromFrame: first.status.frame,
      toFrame: last.status.frame,
      seconds: dt,
      framesPerSecond: d((x) => x.status.frame) / dt,
      inputLatencyMs: latN ? d((x) => x.status.latencySum) / latN : null,
      stallMsPerSecond: d((x) => x.status.stallMs) / dt,
      payloadBytesPerSecondOut: d((x) => x.status.bytesSent) / dt,
      payloadPacketsPerSecondOut: d((x) => x.status.packetsSent) / dt,
      transportBytesPerSecondOut: first.transport && last.transport ? (last.transport.bytesSent - first.transport.bytesSent) / dt : null,
      transportPacketsPerSecondOut: first.transport && last.transport ? (last.transport.packetsSent - first.transport.packetsSent) / dt : null,
      rttAvgMs: last.status.rttAvg,
    };
  });
  const [sa, sb] = states;
  const sameGuest = !!sa.stopped && sa.stopped.guestLine === sb.stopped?.guestLine;
  const sameWindows = !!sa.windows && sa.windows === sb.windows;
  const netA = sa.net?.status, netB = sb.net?.status;
  const noDesync = !!netA && !!netB && !netA.desync && !netB.desync;
  const stoppedAtN = [sa, sb].every((st) => st.stopped?.status.why === `ran ${frames} frames`);
  const matchesExpect = !expect || sa.stopped?.guestLine === expect;
  const noGateMiss = !!netA && !!netB && netA.gateMisses === 0 && netB.gateMisses === 0;
  summary.checks = { sameGuest, sameWindows, noDesync, stoppedAtN, matchesExpect, noGateMiss,
                     pollStops: [netA?.pollStops ?? null, netB?.pollStops ?? null], windows: summary.pages[0].windowsCount,
                     delays: [netA?.delay ?? null, netB?.delay ?? null] };
  if (expect) summary.expect = expect;
  rc = sameGuest && sameWindows && noDesync && stoppedAtN && matchesExpect && noGateMiss ? 0 : 1;
  if (expectReject) {
    // Both refused at the handshake (different roster packs, gameplay mod settings, ...) and neither booted.
    const refused = [sa, sb].map((st) => st.net?.status?.state === 'rejected' && !st.stopped &&
                                         st.phases?.boot === undefined);
    summary.checks = { refused, reasons: [sa, sb].map((st) => st.net?.status?.reason ?? null),
                       errors: [sa, sb].map((st) => st.error?.message ?? null) };
    rc = refused.every(Boolean) ? 0 : 1;
  }
} catch (e) {
  summary.exception = String(e?.stack ?? e);
} finally {
  for (const b of browsers) await b.close();
  server.kill();
}
writeFileSync(join(out, `${label}.json`), JSON.stringify(summary, null, 2));
console.log(JSON.stringify({ checks: summary.checks, exception: summary.exception, steady: summary.steady,
                             measured: summary.measured,
                             pages: summary.pages?.map((p) => ({ who: p.who, guestLine: p.guestLine, why: p.why, error: p.error,
                                                                 net: p.net?.status, route: p.net?.route, transport: p.net?.transport })) },
                           null, 2));
console.log(rc === 0 ? (expectReject ? `PASS: both browsers refused the game: ${summary.checks?.reasons?.[0] ?? ''}`
                                     : 'PASS: both browsers stayed in lockstep') : 'FAIL');
process.exit(rc);
