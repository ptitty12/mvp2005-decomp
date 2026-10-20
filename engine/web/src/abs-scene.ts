// The ABS challenge scene over the game (docs/mods/ABS_CHALLENGE.md §4.6). The
// moment a challenge is made, a red callout in the middle of the picture says who
// called for it ("Eckstein has called for a challenge"); the engine waits
// ANNOUNCE_MS before the review starts, and the callout stays while it rewinds. The
// replay itself plays at half speed (worker.ts) with only a small "Replay" tag.
// After it, the verdict, in the look of the game's own pause screen: "Call Overturned"
// or "Call Stands", the scoreboard row with the team and the challenges it has left,
// and where the pitch crossed, drawn to scale: the batter's zone and the ball (a strike
// when any part of it touches the zone), with how far a ball missed by; it fades out
// shortly after play resumes. Host-side only: nothing here reaches the game.

interface Challenge {
  team: string;   // "home" | "away"
  call: string;   // the umpire's call: "strike" | "ball"
  truth: string;  // what the pitch was
  x: number;      // where the ball's centre crossed, zone-normalized: |x|, |y| <= 1 is inside the zone
  y: number;
  w: number;      // the zone's half-width and half-height, and the ball's radius, in inches
  h: number;
  r: number;
  off: number;    // a ball: inches between the ball's edge and the zone
  by: string;     // who called for it (the batter's or the catcher's last name; may be "")
  club: string;   // the team's nickname (may be "")
  left: number;   // the team's challenges after this one
}

const SVG = 'http://www.w3.org/2000/svg';
// The zone in SVG units: half width 50; the half height and the ball follow in proportion,
// from the engine's inches (a 69 in batter's zone, 9.0 by 10.2 in, until a challenge says).
const HW = 50;
const ZONE_W = 9.0, ZONE_H = 10.2, BALL_IN = 1.45;

function svg<K extends keyof SVGElementTagNameMap>(tag: K, attrs: Record<string, string | number>): SVGElementTagNameMap[K] {
  const e = document.createElementNS(SVG, tag);
  for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, String(v));
  return e;
}

const cap = (s: string) => s.charAt(0).toUpperCase() + s.slice(1);

// How long the callout shows before the replay (ms): the engine waits that long before
// it starts the review (mvp_mod_intro), so the callout can be read.
export const ANNOUNCE_MS = 1500;

export class ChallengeScene {
  private root: HTMLDivElement;  // the verdict panel, in the look of the game's pause screen
  private card: HTMLDivElement;
  private title: HTMLDivElement;
  private club: HTMLSpanElement;   // the scoreboard row: the team (silver) ...
  private left: HTMLSpanElement;   // ... and its challenges left (the blue tab)
  private zone: SVGSVGElement;
  private cells: SVGRectElement[] = [];
  private box: SVGRectElement;
  private ballAt: SVGGElement;      // where the pitch crossed (moved there)
  private ball: SVGGElement;       // the game's white ring (its drop animation)
  private ballEdge: SVGCircleElement;
  private ballRing: SVGCircleElement;
  private off: HTMLDivElement;     // how far a ball missed the zone (or that a strike only touched it)
  private callout: HTMLDivElement;
  private calloutName: HTMLSpanElement;
  private tag: HTMLDivElement;
  private challenge: Challenge | null = null;
  private phase = 'live';
  private fadeTimer = 0;
  private calloutTimer = 0;
  private verdictShown = false;

