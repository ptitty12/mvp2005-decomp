// The VPS server (E5): one Node process behind a TLS reverse proxy.
//
//   any  /...?key=<key>                      the private link (src/access.ts): 303 to the same
//                                            address without the key, with the access cookie
//   GET  /healthz                            "ok" (open; for monitoring)
//   GET  /robots.txt                         "Disallow: /" (open)
//   GET  /config.json                        {"disc":"/api/disc/image"} or {}; "discPage" with uploads; with a
//                                            disc also "discHash", "discSize" and "discManifest" (P1), or
//                                            "discPreparing" while the image is being hashed
//   GET/HEAD /api/disc/image                 the owner's disc image (src/files.ts)
//   GET  /api/disc/manifest                  the image's piece hashes, for the players' one-time download
//                                            into their own browser storage (src/disc_hash.ts)
//   GET  /disc, /api/disc, /api/disc/uploads  the Server disc page and its chunked upload (deploy:
//                                            src/disc_page.ts, src/disc_store.ts), with uploads on
//   GET  /api/packs, /api/packs/<file>       the roster packs the pages offer in a dropdown, and one
//                                            pack's zip (src/pack_store.ts), with a packs directory
//   POST /api/packs, DELETE /api/packs/<file> upload or remove a pack (the Server disc page, its token)
//   GET  /api/rooms                          open rooms (src/lobby.ts)
//   WS   /ws                                 lobby and WebRTC signaling
//   GET  anything else                       the engine site (static files)
//
// Everything except /healthz and /robots.txt needs the access cookie: a page
// load without it gets the "private server" page (403), anything else 401.
import { createHmac, timingSafeEqual } from 'node:crypto';
import { createServer, type IncomingMessage, type Server, type ServerResponse } from 'node:http';
import { readFile } from 'node:fs/promises';
import type { AddressInfo } from 'node:net';
import { WebSocketServer } from 'ws';

import {
  ACCESS_COOKIE,
  NO_ACCESS_CSP,
  accessCookie,
  accessToken,
  isNavigation,
  noAccessPage,
  parseCookies,
  privateLink,
  sameSecret,
} from './access.js';
import type { ServerConfig } from './config.js';
import { DISC_PAGE_CSP, discPage } from './disc_page.js';
import { ManifestCache } from './disc_hash.js';
import { DiscStore, type Answer } from './disc_store.js';
import { serveDisc, serveStatic } from './files.js';
import { PackStore } from './pack_store.js';
import { Lobby } from './lobby.js';
import { iceServers } from './turn.js';

/** Content-Security-Policy of the engine pages: same-origin only, plus compiling the translated game (wasm). */
export const SITE_CSP =
  "default-src 'self'; script-src 'self' 'wasm-unsafe-eval'; worker-src 'self'; connect-src 'self'; " +
  "img-src 'self' data: blob:; style-src 'self'; object-src 'none'; base-uri 'none'; form-action 'self'; frame-ancestors 'none'";

// Cross-origin isolation (P2): COOP same-origin with COEP require-corp makes the
// pages cross-origin isolated, so the engine may share memory with its render
// thread (SharedArrayBuffer, wasm threads). Everything the site loads is its
// own (same-origin), which COEP allows; CORP says the same of each response.
const COMMON_HEADERS: Record<string, string> = {
  'X-Content-Type-Options': 'nosniff',
  'Referrer-Policy': 'no-referrer',
  'X-Frame-Options': 'DENY',
  'Cross-Origin-Opener-Policy': 'same-origin',
  'Cross-Origin-Embedder-Policy': 'require-corp',
  'Cross-Origin-Resource-Policy': 'same-origin',
  // A private server: nothing here belongs in a search engine (robots.txt says the same).
  'X-Robots-Tag': 'noindex, nofollow',
};

