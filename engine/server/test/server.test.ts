// Tests for the VPS server (E5): the private link, the static site, the disc
// route, the lobby and signaling. No game data: the "disc" is random bytes in
// a temporary directory.
import assert from 'node:assert/strict';
import { createHash, createHmac, randomBytes } from 'node:crypto';
import { mkdtempSync, mkdirSync, readFileSync, rmSync, statSync, writeFileSync } from 'node:fs';
import { request as httpRequest } from 'node:http';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { after, before, describe, it } from 'node:test';
import WebSocket from 'ws';

import { ACCESS_COOKIE, NO_ACCESS_CSP, accessToken, parseCookies } from '../src/access.js';
import { defaults, loadConfig, type ServerConfig } from '../src/config.js';
import { parseRange } from '../src/files.js';
import { parseClientMsg } from '../src/lobby.js';
import type { ServerMsg } from '../src/lobby_protocol.js';
import { SITE_CSP, startServer, type App } from '../src/server.js';
import { turnCredentials } from '../src/turn.js';

const KEY = 'test-access-key.0123456789~';
const dir = mkdtempSync(join(tmpdir(), 'mvp-server-test-'));
const site = join(dir, 'site');
const discPath = join(dir, 'disc.bin');
const disc = randomBytes(100_000);

before(() => {
  mkdirSync(join(site, 'sub'), { recursive: true });
  writeFileSync(join(site, 'index.html'), '<!doctype html><title>engine</title>');
  writeFileSync(join(site, 'main.js'), 'console.log(1);');
  writeFileSync(join(site, 'sub', 'a.txt'), 'hello');
  writeFileSync(join(site, 'secret.key'), 'not served: unknown type');
  writeFileSync(join(dir, 'outside.txt'), 'outside the site');
  writeFileSync(discPath, disc);
});
after(() => rmSync(dir, { recursive: true, force: true }));

function config(over: Partial<ServerConfig> = {}): ServerConfig {
  return { ...defaults, site, accessKey: KEY, port: 0, disc: discPath, ...over };
}

interface RawAnswer {
  status: number;
  location?: string;
  cookie?: string;
  headers: Record<string, string | string[] | undefined>;
  body: string;
}

/** A GET with exact headers and an exact path (Node's fetch always says Sec-Fetch-Mode: cors, and normalizes paths). */
function rawGet(app: App, path: string, headers: Record<string, string>): Promise<RawAnswer> {
  const u = new URL(app.url);
  return new Promise((resolve, reject) => {
    httpRequest({ host: u.hostname, port: u.port, path, headers }, (res) => {
      let body = '';
      res.setEncoding('utf8');
      res.on('data', (c: string) => (body += c));
      res.on('end', () =>
        resolve({ status: res.statusCode ?? 0, location: res.headers.location, cookie: res.headers['set-cookie']?.[0], headers: res.headers, body }),
      );
    })
      .on('error', reject)
      .end();
  });
}

const NAVIGATE = { Accept: 'text/html', 'Sec-Fetch-Mode': 'navigate' };

/** Opens the private link as a browser would; returns the cookie it gets ("mvp_access=..."). */
async function enter(app: App): Promise<string> {
  const r = await fetch(app.link, { redirect: 'manual' });
  assert.equal(r.status, 303);
  return (r.headers.get('set-cookie') ?? '').split(';')[0]!;
}

