// What reached the screen in a browser_check run (V1): from the summary's stats
// windows (each carries the page's presenter counts, src/presenter.ts), over the
// windows from field F on (default: all of them).
//
//   node engine/web/tools/display_stats.mjs SUMMARY.json [--from F] [--json]
//
// Prints the game speed, the emulation's ms per field, the render thread's busy ms
// per field, pictures arrived and shown per second, pictures lost (replaced before
// any refresh showed them), queue underruns, and how many refreshes each picture
// stayed on screen. At 60 Hz with the game making a picture every field, a steady
// picture is all "1"; a hitch is a "2" (often next to a lost picture).
import { readFileSync } from 'node:fs';

const args = process.argv.slice(2);
const file = args.find((a) => !a.startsWith('--'));
const opt = (name, def) => {
  const i = args.indexOf(name);
  return i >= 0 && i + 1 < args.length ? args[i + 1] : def;
};
if (!file) {
  console.error('usage: display_stats.mjs SUMMARY.json [--from FIELD] [--json]');
  process.exit(2);
}
const from = Number(opt('--from', '0'));
const raw = JSON.parse(readFileSync(file, 'utf8'));
const run = raw.runs ? raw.runs[0] : raw;
const w = (run.windows ?? []).filter((x) => x.display && x.field >= from);
if (w.length < 2) {
  console.error(`not enough stats windows with display counts from field ${from} (${run.windows?.length ?? 0} windows)`);
  process.exit(1);
}
const a = w[0], b = w[w.length - 1];
const d = (k) => (b.display[k] ?? 0) - (a.display[k] ?? 0);
const wall = (b.at - a.at) / 1000;
const fields = b.field - a.field;
const later = w.slice(1);
const emuMs = later.reduce((s, x) => s + x.ms, 0) / Math.max(1, later.reduce((s, x) => s + x.fields, 0));
const held = b.display.held.map((v, i) => v - (a.display.held[i] ?? 0));
const shownNew = held.reduce((s, v) => s + v, 0);
const out = {
  ok: run.ok,
  fields: [a.field, b.field],
  wallS: +wall.toFixed(1),
  speed: +(fields / 59.94 / wall).toFixed(3),
  emulationMsPerField: +emuMs.toFixed(2),
  renderBusyMsPerField: a.renderBusyMs != null && b.renderBusyMs != null
    ? +((b.renderBusyMs - a.renderBusyMs) / Math.max(1, fields)).toFixed(2) : null,
  mode: b.display.mode,
  hz: b.display.hz,
  arrivedPerS: +(d('arrived') / wall).toFixed(1),
  shownNewPerS: +(shownNew / wall).toFixed(1),
  lostPerS: +(d('dropped') / wall).toFixed(2),
  lostPct: +(100 * d('dropped') / Math.max(1, d('arrived'))).toFixed(2),
  underrunsPerS: +(d('underruns') / wall).toFixed(2),
  meanWaitRefreshes: +(d('waiting') / Math.max(1, d('shown'))).toFixed(2),
  held: { 1: held[1] ?? 0, 2: held[2] ?? 0, 3: held[3] ?? 0, '4+': held.slice(4).reduce((s, v) => s + v, 0) },
  heldOver1Pct: +(100 * held.slice(2).reduce((s, v) => s + v, 0) / Math.max(1, shownNew)).toFixed(2),
};
if (args.includes('--json')) {
  console.log(JSON.stringify(out));
} else {
  console.log(`${out.ok ? 'ok' : 'NOT OK'} fields ${out.fields[0]}-${out.fields[1]} (${out.wallS} s): speed ${out.speed}x, ` +
              `emulation ${out.emulationMsPerField} ms/field` +
              (out.renderBusyMsPerField != null ? `, render thread ${out.renderBusyMsPerField} ms/field` : '') +
              `, display ${out.hz} Hz, mode ${out.mode}`);
  console.log(`  pictures: ${out.arrivedPerS}/s arrived, ${out.shownNewPerS}/s shown, ${out.lostPerS}/s lost (${out.lostPct}%), ` +
              `${out.underrunsPerS}/s underruns, mean wait ${out.meanWaitRefreshes} refreshes`);
  console.log(`  refreshes on screen: 1: ${out.held[1]}, 2: ${out.held[2]}, 3: ${out.held[3]}, 4+: ${out.held['4+']} ` +
              `(${out.heldOver1Pct}% of pictures held more than one)`);
}
