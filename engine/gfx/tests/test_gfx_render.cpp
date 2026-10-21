// End to end on the GPU, data-free: synthetic GX command streams through the
// state machine into the WebGPU backend, read back from the XFB. Covers the
// EFB clear, vertex colors through TEV, textures (decoded from synthetic
// memory), blending, EFB copies to textures sampled by a later draw, and that
// a wide range of generated TEV shaders compile. Exits 77 without a GPU.
#include <doctest/doctest.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>

#include "common/bytes.h"
#include "gfx/gx_processor.h"
#include "gfx/png.h"
#include "gfx/tev_wgsl.h"
#include "gfx/wgpu/gpu.h"
#include "gfx/wgpu/renderer.h"
#include "tests/gx_stream.h"

using namespace mvp;
using namespace mvp::gfx;
using namespace mvp::gfx::test;

namespace {

Gpu& gpu() {
  static std::unique_ptr<Gpu> g;
  if (!g) {
    std::string err;
    g = Gpu::create(&err);
    if (!g) {
      std::printf("skipped: %s\n", err.c_str());
      std::exit(77);
    }
  }
  return *g;
}

// TEV environment words.
constexpr u32 kCPrev = 0, kTexC = 8, kRasC = 10, kKonstC = 14, kZeroC = 15;
constexpr u32 kTexA = 4, kRasA = 5, kKonstA = 6, kZeroA = 7;
u32 color_env(u32 a, u32 b, u32 c, u32 d) { return d | c << 4 | b << 8 | a << 12 | 1u << 19; }
u32 alpha_env(u32 a, u32 b, u32 c, u32 d) { return d << 4 | c << 7 | b << 10 | a << 13 | 1u << 19; }

struct Scene {
  std::vector<std::uint8_t> mem = std::vector<std::uint8_t>(0x100000, 0);
  Renderer r;
  GxProcessor p{&r};
  explicit Scene(u32 scale = 1) : r(gpu(), mem.data(), static_cast<u32>(mem.size()), scale) {
    p.set_memory(mem.data(), static_cast<u32>(mem.size()));
  }
};

// Common state: 2D projection, one color channel from the vertex, one TEV
// stage passing the rasterized color, opaque writes, no depth test.
Stream base() {
  Stream s = setup_2d();
  s.xf(xf::kNumColors, {1});
  s.xf(xf::kColor0Cntrl, {1});
  s.xf(xf::kColor0Cntrl + 2, {1});
  s.bp(bp::kGenMode, 1u << 4);
  s.bp(bp::kTref, 0u << 7);  // stage 0: color0, no texture
  s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kRasC));
  s.bp(bp::kTevAlphaEnv, alpha_env(kZeroA, kZeroA, kZeroA, kRasA));
  s.bp(bp::kCMode0, 1u << 3 | 1u << 4);
  s.bp(bp::kZMode, 0);
  s.bp(bp::kZCompare, 0);  // RGB8_Z24
  s.bp(bp::kAlphaCompare, 7u << 16 | 7u << 19);
  // Identity swap tables, as GXInit leaves them: (r, g) in even KSEL registers, (b, a) in odd ones.
  for (int k = 0; k < 8; ++k) s.bp(static_cast<u8>(bp::kKsel + k), (k & 1) ? (2u | 3u << 2) : (0u | 1u << 2));
  return s;
}

// Copy the 640 x 480 EFB to the XFB at 0x80000, clearing to `clear` (RGBA).
void copy_xfb(Stream& s, u32 clear_rgba, bool do_clear = true) {
  s.bp(bp::kEfbTL, 0);
  s.bp(bp::kEfbWH, 639u | 479u << 10);
  s.bp(bp::kXfbAddr, 0x80000 >> 5);
  s.bp(bp::kClearAR, (clear_rgba >> 24) | (clear_rgba & 0xFF) << 8);
  s.bp(bp::kClearGB, ((clear_rgba >> 8) & 0xFF) | ((clear_rgba >> 16) & 0xFF) << 8);
  s.bp(bp::kClearZ, 0xFFFFFF);
  s.bp(bp::kTriggerCopy, 1u << 14 | (do_clear ? 1u << 11 : 0u));
}

// A quad from (x0, y0) to (x1, y1): position xy f32, color rgba8, tex0 st f32.
void quad(Stream& s, float x0, float y0, float x1, float y1, u32 rgba, float s0 = 0, float t0 = 0, float s1 = 1,
          float t1 = 1) {
  s.cp(cp::kVcdLo, 1u << 9 | 1u << 13);
  s.cp(cp::kVcdHi, 1u);
  s.cp(cp::kVatA, 0u | 4u << 1 | 1u << 13 | 5u << 14 | 1u << 21 | 4u << 22);
  std::vector<std::uint8_t> v;
  auto vert = [&](float x, float y, float ss, float tt) {
    f32(v, x);
    f32(v, y);
    Stream::put32(v, rgba);
    f32(v, ss);
    f32(v, tt);
  };
  vert(x0, y0, s0, t0);
  vert(x1, y0, s1, t0);
  vert(x1, y1, s1, t1);
  vert(x0, y1, s0, t1);
  s.prim(cmd::kQuads, 4, v);
}

const std::uint8_t* px(const std::vector<std::uint8_t>& img, u32 w, u32 x, u32 y) { return &img[(y * w + x) * 4]; }

bool near(const std::uint8_t* p, int r, int g, int b, int tol = 2) {
  return std::abs(p[0] - r) <= tol && std::abs(p[1] - g) <= tol && std::abs(p[2] - b) <= tol;
}

}  // namespace

TEST_CASE("clear, vertex-colored quad, XFB copy") {
  Scene sc;
  Stream s = base();
  copy_xfb(s, 0x0000FFFF);  // clear the EFB to blue (copying the initial black)
  quad(s, 100, 50, 300, 250, 0xFF0000FF);
  copy_xfb(s, 0x000000FF, false);
  s.run(sc.p);
  const Renderer::Frame* f = sc.r.xfb(0x80000);
  REQUIRE(f != nullptr);
  CHECK(f->width == 640);
  CHECK(f->height == 480);
  auto img = sc.r.read_frame(*f);
  if (const char* dbg = std::getenv("MVP_GFX_TEST_PNG")) write_png_rgba(dbg, 640, 480, img.data());
  {
    const std::uint8_t* q = px(img, 640, 200, 150);
    INFO("pixel (200,150) = " << int(q[0]) << "," << int(q[1]) << "," << int(q[2]) << "," << int(q[3]));
    CHECK(near(q, 255, 0, 0));
  }
  CHECK(near(px(img, 640, 101, 51), 255, 0, 0));
  CHECK(near(px(img, 640, 98, 150), 0, 0, 255));
  CHECK(near(px(img, 640, 200, 252), 0, 0, 255));
  CHECK(near(px(img, 640, 639, 479), 0, 0, 255));
  CHECK(sc.r.stats().draws == 1);
  CHECK(gpu().errors() == 0);
}

