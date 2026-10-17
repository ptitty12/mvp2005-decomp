// The statistics overlay (Q5): how the game runs on this computer and, online,
// on the other player's, live over the picture; and a text summary to copy.
//
// Shown with the Stats button or the ` key (the key left of 1), off by default
// (the choice is kept in this browser). Each side measures itself from what its
// engine Worker reports about twice a second (StatsTracker) and, online, sends a
// summary to the other player once a second as a small text message on the data
// channel (netplay.ts), which the engine never sees. Display only: nothing here
// reaches the engine, so it cannot change the game.
//
// What each column shows, over the last few seconds:
// - game speed: VI fields run per second over 59.94 (1.00x = full speed);
// - emulation: host ms the engine Worker spent per field (16.7 ms is a field's
//   time: more cannot keep up), and the speed this machine could run at;
// - frames shown: new pictures presented per second;
// - waiting: the share of the time this side waited for the other player's
//   input, and how often (online);
// - sound: audio underruns (the sound ran dry) per minute;
// - picture: the internal resolution, and the GPU's time per frame.
// Online also: the round trip and its jitter, the input delay, and which
// computer sets the pace (pacer.ts).

import type { PaceState } from './protocol.js';

/** One side's numbers: what the overlay shows, and what is sent to the other player. */
export interface StatsSummary {
  speed: number | null;            // game speed (1 = full speed)
  emuMs: number | null;            // emulation host ms per field
  cap: number | null;              // the speed this machine could run at
  shown: number | null;            // new frames shown per second
  waitPct: number | null;          // % of the time waiting for the other player
  waits: number | null;            // waits per second
  underrunsPerMin: number | null;
  underruns: number | null;        // since the start
  scale: number | null;            // the picture's internal resolution
  gpuMs: number | null;            // the GPU's time per frame (ms)
  threaded: boolean | null;
  machine: string;                 // e.g. "16 threads, amd rdna-3, Chrome 155"
}

// What the Worker and the page report, as the tracker needs it.
export interface StatsInput {
  t: number;                       // performance.now()
  fields: number;                  // VI fields run so far
  windowFields: number;            // the stats window's fields and its host ms in the run
  windowMs: number;
  presentsNew: number | null;      // new frames presented so far
  gpu: { n: number; ms: number } | null;
  cap: number | null;
  scale: number | null;
  threaded: boolean | null;
  stalls: number | null;           // netplay: waits so far and their ms
  stallMs: number | null;
  underruns: number | null;        // the worklet's underruns so far
  paused: boolean;
}

const SPAN_MS = 3000;  // the overlay's numbers cover about this much time

export class StatsTracker {
  private samples: StatsInput[] = [];

  push(s: StatsInput): void {
    if (s.paused) {  // a pause is not slowness: start over when it ends
      this.samples = [];
      return;
    }
    this.samples.push(s);
    while (this.samples.length > 2 && s.t - this.samples[1]!.t >= SPAN_MS) this.samples.shift();
  }

  /** Seconds of play the numbers cover (0 without two samples). */
  seconds(): number {
    const a = this.samples[0], b = this.samples[this.samples.length - 1];
    return a && b ? (b.t - a.t) / 1000 : 0;
  }

  summary(machine: string): StatsSummary {
    const a = this.samples[0], b = this.samples[this.samples.length - 1];
    const sec = this.seconds();
    const d = (f: (x: StatsInput) => number | null): number | null => {
      if (!a || !b || sec <= 0) return null;
      const x = f(a), y = f(b);
      return x === null || y === null ? null : y - x;
    };
    // The run's host time per field over the windows after the first sample.
    let wf = 0, wm = 0;
    for (const x of this.samples.slice(1)) {
      wf += x.windowFields;
      wm += x.windowMs;
    }
    const fields = d((x) => x.fields), shown = d((x) => x.presentsNew), stalls = d((x) => x.stalls);
    const stallMs = d((x) => x.stallMs), under = d((x) => x.underruns);
    const gn = d((x) => x.gpu?.n ?? null), gms = d((x) => x.gpu?.ms ?? null);
    return {
      speed: fields === null ? null : fields / sec / 59.94,
      emuMs: wf > 0 ? wm / wf : null,
      cap: b?.cap ?? null,
      shown: shown === null ? null : shown / sec,
      waitPct: stallMs === null ? null : Math.min(100, (100 * stallMs) / (sec * 1000)),
      waits: stalls === null ? null : stalls / sec,
      underrunsPerMin: under === null ? null : (60 * under) / sec,
      underruns: b?.underruns ?? null,
      scale: b?.scale ?? null,
      gpuMs: gn && gms !== null ? gms / gn : null,
      threaded: b?.threaded ?? null,
      machine,
    };
  }
}

