#include "gfx/wgpu/renderer.h"

#include "gfx/wgpu/gpu_timer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>

#include "gfx/tev_wgsl.h"
#include "gfx/texture_decode.h"
#include "gfx/wgpu/gpu.h"

// V5: profiling builds (MVP_WEB_NAMES, linked at -O1 so that Binaryen does not
// inline them back) keep these functions out of line, so a profile attributes
// the GX path's time to its parts; otherwise nothing.
#if defined(MVP_WEB_NAMES) && defined(__clang__)
#define MVP_PROF_NOINLINE __attribute__((noinline))
#else
#define MVP_PROF_NOINLINE
#endif

namespace mvp::gfx {

namespace {

constexpr std::uint32_t kUniformSlot = 512;              // DrawUniforms (416 bytes), 256-aligned
constexpr std::uint64_t kVertexCapacity = 16u << 20;     // per pass
constexpr std::uint64_t kIndexCapacity = 8u << 20;       // per pass (2 Mi indices)
constexpr std::uint64_t kUniformCapacity = 2u << 20;     // per pass (4,096 draws)

std::uint64_t mix64(std::uint64_t h) {
  h ^= h >> 33;
  h *= 0xFF51AFD7ED558CCDull;
  h ^= h >> 33;
  h *= 0xC4CEB9FE1A85EC53ull;
  return h ^ (h >> 33);
}

// V5: XXH64 (Yann Collet's xxHash, 64-bit, BSD-2), for the caches' keys and
// change checks: palettes, texture memory once per cache epoch (1.4 MB a field
// in play), pipeline configurations. 2.5 times the speed of the multiply-mix
// loop before it in the browser (20 against 8 GB/s); the values only ever meet
// values of the same function.
inline std::uint64_t rotl64(std::uint64_t x, int r) { return (x << r) | (x >> (64 - r)); }
inline std::uint64_t read64(const std::uint8_t* p) {
  std::uint64_t w;
  std::memcpy(&w, p, 8);
  return w;
}
constexpr std::uint64_t kP1 = 0x9E3779B185EBCA87ull, kP2 = 0xC2B2AE3D27D4EB4Full, kP3 = 0x165667B19E3779F9ull,
                        kP4 = 0x85EBCA77C2B2AE63ull, kP5 = 0x27D4EB2F165667C5ull;
inline std::uint64_t xxh_round(std::uint64_t acc, std::uint64_t in) { return rotl64(acc + in * kP2, 31) * kP1; }
inline std::uint64_t xxh_merge(std::uint64_t acc, std::uint64_t v) { return (acc ^ xxh_round(0, v)) * kP1 + kP4; }

std::uint64_t hash_bytes(const std::uint8_t* p, std::size_t n, std::uint64_t seed = 0x9E3779B97F4A7C15ull) {
  const std::uint8_t* const end = p + n;
  std::uint64_t h;
  if (n >= 32) {
    std::uint64_t v1 = seed + kP1 + kP2, v2 = seed + kP2, v3 = seed, v4 = seed - kP1;
    const std::uint8_t* const limit = end - 32;
    do {
      v1 = xxh_round(v1, read64(p));
      v2 = xxh_round(v2, read64(p + 8));
      v3 = xxh_round(v3, read64(p + 16));
      v4 = xxh_round(v4, read64(p + 24));
      p += 32;
    } while (p <= limit);
    h = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
    h = xxh_merge(h, v1);
    h = xxh_merge(h, v2);
    h = xxh_merge(h, v3);
    h = xxh_merge(h, v4);
  } else {
    h = seed + kP5;
  }
  h += static_cast<std::uint64_t>(n);
  for (; p + 8 <= end; p += 8) h = rotl64(h ^ xxh_round(0, read64(p)), 27) * kP1 + kP4;
  if (p + 4 <= end) {
    std::uint32_t w;
    std::memcpy(&w, p, 4);
    h = rotl64(h ^ (static_cast<std::uint64_t>(w) * kP1), 23) * kP2 + kP3;
    p += 4;
  }
  for (; p < end; ++p) h = rotl64(h ^ (static_cast<std::uint64_t>(*p) * kP5), 11) * kP1;
  h ^= h >> 33;
  h *= kP2;
  h ^= h >> 29;
  h *= kP3;
  return h ^ (h >> 32);
}

wgpu::BlendFactor blend_factor(u8 f, bool is_dst, bool alpha_channel, bool efb_has_alpha) {
  switch (f) {
    case 0: return wgpu::BlendFactor::Zero;
    case 1: return wgpu::BlendFactor::One;
    case 2:  // src: dst color; dst: src color
      if (alpha_channel) return is_dst ? wgpu::BlendFactor::SrcAlpha : wgpu::BlendFactor::DstAlpha;
      return is_dst ? wgpu::BlendFactor::Src : wgpu::BlendFactor::Dst;
    case 3:
      if (alpha_channel) return is_dst ? wgpu::BlendFactor::OneMinusSrcAlpha : wgpu::BlendFactor::OneMinusDstAlpha;
      return is_dst ? wgpu::BlendFactor::OneMinusSrc : wgpu::BlendFactor::OneMinusDst;
    case 4: return wgpu::BlendFactor::SrcAlpha;
    case 5: return wgpu::BlendFactor::OneMinusSrcAlpha;
    case 6: return efb_has_alpha ? wgpu::BlendFactor::DstAlpha : wgpu::BlendFactor::One;
    default: return efb_has_alpha ? wgpu::BlendFactor::OneMinusDstAlpha : wgpu::BlendFactor::Zero;
  }
}

wgpu::CompareFunction compare(u8 f) {
  switch (f) {
    case 0: return wgpu::CompareFunction::Never;
    case 1: return wgpu::CompareFunction::Less;
    case 2: return wgpu::CompareFunction::Equal;
    case 3: return wgpu::CompareFunction::LessEqual;
    case 4: return wgpu::CompareFunction::Greater;
    case 5: return wgpu::CompareFunction::NotEqual;
    case 6: return wgpu::CompareFunction::GreaterEqual;
    default: return wgpu::CompareFunction::Always;
  }
}

wgpu::AddressMode address(u8 w) {
  return w == 1 ? wgpu::AddressMode::Repeat : w == 2 ? wgpu::AddressMode::MirrorRepeat : wgpu::AddressMode::ClampToEdge;
}

bool efb_has_alpha(u8 pixel_format) { return pixel_format == 1; }  // RGBA6_Z24

// Vertices packed to a pipeline's layout (tev_wgsl.h: the position, then the
// attributes `attrs` names). `snap` (Q8, above 1x): a 2D vertex (w exactly 1)
// goes to a whole console pixel (Renderer::draw says why). V5: the common
// layouts as constants (the attribute tests folded away).
template <std::uint32_t kAttrs>
inline void pack_layout(const OutVertex* v, std::size_t n, std::uint32_t attrs_in, bool snap, std::uint8_t* out) {
  const std::uint32_t attrs = kAttrs == ~0u ? attrs_in : kAttrs;
  constexpr float kHalfW = kEfbWidth / 2.0f, kHalfH = kEfbHeight / 2.0f;
  for (const OutVertex* e = v + n; v != e; ++v) {
    std::memcpy(out, v->pos, 16);
    if (snap && v->pos[3] == 1.0f) {
      const float xy[2] = {std::ceil((v->pos[0] + 1.0f) * kHalfW - 0.5f) / kHalfW - 1.0f,
                           1.0f - std::ceil((1.0f - v->pos[1]) * kHalfH - 0.5f) / kHalfH};
      std::memcpy(out, xy, 8);
    }
    out += 16;
    if (attrs & 1u) {
      std::memcpy(out, v->color[0], 16);
      out += 16;
    }
    if (attrs & 2u) {
      std::memcpy(out, v->color[1], 16);
      out += 16;
    }
    for (int k = 0; k < 8; ++k)
      if (attrs & (4u << k)) {
        std::memcpy(out, v->tex[k], 12);
        out += 12;
      }
  }
}

MVP_PROF_NOINLINE void pack_vertices(const OutVertex* v, std::size_t n, std::uint32_t attrs, bool snap, std::uint8_t* out) {
  switch (attrs) {
    case 0: return pack_layout<0>(v, n, attrs, snap, out);
    case 1: return pack_layout<1>(v, n, attrs, snap, out);
    case 4: return pack_layout<4>(v, n, attrs, snap, out);
    case 1 | 4: return pack_layout<1 | 4>(v, n, attrs, snap, out);
    case 1 | 4 | 8: return pack_layout<1 | 4 | 8>(v, n, attrs, snap, out);
    case 1 | 4 | 8 | 16: return pack_layout<1 | 4 | 8 | 16>(v, n, attrs, snap, out);
    case 4 | 8: return pack_layout<4 | 8>(v, n, attrs, snap, out);
    default: return pack_layout<~0u>(v, n, attrs, snap, out);
  }
}

const char* kClearWgsl = R"(
struct C { color: vec4f, depth: vec4f };
@group(0) @binding(0) var<uniform> cu: C;
@vertex fn vs(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
  var p = array<vec2f, 3>(vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0));
  return vec4f(p[i], cu.depth.x, 1.0);
}
@fragment fn fs() -> @location(0) vec4f { return cu.color; }
)";