describe('private link', () => {
  let app: App;
  before(async () => (app = await startServer(config())));
  after(() => app.close());

  it('shows page loads without access the private-server page, and gives API calls a 401', async () => {
    let raw = await rawGet(app, '/?room=abc', NAVIGATE);
    assert.equal(raw.status, 403);
    assert.equal(raw.cookie, undefined);
    assert.equal(raw.headers['content-security-policy'], NO_ACCESS_CSP);
    assert.match(NO_ACCESS_CSP, /^default-src 'none'; style-src 'sha256-[A-Za-z0-9+/=]+'; img-src data:; frame-ancestors 'none'; base-uri 'none'$/);
    assert.equal(raw.headers['x-robots-tag'], 'noindex, nofollow');
    assert.match(raw.body, /This is a private server\. Open the private link you were sent\./);
    assert.match(raw.body, /<meta name="robots" content="noindex">/);
    assert.doesNotMatch(raw.body, /<script|<form|<a /);
    raw = await rawGet(app, '/main.js', { 'Sec-Fetch-Mode': 'navigate' });
    assert.equal(raw.status, 403);
    raw = await rawGet(app, '/', { Accept: 'text/html,application/xhtml+xml' });  // curl-like, no Sec-Fetch-*
    assert.equal(raw.status, 403);
    raw = await rawGet(app, '/config.json', { Accept: '*/*', 'Sec-Fetch-Mode': 'cors' });
    assert.equal(raw.status, 401);
    let r: Response;
    for (const p of ['/config.json', '/api/disc/image', '/api/rooms', '/main.js', '/nothing', '/login', '/setup']) {
      r = await fetch(`${app.url}${p}`, { redirect: 'manual' });
      assert.equal(r.status, 401, p);
      assert.deepEqual(await r.json(), { error: 'no access' });
    }
    r = await fetch(`${app.url}/api/disc/image`, { method: 'HEAD', redirect: 'manual' });
    assert.equal(r.status, 401);
    r = await fetch(`${app.url}/config.json`, { headers: { Cookie: `${ACCESS_COOKIE}=${'x'.repeat(43)}` } });
    assert.equal(r.status, 401);
  });

  it('lets a browser in with the right key: 303 to the same address without it, and a lasting cookie', async () => {
    assert.equal(app.link, `${app.url}/?key=${KEY}`);
    let r = await fetch(`${app.url}/?room=ab&key=${KEY}`, { redirect: 'manual' });
    assert.equal(r.status, 303);
    assert.equal(r.headers.get('location'), '/?room=ab');
    assert.equal(r.headers.get('cache-control'), 'no-store');
    const sc = r.headers.get('set-cookie') ?? '';
    assert.equal(sc, `${ACCESS_COOKIE}=${accessToken(KEY)}; Path=/; HttpOnly; SameSite=Lax; Max-Age=${400 * 86400}`);
    assert.match(sc, /^mvp_access=[A-Za-z0-9_-]{43};/);
    assert.equal(accessToken(KEY), createHmac('sha256', KEY).update('mvp-access-v1').digest('base64url'));
    const cookie = sc.split(';')[0]!;
    for (const [from, to] of [
      [`/?key=${KEY}`, '/'],
      [`/?key=${KEY}&room=ab&mod=x&mod=y`, '/?room=ab&mod=x&mod=y'],
      [`/disc?key=${KEY}`, '/disc'],
      [`/sub/a.txt?a=1&key=${KEY}`, '/sub/a.txt?a=1'],
    ]) {
      const raw = await rawGet(app, from!, NAVIGATE);
      assert.equal(raw.status, 303, from);
      assert.equal(raw.location, to, from);
      assert.match(raw.cookie ?? '', /^mvp_access=/);
    }
    // Never a redirect to another host.
    for (const from of [`//evil.example/?key=${KEY}`, `/.//evil.example/?key=${KEY}`, `/\\evil.example/?key=${KEY}`]) {
      const raw = await rawGet(app, from, NAVIGATE);
      assert.equal(raw.status, 303, from);
      assert.match(raw.location ?? '', /^\/(?!\/)/, from);
    }
    // In: the site, and every page load renews the cookie (API calls do not).
    r = await fetch(`${app.url}/`, { headers: { Cookie: cookie } });
    assert.equal(r.status, 200);
    assert.equal(await r.text(), '<!doctype html><title>engine</title>');
    assert.equal(r.headers.get('content-security-policy'), SITE_CSP);
    assert.equal(r.headers.get('referrer-policy'), 'no-referrer');
    assert.equal(r.headers.get('x-robots-tag'), 'noindex, nofollow');
    assert.equal(r.headers.get('set-cookie'), null);
    const raw = await rawGet(app, '/?room=ab', { ...NAVIGATE, Cookie: cookie });
    assert.equal(raw.status, 200);
    assert.equal(raw.cookie, sc);
    r = await fetch(`${app.url}/config.json`, { headers: { Cookie: `other=1; ${cookie}` } });
    assert.equal(r.status, 200);
  });

  it('treats a wrong key as no access', async () => {
    const cookie = await enter(app);
    for (const k of ['wrong-key-0123456789', '', KEY.slice(0, -1), `${KEY}x`]) {
      const raw = await rawGet(app, `/?key=${encodeURIComponent(k)}`, NAVIGATE);
      assert.equal(raw.status, 403, k);
      assert.equal(raw.cookie, undefined);
      // Even with the cookie: the link was wrong.
      assert.equal((await rawGet(app, `/?key=${encodeURIComponent(k)}`, { ...NAVIGATE, Cookie: cookie })).status, 403, k);
      const r = await fetch(`${app.url}/config.json?key=${encodeURIComponent(k)}`, { redirect: 'manual' });
      assert.equal(r.status, 401, k);
    }
  });

  it('keeps a browser in across a restart; a new key locks it out', async () => {
    const cookie = await enter(app);
    const again = await startServer(config());
    try {
      assert.equal((await fetch(`${again.url}/config.json`, { headers: { Cookie: cookie } })).status, 200);
    } finally {
      await again.close();
    }
    const changed = await startServer(config({ accessKey: 'a-new-key-for-the-site-42' }));
    try {
      assert.equal((await fetch(`${changed.url}/config.json`, { headers: { Cookie: cookie } })).status, 401);
      assert.equal((await rawGet(changed, `/?key=${KEY}`, NAVIGATE)).status, 403);  // the old link too
      const fresh = await enter(changed);
      assert.notEqual(fresh, cookie);
      assert.equal((await fetch(`${changed.url}/config.json`, { headers: { Cookie: fresh } })).status, 200);
    } finally {
      await changed.close();
    }
  });

  it('answers /healthz and /robots.txt without access', async () => {
    let r = await fetch(`${app.url}/healthz`);
    assert.equal(r.status, 200);
    assert.equal(await r.text(), 'ok');
    r = await fetch(`${app.url}/robots.txt`);
    assert.equal(r.status, 200);
    assert.equal(await r.text(), 'User-agent: *\nDisallow: /\n');
    assert.equal(r.headers.get('x-robots-tag'), 'noindex, nofollow');
    // The private link works on them too (the check tools use /healthz?key=...: a page that starts nothing).
    const raw = await rawGet(app, `/healthz?key=${KEY}`, NAVIGATE);
    assert.equal(raw.status, 303);
    assert.equal(raw.location, '/healthz');
  });
});

