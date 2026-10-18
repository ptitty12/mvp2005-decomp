// The server's disc image kept in the player's own browser (P1).
//
// On the owner's deployment the server advertises its image in /config.json
// with the image hash, its size and a manifest of piece hashes (engine/server
// src/disc_hash.ts). The first visit downloads the whole image once into the
// origin private file system (OPFS) with a progress bar (disc-download.ts, a
// Worker): resumable, every piece checked against the manifest, the manifest
// checked against the advertised hash. After that the engine reads the disc
// from this browser's storage as it reads a picked file: every guest DVD read
// is local and synchronous, with no network wait in the game. Later visits use
// the stored copy at once and download again only when the server's image hash
// changes. The game's own identity check (GV4E69 rev 0 by the DOL's SHA-1)
// still runs on every start.
//
// The image stays in this browser (owner decision, docs/DECISIONS.md
// 2026-10-14: the players own the game); the server keeps sending no-store, so
// nothing between the server and the browser keeps a copy.
//
//   mvp-disc/<hash>.img        the image, written piece by piece at its offsets
//   mvp-disc/<hash>.part.json  the pieces written and flushed so far (resume)
//   mvp-disc/<hash>.ok         written last: the copy is complete (its size)
//
// Writing a new image removes every other entry first (one image per browser).

export const STORE_DIR = 'mvp-disc';
export const DOWNLOAD_LOCK = 'mvp-disc-download';

/** The server's image as /config.json advertises it (disc.ts). */
export interface DiscOffer {
  url: string;       // the image (same origin)
  manifest: string;  // its piece hashes (same origin)
  hash: string;      // the image hash, lowercase hex
  size: number;
}

export interface Manifest {
  size: number;
  pieceBytes: number;
  pieces: string[];
  hash: string;
}

const HEX64 = /^[0-9a-f]{64}$/;

export function toHex(buf: ArrayBuffer): string {
  return Array.from(new Uint8Array(buf), (b) => b.toString(16).padStart(2, '0')).join('');
}

function fromHex(h: string): Uint8Array {
  const out = new Uint8Array(h.length / 2);
  for (let i = 0; i < out.length; ++i) out[i] = parseInt(h.substr(2 * i, 2), 16);
  return out;
}

/** SHA-256 of `bytes`, hex (the browser's own implementation). */
export async function sha256Hex(bytes: Uint8Array): Promise<string> {
  return toHex(await crypto.subtle.digest('SHA-256', bytes as Uint8Array<ArrayBuffer>));
}

/**
 * Checks a manifest from the server against the hash and size /config.json
 * advertised: the pieces must cover the size and hash to the image hash (SHA-256
 * over their digests). Returns the manifest, or the reason it cannot be used.
 */
export async function checkManifest(m: unknown, hash: string, size: number): Promise<Manifest | string> {
  const o = (m && typeof m === 'object' ? m : {}) as Partial<Manifest>;
  if (o.size !== size) return `the manifest is for ${o.size} bytes, the server advertised ${size}`;
  if (!Number.isSafeInteger(o.pieceBytes) || (o.pieceBytes as number) < 65536) return 'bad piece size';
  if (!Array.isArray(o.pieces) || o.pieces.length !== Math.max(1, Math.ceil(size / o.pieceBytes!)))
    return 'the pieces do not cover the image';
  if (!o.pieces.every((p) => typeof p === 'string' && HEX64.test(p))) return 'bad piece hash';
  const all = new Uint8Array(32 * o.pieces.length);
  o.pieces.forEach((p, i) => all.set(fromHex(p), 32 * i));
  if ((await sha256Hex(all)) !== hash) return 'the piece hashes do not match the image hash';
  return { size, pieceBytes: o.pieceBytes!, pieces: o.pieces, hash };
}

/** Byte range of piece `i`: [start, end). */
export function pieceRange(m: Manifest, i: number): { start: number; end: number } {
  const start = i * m.pieceBytes;
  return { start, end: Math.min(m.size, start + m.pieceBytes) };
}

