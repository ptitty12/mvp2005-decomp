// Q5: the even lockstep pacing (src/pacer.ts), data-free. The controller's
// parts (capacity, proposals, the common pace, the phase), and two simulated
// players in lockstep on one clock: each runs its fields on its paced schedule
// at its own cost per field, waits at a frame whose other input has not
// arrived, and sends its inputs and its pace reports over a link with latency.
// With the pacer both run evenly at the slower machine's pace and neither
// waits; without it (the earlier full-speed schedule) the faster one stops and
// goes.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { load } from './site.mjs';

const { Pacer, Schedule, FIELD_MS } = await load('pacer.js');

test('capacity: a field\'s time over the smoothed cost, its mean per window, following the scene either way', () => {
  const p = new Pacer();
  assert.equal(p.capacity(), 1);
  for (let k = 0; k < 14; ++k) p.noteWork(1, 12);
  assert.equal(p.cost, 0);  // not a whole window yet
  for (let k = 0; k < 196; ++k) p.noteWork(1, 12);
  assert.ok(Math.abs(p.cost - 12) < 1e-9);
  assert.ok(Math.abs(p.capacity() - FIELD_MS / 12) < 1e-9);
  for (let k = 0; k < 45; ++k) p.noteWork(1, 24);  // a heavier scene: most of the way in 45 fields
  assert.ok(p.cost > 20 && p.cost < 23, `cost ${p.cost}`);
  for (let k = 0; k < 45; ++k) p.noteWork(1, 12);  // lighter again: as quickly
  assert.ok(p.cost > 13 && p.cost < 15, `cost ${p.cost}`);
  // Fields that vary a lot from one to the next count by their mean, not their peaks.
  const q = new Pacer();
  for (let k = 0; k < 600; ++k) q.noteWork(1, k % 2 ? 30 : 18);
  assert.ok(Math.abs(q.cost - 24) < 1.0, `cost ${q.cost}`);
  p.noteWork(2, 40);  // a two-field batch counts per field
  p.noteWork(0, 50);  // no field: ignored
  p.noteWork(1, NaN);
  assert.ok(Number.isFinite(p.cost));
});

test('proposal: the capacity with the margin, in steps, lowered at once and raised after a while', () => {
  const p = new Pacer();
  for (let k = 0; k < 300; ++k) p.noteWork(1, 24);  // capacity 0.695
  let st = p.tick(0, null);
  assert.equal(p.prop, 0.68);  // 0.695 * 0.98 = 0.681 -> 0.68
  assert.equal(st.rate, 0.68);
  assert.equal(st.shiftMs, 0);
  for (let k = 0; k < 2000; ++k) p.noteWork(1, 12);  // capacity 1.39: back to full speed, after the hold
  st = p.tick(1000, null);
  assert.equal(p.prop, 0.68);
  st = p.tick(1400, null);
  assert.equal(p.prop, 0.68);
  st = p.tick(1600, null);
  assert.equal(p.prop, 1);
  assert.equal(st.rate, 1);
  // Lower at once; up only with room for two steps, after half a second.
  const q = new Pacer();
  for (let k = 0; k < 400; ++k) q.noteWork(1, 20.5);  // 0.814 * 0.98 = 0.797 -> 0.79
  q.tick(0, null);
  assert.equal(q.prop, 0.79);
  for (let k = 0; k < 2000; ++k) q.noteWork(1, 20.2);  // 0.826 * 0.98 = 0.809 -> 0.80: one step, not enough
  q.tick(100, null);
  q.tick(3000, null);
  assert.equal(q.prop, 0.79);
  for (let k = 0; k < 2000; ++k) q.noteWork(1, 20);  // 0.834 * 0.98 = 0.817 -> 0.81: two steps
  q.tick(3100, null);
  q.tick(3500, null);
  assert.equal(q.prop, 0.79);
  q.tick(3700, null);
  assert.equal(q.prop, 0.81);
  for (let k = 0; k < 2000; ++k) q.noteWork(1, 25);  // heavier: 0.667 * 0.98 = 0.654 -> 0.65 at once
  q.tick(3800, null);
  assert.equal(q.prop, 0.65);
  // Back and forth: the room has to last.
  for (let k = 0; k < 2000; ++k) q.noteWork(1, 24);  // 0.695 * 0.98 = 0.681 -> 0.68
  q.tick(3900, null);
  for (let k = 0; k < 2000; ++k) q.noteWork(1, 25.5);  // 0.654 * 0.98 = 0.641 -> 0.64: below, the wait restarts
  q.tick(4300, null);
  assert.equal(q.prop, 0.64);
  for (let k = 0; k < 2000; ++k) q.noteWork(1, 24);
  q.tick(4400, null);
  q.tick(4800, null);
  assert.equal(q.prop, 0.64);
  q.tick(5000, null);
  assert.equal(q.prop, 0.68);
});

