// Where the disc image comes from (E2c).
//
// On load the page asks the server for /config.json. On the owner's deployment
// (owner decisions 2026-10-14 and 2026-10-20, docs/DECISIONS.md) it holds
// {"disc": "<url>"} for a player (a browser that opened the player link or an
// invite): the image is served from that same-origin URL, and the engine Worker
// streams it with Range requests (web_disc.h); the page shows no picker.
// Anywhere else, and for anyone else on the open site (no config.json, or no
// "disc" in it), the player picks their own image, as before. The identity check
// (GV4E69 rev 0, by the DOL's SHA-1) runs on either.
//
// The server's contract (E5 builds it in engine/server/): GET /config.json;
// HEAD on the image URL gives its size; GET with a Range header answers 206;
// both carry Cache-Control: private, no-store; the image's routes answer 401 to a
// browser that is not a player.
//
// P1: the server also advertises "discHash", "discSize" and "discManifest" (the
// image's piece hashes): the page then downloads the image once into this
// browser's storage and the engine reads it locally (disc-store.ts), instead of
// over the network during play. "discPreparing" means the server is still
// hashing the image (once after an upload or an update): the page asks again.
// "discPage" names the Server disc page (the operator's upload page), and "packs"
// the roster packs the server offers (GET: {packs: [...]}; packs.ts shows them in
// a dropdown). For the server's admin it also says "admin": true, with
// "adminToken" (the CSRF token its own requests carry) and "states" (its save
// states on the server: states.ts).
//
// fetch is a parameter so the unit tests (tests/disc.test.mjs) run these
// against fake responses.

import type { DiscOffer } from './disc-store.js';

/** This browser is the server's admin: its save states (`states`, GET/POST/DELETE) and the CSRF token its requests carry. */
export interface ServerAdmin {
  token: string;
  states?: string;
}

export type ServerDisc =
  | { kind: 'url'; url: string; offer?: DiscOffer; preparing?: boolean; discPage?: string; packs?: string; admin?: ServerAdmin }
  // `deployment`: /config.json is there (the netplay server) but names no disc (none on it, or not for this browser).
  | { kind: 'none'; why: string; deployment?: boolean; discPage?: string; packs?: string; admin?: ServerAdmin }
  | { kind: 'error'; message: string; login?: boolean };

type Fetch = (input: string, init?: RequestInit) => Promise<Response>;

const LOGIN = 'This browser may not use the disc on this server any more. Open the player link again, then reload the page.';

export async function findServerDisc(base: string, fetchFn: Fetch = fetch): Promise<ServerDisc> {
  let res: Response;
  try {
    res = await fetchFn(new URL('/config.json', base).href, { credentials: 'same-origin', cache: 'no-store' });
  } catch (e) {
    return { kind: 'none', why: `no /config.json (${e})` };
  }
  if (res.status === 401) return { kind: 'error', message: LOGIN, login: true };
  if (!res.ok) return { kind: 'none', why: `/config.json: HTTP ${res.status}` };
  let cfg: unknown;
  try {
    cfg = await res.json();
  } catch {
    return { kind: 'none', why: '/config.json is not JSON' };
  }
  const c = (cfg && typeof cfg === 'object' ? cfg : {}) as Record<string, unknown>;
  const sameOrigin = (v: unknown): string | null => {
    if (typeof v !== 'string' || !v) return null;
    try {
      const u = new URL(v, base);
      return u.origin === new URL(base).origin ? u.href : null;
    } catch {
      return null;
    }
  };
  const discPage = sameOrigin(c.discPage) ?? undefined;
  const packs = sameOrigin(c.packs) ?? undefined;
  let admin: ServerAdmin | undefined;
  if (c.admin === true && typeof c.adminToken === 'string' && c.adminToken) {
    admin = { token: c.adminToken };
    const states = sameOrigin(c.states);
    if (states) admin.states = states;
  }
  const disc = c.disc;
  if (typeof disc !== 'string' || !disc) {
    const none: ServerDisc & { kind: 'none' } = { kind: 'none', why: '/config.json names no disc', deployment: !!cfg && typeof cfg === 'object', discPage };
    if (packs) none.packs = packs;
    if (admin) none.admin = admin;
    return none;
  }
  let url: URL;
  try {
    url = new URL(disc, base);
  } catch {
    return { kind: 'error', message: `/config.json: "${disc}" is not a URL` };
  }
  if (url.origin !== new URL(base).origin)
    return { kind: 'error', message: `/config.json: the disc must be on this site (${url.origin} is not)` };
  const found: ServerDisc & { kind: 'url' } = { kind: 'url', url: url.href };
  if (discPage) found.discPage = discPage;
  if (packs) found.packs = packs;
  if (admin) found.admin = admin;
  const manifest = sameOrigin(c.discManifest);
  if (typeof c.discHash === 'string' && /^[0-9a-f]{64}$/.test(c.discHash) && Number.isSafeInteger(c.discSize) &&
      (c.discSize as number) > 0 && manifest)
    found.offer = { url: url.href, manifest, hash: c.discHash, size: c.discSize as number };
  else if (c.discPreparing === true) found.preparing = true;
  return found;
}

export type DiscProbe = { ok: true; size: number } | { ok: false; message: string; login?: boolean };

// The image's size (HEAD), and that the server honours Range requests (a
// one-byte ranged GET must answer 206; a 200 would send the whole image on
// every read). Runs in the Worker before the engine opens the image.
export async function probeDiscUrl(url: string, fetchFn: Fetch = fetch): Promise<DiscProbe> {
  const status = (s: number, what: string): DiscProbe =>
    s === 401 || s === 403 ? { ok: false, message: LOGIN, login: true }
                           : { ok: false, message: `The server answered HTTP ${s} for the disc image (${what}).` };
  let head: Response;
  try {
    head = await fetchFn(url, { method: 'HEAD', credentials: 'same-origin', cache: 'no-store' });
  } catch (e) {
    return { ok: false, message: `The disc image could not be reached (${e}).` };
  }
  if (!head.ok) return status(head.status, 'HEAD');
  let size = Number(head.headers.get('Content-Length') ?? NaN);
  let ranged: Response;
  try {
    ranged = await fetchFn(url, { headers: { Range: 'bytes=0-0' }, credentials: 'same-origin', cache: 'no-store' });
  } catch (e) {
    return { ok: false, message: `The disc image could not be read (${e}).` };
  }
  if (ranged.status !== 206) {
    try {
      await ranged.body?.cancel();
    } catch {
      // nothing to cancel
    }
    return ranged.ok ? { ok: false, message: 'The server does not answer Range requests for the disc image.' }
                     : status(ranged.status, 'Range GET');
  }
  await ranged.arrayBuffer();
  // Content-Range: bytes 0-0/TOTAL gives the size too.
  const m = /\/(\d+)\s*$/.exec(ranged.headers.get('Content-Range') ?? '');
  if (!(size > 0) && m) size = Number(m[1]);
  if (!(size > 0)) return { ok: false, message: 'The server did not say how large the disc image is.' };
  return { ok: true, size };
}
