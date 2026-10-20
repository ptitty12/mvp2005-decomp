// The memory card in slot A (engine/runtime/memcard.h) on the page: the 8 MiB
// the game keeps its profiles, seasons and settings on, saved from the game's
// own menus. Each browser keeps its card (IndexedDB) and gives it to the engine
// before boot; a browser without one gets a new card, which the engine formats
// as the game would. When the game writes to it and the writes settle, the
// Worker sends the card back ('card') and this keeps it.
//
// The server's admin (owner, 2026-10-20: DECISIONS) has the card on the server
// too (engine/server/src/card_store.ts), so the admin's saves follow them to any
// browser: a game starts from the server's card (or this browser's, when it has
// saves the server has not got yet), and each settled card is sent there. A
// write names the server's version it replaces; when another browser has saved
// since, the server refuses it and this browser keeps its card to itself.
//
// No card goes in for an online game (both machines must be the same) or a test
// run (an input script, or a stop field or frame: the game's boot would change);
// `?card=0` takes it out, `?card=1` puts it in a test run. The card is
// game-derived (the game's own files): it leaves this browser only for the
// admin's server, and as a download on request.

import type { ServerAdmin } from './disc.js';

/** A card's size (MemoryCard::kSize: 64 Mbit). */
export const CARD_BYTES = 8 << 20;

// ---- this browser's card (IndexedDB; storage may be off: then this visit only) ----

const DB = 'mvp-card', STORE = 'cards', SLOT_A = 'slot-a';

/** The card as kept: `base` is the server's version it came from (the admin), `dirty` that the server has not got it. */
interface Kept {
  data: ArrayBuffer;
  base: string | null;
  dirty: boolean;
}

function openDb(): Promise<IDBDatabase> {
  return new Promise((resolve, reject) => {
    const r = indexedDB.open(DB, 1);
    r.onupgradeneeded = () => r.result.createObjectStore(STORE);
    r.onsuccess = () => resolve(r.result);
    r.onerror = () => reject(r.error);
  });
}

async function putKept(k: Kept): Promise<void> {
  const db = await openDb();
  await new Promise<void>((resolve, reject) => {
    const tx = db.transaction(STORE, 'readwrite');
    tx.objectStore(STORE).put(k, SLOT_A);
    tx.oncomplete = () => resolve();
    tx.onerror = () => reject(tx.error);
  });
  db.close();
}

async function getKept(): Promise<Kept | null> {
  const db = await openDb();
  const v = await new Promise<unknown>((resolve, reject) => {
    const r = db.transaction(STORE, 'readonly').objectStore(STORE).get(SLOT_A);
    r.onsuccess = () => resolve(r.result);
    r.onerror = () => reject(r.error);
  });
  db.close();
  const k = v as Partial<Kept> | undefined;
  return k?.data instanceof ArrayBuffer && k.data.byteLength === CARD_BYTES
    ? { data: k.data, base: typeof k.base === 'string' ? k.base : null, dirty: !!k.dirty }
    : null;
}

/** The page's memory card: what the next game starts with, and keeping what the game writes. */
export class MemoryCards {
  private kept: Kept | null = null;
  private server: string | null = null;  // the admin's card on the server (/api/card)
  private token = '';
  private loaded: Promise<void> | null = null;
  private done = false;
  private conflict = false;  // the server's card changed elsewhere: this game's card stays here
  private uploading = false;
  private pending = false;   // a newer card to send once the upload under way ends
  private inGame = false;

  constructor(private say: (text: string, error?: boolean) => void,
              private els: { download: HTMLButtonElement | null; file: HTMLInputElement | null; summary: HTMLElement | null }) {
    els.download?.addEventListener('click', () => this.download());
    els.file?.addEventListener('change', () => {
      const f = els.file?.files?.[0];
      if (f) void f.arrayBuffer().then((data) => this.useFile(data, f.name));
      if (els.file) els.file.value = '';
    });
  }

  /** This browser is the server's admin: its card is on the server too. Before load(). */
  setServer(admin: ServerAdmin): void {
    if (!admin.card) return;
    this.server = admin.card;
    this.token = admin.token;
  }

  /** Reads this browser's card and, for the admin, the server's; once. */
  load(): Promise<void> {
    return (this.loaded ??= this.loadNow().finally(() => {
      this.done = true;
      this.show();
    }));
  }

  ready(): boolean {
    return this.done;
  }

  /** The card the next game starts with: its bytes (a copy), or 'new' for a new card. */
  forGame(): ArrayBuffer | 'new' {
    this.inGame = true;
    this.show();
    return this.kept ? this.kept.data.slice(0) : 'new';
  }

  /** The game wrote to the card and the writes settled (the Worker's 'card'). */
  wrote(data: ArrayBuffer): void {
    if (data.byteLength !== CARD_BYTES) return;
    this.kept = { data, base: this.kept?.base ?? null, dirty: !!this.server };
    this.keep();
    this.show();
    if (this.server && !this.conflict) this.upload();
    else this.say('Saved on the memory card in this browser.');
  }

