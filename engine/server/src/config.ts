// Server configuration (E5). Non-secret settings come from a JSON file
// (MVP_SERVER_CONFIG) and environment variables; secrets come only from the
// environment (or files named by it), never from the repository:
//
//   MVP_PASSWORD / MVP_PASSWORD_FILE        the site password (optional: tests and CI;
//                                           it wins over the stored one, E5b)
//   MVP_TURN_SECRET / MVP_TURN_SECRET_FILE  coturn's static-auth-secret (optional; with
//                                           MVP_TURN_SECRET_CREATE=1 a missing file is
//                                           created with a random secret: the Docker deployment)
//
// Without MVP_PASSWORD the owner creates the password in the browser on first
// run (src/setup.ts); the hash is kept in the state file (MVP_STATE_FILE,
// "stateFile"), which is then required.
//
// The Docker deployment (deploy/, docs/engine/DEPLOY_DOKPLOY.md) sets everything
// through the environment: MVP_DISC_DIR (the uploaded disc), MVP_PACKS_DIR (the roster
// packs the pages offer in a dropdown), MVP_TURN_MODE (the
// ICE servers handed to the pages) and the secret file above.
//
// docs/engine/NETPLAY.md ("Deploying on a VPS") has a complete example.
import { randomBytes } from 'node:crypto';
import { chmodSync, existsSync, mkdirSync, readFileSync, renameSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';

import { RETAIL_DISC, type ExpectedDisc } from './disc_image.js';

export interface TurnConfig {
  /**
   * turn:/turns: URLs of the relay, e.g. "turn:turn.example.com:3478?transport=udp". "{host}" stands
   * for the host name of the page (the public URL's, else the one the browser used).
   */
  urls: string[];
  /** coturn's static-auth-secret (use-auth-secret): credentials are derived per session. */
  secret: string;
  /** Lifetime of the derived credentials. */
  ttlSeconds: number;
}

export interface ServerConfig {
  host: string;
  port: number;
  /** The built engine site (build/engine/emscripten/web/site). */
  site: string;
  /** The owner's disc image (ISO or CISO), served only behind the login; empty: none. */
  disc: string;
  /**
   * Where an uploaded disc image is kept (the Server disc page, src/disc_store.ts); empty: no uploads.
   * An image there wins over `disc`.
   */
  discDir: string;
  /** The largest upload accepted, in bytes. */
  maxDiscBytes: number;
  /**
   * Roster packs the pages offer in a dropdown (src/pack_store.ts), uploaded on the Server disc
   * page; empty: none (players can still pick their own pack files).
   */
  packsDir: string;
  /** The largest pack zip accepted, in bytes (a pack is at most 64 MiB: docs/MODDING.md §2). */
  maxPackBytes: number;
  /** The disc uploads must be (GV4E69 rev 0; tests use a synthetic one). Not configurable from outside. */
  expectDisc: ExpectedDisc;
  /** The password from the environment (MVP_PASSWORD), or '' to use the stored one (first-run setup). */
  password: string;
  /** The state file (the setup window and the stored password hash); required when `password` is empty. */
  stateFile: string;
  /** How long first-run setup stays open after the server first started without a password. */
  setupHours: number;
  /** MVP_RESET_PASSWORD: a new value removes the stored password once and reopens setup (src/setup.ts). */
  resetPassword: string;
  /** Set the cookie's Secure flag (true behind TLS). */
  secureCookies: boolean;
  /** Take the client address from the last X-Forwarded-For entry (behind a reverse proxy). */
  trustProxy: boolean;
  /** "https://mvp.example.com": invite links and the allowed Origin. Empty: the request's Host. */
  publicUrl: string;
  sessionHours: number;
  /** stun: URLs ("{host}" as for TURN). */
  stun: string[];
  turn: TurnConfig | null;
  /** Rooms without players are removed after this long. */
  roomIdleMinutes: number;
  /** Failed logins allowed per client address per 15 minutes. */
  loginAttempts: number;
}

export const defaults: Omit<ServerConfig, 'password' | 'site'> = {
  host: '127.0.0.1',
  port: 8080,
  disc: '',
  discDir: '',
  maxDiscBytes: 1536 << 20,
  packsDir: '',
  maxPackBytes: 80 << 20,
  expectDisc: RETAIL_DISC,
  stateFile: '',
  setupHours: 48,
  resetPassword: '',
  secureCookies: false,
  trustProxy: false,
  publicUrl: '',
  sessionHours: 24 * 30,
  stun: [],
  turn: null,
  roomIdleMinutes: 30,
  loginAttempts: 10,
};

function secret(env: NodeJS.ProcessEnv, name: string): string {
  const direct = env[name];
  if (direct) return direct;
  const file = env[`${name}_FILE`];
  if (file) {
    const text = readFileSync(file, 'utf8').trim();
    // An empty secret file must not turn into "no password" (which would open first-run setup).
    if (!text) throw new Error(`${name}_FILE (${file}) is empty`);
    return text;
  }
  return '';
}

/** Creates the TURN secret file (a random secret, readable by coturn's user), atomically, when it is missing. */
function createSecretFile(file: string): void {
  if (existsSync(file)) return;
  mkdirSync(dirname(file), { recursive: true, mode: 0o755 });
  const tmp = `${file}.${randomBytes(6).toString('hex')}.tmp`;
  writeFileSync(tmp, `${randomBytes(32).toString('base64url')}\n`, { flag: 'wx', mode: 0o644 });
  try {
    chmodSync(tmp, 0o644);  // coturn runs as another user in its own container and only reads this file
  } catch {
    // no modes here
  }
  renameSync(tmp, file);
}

/** "host:port" for an ICE URL; IPv6 literals in brackets. */
function hostPort(host: string, port: number): string {
  return host.includes(':') && !host.startsWith('[') ? `[${host}]:${port}` : `${host}:${port}`;
}

/** Reads the configuration; throws with a readable message when it is unusable. */
export function loadConfig(env: NodeJS.ProcessEnv = process.env): ServerConfig {
  let file: Record<string, unknown> = {};
  if (env.MVP_SERVER_CONFIG) file = JSON.parse(readFileSync(env.MVP_SERVER_CONFIG, 'utf8')) as Record<string, unknown>;
  const pick = <T>(key: string, envName: string, parse: (s: string) => T, fallback: T): T => {
    if (env[envName] !== undefined && env[envName] !== '') return parse(env[envName]!);
    if (file[key] !== undefined) return file[key] as T;
    return fallback;
  };
  const bool = (s: string) => s === '1' || s === 'true';
  const password = secret(env, 'MVP_PASSWORD');
  if (password && password.length < 8) throw new Error('MVP_PASSWORD (or MVP_PASSWORD_FILE) must be at least 8 characters');
  const stateFile = pick('stateFile', 'MVP_STATE_FILE', String, defaults.stateFile);
  if (!password && !stateFile) {
    throw new Error('no password source: set MVP_STATE_FILE ("stateFile") so the owner can create the password in the browser, or MVP_PASSWORD');
  }
  const setupHours = pick('setupHours', 'MVP_SETUP_HOURS', Number, defaults.setupHours);
  if (!(setupHours > 0 && setupHours <= 24 * 365)) throw new Error('MVP_SETUP_HOURS ("setupHours") must be between 0 and 8760');
  const site = pick('site', 'MVP_SITE', String, '');
  if (!site) throw new Error('the site directory is not configured (MVP_SITE or "site")');
  // ICE servers: from server.json ("stun", "turn"), or, with MVP_TURN_MODE, built for coturn on this host
  // (the Docker deployment): relay = STUN and TURN, stun = STUN only, off = none (direct connections on one network).
  const turnMode = pick('turnMode', 'MVP_TURN_MODE', String, '');
  if (turnMode && !['relay', 'stun', 'off'].includes(turnMode)) throw new Error('MVP_TURN_MODE must be relay, stun or off');
  if (turnMode === 'relay' && env.MVP_TURN_SECRET_FILE && bool(env.MVP_TURN_SECRET_CREATE ?? '')) createSecretFile(env.MVP_TURN_SECRET_FILE);
  const turnFile = file.turn as Partial<TurnConfig> | undefined;
  const turnSecret = turnMode === 'stun' || turnMode === 'off' ? '' : secret(env, 'MVP_TURN_SECRET');
  let stun = (file.stun as string[] | undefined) ?? defaults.stun;
  let turn: TurnConfig | null = turnFile?.urls?.length && turnSecret
    ? { urls: turnFile.urls, secret: turnSecret, ttlSeconds: turnFile.ttlSeconds ?? 86400 }
    : null;
  if (turnMode) {
    const host = pick('turnHost', 'MVP_TURN_HOST', String, '') || '{host}';
    if (!/^[A-Za-z0-9.:{}\[\]-]{1,253}$/.test(host)) throw new Error('MVP_TURN_HOST must be a host name or an IP address');
    const port = pick('turnPort', 'MVP_TURN_PORT', Number, 3478);
    if (!Number.isInteger(port) || port < 1 || port > 65535) throw new Error('MVP_TURN_PORT must be a port number');
    const hp = hostPort(host, port);
    stun = turnMode === 'off' ? [] : [`stun:${hp}`];
    turn = null;
    if (turnMode === 'relay') {
      if (!turnSecret) throw new Error('MVP_TURN_MODE=relay needs MVP_TURN_SECRET or MVP_TURN_SECRET_FILE');
      turn = { urls: [`turn:${hp}?transport=udp`, `turn:${hp}?transport=tcp`], secret: turnSecret, ttlSeconds: 86400 };
    }
  }
  const maxDiscBytes = pick('maxDiscBytes', 'MVP_DISC_MAX_BYTES', Number, defaults.maxDiscBytes);
  if (!(Number.isSafeInteger(maxDiscBytes) && maxDiscBytes > 0)) throw new Error('MVP_DISC_MAX_BYTES must be a positive number of bytes');
  const discDir = pick('discDir', 'MVP_DISC_DIR', String, defaults.discDir);
  const packsDir = pick('packsDir', 'MVP_PACKS_DIR', String, defaults.packsDir);
  const maxPackBytes = pick('maxPackBytes', 'MVP_PACK_MAX_BYTES', Number, defaults.maxPackBytes);
  if (!(Number.isSafeInteger(maxPackBytes) && maxPackBytes > 0)) throw new Error('MVP_PACK_MAX_BYTES must be a positive number of bytes');
  return {
    host: pick('host', 'MVP_HOST', String, defaults.host),
    port: pick('port', 'MVP_PORT', Number, defaults.port),
    site: resolve(site),
    disc: pick('disc', 'MVP_DISC_IMAGE', String, defaults.disc),
    discDir: discDir ? resolve(discDir) : '',
    maxDiscBytes,
    packsDir: packsDir ? resolve(packsDir) : '',
    maxPackBytes,
    expectDisc: RETAIL_DISC,
    password,
    stateFile: stateFile ? resolve(stateFile) : '',
    setupHours,
    resetPassword: env.MVP_RESET_PASSWORD ?? '',
    secureCookies: pick('secureCookies', 'MVP_SECURE_COOKIES', bool, defaults.secureCookies),
    trustProxy: pick('trustProxy', 'MVP_TRUST_PROXY', bool, defaults.trustProxy),
    publicUrl: pick('publicUrl', 'MVP_PUBLIC_URL', String, defaults.publicUrl),
    sessionHours: pick('sessionHours', 'MVP_SESSION_HOURS', Number, defaults.sessionHours),
    stun,
    turn,
    roomIdleMinutes: pick('roomIdleMinutes', 'MVP_ROOM_IDLE_MINUTES', Number, defaults.roomIdleMinutes),
    loginAttempts: pick('loginAttempts', 'MVP_LOGIN_ATTEMPTS', Number, defaults.loginAttempts),
  };
}
