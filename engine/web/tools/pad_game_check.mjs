// Controllers in the real game (Q3; needs the player's disc, never run in CI).
// Fake gamepads (fake_pads.mjs) play the game's intro and menus in a real
// browser, and the engine reports every input the guest consumed at its frame
// polls (`inputLog=1`, an mvp-input script). Each live run is then replayed from
// its own log with no gamepad at all, and must end in the very same guest state
// (the MEM1 hash line at the same frame): a controller reaches the game only
// through the input queue, once per frame poll, and nothing else changes it.
//
//   node engine/web/tools/pad_game_check.mjs --disc PATH [--browser chrome|msedge|chromium|firefox]
//        [--out DIR] [--site DIR] [--frames N] [--only NAME[,NAME]] [--headed]
//        [--net --server-main PATH]   (the netplay check: two pages, player 2 plays on a pad)
//
// Checks: `standard` (an Xbox-style pad, hot-plugged, START through the intro and
// the title), `raw` (an Xbox pad without the standard mapping: D-pad and triggers
// on axes), `play` (the deployment's Play button pressed with A on a pad), and
// with --net `netplay` (player 2's pad drives port 2 on both pages). Exit status
// 0 when all pass, 1 otherwise, 77 without a disc.

import { spawn } from 'node:child_process';
import { randomBytes } from 'node:crypto';
import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { chromium, firefox } from 'playwright';
import { PADS, installFakePads } from './fake_pads.mjs';

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
const out = resolve(opt('--out', join(repo, 'build', 'engine', 'pad-check')));
const site = resolve(opt('--site', join(repo, 'build', 'engine', 'emscripten', 'web', 'site')));
const frames = Number(opt('--frames', '2800'));
const only = opt('--only', '').split(',').filter(Boolean);
const headed = flag('--headed');
const net = flag('--net');
const serverMain = resolve(opt('--server-main', join(repo, 'engine', 'server', 'dist', 'src', 'main.js')));

if (!disc || !existsSync(disc)) {
  console.error('pad_game_check: no disc image (--disc or MVP_DISC)');
  process.exit(77);
}
mkdirSync(out, { recursive: true });

function startServer(command, args, env = {}) {
  const child = spawn(command, args, { env: { ...process.env, ...env }, stdio: ['ignore', 'pipe', 'inherit'] });
  const url = new Promise((resolveUrl, reject) => {
    child.stdout.on('data', (d) => {
      const m = /(?:serving|listening on) (http:\/\/[^\s;]+)/.exec(String(d));
      if (m) resolveUrl(m[1].replace(/\/$/, ''));
    });
    child.on('exit', (code) => reject(new Error(`server exited ${code}`)));
  });
  return { child, url };
}

const chromiumArgs = ['--enable-unsafe-webgpu', '--disable-features=WebRtcHideLocalIpsWithMdns'];
const launchers = {
  chrome: () => chromium.launch({ channel: 'chrome', headless: !headed, args: chromiumArgs }),
  msedge: () => chromium.launch({ channel: 'msedge', headless: !headed, args: chromiumArgs }),
  chromium: () => chromium.launch({ headless: !headed, args: chromiumArgs }),
  firefox: () => firefox.launch({
    headless: !headed,
    firefoxUserPrefs: { 'dom.webgpu.enabled': true, 'dom.webgpu.workers.enabled': true, 'gfx.webgpu.ignore-blocklist': true,
                        'media.peerconnection.ice.obfuscate_host_addresses': false },
  }),
};

function eq(actual, expected, message) {
  if (JSON.stringify(actual) !== JSON.stringify(expected)) {
    throw new Error(`${message}: expected ${JSON.stringify(expected)}, got ${JSON.stringify(actual)}`);
  }
}
function assert(cond, message) {
  if (!cond) throw new Error(message);
}

