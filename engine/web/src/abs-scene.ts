// The ABS challenge scene over the game (docs/mods/ABS_CHALLENGE.md §7). A review
// first rewinds without showing frames; instead of a frozen picture the page shows
// a broadcast-style card: who challenged which call, the strike zone being
// scanned, and the rewind's progress. The replay itself plays with only a small
// "Replay" tag. After it, the verdict: where the pitch crossed and whether the call
// stands; it fades out shortly after play resumes. Host-side only: nothing here
// reaches the game.

interface Challenge {
  team: string;   // "home" | "away"
  call: string;   // the umpire's call: "strike" | "ball"
  truth: string;  // what the pitch was
  x: number;      // plate crossing, zone-normalized: |x|, |y| <= 1 is a strike
  y: number;
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

export class ChallengeScene {
  private root: HTMLDivElement;
  private card: HTMLDivElement;
  private title: HTMLDivElement;
  private sub: HTMLDivElement;
  private status: HTMLDivElement;
  private bar: HTMLDivElement;
  private barBox: HTMLDivElement;
  private zone: SVGSVGElement;
  private scan: SVGRectElement;
  private ball: SVGCircleElement;
  private tag: HTMLDivElement;
  private challenge: Challenge | null = null;
  private phase = 'live';
  private fadeTimer = 0;
  private cardTimer = 0;
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
    this.sub = document.createElement('div');
    this.sub.className = 'abs-sub';

    this.zone = svg('svg', { viewBox: '-110 -120 220 240', class: 'abs-zone', 'aria-hidden': 'true' });
    const clip = svg('clipPath', { id: 'abs-zone-clip' });
    clip.append(svg('rect', { x: -HW, y: -HH, width: 2 * HW, height: 2 * HH }));
    const defs = svg('defs', {});
    defs.append(clip);
    this.zone.append(defs);
    this.zone.append(svg('rect', { x: -HW, y: -HH, width: 2 * HW, height: 2 * HH, class: 'abs-zone-box' }));
    for (const k of [-1, 1]) {
      this.zone.append(svg('line', { x1: (k * HW) / 3, y1: -HH, x2: (k * HW) / 3, y2: HH, class: 'abs-zone-grid' }));
      this.zone.append(svg('line', { x1: -HW, y1: (k * HH) / 3, x2: HW, y2: (k * HH) / 3, class: 'abs-zone-grid' }));
    }
    const scanGroup = svg('g', { 'clip-path': 'url(#abs-zone-clip)' });
    this.scan = svg('rect', { x: -HW, y: -HH - 18, width: 2 * HW, height: 18, class: 'abs-scan' });
    scanGroup.append(this.scan);
    this.zone.append(scanGroup);
    this.ball = svg('circle', { cx: 0, cy: 0, r: BALL_R, class: 'abs-ball' });
    this.zone.append(this.ball);

    this.barBox = document.createElement('div');
    this.barBox.className = 'abs-progress';
    this.bar = document.createElement('div');
    this.bar.className = 'abs-progress-bar';
    this.barBox.append(this.bar);
    this.status = document.createElement('div');
    this.status.className = 'abs-status';

    this.card.append(this.title, this.sub, this.zone, this.barBox, this.status);
    this.root.append(this.card);

    this.tag = document.createElement('div');
    this.tag.className = 'abs-tag';
    this.tag.textContent = 'Replay';
    this.tag.hidden = true;

    stage.append(this.root, this.tag);
  }

  // Whether the scene is on screen (the page then leaves its banner alone).
  active(): boolean {
    return !this.root.hidden || !this.tag.hidden;
  }

