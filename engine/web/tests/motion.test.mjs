// The Motion setting's Auto (src/motion.ts), data-free: in-between frames on only on a
// fast display with room to spare, off again when the machine cannot keep up, with a hold.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { load } from './site.mjs';

const { MotionAuto, motionSetting } = await load('motion.js');

// Reports twice a second: `busy` render-thread ms per field, `gpu` GPU ms per field (one
// timed frame per field), at `hz`, the game at full speed unless `speed`.
function drive(m, seconds, { hz = 120, busy = 4, gpu = 3, speed = 1, threaded = true, t0 = 0, fields0 = 0, busy0 = 0,
                             gpu0 = 0 } = {}) {
  let t = t0, fields = fields0, busyMs = busy0, gpuMs = gpu0;
  const out = [];
  for (let k = 0; k < seconds * 2; ++k) {
    t += 500;
    const n = Math.round(30 * speed);
    fields += n;
    busyMs += n * busy;
    gpuMs += n * gpu;
    out.push(m.update({ now: t, hz, threaded, fields, busyMs, gpu: { n: fields, ms: gpuMs, dropped: 0 }, speed,
                        paused: false }));
  }
  return { on: m.on, t, fields, busyMs, gpuMs, out };
}

test('Auto: on for a 120 Hz display with room to spare, after a few seconds', () => {
  const m = new MotionAuto();
  const r = drive(m, 10);
  assert.equal(r.on, true);
  assert.equal(r.out[0], false);  // not at once
  assert.match(m.reason, /room to spare/);
});

test('Auto: off on a 60 Hz display, the single-threaded engine, or a busy machine', () => {
  assert.equal(drive(new MotionAuto(), 20, { hz: 60 }).on, false);
  assert.equal(drive(new MotionAuto(), 20, { hz: 59.9 }).on, false);
  assert.equal(drive(new MotionAuto(), 20, { threaded: false }).on, false);
  assert.equal(drive(new MotionAuto(), 20, { busy: 10 }).on, false);   // the render thread busy
  assert.equal(drive(new MotionAuto(), 20, { gpu: 9 }).on, false);     // the GPU busy over half the time
  assert.equal(drive(new MotionAuto(), 20, { speed: 0.9 }).on, false);
  assert.equal(drive(new MotionAuto(), 20, { hz: 144 }).on, true);
});

test('Auto: on, then off when the machine cannot keep up, and a hold before trying again', () => {
  const m = new MotionAuto();
  let r = drive(m, 10);
  assert.equal(r.on, true);
  // The render thread at 15 ms per field: off after a sustained stretch.
  r = drive(m, 6, { busy: 15, t0: r.t, fields0: r.fields, busy0: r.busyMs, gpu0: r.gpuMs });
  assert.equal(r.on, false);
  assert.match(m.reason, /render thread/);
  // Room again: not before the hold (doubled: it had come on less than a minute before).
  r = drive(m, 40, { t0: r.t, fields0: r.fields, busy0: r.busyMs, gpu0: r.gpuMs });
  assert.equal(r.on, false);
  r = drive(m, 30, { t0: r.t, fields0: r.fields, busy0: r.busyMs, gpu0: r.gpuMs });
  assert.equal(r.on, true);
  assert.equal(m.changes.length, 3);
});

test('Auto: a display that slows down (the window moved to a 60 Hz screen) turns it off', () => {
  const m = new MotionAuto();
  let r = drive(m, 10);
  assert.equal(r.on, true);
  r = drive(m, 4, { hz: 60, t0: r.t, fields0: r.fields, busy0: r.busyMs, gpu0: r.gpuMs });
  assert.equal(r.on, false);
  assert.match(m.reason, /60 Hz/);
});

test('the setting: the page parameter, else the stored one, else Auto', () => {
  assert.equal(motionSetting('1', '60'), '120');
  assert.equal(motionSetting('0', '120'), '60');
  assert.equal(motionSetting(null, '120'), '120');
  assert.equal(motionSetting(null, 'bogus'), 'auto');
  assert.equal(motionSetting(null, null), 'auto');
});