/** "470 of 1,380 MB, 11.8 MB/s, about 2 min left" */
export function progressText(done: number, total: number, bytesPerS: number): string {
  const mb = (n: number) => Math.round(n / 1048576).toLocaleString('en-US');
  let text = `${mb(done)} of ${mb(total)} MB`;
  if (bytesPerS > 0) {
    const left = (total - done) / bytesPerS;
    text += `, ${(bytesPerS / 1048576).toFixed(1)} MB/s, about ` +
            (left < 90 ? `${Math.max(1, Math.round(left))} s left` : `${Math.round(left / 60)} min left`);
  }
  return text;
}

async function storeDir(create: boolean): Promise<FileSystemDirectoryHandle | null> {
  try {
    if (!navigator.storage?.getDirectory) return null;
    return await (await navigator.storage.getDirectory()).getDirectoryHandle(STORE_DIR, { create });
  } catch {
    return null;
  }
}

/** Whether this browser has the origin private file system (the download Worker also needs synchronous access). */
export function canStore(): boolean {
  return typeof navigator !== 'undefined' && !!navigator.storage?.getDirectory && typeof Worker !== 'undefined';
}

/** The complete stored copy of the image with this hash: a File the engine reads locally; null if there is none. */
export async function storedDisc(hash: string, size: number): Promise<File | null> {
  const d = await storeDir(false);
  if (!d) return null;
  try {
    const ok = await (await (await d.getFileHandle(`${hash}.ok`)).getFile()).text();
    const file = await (await d.getFileHandle(`${hash}.img`)).getFile();
    return Number(ok) === size && file.size === size ? file : null;
  } catch {
    return null;
  }
}

/** Removes every stored image (the page's "remove" link, tests). */
export async function clearStoredDiscs(): Promise<void> {
  try {
    await (await navigator.storage.getDirectory()).removeEntry(STORE_DIR, { recursive: true });
  } catch {
    // nothing stored
  }
}

// ---- the download, from the page ----

/** What the download Worker reports (disc-download.ts). */
export type DownloadMsg =
  | { type: 'progress'; phase: 'waiting' | 'checking' | 'downloading'; done: number; total: number; bytesPerS: number }
  | { type: 'done'; ms: number; fetched: number; resumedFrom: number }
  | { type: 'error'; kind: 'quota' | 'access' | 'changed' | 'network' | 'corrupt' | 'storage'; message: string };

export interface DownloadResult {
  file: File;
  ms: number;          // this download's duration (0 when the copy was already complete)
  fetched: number;     // bytes fetched by this download
  resumedFrom: number; // bytes already stored when it started
}

export class DownloadError extends Error {
  constructor(readonly kind: Extract<DownloadMsg, { type: 'error' }>['kind'], message: string) {
    super(message);
  }
}

/**
 * Downloads the server's image into this browser's storage (unless a complete
 * copy is there already) and returns the stored file. `onProgress` gets every
 * progress report. `spareBytes`: free space to leave beyond the image (tests of
 * the quota error ask for more than any disk has). Rejects with a DownloadError.
 */
export async function downloadDisc(offer: DiscOffer,
                                   onProgress: (m: Extract<DownloadMsg, { type: 'progress' }>) => void,
                                   spareBytes?: number): Promise<DownloadResult> {
  // Ask the browser to keep the copy under storage pressure (Chrome decides by
  // itself; Firefox may ask the player). The download works either way.
  try {
    await navigator.storage.persist?.();
  } catch {
    // not granted
  }
  const worker = new Worker(new URL('./disc-download.js', import.meta.url), { type: 'module' });
  try {
    const result = await new Promise<Omit<DownloadResult, 'file'>>((resolve, reject) => {
      worker.onmessage = (ev: MessageEvent<DownloadMsg>) => {
        const m = ev.data;
        if (m.type === 'progress') onProgress(m);
        else if (m.type === 'done') resolve({ ms: m.ms, fetched: m.fetched, resumedFrom: m.resumedFrom });
        else reject(new DownloadError(m.kind, m.message));
      };
      worker.onerror = (ev) => reject(new DownloadError('storage', ev.message || 'the download Worker failed'));
      worker.postMessage({ type: 'start', offer, spareBytes });
    });
    const file = await storedDisc(offer.hash, offer.size);
    if (!file) throw new DownloadError('storage', 'the downloaded copy could not be opened');
    return { file, ...result };
  } finally {
    worker.terminate();
  }
}
