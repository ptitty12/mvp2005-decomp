#include "web/wasm/render_side.h"

#include <emscripten.h>

#include <cstdio>

// P2: with a frame port from the page (Module.mvpFrames), the canvas `sel` is
// the engine's own: its picture goes to the page as an ImageBitmap, which the
// page shows (main.ts). Without one the canvas is the page's and shows itself.
// Returns 1 if the canvas must be configured again (Firefox unconfigures a
// WebGPU canvas when its bitmap is taken; Chrome keeps it). V3: an in-between
// picture goes as {bitmap, between: true}, so the page counts it apart.
EM_JS(int, mvp_js_send_frame, (const char* sel, int between), {
  var p = Module['mvpFrames'];
  if (!p) return 0;
  var c = Module['specialHTMLTargets'][UTF8ToString(sel)];
  if (!c || !c.transferToImageBitmap) return 0;
  var b = c.transferToImageBitmap();
  if (between) p.postMessage({bitmap: b, between: true}, [b]);
  else p.postMessage(b, [b]);
  var ctx = c.getContext('webgpu');
  return ctx && ctx.getConfiguration && !ctx.getConfiguration() ? 1 : 0;
});

// V3: whether pictures go to the page (a frame port) rather than into its canvas.
EM_JS(int, mvp_js_has_frames, (), { return Module['mvpFrames'] ? 1 : 0; });

