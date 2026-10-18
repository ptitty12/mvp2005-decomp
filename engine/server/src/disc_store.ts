// The server's disc image and its upload (deploy): on the owner's private
// deployment a player with access can upload the disc image from the "Server
// disc" page (src/disc_page.ts) instead of copying it to the server by hand.
//
//   <dir>/GV4E69.img              the image the disc route serves (ISO or CISO, by its first bytes)
//   <dir>/GV4E69.manifest.json    its piece hashes for the players' one-time download (src/disc_hash.ts),
//                                 kept so a restart does not hash the image again
//   <dir>/upload/upload.json      the upload in progress: id, size, name, file fingerprint, bytes received
//   <dir>/upload/<id>.part        its bytes so far
//
// - One upload at a time; starting another one discards the previous partial file.
// - Chunked and resumable: each chunk is a PUT at the offset the server already
//   has (anything else answers 409 with that offset), streamed to the partial
//   file and flushed to disk before it counts. A browser or server restart
//   resumes from the bytes received.
// - The identity check (src/disc_image.ts) runs on the partial file after each
//   chunk, so a wrong file is refused as soon as its header, FST or main.dol
//   arrives, and its bytes are deleted. Only GV4E69 revision 0 is ever stored.
// - Finishing checks the complete file again and renames it over the current
//   image: readers see the old image or the new one, never a mix.
// - The directory is the owner's only (0700); the disc never leaves it except
//   through the disc route, to browsers with access.
import { randomBytes } from 'node:crypto';
import { promises as fsp, type Stats } from 'node:fs';
import { join } from 'node:path';

import { DiscRejected, NotYet, identifyDisc, type DiscInfo, type ExpectedDisc, type Reader } from './disc_image.js';

export interface DiscStoreOptions {
  dir: string;
  expect: ExpectedDisc;
  /** The largest image accepted (bytes). */
  maxBytes: number;
  now?: () => number;
  log?: (message: string) => void;
}

/** The image the server serves. */
export interface DiscRecord {
  path: string;
  format: DiscInfo['format'];
  size: number;
  gameId: string;
  revision: number;
  dolSha1: string;
  uploadedAt: number;
  /** Changes whenever the image is replaced (the disc URL carries it, so a game never reads two images). */
  version: string;
}

interface Upload {
  id: string;
  size: number;
  name: string;
  fingerprint: string;
  received: number;
  createdAt: number;
  updatedAt: number;
  /** 'ok' once the identity check passed on the bytes received so far. */
  identity: 'pending' | 'ok';
}

/** An HTTP answer: status and JSON body. */
export interface Answer {
  status: number;
  body: Record<string, unknown>;
}

/** Largest chunk the server takes in one PUT. The page sends 8 MiB (less on a slow line). */
export const CHUNK_MAX = 16 << 20;
/** Smallest file worth checking. */
const MIN_BYTES = 64 * 1024;
/** Free space to keep beyond the upload itself. */
const SPARE_BYTES = 64 << 20;
/** An upload untouched this long is discarded. */
const STALE_MS = 7 * 24 * 3600_000;
const ID = /^[A-Za-z0-9_-]{22}$/;

async function fileReader(path: string, size: number, available: number): Promise<{ reader: Reader; close: () => Promise<void> }> {
  const fh = await fsp.open(path, 'r');
  return {
    reader: {
      size,
      async read(offset, length) {
        if (offset + length > available) throw new NotYet();
        const buf = Buffer.alloc(length);
        let done = 0;
        while (done < length) {
          const { bytesRead } = await fh.read(buf, done, length - done, offset + done);
          if (bytesRead === 0) throw new Error('short read');
          done += bytesRead;
        }
        return buf;
      },
    },
    close: () => fh.close(),
  };
}

const isAnswer = (x: unknown): x is Answer => typeof x === 'object' && x !== null && 'status' in x;
const errorAnswer = (status: number, error: string, extra: Record<string, unknown> = {}): Answer => ({ status, body: { error, ...extra } });