// The page's frame counter (stats arrive a few times a second; a step fires on the first report at or past its frame).
async function waitFrame(page, frame, timeoutMs = 600000) {
  await page.waitForFunction((f) => (window.mvp.stats?.status.frames ?? 0) >= f || !!window.mvp.stopped || !!window.mvp.error, frame,
                             { timeout: timeoutMs, polling: 50 });
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const pad = (page, slot, buttons = {}, axes = {}) => page.evaluate(([s, b, a]) => window.__pads.set(s, b, a), [slot, buttons, axes]);
const rest = (page, slot) => page.evaluate((s) => window.__pads.rest(s), slot);
const connect = (page, slot, spec) => page.evaluate(([s, sp]) => window.__pads.connect(s, sp), [slot, spec]);
const disconnect = (page, slot) => page.evaluate((s) => window.__pads.disconnect(s), slot);
// Hold something for ms of wall time (the engine consumes it at a frame poll within a few fields in unpaced mode).
async function hold(page, slot, buttons, axes, ms = 400) {
  await pad(page, slot, buttons, axes);
  await sleep(ms);
  await rest(page, slot);
  await sleep(150);
}

async function stopped(page, timeoutMs = 900000) {
  await page.waitForFunction(() => !!window.mvp.stopped || !!window.mvp.error, null, { timeout: timeoutMs, polling: 100 });
  const r = await page.evaluate(() => ({ stopped: window.mvp.stopped, error: window.mvp.error ?? null, log: window.mvp.inputLog ?? null }));
  if (r.error) throw new Error(`the engine stopped with an error: ${JSON.stringify(r.error)}`);
  return { guestLine: r.stopped.guestLine, why: r.stopped.status.why, inputLog: r.log, status: r.stopped.status };
}

const commands = (log) => (log ?? '').split('\n').filter((l) => l && !l.startsWith('mvp-input')).map((l) => l.replace(/^\d+ /, ''));

// A run with the disc through the file input, unpaced, no renderer (the guest is the same either way).
async function open(context, query, withPads) {
  const page = await context.newPage();
  const errors = [];
  page.on('pageerror', (e) => errors.push(String(e)));
  page.on('console', (m) => { if (m.type() === 'error' && !/Failed to load resource/.test(m.text())) errors.push(m.text()); });
  if (withPads) await page.addInitScript(installFakePads);
  page.__errors = errors;
  await page.route('**/config.json', (r) => r.fulfill({ status: 404, body: 'no' }));
  await page.goto(`${baseUrl}/?${query}`);
  return page;
}
let baseUrl = '';

async function liveAndReplay(context, name, drive) {
  const q = `frames=${frames}&fast=1&render=0&audio=0&cache=1&inputLog=1`;
  // Unpaced through the boot, then at the game's own speed (about 60 frames a second), so the steps below
  // (waiting for frames, a pad held for half a second) are well inside the run.
  const live = await open(context, `${q.replace('fast=1', 'fast=0')}&fastUntil=1000`, true);
  await live.setInputFiles('#disc', disc);
  await live.waitForFunction(() => window.mvp.phases.boot !== undefined || !!window.mvp.error, null, { timeout: 300000 });
  await drive(live);
  const a = await stopped(live);
  eq(a.why, `ran ${frames} frames`, `${name}: the live run stopped at the frame`);
  eq(live.__errors, [], `${name}: page errors`);
  writeFileSync(join(out, `${name}-input-${browserName}.txt`), a.inputLog);
  console.log(`  ${name}: live ${a.guestLine}`);
  console.log(`  ${name}: ${commands(a.inputLog).length} input changes consumed`);
  await live.close();
  // The replay: the same log as an mvp-input script, no gamepad anywhere.
  const replay = await open(context, `${q}&script=${encodeURIComponent(a.inputLog)}`, false);
  await replay.setInputFiles('#disc', disc);
  const b = await stopped(replay);
  console.log(`  ${name}: replay ${b.guestLine}`);
  eq(b.guestLine, a.guestLine, `${name}: the replay of the consumed inputs gives the same guest state`);
  eq(b.why, a.why, `${name}: same stop`);
  await replay.close();
  return a;
}

const checks = [];
const check = (name, fn) => checks.push({ name, fn });

check('standard', async (context) => {
  const a = await liveAndReplay(context, 'standard', async (page) => {
    await connect(page, 0, PADS.xbox);                                   // hidden until its first press
    await waitFrame(page, 1150);
    await hold(page, 0, { 9: 1 });                                       // START: the intro ends
    await waitFrame(page, 1700);
    await hold(page, 0, { 9: 1 });                                       // START at the licence / title
    await waitFrame(page, 2100);
    await disconnect(page, 0);                                           // unplugged mid-game
    await sleep(500);
    await connect(page, 2, PADS.xbox);                                   // plugged in again, another slot
    await hold(page, 2, { 9: 1 });
    await waitFrame(page, 2400);
    await hold(page, 2, { 0: 1 }, { 1: 1 });                             // A with the stick pushed down
    await hold(page, 2, { 13: 1 });                                      // D-pad down
  });
  const c = commands(a.inputLog);
  assert(c.filter((l) => l === '1 press START').length >= 3, `three START presses reached the game (log: ${c.slice(0, 12).join(' | ')})`);
  assert(c.includes('1 press A'), 'A reached the game');
  assert(c.some((l) => l.startsWith('1 stick 0 -')), 'the stick pushed down reached the game');
  assert(c.includes('1 press DOWN'), 'D-pad down reached the game');
  assert(!c.some((l) => /^2 /.test(l)), 'nothing on player 2');
  // Nothing sticks: every press was let go.
  assert(c.filter((l) => /^1 press/.test(l)).length === c.filter((l) => /^1 release/.test(l)).length, 'every press released');
});

check('raw', async (context) => {
  const a = await liveAndReplay(context, 'raw', async (page) => {
    await connect(page, 0, PADS.xpad);                                   // listed at once, no standard mapping
    await waitFrame(page, 1150);
    await hold(page, 0, { 7: 1 });                                       // Start is button 7 in the joystick order
    await waitFrame(page, 1700);
    await hold(page, 0, {}, { 7: -1 });                                  // D-pad up is axis 7
    await hold(page, 0, {}, { 2: 1 });                                   // the left trigger is axis 2
    await hold(page, 0, { 0: 1 });                                       // A
  });
  const c = commands(a.inputLog);
  assert(c.includes('1 press START'), 'START reached the game');
  assert(c.includes('1 press UP'), 'D-pad up (an axis) reached the game');
  assert(c.some((l) => l === '1 press L') && c.some((l) => l === '1 trigger 255 0'), 'the trigger clicked and was analog');
  assert(c.includes('1 press A'), 'A reached the game');
});

check('play', async (context) => {
  // The deployment's Play button, pressed with A on a pad (the mock deployment serves the disc).
  const srv = startServer(process.execPath, [join(here, 'serve.mjs'), '--port', '0', '--root', site, '--disc', disc]);
  const url = await srv.url;
  try {
    const page = await context.newPage();
    await page.addInitScript(installFakePads);
    await page.goto(`${url}/?fields=240&fast=1&render=0&audio=0&store=0`);
    await page.waitForSelector('#play', { state: 'visible', timeout: 60000 });
    await connect(page, 0, PADS.xbox);
    await sleep(300);
    await pad(page, 0, { 0: 1 });                                         // A
    await page.waitForFunction(() => window.mvp.stopped || window.mvp.error, null, { timeout: 300000 });
    const r = await page.evaluate(() => ({ source: window.mvp.source, error: window.mvp.error ?? null, stopped: !!window.mvp.stopped }));
    eq([r.source, r.error, r.stopped], ['url', null, true], 'the game started from the Play button and ran');
    await page.close();
  } finally {
    srv.child.kill();
  }
});

if (net) {
  check('netplay', async () => {
    // Two pages through E5's server: player 1 on the keyboard script of the page, player 2 on a pad. Both
    // guests consume the same inputs (the log has player 2's presses on port 2 and nothing of it on port 1).
    const accessKey = randomBytes(18).toString('base64url');
    const srv = startServer(process.execPath, [serverMain], { MVP_ACCESS_KEY: accessKey, MVP_SITE: site, MVP_PORT: '0', MVP_HOST: '127.0.0.1' });
    const base = await srv.url;
    const browsers = [await launchers[browserName]()];
    try {
      const pages = [];
      for (const who of ['p1', 'p2']) {
        const ctx = await browsers[0].newContext({ viewport: { width: 1000, height: 1000 } });
        const page = await ctx.newPage();
        const errors = [];
        page.on('pageerror', (e) => errors.push(String(e)));
        await page.addInitScript(installFakePads);
        // The private link, on a page that starts nothing (the server goes on to /healthz with the cookie).
        await page.goto(`${base}/healthz?key=${accessKey}`);
        if (page.url() !== `${base}/healthz`) throw new Error(`the private link went to ${page.url()}`);
        pages.push({ who, page, errors });
      }
      const nf = Math.min(frames, 2400);
      const q = `frames=${nf}&fast=1&render=0&audio=0&inputLog=1`;
      const [a, b] = pages;
      await a.page.goto(`${base}/?online=create&delay=3&${q}`);
      await a.page.waitForFunction(() => window.mvp?.net?.invite, null, { timeout: 30000 });
      const invite = await a.page.evaluate(() => window.mvp.net.invite);
      await b.page.goto(`${invite}&${q}`);
      await b.page.waitForFunction(() => window.mvp?.net?.player === 2, null, { timeout: 30000 });
      // Each page's lobby line sees its own pad before the game starts.
      await connect(a.page, 0, PADS.ds4);
      await connect(b.page, 0, PADS.xbox);
      await pad(b.page, 0, { 0: 1 });
      await sleep(400);
      eq(await b.page.$eval('#online-pad', (e) => e.dataset.state), 'ready', 'player 2 lobby: controller ready');
      await rest(b.page, 0);
      // ... and each sees the other's: player 1 learns that player 2's controller is ready, then player 2 that
      // player 1 is on the keyboard until the DualShock is used.
      const peer = (page) => page.waitForFunction(() => document.getElementById('online-peer-pad')?.dataset.state, null, { timeout: 30000 })
        .then(() => page.$eval('#online-peer-pad', (e) => [e.dataset.state, e.textContent]));
      eq((await peer(a.page))[0], 'ready', 'player 1 sees that player 2 has a controller ready');
      eq(await peer(b.page), ['keyboard', 'Player 1 is on the keyboard.'], 'player 2 sees player 1 on the keyboard');
      await pad(a.page, 0, { 0: 1 });
      await b.page.waitForFunction(() => document.getElementById('online-peer-pad')?.dataset.state === 'ready', null, { timeout: 30000 });
      await rest(a.page, 0);
      await Promise.all(pages.map((p) => p.page.setInputFiles('#disc', disc)));
      for (const p of pages) await p.page.waitForFunction(() => window.mvp.phases.boot !== undefined || !!window.mvp.error, null, { timeout: 300000 });
      await waitFrame(b.page, 1150);
      await hold(b.page, 0, { 9: 1 });                                    // player 2's START
      await waitFrame(b.page, 1700);
      await hold(a.page, 0, { 9: 1 });                                    // player 1's START (a DualShock, its first press)
      await hold(b.page, 0, { 0: 1 });
      const [ra, rb] = await Promise.all([stopped(a.page), stopped(b.page)]);
      console.log(`  p1: ${ra.guestLine}\n  p2: ${rb.guestLine}`);
      eq(ra.guestLine, rb.guestLine, 'both games ended in the same guest state');
      eq(ra.inputLog, rb.inputLog, 'both guests consumed the same inputs');
      const c = commands(ra.inputLog);
      assert(c.includes('2 press START') && c.includes('2 press A'), `player 2's pad is on port 2 (log: ${c.join(' | ')})`);
      assert(c.includes('1 press START'), "player 1's pad is on port 1");
      eq([a.errors, b.errors], [[], []], 'no page errors');
    } finally {
      for (const b of browsers) await b.close();
      srv.child.kill();
    }
  });
}

// ---- run ----
const siteServer = startServer(process.execPath, [join(here, 'serve.mjs'), '--port', '0', '--root', site]);
baseUrl = await siteServer.url;
const browser = await launchers[browserName]();
let failed = 0;
const results = [];
for (const { name, fn } of checks) {
  if (only.length && !only.includes(name)) continue;
  const t0 = Date.now();
  const context = await browser.newContext({ viewport: { width: 1000, height: 1000 } });
  console.log(`running ${name} ...`);
  try {
    await fn(context);
    results.push({ name, ok: true, seconds: (Date.now() - t0) / 1000 });
    console.log(`ok     ${name} (${((Date.now() - t0) / 1000).toFixed(0)} s)`);
  } catch (e) {
    failed++;
    results.push({ name, ok: false, error: String(e.message ?? e) });
    console.log(`FAILED ${name}: ${String(e.stack ?? e)}`);
  }
  await context.close();
}
await browser.close();
siteServer.child.kill();
writeFileSync(join(out, `game-summary-${browserName}.json`), JSON.stringify({ browser: browserName, frames, results }, null, 2));
console.log(`${results.length - failed} of ${results.length} game checks passed`);
process.exit(failed ? 1 : 0);
