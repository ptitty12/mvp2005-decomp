// What the GX state machine (gx_processor.h) hands to a rendering backend
// (E1): draws with transformed vertices, the pixel pipeline configuration and
// its constants, the textures they sample; EFB copies; and display events.
// Everything here is plain data, so the processor is tested without a GPU.
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include "gfx/gx_regs.h"

namespace mvp::gfx {

// A vertex after the transform unit: position in clip space of the whole EFB
// (x, y in [-w, w] cover 640 x 528, y up; z in [0, w] is the 24-bit depth range),
// the two lit color channels (0..1) and eight texture coordinates (s, t, q).
struct OutVertex {
  float pos[4];
  float color[2][4];
  float tex[8][3];
};
static_assert(sizeof(OutVertex) == 144);

enum class Topology : u8 { Triangles, Lines, Points };

// One TEV stage, normalized so that equal configurations compare equal.
struct TevStage {
  u32 color_env = 0;  // BP 0xC0 + 2i (dest 22-23, shift 20-21, clamp 19, sub 18, bias 16-17, a/b/c/d)
  u32 alpha_env = 0;  // BP 0xC1 + 2i (same, plus ras swap 0-1, tex swap 2-3)
  u32 ind_cmd = 0;    // BP 0x10 + i (indirect texturing; 0 = direct)
  u8 texmap = 0, texcoord = 0, tex_enable = 0;
  u8 ras_chan = 7;    // 0 color0, 1 color1, 5 alpha bump, 6 normalized bump, 7 zero
  u8 kcsel = 0, kasel = 0;
  u8 pad[2] = {};
};

// Everything that selects a pixel shader and pipeline state. Compared and
// hashed as bytes: construct with value-initialization and set every field.
struct PipelineConfig {
  u8 num_stages = 1;    // 1-16
  u8 num_texgens = 0;   // texture coordinates the vertices carry
  u8 num_chans = 0;     // color channels
  u8 num_ind = 0;       // indirect stages (0: none)
  TevStage stages[16];
  u8 swap[4][4] = {};   // swap tables: source component for r, g, b, a
  u8 alpha_comp0 = 7, alpha_comp1 = 7, alpha_logic = 0;  // 7 = always
  u8 topology = 0;      // Topology
  // Pixel engine.
  u8 blend_enable = 0, blend_sub = 0, blend_src = 1, blend_dst = 0;
  u8 logic_enable = 0, logic_op = 3;
  u8 color_update = 1, alpha_update = 1;
  u8 z_enable = 0, z_func = 7, z_update = 0;
  u8 cull = 0;          // hardware encoding: 0 none, 1 back, 2 front, 3 all
  u8 pixel_format = 0;  // BP 0x43 bits 0-2
  u8 fog_type = 0;      // BP 0xF1 bits 21-23: 0 off, 2 linear, 4 exp, 5 exp2, 6 backexp, 7 backexp2
  u8 fog_proj = 0;      // BP 0xF1 bit 20: 1 orthographic
  u8 iref_map[4] = {}, iref_coord[4] = {};  // indirect stage texture map and coordinate
  u8 ind_scale[4] = {};                      // indirect stage coordinate scales (s 0-3, t 4-7)
  u8 pad[2] = {};