TEST_CASE("textured quad from synthetic RGB565 memory, with konst modulation") {
  Scene sc;
  // An 8 x 8 RGB565 texture at 0x40000: left half red, right half green (4x4 tiles).
  for (u32 y = 0; y < 8; ++y)
    for (u32 x = 0; x < 8; ++x) {
      u32 tile = (y / 4) * 2 + x / 4, in = (y % 4) * 4 + x % 4;
      put_be16(&sc.mem[0x40000 + tile * 32 + in * 2], x < 4 ? 0xF800 : 0x07E0);
    }
  Stream s = base();
  s.xf(xf::kNumTexGens, {1});
  s.xf(xf::kTexGen, {5u << 7});  // regular, tex0, ST, AB11
  s.xff(60 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});  // GX_IDENTITY
  s.cp(cp::kMatIndexA, 60u << 6);
  s.bp(bp::kGenMode, 1u | 1u << 4);
  s.bp(bp::kTref, 0u | 0u << 3 | 1u << 6 | 0u << 7);  // map 0, coord 0, enabled
  // out = tex * konst (konst 0 = (255, 255, 128) via kcsel 12) ... a = 0, b = tex, c = konst, d = 0.
  s.bp(bp::kTevColorEnv, color_env(kZeroC, kTexC, kKonstC, kZeroC));
  s.bp(bp::kTevAlphaEnv, alpha_env(kZeroA, kZeroA, kZeroA, kTexA));
  s.bp(bp::kTevReg + 0, 1u << 23 | 255u | 255u << 12);  // konst 0 RA: r 255, a 255
  s.bp(bp::kTevReg + 1, 1u << 23 | 0u | 128u << 12);    // konst 0 BG: b 0, g 128
  s.bp(bp::kKsel, 12u << 4 | 1u << 2);  // stage 0 kcsel K0, swap table 0 (r, g) kept
  s.bp(bp::tex_reg(bp::kTexImage0, 0), 7u | 7u << 10 | 4u << 20);
  s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x40000 >> 5);
  s.bp(bp::tex_reg(bp::kTexMode0, 0), 0);  // clamp, nearest
  for (int k = 0; k < 8; ++k) {  // texcoord sizes as GX sets them (the texture's size)
    s.bp(static_cast<u8>(bp::kSuSSize + 2 * k), 7);
    s.bp(static_cast<u8>(bp::kSuTSize + 2 * k), 7);
  }
  copy_xfb(s, 0x000000FF);
  quad(s, 0, 0, 320, 240, 0xFFFFFFFF);
  copy_xfb(s, 0x000000FF, false);
  s.run(sc.p);
  const Renderer::Frame* f = sc.r.xfb(0x80000);
  REQUIRE(f != nullptr);
  auto img = sc.r.read_frame(*f);
  CHECK(near(px(img, 640, 40, 100), 255, 0, 0));    // red * (1, 0.5, 0)
  CHECK(near(px(img, 640, 280, 100), 0, 128, 0));   // green * 0.5
  CHECK(near(px(img, 640, 400, 100), 0, 0, 0));     // outside: black
  CHECK(sc.r.stats().textures_decoded == 1);
  CHECK(gpu().errors() == 0);
}

TEST_CASE("indexed draws: strips, merged runs and vertex layouts of different strides in one pass (E3c)") {
  Scene sc;
  // A 4 x 4 RGB565 texture at 0x40000, all blue.
  for (u32 k = 0; k < 16; ++k) put_be16(&sc.mem[0x40000 + k * 2], 0x001F);
  Stream s = base();
  s.xf(xf::kNumTexGens, {1});
  s.xf(xf::kTexGen, {5u << 7});
  s.xff(60 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
  s.cp(cp::kMatIndexA, 60u << 6);
  s.bp(bp::kGenMode, 1u | 1u << 4);
  s.bp(bp::tex_reg(bp::kTexImage0, 0), 3u | 3u << 10 | 4u << 20);
  s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x40000 >> 5);
  s.bp(bp::tex_reg(bp::kTexMode0, 0), 0);
  s.bp(bp::kSuSSize, 3);
  s.bp(bp::kSuTSize, 3);
  copy_xfb(s, 0x000000FF);
  // Rasterized color only: position + color0 (32-byte vertices).
  s.bp(bp::kTref, 0u << 7);
  s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kRasC));
  quad(s, 0, 0, 100, 100, 0xFF0000FF);
  quad(s, 0, 100, 100, 200, 0xFF0000FF);  // the same state: appended to the draw before
  {  // a strip of two triangles, green
    s.cp(cp::kVcdLo, 1u << 9 | 1u << 13);
    std::vector<std::uint8_t> v;
    auto vert = [&](float x, float y) {
      f32(v, x);
      f32(v, y);
      Stream::put32(v, 0x00FF00FF);
      f32(v, 0);
      f32(v, 0);
    };
    vert(100, 0);
    vert(200, 0);
    vert(100, 100);
    vert(200, 100);
    s.prim(cmd::kTriStrip, 4, v);
  }
  // Texture only: position + texcoord 0 (28-byte vertices).
  s.bp(bp::kTref, 0u | 0u << 3 | 1u << 6 | 7u << 7);
  s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kTexC));
  quad(s, 200, 0, 300, 100, 0xFFFFFFFF);
  // Back to the rasterized color: a new run must start at a multiple of 32 bytes.
  s.bp(bp::kTref, 0u << 7);
  s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kRasC));
  quad(s, 300, 0, 400, 100, 0xFFFF00FF);
  copy_xfb(s, 0x000000FF, false);
  s.run(sc.p);
  const Renderer::Frame* f = sc.r.xfb(0x80000);
  REQUIRE(f != nullptr);
  auto img = sc.r.read_frame(*f);
  CHECK(near(px(img, 640, 50, 50), 255, 0, 0));
  CHECK(near(px(img, 640, 50, 150), 255, 0, 0));
  CHECK(near(px(img, 640, 150, 50), 0, 255, 0));
  CHECK(near(px(img, 640, 120, 20), 0, 255, 0));   // both strip triangles
  CHECK(near(px(img, 640, 180, 80), 0, 255, 0));
  CHECK(near(px(img, 640, 250, 50), 0, 0, 255));
  CHECK(near(px(img, 640, 350, 50), 255, 255, 0));
  CHECK(near(px(img, 640, 450, 50), 0, 0, 0));
  const Renderer::Stats& st = sc.r.stats();
  CHECK(st.draws == 5);
  CHECK(st.merged == 2);        // the second red quad and the strip
  CHECK(st.encoded == 3);
  CHECK(st.same_state == 2);    // the second red quad and the strip: only CP writes in between
  // 12 vertices of 32 bytes, 8 bytes of padding to a multiple of 28, 4 of 28, 8 to a multiple of 32, 4 of 32.
  CHECK(st.vertex_bytes == 12 * 32 + 8 + 4 * 28 + 8 + 4 * 32);
  CHECK(st.index_bytes == (6 + 6 + 6 + 6 + 6) * 4);
  CHECK(gpu().errors() == 0);
}

