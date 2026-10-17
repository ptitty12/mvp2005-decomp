// Even lockstep pacing (Q5): two players' games run at one common pace, evenly,
// instead of stop and go when one machine is slower.
//
// Lockstep (docs/engine/NETPLAY.md) lets a frame run only when both players'
// inputs for it are known, so the faster machine is held to the slower one's
// pace. Before Q5 each page paced itself to the wall clock at full speed: the
// slower machine chased (two-field batches, every other frame shown) and the
// faster one ran ahead until it had to wait for the other's input, ran again
// when it came, waited again. Now:
//
// - Capacity. Each side measures what one field costs its machine: the Worker's
//   busy time per field (the run, the waits for its render thread, the present),
//   averaged over windows of 15 fields (single fields vary a lot: a mean per
//   field, not its peaks), then smoothed over about 30 fields either way (the
//   game's scenes change their cost within seconds, and the pace should follow
//   them up as soon as down; the cost does not depend on the pace, so this
//   cannot oscillate). Its capacity is the speed it could run at: a field's time
//   (16.7 ms) over that cost.
// - The common pace. Each side proposes its capacity times a margin (2 %),
//   capped at full speed, in steps of 0.01 (lowered at once, raised once it has
//   had room for two steps more for half a second). Both sides run at the
//   lower of the two proposals, which they tell each other several times a
//   second. When the slower machine keeps up again, its proposal rises, and the
//   pace returns to full speed.
// - Even fields. The page's paced loop runs one field per scheduled slot at that
//   pace (59.94 fields/s times the pace), so frames are made and shown evenly.
//   A side that had to wait for the other does not catch up afterwards (the wait
//   moves its schedule later); a side that fell behind its own schedule (a heavy
//   field) catches up one field at a time and at most two fields (no two-field
//   batches, which would show every other frame).
// - Phase. Equal paces are not enough: a side that runs a little earlier than
//   the other reaches each frame just as (or before) the other's input arrives,
//   and waits. Each side measures its input lead (net/driver.h: how long before
//   it needed the other's input for a frame that input arrived; negative when it
//   waited) and tells the other. The side with the smaller lead is ahead in
//   time; each side moves its schedule by half the difference, a little at a
//   time (at most 3 % of the elapsed time), so both leads become equal: the
//   middle of the window the input delay leaves, as far from waiting as either
//   side can be.
//
// Everything here is host timing: it decides when fields run, never what they
// compute (guest state depends on the disc and the per-frame inputs alone).
// The reports travel as text messages on the data channel next to the engine's
// datagrams (netplay.ts); the engine never sees them.

export const FIELD_MS = 1000 / 59.94;

/** What one side tells the other (a text message on the data channel, `{"mvp":"pace", ...}`). */
export interface PaceReport {
  v: 1;
  n: number;            // sequence number (the channel is unordered: older ones are ignored)
  cap: number;          // the speed this machine could run the game at (1 = full speed)
  prop: number;         // the pace it proposes (its capacity with the margin, in steps, at most 1)
  lead: number | null;  // its input lead, ms (median of its last frames), or null before any
  role?: number;        // Q5b: its role (1: it sets the pace, -1: it follows, 0: neither)
}

export interface PacerOptions {
  margin: number;         // a proposal is the capacity times this
  step: number;           // proposals in steps of this
  raiseAfterMs: number;   // a higher proposal only after it has held this long ...
  raiseSteps: number;     // ... with room for this many more steps
  windowFields: number;   // the cost is averaged over windows of this many fields ...
  attackFields: number;   // ... and smoothed over this many fields when it rises ...
  releaseFields: number;  // ... and when it falls
  phaseGain: number;      // per second: the share of the phase difference corrected
  slew: number;           // at most this many ms of schedule shift per ms of time
  staleMs: number;        // the other side's report counts this long
  minPace: number;        // the pace never goes below this
  roles: number;          // Q5b: 1 = the slower side runs free and the other follows it (below), 0 = both alike
  boost: number;          // Q5b: the slower side's schedule runs this much faster than its capacity ...
  roleMargin: number;     // ... once its capacity is below the other's by this share (the follower's: above) ...
  roleHoldMs: number;     // ... for this long
  trimGain: number;       // Q5b: the follower's pace trim per ms of phase difference per second ...
  trimMax: number;        // ... at most this much either way
  followEarlyMs: number;  // Q5b: the follower aims to run this much earlier than the side it follows
  decide: number;         // Q5b: 1 = this side decides the roles (player 1), 0 = it takes the other's
}

