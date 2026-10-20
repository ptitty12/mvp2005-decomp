// Tests for the admin's save states on the server (src/state_store.ts and its
// routes). No game data: the states are synthetic files in the save-state
// format (magic, version, JSON header, then bytes).
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { mkdirSync, mkdtempSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { after, before, describe, it } from 'node:test';
import { gunzipSync, gzipSync } from 'node:zlib';

import { defaults, type ServerConfig } from '../src/config.js';
import { DISC_PAGE_CSP } from '../src/disc_page.js';
import { startServer, type App } from '../src/server.js';
import { readStateHeader } from '../src/state_store.js';

const KEY = 'state-tests-0123456789';
const ADMIN = 'state-tests-admin-0123456789';
const dir = mkdtempSync(join(tmpdir(), 'mvp-state-test-'));
const site = join(dir, 'site');
let counter = 0;

before(() => {
  mkdirSync(site, { recursive: true });
  writeFileSync(join(site, 'index.html'), '<!doctype html><title>engine</title>');
});
after(() => rmSync(dir, { recursive: true, force: true }));

/** A save-state file: "MVPSTATE", version, the header's length, the header, then `body` bytes of a pattern. */
function state(header: Record<string, unknown> = {}, body = 200_000, version = 1): Buffer {
  const h = Buffer.from(JSON.stringify({ game: 'GV4E69-0', label: 'Top 9th', frame_polls: 136350, mods: ['abs:miss=8'],
    packs: { files: ['mlb-2026.zip'] }, ...header }));
  const head = Buffer.alloc(20);
  head.write('MVPSTATE', 0, 'latin1');
  head.writeUInt32LE(version, 8);
  head.writeBigUInt64LE(BigInt(h.length), 12);
  const rest = Buffer.alloc(body);
  for (let i = 0; i < body; ++i) rest[i] = (i * 7) & 0xff;
  return Buffer.concat([head, h, rest]);
}

describe('save-state files', () => {
  it('reads the header and refuses what is not a state of this version', () => {
    const h = readStateHeader(state());
    assert.equal(typeof h, 'object');
    assert.equal((h as { label: string }).label, 'Top 9th');
    assert.match(readStateHeader(Buffer.from('not a state at all, really')) as string, /not a save state/);
    assert.equal(typeof readStateHeader(state({}, 10, 2)), 'object');  // version 2: with the card's part
    assert.match(readStateHeader(state({}, 10, 3)) as string, /version 3/);
    assert.match(readStateHeader(state().subarray(0, 40)) as string, /cut short/);
  });
});

describe('save states on the server', () => {
  const running: App[] = [];
  const start = async (over: Partial<ServerConfig> = {}) => {
    const app = await startServer({ ...defaults, site, accessKey: KEY, adminKey: ADMIN, port: 0,
      statesDir: join(dir, `states-${++counter}`), ...over }, { log: () => {} });
    running.push(app);
    return app;
  };
  after(async () => {
    for (const a of running) await a.close();
  });
  const player = async (app: App) => {
    const r = await fetch(app.link, { redirect: 'manual' });
    return (r.headers.get('set-cookie') ?? '').split(';')[0]!;
  };
  /** The admin: its cookies, and the CSRF token /config.json gives the game page. */
  const admin = async (app: App) => {
    const r = await fetch(app.adminLink, { redirect: 'manual' });
    const cookie = r.headers.getSetCookie().map((c) => c.split(';')[0]).join('; ');
    const cfg = (await (await fetch(`${app.url}/config.json`, { headers: { Cookie: cookie } })).json()) as Record<string, unknown>;
    return { cookie, token: String(cfg.adminToken), cfg };
  };
  const send = (app: App, o: { cookie: string; token: string }, method: string, path: string, body?: Buffer,
                extra: Record<string, string> = {}) =>
    fetch(`${app.url}${path}`, { method, body, headers: { Cookie: o.cookie, 'X-MVP-CSRF': o.token, Origin: app.url,
      'Content-Type': 'application/octet-stream', ...extra } });

  it('is off without a states directory or without an admin', async () => {
    for (const over of [{ statesDir: '' }, { adminKey: '' }]) {
      const app = await start(over);
      const cookie = over.adminKey === '' ? await player(app) : (await admin(app)).cookie;
      const cfg = (await (await fetch(`${app.url}/config.json`, { headers: { Cookie: cookie } })).json()) as Record<string, unknown>;
      assert.equal(cfg.states, undefined);
      assert.equal((await fetch(`${app.url}/api/states`, { headers: { Cookie: cookie } })).status, 404);
    }
  });

  it('is the admin\'s only', async () => {
    const app = await start();
    const p = await player(app);
    for (const cookie of [undefined, p]) {
      const headers = cookie ? { Cookie: cookie } : undefined;
      for (const path of ['/api/states', '/api/states/quick', '/states/quick.mvpstate']) {
        assert.equal((await fetch(`${app.url}${path}`, { headers })).status, 401, path);
      }
      const cfg = (await (await fetch(`${app.url}/config.json`, { headers })).json()) as Record<string, unknown>;
      assert.equal(cfg.states, undefined);
      assert.equal(cfg.admin, undefined);
    }
    const o = await admin(app);
    assert.equal(o.cfg.states, '/api/states');
    assert.equal(o.cfg.discPage, '/disc');
    // Writes need the admin's token, from this site; a player's cookie with the token is not the admin.
    assert.equal((await send(app, { ...o, token: 'x'.repeat(43) }, 'POST', '/api/states/quick', state())).status, 403);
    assert.equal((await send(app, o, 'POST', '/api/states/quick', state(), { 'Sec-Fetch-Site': 'cross-site' })).status, 403);
    assert.equal((await send(app, o, 'POST', '/api/states/quick', state(), { Origin: 'https://evil.example' })).status, 403);
    assert.equal((await send(app, { ...o, cookie: p }, 'POST', '/api/states/quick', state())).status, 401);
    assert.deepEqual(readdirSync(join(dir, `states-${counter}`)), []);
  });

  it('stores (gzipped), lists, serves, replaces and removes a state', async () => {
    const app = await start();
    const o = await admin(app);
    const raw = state();
    const up = await send(app, o, 'POST', '/api/states/braves-dodgers-9th', raw);
    assert.equal(up.status, 200);
    const info = ((await up.json()) as { state: Record<string, unknown> }).state;
    assert.equal(info.name, 'braves-dodgers-9th');
    assert.equal(info.label, 'Top 9th');
    assert.equal(info.frame, 136350);
    assert.deepEqual(info.mods, ['abs:miss=8']);
    assert.deepEqual(info.packs, ['mlb-2026.zip']);
    const folder = join(dir, `states-${counter}`);
    assert.deepEqual(readdirSync(folder), ['braves-dodgers-9th.mvpstate.gz']);
    assert.ok((info.size as number) < raw.length / 2, String(info.size));
    const list = (await (await fetch(`${app.url}/api/states`, { headers: { Cookie: o.cookie } })).json()) as { states: unknown[] };
    assert.deepEqual(list.states, [info]);
    // Served gzipped (Content-Encoding): the browser, and fetch here, get the file's own bytes.
    for (const path of ['/api/states/braves-dodgers-9th', '/states/braves-dodgers-9th.mvpstate']) {
      const r = await fetch(`${app.url}${path}`, { headers: { Cookie: o.cookie } });
      assert.equal(r.status, 200, path);
      assert.equal(r.headers.get('content-encoding'), 'gzip');
      assert.equal(r.headers.get('cache-control'), 'private, no-store');
      assert.equal(createHash('sha256').update(Buffer.from(await r.arrayBuffer())).digest('hex'),
        createHash('sha256').update(raw).digest('hex'), path);
    }
    // A gzipped upload is kept as it came; the same name replaces the state.
    const raw2 = state({ label: 'Again' });
    const gz = gzipSync(raw2, { level: 1 });
    assert.equal((await send(app, o, 'POST', '/api/states/braves-dodgers-9th', gz)).status, 200);
    const list2 = (await (await fetch(`${app.url}/api/states`, { headers: { Cookie: o.cookie } })).json()) as { states: { label: string; size: number }[] };
    assert.equal(list2.states.length, 1);
    assert.equal(list2.states[0]!.label, 'Again');
    assert.equal(list2.states[0]!.size, gz.length);
    const r = await fetch(`${app.url}/api/states/braves-dodgers-9th`, { headers: { Cookie: o.cookie } });
    assert.equal(r.headers.get('content-length'), String(gz.length));  // the stored bytes, as they came
    assert.deepEqual(Buffer.from(await r.arrayBuffer()), raw2);
    assert.deepEqual(gunzipSync(gz), raw2);
    // Removal.
    assert.equal((await send(app, o, 'DELETE', '/api/states/braves-dodgers-9th')).status, 200);
    assert.equal((await fetch(`${app.url}/states/braves-dodgers-9th.mvpstate`, { headers: { Cookie: o.cookie } })).status, 404);
    assert.equal((await send(app, o, 'DELETE', '/api/states/braves-dodgers-9th')).status, 404);
    assert.deepEqual(readdirSync(folder).filter((n) => !n.startsWith('.')), []);
  });

  it('takes only save states, by valid names, up to the size limit', async () => {
    const app = await start({ maxStateBytes: 100_000 });
    const o = await admin(app);
    let r = await send(app, o, 'POST', '/api/states/x', Buffer.from('plain text, not a state'));
    assert.equal(r.status, 400);
    assert.match(((await r.json()) as { error: string }).error, /Not a save state/);
    r = await send(app, o, 'POST', '/api/states/x', state({}, 10, 3));
    assert.match(((await r.json()) as { error: string }).error, /version 3/);
    r = await send(app, o, 'POST', '/api/states/x', Buffer.from([0x1f, 0x8b, 1, 2, 3]));
    assert.equal(r.status, 400);
    assert.equal((await send(app, o, 'POST', '/api/states/x', state({}, 200_000))).status, 413);
    assert.equal((await send(app, o, 'POST', '/api/states/x', gzipSync(state({}, 200_000)))).status, 413);  // too large inflated
    for (const name of ['Bad', 'a--b', '-a', '..%2Fx', 'a.b', 'x'.repeat(65)]) {
      assert.equal((await send(app, o, 'POST', `/api/states/${name}`, state({}, 10))).status, 404, name);
    }
    assert.equal((await fetch(`${app.url}/states/..%2Fx.mvpstate`, { headers: { Cookie: o.cookie } })).status, 404);
    const list = (await (await fetch(`${app.url}/api/states`, { headers: { Cookie: o.cookie } })).json()) as { states: unknown[] };
    assert.deepEqual(list.states, []);
  });

  it('has a Save states card on the Server disc page, its script pinned', async () => {
    const app = await start();
    const o = await admin(app);
    const r = await fetch(`${app.url}/disc`, { headers: { Cookie: o.cookie } });
    assert.equal(r.status, 200);
    const html = await r.text();
    assert.match(html, /<h2>Save states<\/h2>/);
    assert.match(html, /id="states-list"/);
    for (const [, script] of html.matchAll(/<script>([\s\S]*?)<\/script>/g)) {
      const hash = `'sha256-${createHash('sha256').update(script!).digest('base64')}'`;
      assert.ok(DISC_PAGE_CSP.includes(hash), hash);
    }
    assert.equal([...html.matchAll(/<script>/g)].length, 2);  // the link card and the states
  });
});