// EFB -> texture copies: reads the source rectangle (box-filtered when halving)
// and converts it to what sampling the copy's format returns.
const char* kConvertWgsl = R"(
struct P { src: vec4i, mode: vec4i };  // src: x, y, half, from depth; mode: format, intensity
@group(0) @binding(0) var<uniform> p: P;
@group(0) @binding(1) var efb: texture_2d<f32>;
@group(0) @binding(2) var efbz: texture_depth_2d;
@vertex fn vs(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
  var q = array<vec2f, 3>(vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0));
  return vec4f(q[i], 0.0, 1.0);
}
fn fetch(xy: vec2i) -> vec4f {
  if (p.src.w != 0) { let d = textureLoad(efbz, xy, 0); return vec4f(d, d, d, d); }
  return textureLoad(efb, xy, 0);
}
@fragment fn fs(@builtin(position) pos: vec4f) -> @location(0) vec4f {
  let o = vec2i(pos.xy);
  var c: vec4f;
  if (p.src.z != 0) {
    let b = p.src.xy + o * 2;
    c = (fetch(b) + fetch(b + vec2i(1, 0)) + fetch(b + vec2i(0, 1)) + fetch(b + vec2i(1, 1))) * 0.25;
  } else {
    c = fetch(p.src.xy + o);
  }
  let y = clamp(0.257 * c.r + 0.504 * c.g + 0.098 * c.b + 16.0 / 255.0, 0.0, 1.0);
  let i = select(c.r, y, p.mode.y != 0);
  switch (p.mode.x) {
    case 0, 1, 8: { return vec4f(i, i, i, i); }
    case 2, 3: { return vec4f(i, i, i, c.a); }
    case 4: { return vec4f(c.rgb, 1.0); }
    case 7: { return vec4f(c.a, c.a, c.a, c.a); }
    case 9: { return vec4f(c.g, c.g, c.g, c.g); }
    case 10: { return vec4f(c.b, c.b, c.b, c.b); }
    case 11: { return vec4f(c.r, c.r, c.r, c.g); }
    case 12: { return vec4f(c.g, c.g, c.g, c.b); }
    default: { return c; }
  }
}
)";

// The scan-out mix (P1): per pixel, the frame of the last field (a), unless it
// alternates over the last three fields (a close to c, far from b): then the
// mean of a and b, as an interlaced display shows field-alternating pictures.
const char* kScanMixWgsl = R"(
@group(0) @binding(0) var fa: texture_2d<f32>;
@group(0) @binding(1) var fb: texture_2d<f32>;
@group(0) @binding(2) var fc: texture_2d<f32>;
@vertex fn vs(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
  var q = array<vec2f, 3>(vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0));
  return vec4f(q[i], 0.0, 1.0);
}
@fragment fn fs(@builtin(position) pos: vec4f) -> @location(0) vec4f {
  let o = vec2i(pos.xy);
  let a = textureLoad(fa, o, 0).rgb;
  let b = textureLoad(fb, o, 0).rgb;
  let c = textureLoad(fc, o, 0).rgb;
  let ab = dot(abs(a - b), vec3f(1.0));
  let ac = dot(abs(a - c), vec3f(1.0));
  let alt = ab > 6.0 / 255.0 && ac * 4.0 < ab;
  return vec4f(select(a, (a + b) * 0.5, alt), 1.0);
}
)";

}  // namespace

struct Renderer::TexEntry {
  wgpu::Texture tex;
  wgpu::TextureView view;
  std::uint32_t w = 0, h = 0;
  std::uint64_t mem_hash = 0, epoch = 0;
  std::uint64_t pass = 0;  // the render pass that last sampled it
  std::uint64_t rec = 0;   // V3: the frame record that last sampled it
};

struct Renderer::CopyEntry {
  wgpu::Texture tex;
  wgpu::TextureView view;
  std::uint32_t w = 0, h = 0;    // console pixels (what a texture of this address must measure)
  std::uint32_t pw = 0, ph = 0;  // the texture's (times the scale at the copy)
  std::uint32_t bytes = 0;
  std::uint64_t mem_hash = 0, epoch = 0;
  std::uint64_t serial = 0;  // bumped by every copy to it
  std::uint64_t rec = 0;     // V3: the frame record that last redid it
  // P2: the copy as part of a taller texture of the same width (tall_copy()).
  struct Tall {
    wgpu::Texture tex;
    wgpu::TextureView view;
    std::uint32_t h = 0, scale = 1;
    std::uint8_t format = 0xFF;
    std::uint64_t copy_serial = 0, tail_hash = 0, pass = 0, epoch = 0;
  };
  std::unique_ptr<Tall> tall;
};

std::size_t Renderer::CfgHash::operator()(const PipelineConfig& c) const {
  return static_cast<std::size_t>(hash_bytes(reinterpret_cast<const std::uint8_t*>(&c), sizeof c));
}

