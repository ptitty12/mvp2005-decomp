#include "gfx/wgpu/gpu_timer.h"

#include <cstring>

#include "gfx/wgpu/gpu.h"

namespace mvp::gfx {

namespace {
constexpr std::uint32_t kReads = 8;  // read-back buffers in flight at most
}

GpuTimer::GpuTimer(Gpu& gpu) : gpu_(gpu) {
  if (!gpu.timestamps()) return;
  wgpu::QuerySetDescriptor qd{};
  qd.label = "gpu timer";
  qd.type = wgpu::QueryType::Timestamp;
  qd.count = 2 * kPasses;
  set_ = gpu.device().CreateQuerySet(&qd);
  wgpu::BufferDescriptor bd{};
  bd.label = "gpu timer resolve";
  bd.size = 2 * kPasses * sizeof(std::uint64_t);
  bd.usage = wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::CopySrc;
  resolve_ = gpu.device().CreateBuffer(&bd);
  bd.label = "gpu timer read";
  bd.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
  for (std::uint32_t k = 0; k < kReads; ++k) state_->reads.push_back(Read{gpu.device().CreateBuffer(&bd), false});
  on_ = set_ && resolve_;
}

const wgpu::PassTimestampWrites* GpuTimer::pass() {
  if (!on_) return nullptr;
  if (used_ >= kPasses) {
    ++state_->stats.untimed;
    return nullptr;
  }
  writes_ = wgpu::PassTimestampWrites{};
  writes_.querySet = set_;
  writes_.beginningOfPassWriteIndex = 2 * used_;
  writes_.endOfPassWriteIndex = 2 * used_ + 1;
  ++used_;
  return &writes_;
}

void GpuTimer::end_frame() {
  if (!on_ || used_ == 0) return;
  wgpu::CommandEncoder enc = gpu_.device().CreateCommandEncoder();
  end_frame_encode(enc);
  wgpu::CommandBuffer cb = enc.Finish();
  gpu_.queue().Submit(1, &cb);
  end_frame_submitted();
}

void GpuTimer::end_frame_encode(const wgpu::CommandEncoder& enc) {
  pending_slot_ = -1;
  if (!on_ || used_ == 0) return;
  const std::uint32_t n = used_;
  used_ = 0;
  std::uint32_t slot = 0;
  while (slot < state_->reads.size() && state_->reads[slot].busy) ++slot;
  if (slot == state_->reads.size()) {
    ++state_->stats.dropped;
    return;
  }
  Read& r = state_->reads[slot];
  r.busy = true;
  const std::uint64_t bytes = 2ull * n * sizeof(std::uint64_t);
  enc.ResolveQuerySet(set_, 0, 2 * n, resolve_, 0);
  enc.CopyBufferToBuffer(resolve_, 0, r.buf, 0, bytes);
  pending_slot_ = static_cast<int>(slot);
  pending_n_ = n;
}

void GpuTimer::end_frame_submitted() {
  if (pending_slot_ < 0) return;
  const std::uint32_t slot = static_cast<std::uint32_t>(pending_slot_), n = pending_n_;
  pending_slot_ = -1;
  Read& r = state_->reads[slot];
  const std::uint64_t bytes = 2ull * n * sizeof(std::uint64_t);
  r.buf.MapAsync(wgpu::MapMode::Read, 0, bytes, wgpu::CallbackMode::AllowSpontaneous,
                 [st = state_, slot, n, bytes](wgpu::MapAsyncStatus status, wgpu::StringView) {
                   Read& rr = st->reads[slot];
                   if (status == wgpu::MapAsyncStatus::Success) {
                     std::vector<std::uint64_t> t(2 * n);
                     std::memcpy(t.data(), rr.buf.GetConstMappedRange(0, bytes), bytes);
                     double ns = 0;
                     for (std::uint32_t k = 0; k < n; ++k)
                       if (t[2 * k + 1] > t[2 * k]) ns += static_cast<double>(t[2 * k + 1] - t[2 * k]);
                     rr.buf.Unmap();
                     Stats& s = st->stats;
                     s.last_ms = ns / 1e6;
                     s.ms += s.last_ms;
                     s.passes += n;
                     ++s.frames;
                   }
                   rr.busy = false;
                 });
}

}  // namespace mvp::gfx