test('the common pace is the lower proposal while the other side\'s reports are fresh', () => {
  const a = new Pacer(), b = new Pacer();
  for (let k = 0; k < 300; ++k) {
    a.noteWork(1, 11);
    b.noteWork(1, 24);
  }
  a.tick(0, null);
  b.tick(0, null);
  a.onPeer(b.report(null), 10);
  b.onPeer(a.report(null), 10);
  assert.equal(a.tick(250, null).rate, 0.68);
  assert.equal(b.tick(250, null).rate, 0.68);
  // An older report (the channel is unordered) is ignored; a stale one no longer counts.
  const old = { v: 1, n: 0, cap: 2, prop: 1, lead: null };
  a.onPeer(old, 300);
  assert.equal(a.peer.prop, 0.68);
  assert.equal(a.tick(2500, null).rate, 1);  // 2.49 s without a report: the other side is gone
  a.onPeer({ v: 1, n: 99, cap: 0.5, prop: 0.46, lead: null }, 2600);
  assert.equal(a.tick(2700, null).rate, 0.46);
  a.onPeer({ v: 2, n: 100, cap: 0.1, prop: 0.1, lead: null }, 2800);  // an unknown version: ignored
  assert.equal(a.tick(2900, null).rate, 0.46);
});

test('phase: each side moves its schedule by half the difference of the leads, a little at a time', () => {
  const a = new Pacer();
  a.tick(0, 10);
  a.onPeer({ v: 1, n: 1, cap: 1.4, prop: 1, lead: 50 }, 0);
  const st = a.tick(250, 10);  // this side's lead is 40 ms smaller: it runs early, so later
  assert.ok(st.shiftMs > 0, `shift ${st.shiftMs}`);
  assert.ok(Math.abs(st.shiftMs - 20 * 0.6 * 0.25) < 1e-9);
  a.onPeer({ v: 1, n: 2, cap: 1.4, prop: 1, lead: -1000 }, 250);
  const st2 = a.tick(500, 1000);  // far apart: at most 3 % of the elapsed time
  assert.ok(Math.abs(st2.shiftMs + 0.03 * 250) < 1e-9, `shift ${st2.shiftMs}`);
  const b = new Pacer();
  b.tick(0, null);
  b.onPeer({ v: 1, n: 1, cap: 1, prop: 1, lead: 30 }, 0);
  assert.equal(b.tick(250, null).shiftMs, 0);  // without its own lead, nothing moves
});

test('schedule: the pace changes the period and keeps the position; debts and shifts', () => {
  const s = new Schedule();
  s.reset(1000);
  assert.equal(s.due(1000), 0);
  assert.equal(s.due(1000 + FIELD_MS * 3.5), 3);
  s.fields = 3;
  s.setRate(0.5, 1000 + FIELD_MS * 3.5);
  assert.equal(s.due(1000 + FIELD_MS * 3.5), 0);
  assert.equal(s.due(1000 + FIELD_MS * 4.6), 1);  // half a field later at full speed is a quarter at 0.5
  s.limitDebt(1000 + FIELD_MS * 100, 1);  // a long wait: one field due, no catching up
  assert.equal(s.due(1000 + FIELD_MS * 100), 1);
  s.shift(5);
  assert.equal(s.due(1000 + FIELD_MS * 100), 0);
});