namespace mvp::web {

bool g_gpu_timing = true;

void XfbTap::efb_copy(const gfx::EfbCopy& c) {
  if (c.to_xfb) {
    ++copies;
    if (on) {
      char b[200];
      std::snprintf(b, sizeof b,
                    "{\"k\":\"c\",\"f\":%llu,\"d\":%u,\"x\":%u,\"y\":%u,\"w\":%u,\"h\":%u,\"ys\":%.4f,\"clr\":%d},",
                    static_cast<unsigned long long>(field), c.dest, c.src_x, c.src_y, c.width, c.height,
                    static_cast<double>(c.y_scale), c.clear ? 1 : 0);
      log += b;
    }
  }
  next->efb_copy(c);
}

void RenderSide::set_memory(const std::uint8_t* mem, std::uint32_t size) {
  mem_ = mem;
  mem_size_ = size;
  gp_.attach(mem, size);
  attach();
}

void RenderSide::attach() {
  if (!gpu || !window || !mem_ || renderer_) return;
  renderer_ = std::make_unique<gfx::Renderer>(*gpu, mem_, mem_size_, static_cast<std::uint32_t>(scale_));
  renderer_->set_gp_memory(&gp_);
  if (g_gpu_timing) {
    timer_ = std::make_unique<gfx::GpuTimer>(*gpu);  // Q5: the GPU's time (off without timestamp queries)
    if (timer_->on()) renderer_->set_timer(timer_.get());
  }
  tap_.next = renderer_.get();
  digest_.next = &tap_;
  digest_.set_memory(mem_, mem_size_);
  digest_.set_gp_memory(&gp_);
  proc_ = std::make_unique<gfx::GxProcessor>(&digest_);
  proc_->set_memory(mem_, mem_size_);
  proc_->set_gp_memory(&gp_);
  set_interp(interp_want_);  // V3
}

void RenderSide::set_interp(bool on) {
  // Only where pictures go to the page, which shows one per refresh: drawn into a canvas of
  // the page (the single-threaded engine), the frame right after would replace the in-between.
  interp_want_ = on;
  interp_ = on && mvp_js_has_frames();
  if (proc_) proc_->set_identify(interp_);
  if (renderer_) renderer_->set_interp(interp_);
}

void RenderSide::vi_field(std::uint64_t field, std::uint32_t xfb) {
  gp_.field();  // Q1: snapshots of the game's cache write-backs expire
  if (renderer_) {
    renderer_->flush();
    renderer_->scan_field(xfb);  // what the VI scans out at this field (P1)
    if (interp_) {  // V3: cuts, and in-betweens over a large move, by field (statistics)
      const gfx::Renderer::InterpStats& is = renderer_->interp_stats();
      const std::uint64_t cuts = is.cut_match + is.cut_shift;
      const bool cut = cuts != cuts_seen_, big = is.made != made_seen_ && is.last_shift >= 0.1;
      if (cut || big) {
        if (interp_events_.size() >= 32) interp_events_.erase(interp_events_.begin());
        interp_events_.push_back({field, !cut, static_cast<float>(is.last_match), static_cast<float>(is.last_shift)});
      }
      cuts_seen_ = cuts;
      made_seen_ = is.made;
    }
  }
  if (tap_.on && xfb != xfb_) {
    char b[80];
    std::snprintf(b, sizeof b, "{\"k\":\"v\",\"f\":%llu,\"t\":%u},", static_cast<unsigned long long>(field), xfb);
    tap_.log += b;
  }
  if (digest_.on) {
    using D = gfx::DigestTap;
    char b[200];
    std::snprintf(b, sizeof b, "%llu %016llx %016llx %016llx %016llx %016llx %016llx %llu\n",
                  static_cast<unsigned long long>(field), static_cast<unsigned long long>(digest_.digest()),
                  static_cast<unsigned long long>(digest_.part(D::kState)),
                  static_cast<unsigned long long>(digest_.part(D::kVerts)),
                  static_cast<unsigned long long>(digest_.part(D::kTex)),
                  static_cast<unsigned long long>(digest_.part(D::kTexMem)),
                  static_cast<unsigned long long>(digest_.part(D::kCopies)),
                  static_cast<unsigned long long>(digest_.draws()));
    digest_log_ += digest_.detail_log;
    digest_.detail_log.clear();
    digest_log_ += b;
    digest_.detail = detail_from_ <= field + 1 && field + 1 <= detail_to_;  // the draws of the next field
    digest_.reset();  // one digest per field
  }
  xfb_ = xfb;
  tap_.field = field;
}

bool RenderSide::present(std::uint64_t frame_polls, double now_ms) {
  if (!renderer_ || !window) return false;
  // The picture the VI scanned out at the last fields (P1: field-alternating
  // pixels mixed, as an interlaced display shows them), else the XFB copy at the
  // VI's address, else the latest copy.
  const gfx::Renderer::Frame* f = renderer_->scanout();
  const bool mixed = renderer_->is_mixed(f);
  const bool fallback = !f && !renderer_->xfb(xfb_);
  if (!f) f = renderer_->xfb(xfb_);
  if (!f) f = renderer_->latest_xfb();
  if (!f) return false;
  // V3: a new frame with an in-between picture (made at its XFB copy from the frame
  // shown last): that picture first (set_interp: only where pictures go to the page).
  const gfx::Renderer::Frame* between =
      interp_ && f->serial != last_presented_ ? renderer_->between(last_presented_, f->serial) : nullptr;
  if (tap_.on) {
    char b[200];
    std::snprintf(b, sizeof b,
                  "{\"k\":\"p\",\"f\":%llu,\"fr\":%llu,\"t\":%u,\"a\":%u,\"s\":%llu,\"new\":%d,\"fb\":%d,"
                  "\"n\":%llu,\"w\":%u,\"h\":%u,\"mix\":%d,\"ib\":%d,\"ms\":%.2f},",
                  static_cast<unsigned long long>(tap_.field), static_cast<unsigned long long>(frame_polls), xfb_,
                  f->addr, static_cast<unsigned long long>(f->serial), f->serial != last_presented_ ? 1 : 0,
                  fallback ? 1 : 0, static_cast<unsigned long long>(tap_.copies - copies_at_present_), f->width,
                  f->height, mixed ? 1 : 0, between ? 1 : 0, now_ms);
    tap_.log += b;
  }
  copies_at_present_ = tap_.copies;
  const double t0 = emscripten_get_now();
  gfx::GpuTimer* timer = timer_ && timer_->on() ? timer_.get() : nullptr;
  // A pass still open (the render thread may present in the middle of a field) is submitted first,
  // so the frame's timestamps are all written before they are read (the pass is only split in two).
  if (timer) renderer_->flush();
  if (gpu_load_) window->burn(f->view, f->width, f->height, gpu_load_, timer);  // Q5 (development)
  if (between) {  // V3
    window->present(between->view, between->width, between->height, timer);
    if (mvp_js_send_frame("#mvp-canvas", 1)) window->reconfigure();
    ++presents_between_;
  }
  window->present(f->view, f->width, f->height, timer);
  if (timer) timer->end_frame();  // this frame's passes, timed on the GPU
  if (mvp_js_send_frame("#mvp-canvas", 0)) window->reconfigure();
  present_ms_ += emscripten_get_now() - t0;
  ++presents_;
  if (f->serial != last_presented_) ++presents_new_;
  last_presented_ = f->serial;
  return true;
}


void RenderSide::set_scale(int s) {
  scale_ = s < 1 ? 1 : s > static_cast<int>(gfx::Renderer::kMaxScale) ? static_cast<int>(gfx::Renderer::kMaxScale) : s;
  if (renderer_) renderer_->set_scale(static_cast<std::uint32_t>(scale_));
}

void RenderSide::resize(int width, int height) {
  ++resizes_;
  if (window) window->resize(width, height);
}

std::string RenderSide::trace_take() {
  std::string l = trace_take_raw();
  if (!l.empty() && l.back() == ',') l.pop_back();
  return "[" + l + "]";
}

std::string RenderSide::trace_take_raw() {
  std::string out;
  out.swap(tap_.log);
  return out;
}

std::string RenderSide::digest_take() {
  std::string out;
  out.swap(digest_log_);
  return out;
}

std::string RenderSide::status_json() const {
  std::string s;
  if (renderer_) {
    const auto& rs = renderer_->stats();
    const auto& ps = proc_->stats();
    char b[1024];
    std::snprintf(b, sizeof b,
                  ",\"draws\":%llu,\"xfb_copies\":%llu,\"pipelines\":%llu,\"textures\":%llu,\"gpu_errors\":%llu,"
                  "\"copies_dropped\":%llu,\"copy_size_misses\":%llu,\"copy_tall\":%llu,\"copy_inside\":%llu,"
                  "\"scan_mixed\":%llu,\"scale\":%u",
                  static_cast<unsigned long long>(rs.draws), static_cast<unsigned long long>(ps.xfb_copies),
                  static_cast<unsigned long long>(rs.pipelines), static_cast<unsigned long long>(rs.textures_decoded),
                  static_cast<unsigned long long>(gpu->errors()), static_cast<unsigned long long>(rs.copies_dropped),
                  static_cast<unsigned long long>(rs.copy_size_misses), static_cast<unsigned long long>(rs.copy_tall),
                  static_cast<unsigned long long>(rs.copy_inside),
                  static_cast<unsigned long long>(renderer_->scan_stats().mixed), renderer_->scale());
    s += b;
    // Q5: frames shown; the GPU's time (frames timed, their GPU ms, passes timed, untimed and dropped, the
    // last frame's ms), the host ms in present(), the development load.
    const gfx::GpuTimer::Stats ts = timer_ ? timer_->stats() : gfx::GpuTimer::Stats{};
    std::snprintf(b, sizeof b,
                  ",\"presents_new\":%llu,\"gpu\":{\"timed\":%s,\"n\":%llu,\"ms\":%.1f,\"passes\":%llu,"
                  "\"untimed\":%llu,\"dropped\":%llu,\"last\":%.2f,\"present_ms\":%.1f,\"load\":%d}",
                  static_cast<unsigned long long>(presents_new_), timer_ && timer_->on() ? "true" : "false",
                  static_cast<unsigned long long>(ts.frames), ts.ms, static_cast<unsigned long long>(ts.passes),
                  static_cast<unsigned long long>(ts.untimed), static_cast<unsigned long long>(ts.dropped), ts.last_ms,
                  present_ms_, gpu_load_);
    s += b;
    // V5: the GX path's work (primitives, vertices, pixel-state snapshots) and the
    // renderer's per-draw paths (state reused, draws merged, WebGPU draws, bytes uploaded).
    std::snprintf(b, sizeof b,
                  ",\"gx\":{\"prims\":%llu,\"verts\":%llu,\"snaps\":%llu,\"same\":%llu,\"merged\":%llu,"
                  "\"encoded\":%llu,\"vbytes\":%llu,\"ibytes\":%llu,\"tex_hits\":%llu,\"cmds\":%llu,\"passes\":%llu,"
                  "\"tex_copies\":%llu,\"clears\":%llu}",
                  static_cast<unsigned long long>(ps.draws), static_cast<unsigned long long>(ps.vertices),
                  static_cast<unsigned long long>(ps.snapshots), static_cast<unsigned long long>(rs.same_state),
                  static_cast<unsigned long long>(rs.merged), static_cast<unsigned long long>(rs.encoded),
                  static_cast<unsigned long long>(rs.vertex_bytes), static_cast<unsigned long long>(rs.index_bytes),
                  static_cast<unsigned long long>(rs.texture_hits), static_cast<unsigned long long>(ps.commands),
                  static_cast<unsigned long long>(rs.passes), static_cast<unsigned long long>(rs.tex_copies),
                  static_cast<unsigned long long>(rs.clears));
    s += b;
    // V3: in-between frames (gfx/wgpu/renderer.h, InterpStats): made, not made and why, shown.
    const gfx::Renderer::InterpStats& is = renderer_->interp_stats();
    std::snprintf(b, sizeof b,
                  ",\"interp\":{\"on\":%s,\"frames\":%llu,\"made\":%llu,\"gap\":%llu,\"cut_match\":%llu,"
                  "\"cut_shift\":%llu,\"still\":%llu,\"other\":%llu,\"shown\":%llu,\"retextured\":%llu,\"early_copies\":%llu,"
                  "\"reordered\":%llu,\"match\":%.3f,\"shift\":%.4f,\"ms\":%.1f,\"ms_match\":%.1f,\"ranges\":%llu,"
                  "\"vertices\":%llu,\"replayed\":%llu}",
                  interp_ ? "true" : "false", static_cast<unsigned long long>(is.frames),
                  static_cast<unsigned long long>(is.made), static_cast<unsigned long long>(is.gap),
                  static_cast<unsigned long long>(is.cut_match), static_cast<unsigned long long>(is.cut_shift),
                  static_cast<unsigned long long>(is.still),
                  static_cast<unsigned long long>(is.other), static_cast<unsigned long long>(presents_between_),
                  static_cast<unsigned long long>(is.retextured), static_cast<unsigned long long>(is.early_copies),
                  static_cast<unsigned long long>(is.reordered), is.last_match, is.last_shift, is.ms, is.ms_match,
                  static_cast<unsigned long long>(is.ranges), static_cast<unsigned long long>(is.vertices),
                  static_cast<unsigned long long>(is.replayed));
    s += b;
    s.pop_back();  // the histograms, in the same object
    s += ",\"shift_hist\":[";
    for (std::uint64_t v : is.shift_hist) s += std::to_string(v) + ",";
    s.back() = ']';
    s += ",\"match_hist\":[";
    for (std::uint64_t v : is.match_hist) s += std::to_string(v) + ",";
    s.back() = ']';
    s += ",\"events\":[";
    for (const InterpEvent& e : interp_events_) {
      char ev[96];
      std::snprintf(ev, sizeof ev, "[%llu,\"%s\",%.3f,%.4f],", static_cast<unsigned long long>(e.field),
                    e.made ? "made" : "cut", static_cast<double>(e.match), static_cast<double>(e.shift));
      s += ev;
    }
    if (s.back() == ',') s.pop_back();
    s += "]}";
    const auto& gs = gp_.stats();  // Q1: the GP's view of memory
    std::snprintf(b, sizeof b, ",\"gp_memory\":{\"flushes\":%llu,\"flushed_mb\":%.1f,\"overlays\":%llu,\"lines\":%llu,\"drops\":%llu}",
                  static_cast<unsigned long long>(gs.flushes), static_cast<double>(gs.flushed_bytes) / 1e6,
                  static_cast<unsigned long long>(gs.overlays), static_cast<unsigned long long>(gs.overlaid_lines),
                  static_cast<unsigned long long>(gs.drops));
    s += b;
  }
  if (!error.empty()) {
    s += ",\"gfx_error\":\"";
    for (char ch : error) {
      if (ch == '"' || ch == '\\') s += '\\', s += ch;
      else if (static_cast<unsigned char>(ch) < 0x20) s += ' ';
      else s += ch;
    }
    s += '"';
  }
  return s;
}

}  // namespace mvp::web
