// Save-state files on the page (src/states.ts): the header the page reads before
// the game starts (runtime/state_file.h's layout), and what it says a state needs.
// Built bytes only, no game data.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { load } from './site.mjs';

const { readHeader, describeNeeds } = await load('states.js');

function file(header, { magic = 'MVPSTATE', version = 1, body = 64 } = {}) {
  const json = new TextEncoder().encode(JSON.stringify(header));
  const b = new Uint8Array(20 + json.length + body);
  b.set(new TextEncoder().encode(magic), 0);
  const v = new DataView(b.buffer);
  v.setUint32(8, version, true);
  v.setBigUint64(12, BigInt(json.length), true);
  b.set(json, 20);
  return b.buffer;
}

test('a state file\'s header: mods, packs, poll', () => {
  const h = { game: 'GV4E69-0', frame_polls: 150000, mods: ['abs:miss=8,edge=3,challenges=2'],
              packs: { set_hash: 's', result_hash: 'r', files: ['mlb-2026.zip'] }, label: 'top 9th' };
  assert.deepEqual(readHeader(file(h)), h);
  assert.equal(describeNeeds(h), 'gameplay mods abs:miss=8,edge=3,challenges=2; roster packs mlb-2026.zip');
  assert.equal(describeNeeds({}), 'no gameplay mods; no roster packs');
});

test('not a state, another version, a damaged header', () => {
  assert.equal(readHeader(file({}, { magic: 'NOTSTATE' })), 'not a save state');
  assert.equal(typeof readHeader(file({}, { version: 2 })), 'object');  // version 2: with the card's part
  assert.match(readHeader(file({}, { version: 3 })), /version 3; this page reads versions 1 and 2/);
  assert.equal(readHeader(new ArrayBuffer(10)), 'not a save state (too short)');
  const cut = file({ label: 'x' }, { body: 0 }).slice(0, 25);
  assert.equal(readHeader(cut), 'a damaged save state (header)');
});
