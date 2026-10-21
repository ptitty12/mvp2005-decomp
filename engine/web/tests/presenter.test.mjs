// The page's presenter (src/presenter.ts), data-free: what reaches the screen when
// pictures arrive evenly, in pairs, late, or faster than the display, in both modes.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { load } from './site.mjs';

const { Presenter } = await load('presenter.js');

// A canvas context that records which picture it shows; pictures are numbered objects.
function fakeCtx() {
  return { shown: [], transferFromImageBitmap(b) { this.shown.push(b.n); } };
}
const pic = (n) => ({ n, closed: false, close() { this.closed = true; } });

// Drives a presenter: `arrivals[k]` pictures arrive before refresh k (16.7 ms apart).
function run(mode, arrivals) {
  const ctx = fakeCtx();
  const p = new Presenter(ctx, mode);
  let n = 0, t = 1000;
  for (const a of arrivals) {
    for (let i = 0; i < a; ++i) p.push(pic(n++));
    t += 1000 / 60;
    p.tick(t);
  }
  return { p, ctx, s: p.stats() };
}

test('even pictures: each shown for one refresh, none lost, in both modes', () => {
  for (const mode of ['now', 'queue']) {
    const { s, ctx } = run(mode, new Array(120).fill(1));
    assert.equal(s.dropped, 0, mode);
    assert.equal(s.held[1], 119, mode);  // the last one is still on screen
    assert.equal(s.held[2], 0, mode);
    assert.deepEqual(ctx.shown.slice(0, 3), [0, 1, 2], mode);
    assert.ok(Math.abs(s.hz - 60) < 1, `${mode} ${s.hz}`);
  }
});

test('a pair then a gap: now loses the first, queue shows both one refresh apart', () => {
  const pattern = [];
  for (let k = 0; k < 30; ++k) pattern.push(1, 2, 0, 1);
  const now = run('now', pattern);
  assert.equal(now.s.dropped, 30);
  assert.equal(now.s.held[2], 30);  // the refresh with nothing new repeats a picture
  const q = run('queue', pattern);
  assert.equal(q.s.dropped, 0);
  assert.equal(q.s.held[2], 0);
  assert.equal(q.s.underruns, 0);
  assert.deepEqual(q.ctx.shown.slice(0, 8), [0, 1, 2, 3, 4, 5, 6, 7]);
});

test('queue: a deep queue is trimmed, so the delay stays bounded', () => {
  // Two pictures per refresh for a while: the queue never holds more than QUEUE_MAX.
  const { s, p } = run('queue', [...new Array(20).fill(2), ...new Array(60).fill(1)]);
  assert.ok(s.queueMax <= 4, String(s.queueMax));
  assert.ok(s.trimmed > 0);
  assert.equal(s.arrived, s.shown + s.dropped + p.queue.length);
});

test('queue: an empty queue while pictures keep coming counts as an underrun', () => {
  const { s } = run('queue', [1, 1, 0, 1, 1, 0, 1]);
  assert.equal(s.underruns, 2);
  assert.equal(s.held[2], 2);
});

test('setMode(now) shows what was queued', () => {
  const ctx = fakeCtx();
  const p = new Presenter(ctx, 'queue');
  p.push(pic(0));
  p.push(pic(1));
  p.setMode('now');
  assert.deepEqual(ctx.shown, [0, 1]);
  assert.equal(p.stats().mode, 'now');
});

// V3: in-between pictures. `pairs` frames, each an in-between then the frame, arriving
// together every `every` refreshes of a display at `hz`.
function runPairs(mode, hz, frames, every) {
  const ctx = fakeCtx();
  const p = new Presenter(ctx, mode);
  let n = 0, t = 1000;
  for (let k = 0; k < frames * every; ++k) {
    if (k % every === 0) {
      p.push(pic(n++), true);
      p.push(pic(n++));
    }
    t += 1000 / hz;
    p.tick(t);
  }
  return { p, ctx, s: p.stats() };
}

test('in-betweens at 120 Hz: each picture holds one refresh, none lost, counted apart', () => {
  const { s, ctx } = runPairs('queue', 120, 60, 2);
  assert.equal(s.dropped, 0);
  assert.equal(s.between.arrived, 60);
  assert.equal(s.between.shown, 60);
  assert.equal(s.between.skipped, 0);
  assert.equal(s.held[1], 119);
  assert.equal(s.held[2], 0);
  assert.deepEqual(ctx.shown.slice(0, 4), [0, 1, 2, 3]);  // in-between, frame, in-between, frame
  assert.ok(Math.abs(s.hz - 120) < 1, String(s.hz));
});

test('in-betweens are skipped where they cannot help: now mode, a 60 Hz display', () => {
  const now = runPairs('now', 120, 60, 2);
  assert.equal(now.s.between.skipped, 60);
  assert.equal(now.s.dropped, 0);
  assert.ok(!now.ctx.shown.some((n) => n % 2 === 0), 'no in-between shown');
  // 60 Hz: once the rate is measured, the in-betweens go and every frame holds one refresh.
  const slow = runPairs('queue', 60, 120, 1);
  assert.ok(slow.s.between.skipped >= 110, String(slow.s.between.skipped));
  assert.ok(slow.s.trimmed <= 5, String(slow.s.trimmed));
  assert.ok(slow.s.held[1] >= 110, String(slow.s.held[1]));
});

test('a deep queue drops an in-between before a frame', () => {
  const ctx = fakeCtx();
  const p = new Presenter(ctx, 'queue');
  p.push(pic(0));
  p.push(pic(1), true);
  p.push(pic(2));
  p.push(pic(3), true);
  p.push(pic(4));  // five waiting: one goes
  const s = p.stats();
  assert.equal(s.trimmed, 1);
  assert.equal(s.between.dropped, 1);
  p.tick(1000);
  p.tick(1017);
  assert.deepEqual(ctx.shown, [0, 2]);
});