Renderer::Renderer(Gpu& gpu, const std::uint8_t* mem, std::uint32_t mem_size, std::uint32_t scale)
    : gpu_(gpu), mem_(mem), mem_size_(mem_size) {
  const wgpu::Device& dev = gpu.device();
  set_scale(scale);
  scale_ = want_scale_;
  create_efb();
  wgpu::TextureDescriptor td{};

  // Uniforms: one slot per draw, dynamic offset.
  wgpu::BindGroupLayoutEntry ue{};
  ue.binding = 0;
  ue.visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;
  ue.buffer.type = wgpu::BufferBindingType::Uniform;
  ue.buffer.hasDynamicOffset = true;
  ue.buffer.minBindingSize = kUniformSize;
  wgpu::BindGroupLayoutDescriptor bd{};
  bd.entryCount = 1;
  bd.entries = &ue;
  bgl_uniforms_ = dev.CreateBindGroupLayout(&bd);
  wgpu::BindGroupLayoutEntry te[16]{};
  for (int k = 0; k < 8; ++k) {
    te[k].binding = static_cast<std::uint32_t>(k);
    te[k].visibility = wgpu::ShaderStage::Fragment;
    te[k].texture.sampleType = wgpu::TextureSampleType::Float;
    te[k].texture.viewDimension = wgpu::TextureViewDimension::e2D;
    te[8 + k].binding = static_cast<std::uint32_t>(8 + k);
    te[8 + k].visibility = wgpu::ShaderStage::Fragment;
    te[8 + k].sampler.type = wgpu::SamplerBindingType::Filtering;
  }
  bd.entryCount = 16;
  bd.entries = te;
  bgl_textures_ = dev.CreateBindGroupLayout(&bd);
  wgpu::BindGroupLayout layouts[2] = {bgl_uniforms_, bgl_textures_};
  wgpu::PipelineLayoutDescriptor pl{};
  pl.bindGroupLayoutCount = 2;
  pl.bindGroupLayouts = layouts;
  layout_ = dev.CreatePipelineLayout(&pl);

  wgpu::BufferDescriptor b{};
  b.size = kVertexCapacity;
  b.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
  b.label = "vertices";
  vbuf_ = dev.CreateBuffer(&b);
  b.size = kIndexCapacity;
  b.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
  b.label = "indices";
  ibuf_ = dev.CreateBuffer(&b);
  b.size = kUniformCapacity;
  b.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
  b.label = "uniforms";
  ubuf_ = dev.CreateBuffer(&b);
  wgpu::BindGroupEntry be{};
  be.binding = 0;
  be.buffer = ubuf_;
  be.size = kUniformSize;
  wgpu::BindGroupDescriptor bgd{};
  bgd.layout = bgl_uniforms_;
  bgd.entryCount = 1;
  bgd.entries = &be;
  uniform_group_ = dev.CreateBindGroup(&bgd);

  // A 1x1 texture and a sampler for unused bindings.
  td.size = {1, 1, 1};
  td.format = wgpu::TextureFormat::RGBA8Unorm;
  td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
  td.label = "dummy";
  dummy_tex_ = dev.CreateTexture(&td);
  dummy_view_ = dummy_tex_.CreateView();
  const std::uint8_t black[4] = {0, 0, 0, 0};
  wgpu::TexelCopyTextureInfo dst{};
  dst.texture = dummy_tex_;
  wgpu::TexelCopyBufferLayout lay{};
  lay.bytesPerRow = 4;
  wgpu::Extent3D one{1, 1, 1};
  gpu.queue().WriteTexture(&dst, black, 4, &lay, &one);
  wgpu::SamplerDescriptor sd{};
  dummy_sampler_ = dev.CreateSampler(&sd);
  point_sampler_ = dummy_sampler_;

  // Clear pipelines: one per (color, alpha, depth) write mask.
  {
    wgpu::BindGroupLayoutEntry ce{};
    ce.binding = 0;
    ce.visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;
    ce.buffer.type = wgpu::BufferBindingType::Uniform;
    bd.entryCount = 1;
    bd.entries = &ce;
    bgl_clear_ = dev.CreateBindGroupLayout(&bd);
    wgpu::PipelineLayoutDescriptor cpl{};
    cpl.bindGroupLayoutCount = 1;
    cpl.bindGroupLayouts = &bgl_clear_;
    wgpu::PipelineLayout clayout = dev.CreatePipelineLayout(&cpl);
    wgpu::ShaderModule sm = gpu.shader(kClearWgsl, "clear");
    for (int m = 0; m < 8; ++m) {
      wgpu::ColorTargetState ct{};
      ct.format = wgpu::TextureFormat::RGBA8Unorm;
      ct.writeMask = ((m & 1) ? wgpu::ColorWriteMask::Red | wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Blue
                              : wgpu::ColorWriteMask::None) |
                     ((m & 2) ? wgpu::ColorWriteMask::Alpha : wgpu::ColorWriteMask::None);
      wgpu::FragmentState fs{};
      fs.module = sm;
      fs.entryPoint = "fs";
      fs.targetCount = 1;
      fs.targets = &ct;
      wgpu::DepthStencilState ds{};
      ds.format = wgpu::TextureFormat::Depth32Float;
      ds.depthCompare = wgpu::CompareFunction::Always;
      ds.depthWriteEnabled = (m & 4) ? wgpu::OptionalBool::True : wgpu::OptionalBool::False;
      wgpu::RenderPipelineDescriptor pd{};
      pd.label = "clear";
      pd.layout = clayout;
      pd.vertex.module = sm;
      pd.vertex.entryPoint = "vs";
      pd.fragment = &fs;
      pd.depthStencil = &ds;
      clear_pipes_[m] = dev.CreateRenderPipeline(&pd);
    }
    b.size = 32;
    b.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    b.label = "clear";
    clear_buf_ = dev.CreateBuffer(&b);
  }
  // EFB -> texture conversion.
  {
    wgpu::BindGroupLayoutEntry ce[3]{};
    ce[0].binding = 0;
    ce[0].visibility = wgpu::ShaderStage::Fragment;
    ce[0].buffer.type = wgpu::BufferBindingType::Uniform;
    ce[1].binding = 1;
    ce[1].visibility = wgpu::ShaderStage::Fragment;
    ce[1].texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
    ce[2].binding = 2;
    ce[2].visibility = wgpu::ShaderStage::Fragment;
    ce[2].texture.sampleType = wgpu::TextureSampleType::Depth;
    bd.entryCount = 3;
    bd.entries = ce;
    bgl_convert_ = dev.CreateBindGroupLayout(&bd);
    wgpu::PipelineLayoutDescriptor cpl{};
    cpl.bindGroupLayoutCount = 1;
    cpl.bindGroupLayouts = &bgl_convert_;
    wgpu::PipelineLayout clayout = dev.CreatePipelineLayout(&cpl);
    wgpu::ShaderModule sm = gpu.shader(kConvertWgsl, "efb convert");
    wgpu::ColorTargetState ct{};
    ct.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::FragmentState fs{};
    fs.module = sm;
    fs.entryPoint = "fs";
    fs.targetCount = 1;
    fs.targets = &ct;
    wgpu::RenderPipelineDescriptor pd{};
    pd.label = "efb convert";
    pd.layout = clayout;
    pd.vertex.module = sm;
    pd.vertex.entryPoint = "vs";
    pd.fragment = &fs;
    convert_pipe_ = dev.CreateRenderPipeline(&pd);
    b.size = 32;
    b.label = "convert";
    convert_buf_ = dev.CreateBuffer(&b);
  }
  // The EFB starts cleared to black, depth far.
  EfbCopy c;
  c.width = kEfbWidth;
  c.height = kEfbHeight;
  c.clear_color = c.clear_alpha = c.clear_depth = true;
  c.clear_z = 0xFFFFFF;
  c.clear_rgba[3] = 255;
  clear_efb(c);
  flush();
}

Renderer::~Renderer() = default;

void Renderer::create_efb() {
  wgpu::TextureDescriptor td{};
  td.size = {kEfbWidth * scale_, kEfbHeight * scale_, 1};
  td.format = wgpu::TextureFormat::RGBA8Unorm;
  td.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding;
  td.label = "efb color";
  efb_color_ = gpu_.device().CreateTexture(&td);
  efb_color_view_ = efb_color_.CreateView();
  td.format = wgpu::TextureFormat::Depth32Float;
  // V3: CopySrc for the in-between frames' private EFB, which starts as a copy of this one
  // when a frame does not clear it.
  td.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
  td.label = "efb depth";
  efb_depth_ = gpu_.device().CreateTexture(&td);
  efb_depth_view_ = efb_depth_.CreateView();
}

void Renderer::begin_pass() {
  if (pass_open_) return;
  encoder_ = gpu_.device().CreateCommandEncoder();
  wgpu::RenderPassColorAttachment ca{};
  ca.view = efb_color_view_;
  ca.loadOp = wgpu::LoadOp::Load;
  ca.storeOp = wgpu::StoreOp::Store;
  wgpu::RenderPassDepthStencilAttachment da{};
  da.view = efb_depth_view_;
  da.depthLoadOp = wgpu::LoadOp::Load;
  da.depthStoreOp = wgpu::StoreOp::Store;
  da.depthClearValue = 1.0f;  // unused with Load, but browsers reject webgpu.h's NaN default (E2)
  wgpu::RenderPassDescriptor rp{};
  rp.colorAttachmentCount = 1;
  rp.colorAttachments = &ca;
  rp.depthStencilAttachment = &da;
  if (timer_) rp.timestampWrites = timer_->pass();  // Q5: the GPU's time
  pass_ = encoder_.BeginRenderPass(&rp);
  pass_open_ = true;
  ++pass_serial_;
  vdata_.clear();
  udata_.clear();
  idata_.clear();
  bound_pipeline_ = bound_textures_ = nullptr;
  bound_uoff_ = ~0u;
  bound_scissor_[0] = bound_scissor_[1] = bound_scissor_[2] = bound_scissor_[3] = ~0u;
  bound_vbuf_ = bound_ibuf_ = false;
  ++stats_.passes;
}

void Renderer::emit_pending() {
  if (!pending_.active) return;
  pending_.active = false;
  if (bound_pipeline_ != pending_.pipeline.Get()) {
    pass_.SetPipeline(pending_.pipeline);
    bound_pipeline_ = pending_.pipeline.Get();
  }
  if (bound_uoff_ != pending_.uoff) {
    pass_.SetBindGroup(0, uniform_group_, 1, &pending_.uoff);
    bound_uoff_ = pending_.uoff;
  }
  if (bound_textures_ != pending_.textures.Get()) {
    pass_.SetBindGroup(1, pending_.textures);
    bound_textures_ = pending_.textures.Get();
  }
  if (!bound_vbuf_) {
    pass_.SetVertexBuffer(0, vbuf_, 0, kVertexCapacity);
    bound_vbuf_ = true;
  }
  if (!bound_ibuf_) {
    pass_.SetIndexBuffer(ibuf_, wgpu::IndexFormat::Uint32, 0, kIndexCapacity);
    bound_ibuf_ = true;
  }
  if (std::memcmp(bound_scissor_, pending_.scissor, sizeof bound_scissor_) != 0) {
    const std::uint32_t s = scale_;  // Q1: the scissor in console pixels, the EFB at the scale
    pass_.SetScissorRect(pending_.scissor[0] * s, pending_.scissor[1] * s, (pending_.scissor[2] - pending_.scissor[0]) * s,
                         (pending_.scissor[3] - pending_.scissor[1]) * s);
    std::memcpy(bound_scissor_, pending_.scissor, sizeof bound_scissor_);
  }
  pass_.DrawIndexed(pending_.index_count, 1, pending_.first_index, static_cast<std::int32_t>(pending_.base), 0);
  ++stats_.encoded;
  if (recording_) rec_draw();  // V3
}