TEST_CASE("indirect texturing: the coordinate comes from an indirect texture through a matrix") {
  Scene sc;
  // Map 0: the 8 x 8 red | green texture of the previous case.
  for (u32 y = 0; y < 8; ++y)
    for (u32 x = 0; x < 8; ++x) {
      u32 tile = (y / 4) * 2 + x / 4, in = (y % 4) * 4 + x % 4;
      put_be16(&sc.mem[0x40000 + tile * 32 + in * 2], x < 4 ? 0xF800 : 0x07E0);
    }
  // Map 1: a 4 x 4 IA8 indirect texture, every texel (alpha 6, intensity 1): (s, t, u) = (6, 1, 1).
  for (u32 k = 0; k < 16; ++k) {
    sc.mem[0x50000 + 2 * k] = 6;
    sc.mem[0x50000 + 2 * k + 1] = 1;
  }
  Stream s = base();
  s.xf(xf::kNumTexGens, {1});
  s.xf(xf::kTexGen, {5u << 7});
  s.xff(60 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
  s.cp(cp::kMatIndexA, 60u << 6);
  s.bp(bp::kGenMode, 1u | 1u << 4 | 1u << 16);  // one indirect stage
  s.bp(bp::kIref, 1u | 0u << 3);                 // indirect stage 0: map 1, coordinate 0
  s.bp(bp::kTref, 0u | 0u << 3 | 1u << 6);
  s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kTexC));
  // Matrix 0 = [[512, 0, 0], [0, 512, 0]] with exponent field 18: (M x crd) >> 3, then << 1.
  s.bp(bp::kIndMtx + 0, 512u | 2u << 22);
  s.bp(bp::kIndMtx + 1, 512u << 11);
  s.bp(bp::kIndMtx + 2, 1u << 22);
  // Stage 0: indirect stage 0, 8-bit, matrix 0, both regular coordinates wrapped to 0:
  // the coordinate is (6, 1) texels wherever the quad is.
  s.bp(bp::kIndCmd + 0, 1u << 9 | 6u << 13 | 6u << 16);
  s.bp(bp::tex_reg(bp::kTexImage0, 0), 7u | 7u << 10 | 4u << 20);
  s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x40000 >> 5);
  s.bp(bp::tex_reg(bp::kTexImage0, 1), 3u | 3u << 10 | 3u << 20);
  s.bp(bp::tex_reg(bp::kTexImage3, 1), 0x50000 >> 5);
  for (int k = 0; k < 8; ++k) {
    s.bp(static_cast<u8>(bp::kSuSSize + 2 * k), 7);
    s.bp(static_cast<u8>(bp::kSuTSize + 2 * k), 7);
  }
  copy_xfb(s, 0x000000FF);
  quad(s, 0, 0, 320, 240, 0xFFFFFFFF);
  copy_xfb(s, 0x000000FF, false);
  s.run(sc.p);
  auto img = sc.r.read_frame(*sc.r.xfb(0x80000));
  CHECK(near(px(img, 640, 40, 100), 0, 255, 0));   // the left half samples texel (6, 1) too
  CHECK(near(px(img, 640, 280, 200), 0, 255, 0));
  CHECK(sc.r.stats().indirect_draws == 1);
  CHECK(gpu().errors() == 0);
}

TEST_CASE("alpha blending and the alpha test") {
  Scene sc;
  Stream s = base();
  copy_xfb(s, 0x000000FF);
  // White at alpha 128 over black: src alpha / inverse src alpha.
  s.bp(bp::kCMode0, 1u | 1u << 3 | 1u << 4 | 5u << 5 | 4u << 8);
  quad(s, 0, 0, 100, 100, 0xFFFFFF80);
  // The alpha test drops this one (alpha 64 must be > 100).
  s.bp(bp::kCMode0, 1u << 3 | 1u << 4);
  s.bp(bp::kAlphaCompare, 100u | 4u << 16 | 7u << 19 | 0u << 22);
  quad(s, 200, 0, 300, 100, 0x00FF0040);
  copy_xfb(s, 0x000000FF, false);
  s.run(sc.p);
  auto img = sc.r.read_frame(*sc.r.xfb(0x80000));
  CHECK(near(px(img, 640, 50, 50), 128, 128, 128, 3));
  CHECK(near(px(img, 640, 250, 50), 0, 0, 0));
  CHECK(gpu().errors() == 0);
}

TEST_CASE("an EFB copy to a texture is sampled by a later draw") {
  Scene sc;
  Stream s = base();
  copy_xfb(s, 0x000000FF);
  // Draw a 64 x 64 yellow square at (0, 0), copy it (RGBA8) to 0x60000.
  quad(s, 0, 0, 64, 64, 0xFFFF00FF);
  s.bp(bp::kEfbTL, 0);
  s.bp(bp::kEfbWH, 63u | 63u << 10);
  s.bp(bp::kXfbAddr, 0x60000 >> 5);
  s.bp(bp::kTriggerCopy, ((6u & 7) << 1 | 6u >> 3) << 3);  // RGBA8 (the field holds the format rotated left by one)
  // Clear, then draw a quad sampling that texture at (300, 200)-(428, 328).
  copy_xfb(s, 0x000000FF);
  s.xf(xf::kNumTexGens, {1});
  s.xf(xf::kTexGen, {5u << 7});
  s.xff(60 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
  s.cp(cp::kMatIndexA, 60u << 6);
  s.bp(bp::kGenMode, 1u | 1u << 4);
  s.bp(bp::kTref, 1u << 6);
  s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kTexC));
  s.bp(bp::tex_reg(bp::kTexImage0, 0), 63u | 63u << 10 | 6u << 20);
  s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x60000 >> 5);
  for (int k = 0; k < 2; ++k) {
    s.bp(static_cast<u8>(bp::kSuSSize + 2 * k), 63);
    s.bp(static_cast<u8>(bp::kSuTSize + 2 * k), 63);
  }
  quad(s, 300, 200, 428, 328, 0xFFFFFFFF);
  copy_xfb(s, 0x000000FF, false);
  s.run(sc.p);
  auto img = sc.r.read_frame(*sc.r.xfb(0x80000));
  CHECK(near(px(img, 640, 360, 260), 255, 255, 0));
  CHECK(near(px(img, 640, 30, 30), 0, 0, 0));  // the original square was cleared
  CHECK(sc.r.stats().copy_textures_used == 1);
  CHECK(sc.r.stats().textures_decoded == 0);
  CHECK(gpu().errors() == 0);
}

