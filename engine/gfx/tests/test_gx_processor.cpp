// The GX state machine on synthetic command streams (no game data): vertex
// formats and arrays, the transform unit, lighting, texture coordinate
// generation, primitive assembly, the BP write mask and TEV registers, indexed
// XF loads, TLUT loads and EFB copies.
#include <doctest/doctest.h>

#include <cmath>
#include <cstring>

#include "common/bytes.h"
#include "gfx/gx_processor.h"
#include "gfx/texture_decode.h"
#include "tests/gx_stream.h"

using namespace mvp;
using namespace mvp::gfx;
using namespace mvp::gfx::test;

namespace {

constexpr float kEps = 1e-4f;

// EFB window coordinates of an output vertex (x right, y down, z 0..1).
void window(const OutVertex& v, float& x, float& y, float& z) {
  x = (v.pos[0] / v.pos[3] + 1.0f) * 320.0f;
  y = (1.0f - v.pos[1] / v.pos[3]) * 264.0f;
  z = v.pos[2] / v.pos[3];
}

// VCD: position direct, color0 direct; VAT 0: xyz f32, rgba8888.
Stream pos_color_format() {
  Stream s;
  s.cp(cp::kVcdLo, 1u << 9 | 1u << 13);
  s.cp(cp::kVcdHi, 0);
  s.cp(cp::kVatA, 1u /*xyz*/ | 4u << 1 /*f32*/ | 1u << 13 | 5u << 14 /*rgba8888*/);
  return s;
}

void vtx(std::vector<std::uint8_t>& v, float x, float y, float z, std::uint32_t rgba) {
  f32(v, x);
  f32(v, y);
  f32(v, z);
  Stream::put32(v, rgba);
}

}  // namespace

TEST_CASE("2D quad: transform, viewport and assembly") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s = setup_2d();
  Stream f = pos_color_format();
  s.cmds.insert(s.cmds.end(), f.cmds.begin(), f.cmds.end());
  // One color channel, material from the vertex, no lighting.
  s.xf(xf::kNumColors, {1});
  s.xf(xf::kColor0Cntrl, {1});
  s.xf(xf::kColor0Cntrl + 2, {1});
  std::vector<std::uint8_t> v;
  vtx(v, 0, 0, 0, 0xFF0000FF);
  vtx(v, 640, 0, 0, 0x00FF00FF);
  vtx(v, 640, 480, 0, 0x0000FFFF);
  vtx(v, 0, 480, -1, 0xFFFFFF80);
  s.prim(cmd::kQuads, 4, v);
  s.run(p);
  REQUIRE(cap.draws.size() == 1);
  const DrawCall& d = cap.draws[0];
  CHECK(p.vertex_size(0) == 16);
  REQUIRE(d.verts.size() == 6);  // two triangles
  CHECK(d.cfg.topology == static_cast<u8>(Topology::Triangles));
  float x, y, z;
  window(d.verts[0], x, y, z);
  CHECK(x == doctest::Approx(0).epsilon(kEps));
  CHECK(y == doctest::Approx(0).epsilon(kEps));
  CHECK(z == doctest::Approx(0).epsilon(kEps));
  window(d.verts[2], x, y, z);  // third vertex of the first triangle: (640, 480)
  CHECK(x == doctest::Approx(640));
  CHECK(y == doctest::Approx(480));
  window(d.verts[5], x, y, z);  // (0, 480, -1): far plane
  CHECK(x == doctest::Approx(0).epsilon(kEps));
  CHECK(y == doctest::Approx(480));
  CHECK(z == doctest::Approx(1.0f));
  // Quad split: (0, 1, 2), (0, 2, 3).
  CHECK(d.verts[3].color[0][0] == doctest::Approx(1.0f));
  CHECK(d.verts[4].color[0][2] == doctest::Approx(1.0f));
  CHECK(d.verts[5].color[0][3] == doctest::Approx(128.0f / 255.0f));
  CHECK(d.verts[1].color[0][1] == doctest::Approx(1.0f));
  CHECK(p.stats().triangles == 2);
}

