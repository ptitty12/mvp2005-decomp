// Where the disc comes from (src/disc.ts), against fake server responses: the
// /config.json contract, the login gate, same-origin only, and the image
// probe (HEAD for the size, a ranged GET that must answer 206).
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { load } from './site.mjs';

const { findServerDisc, probeDiscUrl } = await load('disc.js');
const BASE = 'https://mvp.example.test/play/?x=1';

// fetch over a table of URL -> handler(init) -> Response.
function fakeFetch(routes, log = []) {
  return async (url, init = {}) => {
    log.push({ url, method: init.method ?? 'GET', headers: init.headers ?? {}, credentials: init.credentials });
    const h = routes[url];
    if (!h) return new Response('not found', { status: 404 });
    return h(init);
  };
}

const json = (v, status = 200) => new Response(JSON.stringify(v), { status, headers: { 'Content-Type': 'application/json' } });

test('config.json with a disc: a same-origin URL, fetched with credentials', async () => {
  const log = [];
  const r = await findServerDisc(BASE, fakeFetch({
    'https://mvp.example.test/config.json': () => json({ disc: '/api/disc/image' }),
  }, log));
  assert.deepEqual(r, { kind: 'url', url: 'https://mvp.example.test/api/disc/image' });
  assert.equal(log[0].url, 'https://mvp.example.test/config.json');  // the root, whatever the page's path
  assert.equal(log[0].credentials, 'same-origin');
});

test('no config.json, no disc in it, or not JSON: the picker', async () => {
  for (const routes of [
    {},
    { 'https://mvp.example.test/config.json': () => json({ lobby: true }) },
    { 'https://mvp.example.test/config.json': () => json({ disc: '' }) },
    { 'https://mvp.example.test/config.json': () => new Response('<html>', { status: 200 }) },
    { 'https://mvp.example.test/config.json': () => json({}, 500) },
  ]) {
    const r = await findServerDisc(BASE, fakeFetch(routes));
    assert.equal(r.kind, 'none', JSON.stringify(r));
  }
  const offline = await findServerDisc(BASE, async () => { throw new TypeError('Failed to fetch'); });
  assert.equal(offline.kind, 'none');
});

test('login gate and foreign origins are errors', async () => {
  const r = await findServerDisc(BASE, fakeFetch({ 'https://mvp.example.test/config.json': () => json({}, 401) }));
  assert.equal(r.kind, 'error');
  assert.equal(r.login, true);
  const foreign = await findServerDisc(BASE, fakeFetch({
    'https://mvp.example.test/config.json': () => json({ disc: 'https://elsewhere.test/image.iso' }),
  }));
  assert.equal(foreign.kind, 'error');
  assert.match(foreign.message, /must be on this site/);
});

const IMG = 'https://mvp.example.test/api/disc/image';
const SIZE = 1459978240;

function image({ head = 200, range = 206, length = true, contentRange = true } = {}) {
  return (init) => {
    if (init.method === 'HEAD')
      return new Response(null, { status: head, headers: length ? { 'Content-Length': String(SIZE) } : {} });
    if (init.headers?.Range === 'bytes=0-0') {
      if (range === 206)
        return new Response(new Uint8Array([0x43]), {
          status: 206, headers: contentRange ? { 'Content-Range': `bytes 0-0/${SIZE}` } : {},
        });
      return new Response(new Uint8Array(16), { status: range });
    }
    return new Response(null, { status: 400 });
  };
}

test('probe: size from HEAD, Range honoured', async () => {
  const log = [];
  const r = await probeDiscUrl(IMG, fakeFetch({ [IMG]: image() }, log));
  assert.deepEqual(r, { ok: true, size: SIZE });
  assert.equal(log[0].method, 'HEAD');
  assert.equal(log[1].headers.Range, 'bytes=0-0');
  assert.ok(log.every((l) => l.credentials === 'same-origin'));
  // No Content-Length on HEAD: the Content-Range total gives the size.
  const r2 = await probeDiscUrl(IMG, fakeFetch({ [IMG]: image({ length: false }) }));
  assert.deepEqual(r2, { ok: true, size: SIZE });
  const r3 = await probeDiscUrl(IMG, fakeFetch({ [IMG]: image({ length: false, contentRange: false }) }));
  assert.equal(r3.ok, false);
});