void Renderer::flush() {
  if (!pass_open_) return;
  emit_pending();
  pass_.End();
  pass_ = nullptr;
  if (!vdata_.empty()) gpu_.queue().WriteBuffer(vbuf_, 0, vdata_.data(), vdata_.size());
  if (!idata_.empty()) gpu_.queue().WriteBuffer(ibuf_, 0, idata_.data(), idata_.size() * 4);
  if (!udata_.empty()) gpu_.queue().WriteBuffer(ubuf_, 0, udata_.data(), udata_.size());
  stats_.vertex_bytes += vdata_.size();
  stats_.index_bytes += idata_.size() * 4;
  if (recording_) rec_pass_end();  // V3: uploaded; the record keeps the bytes
  wgpu::CommandBuffer cb = encoder_.Finish();
  gpu_.queue().Submit(1, &cb);
  encoder_ = nullptr;
  pass_open_ = false;
}

MVP_PROF_NOINLINE const Renderer::Pipeline& Renderer::pipeline(const PipelineConfig& c) {
  auto it = pipelines_.find(c);
  if (it != pipelines_.end()) return it->second;
  std::string wgsl = generate_wgsl(c);
  wgpu::ShaderModule& sm = modules_[wgsl];
  if (!sm) sm = gpu_.shader(wgsl, "tev");

  // Only the attributes the shader reads, packed (tev_wgsl.h).
  Pipeline info;
  info.attrs = vertex_attrs(c);
  info.stride = vertex_stride(info.attrs);
  wgpu::VertexAttribute attrs[11]{};
  std::uint32_t na = 0, off = 0;
  attrs[na].shaderLocation = 0;
  attrs[na].format = wgpu::VertexFormat::Float32x4;
  attrs[na++].offset = off;
  off += 16;
  for (std::uint32_t k = 0; k < 2; ++k)
    if (info.attrs & (1u << k)) {
      attrs[na].shaderLocation = 1 + k;
      attrs[na].format = wgpu::VertexFormat::Float32x4;
      attrs[na++].offset = off;
      off += 16;
    }
  for (std::uint32_t k = 0; k < 8; ++k)
    if (info.attrs & (4u << k)) {
      attrs[na].shaderLocation = 3 + k;
      attrs[na].format = wgpu::VertexFormat::Float32x3;
      attrs[na++].offset = off;
      off += 12;
    }
  wgpu::VertexBufferLayout vbl{};
  vbl.arrayStride = info.stride;
  vbl.attributeCount = na;
  vbl.attributes = attrs;

  const bool has_alpha = efb_has_alpha(c.pixel_format);
  wgpu::BlendState blend{};
  wgpu::ColorTargetState ct{};
  ct.format = wgpu::TextureFormat::RGBA8Unorm;
  if (c.blend_enable) {
    if (c.blend_sub) {
      blend.color = {wgpu::BlendOperation::ReverseSubtract, wgpu::BlendFactor::One, wgpu::BlendFactor::One};
      blend.alpha = blend.color;
    } else {
      blend.color = {wgpu::BlendOperation::Add, blend_factor(c.blend_src, false, false, has_alpha),
                     blend_factor(c.blend_dst, true, false, has_alpha)};
      blend.alpha = {wgpu::BlendOperation::Add, blend_factor(c.blend_src, false, true, has_alpha),
                     blend_factor(c.blend_dst, true, true, has_alpha)};
    }
    ct.blend = &blend;
  }
  wgpu::ColorWriteMask mask = wgpu::ColorWriteMask::None;
  const bool noop = c.logic_enable && !c.blend_enable && c.logic_op == 5;
  if (c.color_update && !noop) mask = mask | wgpu::ColorWriteMask::Red | wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Blue;
  if (c.alpha_update && has_alpha && !noop) mask = mask | wgpu::ColorWriteMask::Alpha;
  ct.writeMask = mask;

  wgpu::FragmentState fs{};
  fs.module = sm;
  fs.entryPoint = "fs_main";
  fs.targetCount = 1;
  fs.targets = &ct;
  wgpu::DepthStencilState ds{};
  ds.format = wgpu::TextureFormat::Depth32Float;
  ds.depthCompare = c.z_enable ? compare(c.z_func) : wgpu::CompareFunction::Always;
  ds.depthWriteEnabled = (c.z_enable && c.z_update) ? wgpu::OptionalBool::True : wgpu::OptionalBool::False;

  wgpu::RenderPipelineDescriptor pd{};
  pd.label = "tev";
  pd.layout = layout_;
  pd.vertex.module = sm;
  pd.vertex.entryPoint = "vs_main";
  pd.vertex.bufferCount = 1;
  pd.vertex.buffers = &vbl;
  pd.fragment = &fs;
  pd.depthStencil = &ds;
  const Topology topo = static_cast<Topology>(c.topology);
  pd.primitive.topology = topo == Topology::Lines    ? wgpu::PrimitiveTopology::LineList
                          : topo == Topology::Points ? wgpu::PrimitiveTopology::PointList
                                                     : wgpu::PrimitiveTopology::TriangleList;
  pd.primitive.frontFace = wgpu::FrontFace::CW;
  pd.primitive.cullMode = topo != Topology::Triangles ? wgpu::CullMode::None
                          : c.cull == 1               ? wgpu::CullMode::Back
                          : c.cull == 2               ? wgpu::CullMode::Front
                                                      : wgpu::CullMode::None;
  info.pipe = gpu_.device().CreateRenderPipeline(&pd);
  ++stats_.pipelines;
  return pipelines_.emplace(c, std::move(info)).first->second;
}

void Renderer::forget_view(const wgpu::TextureView& v) {
  if (!v) return;
  const void* p = v.Get();
  bound_textures_ = nullptr;  // a group freed here could lend its address to the next one: bind again
  for (auto it = bind_groups_.begin(); it != bind_groups_.end();) {
    bool uses = false;
    for (std::size_t k = 0; k < 8 && !uses; ++k) uses = it->first[k] == p;
    it = uses ? bind_groups_.erase(it) : std::next(it);
  }
}

const std::uint8_t* Renderer::guest(std::uint32_t addr, std::uint32_t size) {
  if (gpm_) return gpm_->view(addr, size, 1);
  addr &= 0x01FFFFFFu;
  if (!mem_ || addr > mem_size_ || size > mem_size_ - addr) return nullptr;
  return mem_ + addr;
}

std::uint64_t Renderer::hash_memory(std::uint32_t addr, std::uint32_t size) {
  const std::uint8_t* p = guest(addr, size);
  return p ? hash_bytes(p, size) : 0;
}

MVP_PROF_NOINLINE wgpu::Sampler Renderer::sampler(const TextureDesc& t) {
  const std::uint32_t mip = bits(t.min_filter, 0, 2);
  const std::uint32_t key = t.wrap_s | t.wrap_t << 2 | t.mag_linear << 4 | bits(t.min_filter, 2, 1) << 5 |
                            (mip == 2 ? 1u : 0u) << 6 | (mip ? 1u : 0u) << 7 |
                            static_cast<std::uint32_t>(std::clamp(t.max_lod, 0.0f, 15.0f) * 16) << 8 |
                            static_cast<std::uint32_t>(std::clamp(t.min_lod, 0.0f, 15.0f) * 16) << 16;
  wgpu::Sampler& s = samplers_[key];
  if (!s) {
    wgpu::SamplerDescriptor sd{};
    sd.addressModeU = address(t.wrap_s);
    sd.addressModeV = address(t.wrap_t);
    sd.magFilter = t.mag_linear ? wgpu::FilterMode::Linear : wgpu::FilterMode::Nearest;
    sd.minFilter = bits(t.min_filter, 2, 1) ? wgpu::FilterMode::Linear : wgpu::FilterMode::Nearest;
    sd.mipmapFilter = mip == 2 ? wgpu::MipmapFilterMode::Linear : wgpu::MipmapFilterMode::Nearest;
    sd.lodMinClamp = mip ? t.min_lod : 0.0f;
    sd.lodMaxClamp = mip ? std::max(t.min_lod, t.max_lod) : 0.0f;
    s = gpu_.device().CreateSampler(&sd);
  }
  return s;
}