TEST_CASE("strips, fans, lines and points") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s = setup_2d();
  s.cp(cp::kVcdLo, 1u << 9);
  s.cp(cp::kVatA, 0u /*xy*/ | 4u << 1);
  std::vector<std::uint8_t> v;
  for (int k = 0; k < 5; ++k) {
    f32(v, static_cast<float>(k));
    f32(v, 0);
  }
  s.prim(cmd::kTriStrip, 5, v);
  s.prim(cmd::kTriFan, 5, v);
  s.prim(cmd::kLineStrip, 5, v);
  s.prim(cmd::kPoints, 5, v);
  s.run(p);
  REQUIRE(cap.draws.size() == 4);
  auto xs = [](const DrawCall& d) {
    std::vector<int> out;
    for (const auto& o : d.verts) out.push_back(static_cast<int>(std::lround((o.pos[0] / o.pos[3] + 1) * 320)));
    return out;
  };
  // Strip: (0,1,2), (2,1,3), (2,3,4): every triangle keeps the first one's winding.
  CHECK(xs(cap.draws[0]) == std::vector<int>{0, 1, 2, 2, 1, 3, 2, 3, 4});
  CHECK(xs(cap.draws[1]) == std::vector<int>{0, 1, 2, 0, 2, 3, 0, 3, 4});
  CHECK(xs(cap.draws[2]) == std::vector<int>{0, 1, 1, 2, 2, 3, 3, 4});
  CHECK(cap.draws[2].cfg.topology == static_cast<u8>(Topology::Lines));
  CHECK(xs(cap.draws[3]) == std::vector<int>{0, 1, 2, 3, 4});
  CHECK(cap.draws[3].cfg.topology == static_cast<u8>(Topology::Points));
}

TEST_CASE("draws carry each vertex once, with indices, and a pixel-state identity (E3c)") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s = setup_2d();
  s.cp(cp::kVcdLo, 1u << 9);
  s.cp(cp::kVatA, 0u /*xy*/ | 4u << 1);
  std::vector<std::uint8_t> v;
  for (int k = 0; k < 5; ++k) {
    f32(v, static_cast<float>(k));
    f32(v, 0);
  }
  std::vector<std::uint8_t> quad(v.begin(), v.begin() + 32);
  s.prim(cmd::kTriStrip, 5, v);
  s.prim(cmd::kQuads, 4, quad);
  s.cp(cp::kMatIndexA, 0);              // CP and XF writes keep the pixel state
  s.xf(xf::kNumColors, {0});
  s.prim(cmd::kTriFan, 5, v);
  s.bp(bp::kTevReg, 0);                 // any BP write starts a new one
  s.prim(cmd::kTriFan, 5, v);
  s.prim(cmd::kTriFan, 5, v);
  s.run(p);
  REQUIRE(cap.raw.size() == 5);
  const DrawCall& strip = cap.raw[0];
  CHECK(strip.verts.size() == 5);
  CHECK(strip.index == std::vector<u32>{0, 1, 2, 2, 1, 3, 2, 3, 4});
  const DrawCall& q = cap.raw[1];
  CHECK(q.verts.size() == 4);
  CHECK(q.index == std::vector<u32>{0, 1, 2, 0, 2, 3});
  CHECK(q.expanded().size() == 6);
  CHECK(q.expanded()[4].pos[0] == q.verts[2].pos[0]);
  CHECK(cap.raw[0].state != 0);
  CHECK(cap.raw[1].state == cap.raw[0].state);
  CHECK(cap.raw[2].state == cap.raw[0].state);
  CHECK(cap.raw[3].state != cap.raw[2].state);
  CHECK(cap.raw[4].state == cap.raw[3].state);
  // The rebuilt state equals the reused one when nothing changed.
  CHECK(cap.raw[3].cfg == cap.raw[2].cfg);
  CHECK(std::memcmp(&cap.raw[3].u, &cap.raw[2].u, sizeof(DrawUniforms)) == 0);
  CHECK(p.stats().snapshots == 2);
}

TEST_CASE("geometry identity: the same across matrices, another for other arrays or vertices (V3)") {
  std::vector<std::uint8_t> mem(0x10000, 0);
  Capture cap;
  GxProcessor p(&cap);
  p.set_memory(mem.data(), static_cast<u32>(mem.size()));
  Stream s = setup_2d();
  s.cp(cp::kVcdLo, 2u << 9);  // position: 8-bit index
  s.cp(cp::kVatA, 0u | 4u << 1);
  s.cp(cp::kArrayBase + 0, 0x1000);
  s.cp(cp::kArrayStride + 0, 8);
  const std::vector<std::uint8_t> idx{0, 1, 2, 3}, idx2{4, 5, 6, 7};
  s.prim(cmd::kQuads, 4, idx);                              // 0
  s.xff(0x000, {1, 0, 0, 50, 0, 1, 0, 0, 0, 0, 1, 0});
  s.prim(cmd::kQuads, 4, idx);                              // 1: another matrix
  s.prim(cmd::kQuads, 4, idx2);                             // 2: other indices
  s.cp(cp::kArrayBase + 0, 0x2000);
  s.prim(cmd::kQuads, 4, idx);                              // 3: another array
  s.prim(cmd::kTriFan, 4, idx);                             // 4: another primitive
  s.run(p);
  REQUIRE(cap.raw.size() == 5);
  CHECK(cap.raw[0].ident == 0);  // off by default
  p.set_identify(true);
  cap.raw.clear();
  s.run(p);
  REQUIRE(cap.raw.size() == 5);
  CHECK(cap.raw[0].ident != 0);
  CHECK(cap.raw[1].ident == cap.raw[0].ident);
  CHECK(cap.raw[2].ident != cap.raw[0].ident);
  CHECK(cap.raw[3].ident != cap.raw[0].ident);
  CHECK(cap.raw[4].ident != cap.raw[3].ident);
}

