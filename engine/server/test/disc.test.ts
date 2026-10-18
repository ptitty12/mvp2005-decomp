// Tests for the server's disc identity check and the Server disc upload
// (deploy): src/disc_image.ts, src/disc_store.ts and their routes. No game
// data: the images are tiny synthetic GCM and CISO files with a fake main.dol,
// checked against a test-only expected SHA-1. The same fake disc with the
// retail expectation shows that the real check refuses it.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { existsSync, mkdirSync, mkdtempSync, readdirSync, readFileSync, rmSync, statSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { after, before, describe, it } from 'node:test';
import WebSocket from 'ws';

import { defaults, loadConfig, type ServerConfig } from '../src/config.js';
import {
  CISO_TRAILER_MAX,
  DiscRejected,
  GAMECUBE_DISC_SIZE,
  NotYet,
  RETAIL_DISC,
  identifyDisc,
  type ExpectedDisc,
  type Reader,
} from '../src/disc_image.js';
import { ManifestCache, PIECE_BYTES, hashImage, imageHash } from '../src/disc_hash.js';
import { DISC_PAGE_CSP } from '../src/disc_page.js';
import { CHUNK_MAX } from '../src/disc_store.js';
import type { ServerMsg } from '../src/lobby_protocol.js';
import { startServer, type App } from '../src/server.js';
import { iceServers } from '../src/turn.js';

const KEY = 'disc-upload-tests-0123456789';
const dir = mkdtempSync(join(tmpdir(), 'mvp-disc-test-'));
const site = join(dir, 'site');
let counter = 0;
before(() => {
  mkdirSync(site, { recursive: true });
  writeFileSync(join(site, 'index.html'), '<!doctype html><title>engine</title>');
});
after(() => rmSync(dir, { recursive: true, force: true }));

// ---- synthetic images ----

/** Deterministic filler bytes (a SHA-256 chain), so the fake DOL has a fixed hash. */
function filler(seed: string, n: number): Buffer {
  const out = Buffer.alloc(n);
  let block = createHash('sha256').update(seed).digest();
  for (let i = 0; i < n; i += 32) {
    block.copy(out, i, 0, Math.min(32, n - i));
    block = createHash('sha256').update(block).digest();
  }
  return out;
}

interface FakeDisc {
  iso: Buffer;
  dolSha1: string;
}

/**
 * A 256 KiB GameCube disc: boot.bin with the game id, revision and magic, an FST
 * with two files, and a fake main.dol (one text and one data section).
 */
function fakeDisc(opts: { gameId?: string; revision?: number; magic?: number; dolSeed?: string } = {}): FakeDisc {
  const iso = Buffer.alloc(0x40000);
  iso.write(opts.gameId ?? 'GV4E69', 0, 'latin1');
  iso[7] = opts.revision ?? 0;
  iso.writeUInt32BE(opts.magic ?? 0xc2339f3d, 0x1c);
  iso.write('SYNTHETIC TEST DISC', 0x20, 'latin1');
  const dolOff = 0x10000, fstOff = 0x9000;
  // FST: root + two files, then the string table.
  const names = Buffer.from('a.bin\0b.bin\0', 'latin1');
  const fst = Buffer.alloc(3 * 12 + names.length);
  fst[0] = 1;
  fst.writeUInt32BE(3, 8);
  fst.writeUInt32BE(0, 12);  // file, name 0
  fst.writeUInt32BE(0x20000, 16);
  fst.writeUInt32BE(0x1000, 20);
  fst.writeUInt32BE(6, 24);  // file, name 6
  fst.writeUInt32BE(0x30000, 28);
  fst.writeUInt32BE(0x800, 32);
  names.copy(fst, 36);
  fst.copy(iso, fstOff);
  iso.writeUInt32BE(dolOff, 0x420);
  iso.writeUInt32BE(fstOff, 0x424);
  iso.writeUInt32BE(fst.length, 0x428);
  iso.writeUInt32BE(fst.length, 0x42c);
  // DOL: text0 at file 0x100 (0x2000 bytes), data0 at 0x2100 (0x1000 bytes).
  const dol = Buffer.alloc(0x3100);
  filler(opts.dolSeed ?? 'fake main.dol', 0x3000).copy(dol, 0x100);
  dol.writeUInt32BE(0x100, 0);
  dol.writeUInt32BE(0x80003100, 0x48);
  dol.writeUInt32BE(0x2000, 0x90);
  dol.writeUInt32BE(0x2100, 4 * 7);
  dol.writeUInt32BE(0x80100000, 0x48 + 4 * 7);
  dol.writeUInt32BE(0x1000, 0x90 + 4 * 7);
  dol.writeUInt32BE(0x80003100, 0xe0);
  dol.copy(iso, dolOff);
  filler('file a', 0x1000).copy(iso, 0x20000);
  filler('file b', 0x800).copy(iso, 0x30000);
  return { iso, dolSha1: createHash('sha1').update(dol).digest('hex') };
}

