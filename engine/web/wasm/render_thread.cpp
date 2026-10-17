// The threaded engine's render thread (P2): see render_thread.h.
#include "web/wasm/render_thread.h"

#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>
#include <pthread.h>

#include <atomic>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

#include "gfx/gp_reads.h"
#include "gfx/render_ring.h"
#include "ppc/mem_access.h"
#include "web/wasm/render_side.h"

// ---- JavaScript glue ----

// On the render thread: take the canvas (or the page's frame port, then a
// canvas of its own of `w` x `h`) when the emulation thread posts it here,
// register it under `sel` (emdawnwebgpu finds canvases there) and tell the
// engine. Messages without a `cmd` are ignored by Emscripten's own handler.
EM_JS(void, mvp_js_render_listen, (const char* sel, int w, int h), {
  var s = UTF8ToString(sel);
  self.addEventListener('message', function(e) {
    var d = e.data;
    if (!d || !d.mvpRender) return;
    Module['specialHTMLTargets'][s] = d.canvas || new OffscreenCanvas(w, h);
    Module['mvpFrames'] = d.frames || null;
    Module['mvpVsync'] = d.vsync || null;
    Module['_mvp_render_canvas']();
  });
});

// On the emulation thread: hand the page's canvas (Module.mvpCanvas) or frame
// port (Module.mvpFrames) to the render thread's Worker. Returns 0 without
// either or without a thread.
EM_JS(int, mvp_js_render_send_canvas, (std::uintptr_t tid), {
  var w = PThread.pthreads[tid];
  var c = Module['mvpCanvas'], f = Module['mvpFrames'];
  if (!w || (!c && !f)) return 0;
  Module['mvpCanvas'] = null;
  Module['mvpFrames'] = null;
  var t = [];
  if (c) t.push(c);
  if (f) t.push(f);
  w.postMessage({ mvpRender: 1, canvas: c || null, frames: f || null, vsync: Module['mvpVsync'] || null }, t);
  return 1;
});

// On the render thread: run the task loop again in a new task, at once (a
// MessageChannel, not setTimeout: nested timeouts are clamped to 4 ms).
EM_JS(void, mvp_js_render_soon, (), {
  var c = Module['mvpRenderChan'];
  if (!c) {
    c = Module['mvpRenderChan'] = new MessageChannel();
    c.port1.onmessage = function() { Module['_mvp_render_task'](); };
  }
  c.port2.postMessage(0);
});

// On the render thread: the display's refresh count. A Worker started by a
// Worker (this pthread) has no requestAnimationFrame, so the page bumps a
// counter in shared memory at each of its animation frames (main.ts) and the
// render thread reads it. -1 without the counter.
EM_JS(int, mvp_js_render_vsync, (), {
  var v = Module['mvpVsyncView'];
  if (!v) {
    var sab = Module['mvpVsync'];
    if (!sab) return -1;
    v = Module['mvpVsyncView'] = new Int32Array(sab);
  }
  return Atomics.load(v, 0) & 0x3FFFFFFF;
});

namespace mvp::web {

namespace {

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Record types in the ring.
enum : u32 {
  kGx = 1,      // a GX command's bytes
  kGxRef,       // {pointer, length}: a command too large for the ring (the emulation waits until it is done)
  kField,       // {field, xfb}
  kPresent,     // {frame polls}
  kResize,      // {width, height}
  kMemory,      // {pointer, size}: guest memory
  kTrace,       // {on}
  kDigest,      // {on}
  kFlush,
  kGpActivate,  // Q1: {addr, len}: the game wrote these lines back from the cache (gfx/gp_memory.h)
  kGpLines,     // Q1: {first line, lines} + their contents as written back (lines that changed)
  kGpDrop,      // Q1: {addr, len}: memory written without the cache
  kScale,       // Q1: {internal resolution}
  kGxBatch,     // Q4: {count n, n lengths, the n commands' bytes one after the other}
  kGpuLoad,     // Q5 (development): {passes}
  kShadow,      // Q6: {addr, len} + memory there: a copy of what the next commands read (GpMemory::shadow)
  kShadowEnd,   // Q6: {addr, len}: the guest writes there next; copies there end
};

constexpr u32 kRingBytes = 16u << 20;
// Tuning (mvp_render_tune): records become visible in batches of kPublishBytes;
// at a fence the emulation polls kFenceSpinUs before it sleeps; while a batch
// runs the render thread polls kSpinMs for records before it sleeps.
u32 kPublishBytes = 4u << 10;
double kFenceSpinUs = 300;
double kSpinMs = 1.0;
// The guest's GPU sync points (draw done, tokens, ...) as fences too. Off: the
// writes to memory the render thread still reads wait instead (GpReads), which
// covers every case the fences cover.
bool kSyncFences = false;
// Q6: what commands read of vertex arrays, XF loads, palettes and preloads goes
// into the ring as a copy (GpMemory::shadow) instead of making the guest's next
// write there wait (render_tune's `copies`; off: they are marked, as before).
bool kCopies = true;
std::atomic<double> g_host_slow{1.0};  // development (Q2): render_host_slow
constexpr double kVsyncPollMs = 1.0;      // with a frame due, it looks at the refresh count this often
constexpr double kVsyncStallMs = 50.0;    // a frame due this long shows even if the count did not move

struct Field {
  u64 field;
  u32 xfb, pad;
};
struct Ref {
  const u8* p;
  u32 n;
};
struct Pair {
  u32 a, b;
};

void render_task_cb(void*);
void mark(u32 addr, u32 size, gfx::ReadKind kind);

struct Shared {
  gfx::RenderRing ring{kRingBytes};
  pthread_t tid{};
  void (*on_gfx)(int, const char*) = nullptr;
  std::string selector;
  int width = 0, height = 0;