/** A short description of this machine for the summary: threads, GPU, browser. */
export function describeMachine(gpuName: string | null): string {
  const n = navigator.hardwareConcurrency;
  const ua = navigator.userAgent;
  const m = /Edg\/(\d+)/.exec(ua) ?? /Firefox\/(\d+)/.exec(ua) ?? /Chrome\/(\d+)/.exec(ua) ?? /Version\/(\d+).*Safari/.exec(ua);
  const browser = m ? `${/Edg\//.test(m[0]) ? 'Edge' : /Firefox/.test(m[0]) ? 'Firefox' : /Chrome/.test(m[0]) ? 'Chrome' : 'Safari'} ${m[1]}` : 'browser?';
  return [n ? `${n} threads` : null, gpuName || null, browser].filter(Boolean).join(', ');
}

export interface OverlayContext {
  online: boolean;
  player: number | null;      // this page's player online
  local: StatsSummary;
  peer: StatsSummary | null;  // the other player's, as last received (null: none yet, or solo)
  peerAgeMs: number | null;   // how old the other player's summary is
  rttMs: number | null;
  jitterMs: number | null;
  delay: number | null;
  pace: PaceState | null;
  isolated: boolean;
}

const fmt = {
  x: (v: number | null) => (v === null ? '-' : `${v.toFixed(2)}x`),
  ms: (v: number | null) => (v === null ? '-' : `${v.toFixed(1)} ms`),
  rate: (v: number | null) => (v === null ? '-' : `${v.toFixed(0)}/s`),
  pct: (v: number | null, n: number | null) => (v === null ? '-' : `${v.toFixed(0)}%${n === null ? '' : ` (${n.toFixed(1)}/s)`}`),
  perMin: (v: number | null, total: number | null) => (v === null ? '-' : `${v.toFixed(1)}/min${total === null ? '' : ` (${total})`}`),
  scale: (s: number | null, g: number | null) => (s === null ? '-' : `${s}x${g === null ? '' : `, GPU ${g.toFixed(1)} ms`}`),
};

/** The rows of the overlay: a label, this side's value, the other side's (online). */
export function overlayRows(c: OverlayContext): [string, string, string][] {
  const l = c.local, p = c.peer;
  const v = (f: (s: StatsSummary) => string) => [f(l), p ? f(p) : c.online ? '...' : ''] as const;
  const rows: [string, string, string][] = [
    ['Game speed', ...v((s) => fmt.x(s.speed))],
    ['Emulation per field', ...v((s) => fmt.ms(s.emuMs))],
    ['Could run at', ...v((s) => fmt.x(s.cap))],
    ['Frames shown', ...v((s) => fmt.rate(s.shown))],
  ];
  if (c.online) rows.push(['Waiting for the other', ...v((s) => fmt.pct(s.waitPct, s.waits))]);
  rows.push(['Sound underruns', ...v((s) => fmt.perMin(s.underrunsPerMin, s.underruns))]);
  rows.push(['Picture', ...v((s) => fmt.scale(s.scale, s.gpuMs))]);
  return rows;
}

/** The lines under the table: the link, the delay, and what limits the game. */
export function overlayNotes(c: OverlayContext): string[] {
  const out: string[] = [];
  const other = c.player === 1 ? 'player 2' : c.player === 2 ? 'player 1' : 'the other player';
  if (c.online) {
    out.push(`Round trip ${c.rttMs === null ? '-' : `${c.rttMs.toFixed(0)} ms`}` +
             `${c.jitterMs === null ? '' : `, jitter ${c.jitterMs.toFixed(1)} ms`}; input delay ` +
             `${c.delay === null ? '-' : `${c.delay} frames`}.`);
    out.push(limiter(c, other));
  } else if (c.local.speed !== null && c.local.speed < 0.97 && c.local.cap !== null && c.local.cap < 1) {
    out.push(`This computer cannot keep up (it could run the game at ${c.local.cap.toFixed(2)}x).`);
  }
  return out;
}

