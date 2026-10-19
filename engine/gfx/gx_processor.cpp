#include "gfx/gx_processor.h"

#include "gfx/texture_decode.h"

#include "common/state_io.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>

namespace mvp::gfx {

namespace {

inline u32 be32_at(const u8* p) { return u32(p[0]) << 24 | u32(p[1]) << 16 | u32(p[2]) << 8 | p[3]; }
inline u16 be16_at(const u8* p) { return static_cast<u16>(p[0] << 8 | p[1]); }

inline std::int32_t sext11(u32 v) { return static_cast<std::int32_t>(v << 21) >> 21; }

inline u32 comp_size(u32 fmt) {
  switch (fmt) {
    case kU8: case kS8: return 1;
    case kU16: case kS16: return 2;
    default: return 4;
  }
}

inline u32 color_size(u32 fmt) {
  switch (fmt) {
    case kRgb565: case kRgba4444: return 2;
    case kRgb888: case kRgba6666: return 3;
    default: return 4;
  }
}

inline u32 index_size(u32 type) { return type == 2 ? 1u : type == 3 ? 2u : 0u; }

inline float read_comp(const u8* p, u32 fmt, float scale) {
  switch (fmt) {
    case kU8: return static_cast<float>(p[0]) * scale;
    case kS8: return static_cast<float>(static_cast<std::int8_t>(p[0])) * scale;
    case kU16: return static_cast<float>(be16_at(p)) * scale;
    case kS16: return static_cast<float>(static_cast<std::int16_t>(be16_at(p))) * scale;
    default: return std::bit_cast<float>(be32_at(p));
  }
}

// n / 255.0f for n = 0..255, computed once (the same division, looked up: P1).
struct Div255 {
  float v[256];
  Div255() {
    for (int n = 0; n < 256; ++n) v[n] = static_cast<float>(n) / 255.0f;
  }
};
const Div255 kDiv255;

inline void read_color(const u8* p, u32 fmt, float out[4]) {
  u32 r = 0, g = 0, b = 0, a = 255;
  switch (fmt) {
    case kRgb565: {
      u32 v = be16_at(p);
      r = (v >> 11) & 31; g = (v >> 5) & 63; b = v & 31;
      r = (r << 3) | (r >> 2); g = (g << 2) | (g >> 4); b = (b << 3) | (b >> 2);
      break;
    }
    case kRgb888: case kRgb888x:
      r = p[0]; g = p[1]; b = p[2];
      break;
    case kRgba4444: {
      u32 v = be16_at(p);
      r = ((v >> 12) & 15) * 17; g = ((v >> 8) & 15) * 17; b = ((v >> 4) & 15) * 17; a = (v & 15) * 17;
      break;
    }
    case kRgba6666: {
      u32 v = u32(p[0]) << 16 | u32(p[1]) << 8 | p[2];
      r = (v >> 18) & 63; g = (v >> 12) & 63; b = (v >> 6) & 63; a = v & 63;
      r = (r << 2) | (r >> 4); g = (g << 2) | (g >> 4); b = (b << 2) | (b >> 4); a = (a << 2) | (a >> 4);
      break;
    }
    default:
      r = p[0]; g = p[1]; b = p[2]; a = p[3];
      break;
  }
  out[0] = kDiv255.v[r];
  out[1] = kDiv255.v[g];
  out[2] = kDiv255.v[b];
  out[3] = kDiv255.v[a];
}

inline void unpack_rgba8(u32 v, float out[4]) {
  out[0] = kDiv255.v[v >> 24];
  out[1] = kDiv255.v[(v >> 16) & 0xFF];
  out[2] = kDiv255.v[(v >> 8) & 0xFF];
  out[3] = kDiv255.v[v & 0xFF];
}

// Texture coordinate count bit and format position for coordinate k in VAT A/B/C.
struct TexVatField {
  int reg, at;
};
constexpr TexVatField kTexVat[8] = {{0, 21}, {1, 0}, {1, 9}, {1, 18}, {1, 27}, {2, 5}, {2, 14}, {2, 23}};

inline float dot3(const float* a, const float* b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

inline void normalize3(float* v) {
  float l = std::sqrt(dot3(v, v));
  if (l > 0) {
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
  }
}

}  // namespace

void GxProcessor::reset() {
  bp_.fill(0);
  bp_mask_ = 0xFFFFFF;
  vcd_lo_ = vcd_hi_ = 0;
  vat_a_.fill(0);
  vat_b_.fill(0);
  vat_c_.fill(0);
  array_base_.fill(0);
  array_stride_.fill(0);
  matidx_a_ = matidx_b_ = 0;
  xf_mem_.fill(0);
  xf_regs_.fill(0);
  std::memset(tev_regs_, 0, sizeof tev_regs_);
  std::memset(konst_, 0, sizeof konst_);
  tlut_.assign(512 * 1024, 0);
  tmem_.assign(kTmemSize, 0);
  fmt_valid_ = 0;
  xs_valid_ = false;
  stats_ = {};
  ++state_serial_;
  // Power-on state that matters before the game's GXInit: one TEV stage,
  // scissor and alpha compare open.
  bp_[bp::kScissorBR] = (342u + kEfbHeight - 1) | (342u + kEfbWidth - 1) << 12;
  bp_[bp::kScissorTL] = 342u | 342u << 12;
  bp_[bp::kScissorOffset] = 171u | 171u << 10;
  bp_[bp::kAlphaCompare] = 7u << 16 | 7u << 19;
}

template <class Ar>
void GxProcessor::state_fields(Ar& a) {
  a(bp_);
  a(bp_mask_);
  a(vcd_lo_);
  a(vcd_hi_);
  a(vat_a_);
  a(vat_b_);
  a(vat_c_);
  a(array_base_);
  a(array_stride_);
  a(matidx_a_);
  a(matidx_b_);
  a(xf_mem_);
  a(xf_regs_);
  a(tev_regs_);
  a(konst_);
  a(tlut_);
  a(tmem_);
}

void GxProcessor::write_state(StateWriter& w) const {
  const_cast<GxProcessor*>(this)->state_fields(w);  // the writer only reads the fields
}

void GxProcessor::read_state(StateReader& r) {
  const std::size_t tlut = tlut_.size(), tmem = tmem_.size();
  state_fields(r);
  if (tlut_.size() != tlut || tmem_.size() != tmem) throw StateError("save state: the renderer's TMEM has another size");
  fmt_valid_ = 0;
  xs_valid_ = false;
  ++state_serial_;
}

void GxProcessor::command(const u8* c, std::size_t len) {
  ++stats_.commands;
  const u8 op = c[0];
  switch (op) {
    case cmd::kNop:
    case cmd::kInvalVtx:
      return;
    case cmd::kLoadCp:
      if (len >= 6) load_cp(c[1], be32_at(c + 2));
      return;
    case cmd::kLoadXf: {
      if (len < 5) return;
      u32 w = be32_at(c + 1);
      u32 count = (w >> 16) + 1;
      if (len < 5 + 4 * static_cast<std::size_t>(count)) return;
      load_xf(w & 0xFFFF, count, c + 5);
      return;
    }
    case cmd::kLoadIndxA: case cmd::kLoadIndxB: case cmd::kLoadIndxC: case cmd::kLoadIndxD:
      if (len >= 5) load_xf_indexed(12 + ((op - cmd::kLoadIndxA) >> 3), be32_at(c + 1));
      return;
    case cmd::kLoadBp:
      if (len >= 5) load_bp(be32_at(c + 1));
      return;
    default:
      break;
  }
  if (op >= 0x80 && op < 0xC0 && len >= 3) {
    u32 count = u32(c[1]) << 8 | c[2];
    primitive(op, c + 3, count);
    return;
  }
  ++stats_.unknown;
}

void GxProcessor::load_cp(u8 reg, u32 v) {
  if (reg == cp::kVcdLo || reg == cp::kVcdHi || (reg & 0xF8) == cp::kVatA || (reg & 0xF8) == cp::kVatB ||
      (reg & 0xF8) == cp::kVatC)
    fmt_valid_ = 0;  // the unpacked vertex formats
  if (reg == cp::kMatIndexA) matidx_a_ = v;
  else if (reg == cp::kMatIndexB) matidx_b_ = v;
  else if (reg == cp::kVcdLo) vcd_lo_ = v;
  else if (reg == cp::kVcdHi) vcd_hi_ = v;
  else if ((reg & 0xF8) == cp::kVatA) vat_a_[reg & 7] = v;
  else if ((reg & 0xF8) == cp::kVatB) vat_b_[reg & 7] = v;
  else if ((reg & 0xF8) == cp::kVatC) vat_c_[reg & 7] = v;
  else if ((reg & 0xF0) == cp::kArrayBase) array_base_[reg & 15] = v & 0x03FFFFFFu;
  else if ((reg & 0xF0) == cp::kArrayStride) array_stride_[reg & 15] = v & 0xFF;
}

void GxProcessor::xf_written(u32 a) {
  if (!xs_valid_) return;  // the next primitive rebuilds everything
  if (a < 0x100) {
    // Position/texture matrix i reads words (4i .. 4i + 11) & 0xFF: word a is in i = q, q - 1, q - 2.
    const u32 q = a >> 2;
    xs_.have_pos &= ~(1ull << q | 1ull << ((q - 1) & 63) | 1ull << ((q - 2) & 63));
  } else if (a < xf::kNrmMtx) {
    // Not read by the transform unit.
  } else if (a < xf::kPostMtx) {
    // Normal matrix n reads words kNrmMtx + 3n .. + 8 (n < 32).
    const u32 w = a - xf::kNrmMtx;
    for (u32 n = w >= 8 ? (w - 6) / 3 : 0; n <= w / 3 && n < 32; ++n) xs_.have_nrm &= ~(1ull << n);
  } else {
    xs_valid_ = false;  // post matrices, lights, registers
  }
}

void GxProcessor::load_xf(u32 addr, u32 count, const u8* data) {
  for (u32 k = 0; k < count; ++k) {
    u32 a = addr + k;
    u32 v = be32_at(data + 4 * k);
    if (a < xf::kMemSize) xf_mem_[a] = v;
    else if (a >= xf::kRegBase && a < xf::kRegBase + xf::kRegCount) xf_regs_[a - xf::kRegBase] = v;
    xf_written(a);
  }
}

void GxProcessor::load_xf_indexed(int array, u32 v) {
  ++stats_.xf_indexed;
  u32 addr = v & 0xFFF;
  u32 count = ((v >> 12) & 0xF) + 1;
  u32 index = v >> 16;
  const u8* p = phys(array_base_[array] + index * array_stride_[array], 4 * count);
  if (!p) return;
  for (u32 k = 0; k < count; ++k)
    if (addr + k < xf::kMemSize) {
      xf_mem_[addr + k] = be32_at(p + 4 * k);
      xf_written(addr + k);
    }
}

void GxProcessor::load_bp(u32 value) {
  ++state_serial_;  // all the draw state snapshot() reads comes from BP writes (TLUT loads included)
  const u8 reg = static_cast<u8>(value >> 24);
  u32 v = value & 0xFFFFFF;
  if (reg == bp::kMask) {
    bp_mask_ = v;
    bp_[reg] = v;
    return;
  }
  v = (bp_[reg] & ~bp_mask_) | (v & bp_mask_);
  bp_mask_ = 0xFFFFFF;
  bp_[reg] = v;
  if (reg == bp::kScissorOffset) xs_valid_ = false;  // the transform's window offset
  if (reg >= bp::kTevReg && reg < bp::kTevReg + 8) {
    int r = (reg - bp::kTevReg) >> 1;
    bool konst = (v >> 23) & 1;
    std::int32_t lo = konst ? static_cast<std::int32_t>(v & 0xFF) : sext11(v & 0x7FF);
    std::int32_t hi = konst ? static_cast<std::int32_t>((v >> 12) & 0xFF) : sext11((v >> 12) & 0x7FF);
    std::int32_t(*dst)[4] = konst ? konst_ : tev_regs_;
    if ((reg & 1) == 0) {  // RA
      dst[r][0] = lo;
      dst[r][3] = hi;
    } else {  // BG
      dst[r][2] = lo;
      dst[r][1] = hi;
    }
    return;
  }
  switch (reg) {
    case bp::kTriggerCopy:
      efb_copy();
      break;
    case bp::kLoadTlut1:
      load_tlut();
      break;
    case bp::kPreloadMode:
      if (v) preload();
      break;
    case bp::kTexInvalidate:
      if (backend_) backend_->invalidate_textures();
      break;
    default:
      break;
  }
}

void GxProcessor::load_tlut() {
  ++stats_.tlut_loads;
  u32 src = (bp_[bp::kLoadTlut0] & 0x1FFFFF) << 5;
  u32 v = bp_[bp::kLoadTlut1];
  u32 dst = (v & 0x3FF) << 9;
  u32 bytes = ((v >> 10) & 0x7FF) * 32;
  if (dst >= tlut_.size()) return;
  bytes = std::min<u32>(bytes, static_cast<u32>(tlut_.size()) - dst);
  const u8* p = phys(src, bytes);
  if (p) std::memcpy(tlut_.data() + dst, p, bytes);
}

// TMEM preload (GXPreLoadEntireTexture): 32-byte lines from memory to the even
// bank; RGBA8 (type 3) sends each 64-byte tile's AR half to the even bank and
// its GB half to the odd bank. Memory is read now, as the hardware does.
void GxProcessor::preload() {
  ++stats_.preloads;
  const u32 v = bp_[bp::kPreloadMode];
  const u32 lines = bits(v, 0, 15), type = bits(v, 15, 2);
  const u32 src = (bp_[bp::kPreloadAddr] & 0xFFFFFF) << 5;
  u32 even = bits(bp_[bp::kPreloadTmemEven], 0, 15) << 5, odd = bits(bp_[bp::kPreloadTmemOdd], 0, 15) << 5;
  if (type != 3) {
    u32 bytes = lines * 32;
    if (even >= kTmemSize) return;
    bytes = std::min(bytes, kTmemSize - even);
    const u8* p = phys(src, bytes);
    if (p) std::memcpy(tmem_.data() + even, p, bytes);
    return;
  }
  const u8* p = phys(src, lines * 64);
  if (!p) return;
  for (u32 k = 0; k < lines && even + 32 <= kTmemSize && odd + 32 <= kTmemSize; ++k, even += 32, odd += 32) {
    std::memcpy(tmem_.data() + even, p + 64 * k, 32);
    std::memcpy(tmem_.data() + odd, p + 64 * k + 32, 32);
  }
}

void GxProcessor::efb_copy() {
  ++stats_.copies;
  const u32 v = bp_[bp::kTriggerCopy];
  EfbCopy c;
  c.copy_value = v;
  c.src_x = bits(bp_[bp::kEfbTL], 0, 10);
  c.src_y = bits(bp_[bp::kEfbTL], 10, 10);
  c.width = bits(bp_[bp::kEfbWH], 0, 10) + 1;
  c.height = bits(bp_[bp::kEfbWH], 10, 10) + 1;
  c.dest = (bp_[bp::kXfbAddr] & 0xFFFFFF) << 5;
  c.dest_stride = bits(bp_[bp::kMipmapStride], 0, 10);
  c.clamp_top = bits(v, 0, 1);
  c.clamp_bottom = bits(v, 1, 1);
  u32 fmt = bits(v, 3, 4);
  c.tex_format = static_cast<u8>((fmt >> 1) | ((fmt & 1) << 3));
  c.gamma = bits(v, 7, 2);
  c.half_scale = bits(v, 9, 1);
  c.clear = bits(v, 11, 1);
  c.to_xfb = bits(v, 14, 1);
  c.intensity = bits(v, 15, 1);
  u32 ys = bits(bp_[bp::kDispCopyYScale], 0, 9);
  c.y_scale = (bits(v, 10, 1) && ys) ? 256.0f / static_cast<float>(ys) : 1.0f;
  u32 ar = bp_[bp::kClearAR], gb = bp_[bp::kClearGB];
  c.clear_rgba[0] = static_cast<u8>(ar & 0xFF);
  c.clear_rgba[1] = static_cast<u8>((gb >> 8) & 0xFF);
  c.clear_rgba[2] = static_cast<u8>(gb & 0xFF);
  c.clear_rgba[3] = static_cast<u8>((ar >> 8) & 0xFF);
  c.clear_z = bp_[bp::kClearZ] & 0xFFFFFF;
  c.clear_color = bits(bp_[bp::kCMode0], 3, 1);
  c.clear_alpha = bits(bp_[bp::kCMode0], 4, 1);
  c.clear_depth = bits(bp_[bp::kZMode], 4, 1);
  c.pixel_format = static_cast<u8>(bits(bp_[bp::kZCompare], 0, 3));
  if (c.to_xfb) ++stats_.xfb_copies;
  if (backend_) backend_->efb_copy(c);
}

u32 GxProcessor::vertex_size(int fmt) const {
  const u32 a = vat_a_[fmt], b = vat_b_[fmt], c = vat_c_[fmt];
  u32 size = 0;
  for (int k = 0; k < 9; ++k) size += bits(vcd_lo_, k, 1);
  u32 t = bits(vcd_lo_, 9, 2);
  if (t == 1) size += (bits(a, 0, 1) ? 3 : 2) * comp_size(bits(a, 1, 3));
  else size += index_size(t);
  t = bits(vcd_lo_, 11, 2);
  bool nbt = bits(a, 9, 1);
  if (t == 1) size += (nbt ? 9 : 3) * comp_size(bits(a, 10, 3));
  else size += index_size(t) * (nbt && bits(a, 31, 1) ? 3 : 1);
  for (int k = 0; k < 2; ++k) {
    t = bits(vcd_lo_, 13 + 2 * k, 2);
    if (t == 1) size += color_size(bits(a, 14 + 4 * k, 3));
    else size += index_size(t);
  }
  for (int k = 0; k < 8; ++k) {
    t = bits(vcd_hi_, 2 * k, 2);
    u32 reg = kTexVat[k].reg == 0 ? a : kTexVat[k].reg == 1 ? b : c;
    int at = kTexVat[k].at;
    if (t == 1) size += (bits(reg, at, 1) ? 2 : 1) * comp_size(bits(reg, at + 1, 3));
    else size += index_size(t);
  }
  return size;
}

VertexFormat GxProcessor::vertex_format(int fmt) const {
  const u32 a = vat_a_[fmt], b = vat_b_[fmt], c = vat_c_[fmt];
  VertexFormat f;
  f.default_pnmtx = static_cast<u8>(bits(matidx_a_, 0, 6));
  for (int k = 0; k < 8; ++k) {
    u32 src = k < 4 ? matidx_a_ : matidx_b_;
    f.default_texmtx[k] = static_cast<u8>(bits(src, k < 4 ? 6 + 6 * k : 6 * (k - 4), 6));
  }
  f.pnmtx = bits(vcd_lo_, 0, 1) != 0;
  for (int k = 0; k < 8; ++k)
    if (bits(vcd_lo_, 1 + k, 1)) f.texmtx |= static_cast<u8>(1u << k);
  // Position.
  f.pos.type = static_cast<u8>(bits(vcd_lo_, 9, 2));
  f.pos.cnt = static_cast<u8>(bits(a, 0, 1) ? 3 : 2);
  f.pos.fmt = static_cast<u8>(bits(a, 1, 3));
  f.pos.scale = 1.0f / static_cast<float>(1u << bits(a, 4, 5));
  f.pos.size = static_cast<u8>(f.pos.cnt * comp_size(f.pos.fmt));
  // Normal, binormal, tangent.
  f.nrm.type = static_cast<u8>(bits(vcd_lo_, 11, 2));
  f.nbt = bits(a, 9, 1) != 0;
  f.nbt3 = f.nrm.type != 1 && f.nbt && bits(a, 31, 1);
  f.nrm.fmt = static_cast<u8>(bits(a, 10, 3));
  f.nrm.scale = f.nrm.fmt == kU8 ? 1.0f / 128 : f.nrm.fmt == kS8 ? 1.0f / 64 : f.nrm.fmt == kU16 ? 1.0f / 32768 : 1.0f / 16384;
  f.nrm.size = static_cast<u8>(3 * comp_size(f.nrm.fmt) * (f.nbt && !f.nbt3 ? 3 : 1));
  // Colors.
  for (int k = 0; k < 2; ++k) {
    f.color[k].type = static_cast<u8>(bits(vcd_lo_, 13 + 2 * k, 2));
    f.color[k].fmt = static_cast<u8>(bits(a, 14 + 4 * k, 3));
    f.color[k].size = static_cast<u8>(color_size(f.color[k].fmt));
  }
  // Texture coordinates.
  for (int k = 0; k < 8; ++k) {
    VertexFormat::Attr& t = f.tex[k];
    t.type = static_cast<u8>(bits(vcd_hi_, 2 * k, 2));
    const u32 reg = kTexVat[k].reg == 0 ? a : kTexVat[k].reg == 1 ? b : c;
    const int at = kTexVat[k].at;
    t.cnt = static_cast<u8>(bits(reg, at, 1) ? 2 : 1);
    t.fmt = static_cast<u8>(bits(reg, at + 1, 3));
    t.scale = 1.0f / static_cast<float>(1u << bits(reg, at + 4, 5));
    t.size = static_cast<u8>(t.cnt * comp_size(t.fmt));
  }
  return f;
}

InVertex GxProcessor::load_vertex(int fmt, const u8* p) const {
  InVertex v;
  load_vertex(vertex_format(fmt), p, v);
  return v;
}

void GxProcessor::load_vertex(const VertexFormat& f, const u8* p, InVertex& v) const {
  v.pnmtx = f.default_pnmtx;
  std::memcpy(v.texmtx, f.default_texmtx, sizeof v.texmtx);
  if (f.pnmtx) v.pnmtx = *p++ & 0x3F;
  for (int k = 0; k < 8; ++k)
    if (f.texmtx & (1u << k)) v.texmtx[k] = *p++ & 0x3F;

  // Where an attribute's data is: inline, or in its array.
  auto source = [&](u32 type, int array, u32 inline_size) -> const u8* {
    if (type == 1) {
      const u8* q = p;
      p += inline_size;
      return q;
    }
    u32 idx;
    if (type == 2) idx = *p++;
    else {
      idx = be16_at(p);
      p += 2;
    }
    return phys(array_base_[array] + idx * array_stride_[array], inline_size);
  };

  // Position.
  if (f.pos.type) {
    const u8* q = source(f.pos.type, 0, f.pos.size);
    if (q) {
      const u32 cs = comp_size(f.pos.fmt);
      for (u32 k = 0; k < f.pos.cnt; ++k) v.pos[k] = read_comp(q + k * cs, f.pos.fmt, f.pos.scale);
    }
  }
  // Normal, binormal, tangent.
  if (f.nrm.type) {
    const u32 cs = comp_size(f.nrm.fmt);
    const u32 vec = 3 * cs;
    float* dst[3] = {v.nrm, v.bin, v.tan};
    if (f.nbt3) {  // three indices, one per vector
      for (int n = 0; n < 3; ++n) {
        const u8* q = source(f.nrm.type, 1, vec);
        if (q)
          for (u32 k = 0; k < 3; ++k) dst[n][k] = read_comp(q + k * cs, f.nrm.fmt, f.nrm.scale);
      }
    } else {
      const u8* q = source(f.nrm.type, 1, f.nrm.size);
      if (q)
        for (int n = 0; n < (f.nbt ? 3 : 1); ++n)
          for (u32 k = 0; k < 3; ++k) dst[n][k] = read_comp(q + n * vec + k * cs, f.nrm.fmt, f.nrm.scale);
    }
  }
  // Colors.
  for (int k = 0; k < 2; ++k) {
    if (!f.color[k].type) continue;
    const u8* q = source(f.color[k].type, 2 + k, f.color[k].size);
    if (q) read_color(q, f.color[k].fmt, v.color[k]);
  }
  // Texture coordinates.
  for (int k = 0; k < 8; ++k) {
    const VertexFormat::Attr& t = f.tex[k];
    if (!t.type) continue;
    const u8* q = source(t.type, 4 + k, t.size);
    if (q) {
      const u32 cs = comp_size(t.fmt);
      for (u32 n = 0; n < t.cnt; ++n) v.tex[k][n] = read_comp(q + n * cs, t.fmt, t.scale);
    }
  }
}

void GxProcessor::setup_transform(XformSetup& x) const {
  for (int k = 0; k < 6; ++k) x.pr[k] = xf_float(xf::kProjection + static_cast<u32>(k));
  x.ortho = xf_word(xf::kProjection + 6) != 0;
  x.wd = xf_float(xf::kViewport);
  x.ht = xf_float(xf::kViewport + 1);
  x.zr = xf_float(xf::kViewport + 2);
  x.xo = xf_float(xf::kViewport + 3);
  x.yo = xf_float(xf::kViewport + 4);
  x.zf = xf_float(xf::kViewport + 5);
  x.ox = static_cast<float>(bits(bp_[bp::kScissorOffset], 0, 10) * 2);
  x.oy = static_cast<float>(bits(bp_[bp::kScissorOffset], 10, 10) * 2);
  x.nchan = xf_word(xf::kNumColors) & 3;
  u32 lights = 0;
  for (u32 ch = 0; ch < 2 && ch < x.nchan; ++ch)
    for (int is_alpha = 0; is_alpha < 2; ++is_alpha) {
      XformSetup::Chan& c = x.chan[ch][is_alpha];
      c.ctl = xf_word(xf::kColor0Cntrl + ch + (is_alpha ? 2u : 0u));
      unpack_rgba8(xf_word(xf::kMaterial0 + ch), c.mat);
      unpack_rgba8(xf_word(xf::kAmbient0 + ch), c.amb);
      if (bits(c.ctl, 1, 1)) lights |= bits(c.ctl, 2, 4) | bits(c.ctl, 11, 4) << 4;
    }
  for (u32 l = 0; l < 8; ++l) {
    if (!(lights & (1u << l))) continue;
    XformSetup::Light& L = x.lights[l];
    const u32 base = xf::kLights + l * 16;
    unpack_rgba8(xf_word(base + 3), L.col);
    for (u32 k = 0; k < 3; ++k) {
      L.cosatt[k] = xf_float(base + 4 + k);
      L.distatt[k] = xf_float(base + 7 + k);
      L.pos[k] = xf_float(base + 10 + k);
      L.dir[k] = xf_float(base + 13 + k);
    }
  }
  x.ngen = xf_word(xf::kNumTexGens) & 15;
  x.dual = xf_word(xf::kDualTex) & 1;
  for (u32 i = 0; i < x.ngen && i < 8; ++i) {
    x.texgen[i] = xf_word(xf::kTexGen + i);
    if (x.dual) {
      x.post[i] = xf_word(xf::kPostInfo + i);
      const u32 pm2 = xf::kPostMtx + (bits(x.post[i], 0, 6) * 4u);
      for (int k = 0; k < 12; ++k) x.postmtx[i][k] = xf_float(pm2 + static_cast<u32>(k));
    }
  }
  x.have_pos = x.have_nrm = 0;
  x.lit = false;
  for (u32 ch = 0; ch < 2 && ch < x.nchan; ++ch)
    x.lit = x.lit || bits(x.chan[ch][0].ctl, 1, 1) || bits(x.chan[ch][1].ctl, 1, 1);
}

OutVertex GxProcessor::transform(const InVertex& v) const {
  XformSetup x;
  setup_transform(x);
  return transform(v, x);
}

OutVertex GxProcessor::transform(const InVertex& v, XformSetup& x) const {
  OutVertex o{};
  transform(v, x, o);
  return o;
}

void GxProcessor::transform(const InVertex& v, XformSetup& x, OutVertex& o) const {
  // Every member is written below or zeroed here (the channels not computed) and
  // after the texture coordinates (the ones not generated): as from OutVertex{}.
  for (u32 ch = std::min<u32>(x.nchan, 2); ch < 2; ++ch) std::memset(o.color[ch], 0, sizeof o.color[ch]);
  // A 3x4 matrix at XF memory index * 4 (position and texture matrices).
  auto mtx34 = [&](u32 index) -> const float* {
    index &= 0x3F;
    if (!(x.have_pos >> index & 1)) {
      const u32 at = index * 4u;
      for (int k = 0; k < 12; ++k) x.pos[index][k] = xf_float(xf::kPosMtx + ((at + static_cast<u32>(k)) & 0xFF));
      x.have_pos |= 1ull << index;
    }
    return x.pos[index];
  };
  // Position: model-view (3x4 at index * 4), projection, viewport, then the
  // EFB's clip space.
  const float* m = mtx34(v.pnmtx);
  const float* p = v.pos;
  float view[3];
  for (int r = 0; r < 3; ++r) view[r] = m[4 * r] * p[0] + m[4 * r + 1] * p[1] + m[4 * r + 2] * p[2] + m[4 * r + 3];

  const float* pr = x.pr;
  float cx, cy, cz, cw;
  if (x.ortho) {
    cx = pr[0] * view[0] + pr[1];
    cy = pr[2] * view[1] + pr[3];
    cz = pr[4] * view[2] + pr[5];
    cw = 1.0f;
  } else {
    cx = pr[0] * view[0] + pr[1] * view[2];
    cy = pr[2] * view[1] + pr[3] * view[2];
    cz = pr[4] * view[2] + pr[5];
    cw = -view[2];
  }
  const float hw = static_cast<float>(kEfbWidth) / 2, hh = static_cast<float>(kEfbHeight) / 2;
  // Window x = cx / cw * wd + xo - ox; EFB clip x = (window x / hw - 1) * cw.
  o.pos[0] = (cx * x.wd + cw * (x.xo - x.ox)) / hw - cw;
  // Window y grows downwards; the EFB clip space's y grows upwards.
  o.pos[1] = cw - (cy * x.ht + cw * (x.yo - x.oy)) / hh;
  // Depth: window z = cz / cw * zrange + zfar, in 24-bit units.
  o.pos[2] = (cz * x.zr + cw * x.zf) / 16777215.0f;
  o.pos[3] = cw;

  // Normal (3x3 at 0x400 + (index & 31) * 3), only when lighting needs it.
  float n[3] = {v.nrm[0], v.nrm[1], v.nrm[2]};
  if (x.lit) {
    const u32 ni = v.pnmtx & 31u;
    if (!(x.have_nrm >> ni & 1)) {
      const u32 nm = xf::kNrmMtx + ni * 3u;
      for (int k = 0; k < 9; ++k) x.nrm[ni][k] = xf_float(nm + static_cast<u32>(k));
      x.have_nrm |= 1ull << ni;
    }
    const float* q = x.nrm[ni];
    float t[3];
    for (int r = 0; r < 3; ++r) t[r] = q[3 * r] * v.nrm[0] + q[3 * r + 1] * v.nrm[1] + q[3 * r + 2] * v.nrm[2];
    n[0] = t[0];
    n[1] = t[1];
    n[2] = t[2];
    normalize3(n);
  }

  // Lighting: color channels 0 and 1, color and alpha separately. A light's
  // factor (attenuation times diffuse term) depends only on the light, the
  // vertex and the two functions: computed once per vertex and reused by the
  // other passes that use the light with the same functions (P1).
  float lf[8];
  u8 lf_fn[8];
  u32 lf_have = 0;
  for (u32 ch = 0; ch < 2 && ch < x.nchan; ++ch) {
    for (int is_alpha = 0; is_alpha < 2; ++is_alpha) {
      const XformSetup::Chan& cs = x.chan[ch][is_alpha];
      const u32 ctl = cs.ctl;
      float mat[4], amb[4];
      if (bits(ctl, 0, 1)) std::memcpy(mat, v.color[ch], sizeof mat);
      else std::memcpy(mat, cs.mat, sizeof mat);
      float out[4];
      std::memcpy(out, mat, sizeof out);
      if (bits(ctl, 1, 1)) {
        if (bits(ctl, 6, 1)) std::memcpy(amb, v.color[ch], sizeof amb);
        else std::memcpy(amb, cs.amb, sizeof amb);
        float acc[4];
        std::memcpy(acc, amb, sizeof acc);
        const u32 mask = bits(ctl, 2, 4) | bits(ctl, 11, 4) << 4;
        const u32 diffuse = bits(ctl, 7, 2), attn_fn = bits(ctl, 9, 2);
        const u8 fn = static_cast<u8>(diffuse << 2 | attn_fn);
        for (u32 l = 0; l < 8; ++l) {
          if (!(mask & (1u << l))) continue;
          const XformSetup::Light& L = x.lights[l];
          if ((lf_have >> l & 1) && lf_fn[l] == fn) {
            const float f = lf[l];
            if (is_alpha) acc[3] += f * L.col[3];
            else
              for (int k = 0; k < 3; ++k) acc[k] += f * L.col[k];
            continue;
          }
          float ldir[3] = {L.pos[0] - view[0], L.pos[1] - view[1], L.pos[2] - view[2]};
          float attn = 1.0f;
          if (attn_fn == 3) {  // spot
            float dist2 = dot3(ldir, ldir), dist = std::sqrt(dist2);
            if (dist > 0) {
              ldir[0] /= dist;
              ldir[1] /= dist;
              ldir[2] /= dist;
            }
            float c = std::max(0.0f, dot3(ldir, L.dir));
            float num = std::max(0.0f, L.cosatt[0] + L.cosatt[1] * c + L.cosatt[2] * c * c);
            float den = L.distatt[0] + L.distatt[1] * dist + L.distatt[2] * dist2;
            attn = den != 0 ? num / den : 0.0f;
          } else if (attn_fn == 1) {  // specular
            normalize3(ldir);
            float c = dot3(n, ldir) >= 0 ? std::max(0.0f, dot3(n, L.dir)) : 0.0f;
            float da[3] = {L.distatt[0], L.distatt[1], L.distatt[2]};
            if (diffuse != 0) normalize3(da);
            float num = std::max(0.0f, L.cosatt[0] + L.cosatt[1] * c + L.cosatt[2] * c * c);
            float den = da[0] + da[1] * c + da[2] * c * c;
            attn = den != 0 ? num / den : 0.0f;
          } else {
            normalize3(ldir);
          }
          float d = 1.0f;
          if (diffuse == 1) d = dot3(ldir, n);
          else if (diffuse == 2) d = std::max(0.0f, dot3(ldir, n));
          const float f = attn * d;
          lf[l] = f;
          lf_fn[l] = fn;
          lf_have |= 1u << l;
          if (is_alpha) acc[3] += f * L.col[3];
          else
            for (int k = 0; k < 3; ++k) acc[k] += f * L.col[k];
        }
        // Only the components this pass writes (rgb for color, a for alpha).
        if (is_alpha) out[3] = mat[3] * std::clamp(acc[3], 0.0f, 1.0f);
        else
          for (int k = 0; k < 3; ++k) out[k] = mat[k] * std::clamp(acc[k], 0.0f, 1.0f);
      }
      if (is_alpha) o.color[ch][3] = out[3];
      else {
        o.color[ch][0] = out[0];
        o.color[ch][1] = out[1];
        o.color[ch][2] = out[2];
      }
    }
  }

  // Texture coordinate generation.
  for (u32 i = 0; i < x.ngen && i < 8; ++i) {
    const u32 info = x.texgen[i];
    const u32 type = bits(info, 4, 3), row = bits(info, 7, 5);
    float out[3] = {0, 0, 1};
    if (type == 0) {
      float src[4] = {0, 0, 1, 1};
      switch (row) {
        case 0: src[0] = v.pos[0]; src[1] = v.pos[1]; src[2] = v.pos[2]; break;
        case 1: src[0] = v.nrm[0]; src[1] = v.nrm[1]; src[2] = v.nrm[2]; break;
        case 2: src[0] = o.color[0][0]; src[1] = o.color[0][1]; src[2] = o.color[0][2]; break;
        case 3: src[0] = v.tan[0]; src[1] = v.tan[1]; src[2] = v.tan[2]; break;
        case 4: src[0] = v.bin[0]; src[1] = v.bin[1]; src[2] = v.bin[2]; break;
        default:
          if (row >= 5 && row <= 12) {
            src[0] = v.tex[row - 5][0];
            src[1] = v.tex[row - 5][1];
          }
          break;
      }
      if (!bits(info, 2, 1)) src[2] = 1.0f;  // AB11
      const float* mtx = mtx34(v.texmtx[i]);
      bool stq = bits(info, 1, 1);
      for (int r = 0; r < (stq ? 3 : 2); ++r)
        out[r] = mtx[4 * r] * src[0] + mtx[4 * r + 1] * src[1] + mtx[4 * r + 2] * src[2] + mtx[4 * r + 3] * src[3];
    } else if (type == 1) {  // emboss: the source coordinate (the light offset is not modeled yet)
      u32 s = bits(info, 12, 3);
      if (s < i) std::memcpy(out, o.tex[s], sizeof out);
    } else {  // color strgb
      const float* col = o.color[type == 2 ? 0 : 1];
      out[0] = col[0];
      out[1] = col[1];
      out[2] = 1.0f;
    }
    if (x.dual && type == 0) {
      const u32 post = x.post[i];
      if (bits(post, 8, 1)) normalize3(out);
      const float* mtx = x.postmtx[i];
      float r3[3];
      for (int r = 0; r < 3; ++r)
        r3[r] = mtx[4 * r] * out[0] + mtx[4 * r + 1] * out[1] + mtx[4 * r + 2] * out[2] + mtx[4 * r + 3];
      std::memcpy(out, r3, sizeof out);
    }
    std::memcpy(o.tex[i], out, sizeof out);
  }
  for (u32 i = std::min<u32>(x.ngen, 8); i < 8; ++i) std::memset(o.tex[i], 0, sizeof o.tex[i]);
}

void GxProcessor::snapshot(DrawCall& d) const {
  PipelineConfig& c = d.cfg;
  c = PipelineConfig{};
  std::memset(&c, 0, sizeof c);
  const u32 gen = bp_[bp::kGenMode];
  c.num_texgens = static_cast<u8>(bits(gen, 0, 4));
  c.num_chans = static_cast<u8>(bits(gen, 4, 3));
  c.num_stages = static_cast<u8>(bits(gen, 10, 4) + 1);
  c.cull = static_cast<u8>(bits(gen, 14, 2));
  c.num_ind = static_cast<u8>(bits(gen, 16, 3));
  std::array<bool, 8> map_used{};
  for (int i = 0; i < c.num_stages; ++i) {
    TevStage& s = c.stages[i];
    s.color_env = bp_[bp::kTevColorEnv + 2 * i];
    s.alpha_env = bp_[bp::kTevAlphaEnv + 2 * i];
    s.ind_cmd = bp_[bp::kIndCmd + i];
    const u32 tref = bp_[bp::kTref + i / 2];
    const int sh = (i & 1) ? 12 : 0;
    s.tex_enable = static_cast<u8>(bits(tref, sh + 6, 1));
    s.texmap = static_cast<u8>(bits(tref, sh, 3));
    s.texcoord = static_cast<u8>(bits(tref, sh + 3, 3));
    s.ras_chan = static_cast<u8>(bits(tref, sh + 7, 3));
    if (!s.tex_enable && !s.ind_cmd) s.texmap = s.texcoord = 0;
    if (s.tex_enable) map_used[s.texmap] = true;
    const u32 ksel = bp_[bp::kKsel + i / 2];
    s.kcsel = static_cast<u8>(bits(ksel, (i & 1) ? 14 : 4, 5));
    s.kasel = static_cast<u8>(bits(ksel, (i & 1) ? 19 : 9, 5));
  }
  for (int t = 0; t < 4; ++t) {
    const u32 s0 = bp_[bp::kKsel + 2 * t], s1 = bp_[bp::kKsel + 2 * t + 1];
    c.swap[t][0] = static_cast<u8>(bits(s0, 0, 2));
    c.swap[t][1] = static_cast<u8>(bits(s0, 2, 2));
    c.swap[t][2] = static_cast<u8>(bits(s1, 0, 2));
    c.swap[t][3] = static_cast<u8>(bits(s1, 2, 2));
  }
  const u32 ac = bp_[bp::kAlphaCompare];
  c.alpha_comp0 = static_cast<u8>(bits(ac, 16, 3));
  c.alpha_comp1 = static_cast<u8>(bits(ac, 19, 3));
  c.alpha_logic = static_cast<u8>(bits(ac, 22, 2));
  const u32 cm = bp_[bp::kCMode0];
  c.blend_enable = static_cast<u8>(bits(cm, 0, 1));
  c.logic_enable = static_cast<u8>(bits(cm, 1, 1));
  c.color_update = static_cast<u8>(bits(cm, 3, 1));
  c.alpha_update = static_cast<u8>(bits(cm, 4, 1));
  c.blend_dst = static_cast<u8>(bits(cm, 5, 3));
  c.blend_src = static_cast<u8>(bits(cm, 8, 3));
  c.blend_sub = static_cast<u8>(bits(cm, 11, 1));
  c.logic_op = static_cast<u8>(bits(cm, 12, 4));
  const u32 zm = bp_[bp::kZMode];
  c.z_enable = static_cast<u8>(bits(zm, 0, 1));
  c.z_func = static_cast<u8>(bits(zm, 1, 3));
  c.z_update = static_cast<u8>(bits(zm, 4, 1));
  c.pixel_format = static_cast<u8>(bits(bp_[bp::kZCompare], 0, 3));
  c.fog_type = static_cast<u8>(bits(bp_[bp::kFog3], 21, 3));
  c.fog_proj = static_cast<u8>(bits(bp_[bp::kFog3], 20, 1));
  if (c.num_ind) {
    const u32 iref = bp_[bp::kIref];
    for (int s = 0; s < 4 && s < c.num_ind; ++s) {
      c.iref_map[s] = static_cast<u8>(bits(iref, 6 * s, 3));
      c.iref_coord[s] = static_cast<u8>(bits(iref, 6 * s + 3, 3));
      const u32 sc = bp_[s < 2 ? bp::kIndScale0 : bp::kIndScale1];
      c.ind_scale[s] = static_cast<u8>(bits(sc, (s & 1) * 8, 8));
      map_used[c.iref_map[s]] = true;
    }
  }

  DrawUniforms& u = d.u;
  std::memcpy(u.tev_regs, tev_regs_, sizeof u.tev_regs);
  std::memcpy(u.konst, konst_, sizeof u.konst);
  u.alpha_ref[0] = static_cast<std::int32_t>(bits(ac, 0, 8));
  u.alpha_ref[1] = static_cast<std::int32_t>(bits(ac, 8, 8));
  const u32 cm1 = bp_[bp::kCMode1];
  u.dst_alpha[0] = static_cast<std::int32_t>(bits(cm1, 8, 1));
  u.dst_alpha[1] = static_cast<std::int32_t>(bits(cm1, 0, 8));
  // Fog: A and C are 20-bit floats (sign, 8-bit exponent, 11-bit mantissa).
  auto fog_float = [](u32 v) {
    return std::bit_cast<float>(bits(v, 19, 1) << 31 | bits(v, 11, 8) << 23 | bits(v, 0, 11) << 12);
  };
  u.fog[0] = fog_float(bp_[bp::kFog0]);
  u.fog[1] = static_cast<float>(bits(bp_[bp::kFog1], 0, 24));
  u.fog[2] = static_cast<float>(bits(bp_[bp::kFog2], 0, 5));
  u.fog[3] = fog_float(bp_[bp::kFog3]);
  u.fog[4] = static_cast<float>(bits(bp_[bp::kFogColor], 16, 8));
  u.fog[5] = static_cast<float>(bits(bp_[bp::kFogColor], 8, 8));
  u.fog[6] = static_cast<float>(bits(bp_[bp::kFogColor], 0, 8));
  for (int m = 0; m < 3; ++m) {
    u32 r0 = bp_[bp::kIndMtx + 3 * m], r1 = bp_[bp::kIndMtx + 3 * m + 1], r2 = bp_[bp::kIndMtx + 3 * m + 2];
    const std::int32_t shift = 17 - static_cast<std::int32_t>(bits(r0, 22, 2) | bits(r1, 22, 2) << 2 | bits(r2, 22, 2) << 4);
    // Rows: (ma, mc, me), (mb, md, mf); elements are 11-bit signed.
    std::int32_t* row0 = u.ind_mtx[2 * m];
    std::int32_t* row1 = u.ind_mtx[2 * m + 1];
    row0[0] = sext11(bits(r0, 0, 11));
    row0[1] = sext11(bits(r1, 0, 11));
    row0[2] = sext11(bits(r2, 0, 11));
    row0[3] = shift;
    row1[0] = sext11(bits(r0, 11, 11));
    row1[1] = sext11(bits(r1, 11, 11));
    row1[2] = sext11(bits(r2, 11, 11));
    row1[3] = shift;
  }

  for (int m = 0; m < 8; ++m) {
    TextureDesc& t = d.tex[static_cast<std::size_t>(m)];
    t = TextureDesc{};
    const u32 img0 = bp_[bp::tex_reg(bp::kTexImage0, m)];
    t.width = static_cast<u16>(bits(img0, 0, 10) + 1);
    t.height = static_cast<u16>(bits(img0, 10, 10) + 1);
    u.tex_scale[m][2] = t.width;
    u.tex_scale[m][3] = t.height;
    u.tex_scale[m][0] = static_cast<float>(bits(bp_[bp::kSuSSize + 2 * m], 0, 16) + 1);
    u.tex_scale[m][1] = static_cast<float>(bits(bp_[bp::kSuTSize + 2 * m], 0, 16) + 1);
    u.lod_bias[m] = 0;
    if (!map_used[static_cast<std::size_t>(m)]) continue;
    t.valid = true;
    t.format = static_cast<u8>(bits(img0, 20, 4));
    t.addr = (bp_[bp::tex_reg(bp::kTexImage3, m)] & 0xFFFFFF) << 5;
    const u32 img1 = bp_[bp::tex_reg(bp::kTexImage1, m)];
    if (bits(img1, 21, 1)) {  // preloaded: the texels are in TMEM
      t.tmem = tmem_.data();
      t.tmem_even = bits(img1, 0, 15) << 5;
      t.tmem_odd = bits(bp_[bp::tex_reg(bp::kTexImage2, m)], 0, 15) << 5;
    }
    const u32 mode0 = bp_[bp::tex_reg(bp::kTexMode0, m)], mode1 = bp_[bp::tex_reg(bp::kTexMode1, m)];
    t.wrap_s = static_cast<u8>(bits(mode0, 0, 2));
    t.wrap_t = static_cast<u8>(bits(mode0, 2, 2));
    t.mag_linear = static_cast<u8>(bits(mode0, 4, 1));
    t.min_filter = static_cast<u8>(bits(mode0, 5, 3));
    t.lod_bias = static_cast<float>(static_cast<std::int8_t>(bits(mode0, 9, 8))) / 32.0f;
    u.lod_bias[m] = t.lod_bias;  // Q1: WebGPU samplers have no bias; the shader adds it (textureSampleBias)
    t.min_lod = static_cast<float>(bits(mode1, 0, 8)) / 16.0f;
    t.max_lod = static_cast<float>(bits(mode1, 8, 8)) / 16.0f;
    t.levels = 1;
    if (bits(t.min_filter, 0, 2) != 0) {
      u32 max_levels = 1;
      for (u32 s = std::max(t.width, t.height); s > 1; s >>= 1) ++max_levels;
      t.levels = static_cast<u8>(std::clamp<u32>(static_cast<u32>(t.max_lod) + 1, 1, max_levels));
    }
    if (t.format == 8 || t.format == 9 || t.format == 10) {
      const u32 tl = bp_[bp::tex_reg(bp::kTexTlut, m)];
      t.tlut_offset = (tl & 0x3FF) << 9;
      t.tlut_format = static_cast<u8>(bits(tl, 10, 2));
      t.tlut_entries = t.format == 8 ? 16 : t.format == 9 ? 256 : 16384;
      u32 room = static_cast<u32>(tlut_.size()) - std::min<u32>(t.tlut_offset, static_cast<u32>(tlut_.size()));
      t.tlut_entries = std::min(t.tlut_entries, room / 2);
      t.tlut = tlut_.data() + std::min<u32>(t.tlut_offset, static_cast<u32>(tlut_.size()));
    }
  }

  // Scissor, in EFB pixels (the offset moves it with the primitives).
  const int ox = static_cast<int>(bits(bp_[bp::kScissorOffset], 0, 10) * 2);
  const int oy = static_cast<int>(bits(bp_[bp::kScissorOffset], 10, 10) * 2);
  int x0 = static_cast<int>(bits(bp_[bp::kScissorTL], 12, 11)) - ox;
  int y0 = static_cast<int>(bits(bp_[bp::kScissorTL], 0, 11)) - oy;
  int x1 = static_cast<int>(bits(bp_[bp::kScissorBR], 12, 11)) - ox + 1;
  int y1 = static_cast<int>(bits(bp_[bp::kScissorBR], 0, 11)) - oy + 1;
  d.scissor[0] = static_cast<u32>(std::clamp(x0, 0, static_cast<int>(kEfbWidth)));
  d.scissor[1] = static_cast<u32>(std::clamp(y0, 0, static_cast<int>(kEfbHeight)));
  d.scissor[2] = static_cast<u32>(std::clamp(x1, static_cast<int>(d.scissor[0]), static_cast<int>(kEfbWidth)));
  d.scissor[3] = static_cast<u32>(std::clamp(y1, static_cast<int>(d.scissor[1]), static_cast<int>(kEfbHeight)));
}

void GxProcessor::primitive(u8 op, const u8* data, u32 count) {
  const int fmt = op & 7;
  const u8 kind = op & 0xF8;
  if (!(fmt_valid_ & (1u << fmt))) {
    fmt_cache_[fmt] = vertex_format(fmt);
    fmt_size_[fmt] = vertex_size(fmt);
    fmt_valid_ |= static_cast<u8>(1u << fmt);
  }
  VertexFormat& vf = fmt_cache_[fmt];
  // The default matrix indices come from CP registers written between draws.
  vf.default_pnmtx = static_cast<u8>(bits(matidx_a_, 0, 6));
  for (int k = 0; k < 8; ++k) {
    u32 src = k < 4 ? matidx_a_ : matidx_b_;
    vf.default_texmtx[k] = static_cast<u8>(bits(src, k < 4 ? 6 + 6 * k : 6 * (k - 4), 6));
  }
  const u32 vsize = fmt_size_[fmt];
  stats_.vertices += count;
  if (count == 0) return;
  DrawCall& d = draw_;
  // Each vertex is loaded and transformed once; assembly below only indexes them.
  std::vector<OutVertex>& vs = d.verts;
  vs.resize(count);
  if (!xs_valid_) {
    setup_transform(xs_);
    xs_valid_ = true;
  }
  for (u32 k = 0; k < count; ++k) {
    InVertex in;
    load_vertex(vf, data + static_cast<std::size_t>(k) * vsize, in);
    transform(in, xs_, vs[k]);
  }

  if (keep_prim_info_) {
    prim_info_.op = op;
    prim_info_.count = count;
    prim_info_.vertex_size = vsize;
    prim_info_.vcd_lo = vcd_lo_;
    prim_info_.vcd_hi = vcd_hi_;
    prim_info_.vat[0] = vat_a_[fmt];
    prim_info_.vat[1] = vat_b_[fmt];
    prim_info_.vat[2] = vat_c_[fmt];
    prim_info_.matidx[0] = matidx_a_;
    prim_info_.matidx[1] = matidx_b_;
    prim_info_.texgen[0] = xf_word(xf::kTexGen);
    prim_info_.texgen[1] = xf_word(xf::kTexGen + 1);
    prim_info_.num_texgens = xf_word(xf::kNumTexGens);
    prim_info_.dual_tex = xf_word(xf::kDualTex);
    for (int k = 0; k < 6; ++k) {
      prim_info_.array_base[k] = array_base_[static_cast<std::size_t>(k)];
      prim_info_.array_stride[k] = array_stride_[static_cast<std::size_t>(k)];
    }
    prim_info_.first_vertex.assign(data, data + vsize);
  }

  // The pixel state: rebuilt only when a register it reads was written since
  // the last draw (consecutive primitives usually share it).
  if (snap_serial_ != state_serial_) {
    d.u = DrawUniforms{};
    snapshot(d);
    snap_serial_ = state_serial_;
    ++stats_.snapshots;
  }
  d.state = state_serial_;
  std::vector<u32>& ix = d.index;
  ix.clear();
  auto tri = [&](u32 a, u32 b, u32 c) {
    ix.push_back(a);
    ix.push_back(b);
    ix.push_back(c);
  };
  Topology topo = Topology::Triangles;
  switch (kind) {
    case cmd::kQuads: case cmd::kQuads2:
      for (u32 k = 0; k + 3 < count; k += 4) {
        tri(k, k + 1, k + 2);
        tri(k, k + 2, k + 3);
      }
      break;
    case cmd::kTriangles:
      for (u32 k = 0; k + 2 < count; k += 3) tri(k, k + 1, k + 2);
      break;
    case cmd::kTriStrip:
      for (u32 k = 2; k < count; ++k) {
        if (k & 1) tri(k - 1, k - 2, k);
        else tri(k - 2, k - 1, k);
      }
      break;
    case cmd::kTriFan:
      for (u32 k = 2; k < count; ++k) tri(0, k - 1, k);
      break;
    case cmd::kLines:
      topo = Topology::Lines;
      for (u32 k = 0; k + 1 < count; k += 2) {
        ix.push_back(k);
        ix.push_back(k + 1);
      }
      break;
    case cmd::kLineStrip:
      topo = Topology::Lines;
      for (u32 k = 1; k < count; ++k) {
        ix.push_back(k - 1);
        ix.push_back(k);
      }
      break;
    default:  // points
      topo = Topology::Points;
      for (u32 k = 0; k < count; ++k) ix.push_back(k);
      break;
  }
  d.cfg.topology = static_cast<u8>(topo);
  if (topo == Topology::Triangles) stats_.triangles += ix.size() / 3;
  ++stats_.draws;
  if (backend_ && !ix.empty()) backend_->draw(d);
}

}  // namespace mvp::gfx