export const DEFAULT_PACER: PacerOptions = {
  margin: 0.98, step: 0.01, raiseAfterMs: 500, raiseSteps: 2, windowFields: 15, attackFields: 30, releaseFields: 30,
  phaseGain: 0.6, slew: 0.03, staleMs: 2000, minPace: 0.25,
  roles: 1, boost: 1.15, roleMargin: 0.03, roleHoldMs: 500, trimGain: 3e-4, trimMax: 0.04,
  followEarlyMs: 0, decide: 1,
};

/** One step of the pacing: the pace to run at and how far to move the schedule. */
export interface PaceStep {
  rate: number;     // the common pace (1 = full speed)
  sched: number;    // the pace this side's schedule runs at (Q5b: the common pace, but for roles)
  shiftMs: number;  // move the schedule this much later (negative: earlier)
}

export class Pacer {
  readonly o: PacerOptions;
  /** The smoothed host time one field costs this machine (ms); 0 before any. */
  cost = 0;
  /** This side's proposal and the common pace. */
  prop = 1;
  rate = 1;
  peer: PaceReport | null = null;
  private peerAt = -Infinity;
  private seq = 0;
  private lastTick = -1;
  private raiseSince = -1;
  /** Q5b: this side's role (1: it sets the pace and runs free, -1: it follows, 0: neither), its trim. */
  role = 0;
  trim = 0;
  private roleSince = -1;
  // Statistics: schedule shifts (ms, in all), fields measured.
  shifted = 0;
  fields = 0;

  constructor(options: Partial<PacerOptions> = {}) {
    this.o = { ...DEFAULT_PACER, ...options };
  }

  /** The speed this machine could run at (1 = full speed): a field's time over its cost. */
  capacity(): number {
    return this.cost > 0 ? FIELD_MS / this.cost : 1;
  }

  /** `ms` of host time went into `fields` fields (the run, its waits, the present). */
  noteWork(fields: number, ms: number): void {
    if (fields <= 0 || !(ms >= 0)) return;
    this.fields += fields;
    this.winFields += fields;
    this.winMs += ms;
    if (this.winFields < this.o.windowFields) return;
    const n = this.winFields, per = this.winMs / n;
    this.winFields = this.winMs = 0;
    if (this.cost === 0) {
      this.cost = per;
      return;
    }
    const k = per > this.cost ? this.o.attackFields : this.o.releaseFields;
    this.cost += (1 - Math.exp(-n / k)) * (per - this.cost);
  }
  private winFields = 0;
  private winMs = 0;

  /** The other side's report (stale or older ones are ignored). */
  onPeer(r: PaceReport, now: number): void {
    if (!r || r.v !== 1 || !(r.prop > 0) || !(r.cap > 0)) return;
    if (this.peer && r.n <= this.peer.n && now - this.peerAt < this.o.staleMs) return;
    this.peer = r;
    this.peerAt = now;
  }

  peerFresh(now: number): boolean {
    return !!this.peer && now - this.peerAt < this.o.staleMs;
  }

  /** This side's report for the other. */
  report(lead: number | null): PaceReport {
    return { v: 1, n: ++this.seq, cap: round3(this.capacity()), prop: this.prop, lead: lead === null ? null : round1(lead),
             role: this.role };
  }

