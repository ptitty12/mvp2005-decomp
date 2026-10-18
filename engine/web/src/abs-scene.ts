// The ABS challenge scene over the game (docs/mods/ABS_CHALLENGE.md §4.6). The
// moment a challenge is made, a red callout in the middle of the picture says who
// called for it ("Eckstein has called for a challenge"); the engine waits
// ANNOUNCE_MS before the review starts, and the callout stays while it rewinds. The
// replay itself plays at half speed (worker.ts) with only a small "Replay" tag.
// After it, the verdict: "Call Overturned" or "Call Stands", where the pitch crossed,
// and the challenges the team has left; it fades out shortly after play resumes.
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
  private root: HTMLDivElement;  // the verdict card over a dimmed picture
  private card: HTMLDivElement;
  private title: HTMLDivElement;
  private status: HTMLDivElement;
  private zone: SVGSVGElement;
  private ball: SVGCircleElement;
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

    this.zone = svg('svg', { viewBox: '-110 -120 220 240', class: 'abs-zone', 'aria-hidden': 'true' });
    this.zone.append(svg('rect', { x: -HW, y: -HH, width: 2 * HW, height: 2 * HH, class: 'abs-zone-box' }));
    for (const k of [-1, 1]) {
      this.zone.append(svg('line', { x1: (k * HW) / 3, y1: -HH, x2: (k * HW) / 3, y2: HH, class: 'abs-zone-grid' }));
      this.zone.append(svg('line', { x1: -HW, y1: (k * HH) / 3, x2: HW, y2: (k * HH) / 3, class: 'abs-zone-grid' }));
    }
    this.ball = svg('circle', { cx: 0, cy: 0, r: BALL_R, class: 'abs-ball' });
    this.zone.append(this.ball);

    this.status = document.createElement('div');
    this.status.className = 'abs-status';

    this.card.append(this.title, this.zone, this.status);
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
        this.status.textContent = this.leftText();
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

  private leftText(): string {
    const c = this.challenge;
    if (!c) return '';
    return `${this.teamName()}: ${c.left} challenge${c.left === 1 ? '' : 's'} left`;
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
      this.ball.setAttribute('cx', cx.toFixed(1));
      this.ball.setAttribute('cy', cy.toFixed(1));
      this.ball.classList.toggle('abs-ball-strike', c.truth === 'strike');
      this.ball.style.display = '';
    } else {
      this.title.textContent = 'Review complete';
      this.ball.style.display = 'none';
    }
    this.status.textContent = this.leftText();
    this.root.hidden = false;
  }

  private fadeOut(): void {
    this.root.classList.add('abs-fading');
    this.fadeTimer = window.setTimeout(() => {
      this.root.hidden = true;
      this.root.classList.remove('abs-fading');
    }, 500);
  }
}