MVP_PROF_NOINLINE wgpu::TextureView Renderer::texture(const TextureDesc& t, std::uint32_t* w, std::uint32_t* h) {
  *w = t.width;
  *h = t.height;
  if (t.tmem) return tmem_texture(t);
  // An EFB copy at this address (the copy lives only on the GPU).
  auto ci = copies_.find(t.addr & 0x01FFFFFFu);
  if (ci != copies_.end()) {
    CopyEntry& e = *ci->second;
    if (e.epoch != epoch_) {  // memory written there since the copy means the CPU replaced it
      if (hash_memory(t.addr, e.bytes) != e.mem_hash) {
        ++stats_.copies_dropped;
        forget_view(e.view);
        if (e.tall) forget_view(e.tall->view);
        copies_.erase(ci);
        ci = copies_.end();
      } else {
        e.epoch = epoch_;
      }
    }
    if (ci != copies_.end() && e.w == t.width && e.h == t.height) {
      ++stats_.copy_textures_used;
      if (recording_ && e.rec != rec_[rec_cur_].id) ++istats_.early_copies;  // V3: sampled before this frame redid it
      return e.view;
    }
    // P2: sampled taller than the copy, at the same width (the game samples a
    // 256 x 224 half-scale copy as a 256 x 256 texture): the hardware reads the
    // rows the copy wrote and, below them, whatever memory holds.
    if (ci != copies_.end() && e.w == t.width && e.h < t.height && t.levels == 1 && !t.tlut &&
        texture_format_known(t.format)) {
      wgpu::TextureView v = tall_copy(e, t);
      if (v) return v;
    }
    if (ci != copies_.end()) ++stats_.copy_size_misses;
  } else if (!copies_.empty()) {
    // Q7 (diagnostic): the texture starts inside an EFB copy's memory.
    auto below = copies_.upper_bound(t.addr & 0x01FFFFFFu);
    if (below != copies_.begin()) {
      --below;
      if ((t.addr & 0x01FFFFFFu) < below->first + below->second->bytes) ++stats_.copy_inside;
    }
  }
  if (!texture_format_known(t.format)) {
    ++stats_.unknown_formats;
    return dummy_view_;
  }
  const std::uint32_t bytes = texture_size(t.format, t.width, t.height, t.levels);
  std::uint64_t tlut_hash = 0;
  if (t.tlut) tlut_hash = hash_bytes(t.tlut, static_cast<std::size_t>(t.tlut_entries) * 2, t.tlut_format + 1u);
  const std::uint64_t key = mix64(t.addr ^ (std::uint64_t(t.format) << 32) ^ (std::uint64_t(t.width) << 36) ^
                                  (std::uint64_t(t.height) << 48) ^ (std::uint64_t(t.levels) << 60)) ^
                            tlut_hash;
  std::unique_ptr<TexEntry>& slot = textures_[key];
  if (slot && slot->epoch == epoch_) {
    ++stats_.texture_hits;
    slot->pass = pass_open_ ? pass_serial_ : pass_serial_ + 1;
    if (recording_) slot->rec = rec_[rec_cur_].id;
    return slot->view;
  }
  const std::uint8_t* src = guest(t.addr, bytes);  // Q1: memory as the GP sees it
  const std::uint64_t mh = src ? hash_bytes(src, bytes) : 0;
  if (slot && slot->mem_hash == mh) {
    slot->epoch = epoch_;
    slot->pass = pass_open_ ? pass_serial_ : pass_serial_ + 1;
    if (recording_) slot->rec = rec_[rec_cur_].id;
    ++stats_.texture_hits;
    return slot->view;
  }
  if (!src) return dummy_view_;
  // V3: new contents for a texture this frame already sampled: the in-between shows the new ones.
  if (recording_ && slot && slot->rec == rec_[rec_cur_].id) ++istats_.retextured;
  // The open pass may already sample this texture's old contents: submit it
  // before the upload replaces them.
  if (slot && pass_open_ && slot->pass == pass_serial_) flush();
  if (!slot) {
    slot = std::make_unique<TexEntry>();
    wgpu::TextureDescriptor td{};
    td.size = {t.width, t.height, 1};
    td.format = wgpu::TextureFormat::RGBA8Unorm;
    td.mipLevelCount = t.levels;
    td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    slot->tex = gpu_.device().CreateTexture(&td);
    slot->view = slot->tex.CreateView();
    slot->w = t.width;
    slot->h = t.height;
  }
  // Decode and upload every level.
  std::uint32_t lw = t.width, lh = t.height, off = 0;
  std::vector<std::uint8_t> rgba;
  for (std::uint32_t l = 0; l < t.levels; ++l) {
    rgba.resize(static_cast<std::size_t>(lw) * lh * 4);
    decode_texture(t.format, lw, lh, src + off, rgba.data(), t.tlut, t.tlut_format, t.tlut_entries);
    wgpu::TexelCopyTextureInfo dst{};
    dst.texture = slot->tex;
    dst.mipLevel = l;
    wgpu::TexelCopyBufferLayout lay{};
    lay.bytesPerRow = lw * 4;
    lay.rowsPerImage = lh;
    wgpu::Extent3D ext{lw, lh, 1};
    gpu_.queue().WriteTexture(&dst, rgba.data(), rgba.size(), &lay, &ext);
    off += texture_level_size(t.format, lw, lh);
    lw = lw > 1 ? lw / 2 : 1;
    lh = lh > 1 ? lh / 2 : 1;
  }
  slot->mem_hash = mh;
  slot->epoch = epoch_;
  slot->pass = pass_open_ ? pass_serial_ : pass_serial_ + 1;
  if (recording_) slot->rec = rec_[rec_cur_].id;
  ++stats_.textures_decoded;
  return slot->view;
}

// A texture taller than the EFB copy at its address, of the same width (P2):
// the copy's rows on top, the rest decoded from memory after the bytes the copy
// covers (in the texture's format the copy's rows take whole block rows, so the
// rows below start right after them). Rebuilt when the copy is copied again or
// that memory changes. Null if the texture lies outside memory.
wgpu::TextureView Renderer::tall_copy(CopyEntry& e, const TextureDesc& t) {
  const std::uint32_t bytes = texture_level_size(t.format, t.width, t.height);
  const std::uint32_t covered = std::min(texture_level_size(t.format, t.width, e.h), bytes);
  const std::uint8_t* src = guest(t.addr, bytes);  // Q1: memory as the GP sees it
  if (!src) return nullptr;
  const std::uint32_t s = e.w ? e.pw / e.w : 1;    // Q1: the copy's internal resolution
  std::unique_ptr<CopyEntry::Tall>& c = e.tall;
  const bool fresh = !c || c->h != t.height || c->scale != s;
  if (fresh) {
    if (c) forget_view(c->view);
    c = std::make_unique<CopyEntry::Tall>();
    wgpu::TextureDescriptor td{};
    td.size = {t.width * s, t.height * s, 1};
    td.format = wgpu::TextureFormat::RGBA8Unorm;
    td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    td.label = "efb copy (tall)";
    c->tex = gpu_.device().CreateTexture(&td);
    c->view = c->tex.CreateView();
    c->h = t.height;
    c->scale = s;
  }
  // The memory below the copy is read once per texture cache epoch, as every
  // texture's is (the threaded engine relies on it: gfx/gp_reads.h).
  std::uint64_t tail = c->tail_hash;
  if (fresh || c->epoch != epoch_ || c->format != t.format) {
    tail = hash_bytes(src + covered, bytes - covered, t.format + 1u);
    c->epoch = epoch_;
  }
  const bool new_tail = fresh || c->tail_hash != tail || c->format != t.format;
  if (new_tail || c->copy_serial != e.serial) {
    // The open pass may already sample the old contents: submit it first.
    if (!fresh && pass_open_ && c->pass == pass_serial_) flush();
    if (new_tail) {
      std::vector<std::uint8_t> rgba(static_cast<std::size_t>(t.width) * t.height * 4);
      decode_texture(t.format, t.width, t.height, src, rgba.data(), nullptr, 0, 0);
      const std::uint32_t rows = t.height - e.h;
      // Memory's rows at the copy's scale (each texel s x s).
      const std::uint32_t pw = t.width * s, prows = rows * s;
      std::vector<std::uint8_t> up(static_cast<std::size_t>(pw) * prows * 4);
      for (std::uint32_t y = 0; y < prows; ++y) {
        const std::uint8_t* row = rgba.data() + (static_cast<std::size_t>(e.h + y / s) * t.width) * 4;
        std::uint8_t* out = up.data() + static_cast<std::size_t>(y) * pw * 4;
        for (std::uint32_t x = 0; x < pw; ++x) std::memcpy(out + x * 4, row + (x / s) * 4, 4);
      }
      wgpu::TexelCopyTextureInfo dst{};
      dst.texture = c->tex;
      dst.origin = {0, e.h * s, 0};
      wgpu::TexelCopyBufferLayout lay{};
      lay.bytesPerRow = pw * 4;
      lay.rowsPerImage = prows;
      wgpu::Extent3D ext{pw, prows, 1};
      gpu_.queue().WriteTexture(&dst, up.data(), up.size(), &lay, &ext);
      c->tail_hash = tail;
      c->format = t.format;
    }
    wgpu::CommandEncoder enc = gpu_.device().CreateCommandEncoder();
    wgpu::TexelCopyTextureInfo csrc{};
    csrc.texture = e.tex;
    wgpu::TexelCopyTextureInfo cdst{};
    cdst.texture = c->tex;
    wgpu::Extent3D ext{e.pw, e.ph, 1};
    enc.CopyTextureToTexture(&csrc, &cdst, &ext);
    wgpu::CommandBuffer cb = enc.Finish();
    gpu_.queue().Submit(1, &cb);
    c->copy_serial = e.serial;
  }
  c->pass = pass_open_ ? pass_serial_ : pass_serial_ + 1;
  ++stats_.copy_textures_used;
  ++stats_.copy_tall;
  return c->view;
}

