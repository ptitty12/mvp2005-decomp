// The VPS server (E5): one Node process behind a TLS reverse proxy.
//
// Who may do what (src/access.ts): anyone, a player (opened the player link, or
// an invite: this server's disc) and the admin (opened the admin link).
//
//   any  /...?key=<key>                      the player link: 303 to the same address without the
//                                            key, with the access cookie
//   any  /...?admin=<key>                    the admin link: the same, with the admin cookie too
//   GET  /healthz                            "ok" (for monitoring)
//   GET  /robots.txt                         "Disallow: /"
//   GET  /config.json                        what this browser may use: "packs"; a player also "disc"
//                                            ("/api/disc/image") with "discHash", "discSize" and
//                                            "discManifest" (P1), or "discPreparing" while the image is
//                                            being hashed; the admin also "admin", "adminToken",
//                                            "discPage", "states" and "card"
//   GET/HEAD /api/disc/image                 the owner's disc image (src/files.ts): players
//   GET  /api/disc/manifest                  the image's piece hashes, for the players' one-time download
//                                            into their own browser storage (src/disc_hash.ts): players
//   GET  /disc, /api/disc, /api/disc/uploads  the Server disc page and its chunked upload (deploy:
//                                            src/disc_page.ts, src/disc_store.ts): the admin
//   GET  /api/packs, /api/packs/<file>       the roster packs the pages offer in a dropdown, and one
//                                            pack's zip (src/pack_store.ts), with a packs directory
//   POST /api/packs, DELETE /api/packs/<file> upload or remove a pack: the admin, from its pages
//   GET  /api/states, /api/states/<name>     the admin's save states and one of them (gzipped,
//                                            src/state_store.ts), with a states directory
//   POST /api/states/<name>, DELETE ...      store (the body) or remove a state: the admin, from its pages
//   GET  /states/<name>.mvpstate             one of them for the game page's ?state=<name>: the admin
//   GET/HEAD /api/card, PUT /api/card        the admin's memory card (gzipped, src/card_store.ts), with a
//                                            states directory: the admin; PUT from its pages
//   GET  /api/rooms                          open rooms (src/lobby.ts)
//   WS   /ws                                 lobby and WebRTC signaling (this site's pages only)
//   GET  anything else                       the engine site (static files)
import { createHmac, timingSafeEqual } from 'node:crypto';
import { createServer, type IncomingMessage, type Server, type ServerResponse } from 'node:http';
import { readFile } from 'node:fs/promises';
import type { AddressInfo } from 'node:net';
import { WebSocketServer } from 'ws';

