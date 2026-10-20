// The admin's memory card on the server (owner, 2026-10-20: DECISIONS): the
// game's memory card in slot A (engine/runtime/memcard.h), the 8 MiB the game
// writes its profiles and seasons to. The game page keeps every player's card
// in their browser; the admin's is kept here too, so the admin's saves follow
// them to any browser. Only the admin reads or writes it (src/server.ts): its
// files are the game's own, so it is game-derived and never public.
//
// It lives in the states directory as "memory-card-a.raw.gz" (a name the save
// states' list skips), gzipped: an erased card is mostly 0xFF and compresses
// to a few KiB. Its version is the SHA-256 of that file (the ETag). A write says
// which version it replaces (If-Match), or that there is none yet
// (If-None-Match: *): a page open since before another browser saved to the
// card is told so (412) instead of putting the older card back.
import { createHash, randomBytes } from 'node:crypto';
import { readFile, rename, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { promisify } from 'node:util';
import { gunzip, gzip } from 'node:zlib';

import type { Answer } from './disc_store.js';

const gzipAsync = promisify(gzip);
const gunzipAsync = promisify(gunzip);

/** A memory card's size (MemoryCard::kSize: 64 Mbit). */
export const CARD_BYTES = 8 << 20;
const FILE = 'memory-card-a.raw.gz';

export class CardStore {
  private queue: Promise<unknown> = Promise.resolve();

  constructor(private readonly dir: string, private readonly log: (line: string) => void = () => {}) {}

  /** The largest body a write may send: a card, gzipped or not (gzip adds a little to data that does not compress). */
  static readonly maxBody = CARD_BYTES + (1 << 20);

  /** The card as stored (gzipped) and its ETag, or null when the server has none yet. */
  async read(): Promise<{ data: Buffer; etag: string } | null> {
    try {
      const data = await readFile(join(this.dir, FILE));
      return { data, etag: CardStore.etag(data) };
    } catch {
      return null;
    }
  }

  static etag(stored: Buffer): string {
    return `"${createHash('sha256').update(stored).digest('hex')}"`;
  }

  /**
   * Stores a card (gzipped or not; CARD_BYTES once inflated) if the server's is still the version
   * `ifMatch` names, or `ifNoneMatch` is "*" and it has none. One write at a time.
   */
  write(body: Buffer, ifMatch: string | undefined, ifNoneMatch: string | undefined): Promise<Answer> {
    const run = this.queue.then(() => this.writeNow(body, ifMatch, ifNoneMatch));
    this.queue = run.catch(() => {});
    return run;
  }

  private async writeNow(body: Buffer, ifMatch: string | undefined, ifNoneMatch: string | undefined): Promise<Answer> {
    if (ifMatch === undefined && ifNoneMatch !== '*') {
      return { status: 428, body: { error: 'A memory card write must say which card it replaces (If-Match or If-None-Match: *).' } };
    }
    const notCard = { status: 400, body: { error: `Not a memory card: a card is ${CARD_BYTES} bytes.` } };
    let raw: Buffer;
    if (body.length >= 2 && body[0] === 0x1f && body[1] === 0x8b) {
      try {
        raw = await gunzipAsync(body, { maxOutputLength: CARD_BYTES });
      } catch {
        return notCard;  // damaged, or more than a card
      }
    } else {
      raw = body;
    }
    if (raw.length !== CARD_BYTES) return notCard;
    const now = await this.read();
    const current = now?.etag ?? null;
    if (ifMatch !== undefined ? ifMatch !== current : current !== null) {
      return { status: 412, body: { error: 'The memory card on the server changed in another browser since this game started.',
                                    etag: current } };
    }
    const data = await gzipAsync(raw, { level: 6 });
    const tmp = join(this.dir, `.${FILE}.${randomBytes(6).toString('hex')}.tmp`);
    await writeFile(tmp, data, { mode: 0o600 });
    await rename(tmp, join(this.dir, FILE));
    this.log(`card: stored ${FILE} (${data.length} bytes gzipped)`);
    return { status: 200, body: { etag: CardStore.etag(data) } };
  }
}