export class DiscStore {
  private disc: DiscRecord | null = null;
  private upload: Upload | null = null;
  private busy = false;
  private readonly now: () => number;
  private readonly log: (message: string) => void;
  private readonly uploadDir: string;
  readonly imagePath: string;
  /** Where the image's manifest is kept (src/disc_hash.ts). */
  readonly manifestPath: string;

  private constructor(private readonly o: DiscStoreOptions) {
    this.now = o.now ?? Date.now;
    this.log = o.log ?? (() => {});
    this.uploadDir = join(o.dir, 'upload');
    this.imagePath = join(o.dir, `${o.expect.gameId}.img`);
    this.manifestPath = join(o.dir, `${o.expect.gameId}.manifest.json`);
  }

  /** Opens the directory (creating it, 0700), checks the stored image and picks up an unfinished upload. */
  static async open(o: DiscStoreOptions): Promise<DiscStore> {
    const s = new DiscStore(o);
    await fsp.mkdir(s.uploadDir, { recursive: true, mode: 0o700 });
    try {
      await fsp.chmod(o.dir, 0o700);
    } catch {
      // no modes here
    }
    await s.loadImage();
    await s.loadUpload();
    return s;
  }

  private partPath(id: string): string {
    return join(this.uploadDir, `${id}.part`);
  }

  private async loadImage(): Promise<void> {
    let st: Stats;
    try {
      st = await fsp.stat(this.imagePath);
    } catch {
      this.log('disc: no image on the server yet (upload it on the Server disc page)');
      return;
    }
    try {
      const { reader, close } = await fileReader(this.imagePath, st.size, st.size);
      try {
        this.disc = this.record(await identifyDisc(reader, this.o.expect), st);
      } finally {
        await close();
      }
      this.log(`disc: ${this.disc.gameId} rev ${this.disc.revision} (${this.disc.format}, ${st.size} bytes) at ${this.imagePath}`);
    } catch (e) {
      this.log(`disc: the stored image ${this.imagePath} does not pass the identity check (${(e as Error).message}); it is not served`);
    }
  }

  private record(info: DiscInfo, st: Stats): DiscRecord {
    return {
      path: this.imagePath,
      format: info.format,
      size: st.size,
      gameId: info.gameId,
      revision: info.revision,
      dolSha1: info.dolSha1,
      uploadedAt: Math.floor(st.mtimeMs),
      version: `${st.size.toString(36)}-${Math.floor(st.mtimeMs).toString(36)}`,
    };
  }

  private async loadUpload(): Promise<void> {
    let u: Upload;
    try {
      u = JSON.parse(await fsp.readFile(join(this.uploadDir, 'upload.json'), 'utf8')) as Upload;
      if (!ID.test(u.id) || !Number.isSafeInteger(u.size) || !Number.isSafeInteger(u.received)) throw new Error('bad record');
      const st = await fsp.stat(this.partPath(u.id));
      // A chunk that was being written when the server stopped did not count: cut it off.
      u.received = Math.min(u.received, st.size);
      await fsp.truncate(this.partPath(u.id), u.received);
    } catch {
      await this.clearUploads();
      return;
    }
    this.upload = u;
    if (this.now() - u.updatedAt > STALE_MS) await this.discard('stale');
    else this.log(`disc upload: resumable at ${u.received} of ${u.size} bytes`);
  }

  private async saveUpload(): Promise<void> {
    const tmp = join(this.uploadDir, `upload.json.${randomBytes(4).toString('hex')}.tmp`);
    await fsp.writeFile(tmp, JSON.stringify(this.upload), { mode: 0o600 });
    await fsp.rename(tmp, join(this.uploadDir, 'upload.json'));
  }

  private async clearUploads(): Promise<void> {
    for (const n of await fsp.readdir(this.uploadDir).catch(() => [] as string[])) await fsp.rm(join(this.uploadDir, n), { force: true });
  }

