// Which guest memory the renderer will read for each GX command (P2). The
// emulation feeds it the command stream (as the GX processor sees it) and it
// reports, command by command, the physical ranges that the GX processor and
// the WebGPU renderer read when they execute that command later on the render
// thread: the vertex array elements a primitive's indices select, the
// matrices of indexed XF loads, TLUT loads and TMEM preloads, the textures a
// draw samples the first time after a texture cache invalidation (when the
// renderer reads them: to decode them or to check its cached copy; a TLUT load
// starts that over, since the palette is part of the renderer's key), and the
// memory under an EFB copy to a texture (whose hash the renderer keeps). The
// emulation then makes every guest write to such a range wait until the render
// thread has finished the command (web/wasm/render_thread.cpp): the render
// thread reads memory as it was when the command was issued, as the inline
// renderer does. Reads nothing itself; only the command bytes.
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_set>

#include "gfx/gx_regs.h"

namespace mvp::gfx {

// What a range is (statistics: which reads guest writes wait for).
enum class ReadKind : u8 { kVertex = 1, kXf, kTlut, kPreload, kCopy, kTexture };
constexpr int kReadKinds = 7;  // 0 unused

class GpReads {
 public:
  // Called with each range [addr, addr + size) (physical) a command reads.
  using Mark = std::function<void(u32 addr, u32 size, ReadKind kind)>;
  explicit GpReads(Mark mark) : mark_(std::move(mark)) {}

  void command(const u8* c, std::size_t len);
  // V4: whether command() does anything for a command starting with `op` (CP and BP
  // loads, indexed XF loads, primitives); the caller may skip the others.
  static bool reads_something(u8 op) {
    return op == 0x08 || (op & 0xE7) == 0x20 || op == 0x61 || (op & 0xC0) == 0x80;
  }

  // Q4: the marks' epoch. Within one epoch the owner promises that a range
  // marked once stays marked (its marks are good for every later command of the
  // epoch, and nothing clears them), so a vertex array's elements are reported
  // only the first time a primitive of the epoch reads them. The owner starts a
  // new epoch (set_epoch with another value, or forget()) whenever that stops
  // holding.
  // Until the first set_epoch, every range is reported.
  void set_epoch(std::uint32_t e) {
    epoch_ = e;
    once_ = true;
  }
  void forget() { ++gen_; }

  struct Stats {
    std::uint64_t primitives = 0, indexed = 0, ranges = 0, textures = 0;
    std::uint64_t skipped = 0;  // Q4: vertex ranges an earlier primitive of the epoch had reported
  };
  const Stats& stats() const { return stats_; }

 private:
  void load_cp(u8 reg, u32 v);
  void load_bp(u32 value);
  void primitive(u8 op, const u8* data, u32 count);
  void vertex_range(u32 array, u32 lo, u32 hi, u32 elem);
  struct Layout;
  const Layout& layout(int fmt);
  void textures();
  void range(u32 addr, u32 size, ReadKind kind);

  Mark mark_;
  // Q4: each vertex format's indexed attributes (from the VCD and VAT), until
  // one of those changes.
  struct Indexed {
    u32 at, isize, array, elem, n;  // index offset, index bytes, array, element bytes, indices (3: NBT3)
  };
  struct Layout {
    bool valid = false;
    u32 vsize = 0, nix = 0;
    Indexed ix[12];
  };
  std::array<Layout, 8> layouts_{};
  // Q4: per array, the elements reported in the current epoch.
  struct Reported {
    std::uint32_t epoch = 0, gen = ~0u;
    u32 base = 0, stride = 0, elem = 0, lo = 0, hi = 0;
  };
  std::array<Reported, 16> reported_{};
  std::uint32_t epoch_ = 0, gen_ = 0;
  bool once_ = false;  // ranges reported once per epoch (set_epoch was called)
  u32 vcd_lo_ = 0, vcd_hi_ = 0;
  std::array<u32, 8> vat_a_{}, vat_b_{}, vat_c_{};
  std::array<u32, 16> array_base_{}, array_stride_{};
  std::array<u32, 256> bp_{};
  u32 bp_mask_ = 0xFFFFFF;
  bool tex_dirty_ = true;  // texture state written since the textures were last looked at
  std::unordered_set<std::uint64_t> seen_;  // textures read since the last invalidation (or TLUT load)
  Stats stats_;
};

}  // namespace mvp::gfx
