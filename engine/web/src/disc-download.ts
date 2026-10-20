// The one-time download of the server's disc image into this browser's storage
// (P1; the format and the reasons are in disc-store.ts). A dedicated Worker,
// because only Workers get synchronous OPFS handles, which write each piece in
// place at its offset (a resumed download never rewrites what it has).
//
//   1. One download per browser at a time (a Web Lock): a second tab waits, then
//      finds the copy complete.
//   2. The manifest: checked against the hash /config.json advertised.
//   3. Room: navigator.storage.estimate() first, then the image file is sized to
//      the whole image at once, so a full disk or a small quota shows up before
//      anything is downloaded (QuotaExceededError).
//   4. The pieces: PARALLEL ranged GETs at once; each piece's SHA-256 must match
//      the manifest before it is written; the written pieces are flushed, then
//      recorded in <hash>.part.json about once a second, so a closed tab or a
//      lost connection resumes from the recorded pieces.
//   5. Complete: <hash>.ok, and the record is removed.

import { DOWNLOAD_LOCK, STORE_DIR, checkManifest, pieceRange, sha256Hex, type DiscOffer, type DownloadMsg, type Manifest }
  from './disc-store.js';

interface SyncHandle {
  read(buf: ArrayBufferView, opts?: { at?: number }): number;
  write(buf: ArrayBufferView, opts?: { at?: number }): number;
  truncate(size: number): void;
  getSize(): number;
  flush(): void;
  close(): void;
}
interface WorkerScope {
  postMessage(msg: DownloadMsg): void;
  onmessage: ((ev: MessageEvent<{ type: 'start'; offer: DiscOffer; spareBytes?: number }>) => void) | null;
}
const scope = self as unknown as WorkerScope;

const PARALLEL = 4;          // pieces in flight
const ATTEMPTS = 6;          // per piece, for network and server errors and bad pieces
const SAVE_EVERY_MS = 1000;  // flush and record the written pieces
const SPARE_BYTES = 32 << 20;  // free space to leave beyond the image

class Failure extends Error {
  constructor(readonly kind: Extract<DownloadMsg, { type: 'error' }>['kind'], message: string) {
    super(message);
  }
}

const sleep = (ms: number) => new Promise((r) => setTimeout(r, ms));
const MB = (n: number) => `${Math.round(n / 1048576).toLocaleString('en-US')} MB`;

function syncHandle(fh: FileSystemFileHandle): Promise<SyncHandle> {
  const create = (fh as unknown as { createSyncAccessHandle?: () => Promise<SyncHandle> }).createSyncAccessHandle;
  if (typeof create !== 'function') throw new Failure('storage', 'this browser cannot write large files to its storage');
  return create.call(fh);
}

function storageFailure(e: unknown, what: string): Failure {
  if (e instanceof Failure) return e;
  const name = (e as { name?: string })?.name ?? '';
  if (name === 'QuotaExceededError')
    return new Failure('quota', `the browser refused to store more data (${what})`);
  if (name === 'NoModificationAllowedError')
    return new Failure('storage', 'another tab of this browser is using the stored game data; close it and reload');
  return new Failure('storage', `${what}: ${String(e)}`);
}

async function fetchManifest(offer: DiscOffer): Promise<Manifest> {
  for (let attempt = 1; ; ++attempt) {
    let r: Response;
    try {
      r = await fetch(offer.manifest, { credentials: 'same-origin', cache: 'no-store' });
    } catch (e) {
      if (attempt >= ATTEMPTS) throw new Failure('network', `the server could not be reached (${String(e)})`);
      await sleep(500 * 2 ** attempt);
      continue;
    }
    if (r.status === 401 || r.status === 403) throw new Failure('access', 'open the player link again');
    if (r.status === 404) throw new Failure('changed', 'the disc image on the server changed');
    if (!r.ok) {
      if (attempt >= ATTEMPTS) throw new Failure('network', `the server answered HTTP ${r.status} for the manifest`);
      await sleep(500 * 2 ** attempt);
      continue;
    }
    const m = await checkManifest(await r.json(), offer.hash, offer.size);
    if (typeof m === 'string') throw new Failure('corrupt', `the server's manifest cannot be used: ${m}`);
    return m;
  }
}

