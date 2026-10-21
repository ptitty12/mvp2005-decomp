// Shows the frames the threaded engine's render thread sends (P2: one ImageBitmap per
// present) on the page's `bitmaprenderer` canvas, and measures what reaches the screen.
//
// The measure (V1, development and the Stats overlay): at every animation frame of the
// page (one per display refresh), how many new pictures arrived since the last one. A
// picture is on screen from the refresh it was shown at until the next one replaces it,
// so `held[k]` counts pictures that stayed k refreshes, and `dropped` counts pictures
// replaced before any refresh showed them. At 60 Hz with the game making 60 pictures a
// second, a steady picture is all 1s; a hitch is a 2 next to a dropped one. At 120 Hz,
// all 2s (or all 1s with in-between frames).
//
// Two ways to show (URL `present=now|queue`; default: `now` for the threaded engine,
// `queue` for the single-threaded one):
// - `now`: each picture is handed to the canvas when its message arrives (P2's way). Two
//   pictures that arrive within one refresh show only the second. The threaded engine's
//   render thread already sends at most one picture per refresh, evenly: measured in play
//   (V1_LOG.md), 0.1-0.3% of pictures lost, with no added delay.
// - `queue` (V1): pictures wait in arrival order and each animation frame shows the
//   oldest one. Two that arrive within one refresh show on two refreshes instead of one
//   being lost; a queue that stays longer than two pictures for half a second drops one,
//   so the delay never grows past a refresh or two. It costs about a refresh of delay.
//   The single-threaded engine sends its pictures unevenly (5.7% lost with `now`, none
//   queued).
//
// In-between pictures (V3, the Motion setting's 120 fps): the render thread sends a
// picture between each two frames of the game (gfx/wgpu/renderer.h), just before the
// frame, and the page shows them in `queue` mode, so on a 120 Hz display each holds one
// refresh. They are counted apart (`between`), and not shown where they cannot help: in
// `now` mode (the frame right after would replace it at once) or on a display measured
// under BETWEEN_MIN_HZ (they would make the queue fall behind).

export interface DisplayStats {
  mode: 'now' | 'queue';
  ticks: number;          // animation frames seen
  arrived: number;        // pictures received
  shown: number;          // pictures put on the canvas
  dropped: number;        // pictures replaced or dropped before any refresh showed them
  held: number[];         // held[k]: pictures that stayed k refreshes (index 0 unused; the last bucket is "or more")
  underruns: number;      // queue: refreshes that found the queue empty while pictures kept coming
  trimmed: number;        // queue: pictures dropped to bound the delay
  queueMax: number;
  waiting: number;        // queue: pictures still waiting after each refresh's, summed (/ shown = the mean extra delay in refreshes)
  hz: number;             // the display's refresh rate, measured
  // V3: in-between pictures: received, shown, dropped by the queue's trimming, and skipped on
  // arrival (`now` mode, or a display under BETWEEN_MIN_HZ). They count in the totals above too.
  between: { arrived: number; shown: number; dropped: number; skipped: number };
}

const HELD_BUCKETS = 8;
const TRIM_TICKS = 30;     // a queue deeper than TRIM_DEPTH for this many refreshes drops one picture
const TRIM_DEPTH = 2;
const QUEUE_MAX = 4;       // never more pictures waiting than this
const ACTIVE_TICKS = 12;   // pictures keep coming: one arrived within this many refreshes
const BETWEEN_MIN_HZ = 90; // V3: in-between pictures are shown on displays measured at least this fast
const HZ_TICKS = 61;       // the refresh rate is measured over this many refreshes

interface Queued {
  bmp: ImageBitmap;
  between: boolean;  // V3: an in-between picture
}

export class Presenter {
  readonly s: DisplayStats;
  private queue: Queued[] = [];
  private sinceTick = 0;       // `now`: pictures that arrived since the last refresh
  private current = 0;         // refreshes the picture on screen has stayed
  private deep = 0;            // queue: refreshes in a row with more than TRIM_DEPTH waiting
  private quiet = 1e9;         // refreshes since the last picture arrived
  private lastTick = 0;
  private tickTimes: number[] = [];  // V3: the last refreshes' times (the rate over them)
  /** Development (V3, `window.mvp.grab`): sees each picture as it arrives, before it is shown. */
  onPicture: ((bmp: ImageBitmap, between: boolean) => void) | null = null;