// ---- two players in lockstep, simulated ----
//
// One poll per field, delay D fields. A player starts field f when its schedule has a field due and
// the other player's input for f has arrived (frames before D are neutral). Starting field f samples
// its own input for f + D, which arrives at the other side `latency` ms later (plus jitter); the field
// then takes `cost(t)` ms, and its picture is shown when it ends. With `pacing`, each side's Pacer
// ticks every 250 ms with its median input lead, and its report reaches the other side with the
// same latency; a wait moves the schedule on (limitDebt). Without, the schedule runs at full speed
// and catches up after waits (the chase: two fields per batch when three are due), the earlier loop.
function simulate({ costA, costB, latency = 5, jitter = 0, delay = 3, seconds = 30, pacing = true,
                    offsetB = 0, step = 0.25, seed = 1, tune = {} }) {
  let rnd = seed;
  const random = () => ((rnd = (rnd * 1103515245 + 12345) % 2147483648) / 2147483648);
  const players = [costA, costB].map((cost, i) => ({
    i, cost, sched: new Schedule(), pacer: new Pacer({ decide: i === 0 ? 1 : 0, ...tune }), frame: 0, busyUntil: 0, inbox: new Map(),
    waiting: false, waitFrom: 0, waits: 0, waitMs: 0, shown: [], leads: [], reports: [], lastTick: -1e9, debtFields: 0,
    busy: 0, ran: [],
  }));
  players[1].sched.start = offsetB;
  const end = seconds * 1000;
  for (let t = 0; t <= end; t += step) {
    for (const p of players) {
      const q = players[1 - p.i];
      // Reports that have arrived.
      while (p.reports.length && p.reports[0].at <= t) p.pacer.onPeer(p.reports.shift().r, t);
      if (pacing && t - p.lastTick >= 250) {
        p.lastTick = t;
        const recent = p.leads.slice(-24).sort((x, y) => x - y);
        const lead = recent.length ? recent[recent.length >> 1] : null;
        const st = p.pacer.tick(t, lead);
        p.sched.setRate(st.sched, t);
        p.sched.shift(st.shiftMs);
        q.reports.push({ at: t + latency, r: p.pacer.report(lead) });
      }
      if (t < p.busyUntil) continue;
      if (pacing) p.sched.limitDebt(t, p.waiting ? 1 : 2);  // as the Worker's paced loop online
      let due = p.sched.due(t);
      if (due > 8) {
        p.sched.limitDebt(t, 1);
        due = 1;
      }
      if (due < 1) continue;
      const f = p.frame;
      const arrived = f < delay ? -Infinity : p.inbox.get(f);
      if (arrived === undefined || arrived > t) {
        if (!p.waiting) {
          p.waiting = true;
          p.waitFrom = t;
          ++p.waits;
        }
        continue;
      }
      const need = p.waiting ? p.waitFrom : t;
      if (p.waiting) {
        p.waitMs += t - p.waitFrom;
        p.waiting = false;
      }
      if (f >= delay) p.leads.push(need - arrived);
      // The earlier chase ran two fields in a batch when three were due (when the second one's input is
      // here too); a batch shows its last picture when it ends.
      const next = f + 1 < delay ? -Infinity : p.inbox.get(f + 1);
      const batch = !pacing && due >= 3 && next !== undefined && next <= t ? 2 : 1;
      const c = p.cost(t, f) * batch;
      // Each field samples this side's input for its frame + D: the other side has it after the latency.
      q.inbox.set(f + delay, t + latency + jitter * random());
      if (batch === 2) q.inbox.set(f + 1 + delay, t + c / 2 + latency + jitter * random());
      p.busyUntil = t + c;
      p.busy += c;
      p.shown.push(t + c);
      p.ran.push([t + c, batch]);
      p.frame += batch;
      p.sched.fields += batch;
      p.pacer.noteWork(batch, c);
    }
  }
  return players;
}

function stats(p, from, to) {
  const shown = p.shown.filter((x) => x >= from && x <= to);
  const d = shown.slice(1).map((x, i) => x - shown[i]).sort((a, b) => a - b);
  const q = (v) => d[Math.min(d.length - 1, Math.floor(v * d.length))];
  return { fps: (1000 * d.length) / (shown[shown.length - 1] - shown[0]), p50: q(0.5), p95: q(0.95), max: d[d.length - 1],
           over25: d.filter((x) => x > 25).length / d.length };
}

test('lockstep: a 1.0x and a 0.7x machine run evenly at the slower one\'s pace, and nobody waits', () => {
  const fast = () => 11.5, slow = () => 24;  // capacity 1.45 and 0.70
  const after = simulate({ costA: fast, costB: slow, seconds: 30 });
  const before = simulate({ costA: fast, costB: slow, seconds: 30, pacing: false });
  const [a, b] = after;
  const rate = a.pacer.rate;
  assert.equal(rate, 0.68);
  assert.equal(b.pacer.rate, 0.68);
  const sa = stats(a, 10000, 30000), sb = stats(b, 10000, 30000);
  const period = FIELD_MS / rate;
  for (const s of [sa, sb]) {
    assert.ok(Math.abs(s.fps - 59.94 * rate) < 1.5, `fps ${s.fps}`);
    assert.ok(s.p95 < period + 2 && s.max < period + 8, `intervals ${JSON.stringify(s)} period ${period}`);
  }
  // Waits after the first seconds: none to speak of.
  const waitsLate = (p) => p.waits;
  assert.ok(a.waitMs < 300, `A waited ${a.waitMs} ms`);
  assert.ok(b.waitMs < 100, `B waited ${b.waitMs} ms`);
  // Before: the faster side stops and goes, and the slower one chases in two-field batches.
  const ba = stats(before[0], 10000, 30000), bb = stats(before[1], 10000, 30000);
  assert.ok(before[0].waits > 200, `before: A waited ${before[0].waits} times`);
  assert.ok(ba.p95 > sa.p95 + 5 || bb.p95 > sb.p95 + 5, `before ${JSON.stringify([ba, bb])} after ${JSON.stringify([sa, sb])}`);
  void waitsLate;
});

