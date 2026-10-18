// The Roster packs panel (E6): the player picks pack folders or .zip files, puts
// them in order and sees what the engine thinks of them: each pack's id, version
// and hash, the set hash, and every problem (docs/MODDING.md §7). The checks run
// in the engine's own pack code (mvp-packs.js, the same C++ as the engine), with
// no disc; the full check runs in the engine Worker when the game starts, against
// the player's disc, and its result comes back through showMounted().
//
// Both players of an online game must pick the same packs in the same order: the
// set hash goes into the netplay handshake, which refuses a game otherwise.
// The packs never leave the browser.
//
// On the owner's deployment the server also offers packs (/config.json "packs",
// engine/server/src/pack_store.ts): a dropdown lists them, and choosing one
// downloads its zip (behind the private link) and adds it as if the player had picked
// the file. The page remembers the server packs a player chose (this browser
// only) and adds them again on the next visit, if the server still has them.

import { EMPTY_SET_HASH, feedPacks, readPick, type PacksModule } from './packs-core.js';
import type { PackInput, PackSetReport } from './protocol.js';

export interface PacksState {
  count: number;
  setHash: string;
  report?: PackSetReport;   // the static check
  mounted?: PackSetReport;  // the engine's, after applying the set to the disc
  locked: boolean;
}

export class PackPanel {
  readonly state: PacksState = { count: 0, setHash: EMPTY_SET_HASH, locked: false };
  private packs: PackInput[] = [];
  private module: Promise<PacksModule> | null = null;
  private readonly list: HTMLOListElement;
  private readonly summary: HTMLElement;
  private readonly issues: HTMLUListElement;
  private readonly inputs: (HTMLInputElement | HTMLSelectElement)[] = [];
  private seq = 0;
  private readonly row: HTMLDivElement;
  private serverMsg: HTMLElement | null = null;
  private readonly downloads = new Set<Promise<void>>();

  /** Called after every change (the page refreshes what depends on the set). */
  onChange: (state: PacksState) => void = () => {};

  constructor(el: HTMLElement) {
    el.replaceChildren();
    const h = document.createElement('h2');
    h.textContent = 'Roster packs';
    const note = document.createElement('p');
    note.className = 'note';
    note.textContent = 'Packs change rosters, players and team names. They are applied to your own disc when the ' +
      'game starts, in this order. In an online game both players need the same packs in the same order.';
    const row = document.createElement('div');
    row.className = 'packs-row';
    this.row = row;
    row.append(this.picker('Add packs (.zip)', 'packs-zip', (input) => {
      input.accept = '.zip,application/zip';
      input.multiple = true;
    }, 'zip'));
    row.append(this.picker('Add a pack folder', 'packs-folder', (input) => {
      input.webkitdirectory = true;
    }, 'files'));
    this.list = document.createElement('ol');
    this.list.className = 'packs-list';
    this.summary = document.createElement('p');
    this.summary.className = 'packs-summary';
    this.issues = document.createElement('ul');
    this.issues.className = 'packs-issues';
    el.append(h, note, row, this.list, this.summary, this.issues);
    void this.refresh();
  }