// A preloaded texture (E3c): its texels come from TMEM, which only preloads
// change, so it is cached by its content.
wgpu::TextureView Renderer::tmem_texture(const TextureDesc& t) {
  if (!texture_format_known(t.format)) {
    ++stats_.unknown_formats;
    return dummy_view_;
  }
  if (!gather_tmem_texture(t.tmem, t.tmem_even, t.tmem_odd, t.format, t.width, t.height, t.levels, tmem_buf_))
    return dummy_view_;
  std::uint64_t key = hash_bytes(tmem_buf_.data(), tmem_buf_.size(), 0x544D454Dull);
  if (t.tlut) key ^= hash_bytes(t.tlut, static_cast<std::size_t>(t.tlut_entries) * 2, t.tlut_format + 1u);
  key = mix64(key ^ (std::uint64_t(t.format) << 32) ^ (std::uint64_t(t.width) << 36) ^ (std::uint64_t(t.height) << 48) ^
              (std::uint64_t(t.levels) << 60));
  std::unique_ptr<TexEntry>& slot = tmem_textures_[key];
  if (slot) {
    ++stats_.texture_hits;
    return slot->view;
  }
  slot = std::make_unique<TexEntry>();
  wgpu::TextureDescriptor td{};
  td.size = {t.width, t.height, 1};
  td.format = wgpu::TextureFormat::RGBA8Unorm;
  td.mipLevelCount = t.levels;
  td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
  slot->tex = gpu_.device().CreateTexture(&td);
  slot->view = slot->tex.CreateView();
  slot->w = t.width;
  slot->h = t.height;
  std::uint32_t lw = t.width, lh = t.height, off = 0;
  std::vector<std::uint8_t> rgba;
  for (std::uint32_t l = 0; l < t.levels; ++l) {
    rgba.resize(static_cast<std::size_t>(lw) * lh * 4);
    decode_texture(t.format, lw, lh, tmem_buf_.data() + off, rgba.data(), t.tlut, t.tlut_format, t.tlut_entries);
    wgpu::TexelCopyTextureInfo dst{};
    dst.texture = slot->tex;
    dst.mipLevel = l;
    wgpu::TexelCopyBufferLayout lay{};
    lay.bytesPerRow = lw * 4;
    lay.rowsPerImage = lh;
    wgpu::Extent3D ext{lw, lh, 1};
    gpu_.queue().WriteTexture(&dst, rgba.data(), rgba.size(), &lay, &ext);
    off += texture_level_size(t.format, lw, lh);
    lw = lw > 1 ? lw / 2 : 1;
    lh = lh > 1 ? lh / 2 : 1;
  }
  ++stats_.textures_decoded;
  ++stats_.tmem_textures;
  return slot->view;
}

void Renderer::draw(const DrawCall& d) {
  const PipelineConfig& c = d.cfg;
  if (c.cull == 3 && static_cast<Topology>(c.topology) == Topology::Triangles) {
    ++stats_.skipped;
    return;
  }
  if (d.scissor[2] <= d.scissor[0] || d.scissor[3] <= d.scissor[1] || d.index.empty()) {
    ++stats_.skipped;
    return;
  }
  // Room for the largest layout (144 bytes per vertex, plus alignment).
  const std::size_t vmax = (d.verts.size() + 1) * sizeof(OutVertex);
  const std::size_t ibytes = d.index.size() * sizeof(std::uint32_t);
  if (vmax > kVertexCapacity || ibytes > kIndexCapacity) {
    ++stats_.skipped;
    return;
  }
  if (pass_open_ && (vdata_.size() + vmax > kVertexCapacity || idata_.size() * 4 + ibytes > kIndexCapacity ||
                     udata_.size() + kUniformSlot > kUniformCapacity))
    flush();

  // The same pixel state as the previous draw, in the same pass and texture
  // epoch: its pipeline, textures and constants (nothing they come from changed).
  const bool same = d.state != 0 && d.state == last_.state && c.topology == last_.topology && pass_open_ &&
                    pass_serial_ == last_.pass && epoch_ == last_.epoch &&
                    (!recording_ || last_.bind_rec == rec_[rec_cur_].id);  // V3: its bind group is in the record
  if (same) {
    ++stats_.same_state;
  } else {
    // Textures first: decoding uploads with the queue, which must precede the pass's submit (it does).
    wgpu::BindGroupEntry te[16]{};
    for (int k = 0; k < 8; ++k) {
      te[k].binding = static_cast<std::uint32_t>(k);
      te[k].textureView = dummy_view_;
      te[8 + k].binding = static_cast<std::uint32_t>(8 + k);
      te[8 + k].sampler = dummy_sampler_;
    }
    for (int k = 0; k < 8; ++k) {
      const TextureDesc& t = d.tex[static_cast<std::size_t>(k)];
      if (!t.valid) continue;
      std::uint32_t w = 0, h = 0;
      te[k].textureView = texture(t, &w, &h);
      te[8 + k].sampler = sampler(t);
    }
    last_.indirect = false;
    for (int s = 0; s < c.num_stages; ++s)
      if (c.stages[s].ind_cmd) {
        last_.indirect = true;
        break;
      }
    last_.fog = c.fog_type != 0;
    last_.pipe = &pipeline(c);
    // Texture bind groups by their views and samplers (handle addresses). Q7: the
    // entry keeps its own references to them, so no address in a key can belong
    // to another object while the entry exists (a bind group alone does not keep
    // them: in the browser a released view's handle is freed and the next view
    // can get its address; see BindEntry).
    std::array<const void*, 16> key{};
    for (int k = 0; k < 8; ++k) {
      key[static_cast<std::size_t>(k)] = te[k].textureView.Get();
      key[static_cast<std::size_t>(8 + k)] = te[8 + k].sampler.Get();
    }
    if (bind_groups_.size() > 4096) {
      bind_groups_.clear();
      bound_textures_ = nullptr;  // as in forget_view()
    }
    BindEntry& be = bind_groups_[key];
    if (!be.group) {
      wgpu::BindGroupDescriptor bgd{};
      bgd.layout = bgl_textures_;
      bgd.entryCount = 16;
      bgd.entries = te;
      be.group = gpu_.device().CreateBindGroup(&bgd);
      for (int k = 0; k < 8; ++k) {
        be.views[static_cast<std::size_t>(k)] = te[k].textureView;
        be.samplers[static_cast<std::size_t>(k)] = te[8 + k].sampler;
      }
    }
    last_.textures = be.group;
    if (recording_) {  // V3
      last_.bind = rec_bind(be);
      last_.bind_rec = rec_[rec_cur_].id;
    }
  }
  if (last_.indirect) ++stats_.indirect_draws;
  if (last_.fog) ++stats_.fog_draws;
  const Pipeline& pl = *last_.pipe;

  begin_pass();
  if (recording_ && rec_[rec_cur_].pass_serial != pass_serial_) {  // V3: a new pass in the frame record
    Record& r = rec_[rec_cur_];
    if (r.npasses == r.passes.size()) r.passes.emplace_back();
    RecPass& rp = r.passes[r.npasses];
    rp.draws.clear();
    rp.filled = false;
    r.ops.push_back({RecKind::Pass, static_cast<std::uint32_t>(r.npasses)});
    ++r.npasses;
    r.pass_serial = pass_serial_;
  }
  last_.state = d.state;
  last_.topology = c.topology;
  last_.pass = pass_serial_;
  last_.epoch = epoch_;
  ++stats_.draws;
  const std::uint32_t stride = pl.stride;
  // The same state as the recorded draw: append to it. Otherwise record a new
  // draw whose vertices start at a multiple of its stride.
  const bool merge = pending_.active && pending_.pipeline.Get() == pl.pipe.Get() &&
                     pending_.textures.Get() == last_.textures.Get() &&
                     ((d.state != 0 && pending_.state == d.state) ||
                      (std::memcmp(pending_.scissor, d.scissor, sizeof pending_.scissor) == 0 &&
                       std::memcmp(&pending_.u, &d.u, sizeof d.u) == 0));
  if (merge) {
    ++stats_.merged;
  } else {
    emit_pending();
    // A new uniform slot unless the constants are those of the draw before.
    std::uint32_t uoff;
    if (!udata_.empty() && std::memcmp(udata_.data() + (udata_.size() - kUniformSlot), &d.u, sizeof d.u) == 0) {
      uoff = static_cast<std::uint32_t>(udata_.size() - kUniformSlot);
    } else {
      uoff = static_cast<std::uint32_t>(udata_.size());
      udata_.resize(udata_.size() + kUniformSlot, 0);
      std::memcpy(udata_.data() + uoff, &d.u, sizeof d.u);
    }
    const std::size_t base = (vdata_.size() + stride - 1) / stride;
    vdata_.resize(base * stride);
    pending_.active = true;
    pending_.pipeline = pl.pipe;
    pending_.textures = last_.textures;
    pending_.uoff = uoff;
    pending_.first_index = static_cast<std::uint32_t>(idata_.size());
    pending_.index_count = 0;
    pending_.base = static_cast<std::uint32_t>(base);
    pending_.vertices = 0;
    pending_.stride = stride;
    std::memcpy(pending_.scissor, d.scissor, sizeof pending_.scissor);
    pending_.u = d.u;
    pending_.pipe = &pl;  // V3: for the frame record
    pending_.bind = last_.bind;
  }
  pending_.state = d.state;
  // Vertices, packed to the pipeline's layout.
  //
  // Q8: above 1x, a 2D vertex (orthographic: w is exactly 1) goes to a whole
  // console pixel, as Dolphin's vertex rounding does. A pixel is drawn when its
  // center is inside, so at 1x an edge at y = 55.44 covers row 55 (center 55.5)
  // and the row is filled; at 3x the row's top third (55.17) stays out while the
  // scissor, in whole pixels, still lets later draws write there. The pitching
  // view's mini-map showed a line of the field's dirt that way, over its black
  // backing quad. ceil(v - 0.5) is the edge, in whole pixels, of the pixels a
  // horizontal or vertical edge at v covers at 1x (a center exactly on it counts
  // for a top or left edge, as the rasterizer's rule has it), so such a 2D edge
  // covers the console pixels it covers at 1x, whole, at every scale. 1x is
  // drawn as is.
  const std::size_t voff = vdata_.size();
  vdata_.resize(voff + d.verts.size() * stride);
  pack_vertices(d.verts.data(), d.verts.size(), pl.attrs, scale_ > 1, vdata_.data() + voff);
  if (recording_) rec_range(d, voff, stride);  // V3: where this geometry's vertices went
  // Indices, relative to the recorded draw's first vertex.
  const std::uint32_t first = pending_.vertices;
  const std::size_t ioff = idata_.size();
  idata_.resize(ioff + d.index.size());
  for (std::size_t k = 0; k < d.index.size(); ++k) idata_[ioff + k] = first + d.index[k];
  pending_.index_count += static_cast<std::uint32_t>(d.index.size());
  pending_.vertices += static_cast<std::uint32_t>(d.verts.size());
}