TEST_CASE("indexed attributes from arrays, fixed-point formats, matrix indices") {
  std::vector<std::uint8_t> mem(0x10000, 0);
  // Positions at 0x1000: s16 xyz with 4 fractional bits, stride 6.
  auto put16 = [&](std::uint32_t a, std::int16_t x) { put_be16(&mem[a], static_cast<std::uint16_t>(x)); };
  put16(0x1000 + 6 * 2 + 0, 16 * 10);
  put16(0x1000 + 6 * 2 + 2, 16 * 20);
  put16(0x1000 + 6 * 2 + 4, -16 * 3);
  // Colors at 0x2000: RGB565, stride 2: index 300 = pure green.
  put_be16(&mem[0x2000 + 2 * 300], 0x07E0);
  // Texture coordinates at 0x3000: u8 st with 7 fractional bits.
  mem[0x3000 + 2 * 1] = 64;
  mem[0x3000 + 2 * 1 + 1] = 128;

  Capture cap;
  GxProcessor p(&cap);
  p.set_memory(mem.data(), static_cast<u32>(mem.size()));
  Stream s = setup_2d();
  // Matrix 3 (rows 12-23): translate x by 100.
  s.xff(12, {1, 0, 0, 100, 0, 1, 0, 0, 0, 0, 1, 0});
  s.cp(cp::kVcdLo, 1u /*pnmtx*/ | 2u << 9 /*pos idx8*/ | 3u << 13 /*col0 idx16*/);
  s.cp(cp::kVcdHi, 2u /*tex0 idx8*/);
  s.cp(cp::kVatA, 1u | 3u << 1 /*s16*/ | 4u << 4 /*shift 4*/ | 0u << 14 /*rgb565*/ | 1u << 21 /*st*/ |
                      0u << 22 /*u8*/ | 7u << 25 /*shift 7*/);
  s.cp(cp::kArrayBase + 0, 0x1000);
  s.cp(cp::kArrayStride + 0, 6);
  s.cp(cp::kArrayBase + 2, 0x80002000);  // a cached address: physical 0x2000
  s.cp(cp::kArrayStride + 2, 2);
  s.cp(cp::kArrayBase + 4, 0x3000);
  s.cp(cp::kArrayStride + 4, 2);
  s.run(p);
  CHECK(p.vertex_size(0) == 1 + 1 + 2 + 1);
  const std::uint8_t vtxbytes[] = {3, 2, 0x01, 0x2C, 1};
  InVertex in = p.load_vertex(0, vtxbytes);
  CHECK(in.pnmtx == 3);
  CHECK(in.pos[0] == doctest::Approx(10));
  CHECK(in.pos[1] == doctest::Approx(20));
  CHECK(in.pos[2] == doctest::Approx(-3));
  CHECK(in.color[0][0] == doctest::Approx(0));
  CHECK(in.color[0][1] == doctest::Approx(1));
  CHECK(in.color[0][3] == doctest::Approx(1));
  CHECK(in.tex[0][0] == doctest::Approx(0.5));
  CHECK(in.tex[0][1] == doctest::Approx(1.0));
  OutVertex o = p.transform(in);
  float x, y, z;
  window(o, x, y, z);
  CHECK(x == doctest::Approx(110));
  CHECK(y == doctest::Approx(20));
}

