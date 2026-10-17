// Q5: the statistics overlay's numbers (src/netstats.ts), data-free: what the
// tracker makes of the Worker's reports, the rows and notes both players see,
// who holds the game back, the text Copy stats gives, and a peer's summary as
// received.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { load } from './site.mjs';

const { StatsTracker, overlayRows, overlayNotes, copyText, parsePeerStats } = await load('netstats.js');

// Reports every 500 ms: `fieldsPerS` fields a second at `msPerField` each, every field shown, `stallFrac` of
// the time waiting (one wait a report), an underrun every `underrunEvery` reports.
function feed(t, seconds, { fieldsPerS = 54, msPerField = 12, stallFrac = 0, underrunEvery = 0, gpuMs = 3,
                            paused = false } = {}, c = { fields: 0, shown: 0, stalls: 0, stallMs: 0, under: 0, gn: 0, gms: 0, t: 0 }) {
  for (let k = 0; k < seconds * 2; ++k) {
    c.t += 500;
    const f = fieldsPerS / 2;
    c.fields += f;
    c.shown += f;
    c.stalls += stallFrac > 0 ? 1 : 0;
    c.stallMs += 500 * stallFrac;
    if (underrunEvery && k % underrunEvery === 0) c.under += 1;
    c.gn += f;
    c.gms += f * gpuMs;
    t.push({ t: c.t, fields: c.fields, windowFields: f, windowMs: f * msPerField, presentsNew: c.shown,
             gpu: { n: c.gn, ms: c.gms }, cap: 16.68 / msPerField, scale: 2, threaded: true, stalls: c.stalls,
             stallMs: c.stallMs, underruns: c.under, paused });
  }
  return c;
}

test('the tracker: speed, emulation per field, frames shown, waiting, underruns, GPU over the last seconds', () => {
  const t = new StatsTracker();
  feed(t, 5, { fieldsPerS: 40, msPerField: 20, stallFrac: 0.25, underrunEvery: 2, gpuMs: 4 });
  const s = t.summary('16 threads, test gpu, Chrome 155');
  assert.ok(Math.abs(s.speed - 40 / 59.94) < 1e-6, `speed ${s.speed}`);
  assert.ok(Math.abs(s.emuMs - 20) < 1e-9);
  assert.ok(Math.abs(s.cap - 16.68 / 20) < 1e-9);
  assert.ok(Math.abs(s.shown - 40) < 1e-9);
  assert.ok(Math.abs(s.waitPct - 25) < 1e-9);
  assert.ok(Math.abs(s.waits - 2) < 1e-9);
  assert.ok(Math.abs(s.underrunsPerMin - 60) < 1e-6, `underruns ${s.underrunsPerMin}`);
  assert.ok(Math.abs(s.gpuMs - 4) < 1e-9);
  assert.equal(s.scale, 2);
  assert.ok(t.seconds() >= 2.5 && t.seconds() <= 3.0, `seconds ${t.seconds()}`);
  // A pause starts it over (a pause is not slowness).
  t.push({ t: 99999, fields: 0, windowFields: 0, windowMs: 0, presentsNew: 0, gpu: null, cap: null, scale: 2,
           threaded: true, stalls: null, stallMs: null, underruns: null, paused: true });
  assert.equal(t.seconds(), 0);
  assert.equal(t.summary('x').speed, null);
});

const ctx = (over) => ({
  online: true, player: 1, local: null, peer: null, peerAgeMs: 500, rttMs: 24, jitterMs: 1.5, delay: 3,
  pace: null, isolated: true, ...over,
});