describe('access configuration', () => {
  const env = (over: Record<string, string>) => ({ MVP_SITE: site, ...over });

  it('needs a key of at least 16 URL-safe characters', () => {
    assert.throws(() => loadConfig(env({})), /no access key: set MVP_ACCESS_KEY, or MVP_ACCESS_KEY_FILE/);
    assert.throws(() => loadConfig(env({ MVP_ACCESS_KEY: 'short-key-123' })), /at least 16 characters/);
    for (const bad of ['sixteen chars ok', 'with/a/slash/0123', 'percent%20encoded0', 'ünïcode-0123456789', 'a+b=c&d0123456789'])
      assert.throws(() => loadConfig(env({ MVP_ACCESS_KEY: bad })), /only letters, digits and \. _ ~ -/, bad);
    const c = loadConfig(env({ MVP_ACCESS_KEY: KEY, MVP_PORT: '9000', MVP_SECURE_COOKIES: '1' }));
    assert.equal(c.accessKey, KEY);
    assert.equal(c.port, 9000);
    assert.equal(c.secureCookies, true);
    assert.equal(c.turn, null);
    assert.ok(!('password' in c) && !('stateFile' in c) && !('trustProxy' in c));
  });

  it('reads the key from a file, where MVP_ACCESS_KEY wins and an empty file is an error', () => {
    const file = join(dir, 'access-key');
    writeFileSync(file, 'from-a-file-0123456789\n');
    assert.equal(loadConfig(env({ MVP_ACCESS_KEY_FILE: file })).accessKey, 'from-a-file-0123456789');
    assert.equal(loadConfig(env({ MVP_ACCESS_KEY_FILE: file, MVP_ACCESS_KEY: KEY })).accessKey, KEY);
    const empty = join(dir, 'empty-key');
    writeFileSync(empty, '\n');
    assert.throws(() => loadConfig(env({ MVP_ACCESS_KEY_FILE: empty })), /MVP_ACCESS_KEY_FILE .* is empty/);
    assert.throws(() => loadConfig(env({ MVP_ACCESS_KEY_FILE: join(dir, 'missing-key') })), /ENOENT/);
  });

  it('creates a missing key file once, owner-only, and keeps it', () => {
    const file = join(dir, 'data', 'access-key');
    const a = loadConfig(env({ MVP_ACCESS_KEY_FILE: file, MVP_ACCESS_KEY_CREATE: '1' }));
    assert.match(a.accessKey, /^[A-Za-z0-9_-]{43}$/);
    assert.equal(readFileSync(file, 'utf8').trim(), a.accessKey);
    if (process.platform !== 'win32') assert.equal(statSync(file).mode & 0o777, 0o600);
    const b = loadConfig(env({ MVP_ACCESS_KEY_FILE: file, MVP_ACCESS_KEY_CREATE: 'true' }));
    assert.equal(b.accessKey, a.accessKey, 'kept across restarts');
    // MVP_ACCESS_KEY set: it wins, and no file is created.
    const unused = join(dir, 'data', 'unused-key');
    assert.equal(loadConfig(env({ MVP_ACCESS_KEY_FILE: unused, MVP_ACCESS_KEY_CREATE: '1', MVP_ACCESS_KEY: KEY })).accessKey, KEY);
    assert.throws(() => statSync(unused), /ENOENT/);
  });

  it('parses cookies', () => {
    assert.deepEqual([...parseCookies('a=1; mvp_access=xyz; b=2').entries()], [['a', '1'], ['mvp_access', 'xyz'], ['b', '2']]);
  });
});