TEST_CASE("BP write mask, TEV registers, konst colors, swap tables and stage order") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s;
  s.bp(bp::kGenMode, 1u /*1 texgen*/ | 1u << 4 /*1 chan*/ | 1u << 10 /*2 stages*/ | 2u << 14);
  // Mask: only bits 0-7 of the next write land.
  s.bp(bp::kTevColorEnv, 0xABCDEF);
  s.bp(bp::kMask, 0x0000FF);
  s.bp(bp::kTevColorEnv, 0x123456);
  s.bp(bp::kTevColorEnv + 2, 0x00F00F);  // mask is used up: full write
  // REG1 = (-5, 300, 1023, -1024) as r, g, b, a; konst 2 = (1, 2, 3, 4).
  s.bp(bp::kTevReg + 4, (0x7FBu) | (0x400u << 12));  // RA: r = -5, a = -1024
  s.bp(bp::kTevReg + 5, (0x3FFu) | (300u << 12));    // BG: b = 1023, g = 300
  s.bp(bp::kTevReg + 4, 1u << 23 | 1u | 4u << 12);   // konst 2 RA
  s.bp(bp::kTevReg + 5, 1u << 23 | 3u | 2u << 12);   // konst 2 BG
  // Stage 0: map 2, coord 1, enabled, color1; stage 1: map 5, coord 0, disabled, zero.
  s.bp(bp::kTref, 2u | 1u << 3 | 1u << 6 | 1u << 7 | 5u << 12 | 0u << 15 | 0u << 18 | 7u << 19);
  // Swap table 1: (b, g, r, a); ksel 0: stage 0 kcsel 12, kasel 28; stage 1 kcsel 1, kasel 2.
  s.bp(bp::kKsel + 0, 12u << 4 | 28u << 9 | 1u << 14 | 2u << 19);
  s.bp(bp::kKsel + 2, 2u | 1u << 2);
  s.bp(bp::kKsel + 3, 0u | 3u << 2);
  s.bp(bp::kAlphaCompare, 10u | 200u << 8 | 4u << 16 | 1u << 19 | 1u << 22);
  s.run(p);
  CHECK(p.bp(bp::kTevColorEnv) == 0xABCD56);
  CHECK(p.bp(bp::kTevColorEnv + 2) == 0x00F00F);
  DrawCall d;
  p.snapshot(d);
  CHECK(d.cfg.num_stages == 2);
  CHECK(d.cfg.num_texgens == 1);
  CHECK(d.cfg.num_chans == 1);
  CHECK(d.cfg.cull == 2);
  CHECK(d.u.tev_regs[2][0] == -5);
  CHECK(d.u.tev_regs[2][1] == 300);
  CHECK(d.u.tev_regs[2][2] == 1023);
  CHECK(d.u.tev_regs[2][3] == -1024);
  CHECK(d.u.konst[2][0] == 1);
  CHECK(d.u.konst[2][1] == 2);
  CHECK(d.u.konst[2][2] == 3);
  CHECK(d.u.konst[2][3] == 4);
  CHECK(d.cfg.stages[0].texmap == 2);
  CHECK(d.cfg.stages[0].texcoord == 1);
  CHECK(d.cfg.stages[0].tex_enable == 1);
  CHECK(d.cfg.stages[0].ras_chan == 1);
  CHECK(d.cfg.stages[0].kcsel == 12);
  CHECK(d.cfg.stages[0].kasel == 28);
  CHECK(d.cfg.stages[1].tex_enable == 0);
  CHECK(d.cfg.stages[1].texmap == 0);  // normalized: unused
  CHECK(d.cfg.stages[1].ras_chan == 7);
  CHECK(d.cfg.stages[1].kcsel == 1);
  CHECK(d.cfg.stages[1].kasel == 2);
  CHECK(d.cfg.swap[1][0] == 2);
  CHECK(d.cfg.swap[1][1] == 1);
  CHECK(d.cfg.swap[1][2] == 0);
  CHECK(d.cfg.swap[1][3] == 3);
  CHECK(d.cfg.alpha_comp0 == 4);
  CHECK(d.cfg.alpha_comp1 == 1);
  CHECK(d.cfg.alpha_logic == 1);
  CHECK(d.u.alpha_ref[0] == 10);
  CHECK(d.u.alpha_ref[1] == 200);
  CHECK(d.tex[2].valid);
  CHECK(!d.tex[5].valid);
  // Equal states give equal configurations.
  DrawCall d2;
  p.snapshot(d2);
  CHECK(d.cfg == d2.cfg);
}

