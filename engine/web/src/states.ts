// Save states on the page (runtime/state_file.h): F2 saves the game where it is,
// F4 goes back to the last state saved or loaded. The engine saves and loads at
// the next frame poll, offline only (either engine: the threaded one through its render thread).
//
// The last state is kept in this browser (IndexedDB), so F4 still works after a
// reload once the same game is running again: same roster packs and gameplay
// mods. A state can also be loaded from a file (the Save states panel) or named
// in the address, `?state=NAME`: /states/NAME.mvpstate on this site (the
// deployment's admin's states, or a local server: serve.mjs --states DIR). The
// page then sets up the state's gameplay mods and roster packs before the game
// starts, and the engine loads it at the first frame poll.
//
// The server's admin (owner, 2026-10-20: DECISIONS) also has save states on the
// server (engine/server/src/state_store.ts), for the admin only: F2 copies each
// save to the admin's slot there ("quick"), F4 in a browser without a save of
// its own comes back to it, "Save to the server" keeps one under a name, and the
// panel lists them. A state is game-derived (it holds the disc's data in memory):
// it leaves this browser only for that, gzipped first.

import type { ServerAdmin } from './disc.js';
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
const VERSIONS = [1, 2];  // 2: with the memory card's part (runtime/state_file.h)

/** The header of a save-state file, or an error message. */
export function readHeader(data: ArrayBuffer): StateHeader | string {
  if (data.byteLength < 20) return 'not a save state (too short)';
  const b = new Uint8Array(data);
  if (String.fromCharCode(...b.subarray(0, 8)) !== MAGIC) return 'not a save state';
  const v = new DataView(data);
  const version = v.getUint32(8, true);
  if (!VERSIONS.includes(version)) return `a save state of version ${version}; this page reads versions ${VERSIONS.join(' and ')}`;
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

/** A state on the server (GET /api/states: engine/server/src/state_store.ts). */
export interface ServerState {
  name: string;
  label: string;
  frame: number;
  mods: string[];
  packs: string[];
  size: number;
  saved: string;
}

/** The admin's slot on the server: F2 copies every save there. */
export const SERVER_SLOT = 'quick';
const NAME_RE = /^[a-z0-9]+(?:-[a-z0-9]+)*$/;

function item(text: string): HTMLLIElement {
  const li = document.createElement('li');
  const span = document.createElement('span');
  span.textContent = text;
  li.append(span);
  return li;
}

/** A name for a state on the server, from what was typed ("Braves Dodgers 9th" -> "braves-dodgers-9th"). */
export function stateName(text: string): string {
  return text.toLowerCase().replace(/\.mvpstate$/, '').replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 64)
    .replace(/-+$/, '');
}

/** The page's save states: the buttons, F2 / F4, the slot. */
export class States {
  private quick: ArrayBuffer | null = null;  // the last state saved or loaded (also in IndexedDB)
  private busy = false;
  private setup: GameSetup | null = null;    // the running game's, once it started
  private server: ServerAdmin | null = null;  // this browser is the server's admin: its states there
  private saveAs: string | null = null;       // the next save also goes to the server under this name
  private uploads: Promise<void> = Promise.resolve();  // one at a time, in order