/** The same disc as a CISO: 32 KiB blocks, all-zero blocks left out, `trailer` extra bytes at the end. */
function toCiso(iso: Buffer, trailer = 0): Buffer {
  const bs = 0x8000;
  const header = Buffer.alloc(0x8000);
  header.write('CISO', 0, 'latin1');
  header.writeUInt32LE(bs, 4);
  const blocks: Buffer[] = [];
  for (let i = 0; i * bs < iso.length; ++i) {
    const b = iso.subarray(i * bs, (i + 1) * bs);
    if (b.some((x) => x !== 0)) {
      header[8 + i] = 1;
      blocks.push(b);
    }
  }
  return Buffer.concat([header, ...blocks, filler('trailer', trailer)]);
}

const memReader = (b: Buffer, available = b.length, size = b.length): Reader => ({
  size,
  async read(offset, length) {
    if (offset + length > available) throw new NotYet();
    if (offset + length > b.length) return Buffer.alloc(length);
    return b.subarray(offset, offset + length);
  },
});

const good = fakeDisc();
const EXPECT: ExpectedDisc = { gameId: 'GV4E69', revision: 0, dolSha1: good.dolSha1 };

describe('disc identity check', () => {
  it('accepts the expected disc as ISO and as CISO', async () => {
    const iso = await identifyDisc(memReader(good.iso), EXPECT);
    assert.deepEqual(iso, { format: 'iso', fileSize: good.iso.length, gameId: 'GV4E69', revision: 0, dolSha1: good.dolSha1, dolSize: 0x3100 });
    const cisoBytes = toCiso(good.iso, 576);  // the owner's CISO has a 576-byte footer
    const ciso = await identifyDisc(memReader(cisoBytes), EXPECT);
    assert.equal(ciso.format, 'ciso');
    assert.equal(ciso.dolSha1, good.dolSha1);
  });

  it('refuses the synthetic disc under the retail expectation (main.dol SHA-1)', async () => {
    await assert.rejects(identifyDisc(memReader(good.iso), RETAIL_DISC), (e: Error) => {
      assert.ok(e instanceof DiscRejected);
      assert.match(e.message, new RegExp(`main.dol SHA-1 ${good.dolSha1} is not GV4E69 rev 0`));
      return true;
    });
    assert.equal(RETAIL_DISC.dolSha1, 'da6becdbea614d03c4b5eae9f8a0fb08e0a184dd');
  });

  it('refuses other games, revisions, bad magic, another DOL and random bytes', async () => {
    const cases: [Buffer, RegExp][] = [
      [fakeDisc({ gameId: 'GALE01' }).iso, /not GV4E69: game id GALE01/],
      [fakeDisc({ gameId: '\u0001\u0002ABCD' }).iso, /game id 0x0102/],
      [fakeDisc({ revision: 1 }).iso, /not revision 0 \(revision 1\)/],
      [fakeDisc({ magic: 0 }).iso, /bad disc magic/],
      [fakeDisc({ dolSeed: 'another dol' }).iso, /main.dol SHA-1 [0-9a-f]{40} is not GV4E69 rev 0/],
      [filler('random', 0x40000), /bad disc magic/],
    ];
    for (const [img, why] of cases) {
      await assert.rejects(identifyDisc(memReader(img), EXPECT), (e: Error) => e instanceof DiscRejected && why.test(e.message));
      await assert.rejects(identifyDisc(memReader(toCiso(img)), EXPECT), (e: Error) => e instanceof DiscRejected && why.test(e.message));
    }
  });

  it('refuses malformed containers and anything carried behind the disc', async () => {
    const ciso = toCiso(good.iso);
    const badBs = Buffer.from(ciso);
    badBs.writeUInt32LE(3, 4);
    const badMap = Buffer.from(ciso);
    badMap[9] = 2;
    const cases: [Reader, RegExp][] = [
      [memReader(badBs), /block size is not a power of two/],
      [memReader(badMap), /bad block map entry/],
      [memReader(toCiso(good.iso, CISO_TRAILER_MAX + 1)), /bytes after the last stored block/],
      [memReader(ciso.subarray(0, ciso.length - 100)), /the stored blocks need/],
      [memReader(good.iso, good.iso.length, GAMECUBE_DISC_SIZE + 1), /larger than a GameCube disc/],
    ];
    for (const [r, why] of cases) await assert.rejects(identifyDisc(r, EXPECT), (e: Error) => e instanceof DiscRejected && why.test(e.message));
    // An FST file outside the disc, a directory with a bad end index.
    const outside = Buffer.from(good.iso);
    outside.writeUInt32BE(0x3ff000, 0x9000 + 12 + 4);
    await assert.rejects(identifyDisc(memReader(outside), EXPECT), /a file lies outside the disc/);
    const badDir = Buffer.from(good.iso);
    badDir[0x9000 + 12] = 1;
    badDir.writeUInt32BE(1, 0x9000 + 12 + 8);
    await assert.rejects(identifyDisc(memReader(badDir), EXPECT), /bad directory end index/);
  });

  it('needs only the start of an upload, and says so until it has it', async () => {
    for (const img of [good.iso, toCiso(good.iso)]) {
      await assert.rejects(identifyDisc(memReader(img, 0x1000, img.length), EXPECT), NotYet);
      // ISO: the DOL ends at 0x13100. CISO: the header plus the blocks up to the DOL's.
      const enough = img === good.iso ? 0x13100 : 0x8000 + 3 * 0x8000;
      assert.equal((await identifyDisc(memReader(img, enough, img.length), EXPECT)).dolSha1, good.dolSha1);
    }
    // A wrong game is known from the first 0x440 bytes.
    await assert.rejects(identifyDisc(memReader(fakeDisc({ gameId: 'GALE01' }).iso, 0x440, 0x40000), EXPECT), DiscRejected);
  });
});