TEST_CASE("texture descriptors and TLUT loads") {
  std::vector<std::uint8_t> mem(0x40000, 0);
  for (int k = 0; k < 64; ++k) mem[0x20000 + k] = static_cast<std::uint8_t>(k + 1);
  Capture cap;
  GxProcessor p(&cap);
  p.set_memory(mem.data(), static_cast<u32>(mem.size()));
  Stream s;
  s.bp(bp::kTref, 5u | 1u << 6);  // stage 0 samples map 5
  s.bp(bp::tex_reg(bp::kTexImage0, 5), 63u | 31u << 10 | 9u << 20);  // 64 x 32, C8
  s.bp(bp::tex_reg(bp::kTexImage3, 5), 0x10000 >> 5);
  s.bp(bp::tex_reg(bp::kTexMode0, 5), 1u | 2u << 2 | 1u << 4 | 6u << 5 | 0xF0u << 9);  // repeat, mirror, linear, lin_mip_lin, bias -0.5
  s.bp(bp::tex_reg(bp::kTexMode1, 5), 0u | 48u << 8);  // max lod 3
  s.bp(bp::tex_reg(bp::kTexTlut, 5), 3u | 2u << 10);   // tmem offset 3 << 9, RGB5A3
  // Load 32 entries (64 bytes) from 0x20000 to TLUT offset 3 << 9.
  s.bp(bp::kLoadTlut0, 0x20000 >> 5);
  s.bp(bp::kLoadTlut1, 3u | 2u << 10);
  s.bp(bp::kTexInvalidate, 0);
  s.run(p);
  CHECK(cap.invalidations == 1);
  CHECK(p.stats().tlut_loads == 1);
  DrawCall d;
  p.snapshot(d);
  const TextureDesc& t = d.tex[5];
  REQUIRE(t.valid);
  CHECK(t.addr == 0x10000);
  CHECK(t.format == 9);
  CHECK(t.width == 64);
  CHECK(t.height == 32);
  CHECK(t.wrap_s == 1);
  CHECK(t.wrap_t == 2);
  CHECK(t.mag_linear == 1);
  CHECK(t.min_filter == 6);
  CHECK(t.lod_bias == doctest::Approx(-0.5));
  CHECK(t.max_lod == doctest::Approx(3));
  CHECK(t.levels == 4);
  CHECK(t.tlut_format == 2);
  CHECK(t.tlut_offset == 3u << 9);
  CHECK(t.tlut_entries == 256);
  REQUIRE(t.tlut != nullptr);
  CHECK(std::memcmp(t.tlut, &mem[0x20000], 64) == 0);
  CHECK(t.tlut[64] == 0);  // only 32 entries were loaded
}

TEST_CASE("TMEM preloads and preloaded textures (E3c)") {
  std::vector<std::uint8_t> mem(0x40000, 0);
  for (int k = 0; k < 0x800; ++k) mem[0x8000 + k] = static_cast<std::uint8_t>(k * 7 + 3);
  Capture cap;
  GxProcessor p(&cap);
  p.set_memory(mem.data(), static_cast<u32>(mem.size()));
  Stream s;
  // An I8 texture of 16 x 8 (two 8x4 blocks... 128 bytes) at TMEM 0x4000.
  s.bp(bp::kPreloadAddr, 0x8000 >> 5);
  s.bp(bp::kPreloadTmemEven, 0x4000 >> 5);
  s.bp(bp::kPreloadTmemOdd, 0x84000 >> 5);
  s.bp(bp::kPreloadMode, 4u);  // 4 lines of 32 bytes
  // RGBA8, 4 x 4 (one 64-byte tile): AR to the even bank, GB to the odd one.
  s.bp(bp::kPreloadAddr, 0x8400 >> 5);
  s.bp(bp::kPreloadTmemEven, 0x6000 >> 5);
  s.bp(bp::kPreloadTmemOdd, 0x86000 >> 5);
  s.bp(bp::kPreloadMode, 1u | 3u << 15);
  s.bp(bp::kTref, 0u | 1u << 6 | (1u | 1u << 6) << 12);  // stage 0 map 0, stage 1 map 1
  s.bp(bp::kGenMode, 1u << 10);                          // two stages
  s.bp(bp::tex_reg(bp::kTexImage0, 0), 15u | 7u << 10 | 1u << 20);  // 16 x 8, I8
  s.bp(bp::tex_reg(bp::kTexImage1, 0), (0x4000 >> 5) | 1u << 21);  // preloaded
  s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x20000 >> 5);                // (not where the texels are)
  s.bp(bp::tex_reg(bp::kTexImage0, 1), 3u | 3u << 10 | 6u << 20);    // 4 x 4, RGBA8
  s.bp(bp::tex_reg(bp::kTexImage1, 1), (0x6000 >> 5) | 1u << 21);
  s.bp(bp::tex_reg(bp::kTexImage2, 1), 0x86000 >> 5);
  s.run(p);
  CHECK(p.stats().preloads == 2);
  DrawCall d;
  p.snapshot(d);
  const TextureDesc& t = d.tex[0];
  REQUIRE(t.valid);
  REQUIRE(t.tmem != nullptr);
  CHECK(t.tmem_even == 0x4000);
  std::vector<std::uint8_t> got;
  REQUIRE(gather_tmem_texture(t.tmem, t.tmem_even, t.tmem_odd, t.format, t.width, t.height, 1, got));
  REQUIRE(got.size() == 128);
  CHECK(std::memcmp(got.data(), &mem[0x8000], 128) == 0);
  const TextureDesc& r = d.tex[1];
  REQUIRE(r.tmem != nullptr);
  CHECK(r.tmem_odd == 0x86000);
  REQUIRE(gather_tmem_texture(r.tmem, r.tmem_even, r.tmem_odd, r.format, r.width, r.height, 1, got));
  REQUIRE(got.size() == 64);
  CHECK(std::memcmp(got.data(), &mem[0x8400], 64) == 0);  // the tile put back together
  CHECK(std::memcmp(p.tmem() + 0x6000, &mem[0x8400], 32) == 0);
  CHECK(std::memcmp(p.tmem() + 0x86000, &mem[0x8420], 32) == 0);
  // Mip levels alternate banks: level 1 of an I8 texture starts in the odd bank.
  std::vector<std::uint8_t> tm(kTmemSize, 0);
  tm[0x100] = 1;
  tm[0x90000] = 2;
  REQUIRE(gather_tmem_texture(tm.data(), 0x100, 0x90000, kTexI8, 16, 8, 2, got));
  CHECK(got.size() == 128 + 32);
  CHECK(got[0] == 1);
  CHECK(got[128] == 2);
  CHECK(!gather_tmem_texture(tm.data(), kTmemSize - 32, 0, kTexI8, 16, 8, 1, got));
}