test('lockstep: the phase settles where both sides have the same lead, from any start', () => {
  for (const offsetB of [-60, 0, 45]) {
    const [a, b] = simulate({ costA: () => 11, costB: () => 13, seconds: 20, offsetB, latency: 8, jitter: 4, seed: 7 });
    const med = (v) => [...v].sort((x, y) => x - y)[v.length >> 1];
    const la = med(a.leads.slice(-120)), lb = med(b.leads.slice(-120));
    assert.ok(Math.abs(la - lb) < 8, `offset ${offsetB}: leads ${la} / ${lb}`);
    assert.ok(la > 15 && lb > 15, `offset ${offsetB}: leads ${la} / ${lb}`);
    assert.equal(a.pacer.rate, 1);
    const late = (p) => p.waitMs;
    assert.ok(late(a) < 400 && late(b) < 400, `offset ${offsetB}: waited ${a.waitMs} / ${b.waitMs} ms`);
  }
});

test('lockstep: back to full speed when the slower machine keeps up again; down at once when it cannot', () => {
  const slowUntil = (ms) => (t) => (t < ms ? 26 : 12);
  const [a, b] = simulate({ costA: () => 11, costB: slowUntil(12000), seconds: 24 });
  assert.equal(a.pacer.rate, 1);
  assert.equal(b.pacer.rate, 1);
  const s = stats(b, 20000, 24000);
  assert.ok(s.fps > 58, `fps ${s.fps}`);
  // A heavier stretch: the pace drops within about a second.
  const heavyFrom = (ms) => (t) => (t < ms ? 12 : 26);
  const [c, d] = simulate({ costA: () => 11, costB: heavyFrom(8000), seconds: 12 });
  assert.ok(d.pacer.rate <= 0.63 && c.pacer.rate <= 0.63, `rates ${c.pacer.rate} ${d.pacer.rate}`);  // 0.641 * 0.98
  const t = stats(d, 10000, 12000);
  assert.ok(t.p95 < FIELD_MS / d.pacer.rate + 3, JSON.stringify(t));
});

test('lockstep over a slower, jittery link: still even, the leads equal', () => {
  const [a, b] = simulate({ costA: () => 11, costB: () => 20, seconds: 25, latency: 30, jitter: 15, delay: 4, seed: 3 });
  assert.equal(a.pacer.rate, b.pacer.rate);
  const sa = stats(a, 10000, 25000), sb = stats(b, 10000, 25000);
  const period = FIELD_MS / a.pacer.rate;
  assert.ok(sa.p95 < period + 3 && sb.p95 < period + 3, JSON.stringify([sa, sb, period]));
  assert.ok(a.waitMs < 500 && b.waitMs < 500, `waited ${a.waitMs} / ${b.waitMs}`);
});

// ---- Q5b: roles ----