// ---- the upload ----

function config(over: Partial<ServerConfig> = {}): ServerConfig {
  return { ...defaults, site, accessKey: KEY, port: 0, discDir: join(dir, `disc-${++counter}`), expectDisc: EXPECT, ...over };
}

/** Opens the private link as a browser would; returns the access cookie. */
async function enter(app: App): Promise<string> {
  const r = await fetch(app.link, { redirect: 'manual' });
  assert.equal(r.status, 303);
  return (r.headers.get('set-cookie') ?? '').split(';')[0]!;
}

/** A client of the Server disc page with access: its cookie and CSRF token. */
class Owner {
  constructor(readonly app: App, readonly cookie: string, readonly token: string) {}
  static async open(app: App): Promise<Owner> {
    const cookie = await enter(app);
    const r = await fetch(`${app.url}/disc`, { headers: { Cookie: cookie } });
    assert.equal(r.status, 200);
    assert.equal(r.headers.get('content-security-policy'), DISC_PAGE_CSP);
    const token = /name="mvp-csrf" content="([^"]+)"/.exec(await r.text())?.[1] ?? '';
    assert.match(token, /^[A-Za-z0-9_-]{43}$/);
    return new Owner(app, cookie, token);
  }
  api(method: string, path: string, body?: RequestInit['body'], type?: string, extra: Record<string, string> = {}): Promise<Response> {
    const headers: Record<string, string> = { Cookie: this.cookie, 'X-MVP-CSRF': this.token, Origin: this.app.url, ...extra };
    if (type) headers['Content-Type'] = type;
    return fetch(`${this.app.url}${path}`, { method, headers, body });
  }
  async status(): Promise<Record<string, any>> {
    return (await (await this.api('GET', '/api/disc')).json()) as Record<string, any>;
  }
  async create(file: Buffer, fingerprint = 'f'): Promise<Response> {
    return this.api('POST', '/api/disc/uploads', JSON.stringify({ size: file.length, name: 'test.iso', fingerprint }), 'application/json');
  }
  put(id: string, offset: number, bytes: Buffer): Promise<Response> {
    return this.api('PUT', `/api/disc/uploads/${id}?offset=${offset}`, bytes, 'application/octet-stream');
  }
  /** Sends `file` from `from` in chunks; returns the last chunk's answer (stops at the first non-200). */
  async send(id: string, file: Buffer, from = 0, chunk = 16384): Promise<{ status: number; body: Record<string, any> }> {
    let last = { status: 0, body: {} as Record<string, any> };
    for (let off = from; off < file.length; off += chunk) {
      const r = await this.put(id, off, file.subarray(off, Math.min(file.length, off + chunk)));
      last = { status: r.status, body: (await r.json()) as Record<string, any> };
      if (r.status !== 200) break;
    }
    return last;
  }
  finish(id: string): Promise<Response> {
    return this.api('POST', `/api/disc/uploads/${id}/finish`);
  }
}

