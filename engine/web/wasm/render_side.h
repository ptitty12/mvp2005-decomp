// The engine's rendering side in the browser (P2): the GX processor, the WebGPU
// renderer and the canvas, and what the page's tools read of them (the
// presentation trace, the renderer digest, statistics). The single-threaded
// engine drives it inline from the emulation (mvp_web.cpp); the threaded engine
// drives it on the render thread from the command ring (render_thread.cpp).
// Either way it sees the same calls in the same order. It only reads guest
// memory.
#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "common/state_io.h"
#include "gfx/digest_tap.h"
#include "gfx/draw.h"
#include "gfx/gp_memory.h"
#include "gfx/gx_processor.h"
#include "gfx/wgpu/gpu.h"
#include "gfx/wgpu/gpu_timer.h"
#include "gfx/wgpu/renderer.h"
#include "gfx/wgpu/window.h"

namespace mvp::web {

// Q5: time the render passes on the GPU where the device can (gfx/wgpu/gpu_timer.h); false
// (`gpuTimer=0`, development) leaves them untimed. Set before the renderer is created.
extern bool g_gpu_timing;

// Presentation trace (P1, development): every VI field's frame buffer address,
// every XFB copy and what each present showed, as JSON records. The tap sits
// between the GX processor and the renderer and forwards everything unchanged.
struct XfbTap : gfx::Backend {
  gfx::Backend* next = nullptr;
  bool on = false;
  std::string log;
  std::uint64_t copies = 0;     // XFB copies seen
  std::uint64_t field = 0;      // VI fields so far (the last field seen)
  void draw(const gfx::DrawCall& d) override { next->draw(d); }
  void efb_copy(const gfx::EfbCopy& c) override;
  void invalidate_textures() override { next->invalidate_textures(); }
};

class RenderSide {
 public:
  // The device and the canvas (set by whoever created them), and why not.
  std::unique_ptr<gfx::Gpu> gpu;
  std::unique_ptr<gfx::Window> window;
  std::string error;

  // Guest memory; the GX processor and the renderer are created once the device,
  // the canvas and memory are all there (attach()).
  void set_memory(const std::uint8_t* mem, std::uint32_t size);
  void attach();
  bool attached() const { return renderer_ != nullptr; }

  void gx(const std::uint8_t* cmd, std::size_t len) {
    if (proc_) proc_->command(cmd, len);
  }
  // Q1: what the GP sees of memory (gfx/gp_memory.h). Inline: the game flushed
  // [addr, addr + len) (taken from memory now). On the render thread: the
  // flushed range and the contents of the lines that changed, as the emulation
  // thread took them. Memory written without the cache.
  void gp_flush(std::uint32_t addr, std::uint32_t len) { gp_.flush(addr, len); }
  void gp_activate(std::uint32_t addr, std::uint32_t len) { gp_.activate(addr, len); }
  void gp_set_lines(std::uint32_t first_line, std::uint32_t lines, const std::uint8_t* data) {
    gp_.set_lines(first_line, lines, data);
  }
  void gp_write_through(std::uint32_t addr, std::uint32_t len) { gp_.write_through(addr, len); }
  // Q6 (the render thread): copies of what commands read (GpMemory::shadow).
  void gp_shadow(std::uint32_t addr, std::uint32_t len, const std::uint8_t* data) { gp_.shadow(addr, len, data); }
  void gp_end_shadow(std::uint32_t addr, std::uint32_t len) { gp_.end_shadow(addr, len); }
  const gfx::GpMemory& gp_memory() const { return gp_; }
  // A VI field ended; `xfb` is the frame buffer the VI scans.
  void vi_field(std::uint64_t field, std::uint32_t xfb);
  // Shows the frame the VI scans out. `frame_polls` is for the trace. True if drawn.
  bool present(std::uint64_t frame_polls, double now_ms);
  void flush() {
    if (renderer_) renderer_->flush();
  }
  void resize(int width, int height);
  // Q1: the internal resolution, 1-3 times the console's (gfx/wgpu/renderer.h);
  // it takes effect with the next frame the game starts.
  void set_scale(int s);

