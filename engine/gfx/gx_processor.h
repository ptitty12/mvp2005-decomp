// The GX state machine (E1): executes the command stream the graphics
// processor reads (runtime/gx_fifo.h hands it every command, display-list
// contents included) the way Flipper would, as far as rendering goes:
// - CP, XF and BP register loads, with the BP write mask;
// - indexed XF loads and TLUT loads, read from guest memory at the moment the
//   command executes (as the hardware does);
// - primitives: vertices decoded from the current VCD/VAT and arrays, then the
//   transform unit (position, normal, lighting, texture coordinate generation)
//   on the CPU, assembled into triangle lists;
// - EFB copies.
// The result goes to a Backend (draw.h). Guest memory is only read.
//
// GPU-side values the guest reads (PE tokens, draw-done, FIFO state) are not
// computed here: they stay on the runtime's deterministic model (E0b-GX1).
#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <vector>

#include "gfx/draw.h"
#include "gfx/gp_memory.h"
#include "gfx/gx_regs.h"

namespace mvp {
class StateWriter;
class StateReader;
}  // namespace mvp

namespace mvp::gfx {

// A vertex as the CP loads it, before the transform unit.
struct InVertex {
  u8 pnmtx = 0;
  u8 texmtx[8] = {};
  float pos[3] = {};
  float nrm[3] = {}, bin[3] = {}, tan[3] = {};
  float color[2][4] = {};
  float tex[8][2] = {};
};

// A vertex format (VCD + one VAT) unpacked once per primitive (E3c): where each
// attribute is, its component format, count and fixed-point scale.
struct VertexFormat {
  struct Attr {
    u8 type = 0;   // 0 none, 1 direct, 2 8-bit index, 3 16-bit index
    u8 fmt = 0;    // component format (colors: color format)
    u8 cnt = 0;    // components
    u8 size = 0;   // bytes of the attribute's data (one vector for NBT with three indices)
    float scale = 1.0f;
  };
  bool pnmtx = false;
  u8 texmtx = 0;  // bit k: texture matrix index k is in the vertex
  Attr pos, nrm, color[2], tex[8];
  bool nbt = false, nbt3 = false;  // normal, binormal, tangent; with three indices
  u8 default_pnmtx = 0, default_texmtx[8] = {};
  u32 matidx_a = 0, matidx_b = 0;  // V5: the CP matrix index registers the defaults come from
  // V5: the same as a loader program (the fast path, GxProcessor::load_vertices):
  // one step per data read, in the vertex's byte order, each with its decoder.
  struct Step {
    u8 op = 0;     // decoder: 0-4 components (u8, s8, u16, s16, f32), 8-13 colors (ColorFmt)
    u8 type = 0;   // 1 direct, 2 8-bit index, 3 16-bit index
    u8 array = 0;  // CP array (indexed)
    u8 size = 0;   // bytes read
    u8 dst = 0;    // first float written, counted in InVertex's floats from pos[0]
    u8 n = 0;      // floats written
    float scale = 1.0f;
  };
  Step steps[14];
  u8 nsteps = 0;
};

// What the transform unit reads from XF registers and memory for one primitive,
// unpacked once (the state cannot change inside a primitive), and the matrices it
// fetched so far by index.
struct XformSetup {
  bool ortho = false;
  float pr[6] = {};
  float wd = 0, ht = 0, zr = 0, xo = 0, yo = 0, zf = 0, ox = 0, oy = 0;
  struct Light {
    float col[4], cosatt[3], distatt[3], pos[3], dir[3];
  };
  struct Chan {
    u32 ctl = 0;
    float mat[4] = {}, amb[4] = {};
  };
  u32 nchan = 0;
  Chan chan[2][2];  // [channel][color, alpha]
  Light lights[8];
  u32 ngen = 0;
  bool dual = false;
  bool lit = false;  // some channel uses lighting: the normal is transformed
  u32 texgen[8] = {}, post[8] = {};
  float postmtx[8][12] = {};
  // Matrices by 6-bit index: position/texture (3x4 at index * 4) and normal (3x3).
  std::uint64_t have_pos = 0, have_nrm = 0;
  float pos[64][12];
  float nrm[64][9];