  // Between the threads.
  std::atomic<u32> idle{0};        // the render thread waits in its event loop for a kick
  std::atomic<u32> emu_active{0};  // a run batch is producing records
  std::atomic<int> gfx_state{0};   // 0 starting, 1 ready, 2 failed
  std::atomic<u64> presents{0};
  std::mutex mu;                   // guards the snapshot strings
  std::string trace, digest, status, gfx_message;

  // Render thread only.
  RenderSide side;
  bool have_canvas = false, gpu_done = false;
  bool vsync = false;       // present once per display refresh (the page's count), else after each batch
  int shown_at = -1;        // the refresh count at the last present
  bool frame_due = false;   // a batch ended since the last present
  u64 due_polls = 0;
  double due_at = 0;        // when the frame became due
  u64 tasks = 0, held = 0;  // tasks run; presents held for the next refresh
  double busy_ms = 0;       // time spent working (not waiting for records)

  // Emulation thread only.
  // The memory each command reads (gfx/gp_reads.h), marked in ppc::g_gp_pages
  // with the ring position after the command (+1); the pages marked so far.
  gfx::GpReads reads{[](u32 addr, u32 size, gfx::ReadKind kind) { mark(addr, size, kind); }};
  std::vector<u32> marked;
  std::vector<u8> kinds = std::vector<u8>(ppc::kGpEntries);  // what each entry was last marked for (statistics)
  u64 write_waits = 0, write_checks = 0;
  double write_wait_ms = 0;
  u64 kind_waits[gfx::kReadKinds] = {};  // the write waits by what they waited for, and their time
  double kind_ms[gfx::kReadKinds] = {};
  // Q1: the render thread's view of what the GP sees (gfx/gp_memory.h), mirrored
  // here: which lines a cache write-back made the GP read from its snapshot this
  // field, and what each line's snapshot holds there (only changed lines travel).
  const u8* gp_live = nullptr;
  u32 gp_lines = 0, gp_gen = 2;
  std::vector<u8> gp_sent;
  std::vector<u32> gp_gen_of, gp_page_gen;
  u64 gp_flushes = 0, gp_sent_bytes = 0, gp_flushed_bytes = 0, gp_unmarked = 0;
  u64 mark_entries = 0;  // Q4: entries marked (statistics)
  // Q6: the ranges the commands being appended read (GpReads, through mark()):
  // copies go into the ring before the commands, marks after them (a mark's
  // epoch must include the commands' records).
  struct Read {
    u32 addr, size;
    gfx::ReadKind kind;
  };
  std::vector<Read> reads_now, reads_later;
  // Per entry that became a copy while an earlier command still read memory
  // there: that command's mark (the guest's next write waits for it), else 0.
  std::vector<u32> kept = std::vector<u32>(ppc::kGpEntries);
  std::vector<u32> kept_at;  // the entries with one (the sweep forgets the finished)
  u64 copies = 0, copy_bytes = 0, copy_ends = 0;  // statistics
  u32 synced = 0;           // ring position of the last fence
  u32 present_pos[2] = {};  // ring positions after the last two presents
  u64 fences = 0, fence_waits = 0, lag_waits = 0, big = 0, resizes = 0;
  u64 fence_hist[4] = {};   // fence waits under 20 us, 100 us, 500 us, longer
  double fence_hist_ms[4] = {}, fence_behind_kb[4] = {};  // their time, and the ring bytes not finished at their start
  double fence_ms = 0, lag_ms = 0;
};

Shared* S = nullptr;

// Q4: what a marked entry holds: the epoch of the commands that read it (the
// ring's publishes before them), with the top bit set (never 0). The epoch ends at
// the next publish, which makes every one of its commands visible: a write waits
// until the render thread has finished the records up to there, which includes
// the command that marked the entry. GpReads reports a vertex range once per
// epoch (gfx/gp_reads.h); with a ring position per command instead (P2), every
// primitive had to mark again.
constexpr u32 kTagBit = 0x80000000u;
u32 current_tag() { return S->ring.publishes() | kTagBit; }
// The ring position after every command an entry's tag stands for.
u32 tag_end(u32 tag) {
  const u32 epoch = tag & ~kTagBit, now = S->ring.publishes() & ~kTagBit;
  const u32 age = (now - epoch) & ~kTagBit;
  if (age == 0 || age >= gfx::RenderRing::kPubHistory) return S->ring.end();  // still open (or long gone): all so far
  return S->ring.published_at(epoch + 1);
}

// Q6: an entry whose memory the render thread has a copy of (GpMemory::shadow):
// the guest's next write there ends the copy (gp_wait). Marks hold an epoch
// with kTagBit set, so this is never one.
constexpr u32 kCopyTag = 1;

// Emulation thread: the command being appended reads [addr, addr + size)
// (reads_copy and reads_mark deal with it).
void mark(u32 addr, u32 size, gfx::ReadKind kind) { S->reads_now.push_back({addr, size, kind}); }

// Marks [addr, addr + size) as read by the commands just appended.
bool gp_snapshot_entry(u32 i);
bool gp_page_snapshot(u32 entry);
void mark_now(u32 addr, u32 size, gfx::ReadKind kind) {
  const u32 first = addr >> ppc::kGpShift, last = (addr + size - 1) >> ppc::kGpShift;
  const u32 tag = current_tag();
  S->mark_entries += last - first + 1;
  constexpr u32 kPerPage = 4096u >> ppc::kGpShift;  // entries per 4 KiB page
  for (u32 p = first; p <= last;) {
    const u32 stop = std::min(last, p | (kPerPage - 1));
    // Q1: lines a cache write-back snapshot covers this field are read from the
    // snapshot (it came through the ring), not from memory: guest writes there
    // need not wait. (Q4: looked at entry by entry only in 4 KiB pages that
    // have a snapshot this field.)
    const bool snapshots = gp_page_snapshot(p & (ppc::kGpEntries - 1));
    for (; p <= stop; ++p) {
      const u32 i = p & (ppc::kGpEntries - 1);
      if (snapshots && gp_snapshot_entry(i)) {
        ++S->gp_unmarked;
        continue;
      }
      std::uint32_t& e = ppc::g_gp_pages[i];
      if (e == kCopyTag) continue;  // Q6: the render thread reads the copy (every read goes through GpMemory::view)
      if (!e) S->marked.push_back(i);
      e = tag;
      S->kinds[i] = static_cast<u8>(kind);
    }
  }
}

// Emulation thread: a guest write to [first, last] (ppc/mem_access.h, the
// translated stores, the hardware's DMA): wait until the render thread has
// finished every command that reads those pages (Q6: where it has a copy, end
// the copy instead, before the write).
void gp_wait(u32 first, u32 last) {
  ++S->write_checks;
  bool copies = false;
  for (u32 p = first >> ppc::kGpShift; p <= (last >> ppc::kGpShift); ++p) {
    const u32 i = p & (ppc::kGpEntries - 1);
    std::uint32_t& e = ppc::g_gp_pages[i];
    if (!e) continue;
    if (e == kCopyTag) {  // Q6: the render thread has a copy: it ends here, before the write
      copies = true;
      S->reads.forget();
      // An earlier command that reads memory there (marked when the copy was
      // taken): the write waits for it, as below.
      e = S->kept[i];
      S->kept[i] = 0;
      if (!e) continue;
    }
    const u32 pos = tag_end(e);
    if (!S->ring.finished(pos)) {
      if (S->idle.load() == 1 && S->idle.exchange(0) == 1)
        emscripten_proxy_async(emscripten_proxy_get_system_queue(), S->tid, render_task_cb, nullptr);
      const double ms = S->ring.wait_read(pos, kFenceSpinUs);
      S->write_wait_ms += ms;
      ++S->write_waits;
      ++S->kind_waits[S->kinds[i] % gfx::kReadKinds];
      S->kind_ms[S->kinds[i] % gfx::kReadKinds] += ms;
    }
    e = 0;
    S->reads.forget();  // Q4: an entry it may count as marked is not any more
  }
  if (copies) {
    const u32 a = (first & 0x01FFFFFFu) & ~((1u << ppc::kGpShift) - 1);
    const u32 len = (((last - first) + (first & ((1u << ppc::kGpShift) - 1))) | ((1u << ppc::kGpShift) - 1)) + 1;
    u8* d = S->ring.reserve(kShadowEnd, 8);
    std::memcpy(d, &a, 4);
    std::memcpy(d + 4, &len, 4);
    S->ring.commit();
    ++S->copy_ends;
  }
}

// Emulation thread, at each VI field: forget the pages whose commands are done.
void sweep() {
  std::size_t n = 0;
  for (u32 p : S->marked) {
    std::uint32_t& e = ppc::g_gp_pages[p];
    if (e == kCopyTag) continue;  // Q6: a copy now (until the guest writes there)
    if (e && S->ring.finished(tag_end(e))) e = 0;
    if (e) S->marked[n++] = p;
  }
  if (n != S->marked.size()) S->reads.forget();  // Q4: entries it may count as marked are not any more
  S->marked.resize(n);
  n = 0;  // Q6: the marks copies keep
  for (u32 p : S->kept_at) {
    u32& h = S->kept[p];
    if (h && S->ring.finished(tag_end(h))) h = 0;
    if (h) S->kept_at[n++] = p;
  }
  S->kept_at.resize(n);
}

// ---- Q1: the GP's view of memory (gfx/gp_memory.h), the emulation side ----

constexpr u32 kLine = 32, kLineShift = 5, kLinesPerPage = 128;  // 32-byte lines, 4 KiB pages

bool gp_line_active(u32 line) { return line < S->gp_lines && S->gp_gen_of[line] == S->gp_gen; }

// Q4: whether the 4 KiB page of entry `i` has lines snapshot this field (else none of its entries is).
bool gp_page_snapshot(u32 i) {
  if (!S->gp_lines) return false;
  const u32 l0 = (i << ppc::kGpShift) >> kLineShift;
  return l0 < S->gp_lines && S->gp_page_gen[l0 / kLinesPerPage] == S->gp_gen;
}

// A 2^kGpShift (64) byte entry of ppc::g_gp_pages whose lines are all snapshot this field.
bool gp_snapshot_entry(u32 i) {
  if (!S->gp_lines) return false;
  const u32 l0 = (i << ppc::kGpShift) >> kLineShift, l1 = (((i + 1) << ppc::kGpShift) >> kLineShift) - 1;
  if (l1 >= S->gp_lines || S->gp_page_gen[l0 / kLinesPerPage] != S->gp_gen) return false;
  for (u32 l = l0; l <= l1; ++l)
    if (!gp_line_active(l)) return false;
  return true;
}

void gp_attach(const u8* mem, u32 size) {
  S->gp_live = mem;
  S->gp_lines = size >> kLineShift;
  S->gp_sent.assign(static_cast<std::size_t>(S->gp_lines) * kLine, 0);  // the render side's snapshots start as zeros too
  S->gp_gen_of.assign(S->gp_lines, 0);
  S->gp_page_gen.assign(S->gp_lines / kLinesPerPage + 1, 0);
}

// Sends lines [first, first + n) (their contents now) to the render thread.
void gp_send_lines(u32 first, u32 n) {
  const u32 max_lines = 16384;  // 512 KiB per record
  while (n) {
    const u32 k = std::min(n, max_lines);
    u8* d = S->ring.reserve(kGpLines, 8 + k * kLine);
    std::memcpy(d, &first, 4);
    std::memcpy(d + 4, &k, 4);
    std::memcpy(d + 8, S->gp_live + static_cast<std::size_t>(first) * kLine, static_cast<std::size_t>(k) * kLine);
    S->ring.commit();
    S->gp_sent_bytes += static_cast<u64>(k) * kLine;
    first += k;
    n -= k;
  }
}

template <class T>
void put(u32 type, const T& v) {
  std::memcpy(S->ring.reserve(type, sizeof v), &v, sizeof v);
  S->ring.commit();
}

// Emulation thread: wake the render thread if it waits in its event loop (with
// nothing due, between batches).
void kick() {
  if (S->idle.load(std::memory_order_seq_cst) == 1 && S->idle.exchange(0) == 1)
    emscripten_proxy_async(emscripten_proxy_get_system_queue(), S->tid, render_task_cb, nullptr);
}

void publish_and_kick() {
  S->ring.publish();
  kick();
}

bool disabled() { return S->gfx_state.load(std::memory_order_relaxed) == 2; }

std::string json_escape(const std::string& in) {
  std::string o;
  for (char ch : in) {
    if (ch == '"' || ch == '\\') o += '\\', o += ch;
    else if (static_cast<unsigned char>(ch) < 0x20) o += ' ';
    else o += ch;
  }
  return o;
}

// ---- render thread ----

void snapshot() {
  std::string t = S->side.trace_take_raw(), d = S->side.digest_take(), s = S->side.status_json();
  char b[96];
  std::snprintf(b, sizeof b, ",\"render\":{\"tasks\":%llu,\"held\":%llu,\"vsync\":%s,\"busy_ms\":%.1f}",
                static_cast<unsigned long long>(S->tasks), static_cast<unsigned long long>(S->held),
                S->vsync ? "true" : "false", S->busy_ms);
  s += b;
  std::lock_guard<std::mutex> lock(S->mu);
  S->trace += t;
  S->digest += d;
  S->status = std::move(s);
}

// Runs on the emulation thread: the renderer is ready (or failed).
void gfx_done_main(void*) {
  std::string msg;
  {
    std::lock_guard<std::mutex> lock(S->mu);
    msg = S->gfx_message;
  }
  if (S->on_gfx) S->on_gfx(S->gfx_state.load() == 1 ? 1 : 0, msg.c_str());
}

// The renderer is ready or failed (once; from either thread).
void gfx_result(bool ok, const std::string& msg) {
  int expected = 0;
  {
    std::lock_guard<std::mutex> lock(S->mu);
    if (!S->gfx_state.compare_exchange_strong(expected, ok ? 1 : 2)) return;
    S->gfx_message = msg;
    if (!ok) S->status = ",\"gfx_error\":\"" + json_escape(msg) + "\"";
  }
  if (emscripten_is_main_runtime_thread()) gfx_done_main(nullptr);
  else
    emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_runtime_thread_id(), gfx_done_main,
                           nullptr);
}

// Both the device and the canvas are here: the surface, then the renderer.
void maybe_ready() {
  if (!S->gpu_done || !S->have_canvas || S->gfx_state.load() != 0) return;
  if (!S->side.gpu) {
    gfx_result(false, S->side.error);
    return;
  }
  std::string err;
  S->side.window = gfx::Window::open_canvas(S->selector.c_str(), S->width, S->height, &err);
  if (!S->side.window || !S->side.window->attach(*S->side.gpu, &err)) {
    S->side.error = err;
    S->side.window.reset();
    gfx_result(false, err);
    return;
  }
  S->side.attach();
  S->vsync = mvp_js_render_vsync() >= 0;
  gfx_result(true, S->side.gpu->adapter_name());
}

// Shows the latest batch's picture.
void show() {
  S->frame_due = false;
  if (S->side.present(S->due_polls, emscripten_get_now())) S->presents.fetch_add(1);
  snapshot();
}

// A frame is due and may be shown now: the first in this display refresh (a
// second one waits for the next refresh and shows the latest batch then, so the
// canvas changes once per refresh), or at once without the page's refresh count.
// If the count stops (a hidden page has no animation frames), the frame shows
// after kVsyncStallMs anyway.
bool may_show() {
  if (!S->frame_due) return false;
  if (!S->vsync) return true;
  const int v = mvp_js_render_vsync();
  if (v == S->shown_at && emscripten_get_now() - S->due_at < kVsyncStallMs) return false;
  S->shown_at = v;
  return true;
}

// Handles one record.
void handle(const gfx::RenderRing::Record& r) {
  switch (r.type) {
    case kGx:
      S->side.gx(r.data, r.len);
      return;
    case kGxRef: {
      Ref v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.gx(v.p, v.n);
      return;
    }
    case kGxBatch: {
      u32 n;
      std::memcpy(&n, r.data, 4);
      const u8* lens = r.data + 4;
      const u8* p = lens + 4 * static_cast<std::size_t>(n);
      for (u32 k = 0; k < n; ++k) {
        u32 len;
        std::memcpy(&len, lens + 4 * k, 4);
        S->side.gx(p, len);
        p += len;
      }
      return;
    }
    case kField: {
      Field v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.vi_field(v.field, v.xfb);
      return;
    }
    case kPresent: {
      if (S->frame_due) ++S->held;  // the previous batch's frame was not shown: this one replaces it
      std::memcpy(&S->due_polls, r.data, sizeof S->due_polls);
      if (!S->frame_due) S->due_at = emscripten_get_now();
      S->frame_due = true;
      return;
    }
    case kResize: {
      Pair v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.resize(static_cast<int>(v.a), static_cast<int>(v.b));
      return;
    }
    case kMemory: {
      Ref v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.set_memory(v.p, v.n);
      return;
    }
    case kTrace: {
      u32 on;
      std::memcpy(&on, r.data, sizeof on);
      S->side.set_trace(on != 0);
      return;
    }
    case kDigest: {
      u64 v[3];
      std::memcpy(v, r.data, sizeof v);
      S->side.set_digest(v[0] != 0, v[1], v[2]);
      return;
    }
    case kFlush:
      S->side.flush();
      snapshot();
      return;
    case kGpActivate: {
      Pair v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.gp_activate(v.a, v.b);
      return;
    }
    case kGpLines: {
      Pair v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.gp_set_lines(v.a, v.b, r.data + sizeof v);
      return;
    }
    case kScale: {
      u32 v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.set_scale(static_cast<int>(v));
      return;
    }
    case kGpuLoad: {
      u32 v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.set_gpu_load(static_cast<int>(v));
      return;
    }
    case kGpDrop: {
      Pair v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.gp_write_through(v.a, v.b);
      return;
    }
    case kShadow: {
      Pair v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.gp_shadow(v.a, v.b, r.data + sizeof v);
      return;
    }
    case kShadowEnd: {
      Pair v;
      std::memcpy(&v, r.data, sizeof v);
      S->side.gp_end_shadow(v.a, v.b);
      return;
    }
    default:
      return;
  }
}

// Polls up to kSpinMs for records while the emulation produces them.
bool spin_for_data() {
  const double t0 = emscripten_get_now();
  for (unsigned n = 1;; ++n) {
    if (S->ring.has_data()) return true;
    if (!S->emu_active.load(std::memory_order_relaxed)) break;
    if (n % 64 == 0 && emscripten_get_now() - t0 >= kSpinMs) break;
  }
  return S->ring.has_data();
}

// The render thread's loop. It waits for records in place (Atomics.wait: the
// emulation's fences need it awake at once). With the page's refresh count it
// presents at most once per refresh: a frame due while records keep coming is
// shown as soon as the count moves. Each present ends the task and the loop
// continues in the next one: a frame drawn into the page's own canvas reaches
// the screen only between tasks, and the thread's event loop gets a turn (the
// page's frame port does not need it: the ImageBitmap is posted at once).
// Between batches, with nothing due, it waits in its event loop until the
// emulation kicks it.
void render_task() {
  ++S->tasks;
  S->idle.store(0, std::memory_order_seq_cst);
  double busy_from = emscripten_get_now();  // the thread's working time (statistics), between waits
  auto rest = [&] {
    const double now = emscripten_get_now(), work = now - busy_from;
    S->busy_ms += work;
    const double slow = g_host_slow.load(std::memory_order_relaxed);
    if (slow > 1)  // development: a slower machine (render_host_slow)
      while (emscripten_get_now() - now < (slow - 1) * work) {
      }
  };
  unsigned records = 0;
  for (;;) {
    // A due frame: the refresh count (a call into JavaScript) is read every 32
    // records, and whenever the ring is empty.
    if (S->frame_due && ((++records & 31) == 0 || !S->ring.has_data()) && may_show()) {
      show();
      rest();
      mvp_js_render_soon();
      return;
    }
    gfx::RenderRing::Record r;
    if (S->ring.next(&r)) {
      handle(r);
      S->ring.done();
      continue;
    }
    rest();
    const bool active = S->emu_active.load(std::memory_order_seq_cst) != 0;
    if (active && spin_for_data()) {
      busy_from = emscripten_get_now();
      continue;
    }
    if (active || S->frame_due) {
      S->ring.wait_data(S->frame_due ? kVsyncPollMs : 2.0);
      busy_from = emscripten_get_now();
      continue;
    }
    S->idle.store(1, std::memory_order_seq_cst);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    if ((S->ring.has_data() || S->emu_active.load(std::memory_order_seq_cst)) && S->idle.exchange(0) == 1) {
      busy_from = emscripten_get_now();
      continue;
    }
    return;
  }
}

void render_task_cb(void*) { render_task(); }

void* render_main(void*) {
  mvp_js_render_listen(S->selector.c_str(), S->width, S->height);
  // The canvas: the emulation thread posts it to this Worker when asked.
  emscripten_proxy_async(
      emscripten_proxy_get_system_queue(), emscripten_main_runtime_thread_id(),
      [](void*) {
        if (!mvp_js_render_send_canvas(reinterpret_cast<std::uintptr_t>(S->tid))) gfx_result(false, "no canvas");
      },
      nullptr);
  gfx::Gpu::create_async([](std::unique_ptr<gfx::Gpu> gpu, std::string err) {
    if (!gpu) S->side.error = err;
    S->side.gpu = std::move(gpu);
    S->gpu_done = true;
    if (!S->side.gpu) gfx_result(false, S->side.error);
    else maybe_ready();
  });
  render_task();
  emscripten_exit_with_live_runtime();  // keep the thread for its event loop
}

}  // namespace