describe('Server disc upload', () => {
  const running: App[] = [];
  const start = async (over: Partial<ServerConfig> = {}) => {
    const app = await startServer(config(over), { log: () => {} });
    running.push(app);
    return app;
  };
  after(async () => {
    for (const a of running) await a.close();
  });

  it('needs access, and is off without a disc directory', async () => {
    const app = await start();
    for (const p of ['/api/disc', '/api/disc/uploads']) {
      const r = await fetch(`${app.url}${p}`, { method: p.endsWith('uploads') ? 'POST' : 'GET' });
      assert.equal(r.status, 401, p);
    }
    const off = await start({ discDir: '' });
    const cookie = await enter(off);
    assert.equal((await fetch(`${off.url}/disc`, { headers: { Cookie: cookie } })).status, 404);
    assert.deepEqual(await (await fetch(`${off.url}/config.json`, { headers: { Cookie: cookie } })).json(), {});
    const r = await fetch(`${off.url}/api/disc/uploads`, { method: 'POST', headers: { Cookie: cookie } });
    assert.equal(r.status, 405);
  });

  it('refuses requests without the page token or from another site', async () => {
    const app = await start();
    const o = await Owner.open(app);
    const body = JSON.stringify({ size: good.iso.length, name: 'x', fingerprint: 'x' });
    let r = await o.api('POST', '/api/disc/uploads', body, 'application/json', { 'X-MVP-CSRF': 'A'.repeat(43) });
    assert.equal(r.status, 403);
    r = await o.api('POST', '/api/disc/uploads', body, 'application/json', { Origin: 'https://evil.example' });
    assert.equal(r.status, 403);
    r = await o.api('POST', '/api/disc/uploads', body, 'application/json', { 'Sec-Fetch-Site': 'cross-site' });
    assert.equal(r.status, 403);
    r = await o.api('POST', '/api/disc/uploads', body, 'text/plain');
    assert.equal(r.status, 415);
    // The token is bound to the access cookie (the same for every browser with access): a server with another key
    // has another cookie and another token.
    const other = await start({ accessKey: 'another-key-0123456789' });
    const elsewhere = await Owner.open(other);
    assert.notEqual(elsewhere.token, o.token);
    r = await o.api('POST', '/api/disc/uploads', body, 'application/json', { 'X-MVP-CSRF': elsewhere.token });
    assert.equal(r.status, 403);
    assert.equal((await o.status()).upload, null);
  });

  it('uploads in chunks, checks the identity early, resumes after a restart, and replaces atomically', async () => {
    const discDir = join(dir, 'resume');
    let app = await start({ discDir });
    let o = await Owner.open(app);
    assert.deepEqual(await (await o.api('GET', '/config.json')).json(), { discPage: '/disc' });
    let st = await o.status();
    assert.equal(st.disc, null);
    assert.equal(st.upload, null);
    assert.deepEqual(st.limits, { maxBytes: defaults.maxDiscBytes, chunkBytes: CHUNK_MAX });

    const file = toCiso(good.iso, 576);
    let r = await o.create(file, 'name|size|mtime');
    assert.equal(r.status, 201);
    const id = ((await r.json()) as { upload: { id: string } }).upload.id;
    // Only the server's offset is accepted.
    r = await o.put(id, 4096, file.subarray(4096, 8192));
    assert.equal(r.status, 409);
    assert.deepEqual(await r.json(), { error: 'The server has a different number of bytes.', received: 0 });
    // Half the file: the identity is confirmed as soon as main.dol has arrived.
    const half = 0x8000 * 4;
    let last = await o.send(id, file.subarray(0, half));
    assert.equal(last.status, 200);
    assert.deepEqual(last.body, { received: half, identity: 'ok' });
    // A chunk past the end of the file is refused.
    r = await o.put(id, half, Buffer.alloc(file.length - half + 1));
    assert.equal(r.status, 413);

    // The server restarts: the upload continues where it stopped. The page's cookie and token still work (an open
    // page goes on uploading).
    await app.close();
    app = await start({ discDir });
    o = new Owner(app, o.cookie, o.token);
    st = await o.status();
    assert.equal(st.upload.received, half);
    assert.equal(st.upload.fingerprint, 'name|size|mtime');
    assert.equal(st.upload.id, id);
    last = await o.send(id, file, half);
    assert.equal(last.status, 200);
    assert.equal(last.body.received, file.length);
    r = await o.finish(id);
    assert.equal(r.status, 200);
    st = (await r.json()) as Record<string, any>;
    assert.equal(st.disc.format, 'ciso');
    assert.equal(st.disc.size, file.length);
    assert.equal(st.disc.dolSha1, good.dolSha1);
    assert.equal(st.upload, null);
    assert.deepEqual(readdirSync(join(discDir, 'upload')), []);
    assert.ok(readFileSync(join(discDir, 'GV4E69.img')).equals(file));

    // /config.json now names the disc; the route serves it (whole, HEAD, ranges) for that version only.
    const cfg = (await (await o.api('GET', '/config.json')).json()) as { disc: string; discPage: string };
    assert.match(cfg.disc, /^\/api\/disc\/image\?v=[a-z0-9]+-[a-z0-9]+$/);
    assert.equal(cfg.discPage, '/disc');
    r = await o.api('GET', cfg.disc);
    assert.equal(r.status, 200);
    assert.ok(Buffer.from(await r.arrayBuffer()).equals(file));
    r = await o.api('GET', cfg.disc, undefined, undefined, { Range: 'bytes=0-3' });
    assert.equal(r.status, 206);
    assert.equal(Buffer.from(await r.arrayBuffer()).toString('latin1'), 'CISO');
    assert.equal(r.headers.get('cache-control'), 'private, no-store');
    r = await o.api('GET', '/api/disc/image?v=old');
    assert.equal(r.status, 404);

    // Replacing it with the ISO: a new version; the old URL stops working, the old bytes are gone.
    r = await o.create(good.iso, 'iso');
    const id2 = ((await r.json()) as { upload: { id: string } }).upload.id;
    assert.equal((await o.send(id2, good.iso, 0, 65536)).status, 200);
    assert.equal((await o.finish(id2)).status, 200);
    const cfg2 = (await (await o.api('GET', '/config.json')).json()) as { disc: string };
    assert.notEqual(cfg2.disc, cfg.disc);
    assert.equal((await o.api('HEAD', cfg.disc)).status, 404);
    r = await o.api('HEAD', cfg2.disc);
    assert.equal(r.status, 200);
    assert.equal(r.headers.get('content-length'), String(good.iso.length));
    await app.discReady();
    assert.deepEqual(readdirSync(discDir).sort(), ['GV4E69.img', 'GV4E69.manifest.json', 'upload']);

    // A restart keeps the disc (checked again at start).
    await app.close();
    app = await start({ discDir });
    o = await Owner.open(app);
    st = await o.status();
    assert.equal(st.disc.format, 'iso');
    assert.equal(((await (await o.api('GET', '/config.json')).json()) as { disc: string }).disc, cfg2.disc);
  });

  it('refuses another file as soon as it shows, and deletes what arrived', async () => {
    const discDir = join(dir, 'refuse');
    const app = await start({ discDir });
    const o = await Owner.open(app);
    const cases: [Buffer, RegExp][] = [
      [filler('not a disc', 0x40000), /bad disc magic/],
      [fakeDisc({ gameId: 'GALE01' }).iso, /not GV4E69/],
      [toCiso(fakeDisc({ dolSeed: 'other' }).iso), /main.dol SHA-1/],
    ];
    for (const [file, why] of cases) {
      const r = await o.create(file);
      assert.equal(r.status, 201);
      const id = ((await r.json()) as { upload: { id: string } }).upload.id;
      const last = await o.send(id, file);
      assert.equal(last.status, 422);
      assert.match(last.body.error, /This image cannot be used/);
      assert.match(last.body.error, why);
      assert.deepEqual(readdirSync(join(discDir, 'upload')), [], 'the partial file is deleted');
      assert.equal((await o.put(id, 0, file.subarray(0, 16))).status, 404);
    }
    const st = await o.status();
    assert.equal(st.disc, null);
    assert.equal(st.upload, null);
    assert.ok(!existsSync(join(discDir, 'GV4E69.img')));
  });

  it('limits sizes and cancels', async () => {
    const app = await start({ maxDiscBytes: 0x50000 });
    const o = await Owner.open(app);
    let r = await o.api('POST', '/api/disc/uploads', JSON.stringify({ size: 0x50001, name: 'big', fingerprint: '' }), 'application/json');
    assert.equal(r.status, 413);
    r = await o.api('POST', '/api/disc/uploads', JSON.stringify({ size: 100, name: 'small', fingerprint: '' }), 'application/json');
    assert.equal(r.status, 422);
    r = await o.api('POST', '/api/disc/uploads', JSON.stringify({ size: 'x' }), 'application/json');
    assert.equal(r.status, 400);
    r = await o.api('POST', '/api/disc/uploads', '{', 'application/json');
    assert.equal(r.status, 400);
    r = await o.create(good.iso);
    const id = ((await r.json()) as { upload: { id: string } }).upload.id;
    assert.equal((await o.put(id, 0, good.iso.subarray(0, 1000))).status, 200);
    r = await o.api('DELETE', `/api/disc/uploads/${id}`);
    assert.equal(r.status, 200);
    assert.equal(((await r.json()) as { upload: unknown }).upload, null);
    assert.equal((await o.finish(id)).status, 404);
    // A new upload replaces an unfinished one.
    const a = ((await (await o.create(good.iso, 'a')).json()) as { upload: { id: string } }).upload.id;
    const b = ((await (await o.create(good.iso, 'b')).json()) as { upload: { id: string } }).upload.id;
    assert.notEqual(a, b);
    assert.equal((await o.put(a, 0, good.iso.subarray(0, 10))).status, 404);
    assert.equal((await o.finish(b)).status, 409);  // not complete
    assert.equal((await o.api('PUT', `/api/disc/uploads/${b}`, Buffer.alloc(4), 'application/octet-stream')).status, 411);
  });

  it('advertises the image hash and its pieces for the players\' one-time download (P1)', async () => {
    const discDir = join(dir, 'manifest');
    let app = await start({ discDir });
    let o = await Owner.open(app);
    const file = toCiso(good.iso, 576);
    const id = ((await (await o.create(file)).json()) as { upload: { id: string } }).upload.id;
    assert.equal((await o.send(id, file, 0, 65536)).status, 200);
    assert.equal((await o.finish(id)).status, 200);
    await app.discReady();
    const cfg = (await (await o.api('GET', '/config.json')).json()) as Record<string, any>;
    const pieces = [createHash('sha256').update(file).digest('hex')];  // under 8 MiB: one piece
    assert.equal(cfg.discHash, imageHash(pieces));
    assert.equal(cfg.discSize, file.length);
    assert.match(cfg.disc, /^\/api\/disc\/image\?v=/);
    assert.equal(cfg.discManifest, cfg.disc.replace('/image', '/manifest'));
    let r = await o.api('GET', cfg.discManifest);
    assert.equal(r.status, 200);
    assert.equal(r.headers.get('cache-control'), 'private, no-store');
    const m = (await r.json()) as Record<string, any>;
    assert.deepEqual({ ...m, version: undefined }, { version: undefined, size: file.length, pieceBytes: PIECE_BYTES, pieces, hash: cfg.discHash });
    assert.equal((await o.api('GET', '/api/disc/manifest?v=old')).status, 404);
    assert.equal((await fetch(`${app.url}${cfg.discManifest}`)).status, 401);  // behind the private link

    // Kept next to the image: a restart reads it instead of hashing the image again.
    await app.close();
    const logs: string[] = [];
    app = await startServer(config({ discDir }), { log: (l) => logs.push(l) });
    running.push(app);
    await app.discReady();
    o = await Owner.open(app);
    assert.equal(((await (await o.api('GET', '/config.json')).json()) as Record<string, any>).discHash, cfg.discHash);
    assert.ok(!logs.some((l) => l.includes('hashing')), logs.join('\n'));
  });
});