  // V5: the transform's plan for the fast path (transform_fast), from the fields
  // above: what each color pass reads and which light factors it accumulates,
  // and what each texture coordinate generator does.
  struct Factor {
    u8 light, diffuse, attn;  // a light's factor with these functions (computed once per vertex)
  };
  struct Pass {
    u8 ch, alpha;             // channel, color (rgb) or alpha pass
    u8 mat_vtx, lit, amb_vtx;  // material from the vertex color, lit, ambient from the vertex color
    u8 nlights;
    u8 light[8], factor[8];   // the lights in order, and their factors (index into factors)
  };
  u32 npasses = 0, nfactors = 0;
  Pass passes[4];
  Factor factors[32];
  struct Gen {
    u8 type, row, ab11, stq, emboss_src, post_norm;
  };
  Gen gens[8];
  u32 ngen8 = 0;  // min(ngen, 8)
};

class GxProcessor {
 public:
  explicit GxProcessor(Backend* backend) : backend_(backend) { reset(); }

  // Guest memory (MEM1) that addresses in the stream point into (physical).
  void set_memory(const u8* mem, u32 size) {
    mem_ = mem;
    mem_size_ = size;
  }
  // Q1: read memory as the GP sees it (gfx/gp_memory.h) instead of directly.
  // Null: directly (tests, tools).
  void set_gp_memory(GpMemory* g) { gpm_ = g; }

  void reset();

  // Save-state files (runtime/state_file.h): the registers, XF memory, TLUT and
  // TMEM that reset() clears, field by field (common/state_io.h). Reading drops
  // the caches built from them.
  void write_state(StateWriter& w) const;
  void read_state(StateReader& r);

  // Executes one complete command (opcode and payload).
  void command(const u8* cmd, std::size_t len);

  // ---- state, for tests and debugging ----
  u32 bp(u8 reg) const { return bp_[reg]; }
  u32 cp_vcd_lo() const { return vcd_lo_; }
  u32 cp_vcd_hi() const { return vcd_hi_; }
  u32 xf_word(u32 addr) const {
    if (addr < xf::kMemSize) return xf_mem_[addr];
    if (addr >= xf::kRegBase && addr < xf::kRegBase + xf::kRegCount) return xf_regs_[addr - xf::kRegBase];
    return 0;
  }
  float xf_float(u32 addr) const { return std::bit_cast<float>(xf_word(addr)); }
  const u8* tlut_memory() const { return tlut_.data(); }
  const u8* tmem() const { return tmem_.data(); }

  // Vertex size in bytes for format `fmt` under the current VCD/VAT.
  u32 vertex_size(int fmt) const;
  // Decodes one vertex of format `fmt` from `p` (vertex_size bytes).
  InVertex load_vertex(int fmt, const u8* p) const;
  // The same with the format unpacked once (vertex_format()).
  VertexFormat vertex_format(int fmt) const;
  void load_vertex(const VertexFormat& f, const u8* p, InVertex& v) const;
  // V5: the fast path's loader (the format's loader program), for tests.
  InVertex load_vertex_fast(int fmt, const u8* p) const;
  // The transform unit for one vertex.
  OutVertex transform(const InVertex& v) const;
  // The same with the state unpacked once per primitive.
  void setup_transform(XformSetup& x) const;
  OutVertex transform(const InVertex& v, XformSetup& x) const;
  void transform(const InVertex& v, XformSetup& x, OutVertex& out) const;  // the same, into `out`
  // The pipeline configuration, uniforms and textures for a draw now.
  void snapshot(DrawCall& d) const;

  struct Stats {
    std::uint64_t commands = 0, draws = 0, vertices = 0, triangles = 0, copies = 0, xfb_copies = 0;
    std::uint64_t tlut_loads = 0, xf_indexed = 0, unknown = 0;
    std::uint64_t snapshots = 0;  // draws whose pixel state was rebuilt (the others reuse the last one)
    std::uint64_t preloads = 0;   // TMEM preloads (BP 0x63)
  };
  const Stats& stats() const { return stats_; }

  // The vertex format state of the last primitive, for debugging tools.
  struct PrimInfo {
    u8 op = 0;
    u32 count = 0, vertex_size = 0;
    u32 vcd_lo = 0, vcd_hi = 0, vat[3] = {}, matidx[2] = {};
    u32 texgen[2] = {}, num_texgens = 0, dual_tex = 0;
    u32 array_base[6] = {}, array_stride[6] = {};  // position, normal, colors, tex0, tex1
    std::vector<u8> first_vertex;
  };
  const PrimInfo& last_primitive() const { return prim_info_; }
  // Fill last_primitive() at every draw (off by default: it costs a copy per draw).
  void set_keep_primitive_info(bool on) { keep_prim_info_ = on; }
  // V5: primitives take the reference vertex path (load_vertex and transform, one
  // vertex at a time) instead of the fast one, and XF loads drop cached matrices
  // word by word; the two give the same bits (test_vertex_exact). For tests.
  void set_reference_vertices(bool on) { ref_vertices_ = on; }
  // V3: compute DrawCall::ident for every draw (in-between frames; off by default).
  void set_identify(bool on) { identify_ = on; }