TEST_CASE("a texture taller than the EFB copy at its address: the copy's rows, then memory (P2)") {
  Scene sc;
  // Memory at 0x60000: a 64 x 64 RGBA8 texture, every texel opaque red (4 x 4 tiles:
  // 16 AR pairs, then 16 GB pairs).
  for (u32 tile = 0; tile < 16 * 16; ++tile)
    for (u32 k = 0; k < 16; ++k) {
      sc.mem[0x60000 + tile * 64 + 2 * k] = 0xFF;      // A
      sc.mem[0x60000 + tile * 64 + 2 * k + 1] = 0xFF;  // R
    }
  Stream s = base();
  copy_xfb(s, 0x000000FF);
  // A 128 x 96 yellow rectangle, copied at half scale (64 x 48, RGBA8) to 0x60000,
  // as the game copies 512 x 448 to 256 x 224 and samples it as 256 x 256.
  quad(s, 0, 0, 128, 96, 0xFFFF00FF);
  s.bp(bp::kEfbTL, 0);
  s.bp(bp::kEfbWH, 127u | 95u << 10);
  s.bp(bp::kXfbAddr, 0x60000 >> 5);
  s.bp(bp::kTriggerCopy, ((6u & 7) << 1 | 6u >> 3) << 3 | 1u << 9);  // RGBA8, half scale
  // Clear, then sample it as a 64 x 64 texture over (300, 200)-(428, 328), two pixels per texel.
  copy_xfb(s, 0x000000FF);
  s.xf(xf::kNumTexGens, {1});
  s.xf(xf::kTexGen, {5u << 7});
  s.xff(60 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
  s.cp(cp::kMatIndexA, 60u << 6);
  s.bp(bp::kGenMode, 1u | 1u << 4);
  s.bp(bp::kTref, 1u << 6);
  s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kTexC));
  s.bp(bp::tex_reg(bp::kTexImage0, 0), 63u | 63u << 10 | 6u << 20);
  s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x60000 >> 5);
  for (int k = 0; k < 2; ++k) {
    s.bp(static_cast<u8>(bp::kSuSSize + 2 * k), 63);
    s.bp(static_cast<u8>(bp::kSuTSize + 2 * k), 63);
  }
  quad(s, 300, 200, 428, 328, 0xFFFFFFFF);
  copy_xfb(s, 0x000000FF, false);
  s.run(sc.p);
  auto img = sc.r.read_frame(*sc.r.xfb(0x80000));
  CHECK(near(px(img, 640, 360, 210), 255, 255, 0));  // rows 0-47: the copy
  CHECK(near(px(img, 640, 360, 290), 255, 255, 0));
  CHECK(near(px(img, 640, 360, 300), 255, 0, 0));    // rows 48-63: memory
  CHECK(near(px(img, 640, 360, 325), 255, 0, 0));
  CHECK(sc.r.stats().copy_tall == 1);
  CHECK(sc.r.stats().copy_size_misses == 0);
  CHECK(gpu().errors() == 0);
}

TEST_CASE("internal resolution: 2x and 3x frames and copies show the 1x picture in more pixels (Q1)") {
  for (u32 scale : {2u, 3u}) {
    CAPTURE(scale);
    Scene sc(scale);
    for (u32 tile = 0; tile < 16 * 16; ++tile)
      for (u32 k = 0; k < 16; ++k) {
        sc.mem[0x60000 + tile * 64 + 2 * k] = 0xFF;
        sc.mem[0x60000 + tile * 64 + 2 * k + 1] = 0xFF;
      }
    // The tall-copy scene above: an XFB copy, a half-scale copy to a texture
    // sampled taller than the copy (its rows, then memory), the quads' scissor.
    Stream s = base();
    copy_xfb(s, 0x000000FF);
    quad(s, 0, 0, 128, 96, 0xFFFF00FF);
    s.bp(bp::kEfbTL, 0);
    s.bp(bp::kEfbWH, 127u | 95u << 10);
    s.bp(bp::kXfbAddr, 0x60000 >> 5);
    s.bp(bp::kTriggerCopy, ((6u & 7) << 1 | 6u >> 3) << 3 | 1u << 9);
    copy_xfb(s, 0x000000FF);
    s.xf(xf::kNumTexGens, {1});
    s.xf(xf::kTexGen, {5u << 7});
    s.xff(60 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
    s.cp(cp::kMatIndexA, 60u << 6);
    s.bp(bp::kGenMode, 1u | 1u << 4);
    s.bp(bp::kTref, 1u << 6);
    s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kTexC));
    s.bp(bp::tex_reg(bp::kTexImage0, 0), 63u | 63u << 10 | 6u << 20);
    s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x60000 >> 5);
    for (int k = 0; k < 2; ++k) {
      s.bp(static_cast<u8>(bp::kSuSSize + 2 * k), 63);
      s.bp(static_cast<u8>(bp::kSuTSize + 2 * k), 63);
    }
    quad(s, 300, 200, 428, 328, 0xFFFFFFFF);
    copy_xfb(s, 0x000000FF, false);
    s.run(sc.p);
    const Renderer::Frame* f = sc.r.xfb(0x80000);
    REQUIRE(f != nullptr);
    CHECK(f->width == 640 * scale);
    CHECK(f->height == 480 * scale);
    auto img = sc.r.read_frame(*f);
    const u32 w = 640 * scale;
    CHECK(near(px(img, w, 360 * scale, 210 * scale), 255, 255, 0));
    CHECK(near(px(img, w, 360 * scale, 290 * scale), 255, 255, 0));
    CHECK(near(px(img, w, 360 * scale, 300 * scale), 255, 0, 0));
    CHECK(near(px(img, w, 360 * scale, 325 * scale), 255, 0, 0));
    CHECK(near(px(img, w, 299 * scale, 250 * scale), 0, 0, 0));        // just left of the quad
    CHECK(near(px(img, w, 300 * scale, 250 * scale), 255, 255, 0));    // its first column
    CHECK(near(px(img, w, 300 * scale + scale - 1, 250 * scale), 255, 255, 0));
    CHECK(sc.r.stats().copy_tall == 1);
    CHECK(sc.r.stats().copy_size_misses == 0);
  }
  // A change of scale applies at the next XFB copy that clears the EFB.
  Scene sc;
  Stream a = base();
  copy_xfb(a, 0x0000FFFF);
  quad(a, 100, 50, 300, 250, 0xFF0000FF);
  a.run(sc.p);
  sc.r.set_scale(2);
  CHECK(sc.r.scale() == 1);
  Stream b;
  copy_xfb(b, 0x0000FFFF);  // copies the red quad at 1x, then clears: 2x from here
  b.run(sc.p);
  CHECK(sc.r.scale() == 2);
  CHECK(sc.r.xfb(0x80000)->width == 640);
  Stream c = base();
  quad(c, 100, 50, 300, 250, 0xFF0000FF);
  copy_xfb(c, 0x000000FF, false);
  c.run(sc.p);
  const Renderer::Frame* f = sc.r.xfb(0x80000);
  CHECK(f->width == 1280);
  auto img = sc.r.read_frame(*f);
  CHECK(near(px(img, 1280, 400, 300), 255, 0, 0));
  CHECK(near(px(img, 1280, 196, 300), 0, 0, 255));
  CHECK(gpu().errors() == 0);
}