// ---- emulation thread ----

bool render_thread_start(const char* selector, int width, int height, void (*on_gfx)(int, const char*)) {
  if (S) return true;
  S = new Shared();
  S->selector = selector;
  S->width = width;
  S->height = height;
  S->on_gfx = on_gfx;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 2u << 20);
  const int rc = pthread_create(&S->tid, &attr, render_main, nullptr);
  pthread_attr_destroy(&attr);
  if (rc != 0) {
    delete S;
    S = nullptr;
    return false;
  }
  ppc::g_gp_wait_hook = gp_wait;  // guest writes to memory the render thread still reads wait for it
  return true;
}

bool render_thread_running() { return S != nullptr; }

void render_tune(double fence_spin_us, double render_spin_ms, int publish_kib, int sync_fences, int copies) {
  if (copies >= 0) kCopies = copies != 0;
  if (sync_fences >= 0) kSyncFences = sync_fences != 0;
  if (fence_spin_us >= 0) kFenceSpinUs = fence_spin_us;
  if (render_spin_ms >= 0) kSpinMs = render_spin_ms;
  if (publish_kib > 0) kPublishBytes = static_cast<u32>(publish_kib) << 10;
}

void render_host_slow(double factor) { g_host_slow.store(factor > 1 ? factor : 1.0); }

