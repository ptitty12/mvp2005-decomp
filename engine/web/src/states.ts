// Save states on the page (runtime/state_file.h): F2 saves the game where it is,
// F4 goes back to the last state saved or loaded. The engine saves and loads at
// the next frame poll, offline only (either engine: the threaded one through its render thread).
//
// The last state is kept in this browser (IndexedDB), so F4 still works after a
// reload once the same game is running again: same roster packs and gameplay
// mods. A state can also be loaded from a file (the Save states panel) or named
// in the address, `?state=NAME`: /states/NAME.mvpstate on this site (a local
// server, serve.mjs --states DIR). The page then sets up the state's gameplay
// mods and roster packs before the game starts, and the engine loads it at the
// first frame poll.
//
// A state is game-derived (it holds the disc's data in memory): it stays in this
// browser or on this machine and is never uploaded.

import type { StateResult } from './protocol.js';

export interface StateHeader {
  game?: string;
  frame_polls?: number;
  vi_fields?: number;
  mods?: string[];                // the gameplay mods' specs ("abs:miss=8,...")
  mod_settings?: string;
  packs?: { set_hash?: string; result_hash?: string; files?: string[] };
  label?: string;
  saved_by?: string;
}

const MAGIC = 'MVPSTATE';
const VERSION = 1;

/** The header of a save-state file, or an error message. */
export function readHeader(data: ArrayBuffer): StateHeader | string {
  if (data.byteLength < 20) return 'not a save state (too short)';
  const b = new Uint8Array(data);
  if (String.fromCharCode(...b.subarray(0, 8)) !== MAGIC) return 'not a save state';
  const v = new DataView(data);
  const version = v.getUint32(8, true);
  if (version !== VERSION) return `a save state of version ${version}; this page reads version ${VERSION}`;
  const n = Number(v.getBigUint64(12, true));
  if (n > 1 << 20 || 20 + n > data.byteLength) return 'a damaged save state (header)';
  try {
    const h = JSON.parse(new TextDecoder().decode(b.subarray(20, 20 + n))) as unknown;
    return h && typeof h === 'object' ? (h as StateHeader) : 'a damaged save state (header)';
  } catch {
    return 'a damaged save state (header)';
  }
}

/** What a state needs set up before boot, in words. */
export function describeNeeds(h: StateHeader): string {
  const mods = h.mods?.length ? `gameplay mods ${h.mods.join(', ')}` : 'no gameplay mods';
  const packs = h.packs?.files?.length ? `roster packs ${h.packs.files.join(', ')}` : 'no roster packs';
  return `${mods}; ${packs}`;
}

// ---- this browser's slot (IndexedDB; storage may be off: then this visit only) ----

const DB = 'mvp-states', STORE = 'slots', QUICK = 'quick';

function openDb(): Promise<IDBDatabase> {
  return new Promise((resolve, reject) => {
    const r = indexedDB.open(DB, 1);
    r.onupgradeneeded = () => r.result.createObjectStore(STORE);
    r.onsuccess = () => resolve(r.result);
    r.onerror = () => reject(r.error);
  });
}

async function putQuick(data: ArrayBuffer): Promise<void> {
  const db = await openDb();
  await new Promise<void>((resolve, reject) => {
    const tx = db.transaction(STORE, 'readwrite');
    tx.objectStore(STORE).put(data, QUICK);
    tx.oncomplete = () => resolve();
    tx.onerror = () => reject(tx.error);
  });
  db.close();
}

async function getQuick(): Promise<ArrayBuffer | null> {
  const db = await openDb();
  const v = await new Promise<unknown>((resolve, reject) => {
    const r = db.transaction(STORE, 'readonly').objectStore(STORE).get(QUICK);
    r.onsuccess = () => resolve(r.result);
    r.onerror = () => reject(r.error);
  });
  db.close();
  return v instanceof ArrayBuffer ? v : null;
}

/** The game's setup, for a save's header and for checking a load. */
export interface GameSetup {
  mods: string[];
  packs: { set_hash?: string; result_hash?: string; files: string[] } | null;  // null: none
}

/** The page's save states: the buttons, F2 / F4, the slot. */
export class States {
  private quick: ArrayBuffer | null = null;  // the last state saved or loaded (also in IndexedDB)
  private busy = false;
  private setup: GameSetup | null = null;    // the running game's, once it started

  constructor(private send: (msg: { type: 'state-save'; header: string } | { type: 'state-load'; data: ArrayBuffer },
                             transfer?: Transferable[]) => void,
              private say: (text: string, error?: boolean) => void,
              private els: { save: HTMLButtonElement | null; load: HTMLButtonElement | null;
                             file: HTMLInputElement | null; download: HTMLButtonElement | null }) {
    els.save?.addEventListener('click', () => this.save());
    els.load?.addEventListener('click', () => void this.load());
    els.download?.addEventListener('click', () => this.download());
    els.file?.addEventListener('change', () => {
      const f = els.file?.files?.[0];
      if (f) void f.arrayBuffer().then((data) => this.loadBytes(data, f.name));
      if (els.file) els.file.value = '';
    });
    this.enable(false);
  }