test('rows and notes: both columns online; the pace and who sets it; waits without a pace limit', () => {
  const a = new StatsTracker(), b = new StatsTracker();
  feed(a, 4, { fieldsPerS: 40, msPerField: 12 });
  feed(b, 4, { fieldsPerS: 40, msPerField: 24 });
  const local = a.summary('desktop'), peer = b.summary('laptop');
  const pace = { rate: 0.68, prop: 1, cost: 12, cap: 1.39, lead: 60, peerProp: 0.68, peerCap: 0.7, peerLead: 61, shifted: 3 };
  const c = ctx({ local, peer, pace });
  const rows = overlayRows(c);
  assert.deepEqual(rows.map((r) => r[0]), ['Game speed', 'Emulation per field', 'Could run at', 'Frames shown',
                                           'Waiting for the other', 'Sound underruns', 'Picture']);
  assert.equal(rows[0][1], '0.67x');
  assert.equal(rows[1][2], '24.0 ms');
  const notes = overlayNotes(c).join(' ');
  assert.match(notes, /Round trip 24 ms, jitter 1\.5 ms; input delay 3 frames/);
  assert.match(notes, /Both games run at 0\.68x: player 2's computer sets the pace \(it could run at 0\.70x\)/);
  // This side sets it.
  const mine = overlayNotes(ctx({ local, peer, pace: { ...pace, prop: 0.6, peerProp: 1, cap: 0.62 } })).join(' ');
  assert.match(mine, /this computer sets the pace \(it could run at 0\.62x\)/);
  // Full pace, but one side waits for the other.
  const w = new StatsTracker();
  feed(w, 4, { fieldsPerS: 59.94, stallFrac: 0.2 });
  const waits = overlayNotes(ctx({ local: w.summary('x'), peer: local, pace: { ...pace, rate: 1, prop: 1, peerProp: 1 } })).join(' ');
  assert.match(waits, /this side waits for player 2 20% of the time/);
  const fine = overlayNotes(ctx({ local, peer, pace: { ...pace, rate: 1, prop: 1, peerProp: 1 } })).join(' ');
  assert.match(fine, /Neither computer holds the game back/);
  // Solo: one column, and a note only when this computer cannot keep up.
  const s = new StatsTracker();
  feed(s, 4, { fieldsPerS: 48, msPerField: 21 });
  const solo = { ...ctx({ local: s.summary('x') }), online: false, player: null };
  assert.equal(overlayRows(solo).length, 6);
  assert.match(overlayNotes(solo).join(' '), /cannot keep up \(it could run the game at 0\.79x\)/);
});

test('Copy stats: a block of text with both columns, the notes and both machines', () => {
  const a = new StatsTracker(), b = new StatsTracker();
  feed(a, 4, { fieldsPerS: 40, msPerField: 12 });
  feed(b, 4, { fieldsPerS: 40, msPerField: 24 });
  const pace = { rate: 0.68, prop: 1, cost: 12, cap: 1.39, lead: 60, peerProp: 0.68, peerCap: 0.695, peerLead: 61, shifted: 3 };
  const text = copyText(ctx({ local: a.summary('28 threads, nvidia lovelace, Chrome 155'),
                              peer: b.summary('16 threads, amd rdna-3, Chrome 155'), pace }),
                        new Date(Date.UTC(2026, 9, 16, 22, 5, 0)));
  const lines = text.split('\n');
  assert.equal(lines[0], 'MVP Baseball 2005 online stats, 2026-10-16 22:05:00 UTC');
  assert.match(lines[1], /You \(player 1\)\s+Player 2/);
  assert.match(text, /Game speed\s+0\.67x\s+0\.67x/);
  assert.match(text, /This computer: 28 threads, nvidia lovelace, Chrome 155; threaded engine\./);
  assert.match(text, /Player 2's computer: 16 threads, amd rdna-3, Chrome 155; threaded engine\./);
  assert.match(text, /Pace: 0\.68x \(this side proposes 1\.00, the other 0\.68\); input lead 60 ms here, 61 ms there\./);
  // Q5b: with roles, which one this side has.
  const follows = copyText(ctx({ local: a.summary('x'), peer: b.summary('y'), pace: { ...pace, role: -1, sched: 0.695, trim: 0.012 } }), new Date());
  assert.match(follows, /61 ms there; this side follows \(its schedule at 0\.69x, trim 1\.2 %\)\./);
  const sets = copyText(ctx({ local: b.summary('y'), peer: a.summary('x'), pace: { ...pace, role: 1, sched: 0.8 } }), new Date());
  assert.match(sets, /this side sets the pace and runs free \(its schedule at 0\.80x\)\./);
});

test('a peer\'s summary as received: known fields with their types only', () => {
  const p = parsePeerStats({ mvp: 'stats', v: 1, speed: 0.7, emuMs: 'x', cap: Infinity, shown: 40, threaded: true,
                             machine: 'a'.repeat(500), evil: '<script>' });
  assert.equal(p.speed, 0.7);
  assert.equal(p.emuMs, null);
  assert.equal(p.cap, null);
  assert.equal(p.shown, 40);
  assert.equal(p.threaded, true);
  assert.equal(p.machine.length, 120);
  assert.equal(p.evil, undefined);
  assert.equal(parsePeerStats({ v: 2, speed: 1 }), null);
});