double render_waited_ms() { return S ? S->write_wait_ms + S->lag_ms + S->fence_ms : 0; }

void render_memory(const std::uint8_t* mem, std::uint32_t size) {
  gp_attach(mem, size);
  put(kMemory, Ref{mem, size});
  publish_and_kick();
}

// Q1: the game wrote the cache's lines over [ea, ea + len) back. The render
// thread gets the range and the contents of the lines that differ from what
// their snapshots hold there (in the front end most of a flushed array is
// unchanged from one flush to the next).
void render_gp_flush(std::uint32_t ea, std::uint32_t len) {
  if (disabled() || !S->gp_lines || !len) return;
  const u32 addr = ea & 0x01FFFFFFu;
  if (addr >= S->gp_lines * kLine) return;
  const u32 first = addr >> kLineShift, last = std::min((addr + len - 1) >> kLineShift, S->gp_lines - 1);
  u32 run = 0, run_n = 0;
  for (u32 l = first; l <= last; ++l) {
    u8* sent = S->gp_sent.data() + static_cast<std::size_t>(l) * kLine;
    const u8* now = S->gp_live + static_cast<std::size_t>(l) * kLine;
    if (std::memcmp(sent, now, kLine) != 0) {
      std::memcpy(sent, now, kLine);
      if (!run_n) run = l;
      ++run_n;
      continue;
    }
    if (run_n) gp_send_lines(run, run_n);
    run_n = 0;
  }
  if (run_n) gp_send_lines(run, run_n);
  for (u32 l = first; l <= last; ++l) S->gp_gen_of[l] = S->gp_gen;
  for (u32 p = first / kLinesPerPage; p <= last / kLinesPerPage; ++p) S->gp_page_gen[p] = S->gp_gen;
  put(kGpActivate, Pair{first * kLine, (last - first + 1) * kLine});
  ++S->gp_flushes;
  S->gp_flushed_bytes += static_cast<u64>(last - first + 1) * kLine;
  S->ring.publish_if(kPublishBytes);
}