// Q8: a 2D quad whose edges lie between pixel centers (the pitching view's
// mini-map: a black backing quad from y = 55.44 under a scissor from row 55)
// covers whole console pixels at 2x and 3x, the ones it covers at 1x. Before,
// the top third of row 55 stayed out of the quad at 3x and showed what later
// draws put there: a thin line.
TEST_CASE("internal resolution: 2D edges between pixel centers cover whole console pixels (Q8)") {
  for (u32 scale : {1u, 2u, 3u}) {
    CAPTURE(scale);
    Scene sc(scale);
    Stream s = base();
    copy_xfb(s, 0x0000FFFF);  // clear to blue
    // At 1x: columns 101-199 (centers 101.5-199.5), rows 55-100 (centers 55.5-100.5).
    quad(s, 100.6f, 55.44f, 200.44f, 101.376f, 0x00FF00FF);
    copy_xfb(s, 0x000000FF, false);
    s.run(sc.p);
    const Renderer::Frame* f = sc.r.xfb(0x80000);
    REQUIRE(f != nullptr);
    auto img = sc.r.read_frame(*f);
    const u32 w = 640 * scale, last = scale - 1;
    auto at = [&](u32 x, u32 y) { return px(img, w, x, y); };
    CHECK(near(at(101 * scale, 55 * scale), 0, 255, 0));            // the first pixel's top-left corner
    CHECK(near(at(199 * scale + last, 100 * scale + last), 0, 255, 0));  // the last pixel's bottom-right corner
    CHECK(near(at(100 * scale + last, 60 * scale), 0, 0, 255));     // column 100: none of it
    CHECK(near(at(200 * scale, 60 * scale), 0, 0, 255));            // column 200: none of it
    CHECK(near(at(150 * scale, 54 * scale + last), 0, 0, 255));     // row 54: none of it
    CHECK(near(at(150 * scale, 101 * scale), 0, 0, 255));           // row 101: none of it
  }
  CHECK(gpu().errors() == 0);
}

// Q7: the game copies a block of the EFB to a texture and draws it back in every
// frame (its top-left 128 x 128 save and restore). A change of scale replaces
// every EFB-copy texture; the draws that sample the copy must see the new copy at
// once, not the texture it replaced (in the browser a bind group cached for the
// old texture could be found again for the new one, and the block stayed frozen).
TEST_CASE("an EFB copy redone at a new scale is the one sampled (Q7)") {
  Scene sc;
  // One frame: a 64 x 64 square of `rgba` at (0, 0), copied (RGBA8) to 0x60000,
  // then drawn from that copy over (300, 200)-(428, 328); the XFB copy ends it.
  auto frame = [&](u32 rgba) {
    Stream s = base();
    quad(s, 0, 0, 64, 64, rgba);
    s.bp(bp::kEfbTL, 0);
    s.bp(bp::kEfbWH, 63u | 63u << 10);
    s.bp(bp::kXfbAddr, 0x60000 >> 5);
    s.bp(bp::kTriggerCopy, ((6u & 7) << 1 | 6u >> 3) << 3 | 1u << 11);  // RGBA8, clear
    s.xf(xf::kNumTexGens, {1});
    s.xf(xf::kTexGen, {5u << 7});
    s.xff(60 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
    s.cp(cp::kMatIndexA, 60u << 6);
    s.bp(bp::kGenMode, 1u | 1u << 4);
    s.bp(bp::kTref, 1u << 6);
    s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kTexC));
    s.bp(bp::tex_reg(bp::kTexImage0, 0), 63u | 63u << 10 | 6u << 20);
    s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x60000 >> 5);
    for (int k = 0; k < 2; ++k) {
      s.bp(static_cast<u8>(bp::kSuSSize + 2 * k), 63);
      s.bp(static_cast<u8>(bp::kSuTSize + 2 * k), 63);
    }
    quad(s, 300, 200, 428, 328, 0xFFFFFFFF);
    copy_xfb(s, 0x000000FF);  // the end of the frame: a new scale starts after it
    s.run(sc.p);
  };
  struct Step {
    u32 scale, rgba;
    int r, g, b;
  };
  // Scale changes in both directions, and a frame at the same scale between them.
  const Step steps[] = {{1, 0xFFFF00FF, 255, 255, 0}, {2, 0x00FFFFFF, 0, 255, 255}, {2, 0xFF00FFFF, 255, 0, 255},
                        {3, 0x00FF00FF, 0, 255, 0},   {1, 0xFF0000FF, 255, 0, 0},   {2, 0x0000FFFF, 0, 0, 255}};
  u32 drawn_at = 1;
  for (const Step& st : steps) {
    CAPTURE(st.scale);
    CAPTURE(st.rgba);
    sc.r.set_scale(st.scale);
    frame(st.rgba);  // drawn at the scale that was current when it started
    const Renderer::Frame* f = sc.r.xfb(0x80000);
    REQUIRE(f != nullptr);
    REQUIRE(f->width == 640 * drawn_at);
    auto img = sc.r.read_frame(*f);
    CHECK(near(px(img, f->width, 360 * drawn_at, 260 * drawn_at), st.r, st.g, st.b));
    drawn_at = sc.r.scale();  // the next frame's
  }
  CHECK(sc.r.stats().copy_textures_used == 6);
  CHECK(gpu().errors() == 0);
}