import {
  ACCESS_COOKIE,
  ADMIN_COOKIE,
  accessCookie,
  accessToken,
  adminCookie,
  adminLink,
  adminToken,
  isNavigation,
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
import { StateStore } from './state_store.js';
import { CardStore } from './card_store.js';
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
  /** The player link on `url` (tests and tools): opening it lets a browser use the server's disc. */
  link: string;
  /** The admin link on `url` (tests and tools); '' without an admin key. */
  adminLink: string;
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
  // The cookies' values: the same for every browser with that access, and across restarts (src/access.ts).
  const token = accessToken(cfg.accessKey);
  const adminTok = cfg.adminKey ? adminToken(cfg.adminKey) : '';
  /** The admin: opened the admin link (never without an admin key). */
  const isAdmin = (req: IncomingMessage) => !!adminTok && sameSecret(parseCookies(req.headers.cookie).get(ADMIN_COOKIE), adminTok);
  /** A player: opened the player link (or an invite), or is the admin. Players may use the server's disc. */
  const isPlayer = (req: IncomingMessage) => sameSecret(parseCookies(req.headers.cookie).get(ACCESS_COOKIE), token) || isAdmin(req);
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
  // The admin's save states: only with a states directory and an admin to see them.
  const states = cfg.statesDir && cfg.adminKey ? await StateStore.open({ dir: cfg.statesDir, maxBytes: cfg.maxStateBytes, log }) : null;
  if (cfg.statesDir && !cfg.adminKey) log('save states are off: they need an admin key (MVP_ADMIN_KEY or MVP_ADMIN_KEY_FILE)');
  // The admin's memory card: beside the states.
  const card = states && cfg.statesDir ? new CardStore(cfg.statesDir, log) : null;
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
  // CSRF tokens of the admin's pages (the Server disc page; the game page gets its own in /config.json): bound to
  // the admin cookie, under a key derived from the admin key, so an open page keeps working across a restart (an
  // upload continues) and a page elsewhere can neither read nor forge one.
  const discKey = createHmac('sha256', cfg.adminKey || cfg.accessKey).update('mvp-disc-csrf-v1').digest();
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
  /** A request from one of the admin's pages: the admin cookie, the page's CSRF token, same-origin. */
  const fromDiscPage = (req: IncomingMessage): boolean => {
    if (!isAdmin(req)) return false;
    const cookie = parseCookies(req.headers.cookie).get(ADMIN_COOKIE)!;
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

  /** One stored state as the file it is (gzipped): the browser inflates it (Content-Encoding). */
  const sendState = async (res: ServerResponse, method: string, name: string): Promise<void> => {
    const p = await states!.path(name);
    if (!p) {
      sendJson(res, { status: 404, body: { error: 'No such state.' } });
      return;
    }
    const data = await readFile(p);
    res
      .writeHead(200, {
        'Content-Type': 'application/octet-stream',
        'Content-Encoding': 'gzip',
        'Content-Length': String(data.length),
        'Cache-Control': 'private, no-store',
      })
      .end(method === 'HEAD' ? undefined : data);
  };

  /**
   * The admin's save states (src/state_store.ts), for the admin only (the caller checked):
   *   GET          /api/states          {states: [{name, label, frame, mods, packs, size, saved}]}
   *   GET/HEAD     /api/states/<name>   the state (gzipped, Content-Encoding: gzip)
   *   POST         /api/states/<name>   store the body (a state, gzipped or not) under that name, replacing one
   *   DELETE       /api/states/<name>   remove it
   * POST and DELETE come from the admin's pages (the CSRF token).
   */
  const handleStates = async (req: IncomingMessage, res: ServerResponse, method: string, path: string): Promise<void> => {
    const rest = path.slice('/api/states'.length);
    if (rest === '' || rest === '/') {
      if (method !== 'GET' && method !== 'HEAD') {
        res.writeHead(405, { Allow: 'GET, HEAD' }).end();
        return;
      }
      res
        .writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'private, no-store' })
        .end(method === 'HEAD' ? undefined : JSON.stringify({ states: await states!.list() }));
      return;
    }
    const name = rest.slice(1);
    if (!StateStore.validName(name)) {
      sendJson(res, { status: 404, body: { error: 'No such state.' } });
      return;
    }
    if (method === 'GET' || method === 'HEAD') {
      await sendState(res, method, name);
      return;
    }
    if (method !== 'POST' && method !== 'DELETE') {
      res.writeHead(405, { Allow: 'GET, HEAD, POST, DELETE' }).end();
      return;
    }
    if (!fromDiscPage(req)) {
      sendJson(res, { status: 403, body: { error: 'This request must come from a page of this site, opened by its admin.' } });
      return;
    }
    if (method === 'DELETE') {
      sendJson(res, await states!.remove(name));
      return;
    }
    const tooLarge = { status: 413, body: { error: `A save state can be at most ${Math.floor(states!.maxBytes / 1048576)} MB.` } };
    if (Number(req.headers['content-length'] ?? 0) > states!.maxBytes) {
      res.setHeader('Connection', 'close');  // refuse at once; do not wait for a body this large
      sendJson(res, tooLarge);
      return;
    }
    const body = await readBodyBytes(req, states!.maxBytes);
    sendJson(res, body === null ? tooLarge : await states!.save(name, body));
  };

  /**
   * The admin's memory card (src/card_store.ts), for the admin only (the caller checked):
   *   GET/HEAD  /api/card   the card (gzipped, Content-Encoding: gzip) and its ETag; 404 before the first write
   *   PUT       /api/card   store the body (a card, gzipped or not) if the server's is still the one
   *                         If-Match names (If-None-Match: * when there is none): {etag}, else 412 {error, etag}
   * PUT comes from the admin's pages (the CSRF token).
   */
  const handleCard = async (req: IncomingMessage, res: ServerResponse, method: string): Promise<void> => {
    if (method === 'GET' || method === 'HEAD') {
      const got = await card!.read();
      if (!got) {
        sendJson(res, { status: 404, body: { error: 'No memory card on the server yet.' } });
        return;
      }
      res
        .writeHead(200, {
          'Content-Type': 'application/octet-stream',
          'Content-Encoding': 'gzip',
          'Content-Length': String(got.data.length),
          'Cache-Control': 'private, no-store',
          ETag: got.etag,
        })
        .end(method === 'HEAD' ? undefined : got.data);
      return;
    }
    if (method !== 'PUT') {
      res.writeHead(405, { Allow: 'GET, HEAD, PUT' }).end();
      return;
    }
    if (!fromDiscPage(req)) {
      sendJson(res, { status: 403, body: { error: 'This request must come from a page of this site, opened by its admin.' } });
      return;
    }
    const tooLarge = { status: 413, body: { error: 'Not a memory card: too large.' } };
    if (Number(req.headers['content-length'] ?? 0) > CardStore.maxBody) {
      res.setHeader('Connection', 'close');
      sendJson(res, tooLarge);
      return;
    }
    const body = await readBodyBytes(req, CardStore.maxBody);
    const header = (h: string | string[] | undefined) => (typeof h === 'string' ? h.trim() : undefined);
    sendJson(res, body === null ? tooLarge : await card!.write(body, header(req.headers['if-match']), header(req.headers['if-none-match'])));
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

    // The player and admin links: the right key gives this browser its cookie(s) and goes on to the same address
    // without the key (so it stays out of the address bar, history and bookmarks). A wrong key gives nothing.
    const key = url.searchParams.get('key');
    const admin = url.searchParams.get('admin');
    const adminOk = admin !== null && !!cfg.adminKey && sameSecret(admin, cfg.adminKey);
    if (adminOk || (key !== null && sameSecret(key, cfg.accessKey))) {
      url.searchParams.delete('key');
      url.searchParams.delete('admin');
      const location = url.pathname.replace(/^\/+/, '/') + url.search;  // never "//host"
      const cookies = [accessCookie(token, cfg.secureCookies)];
      if (adminOk) cookies.push(adminCookie(adminTok, cfg.secureCookies));
      res.writeHead(303, { Location: location, 'Set-Cookie': cookies, 'Cache-Control': 'no-store' }).end();
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

    // ---- open to anyone from here; the player's and the admin's routes check for themselves ----
    const player = isPlayer(req), owner = isAdmin(req);
    /** The answer for a route this browser may not use: 401, without saying more. */
    const noAccess = () => res.writeHead(401, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' }).end('{"error":"no access"}');
    // Every page load renews the cookies: a browser in use keeps its access.
    if (method === 'GET' && isNavigation(req)) {
      const renew: string[] = [];
      if (player) renew.push(accessCookie(token, cfg.secureCookies));
      if (owner) renew.push(adminCookie(adminTok, cfg.secureCookies));
      if (renew.length) res.setHeader('Set-Cookie', renew);
    }
    if (store && path.startsWith('/api/disc/uploads')) {
      if (!owner) noAccess();
      else await handleUpload(req, res, method, path, url);
      return;
    }
    if (packs && (path === '/api/packs' || path.startsWith('/api/packs/'))) {
      await handlePacks(req, res, method, path);
      return;
    }
    if (states && (path === '/api/states' || path.startsWith('/api/states/'))) {
      if (!owner) noAccess();
      else await handleStates(req, res, method, path);
      return;
    }
    if (card && path === '/api/card') {
      if (!owner) noAccess();
      else await handleCard(req, res, method);
      return;
    }
    if (method !== 'GET' && method !== 'HEAD') {
      res.writeHead(405, { Allow: 'GET, HEAD' }).end();
      return;
    }
    if (path === '/config.json') {
      const d = player ? servedDisc() : null;  // the server's disc is for players only
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
      if (packs) body.packs = '/api/packs';
      if (owner) {
        body.admin = true;
        // The CSRF token the game page sends with the admin's own requests (storing and removing save states).
        body.adminToken = discToken(parseCookies(req.headers.cookie).get(ADMIN_COOKIE)!);
        if (store || packs || states) body.discPage = '/disc';
        if (states) body.states = '/api/states';
        if (card) body.card = '/api/card';
      }
      res.writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'private, no-store' }).end(JSON.stringify(body));
      return;
    }
    if ((path === '/api/disc/manifest' || path === '/api/disc/image') && !player) {
      noAccess();
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
    // The Server disc page is the admin's; for anyone else it does not exist.
    if (owner && (store || packs || states) && path === '/disc') {
      const csrf = discToken(parseCookies(req.headers.cookie).get(ADMIN_COOKIE)!);
      // The player link as the owner would send it: on the public URL, else on the address this page was opened at.
      const origin = cfg.publicUrl
        ? new URL(cfg.publicUrl).origin
        : `${cfg.secureCookies ? 'https' : 'http'}://${req.headers.host ?? 'localhost'}`;
      res
        .writeHead(200, { 'Content-Type': 'text/html; charset=utf-8', 'Cache-Control': 'no-store', 'Content-Security-Policy': DISC_PAGE_CSP })
        .end(method === 'HEAD' ? undefined : discPage(csrf, privateLink(origin, cfg.accessKey), { disc: !!store, packs: !!packs, states: !!states }));
      return;
    }
    if (store && path === '/api/disc') {
      if (!owner) noAccess();
      else sendJson(res, { status: 200, body: store.status() });
      return;
    }
    // ?state=<name> on the game page: /states/<name>.mvpstate (src/state_store.ts), the admin's only.
    if (states && path.startsWith('/states/') && path.endsWith('.mvpstate')) {
      if (!owner) noAccess();
      else await sendState(res, method, path.slice('/states/'.length, -'.mvpstate'.length));
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
    if (path !== '/ws' || !originAllowed(req, cfg)) {  // the site's own pages only (anyone may play)
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
    adminLink: cfg.adminKey ? adminLink(local, cfg.adminKey) : '',
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