  void set_trace(bool on) { tap_.on = on; }
  std::string trace_take();      // the records since the last call, as a JSON array
  std::string trace_take_raw();  // the same records, comma-terminated, without the brackets
  // The renderer digest (gfx/digest_tap.h): on from the next command; one line
  // per VI field: "field all state verts textures texture-memory copies draws".
  // Fields detail_from..detail_to also get a "d ..." line per draw.
  void set_digest(bool on, std::uint64_t detail_from = 1, std::uint64_t detail_to = 0) {
    digest_.on = on;
    detail_from_ = detail_from;
    detail_to_ = detail_to;
    digest_.detail = detail_from_ <= 1 && 1 <= detail_to_;
  }
  std::string digest_take();

  // Gameplay mods' reviews (mods/gameplay/timeline.h): the GX processor's state
  // saved with a save-state slot and restored with it; after a restore the
  // renderer's textures and the GP's memory snapshots start over.
  void save_view(int slot) {
    if (!proc_) return;
    if (views_[slot]) *views_[slot] = *proc_;
    else views_[slot] = std::make_unique<gfx::GxProcessor>(*proc_);
  }
  void load_view(int slot) {
    if (proc_ && views_[slot]) *proc_ = *views_[slot];
    if (renderer_) renderer_->invalidate_textures();
    gp_.field();
  }
  // Save-state files (runtime/state_file.h): the GX processor's state as bytes
  // (empty before the renderer is attached), and back. Empty bytes reset it: the
  // game's next frames set up what they draw with.
  std::vector<std::uint8_t> export_view() const {
    if (!proc_) return {};
    StateWriter w;
    proc_->write_state(w);
    return std::move(w.out());
  }
  void import_view(std::span<const std::uint8_t> bytes) {
    if (proc_) {
      if (bytes.empty()) {
        proc_->reset();
      } else {
        StateReader r(bytes);
        proc_->read_state(r);
      }
    }
    if (renderer_) renderer_->invalidate_textures();
    gp_.field();
  }

  // ",\"draws\":..." members for mvp_status (empty without a renderer).
  std::string status_json() const;
  std::uint64_t presents() const { return presents_; }
  // Q5: presents that showed a new picture (frames shown).
  std::uint64_t presents_new() const { return presents_new_; }
  // Q5 (development, `gpuLoad=N`): extra GPU work at every present, in proportion
  // to the picture's internal size (gfx::Window::burn).
  void set_gpu_load(int passes) { gpu_load_ = passes < 0 ? 0 : passes; }
  std::uint64_t resizes() const { return resizes_; }

 private:
  const std::uint8_t* mem_ = nullptr;
  std::uint32_t mem_size_ = 0;
  gfx::GpMemory gp_;  // Q1: memory as the GP sees it; the processor, the renderer and the digest read it
  std::unique_ptr<gfx::Renderer> renderer_;
  std::unique_ptr<gfx::GxProcessor> proc_;
  std::unique_ptr<gfx::GxProcessor> views_[2];  // save_view / load_view
  gfx::DigestTap digest_;
  XfbTap tap_;
  std::string digest_log_;
  std::uint64_t detail_from_ = 1, detail_to_ = 0;
  std::uint32_t xfb_ = 0;
  std::uint64_t presents_ = 0, last_presented_ = 0, copies_at_present_ = 0, resizes_ = 0;
  int scale_ = 1;
  // Q5: the GPU's time per presented frame, from timestamp queries (gfx/wgpu/gpu_timer.h),
  // where the device has them.
  std::unique_ptr<gfx::GpuTimer> timer_;
  std::uint64_t presents_new_ = 0;
  double present_ms_ = 0;  // host time in present() (the canvas's draw and the frame's hand-over), in all
  int gpu_load_ = 0;
};

}  // namespace mvp::web