// Q1: memory over [addr, addr + len) was written without the cache: snapshots
// there end (a record only when there are some).
void render_gp_write_through(std::uint32_t addr, std::uint32_t len) {
  if (disabled() || !S->gp_lines || !len) return;
  addr &= 0x01FFFFFFu;
  if (addr >= S->gp_lines * kLine) return;
  const u32 first = addr >> kLineShift, last = std::min((addr + len - 1) >> kLineShift, S->gp_lines - 1);
  bool any = false;
  for (u32 p = first / kLinesPerPage; p <= last / kLinesPerPage; ++p) {
    if (S->gp_page_gen[p] != S->gp_gen) continue;
    const u32 l0 = std::max(first, p * kLinesPerPage), l1 = std::min(last, p * kLinesPerPage + kLinesPerPage - 1);
    for (u32 l = l0; l <= l1; ++l)
      if (S->gp_gen_of[l] == S->gp_gen) {
        S->gp_gen_of[l] = 0;
        any = true;
      }
  }
  if (any) {
    put(kGpDrop, Pair{addr, len});
    S->reads.forget();  // Q4: what it counts as reported may include those lines, unmarked
  }
}

// Q6: entries [p0, p0 + n) to the render thread as a copy of memory (the caller
// took entries where that is what the GP sees). A mark an entry had (an earlier
// command still reads memory there) is kept in S->kept.
void copy_entries(u32 p0, u32 n) {
  constexpr u32 kMax = 1u << 20;  // bytes per record
  while (n) {
    const u32 k = std::min(n, kMax >> ppc::kGpShift);
    const u32 addr = p0 << ppc::kGpShift, len = k << ppc::kGpShift;
    u8* d = S->ring.reserve(kShadow, 8 + len);
    std::memcpy(d, &addr, 4);
    std::memcpy(d + 4, &len, 4);
    std::memcpy(d + 8, S->gp_live + addr, len);
    S->ring.commit();
    for (u32 p = p0; p < p0 + k; ++p) {
      std::uint32_t& e = ppc::g_gp_pages[p];
      S->kept[p] = e && !S->ring.finished(tag_end(e)) ? e : 0;
      if (S->kept[p]) S->kept_at.push_back(p);
      e = kCopyTag;
    }
    ++S->copies;
    S->copy_bytes += len;
    p0 += k;
    n -= k;
  }
}