  constructor(stage: HTMLElement) {
    this.root = document.createElement('div');
    this.root.className = 'abs-scene';
    this.root.hidden = true;
    this.root.setAttribute('role', 'status');
    this.root.setAttribute('aria-live', 'polite');

    this.card = document.createElement('div');
    this.card.className = 'abs-card';
    this.title = document.createElement('div');
    this.title.className = 'abs-title';

    // The scoreboard row, as in the game's pause screen: the team on a silver bar, then a
    // blue tab with a slanted edge.
    const row = document.createElement('div');
    row.className = 'abs-row';
    this.club = document.createElement('span');
    this.club.className = 'abs-club';
    this.left = document.createElement('span');
    this.left.className = 'abs-left';
    row.append(this.club, this.left);

    // The zone as the game draws it at the plate: nine translucent cells in a light frame;
    // the pitch as its white ring.
    this.zone = svg('svg', { viewBox: '-110 -120 220 240', class: 'abs-zone', 'aria-hidden': 'true' });
    for (let k = 0; k < 9; ++k) {
      const cell = svg('rect', { class: 'abs-zone-cell' });
      this.cells.push(cell);
      this.zone.append(cell);
    }
    this.box = svg('rect', { class: 'abs-zone-box' });
    this.zone.append(this.box);
    this.ballAt = svg('g', {});
    this.ball = svg('g', { class: 'abs-ball' });
    this.ballEdge = svg('circle', { cx: 0, cy: 0, class: 'abs-ball-edge' });
    this.ballRing = svg('circle', { cx: 0, cy: 0, class: 'abs-ball-ring' });
    this.ball.append(this.ballEdge, this.ballRing);
    this.ballAt.append(this.ball);
    this.zone.append(this.ballAt);
    this.drawZone(ZONE_W, ZONE_H, BALL_IN);

    this.off = document.createElement('div');
    this.off.className = 'abs-off';
    this.off.hidden = true;

    this.card.append(this.title, row, this.zone, this.off);
    this.root.append(this.card);

    this.callout = document.createElement('div');
    this.callout.className = 'abs-callout';
    this.callout.setAttribute('role', 'status');
    this.callout.setAttribute('aria-live', 'assertive');
    this.calloutName = document.createElement('span');
    this.calloutName.className = 'abs-callout-name';
    this.callout.append(this.calloutName, document.createTextNode('has called for a challenge'));
    this.callout.hidden = true;

    this.tag = document.createElement('div');
    this.tag.className = 'abs-tag';
    this.tag.textContent = 'Replay';
    this.tag.hidden = true;

    stage.append(this.root, this.callout, this.tag);
  }

  // Whether the scene is on screen (the page then leaves its banner alone).
  active(): boolean {
    return !this.root.hidden || !this.callout.hidden || !this.tag.hidden;
  }

  // The engine's report: the timeline's phase, the leg's progress, the mod's records.
  update(phase: string, _progress: number, records: Record<string, unknown>[]): void {
    for (const r of records) {
      if (r.abs === 'challenge') {
        const num = (v: unknown, d: number) => (Number.isFinite(Number(v)) && Number(v) > 0 ? Number(v) : d);
        this.challenge = { team: String(r.team), call: String(r.call), truth: String(r.truth),
                           x: Number(r.x), y: Number(r.y), w: num(r.w, ZONE_W), h: num(r.h, ZONE_H),
                           r: num(r.r, BALL_IN), off: Number(r.off) || 0,
                           by: String(r.by ?? ''), club: String(r.club ?? ''), left: Number(r.left) };
        this.verdictShown = false;
        this.showCallout();  // at once: the engine waits ANNOUNCE_MS before the review starts
      } else if (r.abs === 'verdict' && this.challenge) {
        this.challenge.left = Number(r.left);
        if (r.club) this.challenge.club = String(r.club);
        this.showRow();
      }
    }
    const prev = this.phase;
    this.phase = phase;
    switch (phase) {
      case 'seeking':
        window.clearTimeout(this.calloutTimer);  // the callout stays while the review rewinds
        break;
      case 'presenting':
        window.clearTimeout(this.calloutTimer);
        this.callout.hidden = true;
        this.root.hidden = true;
        this.tag.textContent = this.challenge ? `Replay · ${this.teamName()} challenge` : 'Replay';
        this.tag.hidden = false;
        break;
      case 'rerunning':
        this.callout.hidden = true;
        this.tag.hidden = true;
        if (!this.verdictShown) this.showVerdict();
        break;
      default:  // live
        this.tag.hidden = true;
        if (prev !== 'live') {
          this.callout.hidden = true;
          if (!this.verdictShown && this.challenge) this.showVerdict();
          window.clearTimeout(this.fadeTimer);
          this.fadeTimer = window.setTimeout(() => this.fadeOut(), 2200);
        }
    }
  }

  private teamName(): string {
    const c = this.challenge;
    return c ? c.club || cap(c.team) : '';
  }