describe('disc image hash (P1)', () => {
  it('hashes pieces, and their digests into the image hash', async () => {
    const path = join(dir, 'pieces.bin');
    const bytes = filler('pieces', 200_000);
    writeFileSync(path, bytes);
    const m = await hashImage(path, 65536);
    const want = [0, 1, 2, 3].map((k) => createHash('sha256').update(bytes.subarray(k * 65536, (k + 1) * 65536)).digest('hex'));
    assert.deepEqual(m.pieces, want);
    assert.equal(m.size, bytes.length);
    assert.equal(m.hash, createHash('sha256').update(Buffer.concat(want.map((h) => Buffer.from(h, 'hex')))).digest('hex'));
  });

  it('computes in the background, keeps the result, and starts over when the file changes', async () => {
    const path = join(dir, 'cached.bin'), sidecar = join(dir, 'cached.manifest.json');
    writeFileSync(path, filler('cached', 50_000));
    const cache = new ManifestCache();
    assert.equal(await cache.get(path, sidecar), 'pending');
    const first = await cache.ready(path, sidecar);
    assert.ok(first && first.size === 50_000);
    assert.equal(await cache.get(path, sidecar), first);
    assert.ok(existsSync(sidecar));
    // A new cache (a restart) takes the stored manifest at once.
    assert.deepEqual(await new ManifestCache().get(path, sidecar), first);
    // Another file at the same path: hashed again.
    writeFileSync(path, filler('changed', 50_001));
    assert.equal(await cache.get(path, sidecar), 'pending');
    const second = await cache.ready(path, sidecar);
    assert.notEqual(second?.hash, first?.hash);
    assert.equal(await cache.get(join(dir, 'missing.bin')), null);
  });
});