  /**
   * The packs the server offers (`listUrl`: GET -> {packs: [{file, name, version, size, sha256}]}): a
   * dropdown before the file pickers. Choosing one adds it to the set; the choice is remembered.
   */
  async addServerList(listUrl: string, fetchFn: typeof fetch = fetch): Promise<void> {
    let packs: ServerPack[] = [];
    try {
      const r = await fetchFn(listUrl, { credentials: 'same-origin', cache: 'no-store' });
      if (!r.ok) return;
      const j = (await r.json()) as { packs?: unknown };
      packs = Array.isArray(j.packs) ? (j.packs as ServerPack[]).filter((p) => p && typeof p.file === 'string' && typeof p.name === 'string') : [];
    } catch {
      return;
    }
    if (!packs.length) return;
    const label = document.createElement('label');
    label.className = 'picker';
    label.textContent = 'Packs on this server ';
    const select = document.createElement('select');
    select.id = 'packs-server';
    const prompt = new Option('Choose a pack to add...', '');
    select.append(prompt, ...packs.map((p) => new Option(`${p.name} ${p.version}`, p.file)));
    label.append(select);
    this.row.prepend(label);
    this.inputs.push(select);
    this.serverMsg = document.createElement('p');
    this.serverMsg.className = 'note';
    this.serverMsg.setAttribute('role', 'status');
    this.row.after(this.serverMsg);
    const base = new URL(listUrl, location.href);
    const download = async (p: ServerPack, remembered: boolean) => {
      if (this.packs.some((x) => x.server === p.file)) return;
      this.say(`Downloading ${p.name} ${p.version} (${(p.size / 1048576).toFixed(1)} MB)...`);
      let data: ArrayBuffer;
      try {
        const r = await fetchFn(new URL(`${base.pathname.replace(/\/$/, '')}/${encodeURIComponent(p.file)}`, base).href,
          { credentials: 'same-origin' });
        if (!r.ok) {
          this.say(`Could not download ${p.name} (HTTP ${r.status}${r.status === 401 ? ': this browser has no access any more; open the private link again' : ''}).`, true);
          return;
        }
        data = await r.arrayBuffer();
      } catch (e) {
        this.say(`Could not download ${p.name}: ${String(e)}.`, true);
        return;
      }
      if (this.state.locked || this.packs.some((x) => x.server === p.file)) return;
      this.packs.push({ label: `${p.name} ${p.version}`, kind: 'zip', files: [{ path: p.file, data }], server: p.file });
      this.say(remembered ? `${p.name} ${p.version} added again (you chose it last time).` : '');
      rememberServerPacks(this.packs);
      await this.refresh();
    };
    // Tracked, so a game started meanwhile waits for it (settled()).
    const add = (p: ServerPack, remembered: boolean): Promise<void> => {
      const job = download(p, remembered).finally(() => this.downloads.delete(job));
      this.downloads.add(job);
      return job;
    };
    select.addEventListener('change', () => {
      const p = packs.find((x) => x.file === select.value);
      select.value = '';
      if (p) void add(p, false);
    });
    for (const file of recallServerPacks()) {
      const p = packs.find((x) => x.file === file);
      if (p) await add(p, true);
    }
  }

  /** A server pack is downloading: starting the game now would leave it out. */
  busy(): boolean {
    return this.downloads.size > 0;
  }

  /** Resolves when no server pack is downloading. */
  async settled(): Promise<void> {
    while (this.downloads.size) await Promise.allSettled([...this.downloads]);
  }

  private say(text: string, err = false): void {
    if (!this.serverMsg) return;
    this.serverMsg.textContent = text;
    this.serverMsg.className = err ? 'note err' : 'note';
  }

  /** The packs for the engine Worker (copies: the panel keeps its own). */
  current(): PackInput[] {
    return this.packs.map((p) => ({ ...p, files: p.files.map((f) => ({ path: f.path, data: f.data.slice(0) })) }));
  }

  /** The game started: the set can no longer change. */
  lock(): void {
    this.state.locked = true;
    for (const i of this.inputs) i.disabled = true;
    this.render();
  }

  /** The engine's result of applying the set to the disc. */
  showMounted(r: PackSetReport): void {
    this.state.mounted = r;
    this.render();
  }

  /** Static checks are ok (no errors). */
  ok(): boolean {
    return !this.state.report || this.state.report.ok;
  }

  private picker(text: string, id: string, setup: (i: HTMLInputElement) => void, kind: 'zip' | 'files'): HTMLElement {
    const label = document.createElement('label');
    label.className = 'picker';
    label.textContent = text + ' ';
    const input = document.createElement('input');
    input.type = 'file';
    input.id = id;
    setup(input);
    input.addEventListener('change', () => {
      const files = input.files;
      if (!files || !files.length) return;
      void readPick(files, kind).then((picked) => {
        input.value = '';
        this.packs.push(...picked);
        return this.refresh();
      });
    });
    label.append(input);
    this.inputs.push(input);
    return label;
  }

  private async check(): Promise<PackSetReport | undefined> {
    if (!this.packs.length) return undefined;
    if (!this.module) {
      this.module = import(new URL('./mvp-packs.js', import.meta.url).href)
        .then((m: { default: () => Promise<PacksModule> }) => m.default());
    }
    return feedPacks(await this.module, this.packs);
  }