test('probe: 401 is the login error; a server without Range support is refused', async () => {
  const r = await probeDiscUrl(IMG, fakeFetch({ [IMG]: image({ head: 401 }) }));
  assert.equal(r.ok, false);
  assert.equal(r.login, true);
  const r2 = await probeDiscUrl(IMG, fakeFetch({ [IMG]: image({ range: 200 }) }));
  assert.equal(r2.ok, false);
  assert.match(r2.message, /Range/);
  const r3 = await probeDiscUrl(IMG, fakeFetch({ [IMG]: image({ head: 404 }) }));
  assert.equal(r3.ok, false);
  assert.match(r3.message, /HTTP 404/);
  const r4 = await probeDiscUrl(IMG, async () => { throw new TypeError('Failed to fetch'); });
  assert.equal(r4.ok, false);
});

test('the Server disc page link: same-origin only, absent anywhere else', async () => {
  const cfg = (v) => fakeFetch({ 'https://mvp.example.test/config.json': () => json(v) });
  const page = async (f) => (await findServerDisc(BASE, f)).discPage ?? null;
  assert.equal(await page(cfg({ disc: '/api/disc/image?v=1', discPage: '/disc' })), 'https://mvp.example.test/disc');
  assert.equal(await page(cfg({ discPage: '/disc' })), 'https://mvp.example.test/disc');
  assert.equal(await page(cfg({ disc: '/api/disc/image' })), null);
  assert.equal(await page(cfg({ discPage: 'https://elsewhere.test/disc' })), null);
  assert.equal(await page(fakeFetch({})), null);
  assert.equal(await page(async () => { throw new TypeError('Failed to fetch'); }), null);
});

test('the roster packs the server offers: same-origin only, with or without a disc', async () => {
  const cfg = (v) => fakeFetch({ 'https://mvp.example.test/config.json': () => json(v) });
  const packs = async (f) => (await findServerDisc(BASE, f)).packs ?? null;
  assert.equal(await packs(cfg({ disc: '/api/disc/image?v=1', packs: '/api/packs' })), 'https://mvp.example.test/api/packs');
  assert.equal(await packs(cfg({ discPage: '/disc', packs: '/api/packs' })), 'https://mvp.example.test/api/packs');
  assert.equal(await packs(cfg({ disc: '/api/disc/image' })), null);
  assert.equal(await packs(cfg({ packs: 'https://elsewhere.test/api/packs' })), null);
  assert.equal(await packs(cfg({ packs: 42 })), null);
  assert.equal(await packs(fakeFetch({})), null);
});

test('the admin: its CSRF token and its save states, same-origin only, with or without a disc', async () => {
  const cfg = (v) => fakeFetch({ 'https://mvp.example.test/config.json': () => json(v) });
  const admin = async (f) => (await findServerDisc(BASE, f)).admin ?? null;
  const want = { token: 't0k', states: 'https://mvp.example.test/api/states' };
  assert.deepEqual(await admin(cfg({ disc: '/api/disc/image', admin: true, adminToken: 't0k', states: '/api/states' })), want);
  assert.deepEqual(await admin(cfg({ discPage: '/disc', admin: true, adminToken: 't0k', states: '/api/states' })), want);
  assert.deepEqual(await admin(cfg({ admin: true, adminToken: 't0k' })), { token: 't0k' });
  assert.deepEqual(await admin(cfg({ admin: true, adminToken: 't0k', states: 'https://elsewhere.test/api/states' })), { token: 't0k' });
  // The memory card on the server (card.ts), same-origin only too.
  assert.deepEqual(await admin(cfg({ admin: true, adminToken: 't0k', states: '/api/states', card: '/api/card' })),
                   { ...want, card: 'https://mvp.example.test/api/card' });
  assert.deepEqual(await admin(cfg({ admin: true, adminToken: 't0k', card: 'https://elsewhere.test/api/card' })), { token: 't0k' });
  assert.equal(await admin(cfg({ admin: true })), null);
  assert.equal(await admin(cfg({ adminToken: 't0k', states: '/api/states' })), null);  // not the admin
  assert.equal(await admin(cfg({ admin: 'yes', adminToken: 't0k' })), null);
  assert.equal(await admin(fakeFetch({})), null);
});