// Q6: whether line `l`'s snapshot (Q1) holds what memory holds there.
bool gp_line_current(u32 l) {
  return std::memcmp(S->gp_sent.data() + static_cast<std::size_t>(l) * kLine,
                     S->gp_live + static_cast<std::size_t>(l) * kLine, kLine) == 0;
}

// Q6: before the commands that read S->reads_now go into the ring: copies of
// what they read of vertex arrays, XF loads, palettes and preloads, entry by
// entry, where the GP sees memory as it is (no snapshot over the entry, or one
// that memory still matches: a copy taken now stays what the GP sees until the
// guest writes there, whatever later flushes and fields do to the snapshot);
// the rest is marked after them (reads_mark).
void reads_copy() {
  S->reads_later.clear();
  for (const Shared::Read& r : S->reads_now) {
    const u32 addr = r.addr & 0x01FFFFFFu;
    const bool copyable = kCopies && S->gp_lines && r.size && r.kind != gfx::ReadKind::kCopy &&
                          r.kind != gfx::ReadKind::kTexture && addr + r.size <= S->gp_lines * kLine;
    if (!copyable) {
      S->reads_later.push_back(r);
      continue;
    }
    const u32 first = addr >> ppc::kGpShift, last = (addr + r.size - 1) >> ppc::kGpShift;
    u32 run = 0, runs = 0;  // entries to copy
    for (u32 p = first; p <= last; ++p) {
      std::uint32_t& e = ppc::g_gp_pages[p];
      bool copy = false;
      if (e == kCopyTag) {
        // copied already
      } else {
        const u32 l0 = (p << ppc::kGpShift) >> kLineShift;
        const bool a0 = gp_line_active(l0), a1 = gp_line_active(l0 + 1);
        if (a0 && a1) ++S->gp_unmarked;  // the snapshot came through the ring (as mark_now)
        else if ((a0 && !gp_line_current(l0)) || (a1 && !gp_line_current(l0 + 1)))
          S->reads_later.push_back({p << ppc::kGpShift, 1u << ppc::kGpShift, r.kind});  // a snapshot memory moved on from
        else copy = true;
      }
      if (copy) {
        if (runs && run + runs == p) {
          ++runs;
        } else {
          if (runs) copy_entries(run, runs);
          run = p;
          runs = 1;
        }
      }
    }
    if (runs) copy_entries(run, runs);
  }
}