  // The single-threaded engine loaded (the page learns it from the Worker): queue its pictures.
  setMode(mode: 'now' | 'queue'): void {
    if (mode === this.s.mode) return;
    this.s.mode = mode;
    if (mode === 'now') for (const q of this.queue.splice(0)) this.ctx.transferFromImageBitmap(q.bmp);
  }

  constructor(private readonly ctx: ImageBitmapRenderingContext, mode: 'now' | 'queue') {
    this.s = { mode, ticks: 0, arrived: 0, shown: 0, dropped: 0, held: new Array<number>(HELD_BUCKETS).fill(0),
               underruns: 0, trimmed: 0, queueMax: 0, waiting: 0, hz: 0,
               between: { arrived: 0, shown: 0, dropped: 0, skipped: 0 } };
  }

  // A picture from the render thread (V3: `between` for an in-between picture).
  push(bmp: ImageBitmap, between = false): void {
    this.onPicture?.(bmp, between);
    ++this.s.arrived;
    this.quiet = 0;
    if (between) {
      ++this.s.between.arrived;
      if (this.s.mode === 'now' || (this.s.hz > 0 && this.s.hz < BETWEEN_MIN_HZ)) {
        bmp.close();
        ++this.s.between.skipped;
        return;
      }
    }
    if (this.s.mode === 'now') {
      this.ctx.transferFromImageBitmap(bmp);
      ++this.s.shown;
      ++this.sinceTick;
      return;
    }
    this.queue.push({ bmp, between });
    while (this.queue.length > QUEUE_MAX) this.trim();
    this.s.queueMax = Math.max(this.s.queueMax, this.queue.length);
  }

  // Drops one waiting picture to bound the delay: the oldest in-between if there is one
  // (V3: the motion between two frames matters less than the frames), else the oldest.
  private trim(): void {
    let k = this.queue.findIndex((q) => q.between);
    if (k < 0) k = 0;
    const [q] = this.queue.splice(k, 1);
    q.bmp.close();
    ++this.s.dropped;
    ++this.s.trimmed;
    if (q.between) ++this.s.between.dropped;
  }

  // The page's animation frame (one per display refresh).
  tick(now: number): void {
    ++this.s.ticks;
    // The rate: refreshes over the last second or so (V3: a count, which uneven intervals do not
    // bias; a pause of more than 100 ms starts it over).
    if (this.lastTick > 0 && now - this.lastTick > 100) this.tickTimes.length = 0;
    this.tickTimes.push(now);
    if (this.tickTimes.length > HZ_TICKS) this.tickTimes.shift();
    const n = this.tickTimes.length;
    if (n >= 4) this.s.hz = Math.round(((n - 1) * 1000 / (now - this.tickTimes[0])) * 10) / 10;
    this.lastTick = now;
    let fresh = 0;
    if (this.s.mode === 'now') {
      fresh = this.sinceTick;
      this.sinceTick = 0;
      if (fresh > 1) this.s.dropped += fresh - 1;
    } else if (this.queue.length) {
      const q = this.queue.shift()!;
      this.ctx.transferFromImageBitmap(q.bmp);
      ++this.s.shown;
      if (q.between) ++this.s.between.shown;
      fresh = 1;
      this.s.waiting += this.queue.length;
      this.deep = this.queue.length > TRIM_DEPTH - 1 ? this.deep + 1 : 0;
      if (this.deep >= TRIM_TICKS) {
        this.trim();
        this.deep = 0;
      }
    } else if (this.quiet < ACTIVE_TICKS) {
      ++this.s.underruns;
    }
    ++this.quiet;
    if (fresh) {
      if (this.current > 0) ++this.s.held[Math.min(this.current, HELD_BUCKETS - 1)];
      this.current = 1;
    } else if (this.current > 0) {
      ++this.current;
    }
  }

  stats(): DisplayStats {
    return { ...this.s, held: [...this.s.held], between: { ...this.s.between } };
  }
}
