// Local development server for the browser build (E2): serves the engine's
// static site directory, and nothing else, on localhost.
//
//   node engine/web/tools/serve.mjs [--root DIR] [--port N] [--host ADDR] [--no-isolation]
//        [--disc PATH [--login] [--latency MS] [--rate MBPS] [--packs DIR]] [--states DIR]
//
// DIR defaults to build/engine/emscripten/web/site (what the "emscripten"
// preset's mvp-web target assembles: the page, the shell, the engine module and
// symbols.txt). It holds no game data: the player's browser reads the disc
// image the player picks, and nothing is ever uploaded here. Only GET and HEAD,
// only files inside DIR, no directory listings. Every response carries the
// cross-origin isolation headers (P2: COOP same-origin, COEP require-corp, CORP
// same-origin), so the page may share memory with threads and loads the
// threaded engine; --no-isolation leaves them out (the single-threaded
// fallback). Prints "serving http://127.0.0.1:PORT/" once it listens (port 0
// picks a free one).
//
// --disc PATH (E2c, local tests only): a mock of the owner's deployment
// (engine/server/, E5, is the real one) on 127.0.0.1: GET /config.json answers
// {"disc": "/api/disc/image"}, and /api/disc/image serves PATH to HEAD and to
// GET with a Range header (206, Cache-Control: private, no-store), so the page
// loads the disc without the picker. --login: /api/* answers 401 unless the
// request carries the cookie mvp_access=ok (the page then shows the no-access
// error). --latency MS / --rate MBPS: each image response waits MS first and
// is sent at that rate (a remote server, for the read-ahead measurements).
// --packs DIR (with --disc): the deployment's roster packs (engine/server's
// pack_store): /config.json also names "/api/packs", which lists DIR's .zip
// files, and /api/packs/FILE serves one (the page's "Packs on this server").
//
// --states DIR (local only): save-state files (runtime/state_file.h), for the
// page's ?state=NAME: GET /states/NAME.mvpstate serves DIR/NAME.mvpstate. A
// state is game-derived: DIR is a folder on this machine, never in the repo.

import { createServer } from 'node:http';
import { createReadStream, readdirSync, statSync } from 'node:fs';
import { basename, extname, join, normalize, resolve, sep } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = fileURLToPath(new URL('.', import.meta.url));
const args = process.argv.slice(2);
const arg = (name, fallback) => {
  const i = args.indexOf(name);
  return i >= 0 && i + 1 < args.length ? args[i + 1] : fallback;
};
const root = resolve(arg('--root', join(here, '..', '..', '..', 'build', 'engine', 'emscripten', 'web', 'site')));
const port = Number(arg('--port', '8000'));
const host = arg('--host', '127.0.0.1');
const disc = arg('--disc', '');
const login = args.includes('--login');
const latencyMs = Number(arg('--latency', '0'));
const rateMBps = Number(arg('--rate', '0'));
const isolate = !args.includes('--no-isolation');
const packsDir = arg('--packs', '');
const statesDir = arg('--states', '');

const types = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.mjs': 'text/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.wasm': 'application/wasm',
  '.txt': 'text/plain; charset=utf-8',
  '.json': 'application/json',
  '.png': 'image/png',
  '.svg': 'image/svg+xml',
};

const stats = { imageRequests: 0, imageBytes: 0, heads: 0 };

