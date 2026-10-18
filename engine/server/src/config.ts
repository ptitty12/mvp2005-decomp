// Server configuration (E5). Non-secret settings come from a JSON file
// (MVP_SERVER_CONFIG) and environment variables; secrets come only from the
// environment (or files named by it), never from the repository:
//
//   MVP_ACCESS_KEY / MVP_ACCESS_KEY_FILE    the key of the private link https://site/?key=<key>
//                                           (required; src/access.ts). MVP_ACCESS_KEY wins. With
//                                           MVP_ACCESS_KEY_CREATE=1 a missing file is created with
//                                           a random key, owner-only: the Docker deployment
//   MVP_TURN_SECRET / MVP_TURN_SECRET_FILE  coturn's static-auth-secret (optional; with
//                                           MVP_TURN_SECRET_CREATE=1 a missing file is
//                                           created with a random secret: the Docker deployment)
//
// The Docker deployment (deploy/, docs/engine/DEPLOY_DOKPLOY.md) sets everything
// through the environment: MVP_DISC_DIR (the uploaded disc), MVP_PACKS_DIR (the roster
// packs the pages offer in a dropdown), MVP_TURN_MODE (the
// ICE servers handed to the pages) and the secret files above.
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
  /** The owner's disc image (ISO or CISO), served only to browsers with access; empty: none. */
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
  /**
   * The key of the private link (MVP_ACCESS_KEY or MVP_ACCESS_KEY_FILE), at least 16 URL-safe characters:
   * a browser that opened https://site/?key=<key> has access (src/access.ts). A new key locks out every browser.
   */
  accessKey: string;
  /** Set the cookie's Secure flag (true behind TLS). */
  secureCookies: boolean;
  /** "https://mvp.example.com": invite links, the private link and the allowed Origin. Empty: the request's Host. */
  publicUrl: string;
  /** stun: URLs ("{host}" as for TURN). */
  stun: string[];
  turn: TurnConfig | null;
  /** Rooms without players are removed after this long. */
  roomIdleMinutes: number;
}

export const defaults: Omit<ServerConfig, 'accessKey' | 'site'> = {
  host: '127.0.0.1',
  port: 8080,
  disc: '',
  discDir: '',
  maxDiscBytes: 1536 << 20,
  packsDir: '',
  maxPackBytes: 80 << 20,
  expectDisc: RETAIL_DISC,
  secureCookies: false,
  publicUrl: '',
  stun: [],
  turn: null,
  roomIdleMinutes: 30,
};

function secret(env: NodeJS.ProcessEnv, name: string): string {
  const direct = env[name];
  if (direct) return direct;
  const file = env[`${name}_FILE`];
  if (file) {
    const text = readFileSync(file, 'utf8').trim();
    // An empty secret file is a mistake, not "no secret".
    if (!text) throw new Error(`${name}_FILE (${file}) is empty`);
    return text;
  }
  return '';
}

/**
 * Creates a secret file (32 random bytes, base64url) atomically when it is missing. `mode`: 0o644 for the TURN
 * secret (coturn runs as another user in its own container and only reads it), 0o600 for the access key (only
 * this server reads it).
 */
function createSecretFile(file: string, mode: number): void {
  if (existsSync(file)) return;
  mkdirSync(dirname(file), { recursive: true, mode: 0o755 });
  const tmp = `${file}.${randomBytes(6).toString('hex')}.tmp`;
  writeFileSync(tmp, `${randomBytes(32).toString('base64url')}\n`, { flag: 'wx', mode });
  try {
    chmodSync(tmp, mode);  // whatever the umask
  } catch {
    // no modes here
  }
  renameSync(tmp, file);
}

/** The key of the private link: MVP_ACCESS_KEY, else the file MVP_ACCESS_KEY_FILE names (created with _CREATE=1). */
function accessKey(env: NodeJS.ProcessEnv): string {
  const create = env.MVP_ACCESS_KEY_CREATE === '1' || env.MVP_ACCESS_KEY_CREATE === 'true';
  if (!env.MVP_ACCESS_KEY && env.MVP_ACCESS_KEY_FILE && create) createSecretFile(env.MVP_ACCESS_KEY_FILE, 0o600);
  const key = secret(env, 'MVP_ACCESS_KEY');
  if (!key) {
    throw new Error(
      'no access key: set MVP_ACCESS_KEY, or MVP_ACCESS_KEY_FILE (with MVP_ACCESS_KEY_CREATE=1 a missing file is ' +
        'created with a random key). The private link is https://<site>/?key=<key>',
    );
  }
  if (key.length < 16) throw new Error('the access key (MVP_ACCESS_KEY or MVP_ACCESS_KEY_FILE) must be at least 16 characters');
  if (!/^[A-Za-z0-9._~-]+$/.test(key)) {
    throw new Error('the access key (MVP_ACCESS_KEY or MVP_ACCESS_KEY_FILE) may hold only letters, digits and . _ ~ - (it goes into a link)');
  }
  return key;
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
  const key = accessKey(env);
  const site = pick('site', 'MVP_SITE', String, '');
  if (!site) throw new Error('the site directory is not configured (MVP_SITE or "site")');
  // ICE servers: from server.json ("stun", "turn"), or, with MVP_TURN_MODE, built for coturn on this host
  // (the Docker deployment): relay = STUN and TURN, stun = STUN only, off = none (direct connections on one network).
  const turnMode = pick('turnMode', 'MVP_TURN_MODE', String, '');
  if (turnMode && !['relay', 'stun', 'off'].includes(turnMode)) throw new Error('MVP_TURN_MODE must be relay, stun or off');
  if (turnMode === 'relay' && env.MVP_TURN_SECRET_FILE && bool(env.MVP_TURN_SECRET_CREATE ?? '')) createSecretFile(env.MVP_TURN_SECRET_FILE, 0o644);
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
    accessKey: key,
    secureCookies: pick('secureCookies', 'MVP_SECURE_COOKIES', bool, defaults.secureCookies),
    publicUrl: pick('publicUrl', 'MVP_PUBLIC_URL', String, defaults.publicUrl),
    stun,
    turn,
    roomIdleMinutes: pick('roomIdleMinutes', 'MVP_ROOM_IDLE_MINUTES', Number, defaults.roomIdleMinutes),
  };
}