// ---- TURN for the Docker deployment ----

describe('TURN settings from the environment (Docker)', () => {
  const env = (over: Record<string, string>) => ({ MVP_SITE: site, MVP_ACCESS_KEY: KEY, ...over });

  it('creates the shared secret once and builds STUN and TURN for the page host', () => {
    const file = join(dir, 'turn', 'secret');
    const c = loadConfig(env({ MVP_TURN_MODE: 'relay', MVP_TURN_SECRET_FILE: file, MVP_TURN_SECRET_CREATE: '1' }));
    const secret = readFileSync(file, 'utf8').trim();
    assert.match(secret, /^[A-Za-z0-9_-]{43}$/);
    if (process.platform !== 'win32') assert.equal(statSync(file).mode & 0o777, 0o644);
    assert.equal(c.turn?.secret, secret);
    assert.deepEqual(c.stun, ['stun:{host}:3478']);
    assert.deepEqual(c.turn?.urls, ['turn:{host}:3478?transport=udp', 'turn:{host}:3478?transport=tcp']);
    const again = loadConfig(env({ MVP_TURN_MODE: 'relay', MVP_TURN_SECRET_FILE: file, MVP_TURN_SECRET_CREATE: '1' }));
    assert.equal(again.turn?.secret, secret, 'kept across restarts');
    const ice = iceServers(c, 1_700_000_000_000, 'mvp.example.test');
    assert.deepEqual(ice[0], { urls: ['stun:mvp.example.test:3478'] });
    assert.deepEqual(ice[1]!.urls, ['turn:mvp.example.test:3478?transport=udp', 'turn:mvp.example.test:3478?transport=tcp']);
    assert.equal(ice[1]!.username, '1700086400:mvp');
  });

  it('takes a TURN host and port, and has STUN-only and off modes', () => {
    const file = join(dir, 'turn2', 'secret');
    let c = loadConfig(env({ MVP_TURN_MODE: 'relay', MVP_TURN_SECRET_FILE: file, MVP_TURN_SECRET_CREATE: 'true', MVP_TURN_HOST: '2001:db8::7', MVP_TURN_PORT: '3479' }));
    assert.deepEqual(c.stun, ['stun:[2001:db8::7]:3479']);
    c = loadConfig(env({ MVP_TURN_MODE: 'stun', MVP_TURN_HOST: '203.0.113.9' }));
    assert.deepEqual(c.stun, ['stun:203.0.113.9:3478']);
    assert.equal(c.turn, null);
    c = loadConfig(env({ MVP_TURN_MODE: 'off' }));
    assert.deepEqual(c.stun, []);
    assert.equal(c.turn, null);
    assert.throws(() => loadConfig(env({ MVP_TURN_MODE: 'relay' })), /needs MVP_TURN_SECRET/);
    assert.throws(() => loadConfig(env({ MVP_TURN_MODE: 'yes' })), /relay, stun or off/);
    assert.throws(() => loadConfig(env({ MVP_TURN_MODE: 'stun', MVP_TURN_HOST: 'a b' })), /host name/);
    assert.throws(() => loadConfig(env({ MVP_TURN_MODE: 'stun', MVP_TURN_PORT: '70000' })), /port/);
    assert.equal(loadConfig(env({ MVP_DISC_DIR: 'x' })).discDir, join(process.cwd(), 'x'));
  });

  it('hands each page the ICE servers of the host it used, and invite links on that origin', async () => {
    const app = await startServer(config({ discDir: '', stun: ['stun:{host}:3478'], turn: { urls: ['turn:{host}:3478?transport=udp'], secret: 's', ttlSeconds: 60 } }), { log: () => {} });
    try {
      const cookie = await enter(app);
      // The page's origin (as the browser sent it) names the host; the server checks it against Host.
      const host = new URL(app.url).host;
      const ws = new WebSocket(`${app.url.replace('http:', 'ws:')}/ws`, { headers: { Cookie: cookie, Origin: app.url, Host: host } });
      const msgs: ServerMsg[] = [];
      await new Promise<void>((resolve, reject) => {
        ws.on('message', (d) => {
          msgs.push(JSON.parse(d.toString()) as ServerMsg);
          if (msgs.length === 1) ws.send(JSON.stringify({ t: 'create' }));
          if (msgs.length === 2) resolve();
        });
        ws.on('error', reject);
      });
      const welcome = msgs[0] as Extract<ServerMsg, { t: 'welcome' }>;
      assert.deepEqual(welcome.iceServers[0], { urls: ['stun:127.0.0.1:3478'] });
      assert.deepEqual(welcome.iceServers[1]!.urls, ['turn:127.0.0.1:3478?transport=udp']);
      const room = msgs[1] as Extract<ServerMsg, { t: 'room' }>;
      assert.equal(room.invite, `${app.url}/?room=${room.room}&key=${KEY}`);
      ws.close();
    } finally {
      await app.close();
    }
  });
});