// ---- the mock deployment (--disc) ----
function mock(req, res, path) {
  if (path === '/config.json') {
    const body = JSON.stringify({ disc: '/api/disc/image', ...(packsDir ? { packs: '/api/packs' } : {}) });
    res.writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store', 'Content-Length': body.length });
    res.end(req.method === 'HEAD' ? undefined : body);
    return true;
  }
  if (!path.startsWith('/api/')) return false;
  if (login && !/(?:^|;\s*)mvp_access=ok(?:;|$)/.test(req.headers.cookie ?? '')) {
    res.writeHead(401, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' }).end('{"error":"no access"}');
    return true;
  }
  if (packsDir && (path === '/api/packs' || path.startsWith('/api/packs/'))) {
    const zips = readdirSync(packsDir).filter((f) => f.toLowerCase().endsWith('.zip'));
    if (path === '/api/packs') {
      const list = zips.map((f) => ({ file: f, name: f.replace(/\.zip$/i, ''), version: '', size: statSync(join(packsDir, f)).size }));
      const body = JSON.stringify({ packs: list });
      res.writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' }).end(req.method === 'HEAD' ? undefined : body);
      return true;
    }
    const f = path.slice('/api/packs/'.length);
    if (!zips.includes(f)) {
      res.writeHead(404).end();
      return true;
    }
    sendFile(req, res, join(packsDir, f), 'application/zip');
    return true;
  }
  if (path !== '/api/disc/image') {
    res.writeHead(404).end();
    return true;
  }
  const size = statSync(disc).size;
  const common = { 'Content-Type': 'application/octet-stream', 'Accept-Ranges': 'bytes',
                   'Cache-Control': 'private, no-store', 'X-Content-Type-Options': 'nosniff' };
  const send = (fn) => (latencyMs > 0 ? setTimeout(fn, latencyMs) : fn());
  if (req.method === 'HEAD') {
    ++stats.heads;
    send(() => res.writeHead(200, { ...common, 'Content-Length': size }).end());
    return true;
  }
  const m = /^bytes=(\d+)-(\d*)$/.exec(req.headers.range ?? '');
  if (!m) {  // the engine never asks for the whole image
    res.writeHead(200, { ...common, 'Content-Length': size });
    createReadStream(disc).pipe(res);
    return true;
  }
  const start = Number(m[1]), end = m[2] ? Math.min(Number(m[2]), size - 1) : size - 1;
  if (start >= size || end < start) {
    res.writeHead(416, { ...common, 'Content-Range': `bytes */${size}` }).end();
    return true;
  }
  ++stats.imageRequests;
  stats.imageBytes += end - start + 1;
  send(() => {
    res.writeHead(206, { ...common, 'Content-Length': end - start + 1, 'Content-Range': `bytes ${start}-${end}/${size}` });
    const stream = createReadStream(disc, { start, end, highWaterMark: 64 * 1024 });
    if (!(rateMBps > 0)) {
      stream.pipe(res);
      return;
    }
    // Throttled: pause after each chunk for as long as the rate says.
    stream.on('data', (chunk) => {
      stream.pause();
      res.write(chunk);
      setTimeout(() => stream.resume(), (chunk.length / (rateMBps * 1e6)) * 1000);
    });
    stream.on('end', () => res.end());
  });
  return true;
}

function sendFile(req, res, file, type) {
  const size = statSync(file).size;
  res.writeHead(200, { 'Content-Type': type, 'Content-Length': size, 'Cache-Control': 'private, no-store',
                       'X-Content-Type-Options': 'nosniff' });
  if (req.method === 'HEAD') res.end();
  else createReadStream(file).pipe(res);
}

const server = createServer((req, res) => {
  if (isolate) {  // P2: cross-origin isolation (SharedArrayBuffer, wasm threads); everything here is same-origin
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
    res.setHeader('Cross-Origin-Resource-Policy', 'same-origin');
  }
  if (req.method !== 'GET' && req.method !== 'HEAD') {
    res.writeHead(405, { Allow: 'GET, HEAD' }).end();
    return;
  }
  let path;
  try {
    path = decodeURIComponent(new URL(req.url ?? '/', 'http://localhost').pathname);
  } catch {
    res.writeHead(400).end();
    return;
  }
  if (disc && mock(req, res, path)) return;
  if (statesDir && path.startsWith('/states/')) {  // save-state files (--states)
    const name = path.slice('/states/'.length);
    const file = join(statesDir, basename(name));
    let ok = false;
    try {
      ok = basename(name) === name && name.endsWith('.mvpstate') && statSync(file).isFile();
    } catch {
      ok = false;
    }
    if (ok) sendFile(req, res, file, 'application/octet-stream');
    else res.writeHead(404, { 'Content-Type': 'text/plain' }).end('not found');
    return;
  }
  if (path === '/_stats') {  // the mock's counters, for the browser check
    res.writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store' }).end(JSON.stringify(stats));
    return;
  }
  if (path.endsWith('/')) path += 'index.html';
  const file = normalize(join(root, path));
  const type = types[extname(file).toLowerCase()];
  let st;
  try {
    st = file.startsWith(root + sep) && type ? statSync(file) : null;
  } catch {
    st = null;
  }
  if (!st || !st.isFile()) {
    res.writeHead(404, { 'Content-Type': 'text/plain' }).end('not found');
    return;
  }
  res.writeHead(200, {
    'Content-Type': type,
    'Content-Length': st.size,
    'Cache-Control': 'no-store',
    'X-Content-Type-Options': 'nosniff',
  });
  if (req.method === 'HEAD') res.end();
  else createReadStream(file).pipe(res);
});

server.listen(port, host, () => {
  const { port: p } = server.address();
  console.log(`serving http://${host === '0.0.0.0' ? '<host>' : host}:${p}/ from ${root}${disc ? ' (mock deployment: /config.json, /api/disc/image)' : ''}` +
              `${packsDir ? `, packs from ${packsDir}` : ''}${statesDir ? `, states from ${statesDir}` : ''}`);
});