// ---- V3: in-between frames ----
//
// Frames end with an XFB copy; scan_field() stands for the VI field between them.
// The geometry keeps its identity when only its matrix changes (the vertex bytes
// stay the same), as the game's models do.
namespace {

struct InterpScene : Scene {
  explicit InterpScene(u32 scale = 1, bool on = true) : Scene(scale) {
    p.set_identify(on);
    r.set_interp(on);
    // The scan-out mix (P1) takes a pixel that is background, then covered, then
    // background again over three fields for a 30 Hz alternation: these frames
    // jump that far. Its own case is below.
    r.scan_mix = false;
    Stream s = base();
    copy_xfb(s, 0x000000FF);  // the first frame starts here
    s.run(p);
    r.scan_field(0x80000);
  }
  // A frame: position matrix 0 translated by (tx, 0), then `body`, then the XFB
  // copy (clearing to black) and `fields` VI fields. Returns the frame's serial.
  template <class F>
  std::uint64_t frame(float tx, F&& body, int fields = 1) {
    Stream s = base();
    s.xff(0x000, {1, 0, 0, tx, 0, 1, 0, 0, 0, 0, 1, 0});
    body(s);
    copy_xfb(s, 0x000000FF);
    s.run(p);
    const std::uint64_t serial = r.xfb(0x80000)->serial;
    for (int k = 0; k < fields; ++k) r.scan_field(0x80000);
    return serial;
  }
};

void red_quad(Stream& s) { quad(s, 100, 100, 200, 200, 0xFF0000FF); }

// A perspective projection (w = -z) and position matrix 0 at depth z = -2: one
// view-space unit in x is 160 pixels.
void persp(Stream& s, float tx) {
  s.xff(0x000, {1, 0, 0, tx, 0, 1, 0, 0, 0, 0, 1, -2});
  s.xff(0x1020, {1, 0, 1, 0, 0, 0});
  s.xf(0x1026, {0});
  s.xff(0x101A, {320.0f, -240.0f, 16777215.0f, 342.0f + 320, 342.0f + 240, 8388607.5f});
}
void persp_quad(Stream& s) { quad(s, -0.5f, -0.5f, 0.5f, 0.5f, 0x00FF00FF); }  // window x 240-400

}  // namespace

TEST_CASE("in-between frames: a quad that moves between two frames is drawn halfway (V3)") {
  InterpScene sc;
  const std::uint64_t a = sc.frame(0, red_quad);    // x 100-200
  const std::uint64_t b = sc.frame(100, red_quad);  // x 200-300
  const Renderer::Frame* ib = sc.r.between(a, b);
  REQUIRE(ib != nullptr);
  CHECK(sc.r.between(b, a) == nullptr);
  CHECK(ib->width == 640);
  CHECK(ib->height == 480);
  auto img = sc.r.read_frame(*ib);
  CHECK(near(px(img, 640, 145, 150), 0, 0, 0));    // halfway: x 150-250
  CHECK(near(px(img, 640, 155, 150), 255, 0, 0));
  CHECK(near(px(img, 640, 245, 150), 255, 0, 0));
  CHECK(near(px(img, 640, 255, 150), 0, 0, 0));
  auto live = sc.r.read_frame(*sc.r.xfb(0x80000));  // the frame itself, as drawn
  CHECK(near(px(live, 640, 195, 150), 0, 0, 0));
  CHECK(near(px(live, 640, 205, 150), 255, 0, 0));
  CHECK(near(px(live, 640, 295, 150), 255, 0, 0));
  const Renderer::InterpStats& st = sc.r.interp_stats();
  CHECK(st.made == 1);
  CHECK(st.last_match == doctest::Approx(1.0));
  // A 3D quad, perspective: halfway on screen too.
  const std::uint64_t c = sc.frame(0, [](Stream& s) { persp(s, 0); persp_quad(s); });
  const std::uint64_t d = sc.frame(0, [](Stream& s) { persp(s, 0.25f); persp_quad(s); });  // 40 pixels right
  ib = sc.r.between(c, d);
  REQUIRE(ib != nullptr);
  img = sc.r.read_frame(*ib);
  CHECK(near(px(img, 640, 255, 240), 0, 0, 0));    // halfway: x 260-420
  CHECK(near(px(img, 640, 265, 240), 0, 255, 0));
  CHECK(near(px(img, 640, 415, 240), 0, 255, 0));
  CHECK(near(px(img, 640, 425, 240), 0, 0, 0));
  CHECK(gpu().errors() == 0);
}