  bool operator==(const PipelineConfig& o) const { return std::memcmp(this, &o, sizeof *this) == 0; }
};

// Constants the pixel shader reads (one per draw).
struct DrawUniforms {
  std::int32_t tev_regs[4][4] = {};   // PREV, REG0-2 (11-bit signed, r g b a)
  std::int32_t konst[4][4] = {};      // K0-K3 (8-bit)
  std::int32_t alpha_ref[4] = {};     // ref0, ref1
  float tex_scale[8][4] = {};         // texcoord n: (s scale / map width, t scale / map height) for its map
  // Indirect matrices 0-2, two rows each: (ma, mc, me, 17 - scale), (mb, md, mf, 17 - scale),
  // elements 11-bit signed (1.0 = 1024), scale the raw 6-bit exponent field.
  std::int32_t ind_mtx[6][4] = {};
  float fog[8] = {};                  // a, b magnitude, b shift, c; color r g b (0..255), unused
  std::int32_t dst_alpha[4] = {};     // enable, value
  float lod_bias[8] = {};             // Q1: texture map n's LOD bias (TX_SETMODE0), added to the sampled level of detail
};

// A texture as the draw samples it: where it is in guest memory and how to
// decode it. For color-indexed formats, `tlut` points at the palette in the
// processor's TLUT memory (valid until the next TLUT load).
struct TextureDesc {
  bool valid = false;
  u32 addr = 0;            // physical
  u8 format = 0;           // GX texture format (0-14)
  u16 width = 0, height = 0;
  u8 levels = 1;           // mip levels present (1 = none)
  u8 tlut_format = 0;      // IA8, RGB565, RGB5A3
  u32 tlut_offset = 0;     // byte offset in TLUT memory
  const u8* tlut = nullptr;
  u32 tlut_entries = 0;
  // Preloaded textures (E3c): the texels are in TMEM, not at `addr`. `tmem` points at
  // the processor's 1 MiB TMEM (valid until the next preload); levels start in the
  // even bank at `tmem_even` (RGBA8: AR tiles there, GB tiles at `tmem_odd`).
  const u8* tmem = nullptr;
  u32 tmem_even = 0, tmem_odd = 0;
  // Sampler state.
  u8 wrap_s = 0, wrap_t = 0;  // 0 clamp, 1 repeat, 2 mirror
  u8 mag_linear = 0, min_filter = 0;  // min: 0 near, 1 linear, 2-5 with mip modes
  float lod_bias = 0, min_lod = 0, max_lod = 0;
};

struct DrawCall {
  PipelineConfig cfg;
  DrawUniforms u;
  std::array<TextureDesc, 8> tex;
  // Scissor rectangle in EFB pixels (x0, y0 inclusive, x1, y1 exclusive),
  // already clamped to the EFB.
  u32 scissor[4] = {0, 0, kEfbWidth, kEfbHeight};
  // The primitive's vertices, each once, as the transform unit produced them,
  // and the triangle list (or line list, point list, by cfg.topology) as
  // indices into them (E3c: strips and quads no longer repeat vertices).
  std::vector<OutVertex> verts;
  std::vector<u32> index;
  // Pixel state identity (E3c): two draws with the same nonzero `state` have the
  // same cfg (topology aside), u, tex and scissor, because no register they come
  // from was written in between. A backend may skip its lookups then. 0: unknown.
  std::uint64_t state = 0;
  // Geometry identity (V3, in-between frames; 0 unless the processor is asked for it:
  // GxProcessor::set_identify): a hash of what stays the same when the game draws
  // "the same object" again in the next frame: the primitive and its vertex format
  // (VCD, VAT), the vertex count, the array bases of the indexed attributes and the
  // first vertex's bytes as the stream holds them (its indices, or its direct data).
  // Matrices are not in it, so an object that moves keeps its identity.
  std::uint64_t ident = 0;

  // The triangle (line, point) list with the vertices repeated, as in E1.
  std::vector<OutVertex> expanded() const {
    std::vector<OutVertex> out;
    out.reserve(index.size());
    for (u32 i : index) out.push_back(verts[i]);
    return out;
  }
};

// An EFB copy (BP 0x52): to the external frame buffer, or into a texture.
struct EfbCopy {
  u32 src_x = 0, src_y = 0, width = 0, height = 0;  // EFB rectangle
  u32 dest = 0;            // physical address
  u32 dest_stride = 0;     // BP 0x4D (32-byte units)
  bool to_xfb = false;
  u8 tex_format = 0;       // texture copies: GX copy format (0-15)
  bool intensity = false;  // texture copies: intensity (YUV) formats
  bool half_scale = false; // box-filter to half size
  bool clamp_top = false, clamp_bottom = false;
  float y_scale = 1.0f;    // XFB copies: vertical scale
  u32 gamma = 0;
  // Clear after the copy.
  bool clear = false;
  u8 clear_rgba[4] = {};
  u32 clear_z = 0;         // 24-bit
  bool clear_color = false, clear_alpha = false, clear_depth = false;
  u8 pixel_format = 0;
  u32 copy_value = 0;      // raw BP 0x52
};

class Backend {
 public:
  virtual ~Backend() = default;
  virtual void draw(const DrawCall& d) = 0;
  virtual void efb_copy(const EfbCopy& c) = 0;
  // The texture cache was invalidated (BP 0x66): textures in memory may have changed.
  virtual void invalidate_textures() {}
};

}  // namespace mvp::gfx