TEST_CASE("indexed XF loads read matrices from memory") {
  std::vector<std::uint8_t> mem(0x10000, 0);
  // Matrix data at 0x4000 + 2 * 48 (index 2, stride 48): 12 floats.
  for (int k = 0; k < 12; ++k) put_be32(&mem[0x4000 + 96 + 4 * k], std::bit_cast<std::uint32_t>(static_cast<float>(k + 1)));
  Capture cap;
  GxProcessor p(&cap);
  p.set_memory(mem.data(), static_cast<u32>(mem.size()));
  Stream s;
  s.cp(cp::kArrayBase + 12, 0x4000);
  s.cp(cp::kArrayStride + 12, 48);
  // LOAD_INDX_A: index 2, 12 words, to XF address 0x18 (matrix 6).
  std::vector<std::uint8_t> c{cmd::kLoadIndxA};
  Stream::put32(c, 2u << 16 | 11u << 12 | 0x18u);
  s.raw(c);
  s.run(p);
  for (int k = 0; k < 12; ++k) CHECK(p.xf_float(0x18 + static_cast<u32>(k)) == doctest::Approx(k + 1));
  CHECK(p.stats().xf_indexed == 1);
}

TEST_CASE("the transform setup is kept across primitives until XF or the scissor offset changes (P1)") {
  // Every primitive after a write must see it: position matrix words (also of the
  // matrix that overlaps them), normal matrices, lights and channel registers, the
  // scissor offset. Each result is checked against a fresh processor that sees the
  // same state before its first primitive.
  auto stream = [](int step) {
    Stream s = setup_2d();
    Stream f = pos_color_format();
    s.cmds.insert(s.cmds.end(), f.cmds.begin(), f.cmds.end());
    s.xff(xf::kNrmMtx, {1, 0, 0, 0, 1, 0, 0, 0, 1});
    s.xf(xf::kNumColors, {1});
    s.xf(xf::kAmbient0, {0x20202020});
    s.xf(xf::kMaterial0, {0xFF8000FF});
    s.xf(xf::kColor0Cntrl, {0u | 1u << 1 | 1u << 2 | 2u << 7});
    s.xf(xf::kColor0Cntrl + 2, {0});
    s.xf(xf::kLights + 3, {0xFFFFFFFF});
    s.xff(xf::kLights + 10, {0, 0, 1e6f});
    if (step >= 1) s.xff(0x003, {10});                   // matrix 0, row 0, translation
    if (step >= 2) s.xff(0x00B, {-7});                   // matrix 0 row 2 (also matrix 2's first word)
    if (step >= 3) s.xff(xf::kNrmMtx, {0, 0, 1, 0, 1, 0, -1, 0, 0});  // normal matrix 0
    if (step >= 4) s.xf(xf::kMaterial0, {0x00FF00FF});  // channel register
    if (step >= 5) s.xf(xf::kLights + 3, {0x80808080});  // light color
    if (step >= 6) s.bp(bp::kScissorOffset, 160u | 171u << 10);
    return s;
  };
  std::vector<std::uint8_t> v;
  vtx(v, 100, 50, 0, 0xFFFFFFFF);
  vtx(v, 200, 50, 0, 0xFFFFFFFF);
  vtx(v, 200, 90, 0, 0xFFFFFFFF);
  vtx(v, 100, 90, 0, 0xFFFFFFFF);
  // One processor: a primitive after each step's writes (the setup cached in between).
  Capture cached;
  GxProcessor p(&cached);
  Stream all = stream(0);
  all.prim(cmd::kQuads, 4, v);
  for (int step = 1; step <= 6; ++step) {
    Stream s = stream(step);
    // Only the writes after stream(step - 1)'s: the extra commands at the end.
    const std::size_t before = stream(step - 1).cmds.size();
    for (std::size_t k = before; k < s.cmds.size(); ++k) all.cmds.push_back(s.cmds[k]);
    all.prim(cmd::kQuads, 4, v);
  }
  all.run(p);
  REQUIRE(cached.raw.size() == 7);
  for (int step = 0; step <= 6; ++step) {
    Capture fresh;
    GxProcessor q(&fresh);
    Stream s = stream(step);
    s.prim(cmd::kQuads, 4, v);
    s.run(q);
    REQUIRE(fresh.raw.size() == 1);
    const DrawCall& a = cached.raw[static_cast<std::size_t>(step)];
    const DrawCall& b = fresh.raw[0];
    REQUIRE(a.verts.size() == b.verts.size());
    for (std::size_t k = 0; k < a.verts.size(); ++k)
      CHECK(std::memcmp(&a.verts[k], &b.verts[k], sizeof(OutVertex)) == 0);
  }
  // The steps did change the output (the test would pass vacuously otherwise).
  CHECK(std::memcmp(&cached.raw[0].verts[0], &cached.raw[1].verts[0], sizeof(OutVertex)) != 0);
  CHECK(std::memcmp(&cached.raw[3].verts[0], &cached.raw[4].verts[0], sizeof(OutVertex)) != 0);
  CHECK(std::memcmp(&cached.raw[5].verts[0], &cached.raw[6].verts[0], sizeof(OutVertex)) != 0);
}

