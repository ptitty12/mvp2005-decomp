// Gameplay mods on the page (docs/mods/ABS_CHALLENGE.md): the ABS challenge
// settings (kept in this browser: a per-viewer convenience), the mod specs the
// engine takes before boot ("abs:miss=8,edge=3,challenges=2"), and the banner
// over the game: "Reviewing the call" while a review runs, then the verdict.
// The URL's mod= parameters, when present, win over the panel. While a review runs,
// the ABS challenge scene (abs-scene.ts) covers the game instead of a frozen picture.

import { ChallengeScene } from './abs-scene.js';

const KEY = 'mvp.mods.abs';

export interface AbsSettings {
  on: boolean;
  miss: number;        // umpire misses, percent of called pitches (0-30)
  edge: number;        // 0 anywhere ... 4 only at the edges
  challenges: number;  // per team (0-5)
}
export const ABS_DEFAULTS: AbsSettings = { on: false, miss: 8, edge: 3, challenges: 2 };

function clampInt(v: unknown, lo: number, hi: number, def: number): number {
  const n = Math.round(Number(v));
  return Number.isFinite(n) ? Math.min(hi, Math.max(lo, n)) : def;
}

export function loadAbs(): AbsSettings {
  try {
    const raw = window.localStorage.getItem(KEY);
    if (!raw) return { ...ABS_DEFAULTS };
    const o = JSON.parse(raw) as Partial<AbsSettings>;
    return {
      on: o.on === true,
      miss: clampInt(o.miss, 0, 30, ABS_DEFAULTS.miss),
      edge: clampInt(o.edge, 0, 4, ABS_DEFAULTS.edge),
      challenges: clampInt(o.challenges, 0, 5, ABS_DEFAULTS.challenges),
    };
  } catch {
    return { ...ABS_DEFAULTS };  // storage blocked or garbled: the defaults
  }
}

function saveAbs(s: AbsSettings): void {
  try {
    window.localStorage.setItem(KEY, JSON.stringify(s));
  } catch {
    // storage blocked: this visit only
  }
}

// The engine's spec for these settings, or null when the mod is off.
export function absSpec(s: AbsSettings): string | null {
  return s.on ? `abs:miss=${s.miss},edge=${s.edge},challenges=${s.challenges}` : null;
}

export class ModsPanel {
  private settings = loadAbs();
  private locked = false;
  private bannerTimer = 0;
  private reviewing = false;
  private scene: ChallengeScene | null;

  constructor(private root: HTMLElement | null, private banner: HTMLElement | null,
              private summary: HTMLElement | null, private fromUrl: string[], stage: HTMLElement | null) {
    this.scene = stage ? new ChallengeScene(stage) : null;
    this.build();
  }

  // The specs for the next game: the URL's, else the panel's.
  specs(): string[] {
    if (this.fromUrl.length) return this.fromUrl;
    const s = absSpec(this.settings);
    return s ? [s] : [];
  }

  // The game started: the settings can no longer change for it.
  lock(): void {
    this.locked = true;
    this.root?.querySelectorAll('input, select').forEach((e) => ((e as HTMLInputElement).disabled = true));
  }

  // The engine's report (mvp_mod_status): the timeline's phase, the review leg's
  // progress, messages for the player and the mod's UI records.
  show(phase: string, notes: string[], progress = 1, ui: Record<string, unknown>[] = []): void {
    if (this.scene) {
      this.scene.update(phase, progress, ui);
      // The scene tells the story of a review; the banner keeps the other messages.
      if (this.scene.active() || phase !== 'live') {
        notes = notes.filter((n) => !/^(ABS challenge|Overturned|The call stands)/.test(n));
        if (!notes.length) return;
      }
    }
    if (!this.banner) return;
    const reviewing = phase !== 'live' && !this.scene;
    if (reviewing && !this.reviewing) this.say('Reviewing the call...', 0);
    this.reviewing = reviewing;
    for (const n of notes) this.say(n, reviewing ? 0 : 4000);
    if (!reviewing && this.banner.dataset.sticky === '1' && !notes.length) this.say('', 0);
  }

  private say(text: string, ms: number): void {
    const b = this.banner!;
    window.clearTimeout(this.bannerTimer);
    b.textContent = text;
    b.hidden = !text;
    b.dataset.sticky = ms ? '0' : '1';
    if (ms) this.bannerTimer = window.setTimeout(() => (b.hidden = true), ms);
  }

  private build(): void {
    const root = this.root;
    if (!root) return;
    root.textContent = '';
    if (this.fromUrl.length) {
      const p = document.createElement('p');
      p.className = 'note';
      p.textContent = `From the address: ${this.fromUrl.join(' ')}`;
      root.append(p);
      this.updateSummary();
      return;
    }
    const s = this.settings;
    const row = (label: string, el: HTMLElement, after?: HTMLElement): HTMLLabelElement => {
      const l = document.createElement('label');
      l.className = 'mod-row';
      l.append(label, ' ', el);
      if (after) l.append(' ', after);
      root.append(l);
      return l;
    };
    const on = document.createElement('input');
    on.type = 'checkbox';
    on.checked = s.on;
    const onLabel = document.createElement('label');
    onLabel.className = 'mod-row mod-title';
    onLabel.append(on, ' ABS challenge: robot-umpire challenges and a human umpire who misses some calls');
    root.append(onLabel);

    const miss = document.createElement('input');
    miss.type = 'range';
    miss.min = '0';
    miss.max = '30';
    miss.value = String(s.miss);
    const missOut = document.createElement('output');
    row('Umpire misses', miss, missOut);

    const edge = document.createElement('select');
    for (const [v, t] of [['0', 'Anywhere'], ['1', 'A little near the edges'], ['2', 'Mostly near the edges'],
                           ['3', 'Near the edges'], ['4', 'Only right at the edge']]) {
      const o = document.createElement('option');
      o.value = v;
      o.textContent = t;
      edge.append(o);
    }
    edge.value = String(s.edge);
    row('Missed calls happen', edge);

    const chal = document.createElement('select');
    for (let k = 0; k <= 5; ++k) {
      const o = document.createElement('option');
      o.value = String(k);
      o.textContent = String(k);
      chal.append(o);
    }
    chal.value = String(s.challenges);
    row('Challenges per team', chal);

    const help = document.createElement('p');
    help.className = 'note';
    help.textContent = 'To challenge a called pitch, press L + R + Z together within a second and a half ' +
      '(keyboard: Q + E + Space). The batting team challenges strikes, the fielding team balls; a team keeps a ' +
      'challenge it wins. Takes effect when the game starts.';
    root.append(help);

    const sync = () => {
      missOut.textContent = `${miss.value}% of called pitches`;
      const disabled = this.locked || !on.checked;
      miss.disabled = edge.disabled = chal.disabled = disabled;
    };
    const changed = () => {
      if (this.locked) return;
      this.settings = { on: on.checked, miss: Number(miss.value), edge: Number(edge.value), challenges: Number(chal.value) };
      saveAbs(this.settings);
      sync();
      this.updateSummary();
    };
    for (const e of [on, miss, edge, chal]) e.addEventListener('input', changed);
    sync();
    this.updateSummary();
  }

  private updateSummary(): void {
    if (!this.summary) return;
    const specs = this.specs();
    this.summary.textContent = specs.length ? `(ABS challenge on)` : '';
  }
}