describe('static site', () => {
  let app: App;
  let cookie: string;
  before(async () => {
    app = await startServer(config());
    cookie = await enter(app);
  });
  after(() => app.close());

  it('serves files inside the site only', async () => {
    const get = (p: string) => fetch(`${app.url}${p}`, { headers: { Cookie: cookie } });
    let r = await get('/sub/a.txt');
    assert.equal(r.status, 200);
    assert.equal(await r.text(), 'hello');
    assert.equal(r.headers.get('content-type'), 'text/plain; charset=utf-8');
    assert.equal(r.headers.get('cache-control'), 'private, no-cache');
    for (const p of ['/../outside.txt', '/%2e%2e/outside.txt', '/sub/../../outside.txt', '/secret.key', '/sub', '/missing.js']) {
      r = await get(p);
      assert.equal(r.status, 404, p);
    }
  });

  it('makes the pages cross-origin isolated (P2: the threaded engine), the answers without access too', async () => {
    for (const r of [await fetch(`${app.url}/sub/a.txt`, { headers: { Cookie: cookie } }), await fetch(`${app.url}/`)]) {
      assert.equal(r.headers.get('cross-origin-opener-policy'), 'same-origin');
      assert.equal(r.headers.get('cross-origin-embedder-policy'), 'require-corp');
      assert.equal(r.headers.get('cross-origin-resource-policy'), 'same-origin');
    }
  });

  it('revalidates with an ETag', async () => {
    let r = await fetch(`${app.url}/main.js`, { headers: { Cookie: cookie } });
    const etag = r.headers.get('etag')!;
    assert.ok(etag);
    r = await fetch(`${app.url}/main.js`, { headers: { Cookie: cookie, 'If-None-Match': etag } });
    assert.equal(r.status, 304);
  });
});

