// Files (E5): the engine site (static, open) and the owner's disc image (for
// players only: src/access.ts; byte ranges, never cached by anyone).
import { createReadStream, promises as fsp } from 'node:fs';
import type { IncomingMessage, ServerResponse } from 'node:http';
import { extname, join, normalize, sep } from 'node:path';

const TYPES: Record<string, string> = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.mjs': 'text/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.wasm': 'application/wasm',
  '.txt': 'text/plain; charset=utf-8',
  '.json': 'application/json',
  '.png': 'image/png',
  '.svg': 'image/svg+xml',
  '.ico': 'image/x-icon',
};

/** Serves `urlPath` from `root`: GET and HEAD, files only, no listings, revalidated with an ETag. */
export async function serveStatic(root: string, urlPath: string, req: IncomingMessage, res: ServerResponse): Promise<void> {
  let path = urlPath;
  if (path.endsWith('/')) path += 'index.html';
  const file = normalize(join(root, path));
  const type = TYPES[extname(file).toLowerCase()];
  let st: Awaited<ReturnType<typeof fsp.stat>> | null = null;
  if (file.startsWith(root + sep) && type) {
    try {
      st = await fsp.stat(file);
    } catch {
      st = null;
    }
  }
  if (!st || !st.isFile()) {
    res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' }).end('not found');
    return;
  }
  const etag = `W/"${st.size.toString(16)}-${Math.floor(st.mtimeMs).toString(16)}"`;
  const headers = {
    'Content-Type': type!,
    'Cache-Control': 'private, no-cache',
    ETag: etag,
  };
  if (req.headers['if-none-match'] === etag) {
    res.writeHead(304, headers).end();
    return;
  }
  res.writeHead(200, { ...headers, 'Content-Length': st.size });
  if (req.method === 'HEAD') res.end();
  else createReadStream(file).pipe(res);
}

/** A single byte range of a `size`-byte resource, or 'none' (no Range header) or 'bad' (416). */
export function parseRange(header: string | undefined, size: number): { start: number; end: number } | 'none' | 'bad' {
  if (!header) return 'none';
  const m = /^bytes=(\d*)-(\d*)$/.exec(header.trim());
  if (!m || (m[1] === '' && m[2] === '')) return 'bad';  // also multiple ranges
  let start: number, end: number;
  if (m[1] === '') {
    const suffix = Number(m[2]);
    if (suffix === 0) return 'bad';
    start = Math.max(0, size - suffix);
    end = size - 1;
  } else {
    start = Number(m[1]);
    end = m[2] === '' ? size - 1 : Math.min(Number(m[2]), size - 1);
    if (Number(m[2]) < start && m[2] !== '') return 'bad';
  }
  if (!Number.isSafeInteger(start) || start >= size || end < start) return 'bad';
  return { start, end };
}

/**
 * The disc route: the configured image with Accept-Ranges, single byte ranges
 * (206) and Cache-Control "private, no-store" on every answer.
 */
export async function serveDisc(path: string, req: IncomingMessage, res: ServerResponse): Promise<void> {
  const base = { 'Cache-Control': 'private, no-store', 'Accept-Ranges': 'bytes' };
  let size: number;
  try {
    const st = await fsp.stat(path);
    if (!st.isFile()) throw new Error('not a file');
    size = st.size;
  } catch {
    res.writeHead(404, { ...base, 'Content-Type': 'text/plain; charset=utf-8' }).end('no disc image');
    return;
  }
  const range = parseRange(req.headers.range, size);
  if (range === 'bad') {
    res.writeHead(416, { ...base, 'Content-Range': `bytes */${size}` }).end();
    return;
  }
  const type = { 'Content-Type': 'application/octet-stream' };
  if (range === 'none') {
    res.writeHead(200, { ...base, ...type, 'Content-Length': size });
    if (req.method === 'HEAD') res.end();
    else createReadStream(path).pipe(res);
    return;
  }
  res.writeHead(206, {
    ...base,
    ...type,
    'Content-Length': range.end - range.start + 1,
    'Content-Range': `bytes ${range.start}-${range.end}/${size}`,
  });
  if (req.method === 'HEAD') res.end();
  else createReadStream(path, { start: range.start, end: range.end }).pipe(res);
}