TEST_CASE("in-between frames: the same object drawn in another order pairs with itself (V3)") {
  InterpScene sc;
  // Two copies of one quad (one identity), at x 100 and x 400; the second frame draws
  // them the other way round, each 20 pixels further right.
  auto two = [](float left, float right, bool swap) {
    return [=](Stream& s) {
      const float xs[2] = {swap ? right : left, swap ? left : right};
      for (float x : xs) {
        s.xff(0x000, {1, 0, 0, x, 0, 1, 0, 0, 0, 0, 1, 0});
        quad(s, 0, 100, 60, 160, 0xFF0000FF);
      }
    };
  };
  const std::uint64_t a = sc.frame(0, two(100, 400, false));
  const std::uint64_t b = sc.frame(0, two(120, 420, true));
  const Renderer::Frame* ib = sc.r.between(a, b);
  REQUIRE(ib != nullptr);
  CHECK(sc.r.interp_stats().reordered == 2);
  auto img = sc.r.read_frame(*ib);
  CHECK(near(px(img, 640, 105, 130), 0, 0, 0));    // halfway: x 110-170 and 410-470
  CHECK(near(px(img, 640, 115, 130), 255, 0, 0));
  CHECK(near(px(img, 640, 165, 130), 255, 0, 0));
  CHECK(near(px(img, 640, 175, 130), 0, 0, 0));
  CHECK(near(px(img, 640, 405, 130), 0, 0, 0));
  CHECK(near(px(img, 640, 415, 130), 255, 0, 0));
  CHECK(near(px(img, 640, 250, 130), 0, 0, 0));    // nothing between them
  // Many copies that all move together keep their order (a crowd in a camera pan).
  auto row = [](float dx) {
    return [=](Stream& s) {
      for (int k = 0; k < 8; ++k) {
        s.xff(0x000, {1, 0, 0, 40.0f * k + dx, 0, 1, 0, 0, 0, 0, 1, 0});
        quad(s, 0, 300, 20, 320, 0x00FF00FF);
      }
    };
  };
  const std::uint64_t c = sc.frame(0, row(0));
  const std::uint64_t d = sc.frame(0, row(36));  // nearer to the next one's old place than to its own
  ib = sc.r.between(c, d);
  REQUIRE(ib != nullptr);
  CHECK(sc.r.interp_stats().reordered == 2);
  img = sc.r.read_frame(*ib);
  CHECK(near(px(img, 640, 17, 310), 0, 0, 0));     // halfway: x 18-38, 58-78, ...
  CHECK(near(px(img, 640, 20, 310), 0, 255, 0));
  CHECK(near(px(img, 640, 60, 310), 0, 255, 0));
  CHECK(near(px(img, 640, 45, 310), 0, 0, 0));
  CHECK(gpu().errors() == 0);
}

TEST_CASE("in-between frames: none on a cut, after a gap, or for new geometry (V3)") {
  InterpScene sc;
  const Renderer::InterpStats& st = sc.r.interp_stats();
  std::uint64_t a = sc.frame(0, red_quad);
  // Other geometry (other vertex bytes): nothing matches.
  std::uint64_t cuts = st.cut_match;
  std::uint64_t b = sc.frame(0, [](Stream& s) { quad(s, 300, 300, 400, 400, 0xFF0000FF); });
  CHECK(sc.r.between(a, b) == nullptr);
  CHECK(st.cut_match == cuts + 1);
  CHECK(st.last_match == doctest::Approx(0.0));
  // The same quad moved too far for one object (half the screen in a frame).
  a = sc.frame(0, red_quad);
  cuts = st.cut_match;
  b = sc.frame(320, red_quad);
  CHECK(sc.r.between(a, b) == nullptr);
  CHECK(st.cut_match == cuts + 1);
  // Frames two fields apart (the game at 30 frames a second): no in-between.
  a = sc.frame(0, red_quad, 2);
  const std::uint64_t gaps = st.gap;
  b = sc.frame(10, red_quad);
  CHECK(sc.r.between(a, b) == nullptr);
  CHECK(st.gap == gaps + 1);
  // A 3D scene that moved too far on average (a camera cut).
  a = sc.frame(0, [](Stream& s) { persp(s, 0); persp_quad(s); });
  b = sc.frame(0, [](Stream& s) { persp(s, 0.5f); persp_quad(s); });  // 0.25 of the screen's width
  CHECK(sc.r.between(a, b) == nullptr);
  CHECK(st.cut_shift == 1);
  CHECK(st.last_shift == doctest::Approx(0.25).epsilon(0.01));
  // And back to normal.
  a = sc.frame(0, red_quad);
  b = sc.frame(10, red_quad);
  CHECK(sc.r.between(a, b) != nullptr);
  CHECK(gpu().errors() == 0);
}

TEST_CASE("in-between frames: 2D edges at 2x still cover whole console pixels (V3, Q8)") {
  InterpScene sc(2);
  auto q = [](Stream& s) { quad(s, 100.6f, 55.44f, 200.44f, 101.376f, 0xFF0000FF); };
  const std::uint64_t a = sc.frame(0, q);
  const std::uint64_t b = sc.frame(51, q);  // halfway: x 126.1-225.94, console columns 126-225
  const Renderer::Frame* ib = sc.r.between(a, b);
  REQUIRE(ib != nullptr);
  auto img = sc.r.read_frame(*ib);
  const u32 w = 1280;
  CHECK(near(px(img, w, 126 * 2, 80 * 2), 255, 0, 0));
  CHECK(near(px(img, w, 125 * 2 + 1, 80 * 2), 0, 0, 0));
  CHECK(near(px(img, w, 225 * 2 + 1, 80 * 2), 255, 0, 0));
  CHECK(near(px(img, w, 226 * 2, 80 * 2), 0, 0, 0));
  CHECK(near(px(img, w, 150 * 2, 55 * 2), 255, 0, 0));   // row 55, whole
  CHECK(near(px(img, w, 150 * 2, 54 * 2 + 1), 0, 0, 0));
  CHECK(gpu().errors() == 0);
}

namespace {

// A frame with EFB copies: a 64 x 64 square of `rgba` at (0, 0) copied (RGBA8, with
// a clear) to 0x60000, a quad at (300 + tx, 200) sampling that copy, then the square
// redrawn magenta and copied there again (nothing samples the second copy).
void copy_body(Stream& s, u32 rgba) {
  s.xff(0x000, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
  quad(s, 0, 0, 64, 64, rgba);
  s.bp(bp::kEfbTL, 0);
  s.bp(bp::kEfbWH, 63u | 63u << 10);
  s.bp(bp::kXfbAddr, 0x60000 >> 5);
  s.bp(bp::kTriggerCopy, ((6u & 7) << 1 | 6u >> 3) << 3 | 1u << 11);
}
template <class Scn>
std::uint64_t copy_frame(Scn& sc, float tx, u32 rgba) {
  return sc.frame(0, [&](Stream& s) {
    copy_body(s, rgba);
    s.xff(0x000, {1, 0, 0, tx, 0, 1, 0, 0, 0, 0, 1, 0});
    s.xf(xf::kNumTexGens, {1});
    s.xf(xf::kTexGen, {5u << 7});
    s.xff(60 * 4, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
    s.cp(cp::kMatIndexA, 60u << 6);
    s.bp(bp::kGenMode, 1u | 1u << 4);
    s.bp(bp::kTref, 1u << 6);
    s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kTexC));
    s.bp(bp::tex_reg(bp::kTexImage0, 0), 63u | 63u << 10 | 6u << 20);
    s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x60000 >> 5);
    for (int k = 0; k < 2; ++k) {
      s.bp(static_cast<u8>(bp::kSuSSize + 2 * k), 63);
      s.bp(static_cast<u8>(bp::kSuTSize + 2 * k), 63);
    }
    quad(s, 300, 200, 428, 328, 0xFFFFFFFF);
    // The rasterized color again, the square in magenta, copied over the first copy.
    s.xf(xf::kNumTexGens, {0});
    s.bp(bp::kGenMode, 1u << 4);
    s.bp(bp::kTref, 0u << 7);
    s.bp(bp::kTevColorEnv, color_env(kZeroC, kZeroC, kZeroC, kRasC));
    copy_body(s, 0xFF00FFFF);
  });
}

}  // namespace

