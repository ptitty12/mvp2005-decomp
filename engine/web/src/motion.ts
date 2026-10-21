// The Motion setting (V3): Auto, 120 fps (in-between frames) or 60 fps.
//
// The game draws 60 frames a second. With in-between frames on, the renderer also
// draws a picture between each two frames, every object halfway (gfx/wgpu/renderer.h),
// and the page shows the pictures from its queue (presenter.ts), one per display
// refresh: 120 pictures a second on a 120 Hz display. Presentation only: the game,
// and an online game's lockstep, are the same either way.
//
// The setting is kept per browser (localStorage `mvp.motion.v1`); the page parameter
// `interp=0|1` overrides it (tests). 120 fps and 60 fps are fixed. Auto turns in-between
// frames on only where they help and the machine has room for them:
//
// - the threaded engine (the in-betweens are drawn on the render thread);
// - the display refreshes at 100 Hz or more (the presenter's measure; under 90 Hz for
//   two seconds turns them off again);
// - the machine keeps up: with them off, the render thread busy at most `onBusyMs` per
//   field and the GPU at most `onGpuShare` of the time (they cost both about as much
//   again: measured in docs/engine/V3_LOG.md), and the game at full speed. With them on,
//   the render thread over `offBusyMs` per field, the GPU over `offGpuShare` of the time
//   or the game under `minSpeed` for `sustainMs` turns them off, and Auto waits `holdMs`
//   (doubling when they had to go off again within a minute of coming on, up to
//   `maxHoldMs`) before it tries again.
//
// The decision only reads the engine's reports; nothing goes back into the game.

export type MotionSetting = 'auto' | '120' | '60';

export interface MotionReport {
  now: number;            // ms (performance.now())
  hz: number;             // the display's refresh rate as measured (0: not yet)
  threaded: boolean;      // the render thread draws
  fields: number;         // VI fields so far
  busyMs: number | null;  // the render thread's working time so far (ms)
  gpu: { n: number; ms: number; dropped?: number } | null;  // GPU frames timed so far, their ms, frames not timed
  speed: number | null;   // the game's speed over the last report (1 = full)
  paused: boolean;
}

export interface MotionOptions {
  onHz: number;         // the display at least this fast...
  offHz: number;        // ... and not under this for `hzMs`
  hzMs: number;
  onBusyMs: number;     // render thread ms per field with them off, to turn them on
  offBusyMs: number;    // ... with them on, to turn them off
  onGpuShare: number;   // the GPU's busy share with them off, to turn them on
  offGpuShare: number;  // ... with them on, to turn them off
  minSpeed: number;     // the game slower than this (them on) turns them off
  windowMs: number;     // the numbers are taken over this long
  sustainMs: number;    // a reason to turn them off must last this long
  fitMs: number;        // the reasons to turn them on must last this long
  holdMs: number;       // after turning them off, wait this long before turning them on
  maxHoldMs: number;
  undoWithinMs: number; // off this soon after on doubles the hold
}

export const DEFAULT_MOTION: MotionOptions = {
  onHz: 100, offHz: 90, hzMs: 2000, onBusyMs: 8.5, offBusyMs: 13.5, onGpuShare: 0.4, offGpuShare: 0.8,
  minSpeed: 0.95, windowMs: 2000, sustainMs: 3000, fitMs: 3000, holdMs: 30000, maxHoldMs: 300000,
  undoWithinMs: 60000,
};

interface Sample {
  at: number;
  fields: number;
  busyMs: number | null;
  gpuN: number;
  gpuMs: number;
  gpuDropped: number;
}

export class MotionAuto {
  readonly o: MotionOptions;
  /** Whether Auto wants in-between frames now. */
  on = false;
  /** Why, in a few words (statistics and the log). */
  reason = 'starting';
  /** The last window's numbers (statistics). */
  last: { busyPerField: number | null; gpuShare: number | null; hz: number; speed: number | null } | null = null;
  changes: { at: number; on: boolean; reason: string }[] = [];
  private samples: Sample[] = [];
  private fitSince = -1;
  private badSince = -1;
  private lowHzSince = -1;
  private offAt = -Infinity;
  private onAt = -Infinity;
  private hold: number;
  private offWhy = '';

  constructor(options: Partial<MotionOptions> = {}) {
    this.o = { ...DEFAULT_MOTION, ...options };
    this.hold = this.o.holdMs;
  }