  /** The game started with this setup (mods and packs); saving and loading are possible from now on. */
  started(setup: GameSetup): void {
    this.setup = setup;
    this.enable(true);
  }

  /** A state the game starts from (?state=): it becomes the slot F4 returns to. */
  startFrom(data: ArrayBuffer): void {
    this.quick = data;
    void putQuick(data).catch(() => {});
  }

  /** F2: save where the game is. */
  save(): void {
    if (!this.ready()) return;
    const s = this.setup!;
    const header = {
      game: 'GV4E69-0', mods: s.mods,
      ...(s.packs ? { packs: s.packs } : {}),
      label: `saved in the browser, ${new Date().toLocaleString()}`,
    };
    this.busy = true;
    this.say('Saving the state...');
    this.send({ type: 'state-save', header: JSON.stringify(header) });
  }

  /** F4: back to the last state saved or loaded (this visit's, else the one kept in this browser). */
  async load(): Promise<void> {
    if (!this.ready()) return;
    let data = this.quick;
    if (!data) {
      try {
        data = await getQuick();
      } catch {
        data = null;
      }
    }
    if (!data) {
      this.say('No saved state yet: F2 saves one.', true);
      return;
    }
    this.loadBytes(data, 'the saved state');
  }

  /** The engine's answer to a save or a load. */
  result(r: StateResult, data?: ArrayBuffer): void {
    this.busy = false;
    if (!r.ok) {
      this.say(`Could not ${r.op} the state: ${r.error ?? 'unknown error'}.`, true);
      return;
    }
    if (r.op === 'save' && data) {
      this.quick = data;
      void putQuick(data).catch(() => {});
      this.say(`State saved (frame ${r.poll}, ${(data.byteLength / 1048576).toFixed(0)} MB). F4 comes back here.`);
    } else if (r.op === 'load') {
      this.say(`State loaded (frame ${r.poll}). F4 comes back here again; F2 saves a new one.`);
    }
  }

  private loadBytes(data: ArrayBuffer, name: string): void {
    if (!this.ready()) return;
    const h = readHeader(data);
    if (typeof h === 'string') {
      this.say(`${name}: ${h}.`, true);
      return;
    }
    const s = this.setup!;
    const want = h.packs?.result_hash ?? '', have = s.packs?.result_hash ?? '';
    if (want !== have) {
      this.say(`${name} needs ${describeNeeds(h)}; this game has other roster packs. Reload with them, ` +
               'or open the page with ?state=.', true);
      return;
    }
    if ((h.mods?.length ?? 0) !== s.mods.length) {
      this.say(`${name} needs ${describeNeeds(h)}; this game has ${s.mods.length ? s.mods.join(', ') : 'no gameplay mods'}.`, true);
      return;
    }
    this.quick = data;
    void putQuick(data).catch(() => {});
    this.busy = true;
    this.say('Loading the state...');
    const copy = data.slice(0);  // the slot keeps its own
    this.send({ type: 'state-load', data: copy }, [copy]);
  }

  /** Saves the slot to a file (a download from this page; nothing leaves the machine). */
  private download(): void {
    if (!this.quick) {
      this.say('No saved state yet: F2 saves one.', true);
      return;
    }
    const a = document.createElement('a');
    a.href = URL.createObjectURL(new Blob([this.quick], { type: 'application/octet-stream' }));
    a.download = `mvp2005-${new Date().toISOString().replace(/[:.]/g, '-').slice(0, 19)}.mvpstate`;
    a.click();
    setTimeout(() => URL.revokeObjectURL(a.href), 10000);
  }

  private ready(): boolean {
    if (!this.setup) {
      this.say('Save states work once the game is running.', true);
      return false;
    }
    if (this.busy) return false;
    return true;
  }

  private enable(on: boolean): void {
    for (const b of [this.els.save, this.els.load, this.els.download]) if (b) b.disabled = !on;
    if (this.els.file) this.els.file.disabled = !on;
  }
}

/** ?state=NAME: the file on this site (/states/NAME.mvpstate, or a same-origin path). */
export async function fetchNamedState(name: string): Promise<{ data: ArrayBuffer; header: StateHeader } | string> {
  let url: URL;
  try {
    url = new URL(/[/.]/.test(name) ? name : `/states/${encodeURIComponent(name)}.mvpstate`, location.href);
  } catch {
    return `"${name}" is not a state name`;
  }
  if (url.origin !== location.origin) return 'a state must come from this site';
  let data: ArrayBuffer;
  try {
    const r = await fetch(url.href, { credentials: 'same-origin', cache: 'no-store' });
    if (!r.ok) return `${url.pathname}: HTTP ${r.status}`;
    data = await r.arrayBuffer();
  } catch (e) {
    return `${url.pathname}: ${String(e)}`;
  }
  const header = readHeader(data);
  return typeof header === 'string' ? `${url.pathname}: ${header}` : { data, header };
}
