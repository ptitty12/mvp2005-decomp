// Tests for the admin's memory card on the server (src/card_store.ts and its
// route). No game data: the cards are synthetic 8 MiB images.
import assert from 'node:assert/strict';
import { mkdirSync, mkdtempSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { after, before, describe, it } from 'node:test';
import { gzipSync } from 'node:zlib';

import { CARD_BYTES } from '../src/card_store.js';
import { defaults, type ServerConfig } from '../src/config.js';
import { startServer, type App } from '../src/server.js';

const KEY = 'card-tests-0123456789';
const ADMIN = 'card-tests-admin-0123456789';
const dir = mkdtempSync(join(tmpdir(), 'mvp-card-test-'));
const site = join(dir, 'site');
let counter = 0;

before(() => {
  mkdirSync(site, { recursive: true });
  writeFileSync(join(site, 'index.html'), '<!doctype html><title>engine</title>');
});
after(() => rmSync(dir, { recursive: true, force: true }));

/** A card: erased (0xFF) but for a few bytes that say which one it is. */
function card(mark: number): Buffer {
  const b = Buffer.alloc(CARD_BYTES, 0xff);
  b.writeUInt32BE(mark, 0x2000);
  return b;
}

describe('the admin\'s memory card on the server', () => {
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
  const admin = async (app: App) => {
    const r = await fetch(app.adminLink, { redirect: 'manual' });
    const cookie = r.headers.getSetCookie().map((c) => c.split(';')[0]).join('; ');
    const cfg = (await (await fetch(`${app.url}/config.json`, { headers: { Cookie: cookie } })).json()) as Record<string, unknown>;
    return { cookie, token: String(cfg.adminToken), cfg };
  };
  const put = (app: App, o: { cookie: string; token: string }, body: Buffer, extra: Record<string, string>) =>
    fetch(`${app.url}/api/card`, { method: 'PUT', body, headers: { Cookie: o.cookie, 'X-MVP-CSRF': o.token, Origin: app.url,
      'Content-Type': 'application/octet-stream', ...extra } });
  const get = (app: App, cookie?: string) => fetch(`${app.url}/api/card`, { headers: cookie ? { Cookie: cookie } : undefined });

  it('is the admin\'s only, and off without a states directory', async () => {
    const app = await start();
    const p = (await fetch(app.link, { redirect: 'manual' })).headers.get('set-cookie')!.split(';')[0]!;
    for (const cookie of [undefined, p]) {
      assert.equal((await get(app, cookie)).status, 401);
      const cfg = (await (await fetch(`${app.url}/config.json`, { headers: cookie ? { Cookie: cookie } : undefined })).json()) as
        Record<string, unknown>;
      assert.equal(cfg.card, undefined);
    }
    const o = await admin(app);
    assert.equal(o.cfg.card, '/api/card');
    assert.equal((await get(app, o.cookie)).status, 404);  // none yet
    // A write needs the admin's token, from this site.
    assert.equal((await put(app, { ...o, token: 'x'.repeat(43) }, card(1), { 'If-None-Match': '*' })).status, 403);
    assert.equal((await put(app, o, card(1), { 'If-None-Match': '*', Origin: 'https://evil.example' })).status, 403);
    assert.equal((await put(app, { ...o, cookie: p }, card(1), { 'If-None-Match': '*' })).status, 401);
    assert.deepEqual(readdirSync(join(dir, `states-${counter}`)), []);

    const off = await start({ statesDir: '' });
    const o2 = await admin(off);
    assert.equal(o2.cfg.card, undefined);
    assert.equal((await get(off, o2.cookie)).status, 404);
  });

  it('stores the card gzipped and serves it back, each write replacing the version it read', async () => {
    const app = await start();
    const o = await admin(app);
    // The first write: there is none yet.
    let r = await put(app, o, card(1), { 'If-None-Match': '*' });
    assert.equal(r.status, 200);
    const e1 = ((await r.json()) as { etag: string }).etag;
    assert.match(e1, /^"[0-9a-f]{64}"$/);
    assert.deepEqual(readdirSync(join(dir, `states-${counter}`)), ['memory-card-a.raw.gz']);
    r = await get(app, o.cookie);
    assert.equal(r.status, 200);
    assert.equal(r.headers.get('content-encoding'), 'gzip');
    assert.equal(r.headers.get('etag'), e1);
    assert.ok(Number(r.headers.get('content-length')) < 100_000);  // an erased card is mostly 0xFF
    assert.deepEqual(Buffer.from(await r.arrayBuffer()), card(1));
    // The states' list does not show it.
    const list = (await (await fetch(`${app.url}/api/states`, { headers: { Cookie: o.cookie } })).json()) as { states: unknown[] };
    assert.deepEqual(list.states, []);
    // A second "first" write, and a write over a version that is not the server's, are refused.
    r = await put(app, o, card(2), { 'If-None-Match': '*' });
    assert.equal(r.status, 412);
    assert.equal(((await r.json()) as { etag: string }).etag, e1);
    assert.equal((await put(app, o, card(2), { 'If-Match': `"${'0'.repeat(64)}"` })).status, 412);
    assert.equal((await put(app, o, card(2), {})).status, 428);
    // Over the version it read (gzipped this time): stored.
    r = await put(app, o, gzipSync(card(2)), { 'If-Match': e1 });
    assert.equal(r.status, 200);
    const e2 = ((await r.json()) as { etag: string }).etag;
    assert.notEqual(e2, e1);
    r = await get(app, o.cookie);
    assert.equal(r.headers.get('etag'), e2);
    assert.deepEqual(Buffer.from(await r.arrayBuffer()), card(2));
    // The page that still had the first version is told; the server keeps the second.
    assert.equal((await put(app, o, card(3), { 'If-Match': e1 })).status, 412);
    assert.deepEqual(Buffer.from(await (await get(app, o.cookie)).arrayBuffer()), card(2));
  });

  it('takes only a card\'s size', async () => {
    const app = await start();
    const o = await admin(app);
    for (const body of [Buffer.alloc(1000, 0xff), Buffer.alloc(CARD_BYTES + 1, 0xff), gzipSync(Buffer.alloc(CARD_BYTES * 2, 0xff)),
                        Buffer.from([0x1f, 0x8b, 1, 2, 3])]) {
      const r = await put(app, o, body, { 'If-None-Match': '*' });
      assert.equal(r.status, 400, String(body.length));
      assert.match(((await r.json()) as { error: string }).error, /Not a memory card/);
    }
    // Refused at once (413, Connection: close): a client still sending the body may see the connection reset instead.
    const big = await put(app, o, Buffer.alloc(CARD_BYTES + (2 << 20), 0xff), { 'If-None-Match': '*' }).then((r) => r.status, () => 'reset');
    assert.ok(big === 413 || big === 'reset', String(big));
    assert.equal((await get(app, o.cookie)).status, 404);
  });
});