  /** A report from the engine; returns whether in-between frames should be on. */
  update(r: MotionReport): boolean {
    const o = this.o;
    if (!r.threaded) return this.set(false, 'single-threaded engine', r.now);
    if (r.paused) return this.on;
    this.samples.push({ at: r.now, fields: r.fields, busyMs: r.busyMs, gpuN: r.gpu?.n ?? 0, gpuMs: r.gpu?.ms ?? 0,
                        gpuDropped: r.gpu?.dropped ?? 0 });
    while (this.samples.length > 2 && r.now - this.samples[1].at >= o.windowMs) this.samples.shift();
    const a = this.samples[0], b = this.samples[this.samples.length - 1];
    const wall = b.at - a.at, fields = b.fields - a.fields;
    if (wall < o.windowMs * 0.5 || fields <= 0) return this.on;  // not enough to go by yet
    const busyPerField = a.busyMs !== null && b.busyMs !== null ? (b.busyMs - a.busyMs) / fields : null;
    const n = b.gpuN - a.gpuN, dropped = b.gpuDropped - a.gpuDropped;
    const gpuShare = n > 0 ? ((b.gpuMs - a.gpuMs) * (n + dropped) / n) / wall : null;
    this.last = { busyPerField, gpuShare, hz: r.hz, speed: r.speed };

    // The display.
    if (r.hz > 0 && r.hz < o.offHz) {
      if (this.lowHzSince < 0) this.lowHzSince = r.now;
    } else {
      this.lowHzSince = -1;
    }
    const slowDisplay = r.hz <= 0 || r.hz < o.onHz;
    if (this.on) {
      if (this.lowHzSince >= 0 && r.now - this.lowHzSince >= o.hzMs)
        return this.set(false, `display at ${r.hz.toFixed(0)} Hz`, r.now);
      const why = busyPerField !== null && busyPerField > o.offBusyMs ? `render thread ${busyPerField.toFixed(1)} ms per field`
        : gpuShare !== null && gpuShare > o.offGpuShare ? `GPU busy ${(100 * gpuShare).toFixed(0)}%`
        : r.speed !== null && r.speed < o.minSpeed ? `game at ${r.speed.toFixed(2)}x`
        : '';
      if (!why) {
        this.badSince = -1;
        return this.on;
      }
      if (this.badSince < 0) this.badSince = r.now;
      if (r.now - this.badSince < o.sustainMs) return this.on;
      // Off, and a hold before the next try (longer when it did not last).
      this.hold = r.now - this.onAt < o.undoWithinMs ? Math.min(o.maxHoldMs, this.hold * 2) : o.holdMs;
      return this.set(false, why, r.now);
    }
    if (slowDisplay) {
      this.fitSince = -1;
      this.reason = r.hz > 0 ? `display at ${r.hz.toFixed(0)} Hz` : 'display rate not measured yet';
      return false;
    }
    if (r.now - this.offAt < this.hold) {
      this.fitSince = -1;
      this.reason = `held off ${((this.hold - (r.now - this.offAt)) / 1000).toFixed(0)} s more after: ${this.offWhy}`;
      return false;
    }
    const fits = (busyPerField === null || busyPerField <= o.onBusyMs) &&
                 (gpuShare === null || gpuShare <= o.onGpuShare) && (r.speed === null || r.speed >= 0.98);
    if (!fits) {
      this.fitSince = -1;
      this.reason = 'not enough room: ' + (busyPerField !== null && busyPerField > o.onBusyMs
        ? `render thread ${busyPerField.toFixed(1)} ms per field`
        : gpuShare !== null && gpuShare > o.onGpuShare ? `GPU busy ${(100 * gpuShare).toFixed(0)}%`
        : `game at ${(r.speed ?? 0).toFixed(2)}x`);
      return false;
    }
    if (this.fitSince < 0) this.fitSince = r.now;
    if (r.now - this.fitSince < o.fitMs) return false;
    return this.set(true, `display at ${r.hz.toFixed(0)} Hz, room to spare`, r.now);
  }

  private set(on: boolean, reason: string, now: number): boolean {
    this.reason = reason;
    if (on === this.on) return on;
    this.on = on;
    this.changes.push({ at: now, on, reason });
    if (this.changes.length > 20) this.changes.shift();
    if (on) this.onAt = now;
    else {
      this.offAt = now;
      this.offWhy = reason;
    }
    this.fitSince = this.badSince = -1;
    this.samples = this.samples.slice(-1);  // the next numbers are the new mode's
    return on;
  }
}

/** The setting from the page parameter `interp=0|1`, else the stored one, else Auto. */
export function motionSetting(param: string | null, stored: string | null): MotionSetting {
  if (param === '1') return '120';
  if (param === '0') return '60';
  if (stored === 'auto' || stored === '120' || stored === '60') return stored;
  return 'auto';
}