  private async discard(why: string): Promise<void> {
    if (this.upload) this.log(`disc upload: discarded (${why}) at ${this.upload.received} of ${this.upload.size} bytes`);
    this.upload = null;
    await this.clearUploads();
  }

  /** The image to serve, or null. */
  current(): DiscRecord | null {
    return this.disc;
  }

  /** What the Server disc page shows. */
  status(): Record<string, unknown> {
    const d = this.disc;
    const u = this.upload;
    return {
      disc: d && { format: d.format, size: d.size, gameId: d.gameId, revision: d.revision, dolSha1: d.dolSha1, uploadedAt: d.uploadedAt },
      upload: u && this.publicUpload(u),
      limits: { maxBytes: this.o.maxBytes, chunkBytes: CHUNK_MAX },
      expected: { gameId: this.o.expect.gameId, revision: this.o.expect.revision },
    };
  }

  private publicUpload(u: Upload): Record<string, unknown> {
    return { id: u.id, size: u.size, name: u.name, fingerprint: u.fingerprint, received: u.received, identity: u.identity };
  }

  /** Starts an upload of `size` bytes (discarding any other unfinished one). */
  async create(input: unknown): Promise<Answer> {
    const o = (typeof input === 'object' && input !== null ? input : {}) as Record<string, unknown>;
    const size = o.size, name = o.name ?? '', fingerprint = o.fingerprint ?? '';
    if (!Number.isSafeInteger(size) || typeof name !== 'string' || typeof fingerprint !== 'string' || name.length > 255 || fingerprint.length > 512)
      return errorAnswer(400, 'Bad upload request.');
    if ((size as number) < MIN_BYTES) return errorAnswer(422, 'This file is too small to be a disc image.');
    if ((size as number) > this.o.maxBytes)
      return errorAnswer(413, `This file is larger than a GameCube disc image can be (${this.o.maxBytes} bytes at most).`);
    if (this.busy) return errorAnswer(409, 'Another upload request is being processed. Try again in a moment.');
    this.busy = true;
    try {
      await this.discard('a new upload started');
      const fs = await fsp.statfs(this.uploadDir).catch(() => null);
      if (fs && fs.bavail * fs.bsize < (size as number) + SPARE_BYTES)
        return errorAnswer(507, `The server's disk does not have room for this file (${Math.floor((fs.bavail * fs.bsize) / 1048576)} MB free).`);
      const t = this.now();
      const u: Upload = {
        id: randomBytes(16).toString('base64url'),
        size: size as number,
        name: name.replace(/[\u0000-\u001f\u007f]/g, ''),
        fingerprint,
        received: 0,
        createdAt: t,
        updatedAt: t,
        identity: 'pending',
      };
      await (await fsp.open(this.partPath(u.id), 'wx', 0o600)).close();
      this.upload = u;
      await this.saveUpload();
      this.log(`disc upload: started, ${u.size} bytes`);
      return { status: 201, body: { upload: this.publicUpload(u) } };
    } finally {
      this.busy = false;
    }
  }

  /**
   * Appends one chunk: `length` bytes from `body` at `offset`, which must be the
   * number of bytes the server has. Afterwards the identity check runs on what
   * has arrived; a file that is not the game is discarded (422).
   */
  async write(id: string, offset: number, length: number, body: AsyncIterable<Buffer>): Promise<Answer> {
    const u = this.upload;
    if (!u || u.id !== id) return errorAnswer(404, 'This upload does not exist (any more). Choose the file again.');
    if (this.busy) return errorAnswer(409, 'Another request for this upload is being processed.', { received: u.received });
    if (offset !== u.received) return errorAnswer(409, 'The server has a different number of bytes.', { received: u.received });
    if (!Number.isSafeInteger(length) || length <= 0 || length > CHUNK_MAX || offset + length > u.size)
      return errorAnswer(413, `A chunk must be 1 to ${CHUNK_MAX} bytes and stay inside the file.`, { received: u.received });
    this.busy = true;
    try {
      const fh = await fsp.open(this.partPath(id), 'r+');
      let got = 0;
      try {
        await fh.truncate(offset);
        for await (const c of body) {
          if (got + c.length > length) throw new Error('more bytes than announced');
          await fh.write(c, 0, c.length, offset + got);
          got += c.length;
        }
        if (got !== length) throw new Error('the chunk was cut short');
        await fh.sync();
      } catch (e) {
        await fh.truncate(offset).catch(() => {});
        return errorAnswer(400, `The chunk did not arrive whole (${(e as Error).message}); send it again.`, { received: u.received });
      } finally {
        await fh.close();
      }
      u.received += length;
      u.updatedAt = this.now();
      await this.saveUpload();
      if (u.identity === 'pending') {
        const r = await this.check(u, u.received);
        if (isAnswer(r)) return r;
      }
      return { status: 200, body: { received: u.received, identity: u.identity } };
    } finally {
      this.busy = false;
    }
  }