// Who holds the game back: the pace's limit, else who waits for whom.
function limiter(c: OverlayContext, other: string): string {
  const p = c.pace;
  if (p && p.rate < 0.995) {
    const theirs = p.peerProp !== null && p.peerProp < p.prop;
    const cap = theirs ? p.peerCap : p.cap;
    return `Both games run at ${p.rate.toFixed(2)}x: ${theirs ? `${other}'s computer` : 'this computer'} sets the pace` +
           `${cap === null ? '' : ` (it could run at ${cap.toFixed(2)}x)`}.`;
  }
  const mine = c.local.waitPct, theirs = c.peer?.waitPct ?? null;
  if (mine !== null && mine >= 5 && (theirs === null || mine > theirs + 3))
    return `Full pace, but this side waits for ${other} ${mine.toFixed(0)}% of the time (the network, or ${other}'s computer).`;
  if (theirs !== null && theirs >= 5 && (mine === null || theirs > mine + 3))
    return `Full pace, but ${other} waits for this side ${theirs.toFixed(0)}% of the time (the network, or this computer).`;
  return 'Neither computer holds the game back.';
}

/** The summary to paste into a chat: both columns, the notes, both machines. */
export function copyText(c: OverlayContext, when: Date): string {
  const rows = overlayRows(c);
  const head = c.online ? ['', `You (player ${c.player ?? '?'})`, `Player ${c.player === 1 ? 2 : 1}`] : ['', 'This computer', ''];
  const w0 = Math.max(...rows.map((r) => r[0].length), 0) + 2;
  const w1 = Math.max(head[1]!.length, ...rows.map((r) => r[1].length)) + 2;
  const line = (r: readonly string[]) => (r[0]!.padEnd(w0) + r[1]!.padEnd(w1) + (r[2] ?? '')).trimEnd();
  const out = [`MVP Baseball 2005 ${c.online ? 'online' : 'solo'} stats, ${when.toISOString().replace('T', ' ').slice(0, 19)} UTC`,
               line(head), ...rows.map(line), ...overlayNotes(c)];
  out.push(`This computer: ${c.local.machine}; ${c.local.threaded ? 'threaded engine' : 'single-threaded engine'}` +
           `${c.isolated ? '' : ' (page not cross-origin isolated)'}.`);
  if (c.peer) out.push(`${c.online ? `Player ${c.player === 1 ? 2 : 1}'s computer` : 'Other'}: ${c.peer.machine}; ` +
                       `${c.peer.threaded ? 'threaded engine' : 'single-threaded engine'}` +
                       `${c.peerAgeMs !== null && c.peerAgeMs > 3000 ? ` (numbers ${Math.round(c.peerAgeMs / 1000)} s old)` : ''}.`);
  if (c.pace) out.push(`Pace: ${c.pace.rate.toFixed(2)}x (this side proposes ${c.pace.prop.toFixed(2)}, the other ` +
                       `${c.pace.peerProp === null ? '-' : c.pace.peerProp.toFixed(2)}); input lead ` +
                       `${c.pace.lead === null ? '-' : `${c.pace.lead.toFixed(0)} ms`} here, ` +
                       `${c.pace.peerLead === null ? '-' : `${c.pace.peerLead.toFixed(0)} ms`} there` +
                       // Q5b: the roles (pacer.ts), and the pace this side's schedule runs at
                       `${c.pace.role === 1 ? `; this side sets the pace and runs free (its schedule at ${(c.pace.sched ?? 1).toFixed(2)}x)`
                         : c.pace.role === -1 ? `; this side follows (its schedule at ${(c.pace.sched ?? 1).toFixed(2)}x, trim ${((c.pace.trim ?? 0) * 100).toFixed(1)} %)`
                         : ''}.`);
  return out.join('\n');
}

/** A peer's summary as received (a text message): only the known fields, with their types. */
export function parsePeerStats(m: Record<string, unknown>): StatsSummary | null {
  const num = (k: string) => (typeof m[k] === 'number' && Number.isFinite(m[k]) ? (m[k] as number) : null);
  if (m.v !== 1) return null;
  return {
    speed: num('speed'), emuMs: num('emuMs'), cap: num('cap'), shown: num('shown'), waitPct: num('waitPct'),
    waits: num('waits'), underrunsPerMin: num('underrunsPerMin'), underruns: num('underruns'), scale: num('scale'),
    gpuMs: num('gpuMs'), threaded: typeof m.threaded === 'boolean' ? m.threaded : null,
    machine: typeof m.machine === 'string' ? m.machine.slice(0, 120) : '',
  };
}