  private async loadNow(): Promise<void> {
    try {
      this.kept = await getKept();
    } catch {
      this.kept = null;  // no storage: a new card, for this visit
    }
    if (!this.server) return;
    let r: Response;
    try {
      r = await fetch(this.server, { credentials: 'same-origin', cache: 'no-store' });
    } catch (e) {
      this.say(`The memory card on the server could not be read (${String(e)}); this browser's card goes in.`, true);
      return;
    }
    if (r.status === 404) {  // the server has none yet: this browser's becomes it
      if (this.kept) {
        this.kept = { ...this.kept, base: null, dirty: true };
        this.upload();
      }
      return;
    }
    if (!r.ok) {
      this.say(`The memory card on the server could not be read (HTTP ${r.status}); this browser's card goes in.`, true);
      return;
    }
    const etag = r.headers.get('etag');
    const data = await r.arrayBuffer();
    if (data.byteLength !== CARD_BYTES || !etag) {
      this.say('The memory card on the server is damaged; this browser\'s card goes in.', true);
      return;
    }
    // This browser's own saves the server has not got yet, made on the server's card as it still is: they go in.
    if (this.kept?.dirty && this.kept.base === etag) {
      this.upload();
      return;
    }
    if (this.kept?.dirty) this.say('This browser had memory card saves the server did not take; the server\'s card goes in.', true);
    this.kept = { data, base: etag, dirty: false };
    this.keep();
  }

  private keep(): void {
    const k = this.kept;
    if (k) void putKept(k).catch(() => {});
  }

  /** Sends the card to the server, after any upload still under way (only the latest goes). */
  private upload(): void {
    if (this.uploading) {
      this.pending = true;
      return;
    }
    const k = this.kept, url = this.server;
    if (!k || !url) return;
    this.uploading = true;
    void (async () => {
      try {
        let body: Blob = new Blob([k.data]);
        if (typeof CompressionStream !== 'undefined')
          body = await new Response(body.stream().pipeThrough(new CompressionStream('gzip'))).blob();
        const headers: Record<string, string> = { 'X-MVP-CSRF': this.token, 'Content-Type': 'application/octet-stream' };
        if (k.base) headers['If-Match'] = k.base;
        else headers['If-None-Match'] = '*';
        const r = await fetch(url, { method: 'PUT', body, credentials: 'same-origin', cache: 'no-store', headers });
        const j = (await r.json().catch(() => ({}))) as { etag?: string; error?: string };
        if (r.status === 412) {
          this.conflict = true;
          this.say('The memory card on the server changed in another browser since this game started, so this ' +
                   'game\'s saves stay in this browser. Reload to play on the server\'s card.', true);
        } else if (!r.ok || !j.etag) {
          throw new Error(j.error ?? `HTTP ${r.status}`);
        } else if (this.kept === k) {
          this.kept = { data: k.data, base: j.etag, dirty: false };
          this.keep();
          this.say('Saved on the memory card, in this browser and on the server.');
        } else if (this.kept) {
          this.kept = { ...this.kept, base: j.etag };  // a newer card came meanwhile: it goes next, over this one
          this.keep();
        }
      } catch (e) {
        this.say(`Saved on the memory card in this browser, but not on the server: ${String(e).replace(/^Error: /, '')}`, true);
      } finally {
        this.uploading = false;
        if (this.pending) {
          this.pending = false;
          if (!this.conflict) this.upload();
        }
      }
    })();
  }

  private download(): void {
    if (!this.kept) return;
    const a = document.createElement('a');
    a.href = URL.createObjectURL(new Blob([this.kept.data], { type: 'application/octet-stream' }));
    a.download = 'mvp2005-memory-card-a.raw';
    a.click();
    setTimeout(() => URL.revokeObjectURL(a.href), 10_000);
  }

  /** A card file (a download from here) for the next game: before it starts. */
  private useFile(data: ArrayBuffer, name: string): void {
    if (this.inGame) {
      this.say('A card file goes in before the game starts (reload the page first).', true);
      return;
    }
    if (data.byteLength !== CARD_BYTES) {
      this.say(`${name} is not a memory card of this size (${CARD_BYTES / 1048576} MiB).`, true);
      return;
    }
    this.kept = { data, base: this.kept?.base ?? null, dirty: !!this.server };
    this.keep();
    this.say(`${name} goes in slot A for the next game` + (this.server ? ', and on the server.' : '.'));
    if (this.server) this.upload();
    this.show();
  }

  private show(): void {
    const { download, file, summary } = this.els;
    if (download) download.disabled = !this.kept;
    if (file) file.disabled = !this.done || this.inGame;
    if (summary) summary.textContent = !this.done ? '' : this.kept ? (this.server ? 'this browser and the server' : 'in this browser') : 'new';
  }
}
