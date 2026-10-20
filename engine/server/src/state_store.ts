// The admin's save states on the server (owner, 2026-10-20: DECISIONS): a folder
// (MVP_STATES_DIR) of save-state files (engine/runtime/state_file.h), saved from the
// game page (F2 copies the state to the admin's slot, "quick"; "Save to the server"
// keeps one under a name) or uploaded on the Server disc page. Only the admin sees
// them (src/server.ts): a state holds the game's memory, so it is game-derived and
// never public.
//
// A state is about 50 MB and compresses to about a third, so each is kept gzipped
// as "<name>.mvpstate.gz" and served as it is, with Content-Encoding: gzip: the
// browser inflates it, and the page gets the file's own bytes. Uploads may come
// gzipped (the game page compresses before it sends) or not (the server does it).
// A file is accepted only if it is a save state: the magic, the version this
// engine writes, and its JSON header (the label, the gameplay mods and roster
// packs it needs, the frame it was saved at) are checked; the rest is the
// engine's to check when it loads it.
import { randomBytes } from 'node:crypto';
import { createReadStream } from 'node:fs';
import { mkdir, readdir, rename, rm, stat, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { promisify } from 'node:util';
import { createGunzip, gunzip, gzip } from 'node:zlib';

import type { Answer } from './disc_store.js';

const gzipAsync = promisify(gzip);
const gunzipAsync = promisify(gunzip);

export interface StateInfo {
  /** The state's name: "/states/<name>.mvpstate" and ?state=<name> on the game page. */
  name: string;
  label: string;
  /** The frame poll it was saved at. */
  frame: number;
  /** The gameplay mods' specs and the roster packs' files it needs (the page sets them up first). */
  mods: string[];
  packs: string[];
  /** Bytes on the server (gzipped). */
  size: number;
  /** When it was stored (ISO 8601). */
  saved: string;
}

export interface StateStoreOptions {
  dir: string;
  /** The largest state accepted, before compression. */
  maxBytes: number;
  log?: (line: string) => void;
}

const NAME_RE = /^[a-z0-9]+(?:-[a-z0-9]+)*$/;
const SUFFIX = '.mvpstate.gz';
const MAGIC = 'MVPSTATE';
const VERSION = 1;
const HEADER_MAX = 1 << 20;

interface Header {
  label?: unknown;
  frame_polls?: unknown;
  mods?: unknown;
  packs?: { files?: unknown };
}

/** The JSON header of a save state's first bytes (at least 20 + its length), or an error message. */
export function readStateHeader(b: Buffer): Header | string {
  if (b.length < 20 || b.toString('latin1', 0, 8) !== MAGIC) return 'not a save state';
  const version = b.readUInt32LE(8);
  if (version !== VERSION) return `a save state of version ${version}; this server takes version ${VERSION}`;
  const n = Number(b.readBigUInt64LE(12));
  if (n > HEADER_MAX) return 'a damaged save state (header)';
  if (b.length < 20 + n) return 'a damaged save state (cut short)';
  try {
    const h = JSON.parse(b.toString('utf8', 20, 20 + n)) as unknown;
    return h && typeof h === 'object' && !Array.isArray(h) ? (h as Header) : 'a damaged save state (header)';
  } catch {
    return 'a damaged save state (header)';
  }
}

const strings = (v: unknown): string[] => (Array.isArray(v) ? v.filter((x): x is string => typeof x === 'string') : []);

/** The first bytes of a gzipped file, inflated: enough for the state's header. */
function inflatedPrefix(path: string, want: number): Promise<Buffer> {
  return new Promise((resolve, reject) => {
    const chunks: Buffer[] = [];
    let have = 0;
    const src = createReadStream(path);
    const z = createGunzip();
    const done = () => {
      src.destroy();
      z.destroy();
      resolve(Buffer.concat(chunks));
    };
    z.on('data', (c: Buffer) => {
      chunks.push(c);
      have += c.length;
      if (have >= 20) {
        const b = Buffer.concat(chunks);
        const n = b.toString('latin1', 0, 8) === MAGIC ? Number(b.readBigUInt64LE(12)) : 0;
        if (n > HEADER_MAX || have >= 20 + n || have >= want) done();
      }
    });
    z.on('end', done);
    z.on('error', reject);
    src.on('error', reject);
    src.pipe(z);
  });
}

export class StateStore {
  private cache = new Map<string, { mtimeMs: number; size: number; info: StateInfo }>();

  private constructor(private readonly opts: Required<StateStoreOptions>) {}

  static async open(opts: StateStoreOptions): Promise<StateStore> {
    await mkdir(opts.dir, { recursive: true, mode: 0o700 });
    return new StateStore({ log: () => {}, ...opts });
  }

  static validName(name: string): boolean {
    return NAME_RE.test(name) && name.length <= 64;
  }

  get maxBytes(): number {
    return this.opts.maxBytes;
  }

  /** The gzipped file on disk, or null when there is no such state. */
  async path(name: string): Promise<string | null> {
    if (!StateStore.validName(name)) return null;
    const p = join(this.opts.dir, name + SUFFIX);
    try {
      return (await stat(p)).isFile() ? p : null;
    } catch {
      return null;
    }
  }

  /** Every state, the most recent first; files that are not states are skipped (and logged once). */
  async list(): Promise<StateInfo[]> {
    let files: string[];
    try {
      files = await readdir(this.opts.dir);
    } catch {
      return [];
    }
    const out: StateInfo[] = [];
    for (const f of files) {
      if (!f.endsWith(SUFFIX)) continue;
      const info = await this.info(f.slice(0, -SUFFIX.length));
      if (info) out.push(info);
    }
    return out.sort((a, b) => b.saved.localeCompare(a.saved) || a.name.localeCompare(b.name));
  }

  async info(name: string): Promise<StateInfo | null> {
    const p = await this.path(name);
    if (!p) return null;
    const st = await stat(p);
    const hit = this.cache.get(name);
    if (hit && hit.mtimeMs === st.mtimeMs && hit.size === st.size) return hit.info;
    let h: Header | string;
    try {
      h = readStateHeader(await inflatedPrefix(p, 20 + HEADER_MAX));
    } catch {
      h = 'not gzipped';
    }
    if (typeof h === 'string') {
      if (!hit) this.opts.log(`states: skipping ${name}${SUFFIX}: ${h}`);
      this.cache.delete(name);
      return null;
    }
    const info: StateInfo = {
      name,
      label: typeof h.label === 'string' ? h.label.slice(0, 200) : '',
      frame: typeof h.frame_polls === 'number' ? h.frame_polls : 0,
      mods: strings(h.mods),
      packs: strings(h.packs?.files),
      size: st.size,
      saved: new Date(st.mtimeMs).toISOString(),
    };
    this.cache.set(name, { mtimeMs: st.mtimeMs, size: st.size, info });
    return info;
  }

  /** A state (gzipped or not): checked, then stored gzipped as "<name>.mvpstate.gz", replacing one of that name. */
  async save(name: string, body: Buffer): Promise<Answer> {
    const tooLarge = { status: 413, body: { error: `A save state can be at most ${Math.floor(this.opts.maxBytes / 1048576)} MB.` } };
    if (!StateStore.validName(name)) {
      return { status: 400, body: { error: 'A state name is 1-64 lowercase letters, digits and single hyphens.' } };
    }
    const gzipped = body.length >= 2 && body[0] === 0x1f && body[1] === 0x8b;
    let raw: Buffer;
    if (gzipped) {
      try {
        raw = await gunzipAsync(body, { maxOutputLength: this.opts.maxBytes });
      } catch (e) {
        return (e as { code?: string }).code === 'ERR_BUFFER_TOO_LARGE' || e instanceof RangeError
          ? tooLarge
          : { status: 400, body: { error: 'Not a save state: the gzip data is damaged.' } };
      }
    } else {
      raw = body;
      if (raw.length > this.opts.maxBytes) return tooLarge;
    }
    const h = readStateHeader(raw);
    if (typeof h === 'string') return { status: 400, body: { error: `Not a save state: ${h}.` } };
    const data = gzipped ? body : await gzipAsync(raw, { level: 6 });
    const file = name + SUFFIX;
    const tmp = join(this.opts.dir, `.${file}.${randomBytes(6).toString('hex')}.tmp`);
    await writeFile(tmp, data, { mode: 0o600 });
    await rename(tmp, join(this.opts.dir, file));
    this.cache.delete(name);
    const info = await this.info(name);
    this.opts.log(`states: stored ${file} (${raw.length} bytes, ${data.length} gzipped)`);
    return { status: 200, body: { state: info } };
  }

  async remove(name: string): Promise<Answer> {
    const p = await this.path(name);
    if (!p) return { status: 404, body: { error: 'No such state.' } };
    await rm(p, { force: true });
    this.cache.delete(name);
    this.opts.log(`states: removed ${name}${SUFFIX}`);
    return { status: 200, body: { removed: name } };
  }
}