describe('disc route', () => {
  it('advertises the disc in /config.json only when configured', async () => {
    // One piece: the image hash is SHA-256 over that piece's SHA-256 (src/disc_hash.ts).
    const hash = createHash('sha256').update(createHash('sha256').update(disc).digest()).digest('hex');
    const withDisc = { disc: '/api/disc/image', discHash: hash, discSize: disc.length, discManifest: '/api/disc/manifest' };
    for (const [d, body] of [[discPath, withDisc], ['', {}]] as const) {
      const app = await startServer(config({ disc: d }), { log: () => {} });
      try {
        await app.discReady();
        const cookie = await enter(app);
        const r = await fetch(`${app.url}/config.json`, { headers: { Cookie: cookie } });
        assert.equal(r.status, 200);
        assert.deepEqual(await r.json(), body);
        const img = await fetch(`${app.url}/api/disc/image`, { method: 'HEAD', headers: { Cookie: cookie } });
        assert.equal(img.status, d ? 200 : 404);
      } finally {
        await app.close();
      }
    }
  });

  it('serves HEAD, whole and ranged GETs, never cacheable', async () => {
    const app = await startServer(config());
    try {
      const cookie = await enter(app);
      const req = (init: RequestInit & { headers?: Record<string, string> } = {}) =>
        fetch(`${app.url}/api/disc/image`, { ...init, headers: { Cookie: cookie, ...(init.headers ?? {}) } });
      let r = await req({ method: 'HEAD' });
      assert.equal(r.status, 200);
      assert.equal(r.headers.get('content-length'), String(disc.length));
      assert.equal(r.headers.get('accept-ranges'), 'bytes');
      assert.equal(r.headers.get('cache-control'), 'private, no-store');
      r = await req();
      assert.equal(r.status, 200);
      assert.ok(Buffer.from(await r.arrayBuffer()).equals(disc));
      const cases: [string, number, number][] = [
        ['bytes=10-19', 10, 19],
        ['bytes=99990-', 99990, 99999],
        ['bytes=-16', 99984, 99999],
        ['bytes=99000-200000', 99000, 99999],
        ['bytes=0-0', 0, 0],
      ];
      for (const [range, s, e] of cases) {
        r = await req({ headers: { Range: range } });
        assert.equal(r.status, 206, range);
        assert.equal(r.headers.get('content-range'), `bytes ${s}-${e}/${disc.length}`);
        assert.equal(r.headers.get('content-length'), String(e - s + 1));
        assert.equal(r.headers.get('cache-control'), 'private, no-store');
        assert.ok(Buffer.from(await r.arrayBuffer()).equals(disc.subarray(s, e + 1)), range);
      }
      for (const bad of ['bytes=100000-', 'bytes=5-3', 'bytes=0-1,5-6', 'items=0-1', 'bytes=-0']) {
        r = await req({ headers: { Range: bad } });
        assert.equal(r.status, 416, bad);
        assert.equal(r.headers.get('content-range'), `bytes */${disc.length}`);
        await r.arrayBuffer();
      }
    } finally {
      await app.close();
    }
  });

  it('parses ranges', () => {
    assert.equal(parseRange(undefined, 10), 'none');
    assert.deepEqual(parseRange('bytes=2-', 10), { start: 2, end: 9 });
    assert.deepEqual(parseRange('bytes=-3', 10), { start: 7, end: 9 });
    assert.deepEqual(parseRange('bytes=-30', 10), { start: 0, end: 9 });
    assert.equal(parseRange('bytes=10-', 10), 'bad');
    assert.equal(parseRange('bytes=-', 10), 'bad');
  });
});