  constructor(private send: (msg: { type: 'state-save'; header: string } | { type: 'state-load'; data: ArrayBuffer },
                             transfer?: Transferable[]) => void,
              private say: (text: string, error?: boolean) => void,
              private els: { save: HTMLButtonElement | null; load: HTMLButtonElement | null;
                             file: HTMLInputElement | null; download: HTMLButtonElement | null;
                             serverBox?: HTMLElement | null; serverList?: HTMLElement | null;
                             serverName?: HTMLInputElement | null; serverSave?: HTMLButtonElement | null }) {
    els.save?.addEventListener('click', () => this.save());
    els.load?.addEventListener('click', () => void this.load());
    els.download?.addEventListener('click', () => this.download());
    els.file?.addEventListener('change', () => {
      const f = els.file?.files?.[0];
      if (f) void f.arrayBuffer().then((data) => this.loadBytes(data, f.name));
      if (els.file) els.file.value = '';
    });
    els.serverSave?.addEventListener('click', () => this.saveToServer());
    els.serverName?.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') this.saveToServer();
    });
    this.enable(false);
  }

  /** This browser is the server's admin (/config.json): its states there, in the panel. */
  setServer(admin: ServerAdmin): void {
    if (!admin.states) return;
    this.server = admin;
    if (this.els.serverBox) this.els.serverBox.hidden = false;
    void this.refreshServer();
  }

  /** "Save to the server": a save now, kept on the server under the name typed. */
  private saveToServer(): void {
    if (!this.server) return;
    const name = stateName(this.els.serverName?.value ?? '');
    if (!NAME_RE.test(name)) {
      this.say('Give the state a name for the server: letters, digits and hyphens.', true);
      return;
    }
    if (this.els.serverName) this.els.serverName.value = name;
    if (!this.ready()) return;
    this.saveAs = name;
    this.save();
  }

  /** The admin's states on the server, in the panel. */
  private async refreshServer(): Promise<void> {
    const list = this.els.serverList;
    if (!this.server?.states || !list) return;
    let states: ServerState[];
    try {
      const r = await fetch(this.server.states, { credentials: 'same-origin', cache: 'no-store' });
      if (!r.ok) throw new Error(r.status === 401 ? 'this browser is not the admin any more' : `HTTP ${r.status}`);
      states = ((await r.json()) as { states?: ServerState[] }).states ?? [];
    } catch (e) {
      list.replaceChildren(item(`Could not list the states on the server: ${String(e).replace(/^Error: /, '')}.`));
      return;
    }
    list.replaceChildren();
    if (!states.length) list.append(item('None yet: F2 copies a save to your slot there; "Save to the server" keeps one under a name.'));
    for (const s of states) {
      const slot = s.name === SERVER_SLOT;
      const li = item(`${slot ? 'Your slot (F2)' : `${s.name}: ${s.label || 'no label'}`} (frame ${s.frame}, ` +
                      `${new Date(s.saved).toLocaleString()})`);
      const load = document.createElement('button');
      load.type = 'button';
      load.textContent = 'Load';
      load.title = 'Load it into this game (it needs the same roster packs and gameplay mods)';
      load.addEventListener('click', () => void this.loadServer(s.name));
      const start = document.createElement('a');
      start.href = `?state=${encodeURIComponent(s.name)}`;
      start.textContent = 'Start from it';
      start.title = 'Reload the page and start from it, with its roster packs and gameplay mods';
      li.append(load, start);
      list.append(li);
    }
  }

  /** A state on the server into the running game. */
  private async loadServer(name: string): Promise<void> {
    if (!this.server?.states || !this.ready()) return;
    const data = await this.fetchServer(name);
    if (typeof data === 'string') this.say(data, true);
    else this.loadBytes(data, name === SERVER_SLOT ? 'your slot on the server' : name);
  }

  /** A state's bytes from the server (served gzipped; the browser inflates it), or an error message. */
  private async fetchServer(name: string): Promise<ArrayBuffer | string> {
    try {
      const r = await fetch(`${this.server!.states}/${encodeURIComponent(name)}`, { credentials: 'same-origin', cache: 'no-store' });
      if (r.status === 404) return name === SERVER_SLOT ? 'No saved state yet: F2 saves one.' : `${name} is not on the server any more.`;
      if (!r.ok) return `Could not get ${name} from the server: HTTP ${r.status}.`;
      return await r.arrayBuffer();
    } catch (e) {
      return `Could not get ${name} from the server: ${String(e)}.`;
    }
  }

  /** Copies a save to the server under `name` (gzipped where the browser can), after any copy still under way. */
  private upload(name: string, data: ArrayBuffer, done: string): void {
    const server = this.server;
    if (!server?.states) return;
    this.uploads = this.uploads.then(async () => {
      try {
        let body: Blob = new Blob([data]);
        if (typeof CompressionStream !== 'undefined')
          body = await new Response(body.stream().pipeThrough(new CompressionStream('gzip'))).blob();
        const r = await fetch(`${server.states}/${encodeURIComponent(name)}`, {
          method: 'POST', body, credentials: 'same-origin', cache: 'no-store',
          headers: { 'X-MVP-CSRF': server.token, 'Content-Type': 'application/octet-stream' },
        });
        if (!r.ok) {
          const j = (await r.json().catch(() => ({}))) as { error?: string };
          throw new Error(j.error ?? `HTTP ${r.status}`);
        }
        this.say(done);
      } catch (e) {
        this.say(`Saved in this browser, but not on the server: ${String(e).replace(/^Error: /, '')}`, true);
      }
      await this.refreshServer();
    });
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
    if (!data && this.server) {  // the admin, in a browser without a save of its own: the slot on the server
      const got = await this.fetchServer(SERVER_SLOT);
      if (typeof got === 'string') {
        this.say(got, true);
        return;
      }
      data = got;
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
    const saveAs = this.saveAs;
    this.saveAs = null;
    if (!r.ok) {
      this.say(`Could not ${r.op} the state: ${r.error ?? 'unknown error'}.`, true);
      return;
    }
    if (r.op === 'save' && data) {
      this.quick = data;
      void putQuick(data).catch(() => {});
      const saved = `State saved (frame ${r.poll}, ${(data.byteLength / 1048576).toFixed(0)} MB). F4 comes back here.`;
      if (this.server) {
        // The admin: a copy in the slot on the server, and under the name asked for.
        this.say(`${saved} Copying it to the server...`);
        this.upload(SERVER_SLOT, data, `${saved} It is in your slot on the server too.`);
        if (saveAs && saveAs !== SERVER_SLOT) this.upload(saveAs, data, `${saved} On the server as ${saveAs}.`);
      } else {
        this.say(saved);
      }
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
    const restart = this.server ? ' "Start from it" sets them up.' : ' Reload with them, or open the page with ?state=.';
    if (want !== have) {
      this.say(`${name} needs ${describeNeeds(h)}; this game has other roster packs.${restart}`, true);
      return;
    }
    if ((h.mods?.length ?? 0) !== s.mods.length) {
      this.say(`${name} needs ${describeNeeds(h)}; this game has ${s.mods.length ? s.mods.join(', ') : 'no gameplay mods'}.` +
               restart, true);
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
    for (const b of [this.els.save, this.els.load, this.els.download, this.els.serverSave]) if (b) b.disabled = !on;
    if (this.els.file) this.els.file.disabled = !on;
    if (this.els.serverName) this.els.serverName.disabled = !on;
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
