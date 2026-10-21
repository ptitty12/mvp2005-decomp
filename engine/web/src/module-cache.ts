// The browser's cache of the translated module (E2c), in the origin private
// file system (OPFS), used by the engine Worker.
//
// Entry: <key>.bin, the engine's serialized module (web/wasm/module_cache.h),
// and <key>.ok, written only after the .bin is complete (a write cut short by a
// closed tab leaves no .ok, so it is never read). The key is the DOL's SHA-1
// plus the engine build's identity (build-id.json: the hashes of mvp-web.wasm
// and symbols.txt), so another disc revision, engine build or symbol map never
// reads a stale entry. Writing an entry removes every other one (a new engine
// build replaces the old translation). Any failure means "no cache": the Worker
// translates as on a first visit.
//
// The module is derived from the player's disc: it stays in this browser's
// storage (ARCHITECTURE.md §3) and is never sent anywhere.

const DIR = 'mvp-translated';

interface SyncHandle {
  read(buf: ArrayBufferView, opts?: { at?: number }): number;
  write(buf: ArrayBufferView, opts?: { at?: number }): number;
  truncate(size: number): void;
  flush(): void;
  close(): void;
}

// `mt`: the threaded engine (P2; its translation imports a shared memory), keyed
// by mvp-web-mt.wasm's hash.
export async function cacheKey(dolSha1: string, buildIdUrl: URL, mt = false): Promise<string | null> {
  try {
    const res = await fetch(buildIdUrl, { cache: 'no-store' });
    if (!res.ok) return null;
    const id = (await res.json()) as { engine?: string; engine_mt?: string; symbols?: string };
    const engine = mt ? id.engine_mt : id.engine;
    if (!engine || !id.symbols || !/^[0-9a-f]{40}$/.test(dolSha1)) return null;
    return `${dolSha1}-${mt ? 'mt' : ''}${engine.slice(0, 16)}-${id.symbols.slice(0, 16)}`;
  } catch {
    return null;
  }
}

async function dir(): Promise<FileSystemDirectoryHandle | null> {
  try {
    if (!navigator.storage?.getDirectory) return null;
    const root = await navigator.storage.getDirectory();
    return await root.getDirectoryHandle(DIR, { create: true });
  } catch {
    return null;
  }
}

// Reads the cached blob into the buffer `alloc(size)` returns (the engine's
// heap: no copy in between where the browser allows synchronous access).
// Returns the size, or 0 when there is no complete entry.
export async function readCached(key: string, alloc: (size: number) => Uint8Array): Promise<number> {
  const d = await dir();
  if (!d) return 0;
  try {
    const ok = await (await (await d.getFileHandle(`${key}.ok`)).getFile()).text();
    const fh = await d.getFileHandle(`${key}.bin`);
    const file = await fh.getFile();
    if (Number(ok) !== file.size || file.size === 0) return 0;
    const sync = (fh as unknown as { createSyncAccessHandle?: () => Promise<SyncHandle> }).createSyncAccessHandle;
    if (typeof sync === 'function') {
      const h = await sync.call(fh);
      try {
        const heap = alloc(file.size);
        // The threaded engine's heap is shared (P2): read into a plain buffer, then copy.
        const shared = typeof SharedArrayBuffer !== 'undefined' && heap.buffer instanceof SharedArrayBuffer;
        const dst = shared ? new Uint8Array(file.size) : heap;
        let at = 0;
        while (at < file.size) {
          const n = h.read(dst.subarray(at), { at });
          if (n <= 0) return 0;
          at += n;
        }
        if (shared) heap.set(dst);
      } finally {
        h.close();
      }
    } else {
      const bytes = new Uint8Array(await file.arrayBuffer());
      alloc(file.size).set(bytes);
    }
    return file.size;
  } catch {
    return 0;
  }
}

// V4: the file is the parts one after the other (the module's header and its bytes,
// without joining them into one more copy), written 4 MiB at a time with a task
// boundary between pieces: the synchronous writes run on the engine's thread, and
// the game keeps running between them instead of stopping for the whole module.
const kWritePiece = 4 << 20;
const nextTask = (): Promise<void> => new Promise((resolve) => setTimeout(resolve, 0));
async function writeFile(d: FileSystemDirectoryHandle, name: string, parts: Uint8Array[]): Promise<void> {
  const fh = await d.getFileHandle(name, { create: true });
  const sync = (fh as unknown as { createSyncAccessHandle?: () => Promise<SyncHandle> }).createSyncAccessHandle;
  if (typeof sync === 'function') {  // Workers: fast and everywhere OPFS is
    const h = await sync.call(fh);
    try {
      h.truncate(0);
      let at = 0;
      for (const bytes of parts) {
        let k = 0;
        while (k < bytes.length) {
          const wrote = h.write(bytes.subarray(k, Math.min(bytes.length, k + kWritePiece)), { at });
          k += wrote;
          at += wrote;
          if (k < bytes.length) await nextTask();
        }
      }
      h.flush();
    } finally {
      h.close();
    }
    return;
  }
  const w = await fh.createWritable();
  for (const bytes of parts) await w.write(bytes as Uint8Array<ArrayBuffer>);
  await w.close();
}

// Stores the blob under `key` and removes every other entry. Returns the
// milliseconds it took, or an error message.
export async function writeCached(key: string, bytes: Uint8Array | Uint8Array[]): Promise<{ ms: number } | { error: string }> {
  const parts = Array.isArray(bytes) ? bytes : [bytes];
  const total = parts.reduce((s, p) => s + p.length, 0);
  const t0 = performance.now();
  const d = await dir();
  if (!d) return { error: 'no origin private file system in this browser' };
  try {
    for await (const name of (d as unknown as { keys(): AsyncIterable<string> }).keys())
      if (!name.startsWith(`${key}.`)) await d.removeEntry(name).catch(() => undefined);
    await d.removeEntry(`${key}.ok`).catch(() => undefined);
    await writeFile(d, `${key}.bin`, parts);
    await writeFile(d, `${key}.ok`, [new TextEncoder().encode(String(total))]);
    return { ms: performance.now() - t0 };
  } catch (e) {
    return { error: String(e) };
  }
}

// Removes the whole cache (a damaged entry, or the page's "clear" button).
export async function clearCache(): Promise<void> {
  try {
    const root = await navigator.storage.getDirectory();
    await root.removeEntry(DIR, { recursive: true });
  } catch {
    // nothing cached
  }
}