export interface App {
  server: Server;
  lobby: Lobby;
  /** http://127.0.0.1:port */
  url: string;
  /** The private link on `url` (tests and tools): opening it gives a browser access. */
  link: string;
  /** Resolves when the served disc's manifest is computed (or cannot be): tests. */
  discReady(): Promise<void>;
  close(): Promise<void>;
}

/** For tests: the clock (stale uploads) and where the log lines go. */
export interface StartOptions {
  now?: () => number;
  log?: (message: string) => void;
}

function readBody(req: IncomingMessage, limit: number): Promise<string | null> {
  return new Promise((resolve) => {
    let size = 0;
    const chunks: Buffer[] = [];
    req.on('data', (c: Buffer) => {
      size += c.length;
      if (size > limit) {
        resolve(null);
        req.destroy();
        return;
      }
      chunks.push(c);
    });
    req.on('end', () => resolve(Buffer.concat(chunks).toString('utf8')));
    req.on('error', () => resolve(null));
  });
}

/**
 * The body as bytes, or null when it is larger than `limit` or fails. A body over the
 * limit is read to its end and dropped, so the page gets an answer (413), not a reset.
 */
function readBodyBytes(req: IncomingMessage, limit: number): Promise<Buffer | null> {
  return new Promise((resolve) => {
    let size = 0;
    const chunks: Buffer[] = [];
    req.on('data', (c: Buffer) => {
      size += c.length;
      if (size <= limit) chunks.push(c);
      else chunks.length = 0;
    });
    req.on('end', () => resolve(size <= limit ? Buffer.concat(chunks) : null));
    req.on('error', () => resolve(null));
  });
}

/** The origin requests must come from: the configured public URL, else the request's own host. */
function originAllowed(req: IncomingMessage, cfg: ServerConfig): boolean {
  const origin = req.headers.origin;
  if (!origin) return false;
  if (cfg.publicUrl) return origin === new URL(cfg.publicUrl).origin;
  try {
    return new URL(origin).host === req.headers.host;
  } catch {
    return false;
  }
}

/** A form POST from our own page: Sec-Fetch-Site (when sent) says same-origin, Origin (when sent) is ours. */
function sameOriginPost(req: IncomingMessage, cfg: ServerConfig): boolean {
  const site = req.headers['sec-fetch-site'];
  if (site !== undefined && site !== 'same-origin') return false;
  return req.headers.origin === undefined || originAllowed(req, cfg);
}

