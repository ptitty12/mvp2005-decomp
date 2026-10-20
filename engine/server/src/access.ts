// Who may do what on the server (owner, 2026-10-20: "just anyone who goes to the
// site should be able to access", "only i should be able to see /access the save
// states"; DECISIONS). The site is open: anyone can load the page, play with a
// disc image of their own, use the lobby and the roster packs. Two links give a
// browser more, each opened once (the server answers with a cookie that lasts
// 400 days, renewed on every page load, and nobody types anything):
//
// - the player link, https://site/?key=<access key>: this server's own disc
//   image. It stays behind a link because serving the whole retail disc to
//   anyone who finds the address would be distributing it (DECISIONS
//   2026-10-17). Lobby invites carry it, and browsers that opened the old
//   private link keep it.
// - the admin link, https://site/?admin=<admin key>: the owner. The Server disc
//   page (the disc and pack uploads) and the save states (src/state_store.ts).
//   The admin is a player too.
//
// Each cookie is HMAC-SHA256(key, label): stateless, so it survives restarts and
// redeploys, and a new key takes that access from every browser at once.
import { createHash, createHmac, timingSafeEqual } from 'node:crypto';
import type { IncomingMessage } from 'node:http';

export const ACCESS_COOKIE = 'mvp_access';
export const ADMIN_COOKIE = 'mvp_admin';
/** How long a browser keeps access after its last page load (400 days: browsers cap cookie lifetimes there). */
export const ACCESS_DAYS = 400;

/** The cookie value that shows a browser opened the player link. */
export function accessToken(key: string): string {
  return createHmac('sha256', key).update('mvp-access-v1').digest('base64url');
}

/** The cookie value that shows a browser opened the admin link. */
export function adminToken(key: string): string {
  return createHmac('sha256', key).update('mvp-admin-v1').digest('base64url');
}

/** Compares a candidate with a secret in constant time (hashed first, so the lengths do not show either). */
export function sameSecret(candidate: string | null | undefined, secret: string): boolean {
  if (typeof candidate !== 'string') return false;
  const a = createHash('sha256').update(candidate, 'utf8').digest();
  const b = createHash('sha256').update(secret, 'utf8').digest();
  return timingSafeEqual(a, b);
}

function cookie(name: string, token: string, secure: boolean): string {
  return `${name}=${token}; Path=/; HttpOnly; SameSite=Lax; Max-Age=${ACCESS_DAYS * 86400}${secure ? '; Secure' : ''}`;
}

export function accessCookie(token: string, secure: boolean): string {
  return cookie(ACCESS_COOKIE, token, secure);
}

export function adminCookie(token: string, secure: boolean): string {
  return cookie(ADMIN_COOKIE, token, secure);
}

/** The player link on `origin` ("https://mvp.example.com"); `room`: an invite link to that game. */
export function privateLink(origin: string, key: string, room?: string): string {
  return `${origin}/?${room ? `room=${encodeURIComponent(room)}&` : ''}key=${encodeURIComponent(key)}`;
}

/** The admin link on `origin`. */
export function adminLink(origin: string, key: string): string {
  return `${origin}/?admin=${encodeURIComponent(key)}`;
}

export function parseCookies(header: string | undefined): Map<string, string> {
  const out = new Map<string, string>();
  for (const part of (header ?? '').split(';')) {
    const eq = part.indexOf('=');
    if (eq < 0) continue;
    const k = part.slice(0, eq).trim();
    if (k && !out.has(k)) out.set(k, part.slice(eq + 1).trim());
  }
  return out;
}

/** Whether a request is a page load rather than an API call. */
export function isNavigation(req: IncomingMessage): boolean {
  const mode = req.headers['sec-fetch-mode'];
  if (mode) return mode === 'navigate';
  return req.method === 'GET' && (req.headers.accept ?? '').includes('text/html');
}

export function escapeHtml(s: string): string {
  return s.replace(/[&<>"']/g, (c) => `&#${c.charCodeAt(0)};`);
}