 private:
  template <class Ar>
  void state_fields(Ar& a);
  void load_bp(u32 value);
  void load_cp(u8 reg, u32 value);
  void load_xf(u32 addr, u32 count, const u8* data);
  void load_xf_indexed(int array, u32 value);
  void primitive(u8 op, const u8* data, u32 count);
  // V5: the fast vertex path: `count` vertices of format `f` from `data` (`vsize`
  // bytes each) loaded and transformed into `out`.
  void vertices(const VertexFormat& f, const u8* data, u32 count, u32 vsize, OutVertex* out);
  void transform_fast(const InVertex& v, XformSetup& x, OutVertex& o) const;
  void load_fast(const VertexFormat& f, const u8* p, InVertex& v) const;
  void efb_copy();
  void load_tlut();
  void preload();
  void xf_written(u32 addr);  // drops what xs_ holds of XF word `addr`
  bool xf_matrices_written(u32 addr, u32 last);  // V5: the same for a range of matrix words, at once
  // V5: an indexed attribute's data in loader array `array` (0-11): through the
  // array's window of the GP's view (gp_memory.h) while it holds, else phys().
  const u8* fetch(u32 array, u32 addr, u32 len) const {
    if (!gpm_) return phys(addr, len);
    GpMemory::Window& w = win_[array];
    const u32 a = addr & 0x01FFFFFFu;
    if (w.version == gpm_->version() && a >= w.lo && a + len <= w.hi) return w.base + a;
    if (gpm_->window(a, len, w)) return w.base + a;
    w.version = 0;
    return gpm_->view(addr, len, 0);
  }
  const u8* phys(u32 addr, u32 len) const {  // null if outside memory
    if (gpm_) return gpm_->view(addr, len, 0);
    addr &= 0x01FFFFFFu;
    if (!mem_ || addr > mem_size_ || len > mem_size_ - addr) return nullptr;
    return mem_ + addr;
  }

  Backend* backend_;
  const u8* mem_ = nullptr;
  u32 mem_size_ = 0;
  GpMemory* gpm_ = nullptr;

  std::array<u32, 256> bp_{};
  u32 bp_mask_ = 0xFFFFFF;
  u32 vcd_lo_ = 0, vcd_hi_ = 0;
  std::array<u32, 8> vat_a_{}, vat_b_{}, vat_c_{};
  std::array<u32, 16> array_base_{}, array_stride_{};
  u32 matidx_a_ = 0, matidx_b_ = 0;
  std::array<u32, xf::kMemSize> xf_mem_{};
  std::array<u32, xf::kRegCount> xf_regs_{};
  // TEV color registers (PREV, REG0-2) and konst colors as written (11-bit signed).
  std::int32_t tev_regs_[4][4] = {};
  std::int32_t konst_[4][4] = {};
  std::vector<u8> tlut_;  // TMEM's palette half: 512 KiB
  std::vector<u8> tmem_;  // TMEM as preloads fill it (E3c): 1 MiB
  Stats stats_;
  PrimInfo prim_info_;
  bool keep_prim_info_ = false;
  bool ref_vertices_ = false;
  mutable GpMemory::Window win_[12];  // V5: the loader's windows, per array (fetch)
  bool identify_ = false;  // V3
  // Vertex formats 0-7 unpacked, and their sizes, valid until a VCD or VAT write (E3c).
  VertexFormat fmt_cache_[8];
  u32 fmt_size_[8] = {};
  u8 fmt_valid_ = 0;
  // Bumped by every write that snapshot() depends on (BP registers, TLUT loads);
  // a draw with the serial of the last snapshot reuses it (E3c).
  std::uint64_t state_serial_ = 1, snap_serial_ = 0;
  // Reused per primitive (no allocation per draw). P1: kept across primitives while
  // nothing it was built from changed: XF registers, lights and post matrices, the
  // scissor offset (xs_valid_); its cached matrices are dropped one by one when
  // their XF words are written (xf_written).
  XformSetup xs_;
  bool xs_valid_ = false;
  std::vector<OutVertex> vs_;
  DrawCall draw_;
};

}  // namespace mvp::gfx