export async function startServer(cfg: ServerConfig, opts: StartOptions = {}): Promise<App> {
  const now = opts.now ?? Date.now;
  const log = opts.log ?? ((m: string) => console.log(`mvp-server: ${m}`));
  // The access cookie's value: the same for every browser with access, and across restarts (src/access.ts).
  const token = accessToken(cfg.accessKey);
  const hasAccess = (req: IncomingMessage) => sameSecret(parseCookies(req.headers.cookie).get(ACCESS_COOKIE), token);
  const lobby = new Lobby({
    // An invite carries the key, so it also lets the other player's browser in.
    invite: (room, origin) => privateLink(origin, cfg.accessKey, room),
    ice: (host) => iceServers(cfg, Date.now(), host),
    idleMinutes: cfg.roomIdleMinutes,
  });
  // The uploaded disc (deploy): only when a disc directory is configured.
  const store = cfg.discDir ? await DiscStore.open({ dir: cfg.discDir, expect: cfg.expectDisc, maxBytes: cfg.maxDiscBytes, now, log }) : null;
  // Roster packs the pages offer in a dropdown: only when a packs directory is configured.
  const packs = cfg.packsDir ? await PackStore.open({ dir: cfg.packsDir, maxBytes: cfg.maxPackBytes, log }) : null;
  /**
   * The image the disc route serves, its URL and version: an uploaded one, else the configured file.
   * `sidecar`: where its manifest is kept between restarts (uploads only).
   */
  const servedDisc = (): { path: string; url: string; version: string; sidecar?: string } | null => {
    const up = store?.current();
    if (up) return { path: up.path, url: `/api/disc/image?v=${up.version}`, version: up.version, sidecar: store!.manifestPath };
    return cfg.disc ? { path: cfg.disc, url: '/api/disc/image', version: '' } : null;
  };
  // The served image's piece hashes (P1), computed in the background at start and after an upload.
  const manifests = new ManifestCache(log);
  const warmManifest = () => {
    const d = servedDisc();
    if (d) void manifests.get(d.path, d.sidecar);
  };
  warmManifest();
  // CSRF tokens of the Server disc page: bound to the access cookie, under a key derived from the access key, so
  // an open page keeps working across a restart (an upload continues) and a page elsewhere can neither read nor
  // forge one.
  const discKey = createHmac('sha256', cfg.accessKey).update('mvp-disc-csrf-v1').digest();
  const discToken = (cookie: string) => createHmac('sha256', discKey).update(`disc:${cookie}`).digest('base64url');
  const wss = new WebSocketServer({ noServer: true, maxPayload: 64 * 1024 });

  const sendJson = (res: ServerResponse, a: Answer) =>
    res.writeHead(a.status, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' }).end(JSON.stringify(a.body));

  /**
   * The Server disc page's upload API (with access already; src/disc_store.ts):
   *   POST   /api/disc/uploads                {size, name, fingerprint}: a new upload (201)
   *   PUT    /api/disc/uploads/<id>?offset=N  one chunk (application/octet-stream, at most CHUNK_MAX bytes)
   *   POST   /api/disc/uploads/<id>/finish    check the complete file and make it the server's disc
   *   DELETE /api/disc/uploads/<id>           abandon it
   * Each request must come from the page itself: the X-MVP-CSRF header holds the page's token,
   * Sec-Fetch-Site (when sent) says same-origin and Origin (when sent) is this site.
   */
  /** A request from the Server disc page itself: its CSRF token, same-origin. */
  const fromDiscPage = (req: IncomingMessage): boolean => {
    const cookie = parseCookies(req.headers.cookie).get(ACCESS_COOKIE)!;
    const got = String(req.headers['x-mvp-csrf'] ?? '');
    const want = discToken(cookie);
    const tokenOk = got.length === want.length && timingSafeEqual(Buffer.from(got), Buffer.from(want));
    return tokenOk && sameOriginPost(req, cfg);
  };

  /**
   * The roster packs (src/pack_store.ts):
   *   GET          /api/packs          {packs: [{file, id, name, version, description, size, sha256}]}
   *   GET/HEAD     /api/packs/<file>   the zip (ETag = its SHA-256: a page revalidates and keeps its copy)
   *   POST         /api/packs          upload a zip (the body), from the Server disc page
   *   DELETE       /api/packs/<file>   remove it, from the Server disc page
   */
  const handlePacks = async (req: IncomingMessage, res: ServerResponse, method: string, path: string): Promise<void> => {
    const forbidden = () =>
      sendJson(res, { status: 403, body: { error: 'This request must come from the Server disc page of this site.' } });
    const rest = path.slice('/api/packs'.length);
    if (rest === '' || rest === '/') {
      if (method === 'GET' || method === 'HEAD') {
        res
          .writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'private, no-store' })
          .end(method === 'HEAD' ? undefined : JSON.stringify({ packs: await packs!.list() }));
        return;
      }
      if (method === 'POST') {
        if (!fromDiscPage(req)) {
          forbidden();
          return;
        }
        const type = req.headers['content-type'] ?? '';
        if (!type.startsWith('application/zip') && !type.startsWith('application/octet-stream')) {
          sendJson(res, { status: 415, body: { error: 'Expected a .zip file.' } });
          return;
        }
        if (Number(req.headers['content-length'] ?? 0) > packs!.maxBytes) {
          res.setHeader('Connection', 'close');  // refuse at once; do not wait for a body this large
          sendJson(res, { status: 413, body: { error: `A pack zip can be at most ${Math.floor(packs!.maxBytes / 1048576)} MB.` } });
          return;
        }
        const body = await readBodyBytes(req, packs!.maxBytes);
        if (body === null) {
          sendJson(res, { status: 413, body: { error: `A pack zip can be at most ${Math.floor(packs!.maxBytes / 1048576)} MB.` } });
          return;
        }
        sendJson(res, await packs!.save(body));
        return;
      }
      res.writeHead(405, { Allow: 'GET, HEAD, POST' }).end();
      return;
    }
    const file = rest.slice(1);  // `path` is decoded already
    if (!PackStore.validFile(file)) {
      sendJson(res, { status: 404, body: { error: 'No such pack.' } });
      return;
    }
    if (method === 'DELETE') {
      if (!fromDiscPage(req)) {
        forbidden();
        return;
      }
      sendJson(res, await packs!.remove(file));
      return;
    }
    if (method !== 'GET' && method !== 'HEAD') {
      res.writeHead(405, { Allow: 'GET, HEAD, DELETE' }).end();
      return;
    }
    const info = await packs!.info(file);
    const p = info ? await packs!.path(file) : null;
    if (!info || !p) {
      sendJson(res, { status: 404, body: { error: 'No such pack.' } });
      return;
    }
    const etag = `"${info.sha256}"`;
    const headers = { 'Content-Type': 'application/zip', 'Cache-Control': 'private, no-cache', ETag: etag };
    if (req.headers['if-none-match'] === etag) {
      res.writeHead(304, headers).end();
      return;
    }
    const data = method === 'HEAD' ? undefined : await readFile(p);
    res.writeHead(200, { ...headers, 'Content-Length': String(info.size) }).end(data);
  };

  const handleUpload = async (req: IncomingMessage, res: ServerResponse, method: string, path: string, url: URL): Promise<void> => {
    if (!fromDiscPage(req)) {
      sendJson(res, { status: 403, body: { error: 'This request must come from the Server disc page of this site.' } });
      return;
    }
    const parts = path.split('/').slice(4);  // after ['', 'api', 'disc', 'uploads']
    const id = parts[0] ?? '';
    if (parts.length === 0 && method === 'POST') {
      if (!(req.headers['content-type'] ?? '').startsWith('application/json')) {
        sendJson(res, { status: 415, body: { error: 'Expected JSON.' } });
        return;
      }
      const body = await readBody(req, 4096);
      let input: unknown = null;
      try {
        input = body === null ? null : JSON.parse(body);
      } catch {
        input = null;
      }
      sendJson(res, input === null ? { status: 400, body: { error: 'Bad upload request.' } } : await store!.create(input));
      return;
    }
    if (!DiscStore.validId(id)) {
      sendJson(res, { status: 404, body: { error: 'No such upload.' } });
      return;
    }
    if (parts.length === 1 && method === 'PUT') {
      const offset = Number(url.searchParams.get('offset') ?? NaN);
      if (!Number.isSafeInteger(offset) || offset < 0 || req.headers['content-length'] === undefined) {
        sendJson(res, { status: 411, body: { error: 'A chunk needs ?offset= and a Content-Length.' } });
        return;
      }
      sendJson(res, await store!.write(id, offset, Number(req.headers['content-length']), req));
      return;
    }
    if (parts.length === 2 && parts[1] === 'finish' && method === 'POST') {
      const a = await store!.finish(id);
      if (a.status === 200) warmManifest();  // players' pages download the new image (P1)
      sendJson(res, a);
      return;
    }
    if (parts.length === 1 && method === 'DELETE') {
      sendJson(res, await store!.cancel(id));
      return;
    }
    sendJson(res, { status: 405, body: { error: 'Method not allowed.' } });
  };

  const handle = async (req: IncomingMessage, res: ServerResponse): Promise<void> => {
    for (const [k, v] of Object.entries(COMMON_HEADERS)) res.setHeader(k, v);
    let url: URL;
    try {
      url = new URL(req.url ?? '/', 'http://localhost');
      decodeURIComponent(url.pathname);
    } catch {
      res.writeHead(400).end();
      return;
    }
    const path = decodeURIComponent(url.pathname);
    const method = req.method ?? 'GET';

    // The private link: the right key gives this browser the cookie and goes on to the same address without the key
    // (so it stays out of the address bar, history and bookmarks).
    const key = url.searchParams.get('key');
    if (key !== null && sameSecret(key, cfg.accessKey)) {
      url.searchParams.delete('key');
      const location = url.pathname.replace(/^\/+/, '/') + url.search;  // never "//host"
      res
        .writeHead(303, { Location: location, 'Set-Cookie': accessCookie(token, cfg.secureCookies), 'Cache-Control': 'no-store' })
        .end();
      return;
    }
    if (path === '/healthz') {
      res.writeHead(200, { 'Content-Type': 'text/plain', 'Cache-Control': 'no-store' }).end('ok');
      return;
    }
    if (path === '/robots.txt') {
      res.writeHead(200, { 'Content-Type': 'text/plain; charset=utf-8', 'Cache-Control': 'no-cache' }).end('User-agent: *\nDisallow: /\n');
      return;
    }

    // ---- everything below needs access (a wrong key counts as none) ----
    if (key !== null || !hasAccess(req)) {
      if (isNavigation(req)) {
        res
          .writeHead(403, { 'Content-Type': 'text/html; charset=utf-8', 'Cache-Control': 'no-store', 'Content-Security-Policy': NO_ACCESS_CSP })
          .end(noAccessPage());
      } else {
        res.writeHead(401, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' }).end('{"error":"no access"}');
      }
      return;
    }
    // Every page load renews the cookie: a browser in use keeps access.
    if (method === 'GET' && isNavigation(req)) res.setHeader('Set-Cookie', accessCookie(token, cfg.secureCookies));
    if (store && path.startsWith('/api/disc/uploads')) {
      await handleUpload(req, res, method, path, url);
      return;
    }
    if (packs && (path === '/api/packs' || path.startsWith('/api/packs/'))) {
      await handlePacks(req, res, method, path);
      return;
    }
    if (method !== 'GET' && method !== 'HEAD') {
      res.writeHead(405, { Allow: 'GET, HEAD' }).end();
      return;
    }
    if (path === '/config.json') {
      const d = servedDisc();
      const body: Record<string, string | number | boolean> = {};
      if (d) {
        body.disc = d.url;
        // The image hash names the image: a page keeps one downloaded copy per hash (P1).
        const m = await manifests.get(d.path, d.sidecar);
        if (m === 'pending') body.discPreparing = true;
        else if (m) {
          body.discHash = m.hash;
          body.discSize = m.size;
          body.discManifest = d.version ? `/api/disc/manifest?v=${d.version}` : '/api/disc/manifest';
        }
      }
      if (store || packs) body.discPage = '/disc';
      if (packs) body.packs = '/api/packs';
      res.writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'private, no-store' }).end(JSON.stringify(body));
      return;
    }
    if (path === '/api/disc/manifest') {
      const d = servedDisc();
      const v = url.searchParams.get('v');
      const json = (status: number, body: unknown, extra: Record<string, string> = {}) =>
        res.writeHead(status, { 'Content-Type': 'application/json', 'Cache-Control': 'private, no-store', ...extra }).end(JSON.stringify(body));
      if (!d || (v !== null && v !== d.version)) {
        json(404, { error: d ? 'the disc image changed; reload the page' : 'no disc image' });
        return;
      }
      const m = await manifests.get(d.path, d.sidecar);
      if (m === 'pending') json(503, { error: 'the server is preparing the disc image; try again in a moment' }, { 'Retry-After': '2' });
      else if (!m) json(404, { error: 'the disc image could not be hashed' });
      else json(200, { version: d.version, ...m });
      return;
    }
    if (path === '/api/disc/image') {
      const d = servedDisc();
      const v = url.searchParams.get('v');
      if (!d || (v !== null && v !== d.version)) {
        // No image, or the one this page started with was replaced: the page must reload.
        res
          .writeHead(404, { 'Content-Type': 'application/json', 'Cache-Control': 'private, no-store' })
          .end(d ? '{"error":"the disc image changed; reload the page"}' : '{"error":"no disc image"}');
        return;
      }
      await serveDisc(d.path, req, res);
      return;
    }
    if ((store || packs) && path === '/disc') {
      const csrf = discToken(parseCookies(req.headers.cookie).get(ACCESS_COOKIE)!);
      // The private link as the owner would send it: on the public URL, else on the address this page was opened at.
      const origin = cfg.publicUrl
        ? new URL(cfg.publicUrl).origin
        : `${cfg.secureCookies ? 'https' : 'http'}://${req.headers.host ?? 'localhost'}`;
      res
        .writeHead(200, { 'Content-Type': 'text/html; charset=utf-8', 'Cache-Control': 'no-store', 'Content-Security-Policy': DISC_PAGE_CSP })
        .end(method === 'HEAD' ? undefined : discPage(csrf, privateLink(origin, cfg.accessKey), { disc: !!store, packs: !!packs }));
      return;
    }
    if (store && path === '/api/disc') {
      sendJson(res, { status: 200, body: store.status() });
      return;
    }
    if (path === '/api/rooms') {
      res.writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' }).end(JSON.stringify({ rooms: lobby.list() }));
      return;
    }
    if (path.startsWith('/api/')) {
      res.writeHead(404, { 'Content-Type': 'application/json' }).end('{"error":"not found"}');
      return;
    }
    res.setHeader('Content-Security-Policy', SITE_CSP);
    await serveStatic(cfg.site, path, req, res);
  };

  const server = createServer((req, res) => {
    handle(req, res).catch(() => {
      if (!res.headersSent) res.writeHead(500).end();
      else res.destroy();
    });
  });

  server.on('upgrade', (req, socket, head) => {
    const path = (req.url ?? '').split('?')[0];
    if (path !== '/ws' || !hasAccess(req) || !originAllowed(req, cfg)) {
      socket.write('HTTP/1.1 401 Unauthorized\r\nConnection: close\r\nContent-Length: 0\r\n\r\n');
      socket.destroy();
      return;
    }
    // Invite links and the ICE servers' host follow the address the page used (or the configured public URL).
    const origin = cfg.publicUrl ? new URL(cfg.publicUrl).origin : String(req.headers.origin);
    wss.handleUpgrade(req, socket, head, (ws) => {
      const client = lobby.connect(ws, origin);
      let alive = true;
      ws.on('pong', () => (alive = true));
      const ping = setInterval(() => {
        if (!alive) {
          ws.terminate();
          return;
        }
        alive = false;
        ws.ping();
      }, 30_000);
      ws.on('message', (data, isBinary) => {
        if (isBinary) return;
        lobby.handle(client, data.toString());
      });
      ws.on('close', () => {
        clearInterval(ping);
        lobby.leave(client);
      });
    });
  });

  await new Promise<void>((resolve) => server.listen(cfg.port, cfg.host, resolve));
  const addr = server.address() as AddressInfo;
  const local = `http://${addr.address.includes(':') ? `[${addr.address}]` : addr.address}:${addr.port}`;
  return {
    server,
    lobby,
    url: local,
    link: privateLink(local, cfg.accessKey),
    discReady: async () => {
      const d = servedDisc();
      if (d) await manifests.ready(d.path, d.sidecar);
    },
    close: () =>
      new Promise<void>((resolve) => {
        for (const ws of wss.clients) ws.terminate();
        wss.close();
        server.closeAllConnections();
        server.close(() => resolve());
      }),
  };
}