// Q6: after the commands went into the ring: the marks reads_copy left.
void reads_mark() {
  for (const Shared::Read& r : S->reads_later) mark_now(r.addr, r.size, r.kind);
  S->reads_now.clear();
  S->reads_later.clear();
}

void render_gx(const std::uint8_t* cmd, std::size_t len) {
  if (disabled()) return;
  const u32 n = static_cast<u32>(len);
  S->reads.set_epoch(S->ring.publishes());
  S->reads.command(cmd, n);  // what it reads, so that guest writes there wait for it (Q6: or copies)
  reads_copy();
  if (n <= S->ring.max_payload()) {
    std::memcpy(S->ring.reserve(kGx, n), cmd, n);
    S->ring.commit();
    reads_mark();
    S->ring.publish_if(kPublishBytes);
    return;
  }
  // Too large to copy (a primitive of several MB): the render thread reads it
  // where it is, and the emulation waits until it has.
  put(kGxRef, Ref{cmd, n});
  ++S->big;
  reads_mark();
  kick();
  S->fence_ms += S->ring.wait_read(S->ring.end());
  S->synced = S->ring.end();
}

// Q4: one record for the commands of a push (usually the few commands of a
// 32-byte burst), instead of one each.
void render_gx_batch(const rt::GxCommand* cmds, std::size_t n) {
  if (disabled() || !n) return;
  std::size_t total = 4 + 4 * n;
  for (std::size_t k = 0; k < n; ++k) total += cmds[k].len;
  if (total > S->ring.max_payload()) {  // a large command among them: one by one
    for (std::size_t k = 0; k < n; ++k) render_gx(cmds[k].p, cmds[k].len);
    return;
  }
  S->reads.set_epoch(S->ring.publishes());
  for (std::size_t k = 0; k < n; ++k) S->reads.command(cmds[k].p, cmds[k].len);
  reads_copy();  // Q6: copies before the commands
  u8* d = S->ring.reserve(kGxBatch, static_cast<u32>(total));
  const u32 count = static_cast<u32>(n);
  std::memcpy(d, &count, 4);
  u8* lens = d + 4;
  u8* out = lens + 4 * n;
  const u8* run = cmds[0].p;  // consecutive commands (a burst's) are copied at once
  std::size_t run_len = 0;
  for (std::size_t k = 0; k < n; ++k) {
    std::memcpy(lens + 4 * k, &cmds[k].len, 4);
    if (cmds[k].p != run + run_len) {
      std::memcpy(out, run, run_len);
      out += run_len;
      run = cmds[k].p;
      run_len = 0;
    }
    run_len += cmds[k].len;
  }
  std::memcpy(out, run, run_len);
  S->ring.commit();
  reads_mark();
  S->ring.publish_if(kPublishBytes);
}

void render_field(std::uint64_t field, std::uint32_t xfb) {
  if (disabled()) return;
  ++S->gp_gen;  // Q1: as the render side's GpMemory::field() at this record
  put(kField, Field{field, xfb, 0});
  S->ring.publish();
  sweep();
}

void render_sync() {
  const u32 end = S->ring.end();
  if (end == S->synced || !kSyncFences) return;  // nothing new since the last fence
  S->synced = end;
  ++S->fences;
  if (S->ring.finished(end)) return;
  const u32 behind = S->ring.behind(end);
  kick();
  const double ms = S->ring.wait_read(end, kFenceSpinUs);
  S->fence_ms += ms;
  ++S->fence_waits;
  const int h = ms < 0.02 ? 0 : ms < 0.1 ? 1 : ms < 0.5 ? 2 : 3;
  ++S->fence_hist[h];
  S->fence_behind_kb[h] += behind / 1024.0;
  S->fence_hist_ms[h] += ms;
}

void render_batch_begin() {
  // At most two batches in flight: the render thread must have finished the
  // batch before the last one.
  const u32 pos = S->present_pos[0];
  if (!S->ring.finished(pos)) {
    kick();
    S->lag_ms += S->ring.wait_read(pos);
    ++S->lag_waits;
  }
  S->emu_active.store(1, std::memory_order_seq_cst);
  kick();
}

bool render_present(std::uint64_t frame_polls) {
  if (!disabled()) {
    put(kPresent, frame_polls);
    S->present_pos[0] = S->present_pos[1];
    S->present_pos[1] = S->ring.end();
  }
  S->ring.publish();
  S->emu_active.store(0, std::memory_order_seq_cst);
  kick();
  return S->presents.load() > 0;
}

// Q5: the batch ends with no new field to show (a netplay run that waited at a
// frame poll before it finished one): the render thread stops looking for
// records, and nothing is presented again.
void render_batch_end() {
  S->ring.publish();
  S->emu_active.store(0, std::memory_order_seq_cst);
  kick();
}