void Renderer::clear_efb(const EfbCopy& c) {
  ++stats_.clears;
  const bool has_alpha = efb_has_alpha(c.pixel_format);
  // Formats without alpha keep the EFB's alpha at 1 (written with the color).
  const bool alpha = has_alpha ? c.clear_alpha : c.clear_color;
  const int m = (c.clear_color ? 1 : 0) | (alpha ? 2 : 0) | (c.clear_depth ? 4 : 0);
  if (m == 0) return;
  float data[8] = {c.clear_rgba[0] / 255.0f, c.clear_rgba[1] / 255.0f, c.clear_rgba[2] / 255.0f,
                   has_alpha ? c.clear_rgba[3] / 255.0f : 1.0f, static_cast<float>(c.clear_z) / 16777215.0f, 0, 0, 0};
  flush();
  gpu_.queue().WriteBuffer(clear_buf_, 0, data, sizeof data);
  wgpu::BindGroupEntry be{};
  be.binding = 0;
  be.buffer = clear_buf_;
  be.size = 32;
  wgpu::BindGroupDescriptor bgd{};
  bgd.layout = bgl_clear_;
  bgd.entryCount = 1;
  bgd.entries = &be;
  wgpu::BindGroup bg = gpu_.device().CreateBindGroup(&bgd);
  begin_pass();
  pass_.SetPipeline(clear_pipes_[m]);
  pass_.SetBindGroup(0, bg);
  std::uint32_t x0 = std::min(c.src_x, kEfbWidth), y0 = std::min(c.src_y, kEfbHeight);
  std::uint32_t x1 = std::min(c.src_x + c.width, kEfbWidth), y1 = std::min(c.src_y + c.height, kEfbHeight);
  if (x1 <= x0 || y1 <= y0) return;
  pass_.SetScissorRect(x0 * scale_, y0 * scale_, (x1 - x0) * scale_, (y1 - y0) * scale_);
  pass_.Draw(3);
  if (recording_) {  // V3
    const std::uint32_t rect[4] = {x0 * scale_, y0 * scale_, (x1 - x0) * scale_, (y1 - y0) * scale_};
    rec_clear(m, data, rect);
  }
  // The clear must land before the next clear rewrites its buffer.
  flush();
}

void Renderer::copy_to_texture(const EfbCopy& c) {
  ++stats_.tex_copies;
  const std::uint32_t w = c.half_scale ? std::max(1u, c.width / 2) : c.width;
  const std::uint32_t h = c.half_scale ? std::max(1u, c.height / 2) : c.height;
  const std::uint32_t addr = c.dest & 0x01FFFFFFu;
  std::unique_ptr<CopyEntry>& e = copies_[addr];
  if (!e || e->w != w || e->h != h || e->pw != w * scale_ || e->ph != h * scale_) {
    if (e) {  // Q7: a new texture (another size, or another scale): the old one's bind groups go with it
      forget_view(e->view);
      if (e->tall) forget_view(e->tall->view);
    }
    e = std::make_unique<CopyEntry>();
    wgpu::TextureDescriptor td{};
    td.size = {w * scale_, h * scale_, 1};
    td.format = wgpu::TextureFormat::RGBA8Unorm;
    td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    td.label = "efb copy";
    e->tex = gpu_.device().CreateTexture(&td);
    e->view = e->tex.CreateView();
    e->w = w;
    e->h = h;
    e->pw = w * scale_;
    e->ph = h * scale_;
  }
  // What a texture of this format and size covers in memory; the copy stays
  // valid while those bytes are unchanged.
  const std::uint8_t sample_fmt = c.tex_format <= 6 ? c.tex_format : kTexI8;
  e->bytes = texture_level_size(sample_fmt, w, h);
  e->mem_hash = hash_memory(addr, e->bytes);
  e->epoch = epoch_;
  e->serial = ++copy_serial_;

  std::int32_t params[8] = {static_cast<std::int32_t>(c.src_x * scale_), static_cast<std::int32_t>(c.src_y * scale_),
                            c.half_scale ? 1 : 0, c.pixel_format == 3 ? 1 : 0,
                            static_cast<std::int32_t>(c.tex_format), c.intensity ? 1 : 0, 0, 0};
  gpu_.queue().WriteBuffer(convert_buf_, 0, params, sizeof params);
  wgpu::BindGroupEntry be[3]{};
  be[0].binding = 0;
  be[0].buffer = convert_buf_;
  be[0].size = 32;
  be[1].binding = 1;
  be[1].textureView = efb_color_view_;
  be[2].binding = 2;
  be[2].textureView = efb_depth_view_;
  wgpu::BindGroupDescriptor bgd{};
  bgd.layout = bgl_convert_;
  bgd.entryCount = 3;
  bgd.entries = be;
  wgpu::BindGroup bg = gpu_.device().CreateBindGroup(&bgd);
  wgpu::CommandEncoder enc = gpu_.device().CreateCommandEncoder();
  wgpu::RenderPassColorAttachment ca{};
  ca.view = e->view;
  ca.loadOp = wgpu::LoadOp::Clear;
  ca.storeOp = wgpu::StoreOp::Store;
  wgpu::RenderPassDescriptor rp{};
  rp.colorAttachmentCount = 1;
  rp.colorAttachments = &ca;
  if (timer_) rp.timestampWrites = timer_->pass();  // Q5
  wgpu::RenderPassEncoder pass = enc.BeginRenderPass(&rp);
  pass.SetPipeline(convert_pipe_);
  pass.SetBindGroup(0, bg);
  pass.Draw(3);
  pass.End();
  wgpu::CommandBuffer cb = enc.Finish();
  gpu_.queue().Submit(1, &cb);
  if (recording_) {  // V3
    e->rec = rec_[rec_cur_].id;
    rec_copy(e->view, e->pw, e->ph, params);
  }
}