// ---- lobby and signaling ----

class Peer {
  readonly msgs: ServerMsg[] = [];
  private waiters: (() => void)[] = [];
  constructor(readonly ws: WebSocket) {
    ws.on('message', (d) => {
      this.msgs.push(JSON.parse(d.toString()) as ServerMsg);
      for (const w of this.waiters.splice(0)) w();
    });
  }
  static open(url: string, headers: Record<string, string>): Promise<Peer> {
    return new Promise((resolve, reject) => {
      const ws = new WebSocket(url, { headers });
      const p = new Peer(ws);
      ws.on('open', () => resolve(p));
      ws.on('error', reject);
      ws.on('unexpected-response', (_req, res) => reject(new Error(`HTTP ${res.statusCode}`)));
    });
  }
  send(m: unknown): void {
    this.ws.send(typeof m === 'string' ? m : JSON.stringify(m));
  }
  async next<T extends ServerMsg['t']>(t: T): Promise<Extract<ServerMsg, { t: T }>> {
    for (;;) {
      const i = this.msgs.findIndex((m) => m.t === t);
      if (i >= 0) return this.msgs.splice(i, 1)[0] as Extract<ServerMsg, { t: T }>;
      await new Promise<void>((resolve, reject) => {
        const timer = setTimeout(() => reject(new Error(`timeout waiting for ${t}`)), 3000);
        this.waiters.push(() => {
          clearTimeout(timer);
          resolve();
        });
      });
    }
  }
}