/** Fetches piece `i` and checks it; retries network and server errors and bad pieces. */
async function fetchPiece(offer: DiscOffer, m: Manifest, i: number): Promise<Uint8Array> {
  const { start, end } = pieceRange(m, i);
  let last = '';
  for (let attempt = 1; attempt <= ATTEMPTS; ++attempt) {
    if (attempt > 1) await sleep(Math.min(16000, 500 * 2 ** (attempt - 2)));
    let r: Response;
    try {
      r = await fetch(offer.url, { headers: { Range: `bytes=${start}-${end - 1}` }, credentials: 'same-origin', cache: 'no-store' });
    } catch (e) {
      last = `network error (${String(e)})`;
      continue;
    }
    if (r.status === 401 || r.status === 403) throw new Failure('access', 'open the player link again');
    if (r.status === 404 || r.status === 416) throw new Failure('changed', 'the disc image on the server changed');
    if (r.status !== 206) {
      await r.body?.cancel().catch(() => undefined);
      last = r.status === 200 ? 'the server ignored the Range request' : `HTTP ${r.status}`;
      continue;
    }
    let bytes: Uint8Array;
    try {
      bytes = new Uint8Array(await r.arrayBuffer());
    } catch (e) {
      last = `the connection broke (${String(e)})`;
      continue;
    }
    if (bytes.length !== end - start) {
      last = `got ${bytes.length} bytes instead of ${end - start}`;
      continue;
    }
    if ((await sha256Hex(bytes)) !== m.pieces[i]) {
      last = 'a piece did not match its hash';
      continue;
    }
    return bytes;
  }
  throw new Failure(last.startsWith('a piece') ? 'corrupt' : 'network',
                    `piece ${i + 1} of ${m.pieces.length} could not be downloaded: ${last}`);
}

async function readRecord(dir: FileSystemDirectoryHandle, offer: DiscOffer, m: Manifest): Promise<Set<number>> {
  try {
    const rec = JSON.parse(await (await (await dir.getFileHandle(`${offer.hash}.part.json`)).getFile()).text()) as
      { hash?: string; size?: number; pieceBytes?: number; done?: number[] };
    if (rec.hash !== offer.hash || rec.size !== m.size || rec.pieceBytes !== m.pieceBytes || !Array.isArray(rec.done))
      return new Set();
    return new Set(rec.done.filter((i) => Number.isInteger(i) && i >= 0 && i < m.pieces.length));
  } catch {
    return new Set();
  }
}