TEST_CASE("lighting: a diffuse light and the ambient color") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s = setup_2d();
  // Normal matrix 0 = identity.
  s.xff(xf::kNrmMtx, {1, 0, 0, 0, 1, 0, 0, 0, 1});
  s.xf(xf::kNumColors, {1});
  s.xf(xf::kAmbient0, {0x20202020});
  s.xf(xf::kMaterial0, {0xFF8000FF});
  // Color0: material from register, lighting on, light 0, ambient from register,
  // diffuse clamp, no attenuation. Alpha0: material only.
  s.xf(xf::kColor0Cntrl, {0u | 1u << 1 | 1u << 2 | 2u << 7});
  s.xf(xf::kColor0Cntrl + 2, {0});
  // Light 0: white, far along +z.
  s.xf(xf::kLights + 3, {0xFFFFFFFF});
  s.xff(xf::kLights + 10, {0, 0, 1e6f});
  s.run(p);
  InVertex in;
  in.nrm[2] = 1.0f;  // facing the light
  OutVertex o = p.transform(in);
  CHECK(o.color[0][0] == doctest::Approx(1.0f));             // 1 * clamp(0.125 + 1)
  CHECK(o.color[0][1] == doctest::Approx(128.0f / 255.0f));
  CHECK(o.color[0][3] == doctest::Approx(1.0f));
  in.nrm[2] = -1.0f;  // facing away: ambient only
  o = p.transform(in);
  CHECK(o.color[0][0] == doctest::Approx(0x20 / 255.0f));
  CHECK(o.color[0][1] == doctest::Approx(128.0f / 255.0f * 0x20 / 255.0f));
}