void render_flush() {
  put(kFlush, u32{0});
  publish_and_kick();
}

void render_resize(int width, int height) {
  ++S->resizes;
  put(kResize, Pair{static_cast<u32>(width), static_cast<u32>(height)});
  publish_and_kick();
}

void render_scale(int scale) {
  put(kScale, static_cast<u32>(scale));
  publish_and_kick();
}

void render_gpu_load(int passes) {
  put(kGpuLoad, static_cast<u32>(passes < 0 ? 0 : passes));
  publish_and_kick();
}

void render_trace(bool on) {
  put(kTrace, u32{on ? 1u : 0u});
  publish_and_kick();
}

void render_digest(bool on, std::uint64_t detail_from, std::uint64_t detail_to) {
  struct {
    u64 on, from, to;
  } v{on ? 1u : 0u, detail_from, detail_to};
  put(kDigest, v);
  publish_and_kick();
}

std::string render_trace_take() {
  std::string l;
  {
    std::lock_guard<std::mutex> lock(S->mu);
    l.swap(S->trace);
  }
  if (!l.empty() && l.back() == ',') l.pop_back();
  return "[" + l + "]";
}

std::string render_digest_take() {
  std::lock_guard<std::mutex> lock(S->mu);
  std::string d;
  d.swap(S->digest);
  return d;
}

std::string render_status_json() {
  std::string s;
  {
    std::lock_guard<std::mutex> lock(S->mu);
    s = S->status;
  }
  // Write waits by what they waited for: vertex arrays, XF loads, palettes, preloads, EFB copies, textures.
  std::string kinds = "[";
  for (int k = 1; k < gfx::kReadKinds; ++k) {
    char kb[48];
    std::snprintf(kb, sizeof kb, "%s[%llu,%.1f]", k > 1 ? "," : "", static_cast<unsigned long long>(S->kind_waits[k]),
                  S->kind_ms[k]);
    kinds += kb;
  }
  kinds += "]";
  char b[1400];
  std::snprintf(b, sizeof b,
                ",\"resizes\":%llu,\"thread\":{\"fences\":%llu,\"fence_waits\":%llu,\"fence_ms\":%.1f,"
                "\"fence_hist\":[%llu,%llu,%llu,%llu],\"fence_hist_ms\":[%.0f,%.0f,%.0f,%.0f],"
                "\"fence_behind_kb\":[%.0f,%.0f,%.0f,%.0f],\"write_checks\":%llu,\"write_waits\":%llu,"
                "\"write_wait_ms\":%.1f,\"pending_pages\":%zu,\"read_ranges\":%llu,\"lag_waits\":%llu,"
                "\"lag_ms\":%.1f,\"records\":%llu,\"ring_mb\":%.1f,\"full_waits\":%llu,\"big\":%llu,"
                "\"wait_kinds\":%s,\"gp_flushes\":%llu,\"gp_flushed_mb\":%.1f,\"gp_sent_mb\":%.2f,\"gp_unmarked\":%llu,"
                "\"read_primitives\":%llu,\"read_indexed\":%llu,\"read_skipped\":%llu,\"mark_entries\":%llu,"
                "\"copies\":%llu,\"copy_mb\":%.2f,\"copy_ends\":%llu}",
                static_cast<unsigned long long>(S->resizes), static_cast<unsigned long long>(S->fences),
                static_cast<unsigned long long>(S->fence_waits), S->fence_ms,
                static_cast<unsigned long long>(S->fence_hist[0]), static_cast<unsigned long long>(S->fence_hist[1]),
                static_cast<unsigned long long>(S->fence_hist[2]), static_cast<unsigned long long>(S->fence_hist[3]),
                S->fence_hist_ms[0], S->fence_hist_ms[1], S->fence_hist_ms[2], S->fence_hist_ms[3],
                S->fence_behind_kb[0], S->fence_behind_kb[1], S->fence_behind_kb[2], S->fence_behind_kb[3],
                static_cast<unsigned long long>(S->write_checks), static_cast<unsigned long long>(S->write_waits),
                S->write_wait_ms, S->marked.size(), static_cast<unsigned long long>(S->reads.stats().ranges),
                static_cast<unsigned long long>(S->lag_waits), S->lag_ms,
                static_cast<unsigned long long>(S->ring.records()), static_cast<double>(S->ring.bytes()) / (1 << 20),
                static_cast<unsigned long long>(S->ring.full_waits()), static_cast<unsigned long long>(S->big),
                kinds.c_str(), static_cast<unsigned long long>(S->gp_flushes), static_cast<double>(S->gp_flushed_bytes) / 1e6,
                static_cast<double>(S->gp_sent_bytes) / 1e6, static_cast<unsigned long long>(S->gp_unmarked),
                static_cast<unsigned long long>(S->reads.stats().primitives),
                static_cast<unsigned long long>(S->reads.stats().indexed),
                static_cast<unsigned long long>(S->reads.stats().skipped),
                static_cast<unsigned long long>(S->mark_entries), static_cast<unsigned long long>(S->copies),
                static_cast<double>(S->copy_bytes) / 1e6, static_cast<unsigned long long>(S->copy_ends));
  return s + b;
}

std::uint64_t render_presents() { return S ? S->presents.load() : 0; }

}  // namespace mvp::web

// Called from JavaScript on the render thread.
extern "C" {

// The canvas arrived (mvp_js_render_listen).
EMSCRIPTEN_KEEPALIVE void mvp_render_canvas() {
  using namespace mvp::web;
  S->have_canvas = true;
  maybe_ready();
}

// The task loop, again (mvp_js_render_soon).
EMSCRIPTEN_KEEPALIVE void mvp_render_task() { mvp::web::render_task(); }

}  // extern "C"