  /**
   * Runs the identity check on the bytes received so far: the disc's description,
   * 'not-yet' when more bytes are needed, or the 422 answer after discarding an
   * upload that is not the game.
   */
  private async check(u: Upload, available: number): Promise<DiscInfo | 'not-yet' | Answer> {
    const { reader, close } = await fileReader(this.partPath(u.id), u.size, available);
    let outcome: DiscInfo | 'not-yet' | string;
    try {
      outcome = await identifyDisc(reader, this.o.expect);
    } catch (e) {
      outcome = e instanceof NotYet ? 'not-yet'
              : e instanceof DiscRejected ? e.message
              : `the image could not be read (${(e as Error).message})`;
    } finally {
      await close();
    }
    if (outcome === 'not-yet') return outcome;
    if (typeof outcome === 'string') {
      await this.discard(`not the game: ${outcome}`);
      return errorAnswer(422, `This image cannot be used: the server keeps only MVP Baseball 2005 for GameCube, USA ` +
                              `(${this.o.expect.gameId}), revision ${this.o.expect.revision}. (${outcome})`);
    }
    if (u.identity !== 'ok') this.log(`disc upload: identity confirmed (${outcome.gameId} rev ${outcome.revision}) at ${available} bytes`);
    u.identity = 'ok';
    return outcome;
  }

  /** Checks the complete upload and makes it the server's image (an atomic rename over the old one). */
  async finish(id: string): Promise<Answer> {
    const u = this.upload;
    if (!u || u.id !== id) return errorAnswer(404, 'This upload does not exist (any more). Choose the file again.');
    if (this.busy) return errorAnswer(409, 'Another request for this upload is being processed.', { received: u.received });
    if (u.received !== u.size) return errorAnswer(409, 'The upload is not complete yet.', { received: u.received });
    this.busy = true;
    try {
      const info = await this.check(u, u.size);
      if (info === 'not-yet') return errorAnswer(500, 'The identity check could not read the whole file.');
      if (isAnswer(info)) return info;
      await fsp.rename(this.partPath(id), this.imagePath);
      try {
        const dfd = await fsp.open(this.o.dir, 'r');  // make the rename durable (POSIX; Windows cannot open a directory)
        await dfd.sync().finally(() => dfd.close());
      } catch {
        // not supported here
      }
      this.disc = this.record(info, await fsp.stat(this.imagePath));
      this.upload = null;
      await this.clearUploads();
      this.log(`disc: replaced by an upload (${this.disc.format}, ${this.disc.size} bytes); /config.json now names it`);
      return { status: 200, body: this.status() };
    } finally {
      this.busy = false;
    }
  }

  /** Abandons the upload in progress. */
  async cancel(id: string): Promise<Answer> {
    const u = this.upload;
    if (!u || u.id !== id) return errorAnswer(404, 'This upload does not exist (any more).');
    if (this.busy) return errorAnswer(409, 'Another request for this upload is being processed.');
    await this.discard('cancelled');
    return { status: 200, body: this.status() };
  }

  static validId(id: string): boolean {
    return ID.test(id);
  }
}
