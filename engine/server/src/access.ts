// Access to the private server: a private link instead of a password (owner,
// 2026-10-17). The owner opens https://site/?key=<key> once in each browser;
// the server answers with a cookie that keeps that browser in for 400 days
// (renewed on every page load), and nobody ever types anything.
//
// The cookie is HMAC-SHA256(key, "mvp-access-v1"): stateless, so it survives
// restarts and redeploys, and a new key (MVP_ACCESS_KEY) locks out every
// browser at once. Lobby invite links carry the key, so an invite also lets a
// new browser in. A browser without the cookie sees only a short "private
// server" page (noAccessPage) or a 401.
import { createHash, createHmac, timingSafeEqual } from 'node:crypto';
import type { IncomingMessage } from 'node:http';

import { THEME_CSS, TOPBAR_HTML } from './theme.js';

export const ACCESS_COOKIE = 'mvp_access';
/** How long a browser keeps access after its last page load (400 days: browsers cap cookie lifetimes there). */
export const ACCESS_DAYS = 400;

/** The cookie value that shows a browser opened the private link. */
export function accessToken(key: string): string {
  return createHmac('sha256', key).update('mvp-access-v1').digest('base64url');
}

/** Compares a candidate with a secret in constant time (hashed first, so the lengths do not show either). */
export function sameSecret(candidate: string | null | undefined, secret: string): boolean {
  if (typeof candidate !== 'string') return false;
  const a = createHash('sha256').update(candidate, 'utf8').digest();
  const b = createHash('sha256').update(secret, 'utf8').digest();
  return timingSafeEqual(a, b);
}

export function accessCookie(token: string, secure: boolean): string {
  return `${ACCESS_COOKIE}=${token}; Path=/; HttpOnly; SameSite=Lax; Max-Age=${ACCESS_DAYS * 86400}${secure ? '; Secure' : ''}`;
}

/** The private link on `origin` ("https://mvp.example.com"); `room`: an invite link to that game. */
export function privateLink(origin: string, key: string, room?: string): string {
  return `${origin}/?${room ? `room=${encodeURIComponent(room)}&` : ''}key=${encodeURIComponent(key)}`;
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

/** Whether a request is a page load (gets the "private server" page) rather than an API call (gets 401). */
export function isNavigation(req: IncomingMessage): boolean {
  const mode = req.headers['sec-fetch-mode'];
  if (mode) return mode === 'navigate';
  return req.method === 'GET' && (req.headers.accept ?? '').includes('text/html');
}

export function escapeHtml(s: string): string {
  return s.replace(/[&<>"']/g, (c) => `&#${c.charCodeAt(0)};`);
}

// ---- the page a browser without access sees ----

/** The game page's look (src/theme.ts), with one narrow card. */
export const NO_ACCESS_CSS = `${THEME_CSS}
main,.title{max-width:512px}main{margin-top:12vh}`;

const sha = (s: string) => `'sha256-${createHash('sha256').update(s).digest('base64')}'`;
/** CSP of the "private server" page: its one style by hash, no script. */
export const NO_ACCESS_CSP = `default-src 'none'; style-src ${sha(NO_ACCESS_CSS)}; img-src data:; frame-ancestors 'none'; base-uri 'none'`;

/** The "private server" page: says nothing about the server beyond that. */
export function noAccessPage(): string {
  return `<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="robots" content="noindex"><title>MVP Baseball 2005</title><link rel="icon" href="data:,"><style>${NO_ACCESS_CSS}</style></head><body>
${TOPBAR_HTML}<main><div class="card"><h1>Private server</h1><p>This is a private server. Open the private link you were sent.</p></div></main></body></html>`;
}