TEST_CASE("texture coordinate generation: matrix, STQ and post-transform") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s;
  s.xf(xf::kNumTexGens, {2});
  // Texgen 0: tex0, AB11, ST, regular, matrix 30 (scale s by 2, offset t by 0.25).
  s.xf(xf::kTexGen, {0u << 1 | 0u << 2 | 0u << 4 | 5u << 7});
  s.xff(30 * 4, {2, 0, 0, 0, 0, 1, 0, 0.25f});
  // Texgen 1: position, ABC1, STQ, matrix 33 = identity rows; post matrix 1 scales by 0.5.
  s.xf(xf::kTexGen + 1, {1u << 1 | 1u << 2 | 0u << 7});
  s.xff(33 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
  s.xf(xf::kDualTex, {1});
  s.xf(xf::kPostInfo + 0, {61});  // identity post matrix (loaded below)
  s.xf(xf::kPostInfo + 1, {1});
  s.xff(xf::kPostMtx + 61 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
  s.xff(xf::kPostMtx + 1 * 4, {0.5f, 0, 0, 0, 0, 0.5f, 0, 0, 0, 0, 1, 0});
  s.cp(cp::kMatIndexA, 30u << 6 | 33u << 12);
  s.run(p);
  std::uint8_t none[1] = {};
  InVertex in = p.load_vertex(0, none);  // takes the matrix indices from MATINDEX_A
  CHECK(in.texmtx[0] == 30);
  CHECK(in.texmtx[1] == 33);
  in.tex[0][0] = 0.25f;
  in.tex[0][1] = 0.5f;
  in.pos[0] = 4;
  in.pos[1] = 6;
  in.pos[2] = 2;
  OutVertex o = p.transform(in);
  CHECK(o.tex[0][0] == doctest::Approx(0.5f));
  CHECK(o.tex[0][1] == doctest::Approx(0.75f));
  CHECK(o.tex[0][2] == doctest::Approx(1.0f));
  CHECK(o.tex[1][0] == doctest::Approx(2.0f));
  CHECK(o.tex[1][1] == doctest::Approx(3.0f));
  CHECK(o.tex[1][2] == doctest::Approx(2.0f));
}

TEST_CASE("EFB copy to the XFB and to a texture, with clear") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s;
  s.bp(bp::kEfbTL, 0u | 0u << 10);
  s.bp(bp::kEfbWH, 639u | 479u << 10);
  s.bp(bp::kXfbAddr, 0x00300000 >> 5);
  s.bp(bp::kClearAR, 0x40 | 0xFF << 8);
  s.bp(bp::kClearGB, 0x60 | 0x50 << 8);
  s.bp(bp::kClearZ, 0xFFFFFF);
  s.bp(bp::kCMode0, 1u << 3 | 1u << 4);
  s.bp(bp::kZMode, 1u | 7u << 1 | 1u << 4);
  s.bp(bp::kDispCopyYScale, 256);
  s.bp(bp::kTriggerCopy, 1u << 14 | 1u << 11 | 3u);  // XFB, clear, clamp both
  // A texture copy of 64 x 32 at (16, 8), format RGB565 (raw 4 -> 0x4 << 1), half scale.
  s.bp(bp::kEfbTL, 16u | 8u << 10);
  s.bp(bp::kEfbWH, 63u | 31u << 10);
  s.bp(bp::kXfbAddr, 0x00100000 >> 5);
  s.bp(bp::kTriggerCopy, (4u << 1) << 3 | 1u << 9);
  s.run(p);
  REQUIRE(cap.copies.size() == 2);
  const EfbCopy& a = cap.copies[0];
  CHECK(a.to_xfb);
  CHECK(a.clear);
  CHECK(a.clamp_top);
  CHECK(a.clamp_bottom);
  CHECK(a.width == 640);
  CHECK(a.height == 480);
  CHECK(a.dest == 0x00300000);
  CHECK(a.clear_rgba[0] == 0x40);
  CHECK(a.clear_rgba[1] == 0x50);
  CHECK(a.clear_rgba[2] == 0x60);
  CHECK(a.clear_rgba[3] == 0xFF);
  CHECK(a.clear_z == 0xFFFFFF);
  CHECK(a.clear_color);
  CHECK(a.clear_alpha);
  CHECK(a.clear_depth);
  const EfbCopy& b = cap.copies[1];
  CHECK(!b.to_xfb);
  CHECK(!b.clear);
  CHECK(b.src_x == 16);
  CHECK(b.src_y == 8);
  CHECK(b.width == 64);
  CHECK(b.height == 32);
  CHECK(b.dest == 0x00100000);
  CHECK(b.tex_format == 4);
  CHECK(b.half_scale);
  CHECK(p.stats().xfb_copies == 1);
}

TEST_CASE("scissor and its offset") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s;
  s.bp(bp::kScissorTL, (342u + 10) | (342u + 20) << 12);
  s.bp(bp::kScissorBR, (342u + 99) | (342u + 199) << 12);
  s.bp(bp::kScissorOffset, ((342u + 4) / 2) | ((342u + 2) / 2) << 10);
  s.run(p);
  DrawCall d;
  p.snapshot(d);
  CHECK(d.scissor[0] == 16);
  CHECK(d.scissor[1] == 8);
  CHECK(d.scissor[2] == 196);
  CHECK(d.scissor[3] == 98);
}

TEST_CASE("fog parameters") {
  Capture cap;
  GxProcessor p(&cap);
  Stream s;
  s.bp(bp::kFog0, 127u << 11 | 0x400u);           // A = 1.5
  s.bp(bp::kFog1, 0x123456);                       // B magnitude
  s.bp(bp::kFog2, 9);                              // B shift
  s.bp(bp::kFog3, 126u << 11 | 1u << 19 | 4u << 21 | 1u << 20);  // C = -0.5, orthographic, exp
  s.bp(bp::kFogColor, 0x102030);
  s.run(p);
  DrawCall d;
  p.snapshot(d);
  CHECK(d.cfg.fog_type == 4);
  CHECK(d.cfg.fog_proj == 1);
  CHECK(d.u.fog[0] == doctest::Approx(1.5));
  CHECK(d.u.fog[1] == doctest::Approx(0x123456));
  CHECK(d.u.fog[2] == doctest::Approx(9));
  CHECK(d.u.fog[3] == doctest::Approx(-0.5));
  CHECK(d.u.fog[4] == doctest::Approx(0x10));
  CHECK(d.u.fog[5] == doctest::Approx(0x20));
  CHECK(d.u.fog[6] == doctest::Approx(0x30));
}