  // The engine's report: the timeline's phase, the leg's progress, the mod's records.
  update(phase: string, progress: number, records: Record<string, unknown>[]): void {
    for (const r of records) {
      if (r.abs === 'challenge') {
        this.challenge = { team: String(r.team), call: String(r.call), truth: String(r.truth),
                           x: Number(r.x), y: Number(r.y) };
        this.verdictShown = false;
      } else if (r.abs === 'verdict' && this.challenge) {
        const left = Number(r.left);
        this.status.textContent = r.overturned
          ? `${cap(this.challenge.team)} keeps the challenge (${left} left)`
          : `${cap(this.challenge.team)}: ${left} challenge${left === 1 ? '' : 's'} left`;
      }
    }
    const prev = this.phase;
    this.phase = phase;
    switch (phase) {
      case 'seeking':
        // From a snapshot near the release the rewind is over in a few frames
        // (ABS_CHALLENGE.md §7.2): the card waits so that it does not flash.
        if (prev !== 'seeking') {
          window.clearTimeout(this.cardTimer);
          this.cardTimer = window.setTimeout(() => {
            if (this.phase === 'seeking') this.showReview();
          }, 200);
        }
        this.setProgress(progress);
        break;
      case 'presenting':
        window.clearTimeout(this.cardTimer);
        this.root.hidden = true;
        this.tag.textContent = this.challenge ? `Replay · ${cap(this.challenge.team)} challenge` : 'Replay';
        this.tag.hidden = false;
        break;
      case 'rerunning':
        this.tag.hidden = true;
        if (!this.verdictShown) this.showVerdict();
        this.setProgress(progress);
        break;
      default:  // live
        this.tag.hidden = true;
        if (prev !== 'live') {
          if (!this.verdictShown && this.challenge) this.showVerdict();
          this.barBox.hidden = true;
          window.clearTimeout(this.fadeTimer);
          this.fadeTimer = window.setTimeout(() => this.fadeOut(), 2200);
        }
    }
  }

  private showReview(): void {
    window.clearTimeout(this.fadeTimer);
    const c = this.challenge;
    this.root.classList.remove('abs-fading', 'abs-verdict', 'abs-overturned', 'abs-stands');
    this.root.classList.add('abs-reviewing');
    this.title.textContent = 'ABS challenge';
    this.sub.textContent = c ? `${cap(c.team)} challenges the ${c.call} call` : 'Reviewing the call';
    this.status.textContent = 'Tracking the pitch...';
    this.ball.style.display = 'none';
    this.barBox.hidden = false;
    this.root.hidden = false;
  }

  private showVerdict(): void {
    const c = this.challenge;
    this.verdictShown = true;
    window.clearTimeout(this.fadeTimer);
    this.root.classList.remove('abs-reviewing', 'abs-fading');
    this.root.classList.add('abs-verdict');
    if (c) {
      const overturned = c.truth !== c.call;
      this.root.classList.toggle('abs-overturned', overturned);
      this.root.classList.toggle('abs-stands', !overturned);
      this.title.textContent = overturned ? 'Overturned' : 'Call stands';
      this.sub.textContent = overturned ? `It was a ${c.truth}` : `${cap(c.call)}, as called`;
      const cx = Math.max(-100, Math.min(100, c.x * HW));
      const cy = Math.max(-110, Math.min(110, -c.y * HH));
      this.ball.setAttribute('cx', cx.toFixed(1));
      this.ball.setAttribute('cy', cy.toFixed(1));
      this.ball.classList.toggle('abs-ball-strike', c.truth === 'strike');
      this.ball.style.display = '';
    } else {
      this.title.textContent = 'Review complete';
      this.sub.textContent = '';
    }
    this.status.textContent = 'Back to the game...';
    this.root.hidden = false;
  }

  private setProgress(p: number): void {
    this.bar.style.width = `${Math.round(Math.max(0, Math.min(1, p)) * 100)}%`;
  }

  private fadeOut(): void {
    this.root.classList.add('abs-fading');
    this.fadeTimer = window.setTimeout(() => {
      this.root.hidden = true;
      this.root.classList.remove('abs-fading');
    }, 500);
  }
}
