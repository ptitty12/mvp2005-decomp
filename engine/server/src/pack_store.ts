// Roster packs on the server (deploy): a folder of pack zips (MVP_PACKS_DIR) that the
// players' pages offer in a dropdown, so nobody has to bring their own file. The owner
// uploads and removes them on the Server disc page (src/disc_page.ts); the pages
// list them with GET /api/packs and download one with GET /api/packs/<file>, behind
// the login like the disc (src/server.ts).
//
// A pack is accepted only if it is a zip holding a pack.json (at the top, or in its
// one folder: docs/MODDING.md §2) for this game, with an id, a name and a version.
// The full check (every op, against the player's own disc) still runs in each
// player's browser, in the engine's own pack code; the server never opens the disc
// for packs. Files are stored as "<pack id>.zip", so uploading a new version of a
// pack replaces the old one.
import { createHash, randomBytes } from 'node:crypto';
import { mkdir, readFile, readdir, rename, rm, stat, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { inflateRawSync } from 'node:zlib';

import type { Answer } from './disc_store.js';

export interface PackInfo {
  /** The file name the pages download ("<id>.zip"). */
  file: string;
  id: string;
  name: string;
  version: string;
  description: string;
  size: number;
  /** SHA-256 of the zip: the ETag, and how a page tells versions apart. */
  sha256: string;
}

export interface PackStoreOptions {
  dir: string;
  /** The largest zip accepted (a pack is at most 64 MiB: docs/MODDING.md §2). */
  maxBytes: number;
  /** The game id packs must name (MODDING.md §3). */
  game?: string;
  log?: (line: string) => void;
}

const FILE_RE = /^[a-z0-9]+(?:-[a-z0-9]+)*\.zip$/;
const ID_RE = /^[a-z0-9]+(?:-[a-z0-9]+)*$/;
const SEMVER_RE = /^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$/;

/** pack.json of a zip (at the top or in its single folder), or an error message. */
export function readZipManifest(zip: Buffer): { manifest: Record<string, unknown> } | { error: string } {
  // End of central directory: the last 22..65557 bytes.
  let eocd = -1;
  for (let i = zip.length - 22; i >= Math.max(0, zip.length - 65557); i--) {
    if (zip.readUInt32LE(i) === 0x06054b50) {
      eocd = i;
      break;
    }
  }
  if (eocd < 0) return { error: 'not a zip file' };
  const count = zip.readUInt16LE(eocd + 10);
  let p = zip.readUInt32LE(eocd + 16);
  const entries: { name: string; method: number; csize: number; local: number }[] = [];
  for (let k = 0; k < count; k++) {
    if (p + 46 > zip.length || zip.readUInt32LE(p) !== 0x02014b50) return { error: 'the zip directory is damaged' };
    const method = zip.readUInt16LE(p + 10);
    const csize = zip.readUInt32LE(p + 20);
    const nlen = zip.readUInt16LE(p + 28), xlen = zip.readUInt16LE(p + 30), clen = zip.readUInt16LE(p + 32);
    const local = zip.readUInt32LE(p + 42);
    entries.push({ name: zip.subarray(p + 46, p + 46 + nlen).toString('utf8'), method, csize, local });
    p += 46 + nlen + xlen + clen;
  }
  const files = entries.filter((e) => !e.name.endsWith('/'));
  let entry = files.find((e) => e.name === 'pack.json');
  if (!entry) {
    const tops = new Set(files.map((e) => e.name.split('/')[0]));
    const only = tops.size === 1 ? [...tops][0] : null;
    entry = only ? files.find((e) => e.name === `${only}/pack.json`) : undefined;
  }
  if (!entry) return { error: 'the zip has no pack.json at its top or in its one folder' };
  const lh = entry.local;
  if (lh + 30 > zip.length || zip.readUInt32LE(lh) !== 0x04034b50) return { error: 'the zip is damaged' };
  const start = lh + 30 + zip.readUInt16LE(lh + 26) + zip.readUInt16LE(lh + 28);
  const raw = zip.subarray(start, start + entry.csize);
  let data: Buffer;
  try {
    if (entry.method === 0) data = Buffer.from(raw);
    else if (entry.method === 8) data = inflateRawSync(raw, { maxOutputLength: 1 << 20 });
    else return { error: `pack.json uses an unsupported zip compression (${entry.method})` };
  } catch {
    return { error: 'pack.json could not be read from the zip' };
  }
  try {
    const text = data.toString('utf8').replace(/^﻿/, '');
    const m = JSON.parse(text) as unknown;
    if (!m || typeof m !== 'object' || Array.isArray(m)) return { error: 'pack.json is not an object' };
    return { manifest: m as Record<string, unknown> };
  } catch {
    return { error: 'pack.json is not valid JSON' };
  }
}

/** The checks the server makes before it keeps an uploaded pack: an error message, or the info. */
export function checkPack(zip: Buffer, game: string): { info: Omit<PackInfo, 'file' | 'size' | 'sha256'> } | { error: string } {
  const r = readZipManifest(zip);
  if ('error' in r) return r;
  const m = r.manifest;
  const id = m.id, name = m.name, version = m.version;
  if (typeof id !== 'string' || id.length < 3 || id.length > 64 || !ID_RE.test(id)) {
    return { error: 'pack.json: "id" must be 3-64 characters of a-z, 0-9 and single hyphens' };
  }
  if (typeof name !== 'string' || !name.trim() || name.length > 100) return { error: 'pack.json: "name" must be 1-100 characters' };
  if (typeof version !== 'string' || !SEMVER_RE.test(version)) return { error: 'pack.json: "version" must be MAJOR.MINOR.PATCH' };
  if (m.game !== game) return { error: `pack.json: this pack is for "${String(m.game)}", not ${game}` };
  const description = typeof m.description === 'string' ? m.description.slice(0, 2000) : '';
  return { info: { id, name, version, description } };
}

export class PackStore {
  private cache = new Map<string, { mtimeMs: number; size: number; info: PackInfo }>();

  private constructor(private readonly opts: Required<PackStoreOptions>) {}

  static async open(opts: PackStoreOptions): Promise<PackStore> {
    await mkdir(opts.dir, { recursive: true, mode: 0o700 });
    return new PackStore({ game: 'GV4E69-0', log: () => {}, ...opts });
  }

  static validFile(file: string): boolean {
    return FILE_RE.test(file) && file.length <= 70;
  }

  get maxBytes(): number {
    return this.opts.maxBytes;
  }

  /** The zip's path on disk, or null when there is no such pack. */
  async path(file: string): Promise<string | null> {
    if (!PackStore.validFile(file)) return null;
    const p = join(this.opts.dir, file);
    try {
      return (await stat(p)).isFile() ? p : null;
    } catch {
      return null;
    }
  }

  /** Every pack in the folder, by name; files that are not valid packs are skipped (and logged once). */
  async list(): Promise<PackInfo[]> {
    let names: string[];
    try {
      names = await readdir(this.opts.dir);
    } catch {
      return [];
    }
    const out: PackInfo[] = [];
    for (const file of names.filter((n) => PackStore.validFile(n)).sort()) {
      const info = await this.info(file);
      if (info) out.push(info);
    }
    return out.sort((a, b) => a.name.localeCompare(b.name) || a.file.localeCompare(b.file));
  }

  async info(file: string): Promise<PackInfo | null> {
    const p = await this.path(file);
    if (!p) return null;
    const st = await stat(p);
    const hit = this.cache.get(file);
    if (hit && hit.mtimeMs === st.mtimeMs && hit.size === st.size) return hit.info;
    const zip = await readFile(p);
    const c = checkPack(zip, this.opts.game);
    if ('error' in c) {
      if (!hit) this.opts.log(`packs: skipping ${file}: ${c.error}`);
      this.cache.delete(file);
      return null;
    }
    const info: PackInfo = { file, ...c.info, size: zip.length, sha256: createHash('sha256').update(zip).digest('hex') };
    this.cache.set(file, { mtimeMs: st.mtimeMs, size: st.size, info });
    return info;
  }

  /** An uploaded zip: checked, then stored atomically as "<id>.zip" (replacing an older version). */
  async save(zip: Buffer): Promise<Answer> {
    if (zip.length > this.opts.maxBytes) {
      return { status: 413, body: { error: `A pack zip can be at most ${Math.floor(this.opts.maxBytes / 1048576)} MB.` } };
    }
    const c = checkPack(zip, this.opts.game);
    if ('error' in c) return { status: 400, body: { error: `Not a roster pack for this game: ${c.error}.` } };
    const file = `${c.info.id}.zip`;
    const dest = join(this.opts.dir, file);
    const tmp = join(this.opts.dir, `.${file}.${randomBytes(6).toString('hex')}.tmp`);
    await writeFile(tmp, zip, { mode: 0o600 });
    await rename(tmp, dest);
    this.cache.delete(file);
    const info = await this.info(file);
    this.opts.log(`packs: stored ${file} (${info?.name} ${info?.version}, ${zip.length} bytes)`);
    return { status: 200, body: { pack: info } };
  }

  async remove(file: string): Promise<Answer> {
    const p = await this.path(file);
    if (!p) return { status: 404, body: { error: 'No such pack.' } };
    await rm(p, { force: true });
    this.cache.delete(file);
    this.opts.log(`packs: removed ${file}`);
    return { status: 200, body: { removed: file } };
  }
}