describe('lobby and signaling', () => {
  let app: App;
  let cookie: string;
  const secret = 'turn-secret-for-tests';
  before(async () => {
    app = await startServer(
      config({
        stun: ['stun:turn.example.test:3478'],
        turn: { urls: ['turn:turn.example.test:3478?transport=udp'], secret, ttlSeconds: 3600 },
        publicUrl: '',
      }),
    );
    cookie = await enter(app);
  });
  after(() => app.close());
  const wsUrl = () => app.url.replace('http:', 'ws:') + '/ws';
  const open = () => Peer.open(wsUrl(), { Cookie: cookie, Origin: app.url });

  it('refuses sockets without access or from another origin', async () => {
    await assert.rejects(Peer.open(wsUrl(), { Origin: app.url }), /401/);
    await assert.rejects(Peer.open(wsUrl(), { Cookie: `${ACCESS_COOKIE}=${accessToken('another-key-0123456789')}`, Origin: app.url }), /401/);
    await assert.rejects(Peer.open(wsUrl(), { Cookie: cookie, Origin: 'https://evil.example' }), /401/);
    await assert.rejects(Peer.open(wsUrl(), { Cookie: cookie }), /401/);
  });

  it('welcomes with STUN and time-limited TURN credentials', async () => {
    const p = await open();
    const w = await p.next('welcome');
    assert.equal(w.v, 1);
    assert.deepEqual(w.iceServers[0], { urls: ['stun:turn.example.test:3478'] });
    const turn = w.iceServers[1]!;
    assert.deepEqual(turn.urls, ['turn:turn.example.test:3478?transport=udp']);
    const [expiry, tag] = turn.username!.split(':');
    assert.equal(tag, 'mvp');
    const left = Number(expiry) - Date.now() / 1000;
    assert.ok(left > 3500 && left <= 3600, String(left));
    assert.equal(turn.credential, createHmac('sha1', secret).update(turn.username!).digest('base64'));
    p.ws.close();
  });

  it('creates a room, joins it, relays signals, and tells who left', async () => {
    const a = await open();
    const b = await open();
    const c = await open();
    a.send({ t: 'create', name: 'Patrick', delay: 4 });
    const ra = await a.next('room');
    assert.equal(ra.player, 1);
    assert.equal(ra.delay, 4);
    assert.equal(ra.peer, false);
    assert.match(ra.room, /^[a-z2-9]{12}$/);
    // The invite carries the key: it also lets the other player's browser in, and lands on the game.
    assert.equal(ra.invite, `${app.url}/?room=${ra.room}&key=${KEY}`);
    const viaInvite = await fetch(ra.invite, { redirect: 'manual' });
    assert.equal(viaInvite.status, 303);
    assert.equal(viaInvite.headers.get('location'), `/?room=${ra.room}`);
    assert.equal((viaInvite.headers.get('set-cookie') ?? '').split(';')[0], cookie);

    const r = await fetch(`${app.url}/api/rooms`, { headers: { Cookie: cookie } });
    const rooms = ((await r.json()) as { rooms: { room: string; name: string; players: number }[] }).rooms;
    assert.ok(rooms.some((x) => x.room === ra.room && x.name === 'Patrick' && x.players === 1));

    b.send({ t: 'join', room: ra.room });
    const rb = await b.next('room');
    assert.equal(rb.player, 2);
    assert.equal(rb.delay, 4);
    assert.equal(rb.peer, true);
    assert.deepEqual(await a.next('peer'), { t: 'peer', present: true });

    c.send({ t: 'join', room: ra.room });
    assert.equal((await c.next('error')).code, 'room-full');
    c.send({ t: 'join', room: 'nosuchroom' });
    assert.equal((await c.next('error')).code, 'no-room');

    const offer = { kind: 'offer', sdp: 'v=0\r\no=- 1 2 IN IP4 127.0.0.1\r\n' } as const;
    a.send({ t: 'signal', data: offer });
    assert.deepEqual((await b.next('signal')).data, offer);
    const cand = { kind: 'candidate', candidate: { candidate: 'candidate:1 1 udp 1 127.0.0.1 5000 typ host', sdpMid: '0', sdpMLineIndex: 0 } } as const;
    b.send({ t: 'signal', data: cand });
    assert.deepEqual((await a.next('signal')).data, cand);
    b.send({ t: 'signal', data: { kind: 'candidate', candidate: null } });
    assert.deepEqual((await a.next('signal')).data, { kind: 'candidate', candidate: null });
    c.send({ t: 'signal', data: offer });
    assert.equal((await c.next('error')).code, 'no-peer');

    b.ws.close();
    assert.deepEqual(await a.next('peer'), { t: 'peer', present: false });
    a.send({ t: 'list' });
    const list = await a.next('rooms');
    assert.ok(list.rooms.some((x) => x.room === ra.room && x.players === 1));
    a.ws.close();
    c.ws.close();
  });

  it('rejects malformed messages', async () => {
    const p = await open();
    for (const bad of ['not json', '{}', '{"t":"join","room":"../x"}', '{"t":"create","delay":99}',
                       '{"t":"signal","data":{"kind":"offer"}}', '{"t":"signal","data":{"kind":"candidate","candidate":{"candidate":5}}}']) {
      p.send(bad);
      assert.equal((await p.next('error')).code, 'bad-message', bad);
    }
    assert.equal(parseClientMsg('{"t":"signal","data":{"kind":"offer","sdp":"' + 'x'.repeat(40000) + '"}}'), null);
    p.ws.close();
  });
});

describe('TURN credentials', () => {
  it('match coturn REST API credentials', () => {
    const c = turnCredentials('s3cret', 600, 1_700_000_000_000);
    assert.equal(c.username, '1700000600:mvp');
    assert.equal(c.credential, createHmac('sha1', 's3cret').update('1700000600:mvp').digest('base64'));
  });
});