async function download(offer: DiscOffer, spareBytes = SPARE_BYTES): Promise<void> {
  const t0 = performance.now();
  const total = offer.size;
  const progress = (phase: 'waiting' | 'checking' | 'downloading', done: number, bytesPerS = 0) =>
    scope.postMessage({ type: 'progress', phase, done, total, bytesPerS });
  progress('checking', 0);
  if (!navigator.storage?.getDirectory) throw new Failure('storage', 'this browser has no storage for sites');
  let dir: FileSystemDirectoryHandle;
  try {
    dir = await (await navigator.storage.getDirectory()).getDirectoryHandle(STORE_DIR, { create: true });
  } catch (e) {
    throw storageFailure(e, 'the site storage could not be opened');
  }
  // Complete already (another tab finished it while this one waited)?
  try {
    const ok = await (await (await dir.getFileHandle(`${offer.hash}.ok`)).getFile()).text();
    const img = await (await dir.getFileHandle(`${offer.hash}.img`)).getFile();
    if (Number(ok) === total && img.size === total) {
      scope.postMessage({ type: 'done', ms: 0, fetched: 0, resumedFrom: total });
      return;
    }
  } catch {
    // not complete
  }
  const m = await fetchManifest(offer);
  // One image per browser: remove any other (an older image of the server) to make room.
  for await (const name of (dir as unknown as { keys(): AsyncIterable<string> }).keys())
    if (!name.startsWith(`${offer.hash}.`)) await dir.removeEntry(name).catch(() => undefined);
  await dir.removeEntry(`${offer.hash}.ok`).catch(() => undefined);

  let done = await readRecord(dir, offer, m);
  let fh: FileSystemFileHandle, have = 0;
  try {
    fh = await dir.getFileHandle(`${offer.hash}.img`, { create: true });
    have = (await fh.getFile()).size;
  } catch (e) {
    throw storageFailure(e, 'the image file could not be created');
  }
  // Room for the rest, before asking for anything.
  try {
    const est = await navigator.storage.estimate();
    if (est.quota !== undefined && est.usage !== undefined) {
      const free = est.quota - est.usage, need = total - Math.min(have, total) + spareBytes;
      if (free < need)
        throw new Failure('quota', `the browser lets this site store ${MB(Math.max(0, free))} more, and the game data ` +
                                   `needs ${MB(need)}`);
    }
  } catch (e) {
    if (e instanceof Failure) throw e;
  }
  const h = await syncHandle(fh).catch((e: unknown) => { throw storageFailure(e, 'the image file could not be opened'); });
  try {
    try {
      if (h.getSize() !== total) {
        done = new Set();
        h.truncate(total);
        h.flush();
      }
    } catch (e) {
      throw storageFailure(e, `the browser could not reserve ${MB(total)} for the game data`);
    }
    const record = await syncHandle(await dir.getFileHandle(`${offer.hash}.part.json`, { create: true }));
    const save = () => {
      h.flush();  // the pieces first, then the record that names them
      const text = new TextEncoder().encode(JSON.stringify({ hash: offer.hash, size: total, pieceBytes: m.pieceBytes,
                                                             done: [...done].sort((a, b) => a - b) }));
      record.truncate(0);
      record.write(text, { at: 0 });
      record.flush();
    };
    try {
      const resumedFrom = [...done].reduce((n, i) => n + (pieceRange(m, i).end - pieceRange(m, i).start), 0);
      let stored = resumedFrom, fetched = 0;
      const tStart = performance.now();
      let lastSave = tStart, lastReport = 0;
      const queue = m.pieces.map((_, i) => i).filter((i) => !done.has(i));
      progress('downloading', stored);
      let failed = false;  // one lane's failure stops the others
      const lane = async () => {
        for (let i = queue.shift(); i !== undefined && !failed; i = queue.shift()) {
          let bytes: Uint8Array;
          try {
            bytes = await fetchPiece(offer, m, i);
          } catch (e) {
            failed = true;
            throw e;
          }
          if (failed) return;
          try {
            let at = 0;
            const { start } = pieceRange(m, i);
            while (at < bytes.length) at += h.write(bytes.subarray(at), { at: start + at });
          } catch (e) {
            failed = true;
            throw storageFailure(e, 'writing the game data failed');
          }
          done.add(i);
          stored += bytes.length;
          fetched += bytes.length;
          const now = performance.now();
          if (now - lastSave >= SAVE_EVERY_MS) {
            lastSave = now;
            try {
              save();
            } catch (e) {
              failed = true;
              throw storageFailure(e, 'writing the game data failed');
            }
          }
          if (now - lastReport >= 200) {
            lastReport = now;
            progress('downloading', stored, fetched / Math.max(0.001, (now - tStart) / 1000));
          }
        }
      };
      await Promise.all(Array.from({ length: PARALLEL }, lane));
      save();
      record.close();
      h.close();
      await dir.removeEntry(`${offer.hash}.part.json`).catch(() => undefined);
      const okHandle = await syncHandle(await dir.getFileHandle(`${offer.hash}.ok`, { create: true }));
      okHandle.truncate(0);
      okHandle.write(new TextEncoder().encode(String(total)), { at: 0 });
      okHandle.flush();
      okHandle.close();
      scope.postMessage({ type: 'done', ms: performance.now() - t0, fetched, resumedFrom });
    } finally {
      try {
        record.close();
      } catch {
        // closed
      }
    }
  } finally {
    try {
      h.close();
    } catch {
      // closed
    }
  }
}

scope.onmessage = (ev) => {
  if (ev.data?.type !== 'start') return;
  const offer = ev.data.offer, spare = ev.data.spareBytes;
  const run = async () => {
    // A second tab waits for the first one's download, then finds the copy complete.
    const locks = (navigator as Navigator & { locks?: LockManager }).locks;
    if (!locks) return download(offer, spare);
    let started = false;
    await locks.request(DOWNLOAD_LOCK, { ifAvailable: true }, async (lock) => {
      if (!lock) return;
      started = true;
      await download(offer, spare);
    });
    if (started) return;
    scope.postMessage({ type: 'progress', phase: 'waiting', done: 0, total: offer.size, bytesPerS: 0 });
    await locks.request(DOWNLOAD_LOCK, () => download(offer, spare));
  };
  run().catch((e: unknown) => {
    const f = e instanceof Failure ? e : new Failure('storage', String(e));
    scope.postMessage({ type: 'error', kind: f.kind, message: f.message });
  });
};