TEST_CASE("in-between frames: EFB copies are redone privately, at their place in the frame (V3)") {
  InterpScene sc;
  const std::uint64_t a = copy_frame(sc, 0, 0x00FFFFFF);   // cyan
  const std::uint64_t b = copy_frame(sc, 40, 0xFFFF00FF);  // yellow, 40 pixels right
  auto live = sc.r.read_frame(*sc.r.xfb(0x80000));
  CHECK(near(px(live, 640, 400, 260), 255, 255, 0));  // the frame samples its first copy
  const Renderer::Frame* ib = sc.r.between(a, b);
  REQUIRE(ib != nullptr);
  auto img = sc.r.read_frame(*ib);
  // Halfway (x 320-448), with what the first copy held at that point of the frame: yellow,
  // not the live texture's magenta from the frame's last copy.
  CHECK(near(px(img, 640, 315, 260), 0, 0, 0));
  CHECK(near(px(img, 640, 325, 260), 255, 255, 0));
  CHECK(near(px(img, 640, 440, 260), 255, 255, 0));
  CHECK(near(px(img, 640, 455, 260), 0, 0, 0));
  CHECK(gpu().errors() == 0);
}

TEST_CASE("in-between frames: mixed like the scan-out where the game alternates pictures (V3, P1)") {
  InterpScene sc;
  sc.r.scan_mix = true;
  // A slowly moving quad, two still ones, and a white square drawn in every other frame
  // (a 30 Hz alternation, which the scan-out shows as a steady mix of both).
  auto body = [](bool blink) {
    return [blink](Stream& s) {
      red_quad(s);
      s.xff(0x000, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0});
      quad(s, 20, 300, 60, 340, 0x0000FFFF);
      quad(s, 80, 300, 120, 340, 0x0000FFFF);
      if (blink) quad(s, 400, 300, 440, 340, 0xFFFFFFFF);
    };
  };
  sc.frame(0, body(true));
  sc.frame(2, body(false));
  const std::uint64_t a = sc.frame(4, body(true));
  const std::uint64_t b = sc.frame(6, body(false));
  const Renderer::Frame* ib = sc.r.between(a, b);
  REQUIRE(ib != nullptr);
  auto img = sc.r.read_frame(*ib);
  auto shown = sc.r.read_frame(*sc.r.scanout());
  // The square is in neither the frame nor its in-between, but both show it mixed.
  CHECK(near(px(shown, 640, 420, 320), 128, 128, 128, 3));
  CHECK(near(px(img, 640, 420, 320), 128, 128, 128, 3));
  CHECK(near(px(img, 640, 150, 150), 255, 0, 0));
  CHECK(gpu().errors() == 0);
}

TEST_CASE("in-between frames: the frames themselves are the same with them on and off (V3)") {
  InterpScene on(2, true), off(2, false);
  const float steps[] = {0, 12, 30, 31, 80, 80, 20};
  for (int k = 0; k < 7; ++k) {
    CAPTURE(k);
    copy_frame(on, steps[k], k & 1 ? 0x00FFFFFF : 0xFFFF00FF);
    copy_frame(off, steps[k], k & 1 ? 0x00FFFFFF : 0xFFFF00FF);
    auto x = on.r.read_frame(*on.r.xfb(0x80000));
    auto y = off.r.read_frame(*off.r.xfb(0x80000));
    REQUIRE(x.size() == y.size());
    CHECK(std::memcmp(x.data(), y.data(), x.size()) == 0);
    auto ex = on.r.read_efb();
    auto ey = off.r.read_efb();
    CHECK(std::memcmp(ex.data(), ey.data(), ex.size()) == 0);
  }
  CHECK(on.r.interp_stats().made >= 5);
  CHECK(off.r.interp_stats().frames == 0);
  CHECK(on.r.stats().draws == off.r.stats().draws);
  CHECK(on.r.stats().encoded == off.r.stats().encoded);
  CHECK(gpu().errors() == 0);
}

TEST_CASE("generated TEV shaders compile for many configurations") {
  std::mt19937 rng(1234);
  int made = 0;
  for (int n = 0; n < 200; ++n) {
    PipelineConfig c{};
    std::memset(&c, 0, sizeof c);
    c.num_stages = static_cast<u8>(1 + rng() % 16);
    for (int i = 0; i < c.num_stages; ++i) {
      TevStage& st = c.stages[i];
      st.color_env = rng() & 0xFFFFFF;
      st.alpha_env = rng() & 0xFFFFFF;
      st.tex_enable = rng() & 1;
      st.texmap = static_cast<u8>(rng() % 8);
      st.texcoord = static_cast<u8>(rng() % 8);
      st.ras_chan = static_cast<u8>(rng() % 8);
      st.kcsel = static_cast<u8>(rng() % 32);
      st.kasel = static_cast<u8>(rng() % 32);
      st.ind_cmd = (rng() & 1) ? (rng() & 0x1FFFFF) : 0;
    }
    for (auto& t : c.swap)
      for (auto& k : t) k = static_cast<u8>(rng() % 4);
    c.num_ind = static_cast<u8>(rng() % 5);
    for (int k = 0; k < 4; ++k) {
      c.iref_map[k] = static_cast<u8>(rng() % 8);
      c.iref_coord[k] = static_cast<u8>(rng() % 8);
      c.ind_scale[k] = static_cast<u8>(rng() % 256);
    }
    c.alpha_comp0 = static_cast<u8>(rng() % 8);
    c.alpha_comp1 = static_cast<u8>(rng() % 8);
    c.alpha_logic = static_cast<u8>(rng() % 4);
    c.logic_enable = rng() & 1;
    c.logic_op = static_cast<u8>(rng() % 16);
    c.fog_type = static_cast<u8>(rng() % 8);
    c.fog_proj = static_cast<u8>(rng() & 1);
    std::string w = generate_wgsl(c);
    gpu().shader(w, "fuzz");
    ++made;
  }
  gpu().wait_idle();
  CHECK(made == 200);
  CHECK(gpu().errors() == 0);
}