  private async refresh(): Promise<void> {
    const seq = ++this.seq;
    let report: PackSetReport | undefined;
    try {
      report = await this.check();
    } catch (e) {
      report = { ok: false, set_hash: '', packs: [], issues: [{ severity: 'error', code: 'E-PACK', where: 'page',
        message: `the pack checker failed to load: ${String(e)}` }] };
    }
    if (seq !== this.seq) return;  // a newer change is being checked
    this.state.report = report;
    this.state.count = this.packs.length;
    this.state.setHash = report?.set_hash || EMPTY_SET_HASH;
    this.render();
    this.onChange(this.state);
  }

  private move(i: number, d: number): void {
    const j = i + d;
    if (j < 0 || j >= this.packs.length) return;
    [this.packs[i], this.packs[j]] = [this.packs[j]!, this.packs[i]!];
    rememberServerPacks(this.packs);
    void this.refresh();
  }

  private render(): void {
    const r = this.state.mounted ?? this.state.report;
    this.list.replaceChildren(...this.packs.map((p, i) => {
      const li = document.createElement('li');
      const info = r?.packs[i];
      const name = document.createElement('strong');
      name.textContent = info?.name || p.label;
      const meta = document.createElement('span');
      meta.className = 'packs-meta';
      meta.textContent = info?.id ? ` ${info.id} ${info.version}` + (info.hash ? `, ${info.hash.slice(0, 12)}` : '') : ` ${p.label}`;
      li.append(name, meta);
      if (!this.state.locked) {
        const btn = (text: string, title: string, f: () => void) => {
          const b = document.createElement('button');
          b.type = 'button';
          b.textContent = text;
          b.title = title;
          b.addEventListener('click', f);
          return b;
        };
        li.append(btn('Up', 'Apply earlier', () => this.move(i, -1)), btn('Down', 'Apply later', () => this.move(i, 1)),
                  btn('Remove', 'Remove from the set', () => {
                    this.packs.splice(i, 1);
                    rememberServerPacks(this.packs);
                    void this.refresh();
                  }));
      }
      return li;
    }));
    const errors = r?.issues.filter((x) => x.severity === 'error').length ?? 0;
    const warnings = (r?.issues.length ?? 0) - errors;
    let text = this.packs.length
      ? `Pack set ${this.state.setHash.slice(0, 16)} (${this.packs.length} pack${this.packs.length > 1 ? 's' : ''}): ` +
        (errors ? `${errors} error(s), ${warnings} warning(s). Fix or remove the packs with errors.`
                : warnings ? `no errors, ${warnings} warning(s).` : 'no problems found.')
      : 'No packs: the game\'s own rosters.';
    if (this.state.mounted) {
      text += this.state.mounted.ok
        ? ` Applied to your disc: ${this.state.mounted.files?.length ?? 0} file(s) patched, result ` +
          `${(this.state.mounted.result_hash ?? '').slice(0, 16)}.`
        : ` Could not be applied to your disc${this.state.mounted.fatal ? `: ${this.state.mounted.fatal}` : ''}.`;
    } else if (this.packs.length && !this.state.locked) {
      text += ' The full check runs against your disc when the game starts.';
    }
    this.summary.textContent = text;
    this.summary.className = 'packs-summary' + (errors ? ' err' : '');
    this.issues.replaceChildren(...(r?.issues ?? []).slice(0, 200).map((x) => {
      const li = document.createElement('li');
      li.className = x.severity === 'error' ? 'err' : 'warn';
      li.textContent = `${x.code} ${x.where}: ${x.message}`;
      return li;
    }));
  }
}

/** A pack the server offers (engine/server/src/pack_store.ts). */
interface ServerPack {
  file: string;
  name: string;
  version: string;
  size: number;
  sha256?: string;
}

const REMEMBER_KEY = 'mvp.serverPacks';

/** The server packs of the set, in order, for the next visit (this browser only; storage may be off). */
function rememberServerPacks(packs: PackInput[]): void {
  try {
    localStorage.setItem(REMEMBER_KEY, JSON.stringify(packs.map((p) => p.server).filter(Boolean)));
  } catch {
    // no storage here: nothing is remembered
  }
}

function recallServerPacks(): string[] {
  try {
    const v = JSON.parse(localStorage.getItem(REMEMBER_KEY) ?? '[]') as unknown;
    return Array.isArray(v) ? v.filter((x): x is string => typeof x === 'string') : [];
  } catch {
    return [];
  }
}