void Renderer::efb_copy(const EfbCopy& c) {
  flush();
  const std::uint32_t x0 = std::min(c.src_x, kEfbWidth), y0 = std::min(c.src_y, kEfbHeight);
  const std::uint32_t w = std::min(c.width, kEfbWidth - x0), h = std::min(c.height, kEfbHeight - y0);
  if (w && h) {
    if (c.to_xfb) {
      ++stats_.xfb_copies;
      Frame& f = frames_[c.dest & 0x01FFFFFFu];
      const std::uint32_t pw = w * scale_, ph = h * scale_;  // Q1: the frame at the internal resolution
      if (!f.tex || f.width != pw || f.height != ph) {
        wgpu::TextureDescriptor td{};
        td.size = {pw, ph, 1};
        td.format = wgpu::TextureFormat::RGBA8Unorm;
        td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        td.label = "xfb";
        f.tex = gpu_.device().CreateTexture(&td);
        f.view = f.tex.CreateView();
        f.width = pw;
        f.height = ph;
      }
      f.addr = c.dest & 0x01FFFFFFu;
      f.serial = ++serial_;
      latest_ = &f;
      wgpu::CommandEncoder enc = gpu_.device().CreateCommandEncoder();
      wgpu::TexelCopyTextureInfo src{};
      src.texture = efb_color_;
      src.origin = {x0 * scale_, y0 * scale_, 0};
      wgpu::TexelCopyTextureInfo dst{};
      dst.texture = f.tex;
      wgpu::Extent3D ext{pw, ph, 1};
      enc.CopyTextureToTexture(&src, &dst, &ext);
      wgpu::CommandBuffer cb = enc.Finish();
      gpu_.queue().Submit(1, &cb);
      // V3: the frame ends here: its in-between picture, then the next frame's record
      // (which starts with this copy's clear).
      if (recording_) rec_end(x0, y0, w, h, f.serial);
      if (interp_on_) rec_begin(c);
    } else {
      EfbCopy cc = c;
      cc.src_x = x0;
      cc.src_y = y0;
      cc.width = w;
      cc.height = h;
      copy_to_texture(cc);
    }
  }
  if (c.clear) clear_efb(c);
  // Q1: a new internal resolution starts with a frame: after an XFB copy that
  // clears (the game's end of frame). The new EFB starts all cleared with that
  // copy's clear values.
  if (want_scale_ != scale_ && c.to_xfb && c.clear) {
    flush();
    if (recording_) rec_drop();  // V3: a new scale: the next frames start over
    scale_ = want_scale_;
    create_efb();
    EfbCopy all = c;
    all.src_x = all.src_y = 0;
    all.width = kEfbWidth;
    all.height = kEfbHeight;
    all.clear_color = all.clear_alpha = all.clear_depth = true;
    clear_efb(all);
  }
}

const Renderer::Frame* Renderer::xfb(std::uint32_t addr) const {
  auto it = frames_.find(addr & 0x01FFFFFFu);
  return it == frames_.end() ? nullptr : &it->second;
}

std::vector<std::uint8_t> Renderer::read_frame(const Frame& f) {
  flush();
  auto px = gpu_.read_texture(f.tex, f.width, f.height);
  for (std::size_t i = 3; i < px.size(); i += 4) px[i] = 255;  // the XFB has no alpha
  return px;
}

void Renderer::scan_field(std::uint32_t addr) {
  ++fields_;  // V3: frames one field apart get an in-between
  const Frame* f = xfb(addr);
  if (!f) f = latest_;
  if (!f) return;
  ++scan_stats_.fields;
  // A new size starts the history over.
  if (!scan_[0].tex || scan_[0].width != f->width || scan_[0].height != f->height) {
    for (Frame& s : scan_) {
      wgpu::TextureDescriptor td{};
      td.size = {f->width, f->height, 1};
      td.format = wgpu::TextureFormat::RGBA8Unorm;
      td.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
      td.label = "scanout";
      s.tex = gpu_.device().CreateTexture(&td);
      s.view = s.tex.CreateView();
      s.width = f->width;
      s.height = f->height;
    }
    for (int k = 0; k < 3; ++k) {
      scan_slot_[k] = -1;
      scan_serial_[k] = 0;
    }
    mixed_key_ = ~0ull;
  }
  const bool same = scan_slot_[0] >= 0 && scan_serial_[0] == f->serial;
  scan_slot_[2] = scan_slot_[1];
  scan_serial_[2] = scan_serial_[1];
  scan_slot_[1] = scan_slot_[0];
  scan_serial_[1] = scan_serial_[0];
  if (same) return;  // the same frame as at the last field: [0] stays
  // A slot the two earlier fields do not use.
  int slot = 0;
  while (slot == scan_slot_[1] || slot == scan_slot_[2]) ++slot;
  ++scan_stats_.new_frames;
  flush();
  wgpu::CommandEncoder enc = gpu_.device().CreateCommandEncoder();
  wgpu::TexelCopyTextureInfo src{};
  src.texture = f->tex;
  wgpu::TexelCopyTextureInfo dst{};
  dst.texture = scan_[slot].tex;
  wgpu::Extent3D ext{f->width, f->height, 1};
  enc.CopyTextureToTexture(&src, &dst, &ext);
  wgpu::CommandBuffer cb = enc.Finish();
  gpu_.queue().Submit(1, &cb);
  scan_slot_[0] = slot;
  scan_serial_[0] = f->serial;
  scan_[slot].serial = f->serial;
  scan_[slot].addr = f->addr;
}

void Renderer::ensure_mix_pipe() {
  if (mix_pipe_) return;
  wgpu::ShaderModule sm = gpu_.shader(kScanMixWgsl, "scanout mix");
  wgpu::ColorTargetState ct{};
  ct.format = wgpu::TextureFormat::RGBA8Unorm;
  wgpu::FragmentState fs{};
  fs.module = sm;
  fs.entryPoint = "fs";
  fs.targetCount = 1;
  fs.targets = &ct;
  wgpu::RenderPipelineDescriptor pd{};
  pd.label = "scanout mix";
  pd.vertex.module = sm;
  pd.vertex.entryPoint = "vs";
  pd.fragment = &fs;
  mix_pipe_ = gpu_.device().CreateRenderPipeline(&pd);
}

const Renderer::Frame* Renderer::scanout() {
  if (scan_slot_[0] < 0) return nullptr;
  const Frame& a = scan_[scan_slot_[0]];
  // Alternation needs three different frames at the last three fields.
  if (!scan_mix || scan_slot_[1] < 0 || scan_slot_[2] < 0 || scan_serial_[1] == scan_serial_[0] ||
      scan_serial_[2] == scan_serial_[1])
    return &a;
  const std::uint64_t key =
      scan_serial_[0] * 0x9E3779B97F4A7C15ull ^ scan_serial_[1] * 0xC2B2AE3D27D4EB4Full ^ scan_serial_[2];
  if (key == mixed_key_) return &mixed_;
  ensure_mix_pipe();
  if (!mixed_.tex || mixed_.width != a.width || mixed_.height != a.height) {
    wgpu::TextureDescriptor td{};
    td.size = {a.width, a.height, 1};
    td.format = wgpu::TextureFormat::RGBA8Unorm;
    td.usage =
        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    td.label = "scanout mixed";
    mixed_.tex = gpu_.device().CreateTexture(&td);
    mixed_.view = mixed_.tex.CreateView();
    mixed_.width = a.width;
    mixed_.height = a.height;
  }
  wgpu::BindGroupEntry be[3]{};
  for (int k = 0; k < 3; ++k) {
    be[k].binding = static_cast<std::uint32_t>(k);
    be[k].textureView = scan_[scan_slot_[k]].view;
  }
  wgpu::BindGroupDescriptor bgd{};
  bgd.layout = mix_pipe_.GetBindGroupLayout(0);
  bgd.entryCount = 3;
  bgd.entries = be;
  wgpu::BindGroup bg = gpu_.device().CreateBindGroup(&bgd);
  wgpu::CommandEncoder enc = gpu_.device().CreateCommandEncoder();
  wgpu::RenderPassColorAttachment ca{};
  ca.view = mixed_.view;
  ca.loadOp = wgpu::LoadOp::Clear;
  ca.storeOp = wgpu::StoreOp::Store;
  wgpu::RenderPassDescriptor rp{};
  rp.colorAttachmentCount = 1;
  rp.colorAttachments = &ca;
  if (timer_) rp.timestampWrites = timer_->pass();  // Q5
  wgpu::RenderPassEncoder pass = enc.BeginRenderPass(&rp);
  pass.SetPipeline(mix_pipe_);
  pass.SetBindGroup(0, bg);
  pass.Draw(3);
  pass.End();
  wgpu::CommandBuffer cb = enc.Finish();
  gpu_.queue().Submit(1, &cb);
  mixed_.serial = a.serial;
  mixed_.addr = a.addr;
  mixed_key_ = key;
  ++scan_stats_.mixed;
  return &mixed_;
}

std::vector<std::uint8_t> Renderer::read_efb() {
  flush();
  return gpu_.read_texture(efb_color_, kEfbWidth * scale_, kEfbHeight * scale_);
}

}  // namespace mvp::gfx
