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
}

const HELD_BUCKETS = 8;
const TRIM_TICKS = 30;     // a queue deeper than TRIM_DEPTH for this many refreshes drops one picture
const TRIM_DEPTH = 2;
const QUEUE_MAX = 4;       // never more pictures waiting than this
const ACTIVE_TICKS = 12;   // pictures keep coming: one arrived within this many refreshes

export class Presenter {
  readonly s: DisplayStats;
  private queue: ImageBitmap[] = [];
  private sinceTick = 0;       // `now`: pictures that arrived since the last refresh
  private current = 0;         // refreshes the picture on screen has stayed
  private deep = 0;            // queue: refreshes in a row with more than TRIM_DEPTH waiting
  private quiet = 1e9;         // refreshes since the last picture arrived
  private lastTick = 0;
  private hzAvg = 0;

  // The single-threaded engine loaded (the page learns it from the Worker): queue its pictures.
  setMode(mode: 'now' | 'queue'): void {
    if (mode === this.s.mode) return;
    this.s.mode = mode;
    if (mode === 'now') for (const b of this.queue.splice(0)) this.ctx.transferFromImageBitmap(b);
  }

  constructor(private readonly ctx: ImageBitmapRenderingContext, mode: 'now' | 'queue') {
    this.s = { mode, ticks: 0, arrived: 0, shown: 0, dropped: 0, held: new Array<number>(HELD_BUCKETS).fill(0),
               underruns: 0, trimmed: 0, queueMax: 0, waiting: 0, hz: 0 };
  }

  // A picture from the render thread.
  push(bmp: ImageBitmap): void {
    ++this.s.arrived;
    this.quiet = 0;
    if (this.s.mode === 'now') {
      this.ctx.transferFromImageBitmap(bmp);
      ++this.s.shown;
      ++this.sinceTick;
      return;
    }
    this.queue.push(bmp);
    while (this.queue.length > QUEUE_MAX) {
      this.queue.shift()!.close();
      ++this.s.dropped;
      ++this.s.trimmed;
    }
    this.s.queueMax = Math.max(this.s.queueMax, this.queue.length);
  }

  // The page's animation frame (one per display refresh).
  tick(now: number): void {
    ++this.s.ticks;
    if (this.lastTick > 0) {
      const dt = now - this.lastTick;
      if (dt > 2 && dt < 50) this.hzAvg += 0.05 * (1000 / dt - this.hzAvg);
      this.s.hz = Math.round(this.hzAvg * 10) / 10;
    }
    this.lastTick = now;
    let fresh = 0;
    if (this.s.mode === 'now') {
      fresh = this.sinceTick;
      this.sinceTick = 0;
      if (fresh > 1) this.s.dropped += fresh - 1;
    } else if (this.queue.length) {
      this.ctx.transferFromImageBitmap(this.queue.shift()!);
      ++this.s.shown;
      fresh = 1;
      this.s.waiting += this.queue.length;
      this.deep = this.queue.length > TRIM_DEPTH - 1 ? this.deep + 1 : 0;
      if (this.deep >= TRIM_TICKS) {
        this.queue.shift()!.close();
        ++this.s.dropped;
        ++this.s.trimmed;
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
    return { ...this.s, held: [...this.s.held] };
  }
}
