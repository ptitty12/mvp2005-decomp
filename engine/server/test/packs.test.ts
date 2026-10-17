// Tests for the roster packs on the server (deploy): src/pack_store.ts and the
// /api/packs routes. No game data: the packs are tiny synthetic zips built here.
import assert from 'node:assert/strict';
import { mkdirSync, mkdtempSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { after, before, describe, it } from 'node:test';
import { crc32, deflateRawSync } from 'node:zlib';

import { defaults, type ServerConfig } from '../src/config.js';
import { DISC_PAGE_CSP } from '../src/disc_page.js';
import { checkPack, readZipManifest } from '../src/pack_store.js';
import { startServer, type App } from '../src/server.js';

const PASSWORD = 'pack tests 12345';
const dir = mkdtempSync(join(tmpdir(), 'mvp-pack-test-'));
const site = join(dir, 'site');
let counter = 0;
before(() => {
  mkdirSync(site, { recursive: true });
  writeFileSync(join(site, 'index.html'), '<!doctype html><title>engine</title>');
});
after(() => rmSync(dir, { recursive: true, force: true }));

/** A zip of `entries` (stored or deflated), with a correct central directory. */
function makeZip(entries: { name: string; data: Buffer | string; deflate?: boolean }[]): Buffer {
  const locals: Buffer[] = [];
  const centrals: Buffer[] = [];
  let offset = 0;
  for (const e of entries) {
    const raw = Buffer.isBuffer(e.data) ? e.data : Buffer.from(e.data, 'utf8');
    const body = e.deflate ? deflateRawSync(raw) : raw;
    const name = Buffer.from(e.name, 'utf8');
    const crc = crc32(raw);
    const lh = Buffer.alloc(30);
    lh.writeUInt32LE(0x04034b50, 0);
    lh.writeUInt16LE(20, 4);
    lh.writeUInt16LE(e.deflate ? 8 : 0, 8);
    lh.writeUInt32LE(crc, 14);
    lh.writeUInt32LE(body.length, 18);
    lh.writeUInt32LE(raw.length, 22);
    lh.writeUInt16LE(name.length, 26);
    const ch = Buffer.alloc(46);
    ch.writeUInt32LE(0x02014b50, 0);
    ch.writeUInt16LE(20, 4);
    ch.writeUInt16LE(20, 6);
    ch.writeUInt16LE(e.deflate ? 8 : 0, 10);
    ch.writeUInt32LE(crc, 16);
    ch.writeUInt32LE(body.length, 20);
    ch.writeUInt32LE(raw.length, 24);
    ch.writeUInt16LE(name.length, 28);
    ch.writeUInt32LE(offset, 42);
    locals.push(lh, name, body);
    centrals.push(ch, name);
    offset += lh.length + name.length + body.length;
  }
  const cd = Buffer.concat(centrals);
  const eocd = Buffer.alloc(22);
  eocd.writeUInt32LE(0x06054b50, 0);
  eocd.writeUInt16LE(entries.length, 8);
  eocd.writeUInt16LE(entries.length, 10);
  eocd.writeUInt32LE(cd.length, 12);
  eocd.writeUInt32LE(offset, 16);
  return Buffer.concat([...locals, cd, eocd]);
}

function pack(over: Record<string, unknown> = {}, opts: { folder?: string; deflate?: boolean } = {}): Buffer {
  const manifest = { id: 'test-league', name: 'Test League', version: '1.0.0', game: 'GV4E69-0', engine_api: '1.0',
    patches: ['patches/a.json'], ...over };
  const pre = opts.folder ? `${opts.folder}/` : '';
  return makeZip([
    { name: `${pre}pack.json`, data: JSON.stringify(manifest), deflate: opts.deflate ?? true },
    { name: `${pre}patches/a.json`, data: '{"ops": []}', deflate: opts.deflate ?? true },
  ]);
}

describe('pack zips', () => {
  it('reads pack.json at the top or in the one folder, stored or deflated', () => {
    for (const opts of [{}, { folder: 'test-league' }, { deflate: false }, { folder: 'x', deflate: false }]) {
      const r = readZipManifest(pack({}, opts));
      assert.ok('manifest' in r, JSON.stringify(r));
      assert.equal(r.manifest.id, 'test-league');
    }
  });
  it('refuses what is not a pack for this game', () => {
    const err = (zip: Buffer) => {
      const c = checkPack(zip, 'GV4E69-0');
      return 'error' in c ? c.error : '';
    };
    assert.match(err(Buffer.from('not a zip at all, just text')), /not a zip/);
    assert.match(err(makeZip([{ name: 'readme.txt', data: 'hi' }])), /no pack\.json/);
    assert.match(err(makeZip([{ name: 'a/pack.json', data: '{}' }, { name: 'b/x', data: '' }])), /no pack\.json/);
    assert.match(err(makeZip([{ name: 'pack.json', data: '{not json' }])), /not valid JSON/);
    assert.match(err(pack({ game: 'GXXE01-0' })), /for "GXXE01-0"/);
    assert.match(err(pack({ id: 'Bad_Id' })), /"id"/);
    assert.match(err(pack({ id: '../../etc' })), /"id"/);
    assert.match(err(pack({ version: '1.0' })), /"version"/);
    assert.match(err(pack({ name: '' })), /"name"/);
    assert.equal(err(pack()), '');
  });
});

describe('roster packs on the server', () => {
  const running: App[] = [];
  const start = async (over: Partial<ServerConfig> = {}) => {
    const app = await startServer({ ...defaults, site, password: PASSWORD, port: 0, packsDir: join(dir, `packs-${++counter}`), ...over },
      { log: () => {} });
    running.push(app);
    return app;
  };
  after(async () => {
    for (const a of running) await a.close();
  });
  const login = async (app: App) => {
    const r = await fetch(`${app.url}/login`, { method: 'POST', body: new URLSearchParams({ password: PASSWORD, next: '/' }), redirect: 'manual' });
    assert.equal(r.status, 303);
    return (r.headers.get('set-cookie') ?? '').split(';')[0]!;
  };
  const owner = async (app: App) => {
    const cookie = await login(app);
    const r = await fetch(`${app.url}/disc`, { headers: { Cookie: cookie } });
    assert.equal(r.status, 200);
    assert.equal(r.headers.get('content-security-policy'), DISC_PAGE_CSP);
    const html = await r.text();
    const token = /name="mvp-csrf" content="([^"]+)"/.exec(html)?.[1] ?? '';
    return { cookie, token, html };
  };
  const send = (app: App, o: { cookie: string; token: string }, method: string, path: string, body?: Buffer, type = 'application/zip',
                extra: Record<string, string> = {}) =>
    fetch(`${app.url}${path}`, { method, body, headers: { Cookie: o.cookie, 'X-MVP-CSRF': o.token, Origin: app.url, 'Content-Type': type, ...extra } });

  it('is off without a packs directory', async () => {
    const app = await start({ packsDir: '' });
    const cookie = await login(app);
    const cfg = await (await fetch(`${app.url}/config.json`, { headers: { Cookie: cookie } })).json() as Record<string, unknown>;
    assert.equal(cfg.packs, undefined);
    assert.equal((await fetch(`${app.url}/api/packs`, { headers: { Cookie: cookie } })).status, 404);
    assert.equal((await fetch(`${app.url}/disc`, { headers: { Cookie: cookie } })).status, 404);
  });

  it('needs the login, and names the packs and the page in /config.json', async () => {
    const app = await start();
    assert.equal((await fetch(`${app.url}/api/packs`)).status, 401);
    assert.equal((await fetch(`${app.url}/api/packs/test-league.zip`)).status, 401);
    const o = await owner(app);
    assert.match(o.html, /Roster packs/);
    assert.doesNotMatch(o.html, /Upload the disc image/);  // no disc directory: the packs part only
    const cfg = await (await fetch(`${app.url}/config.json`, { headers: { Cookie: o.cookie } })).json() as Record<string, unknown>;
    assert.equal(cfg.packs, '/api/packs');
    assert.equal(cfg.discPage, '/disc');
    const list = await (await fetch(`${app.url}/api/packs`, { headers: { Cookie: o.cookie } })).json() as { packs: unknown[] };
    assert.deepEqual(list.packs, []);
  });

  it('accepts uploads only from the Server disc page, and only packs for this game', async () => {
    const app = await start();
    const o = await owner(app);
    assert.equal((await send(app, { ...o, token: 'x'.repeat(43) }, 'POST', '/api/packs', pack())).status, 403);
    assert.equal((await send(app, o, 'POST', '/api/packs', pack(), 'application/zip', { 'Sec-Fetch-Site': 'cross-site' })).status, 403);
    assert.equal((await send(app, o, 'POST', '/api/packs', pack(), 'text/plain')).status, 415);
    const bad = await send(app, o, 'POST', '/api/packs', pack({ game: 'GXXE01-0' }));
    assert.equal(bad.status, 400);
    assert.match(((await bad.json()) as { error: string }).error, /Not a roster pack for this game/);
    assert.equal((await send(app, o, 'POST', '/api/packs', Buffer.from('plain text'))).status, 400);
    const small = await start({ maxPackBytes: 64 });
    const o2 = await owner(small);
    assert.equal((await send(small, o2, 'POST', '/api/packs', pack())).status, 413);
    const list = await (await fetch(`${app.url}/api/packs`, { headers: { Cookie: o.cookie } })).json() as { packs: unknown[] };
    assert.deepEqual(list.packs, []);
  });

  it('stores, lists, serves (with revalidation), replaces and removes a pack', async () => {
    const app = await start();
    const o = await owner(app);
    const zip = pack({}, { folder: 'test-league' });
    const up = await send(app, o, 'POST', '/api/packs', zip);
    assert.equal(up.status, 200);
    const info = ((await up.json()) as { pack: Record<string, unknown> }).pack;
    assert.equal(info.file, 'test-league.zip');
    assert.equal(info.name, 'Test League');
    assert.equal(info.size, zip.length);
    const list = (await (await fetch(`${app.url}/api/packs`, { headers: { Cookie: o.cookie } })).json()) as { packs: Record<string, unknown>[] };
    assert.equal(list.packs.length, 1);
    assert.deepEqual(list.packs[0], info);
    // download: the same bytes, an ETag, and a 304 for a page that has them
    const got = await fetch(`${app.url}/api/packs/test-league.zip`, { headers: { Cookie: o.cookie } });
    assert.equal(got.status, 200);
    assert.equal(got.headers.get('content-type'), 'application/zip');
    assert.deepEqual(Buffer.from(await got.arrayBuffer()), zip);
    const etag = got.headers.get('etag')!;
    assert.equal(etag, `"${info.sha256}"`);
    assert.equal((await fetch(`${app.url}/api/packs/test-league.zip`, { headers: { Cookie: o.cookie, 'If-None-Match': etag } })).status, 304);
    const head = await fetch(`${app.url}/api/packs/test-league.zip`, { method: 'HEAD', headers: { Cookie: o.cookie } });
    assert.equal(head.status, 200);
    assert.equal(head.headers.get('content-length'), String(zip.length));
    // a new version replaces the old one (same file)
    const v2 = pack({ version: '1.1.0' });
    assert.equal((await send(app, o, 'POST', '/api/packs', v2)).status, 200);
    const list2 = (await (await fetch(`${app.url}/api/packs`, { headers: { Cookie: o.cookie } })).json()) as { packs: Record<string, unknown>[] };
    assert.equal(list2.packs.length, 1);
    assert.equal(list2.packs[0]!.version, '1.1.0');
    assert.equal((await fetch(`${app.url}/api/packs/test-league.zip`, { headers: { Cookie: o.cookie, 'If-None-Match': etag } })).status, 200);
    // removal only from the page
    assert.equal((await send(app, { ...o, token: 'y'.repeat(43) }, 'DELETE', '/api/packs/test-league.zip')).status, 403);
    assert.equal((await send(app, o, 'DELETE', '/api/packs/test-league.zip')).status, 200);
    assert.equal((await fetch(`${app.url}/api/packs/test-league.zip`, { headers: { Cookie: o.cookie } })).status, 404);
    assert.equal((await send(app, o, 'DELETE', '/api/packs/test-league.zip')).status, 404);
    assert.deepEqual(readdirSync(join(dir, `packs-${counter}`)).filter((n) => !n.startsWith('.')), []);
  });

  it('serves only pack files of its own folder', async () => {
    const app = await start();
    const o = await owner(app);
    const folder = join(dir, `packs-${counter}`);
    writeFileSync(join(folder, 'junk.zip'), 'not a zip');           // not a pack: skipped, not served
    writeFileSync(join(dir, 'outside.zip'), pack());
    const cookie = { Cookie: o.cookie };
    for (const p of ['/api/packs/..%2Foutside.zip', '/api/packs/../outside.zip', '/api/packs/Test.zip', '/api/packs/junk.zip',
                     '/api/packs/%2E%2E%2Foutside.zip']) {
      assert.equal((await fetch(`${app.url}${p}`, { headers: cookie })).status, 404, p);
    }
    assert.equal((await fetch(`${app.url}/api/packs/%E0%A4%A.zip`, { headers: cookie })).status, 400);  // a broken escape
    const list = (await (await fetch(`${app.url}/api/packs`, { headers: cookie })).json()) as { packs: unknown[] };
    assert.deepEqual(list.packs, []);
  });
});
