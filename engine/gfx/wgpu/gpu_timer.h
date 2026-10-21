// The GPU's time, from timestamp queries (Q5): how long the GPU worked on the
// render passes of each presented frame. The browser's picture setting (Auto)
// lowers the internal resolution while the GPU is busy nearly all the time
// (engine/web/src/autoscale.ts).
//
// Optional: it needs the device feature TimestampQuery, which Gpu requests in
// the browser when the adapter has it (Chrome has it; it rounds the stamps to
// 100 us). Each timed render pass writes a timestamp at its start and its end;
// at the end of a frame the stamps are resolved and copied into a buffer that
// is read back asynchronously (never waited for), and the sum of the passes'
// durations becomes one sample. Passes beyond kPasses in one frame, and frames
// that find all eight read-back buffers still in flight, are not timed (stats:
// a heavily loaded GPU returns its results late, so those frames are counted as
// dropped and weighed at the mean of the timed ones).
//
// It measures only: nothing the GPU reports reaches guest state.
#pragma once

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace mvp::gfx {

class Gpu;

class GpuTimer {
 public:
  explicit GpuTimer(Gpu& gpu);

  // Whether the device can time passes.
  bool on() const { return on_; }

  // Timestamp writes for one more render pass (its start and its end), or null
  // (timing off, or this frame's queries are used up). Valid until the next call.
  const wgpu::PassTimestampWrites* pass();

  // Every pass of the frame has been submitted: resolve and read back their
  // stamps (without waiting); the sum of their durations becomes one sample.
  void end_frame();
  // V6: end_frame() in two parts, so the resolve goes into a command buffer of the
  // caller's (after the frame's last pass) instead of a submit of its own: the
  // resolve and copy into `enc`, then, once `enc`'s commands are submitted, the
  // read-back.
  void end_frame_encode(const wgpu::CommandEncoder& enc);
  void end_frame_submitted();

  struct Stats {
    std::uint64_t frames = 0;   // samples read back
    std::uint64_t passes = 0;   // passes timed in them
    std::uint64_t untimed = 0;  // passes that found no query left in their frame
    std::uint64_t dropped = 0;  // frames not read back (every read-back buffer in flight)
    double ms = 0;              // the samples' total GPU time
    double last_ms = 0;
  };
  const Stats& stats() const { return state_->stats; }

  static constexpr std::uint32_t kPasses = 128;  // per frame

 private:
  struct Read {
    wgpu::Buffer buf;
    bool busy = false;
  };
  // Shared with the read-backs' callbacks, which may come after the timer is gone.
  struct State {
    std::vector<Read> reads;
    Stats stats;
  };
  Gpu& gpu_;
  bool on_ = false;
  wgpu::QuerySet set_;
  wgpu::Buffer resolve_;
  std::shared_ptr<State> state_ = std::make_shared<State>();
  std::uint32_t used_ = 0;  // passes timed in the frame so far
  wgpu::PassTimestampWrites writes_{};
  // V6: the read-back end_frame_encode() set up, for end_frame_submitted(): its slot
  // (-1: none) and passes.
  int pending_slot_ = -1;
  std::uint32_t pending_n_ = 0;
};

}  // namespace mvp::gfx
