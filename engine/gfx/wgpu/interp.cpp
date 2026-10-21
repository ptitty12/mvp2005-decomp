// In-between frames (V3): the frame record, the matching of two frames' geometry
// and the replay of a frame with its vertices halfway (renderer.h, "In-between
// frames"). Presentation only: the live frame's draws, copies and results are
// untouched, and nothing here reaches guest state.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>

#include "gfx/tev_wgsl.h"
#include "gfx/wgpu/gpu.h"
#include "gfx/wgpu/gpu_timer.h"
#include "gfx/wgpu/renderer.h"

namespace mvp::gfx {

namespace {

constexpr std::uint64_t kSlot = 256;                  // clear and conversion constants per op
constexpr std::uint64_t kMaxReplayBytes = 96u << 20;  // a frame's vertex + index + constant bytes (else no in-between)

bool has_alpha(std::uint8_t pixel_format) { return pixel_format == 1; }  // RGBA6_Z24

// A clip-space position on screen (NDC), if in front of the camera.
inline bool ndc(const float* v, float& x, float& y) {
  const float w = v[3];
  if (!(w > 1e-6f)) return false;
  x = v[0] / w;
  y = v[1] / w;
  return true;
}

// How far apart two vertices are on screen (NDC units); large when only one is in front.
inline float screen_dist(const float* a, const float* b) {
  float ax, ay, bx, by;
  const bool ia = ndc(a, ax, ay), ib = ndc(b, bx, by);
  if (ia && ib) return std::sqrt((ax - bx) * (ax - bx) + (ay - by) * (ay - by));
  if (!ia && !ib) return std::memcmp(a, b, 16) == 0 ? 0.0f : 1.0f;
  return 1e9f;
}

}  // namespace

void Renderer::set_interp(bool on) {
  if (on == interp_on_) return;
  interp_on_ = on;
  if (on) return;  // recording starts at the next XFB copy
  rec_drop();
  // Off: the private targets go (they are made again when it is turned on).
  priv_copies_.clear();
  ibind_.clear();
  subst_.clear();
  iefb_color_ = nullptr;
  iefb_depth_ = nullptr;
  iefb_color_view_ = nullptr;
  iefb_depth_view_ = nullptr;
  iefb_scale_ = 0;
  for (Between& b : between_) b = Between{};
  between_mixed_ = Frame{};
  between_mixed_key_ = ~0ull;
  ivbuf_ = iibuf_ = iubuf_ = iclear_buf_ = iconv_buf_ = nullptr;
  ivcap_ = iicap_ = iucap_ = iclear_cap_ = iconv_cap_ = 0;
  iuniform_group_ = nullptr;
}

void Renderer::rec_drop() {
  recording_ = false;
  for (Record& r : rec_) {
    r.valid = false;
    r.binds.clear();  // the textures they hold
    r.bind_index.clear();
    r.copies.clear();
  }
}

void Renderer::rec_begin(const EfbCopy& c) {
  Record& r = rec_[rec_cur_];
  r.valid = true;
  r.id = ++rec_ids_;
  r.field = r.serial = 0;
  r.scale = scale_;
  r.ops.clear();
  r.npasses = 0;
  r.clears.clear();
  r.copies.clear();
  r.binds.clear();
  r.bind_index.clear();
  r.ranges.clear();
  r.pos.clear();
  r.pass_serial = 0;
  recording_ = true;
  // The frame starts with the EFB as this copy's clear leaves it (recorded next). A
  // copy that does not clear color and depth leaves the EFB as it is now: the private
  // EFB starts as a copy of it.
  const bool alpha = has_alpha(c.pixel_format) ? c.clear_alpha : true;
  if (c.clear && c.clear_color && c.clear_depth && alpha) return;
  interp_targets();
  wgpu::CommandEncoder enc = gpu_.device().CreateCommandEncoder();
  const wgpu::Extent3D ext{kEfbWidth * scale_, kEfbHeight * scale_, 1};
  wgpu::TexelCopyTextureInfo src{}, dst{};
  src.texture = efb_color_;
  dst.texture = iefb_color_;
  enc.CopyTextureToTexture(&src, &dst, &ext);
  src.texture = efb_depth_;
  dst.texture = iefb_depth_;
  enc.CopyTextureToTexture(&src, &dst, &ext);
  wgpu::CommandBuffer cb = enc.Finish();
  gpu_.queue().Submit(1, &cb);
}

std::uint32_t Renderer::rec_bind(const BindEntry& be) {
  Record& r = rec_[rec_cur_];
  auto [it, fresh] = r.bind_index.try_emplace(be.group.Get(), static_cast<std::uint32_t>(r.binds.size()));
  if (fresh) r.binds.push_back({be.group, be.views, be.samplers});
  return it->second;
}

void Renderer::rec_range(const DrawCall& d, std::size_t voff, std::uint32_t stride) {
  Record& r = rec_[rec_cur_];
  if (r.npasses == 0) return;
  const std::uint32_t n = static_cast<std::uint32_t>(d.verts.size());
  const std::size_t at = r.pos.size();
  r.pos.resize(at + 4 * static_cast<std::size_t>(n));
  float* out = r.pos.data() + at;
  std::uint32_t n3 = 0;
  for (const OutVertex& v : d.verts) {
    std::memcpy(out, v.pos, 16);
    n3 += v.pos[3] > 1e-6f && v.pos[3] != 1.0f;
    out += 4;
  }
  r.ranges.push_back({d.ident, static_cast<std::uint32_t>(r.npasses - 1), static_cast<std::uint32_t>(voff), n, stride,
                      static_cast<std::uint32_t>(at / 4), n3});
}

void Renderer::rec_draw() {
  Record& r = rec_[rec_cur_];
  if (r.npasses == 0 || r.pass_serial != pass_serial_) return;
  RecDraw d;
  d.pipe = static_cast<const Pipeline*>(pending_.pipe);
  d.bind = pending_.bind;
  d.uoff = pending_.uoff;
  d.first_index = pending_.first_index;
  d.index_count = pending_.index_count;
  d.base = pending_.base;
  std::memcpy(d.scissor, pending_.scissor, sizeof d.scissor);
  r.passes[r.npasses - 1].draws.push_back(d);
}

void Renderer::rec_pass_end() {
  Record& r = rec_[rec_cur_];
  if (r.npasses == 0 || r.pass_serial != pass_serial_) return;
  RecPass& p = r.passes[r.npasses - 1];
  // The bytes were uploaded; the record takes them (and the pass gets the record's
  // old buffers, cleared at its next begin_pass()).
  std::swap(p.v, vdata_);
  std::swap(p.i, idata_);
  std::swap(p.u, udata_);
  p.filled = true;
}

void Renderer::rec_clear(int m, const float* data, const std::uint32_t* rect) {
  Record& r = rec_[rec_cur_];
  RecClear c;
  c.m = m;
  std::memcpy(c.data, data, sizeof c.data);
  std::memcpy(c.rect, rect, sizeof c.rect);
  r.ops.push_back({RecKind::Clear, static_cast<std::uint32_t>(r.clears.size())});
  r.clears.push_back(c);
}

void Renderer::rec_copy(const wgpu::TextureView& view, std::uint32_t pw, std::uint32_t ph,
                        const std::int32_t* params) {
  Record& r = rec_[rec_cur_];
  RecCopy c;
  c.live = view;
  c.pw = pw;
  c.ph = ph;
  std::memcpy(c.params, params, sizeof c.params);
  r.ops.push_back({RecKind::Copy, static_cast<std::uint32_t>(r.copies.size())});
  r.copies.push_back(std::move(c));
}

void Renderer::rec_end(std::uint32_t x0, std::uint32_t y0, std::uint32_t w, std::uint32_t h, std::uint64_t serial) {
  Record& cur = rec_[rec_cur_];
  Record& prev = rec_[rec_cur_ ^ 1];
  recording_ = false;
  if (!cur.valid) return;
  cur.field = fields_;
  cur.serial = serial;
  cur.xfb[0] = x0;
  cur.xfb[1] = y0;
  cur.xfb[2] = w;
  cur.xfb[3] = h;
  ++istats_.frames;
  const auto t0 = std::chrono::steady_clock::now();
  if (!prev.valid || prev.field + 1 != cur.field) ++istats_.gap;
  else if (prev.scale != cur.scale || cur.scale != scale_) ++istats_.other;
  else interp_make(cur, prev);
  istats_.ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
  // This frame is the one before the next (its positions are what the next one
  // matches); the textures its draws bound are not needed any more.
  cur.binds.clear();
  cur.bind_index.clear();
  cur.copies.clear();
  rec_cur_ ^= 1;
}

wgpu::Buffer Renderer::interp_buffer(wgpu::Buffer& buf, std::uint64_t& cap, std::uint64_t need, wgpu::BufferUsage usage,
                                     const char* label) {
  if (buf && cap >= need) return buf;
  std::uint64_t c = 1u << 20;
  while (c < need) c *= 2;
  wgpu::BufferDescriptor bd{};
  bd.size = c;
  bd.usage = usage | wgpu::BufferUsage::CopyDst;
  bd.label = label;
  buf = gpu_.device().CreateBuffer(&bd);
  cap = c;
  return buf;
}

void Renderer::interp_targets() {
  if (iefb_scale_ == scale_ && iefb_color_) return;
  wgpu::TextureDescriptor td{};
  td.size = {kEfbWidth * scale_, kEfbHeight * scale_, 1};
  td.format = wgpu::TextureFormat::RGBA8Unorm;
  td.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
             wgpu::TextureUsage::TextureBinding;
  td.label = "in-between efb color";
  iefb_color_ = gpu_.device().CreateTexture(&td);
  iefb_color_view_ = iefb_color_.CreateView();
  td.format = wgpu::TextureFormat::Depth32Float;
  td.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
  td.label = "in-between efb depth";
  iefb_depth_ = gpu_.device().CreateTexture(&td);
  iefb_depth_view_ = iefb_depth_.CreateView();
  iefb_scale_ = scale_;
}

const wgpu::BindGroup& Renderer::interp_bind(const RecBind& b) {
  if (subst_.empty()) return b.group;
  std::array<const void*, 16> key{};
  bool any = false;
  for (std::size_t k = 0; k < 8; ++k) {
    const void* v = b.views[k].Get();
    auto it = subst_.find(v);
    if (it != subst_.end()) {
      v = it->second.Get();
      any = true;
    }
    key[k] = v;
    key[8 + k] = b.samplers[k].Get();
  }
  if (!any) return b.group;
  if (ibind_.size() > 1024) ibind_.clear();
  BindEntry& e = ibind_[key];
  if (!e.group) {
    wgpu::BindGroupEntry te[16]{};
    for (std::size_t k = 0; k < 8; ++k) {
      auto it = subst_.find(b.views[k].Get());
      e.views[k] = it != subst_.end() ? it->second : b.views[k];
      e.samplers[k] = b.samplers[k];
      te[k].binding = static_cast<std::uint32_t>(k);
      te[k].textureView = e.views[k];
      te[8 + k].binding = static_cast<std::uint32_t>(8 + k);
      te[8 + k].sampler = e.samplers[k];
    }
    wgpu::BindGroupDescriptor bgd{};
    bgd.layout = bgl_textures_;
    bgd.entryCount = 16;
    bgd.entries = te;
    e.group = gpu_.device().CreateBindGroup(&bgd);
  }
  return e.group;
}

bool Renderer::interp_make(Record& cur, Record& prev) {
  const auto t0 = std::chrono::steady_clock::now();
  const std::vector<RecRange>& C = cur.ranges;
  const std::vector<RecRange>& P = prev.ranges;
  if (C.empty() || cur.npasses == 0) {
    ++istats_.other;
    return false;
  }

  // ---- Matching: each draw of this frame with one of the frame before. ----
  // The k-th draw of an identity here pairs with the k-th draw of that identity there (a
  // table of the frame before's identities, each with its draws in order). Where some of
  // an identity's draws moved far enough to have changed places (an order pair far apart
  // on screen, or of another vertex count), each takes the nearest one left instead, if
  // that moves them less than half as far in all: the game drew them in another order.
  // Identical objects that all moved (a crowd in a camera pan) stay in order.
  auto first_dist = [&](const RecRange& c, const RecRange& p) {
    return screen_dist(&cur.pos[4 * static_cast<std::size_t>(c.pos)], &prev.pos[4 * static_cast<std::size_t>(p.pos)]);
  };
  const float reorder = interp_tune.reorder_shift;
  std::size_t cap = 64;
  while (cap < 2 * P.size()) cap *= 2;
  slots_.assign(cap, Slot{});
  pnext_.assign(P.size(), ~0u);
  cnext_.assign(C.size(), ~0u);
  match_.assign(C.size(), ~0u);
  taken_.assign(P.size(), 0);
  const std::size_t mask = cap - 1;
  auto find = [&](std::uint64_t id) -> Slot& {
    std::size_t h = static_cast<std::size_t>(id ^ (id >> 31)) & mask;
    while (slots_[h].ident != 0 && slots_[h].ident != id) h = (h + 1) & mask;
    return slots_[h];
  };
  for (std::uint32_t k = 0; k < P.size(); ++k) {
    const std::uint64_t id = P[k].ident;
    if (id == 0) continue;
    Slot& sl = find(id);
    if (sl.ident == 0) {
      sl.ident = id;
      sl.phead = sl.ptail = sl.cursor = k;
    } else {
      pnext_[sl.ptail] = k;
      sl.ptail = k;
    }
    ++sl.np;
  }
  far_.clear();
  for (std::uint32_t k = 0; k < C.size(); ++k) {
    const RecRange& c = C[k];
    if (c.ident == 0) continue;
    Slot& sl = find(c.ident);
    if (sl.ident == 0) continue;  // not in the frame before
    if (sl.nc++ == 0) sl.chead = k;
    else cnext_[sl.ctail] = k;
    sl.ctail = k;
    const std::uint32_t p = sl.cursor;
    bool far = p == ~0u;  // more of them here than there
    if (p != ~0u) {
      sl.cursor = pnext_[p];
      if (P[p].count == c.count) match_[k] = p;
      far = P[p].count != c.count || first_dist(c, P[p]) > reorder;
    }
    if (far && !sl.far) {
      sl.far = 1;
      far_.push_back(static_cast<std::uint32_t>(&sl - slots_.data()));
    }
  }
  for (std::uint32_t si : far_) {
    const Slot& sl = slots_[si];
    if (sl.np < 2 && sl.nc < 2) continue;                              // one each: nothing to reorder
    if (static_cast<std::uint64_t>(sl.np) * sl.nc > 65536) continue;  // too many to search: by order
    double by_order = 0, by_near = 0;
    near_.clear();
    for (std::uint32_t k = sl.chead; k != ~0u; k = cnext_[k]) {
      by_order += match_[k] != ~0u ? std::min(1.0f, first_dist(C[k], P[match_[k]])) : 1.0;
      std::uint32_t best = ~0u;
      float bd = 0;
      for (std::uint32_t q = sl.phead; q != ~0u; q = pnext_[q]) {
        if (taken_[q] || P[q].count != C[k].count) continue;
        const float dd = first_dist(C[k], P[q]);
        if (best == ~0u || dd < bd) {
          best = q;
          bd = dd;
        }
      }
      if (best != ~0u) taken_[best] = 1;
      near_.push_back({k, best});
      by_near += best != ~0u ? std::min(1.0f, bd) : 1.0;
    }
    for (std::uint32_t q = sl.phead; q != ~0u; q = pnext_[q]) taken_[q] = 0;
    if (by_near >= 0.5 * by_order) continue;
    for (const Pair& pr : near_) {
      if (pr.prev != match_[pr.cur]) ++istats_.reordered;
      match_[pr.cur] = pr.prev;
    }
  }

  // ---- How much matched, how far it moved: a scene cut gets no in-between. ----
  // A draw's move is measured on up to four of its vertices (the first, the last and two
  // between); the frame's mean move of 3D vertices weighs each draw by its 3D vertices.
  constexpr std::uint32_t kStill = ~0u - 1;
  std::uint64_t total = 0, matched = 0, n3 = 0, moved = 0;
  double sum3 = 0;
  for (const RecRange& c : C) total += c.count;
  const float range_shift = interp_tune.range_shift;
  for (std::uint32_t k = 0; k < C.size(); ++k) {
    if (match_[k] == ~0u) continue;
    const RecRange& c = C[k];
    const RecRange& p = P[match_[k]];
    const float* a = &prev.pos[4 * static_cast<std::size_t>(p.pos)];
    const float* b = &cur.pos[4 * static_cast<std::size_t>(c.pos)];
    if (std::memcmp(a, b, 16 * static_cast<std::size_t>(c.count)) == 0) {
      // Still (much of a frame, often): no move to measure, nothing to write.
      matched += c.count;
      n3 += c.n3;
      match_[k] = kStill;
      continue;
    }
    float s = 0, s3 = 0;
    std::uint32_t n = 0, m3 = 0;
    const std::uint32_t cnt = c.count, samples = cnt < 4 ? cnt : 4;
    for (std::uint32_t t = 0; t < samples; ++t) {
      const std::uint32_t v = samples == cnt ? t : (cnt - 1) * t / (samples - 1);
      const float* va = a + 4 * static_cast<std::size_t>(v);
      const float* vb = b + 4 * static_cast<std::size_t>(v);
      if (!(va[3] > 1e-6f) || !(vb[3] > 1e-6f)) continue;  // behind the camera
      const float ia = 1.0f / va[3], ib = 1.0f / vb[3];
      const float dx = va[0] * ia - vb[0] * ib, dy = va[1] * ia - vb[1] * ib;
      const float d = std::sqrt(dx * dx + dy * dy);
      s += d;
      ++n;
      if (!(va[3] == 1.0f && vb[3] == 1.0f)) {  // 3D (2D vertices have w exactly 1)
        s3 += d;
        ++m3;
      }
    }
    if (n && s / static_cast<float>(n) > range_shift) {
      match_[k] = ~0u;  // too far for the same object: drawn where it is now
      continue;
    }
    matched += cnt;
    ++moved;
    if (m3) {
      sum3 += static_cast<double>(s3) / m3 * c.n3;
      n3 += c.n3;
    }
  }
  const double share = total ? static_cast<double>(matched) / static_cast<double>(total) : 0.0;
  const double shift = n3 ? sum3 / static_cast<double>(n3) : 0.0;
  istats_.last_match = share;
  istats_.last_shift = shift;
  {
    static constexpr double kShift[6] = {0.01, 0.02, 0.05, 0.1, 0.2, 0.4};
    static constexpr double kMatch[5] = {0.6, 0.8, 0.9, 0.95, 0.99};
    int b = 0;
    while (b < 6 && shift >= kShift[b]) ++b;
    ++istats_.shift_hist[b];
    b = 0;
    while (b < 5 && share >= kMatch[b]) ++b;
    ++istats_.match_hist[b];
  }
  if (share < interp_tune.min_match) {
    ++istats_.cut_match;
    return false;
  }
  if (shift > interp_tune.cut_shift) {
    ++istats_.cut_shift;
    return false;
  }
  if (moved == 0) {  // a still frame (a menu, a pause): the in-between would be the frame itself
    ++istats_.still;
    return false;
  }

  // ---- The halfway positions, written over this frame's recorded vertices (the record
  // keeps its own positions for the next frame's matching). ----
  const bool snap = cur.scale > 1;
  constexpr float kHalfW = kEfbWidth / 2.0f, kHalfH = kEfbHeight / 2.0f;
  for (std::uint32_t k = 0; k < C.size(); ++k) {
    if (match_[k] == ~0u || match_[k] == kStill) continue;  // drawn where it is now; or it did not move
    const RecRange& c = C[k];
    const RecRange& p = P[match_[k]];
    RecPass& rp = cur.passes[c.pass];
    if (!rp.filled || c.voff + static_cast<std::size_t>(c.count) * c.stride > rp.v.size()) continue;
    const float* a = &prev.pos[4 * static_cast<std::size_t>(p.pos)];
    const float* b = &cur.pos[4 * static_cast<std::size_t>(c.pos)];
    std::uint8_t* out = rp.v.data() + c.voff;
    for (std::uint32_t v = 0; v < c.count; ++v, a += 4, b += 4, out += c.stride) {
      if ((a[3] > 0) != (b[3] > 0)) continue;  // crosses the camera plane: where it is now
      float m[4] = {0.5f * (a[0] + b[0]), 0.5f * (a[1] + b[1]), 0.5f * (a[2] + b[2]), 0.5f * (a[3] + b[3])};
      if (snap && m[3] == 1.0f) {  // Q8's 2D snap, as Renderer::draw does it
        m[0] = std::ceil((m[0] + 1.0f) * kHalfW - 0.5f) / kHalfW - 1.0f;
        m[1] = 1.0f - std::ceil((1.0f - m[1]) * kHalfH - 0.5f) / kHalfH;
      }
      std::memcpy(out, m, 16);
    }
  }

  istats_.ms_match += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
  istats_.ranges += C.size();
  istats_.vertices += total;

  // ---- The replay, into the private targets. ----
  struct Off {
    std::uint64_t v, i, u;
  };
  std::vector<Off> offs(cur.npasses);
  std::uint64_t vb = 0, ib = 0, ub = 0;
  for (std::size_t k = 0; k < cur.npasses; ++k) {
    const RecPass& p = cur.passes[k];
    offs[k] = {vb, ib, ub};
    if (!p.filled) continue;
    vb += (p.v.size() + 255) & ~std::uint64_t(255);
    ib += (p.i.size() * 4 + 255) & ~std::uint64_t(255);
    ub += (p.u.size() + 255) & ~std::uint64_t(255);
  }
  if (vb + ib + ub > kMaxReplayBytes) {
    ++istats_.other;
    return false;
  }
  interp_targets();
  const wgpu::Device& dev = gpu_.device();
  wgpu::Queue q = gpu_.queue();
  interp_buffer(ivbuf_, ivcap_, std::max<std::uint64_t>(vb, 256), wgpu::BufferUsage::Vertex, "in-between vertices");
  interp_buffer(iibuf_, iicap_, std::max<std::uint64_t>(ib, 256), wgpu::BufferUsage::Index, "in-between indices");
  const std::uint64_t ucap = iucap_;
  interp_buffer(iubuf_, iucap_, std::max<std::uint64_t>(ub, 1024), wgpu::BufferUsage::Uniform, "in-between uniforms");
  if (!iuniform_group_ || ucap != iucap_) {
    wgpu::BindGroupEntry be{};
    be.binding = 0;
    be.buffer = iubuf_;
    be.size = kUniformSize;
    wgpu::BindGroupDescriptor bgd{};
    bgd.layout = bgl_uniforms_;
    bgd.entryCount = 1;
    bgd.entries = &be;
    iuniform_group_ = dev.CreateBindGroup(&bgd);
  }
  for (std::size_t k = 0; k < cur.npasses; ++k) {
    const RecPass& p = cur.passes[k];
    if (!p.filled) continue;
    if (!p.v.empty()) q.WriteBuffer(ivbuf_, offs[k].v, p.v.data(), p.v.size());
    if (!p.i.empty()) q.WriteBuffer(iibuf_, offs[k].i, p.i.data(), p.i.size() * 4);
    if (!p.u.empty()) q.WriteBuffer(iubuf_, offs[k].u, p.u.data(), p.u.size());
  }
  // Clear and conversion constants, one 256-byte slot per op.
  std::vector<wgpu::BindGroup> clear_groups, conv_groups;
  if (!cur.clears.empty()) {
    interp_buffer(iclear_buf_, iclear_cap_, cur.clears.size() * kSlot, wgpu::BufferUsage::Uniform, "in-between clears");
    std::vector<std::uint8_t> slots(cur.clears.size() * kSlot, 0);
    for (std::size_t k = 0; k < cur.clears.size(); ++k) {
      std::memcpy(slots.data() + k * kSlot, cur.clears[k].data, sizeof cur.clears[k].data);
      wgpu::BindGroupEntry be{};
      be.binding = 0;
      be.buffer = iclear_buf_;
      be.offset = k * kSlot;
      be.size = 32;
      wgpu::BindGroupDescriptor bgd{};
      bgd.layout = bgl_clear_;
      bgd.entryCount = 1;
      bgd.entries = &be;
      clear_groups.push_back(dev.CreateBindGroup(&bgd));
    }
    q.WriteBuffer(iclear_buf_, 0, slots.data(), slots.size());
  }
  if (!cur.copies.empty()) {
    interp_buffer(iconv_buf_, iconv_cap_, cur.copies.size() * kSlot, wgpu::BufferUsage::Uniform, "in-between copies");
    std::vector<std::uint8_t> slots(cur.copies.size() * kSlot, 0);
    for (std::size_t k = 0; k < cur.copies.size(); ++k) {
      std::memcpy(slots.data() + k * kSlot, cur.copies[k].params, sizeof cur.copies[k].params);
      wgpu::BindGroupEntry be[3]{};
      be[0].binding = 0;
      be[0].buffer = iconv_buf_;
      be[0].offset = k * kSlot;
      be[0].size = 32;
      be[1].binding = 1;
      be[1].textureView = iefb_color_view_;
      be[2].binding = 2;
      be[2].textureView = iefb_depth_view_;
      wgpu::BindGroupDescriptor bgd{};
      bgd.layout = bgl_convert_;
      bgd.entryCount = 3;
      bgd.entries = be;
      conv_groups.push_back(dev.CreateBindGroup(&bgd));
    }
    q.WriteBuffer(iconv_buf_, 0, slots.data(), slots.size());
  }

  ++replays_;
  subst_.clear();
  wgpu::CommandEncoder enc = dev.CreateCommandEncoder();
  wgpu::RenderPassEncoder pass;
  bool open = false;
  const void* bpipe = nullptr;
  const void* btex = nullptr;
  std::uint32_t buoff = ~0u, bsc[4] = {~0u, ~0u, ~0u, ~0u};
  const std::uint32_t s = cur.scale;
  auto begin = [&] {
    if (open) return;
    wgpu::RenderPassColorAttachment ca{};
    ca.view = iefb_color_view_;
    ca.loadOp = wgpu::LoadOp::Load;
    ca.storeOp = wgpu::StoreOp::Store;
    wgpu::RenderPassDepthStencilAttachment da{};
    da.view = iefb_depth_view_;
    da.depthLoadOp = wgpu::LoadOp::Load;
    da.depthStoreOp = wgpu::StoreOp::Store;
    da.depthClearValue = 1.0f;
    wgpu::RenderPassDescriptor rp{};
    rp.colorAttachmentCount = 1;
    rp.colorAttachments = &ca;
    rp.depthStencilAttachment = &da;
    if (timer_) rp.timestampWrites = timer_->pass();
    pass = enc.BeginRenderPass(&rp);
    open = true;
    bpipe = btex = nullptr;
    buoff = ~0u;
    bsc[0] = bsc[1] = bsc[2] = bsc[3] = ~0u;
  };
  auto end = [&] {
    if (!open) return;
    pass.End();
    pass = nullptr;
    open = false;
  };
  for (const RecOp& op : cur.ops) {
    switch (op.kind) {
      case RecKind::Pass: {
        const RecPass& p = cur.passes[op.index];
        if (!p.filled || p.draws.empty() || p.v.empty() || p.i.empty() || p.u.empty()) break;
        begin();
        const Off& o = offs[op.index];
        pass.SetVertexBuffer(0, ivbuf_, o.v, p.v.size());
        pass.SetIndexBuffer(iibuf_, wgpu::IndexFormat::Uint32, o.i, p.i.size() * 4);
        buoff = ~0u;
        for (const RecDraw& d : p.draws) {
          if (bpipe != d.pipe) {
            pass.SetPipeline(d.pipe->pipe);
            bpipe = d.pipe;
          }
          const std::uint32_t uo = static_cast<std::uint32_t>(o.u) + d.uoff;
          if (buoff != uo) {
            pass.SetBindGroup(0, iuniform_group_, 1, &uo);
            buoff = uo;
          }
          const wgpu::BindGroup& g = interp_bind(cur.binds[d.bind]);
          if (btex != g.Get()) {
            pass.SetBindGroup(1, g);
            btex = g.Get();
          }
          if (std::memcmp(bsc, d.scissor, sizeof bsc) != 0) {
            pass.SetScissorRect(d.scissor[0] * s, d.scissor[1] * s, (d.scissor[2] - d.scissor[0]) * s,
                                (d.scissor[3] - d.scissor[1]) * s);
            std::memcpy(bsc, d.scissor, sizeof bsc);
          }
          pass.DrawIndexed(d.index_count, 1, d.first_index, static_cast<std::int32_t>(d.base), 0);
        }
        istats_.replayed += p.draws.size();
        break;
      }
      case RecKind::Clear: {
        const RecClear& c = cur.clears[op.index];
        begin();
        pass.SetPipeline(clear_pipes_[c.m]);
        pass.SetBindGroup(0, clear_groups[op.index]);
        pass.SetScissorRect(c.rect[0], c.rect[1], c.rect[2], c.rect[3]);
        pass.Draw(3);
        bpipe = btex = nullptr;  // the next draw binds its own state again
        buoff = ~0u;
        bsc[0] = bsc[1] = bsc[2] = bsc[3] = ~0u;
        break;
      }
      case RecKind::Copy: {
        end();
        const RecCopy& c = cur.copies[op.index];
        PrivCopy& pc = priv_copies_[c.live.Get()];
        if (!pc.tex || pc.pw != c.pw || pc.ph != c.ph) {
          wgpu::TextureDescriptor td{};
          td.size = {c.pw, c.ph, 1};
          td.format = wgpu::TextureFormat::RGBA8Unorm;
          td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
          td.label = "in-between efb copy";
          pc.tex = dev.CreateTexture(&td);
          pc.view = pc.tex.CreateView();
          pc.pw = c.pw;
          pc.ph = c.ph;
          pc.live = c.live;
        }
        pc.used = replays_;
        wgpu::RenderPassColorAttachment ca{};
        ca.view = pc.view;
        ca.loadOp = wgpu::LoadOp::Clear;
        ca.storeOp = wgpu::StoreOp::Store;
        wgpu::RenderPassDescriptor rp{};
        rp.colorAttachmentCount = 1;
        rp.colorAttachments = &ca;
        if (timer_) rp.timestampWrites = timer_->pass();
        wgpu::RenderPassEncoder cp = enc.BeginRenderPass(&rp);
        cp.SetPipeline(convert_pipe_);
        cp.SetBindGroup(0, conv_groups[op.index]);
        cp.Draw(3);
        cp.End();
        subst_[c.live.Get()] = pc.view;  // later draws that sample this copy sample the private one
        break;
      }
    }
  }
  end();
  // The picture: the XFB copy's rectangle of the private EFB.
  Between& bt = between_[between_next_];
  between_next_ = (between_next_ + 1) % 3;
  const std::uint32_t pw = cur.xfb[2] * s, ph = cur.xfb[3] * s;
  if (!bt.frame.tex || bt.frame.width != pw || bt.frame.height != ph) {
    wgpu::TextureDescriptor td{};
    td.size = {pw, ph, 1};
    td.format = wgpu::TextureFormat::RGBA8Unorm;
    td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    td.label = "in-between frame";
    bt.frame.tex = dev.CreateTexture(&td);
    bt.frame.view = bt.frame.tex.CreateView();
    bt.frame.width = pw;
    bt.frame.height = ph;
  }
  {
    wgpu::TexelCopyTextureInfo src{};
    src.texture = iefb_color_;
    src.origin = {cur.xfb[0] * s, cur.xfb[1] * s, 0};
    wgpu::TexelCopyTextureInfo dst{};
    dst.texture = bt.frame.tex;
    wgpu::Extent3D ext{pw, ph, 1};
    enc.CopyTextureToTexture(&src, &dst, &ext);
  }
  wgpu::CommandBuffer cb = enc.Finish();
  q.Submit(1, &cb);
  bt.from = prev.serial;
  bt.to = cur.serial;
  bt.frame.serial = cur.serial;
  bt.frame.addr = 0;
  // Private copies unused for two seconds go (and the live textures they keep).
  for (auto it = priv_copies_.begin(); it != priv_copies_.end();)
    it = replays_ - it->second.used > 120 ? priv_copies_.erase(it) : std::next(it);
  ++istats_.made;
  return true;
}

const Renderer::Frame* Renderer::between(std::uint64_t from, std::uint64_t to) {
  if (!interp_on_) return nullptr;
  const Between* b = nullptr;
  for (const Between& x : between_)
    if (x.frame.tex && x.to == to && x.from == from) b = &x;
  if (!b) return nullptr;
  ++istats_.shown;
  // Where scanout() mixes the frames at the last three fields (pixels the game
  // alternates at 30 Hz), the in-between is mixed with the two frames before it too,
  // so those pixels look the same in both pictures.
  if (!scan_mix || scan_slot_[0] < 0 || scan_slot_[1] < 0 || scan_slot_[2] < 0 || scan_serial_[0] != to ||
      scan_serial_[1] != from || scan_serial_[2] == scan_serial_[1])
    return &b->frame;
  const Frame& fb = scan_[scan_slot_[1]];
  const Frame& fc = scan_[scan_slot_[2]];
  if (fb.width != b->frame.width || fb.height != b->frame.height) return &b->frame;
  const std::uint64_t key = to * 0x9E3779B97F4A7C15ull ^ from * 0xC2B2AE3D27D4EB4Full ^ scan_serial_[2];
  if (key == between_mixed_key_) return &between_mixed_;
  ensure_mix_pipe();
  if (!between_mixed_.tex || between_mixed_.width != b->frame.width || between_mixed_.height != b->frame.height) {
    wgpu::TextureDescriptor td{};
    td.size = {b->frame.width, b->frame.height, 1};
    td.format = wgpu::TextureFormat::RGBA8Unorm;
    td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    td.label = "in-between mixed";
    between_mixed_.tex = gpu_.device().CreateTexture(&td);
    between_mixed_.view = between_mixed_.tex.CreateView();
    between_mixed_.width = b->frame.width;
    between_mixed_.height = b->frame.height;
  }
  wgpu::BindGroupEntry be[3]{};
  be[0].binding = 0;
  be[0].textureView = b->frame.view;
  be[1].binding = 1;
  be[1].textureView = fb.view;
  be[2].binding = 2;
  be[2].textureView = fc.view;
  wgpu::BindGroupDescriptor bgd{};
  bgd.layout = mix_pipe_.GetBindGroupLayout(0);
  bgd.entryCount = 3;
  bgd.entries = be;
  wgpu::BindGroup bg = gpu_.device().CreateBindGroup(&bgd);
  wgpu::CommandEncoder enc = gpu_.device().CreateCommandEncoder();
  wgpu::RenderPassColorAttachment ca{};
  ca.view = between_mixed_.view;
  ca.loadOp = wgpu::LoadOp::Clear;
  ca.storeOp = wgpu::StoreOp::Store;
  wgpu::RenderPassDescriptor rp{};
  rp.colorAttachmentCount = 1;
  rp.colorAttachments = &ca;
  if (timer_) rp.timestampWrites = timer_->pass();
  wgpu::RenderPassEncoder pass = enc.BeginRenderPass(&rp);
  pass.SetPipeline(mix_pipe_);
  pass.SetBindGroup(0, bg);
  pass.Draw(3);
  pass.End();
  wgpu::CommandBuffer cb = enc.Finish();
  gpu_.queue().Submit(1, &cb);
  between_mixed_.serial = b->frame.serial;
  between_mixed_key_ = key;
  return &between_mixed_;
}

}  // namespace mvp::gfx