test('roles: the clearly slower side sets the pace and runs free; the other follows and trims its pace', () => {
  const a = new Pacer(), b = new Pacer({ decide: 0 });  // roles are on by default; a (player 1) decides them
  for (let k = 0; k < 300; ++k) {
    a.noteWork(1, 11);
    b.noteWork(1, 24);  // capacity 0.695
  }
  const exchange = (t) => {
    a.onPeer(b.report(null), t);
    b.onPeer(a.report(null), t);
  };
  a.tick(0, null);
  b.tick(0, null);
  exchange(10);
  let sa = a.tick(250, null), sb = b.tick(250, null);
  assert.equal(sa.rate, 0.68);
  assert.equal(sb.rate, 0.68);
  assert.equal(a.role, 0);  // not before the difference has held a second
  assert.equal(sa.sched, 0.68);
  for (let t = 500; t <= 1500; t += 250) {
    exchange(t - 1);
    sa = a.tick(t, null);
    sb = b.tick(t, null);
  }
  assert.equal(b.role, 1);
  assert.equal(a.role, -1);
  assert.ok(Math.abs(sb.sched - (FIELD_MS / 24) * 1.15) < 1e-9, `setter ${sb.sched}`);  // past its capacity: it never idles
  assert.equal(sb.shiftMs, 0);
  assert.equal(sa.sched, 0.695);  // the setter's capacity, as it reports it
  assert.equal(sa.rate, 0.68);    // the pace the overlay and the sound go by: as before
  // The follower corrects the whole lead difference itself, and trims its pace by it: its lead is
  // 40 ms smaller (it runs 20 ms early), so it moves later and runs a little slower.
  a.onPeer({ ...b.report(50), n: 99 }, 1600);
  const st = a.tick(1750, 10);
  assert.ok(Math.abs(st.shiftMs - 20 * 2 * 0.6 * 0.25) < 1e-9, `shift ${st.shiftMs}`);
  assert.ok(Math.abs(a.trim + 3e-4 * 20 * 0.25) < 1e-12, `trim ${a.trim}`);
  assert.ok(Math.abs(st.sched - 0.695 * (1 - 3e-4 * 20 * 0.25)) < 1e-12, `sched ${st.sched}`);
  // followEarlyMs (development): it aims to run that much earlier than the setter.
  const early = new Pacer({ followEarlyMs: 12 }), setter = new Pacer({ decide: 0 });
  for (let k = 0; k < 300; ++k) {
    early.noteWork(1, 11);
    setter.noteWork(1, 24);
  }
  setter.tick(0, null);  // its proposal: below full speed
  let sf;
  for (let t = 0; t <= 1500; t += 250) {
    early.onPeer({ ...setter.report(50), n: t + 1 }, t);
    sf = early.tick(t, 10);
  }
  assert.equal(early.role, -1);
  assert.ok(Math.abs(sf.shiftMs - 8 * 2 * 0.6 * 0.25) < 1e-9, `shift ${sf.shiftMs}`);
  // Close capacities: no roles (both alike, as in Q5); and none at full speed.
  const c = new Pacer(), d = new Pacer();
  for (let k = 0; k < 300; ++k) {
    c.noteWork(1, 24);
    d.noteWork(1, 24.5);
  }
  for (let t = 0; t <= 3000; t += 250) {
    c.onPeer(d.report(null), t);
    c.tick(t, null);
    d.tick(t, null);
  }
  assert.equal(c.role, 0);
  assert.equal(c.tick(3250, null).sched, c.rate);
  // At full speed the roles stay (for the next slower stretch), and both schedules run at 1.
  const e = new Pacer();
  for (let k = 0; k < 300; ++k) e.noteWork(1, 10);
  let se;
  for (let t = 0; t <= 3000; t += 250) {
    e.onPeer({ v: 1, n: t + 1, cap: 1.2, prop: 1, lead: 20 }, t);
    se = e.tick(t, 30);
  }
  assert.equal(e.role, -1);
  assert.equal(e.rate, 1);
  assert.equal(se.sched, 1);
  assert.equal(e.trim, 0);  // the trim waits at full speed
});

test('roles: two players in lockstep, one 1.3x slower, fields that vary: faster than alike, still even', () => {
  // The slower machine's fields cost 21 ms on average, from 13 to 33 ms; the faster one's 1.3x less.
  const vary = (f) => 1 + 0.55 * ((((f * 7919) % 101) / 100) - 0.5) + (f % 37 === 0 ? 0.5 : 0);
  const slow = (t, f) => 21 * vary(f), fast = (t, f) => (21 / 1.3) * vary(f + 13);
  const run = (tune) => simulate({ costA: fast, costB: slow, seconds: 40, latency: 1, jitter: 0.5, tune });
  const speed = (p) => p.ran.filter(([x]) => x >= 10000).reduce((n, [, k]) => n + k, 0) / 30 / 59.94;
  const alike = run({ roles: 0 });  // Q5: both alike
  const roles = run({});
  const [a, b] = roles;
  const cap = FIELD_MS / 21.3;
  assert.equal(b.pacer.role, 1);
  assert.equal(a.pacer.role, -1);
  assert.ok(speed(b) > speed(alike[1]) + 0.01, `speed ${speed(b)} against ${speed(alike[1])}`);
  assert.ok(speed(b) > 0.95 * cap, `speed ${speed(b)} capacity ${cap}`);
  // Even: the follower shows its frames at the pace's period; the setter at its fields' cost.
  const sa = stats(a, 10000, 40000), sb = stats(b, 10000, 40000);
  assert.ok(sa.p95 < FIELD_MS / a.pacer.rate + 6, JSON.stringify(sa));
  assert.ok(sb.p95 < 35, JSON.stringify(sb));
  assert.ok(a.waitMs < 1200 && b.waitMs < 600, `waited ${a.waitMs} / ${b.waitMs} ms`);
});