  /**
   * Updates the proposal and the common pace, and says how far to move the
   * schedule. `lead`: this side's input lead now (null: unknown). Call a few
   * times a second.
   */
  tick(now: number, lead: number | null): PaceStep {
    const o = this.o;
    const dt = this.lastTick < 0 ? 0 : Math.min(1000, now - this.lastTick);
    this.lastTick = now;
    // The proposal: down at once, up only after it has had room for a while.
    const want = Math.max(o.minPace, Math.min(1, quantize(this.capacity() * o.margin, o.step)));
    if (want < this.prop) {
      this.prop = want;
      this.raiseSince = -1;
    } else if (want > this.prop) {
      const room = want >= 1 || want >= this.prop + o.raiseSteps * o.step - 1e-9;
      if (!room) this.raiseSince = -1;
      else if (this.raiseSince < 0) this.raiseSince = now;
      else if (now - this.raiseSince >= o.raiseAfterMs) {
        this.prop = want;
        this.raiseSince = -1;
      }
    } else {
      this.raiseSince = -1;
    }
    const fresh = this.peerFresh(now);
    this.rate = fresh ? Math.min(this.prop, this.peer!.prop) : this.prop;
    if (fresh && o.roles) {
      // One side decides (player 1), so both always agree; the other takes the opposite role.
      this.role = o.decide ? this.roleNow(this.peer!, now) : -(this.peer!.role ?? 0) || 0;
    } else {
      this.role = 0;
      this.roleSince = -1;
    }
    if (this.role !== -1) this.trim = 0;
    let sched = this.rate;
    // The phase: half the difference of the two leads, a little at a time.
    let shiftMs = 0;
    if (this.role === 1) {
      sched = Math.min(1, this.capacity() * o.boost);
    } else if (fresh && lead !== null && this.peer!.lead !== null && dt > 0) {
      // > 0: this side's lead is smaller, it runs early (Q5b: a follower, earlier than it aims to).
      const e = (this.peer!.lead - lead) / 2 - (this.role === -1 && this.rate < 1 ? o.followEarlyMs : 0);
      const limit = o.slew * dt;
      // Q5b: a follower corrects the whole difference itself (the side it follows does not), and
      // trims its pace by the difference that remains (the other's measured capacity is not exact).
      const gain = this.role === -1 ? 2 * o.phaseGain : o.phaseGain;
      shiftMs = Math.max(-limit, Math.min(limit, e * gain * (dt / 1000)));
      this.shifted += shiftMs;
      if (this.role === -1 && this.peer!.cap * (1 + this.trim) < 1) {  // (at full speed the trim waits)
        this.trim = Math.max(-o.trimMax, Math.min(o.trimMax, this.trim - o.trimGain * e * (dt / 1000)));
      }
    }
    if (this.role === -1) sched = Math.min(1, this.peer!.cap * (1 + this.trim));
    return { rate: this.rate, sched, shiftMs };
  }

  // Q5b: with roles, the side whose machine is clearly slower (its capacity `roleMargin` below
  // the other's for `roleHoldMs`) sets the pace: its schedule runs a little faster than its
  // capacity (`boost`), so it is always a little behind and runs each field as soon as the last
  // is done (one at a time, every frame shown), at what its machine can do. The other follows
  // it at that capacity (its reports), moving its schedule toward equal input leads (less
  // `followEarlyMs`) and trimming its pace by what is left. Close capacities keep the roles
  // they have (from the start: both alike, as without roles); the roles last through stretches
  // both machines run at full speed.
  private roleNow(peer: PaceReport, now: number): number {
    const mine = this.capacity(), theirs = peer.cap, m = this.o.roleMargin;
    const want = mine < theirs * (1 - m) ? 1 : mine > theirs * (1 + m) ? -1 : this.role;
    if (want === this.role) {
      this.roleSince = -1;
      return this.role;
    }
    if (this.roleSince < 0) this.roleSince = now;
    if (now - this.roleSince < this.o.roleHoldMs) return this.role;
    this.roleSince = -1;
    return want;
  }
}

function quantize(v: number, step: number): number {
  return Math.floor(v / step + 1e-9) * step;
}

function round1(v: number): number {
  return Math.round(v * 10) / 10;
}

function round3(v: number): number {
  return Math.round(v * 1000) / 1000;
}

/**
 * The schedule of the paced loop (worker.ts): field k of the schedule is due at
 * start + k * period, with the period a field's time over the pace. Changing the
 * pace keeps the current position; a shift moves every later slot.
 */
export class Schedule {
  start = 0;
  fields = 0;  // fields run on this schedule
  rate = 1;

  period(): number {
    return FIELD_MS / this.rate;
  }

  // (A microsecond's margin keeps floating-point rounding from putting a slot just after `now`.)

  /** Restart: the next field is due a field's time from now. */
  reset(now: number): void {
    this.start = now - this.fields * this.period() - 1e-3;
  }

  /** How many fields are due at `now` (negative: ahead of the schedule). */
  due(now: number): number {
    return Math.floor((now - this.start) / this.period()) - this.fields;
  }

  setRate(rate: number, now: number): void {
    if (!(rate > 0) || rate === this.rate) return;
    const pos = (now - this.start) / this.period();
    this.rate = rate;
    this.start = now - pos * this.period();
  }

  /**
   * At most `max` fields due from now on (a wait, or a debt too large to catch up, is not made
   * up). Returns the schedule time given up (ms).
   */
  limitDebt(now: number, max: number): number {
    const start = Math.max(this.start, now - (this.fields + max) * this.period() - 1e-3);
    const gave = start - this.start;
    this.start = start;
    return gave;
  }

  shift(ms: number): void {
    this.start += ms;
  }
}