  // The scoreboard row: the team, and the challenges it has left.
  private showRow(): void {
    const c = this.challenge;
    this.club.textContent = this.teamName();
    this.left.textContent = c ? `${c.left} challenge${c.left === 1 ? '' : 's'} left` : '';
  }

  private showCallout(): void {
    const c = this.challenge!;
    window.clearTimeout(this.fadeTimer);
    this.root.hidden = true;
    this.tag.hidden = true;
    this.calloutName.textContent = c.by || this.teamName();
    this.callout.hidden = false;
    // A fresh pop each time (the animation runs when the element is shown again).
    this.callout.style.animation = 'none';
    void this.callout.offsetWidth;
    this.callout.style.animation = '';
    // A review that never starts (the engine dropped it) must not leave the callout up.
    window.clearTimeout(this.calloutTimer);
    this.calloutTimer = window.setTimeout(() => (this.callout.hidden = true), ANNOUNCE_MS + 3000);
  }

  private showVerdict(): void {
    const c = this.challenge;
    this.verdictShown = true;
    window.clearTimeout(this.fadeTimer);
    this.root.classList.remove('abs-fading', 'abs-overturned', 'abs-stands');
    if (c) {
      const overturned = c.truth !== c.call;
      this.root.classList.toggle('abs-overturned', overturned);
      this.root.classList.toggle('abs-stands', !overturned);
      this.title.textContent = overturned ? 'Call Overturned' : 'Call Stands';
      const hh = this.drawZone(c.w, c.h, c.r);
      const cx = Math.max(-100, Math.min(100, c.x * HW));
      const cy = Math.max(-110, Math.min(110, -c.y * hh));
      this.ballAt.setAttribute('transform', `translate(${cx.toFixed(1)} ${cy.toFixed(1)})`);
      this.ballAt.style.display = '';
      // A ball only: how far its edge missed the zone (owner, 2026-10-20: the tag only ever says
      // "Missed by x in"). Hundredths below a tenth, so it never reads 0.0.
      const off = c.off < 0.1 ? Math.max(0.01, c.off).toFixed(2) : c.off.toFixed(1);
      this.off.textContent = c.truth === 'ball' ? `Missed by ${off} in` : '';
      this.off.hidden = !this.off.textContent;
    } else {
      this.title.textContent = 'Review complete';
      this.ballAt.style.display = 'none';
      this.off.hidden = true;
    }
    this.showRow();
    this.root.hidden = false;
    // The ring drops in again each time, and the popup after it (their animations run when shown again).
    for (const e of [this.ball, this.off] as (HTMLElement | SVGElement)[]) {
      e.style.animation = 'none';
      void e.getBoundingClientRect();
      e.style.animation = '';
    }
  }

  // The zone and the ball to scale: half-width `w`, half-height `h` and the ball's radius
  // `r` in inches, the zone's half-width HW SVG units. The ring's outer edge is the ball's
  // edge. Returns the half-height in SVG units.
  private drawZone(w: number, h: number, r: number): number {
    const k = HW / w, hh = Math.min(110, h * k), br = r * k;
    for (let i = 0; i < 3; ++i) {
      for (let j = 0; j < 3; ++j) {
        const cell = this.cells[3 * i + j]!;
        cell.setAttribute('x', String(-HW + (i * 2 * HW) / 3 + 1.5));
        cell.setAttribute('y', String(-hh + (j * 2 * hh) / 3 + 1.5));
        cell.setAttribute('width', String((2 * HW) / 3 - 3));
        cell.setAttribute('height', String((2 * hh) / 3 - 3));
      }
    }
    this.box.setAttribute('x', String(-HW));
    this.box.setAttribute('y', String(-hh));
    this.box.setAttribute('width', String(2 * HW));
    this.box.setAttribute('height', String(2 * hh));
    this.ballRing.setAttribute('r', String(br - 1.25));       // a 2.5 wide white ring inside the ball's edge
    this.ballEdge.setAttribute('r', String(Math.max(0, br - 3.5)));  // and a dark line inside it
    return hh;
  }

  private fadeOut(): void {
    this.root.classList.add('abs-fading');
    this.fadeTimer = window.setTimeout(() => {
      this.root.hidden = true;
      this.root.classList.remove('abs-fading');
    }, 500);
  }
}
