// The ABS challenge scene over the game (docs/mods/ABS_CHALLENGE.md §4.6). The
// moment a challenge is made, a red callout in the middle of the picture says who
// called for it ("Eckstein has called for a challenge"); the engine waits
// ANNOUNCE_MS before the review starts, and the callout stays while it rewinds. The
// replay itself plays at half speed (worker.ts) with only a small "Replay" tag.
// After it, the verdict, in the look of the game's own pause screen: "Call Overturned"
// or "Call Stands", the scoreboard row with the team and the challenges it has left,
// and where the pitch crossed; it fades out shortly after play resumes.
// Host-side only: nothing here reaches the game.

interface Challenge {
  team: string;   // "home" | "away"
  call: string;   // the umpire's call: "strike" | "ball"
  truth: string;  // what the pitch was
  x: number;      // plate crossing, zone-normalized: |x|, |y| <= 1 is a strike
  y: number;
  by: string;     // who called for it (the batter's or the catcher's last name; may be "")
  club: string;   // the team's nickname (may be "")
  left: number;   // the team's challenges after this one
}

const SVG = 'http://www.w3.org/2000/svg';
// Zone geometry in SVG units: half width 50, half height 65 (a zone is taller than wide).
const HW = 50, HH = 65, BALL_R = 8.5;

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
  private ballAt: SVGGElement;      // where the pitch crossed (moved there)
  private ball: SVGGElement;       // the game's white ring (its drop animation)
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
    for (let i = 0; i < 3; ++i) {
      for (let j = 0; j < 3; ++j) {
        this.zone.append(svg('rect', { x: -HW + (i * 2 * HW) / 3 + 1.5, y: -HH + (j * 2 * HH) / 3 + 1.5,
                                       width: (2 * HW) / 3 - 3, height: (2 * HH) / 3 - 3, class: 'abs-zone-cell' }));
      }
    }
    this.zone.append(svg('rect', { x: -HW, y: -HH, width: 2 * HW, height: 2 * HH, class: 'abs-zone-box' }));
    this.ballAt = svg('g', {});
    this.ball = svg('g', { class: 'abs-ball' });
    this.ball.append(svg('circle', { cx: 0, cy: 0, r: BALL_R, class: 'abs-ball-edge' }),
                     svg('circle', { cx: 0, cy: 0, r: BALL_R, class: 'abs-ball-ring' }));
    this.ballAt.append(this.ball);
    this.zone.append(this.ballAt);

    this.card.append(this.title, row, this.zone);
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
        this.challenge = { team: String(r.team), call: String(r.call), truth: String(r.truth),
                           x: Number(r.x), y: Number(r.y), by: String(r.by ?? ''), club: String(r.club ?? ''),
                           left: Number(r.left) };
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
      const cx = Math.max(-100, Math.min(100, c.x * HW));
      const cy = Math.max(-110, Math.min(110, -c.y * HH));
      this.ballAt.setAttribute('transform', `translate(${cx.toFixed(1)} ${cy.toFixed(1)})`);
      this.ballAt.style.display = '';
    } else {
      this.title.textContent = 'Review complete';
      this.ballAt.style.display = 'none';
    }
    this.showRow();
    this.root.hidden = false;
    // The ring drops in again each time (its animation runs when the element is shown again).
    this.ball.style.animation = 'none';
    void this.ball.getBoundingClientRect();
    this.ball.style.animation = '';
  }

  private fadeOut(): void {
    this.root.classList.add('abs-fading');
    this.fadeTimer = window.setTimeout(() => {
      this.root.hidden = true;
      this.root.classList.remove('abs-fading');
    }, 500);
  }
}
