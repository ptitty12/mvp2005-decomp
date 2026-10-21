// mvp-gx-bench (P1, development): the GX processor's cost per primitive and
// per vertex on a synthetic stream shaped like the game's in-play meshes (no
// game data): skinned triangle strips of about seven indexed vertices (a matrix
// index, a texture matrix index, position, normal, color and texture
// coordinate indices: the VCD/VAT values the game uses), a lit color channel,
// two texture coordinate generators with post matrices, and a 3x4 matrix
// loaded through XF every two primitives (the game's CPU skinning).
//
//   mvp-gx-bench [--prims N] [--reps N] [--pack] [--no-light] [--no-texgen] [--no-skin] [--alpha-lit] [--ref] [--gp]
//
// It also prints a hash of every vertex and index it produced (P2): changes to
// the processor's arithmetic must keep it.
//
// --pack adds a backend that packs each draw's vertices and indices the way the
// WebGPU renderer does (without a GPU), to time that part too. Build it with
// the Emscripten preset and run it under Node to see the browser's costs.
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "common/bytes.h"
#include "gfx/gp_memory.h"
#include "gfx/gx_processor.h"
#include "gfx/tests/gx_stream.h"

using namespace mvp;
using namespace mvp::gfx;
using namespace mvp::gfx::test;

namespace {

struct NullBackend : Backend {
  std::uint64_t draws = 0, verts = 0;
  void draw(const DrawCall& d) override {
    ++draws;
    verts += d.verts.size();
  }
  void efb_copy(const EfbCopy&) override {}
};

// A hash of every draw's vertices and indices (P2: optimizations must keep it).
struct HashBackend : Backend {
  std::uint64_t h = 0xCBF29CE484222325ull;
  void fold(const void* p, std::size_t n) {
    const auto* b = static_cast<const std::uint8_t*>(p);
    for (std::size_t i = 0; i < n; ++i) h = (h ^ b[i]) * 0x100000001B3ull;
  }
  void draw(const DrawCall& d) override {
    fold(d.verts.data(), d.verts.size() * sizeof(OutVertex));
    fold(d.index.data(), d.index.size() * sizeof(std::uint32_t));
  }
  void efb_copy(const EfbCopy&) override {}
};

// The renderer's per-draw packing (position, color 0, texture coordinates 0-1), on the CPU only.
struct PackBackend : Backend {
  std::vector<std::uint8_t> v;
  std::vector<std::uint32_t> ix;
  std::uint64_t draws = 0;
  void draw(const DrawCall& d) override {
    ++draws;
    if (v.size() > (32u << 20)) {
      v.clear();
      ix.clear();
    }
    const std::uint32_t base = static_cast<std::uint32_t>(v.size() / 56);
    const std::size_t at = v.size();
    v.resize(at + d.verts.size() * 56);
    std::uint8_t* o = v.data() + at;
    for (const OutVertex& x : d.verts) {
      std::memcpy(o, x.pos, 16);
      std::memcpy(o + 16, x.color[0], 16);
      std::memcpy(o + 32, x.tex[0], 12);
      std::memcpy(o + 44, x.tex[1], 12);
      o += 56;
    }
    for (std::uint32_t i : d.index) ix.push_back(base + i);
  }
  void efb_copy(const EfbCopy&) override {}
};

}  // namespace

int main(int argc, char** argv) {
  int prims = 8000, reps = 20;
  bool pack = false, light = true, texgen = true, skin = true, alpha_lit = false, ref = false, gp = false, dark = false;
  for (int k = 1; k < argc; ++k) {
    std::string a = argv[k];
    if (a == "--prims" && k + 1 < argc) prims = std::atoi(argv[++k]);
    else if (a == "--reps" && k + 1 < argc) reps = std::atoi(argv[++k]);
    else if (a == "--pack") pack = true;
    else if (a == "--no-light") light = false;      // variants, to see what each part costs
    else if (a == "--no-texgen") texgen = false;
    else if (a == "--no-skin") skin = false;
    else if (a == "--alpha-lit") alpha_lit = true;  // alpha 0 lit like color 0 (GX_COLOR0A0)
    else if (a == "--ref") ref = true;              // V5: the reference vertex path (the one before V5)
    else if (a == "--dark") dark = true;            // V5: color 0 lit by no light (ambient times material: the game's usual setup)
    else if (a == "--gp") gp = true;                // V5: arrays read through a GpMemory holding copies of them (the render thread's case)
  }

  // Arrays: 256 positions (s16 xyz), normals (s8 xyz), colors (rgba8), texture coordinates (s16 st).
  std::vector<std::uint8_t> mem(0x100000, 0);
  const std::uint32_t kPos = 0x10000, kNrm = 0x20000, kCol = 0x30000, kTex = 0x40000;
  for (int i = 0; i < 256; ++i) {
    put_be16(&mem[kPos + 6 * i], static_cast<std::uint16_t>(i * 37 - 4000));
    put_be16(&mem[kPos + 6 * i + 2], static_cast<std::uint16_t>(i * 11 + 300));
    put_be16(&mem[kPos + 6 * i + 4], static_cast<std::uint16_t>(-i * 23));
    mem[kNrm + 6 * i] = static_cast<std::uint8_t>(i);
    mem[kNrm + 6 * i + 1] = static_cast<std::uint8_t>(64 - i / 4);
    mem[kNrm + 6 * i + 2] = 20;
    put_be32(&mem[kCol + 4 * i], 0x80604020u + static_cast<std::uint32_t>(i));
    put_be16(&mem[kTex + 4 * i], static_cast<std::uint16_t>(i * 64));
    put_be16(&mem[kTex + 4 * i + 2], static_cast<std::uint16_t>(i * 32));
  }

  Stream s = setup_2d();
  // A perspective projection.
  s.xff(xf::kProjection, {1.8f, 0.0f, 2.4f, 0.0f, -1.0002f, -2.0002f});
  s.xf(xf::kProjection + 6, {0});
  s.cp(cp::kVcdLo, 0x5405);
  s.cp(cp::kVcdHi, 0x2);
  s.cp(cp::kVatA, 0x54F76C87);
  s.cp(cp::kVatB, 0xC82412A7);
  s.cp(cp::kVatC, 0x04824120);
  s.cp(cp::kMatIndexA, 0x3C79EF00);
  s.cp(cp::kMatIndexB, 0x00F3CF3C);
  s.cp(cp::kArrayBase + 0, kPos);
  s.cp(cp::kArrayStride + 0, 6);
  s.cp(cp::kArrayBase + 1, kNrm);
  s.cp(cp::kArrayStride + 1, 6);
  s.cp(cp::kArrayBase + 2, kCol);
  s.cp(cp::kArrayStride + 2, 4);
  s.cp(cp::kArrayBase + 4, kTex);
  s.cp(cp::kArrayStride + 4, 4);
  // Matrices: position/texture 0-63, normal 0-31, post 0-63.
  for (int m = 0; m < 64; ++m) {
    const float t = static_cast<float>(m);
    s.xff(static_cast<std::uint32_t>(4 * m), {1, 0, 0, t, 0, 1, 0, -t, 0, 0, 1, -900 - t});
    s.xff(xf::kPostMtx + static_cast<std::uint32_t>(4 * m), {0.5f, 0, 0, 0.25f, 0, 0.5f, 0, 0.25f, 0, 0, 1, 0});
  }
  for (int m = 0; m < 32; ++m) s.xff(xf::kNrmMtx + static_cast<std::uint32_t>(3 * m), {1, 0, 0, 0, 1, 0, 0, 0, 1});
  // One color channel lit by two lights (diffuse, spot attenuation off), material from the vertex.
  s.xf(xf::kNumColors, {1});
  s.xf(xf::kAmbient0, {0x30303030});
  s.xf(xf::kMaterial0, {0xFFFFFFFF});
  s.xf(xf::kColor0Cntrl, {dark ? (1u | 1u << 1 | 2u << 7 | 2u << 9) : light ? (1u | 1u << 1 | 3u << 2 | 2u << 7 | 2u << 9) : 1u});
  s.xf(xf::kColor0Cntrl + 2, {light && alpha_lit ? (1u | 1u << 1 | 3u << 2 | 2u << 7 | 2u << 9) : 1u});
  for (int l = 0; l < 2; ++l) {
    const std::uint32_t b = xf::kLights + 16u * static_cast<std::uint32_t>(l);
    s.xf(b + 3, {0xC0C0C0FF});
    s.xff(b + 4, {1, 0, 0, 1, 0, 0});
    s.xff(b + 10, {100.0f * static_cast<float>(l + 1), 500, -200, 0, 0, -1});
  }
  s.xf(xf::kNumTexGens, {texgen ? 2u : 0u});
  s.xf(xf::kTexGen, {0x280});
  s.xf(xf::kTexGen + 1, {0x86});
  s.xf(xf::kDualTex, {1});
  s.xf(xf::kPostInfo, {0});
  s.xf(xf::kPostInfo + 1, {3});

  // The primitives: strips of 5-9 vertices; a skinning matrix every two.
  Stream body;
  std::uint32_t rnd = 12345;
  auto next = [&]() {
    rnd = rnd * 1664525u + 1013904223u;
    return rnd >> 8;
  };
  std::uint64_t vertices = 0;
  for (int p = 0; p < prims; ++p) {
    if (skin && p % 2 == 0) {
      const float w = static_cast<float>(next() % 100) / 100.0f;
      body.xff(static_cast<std::uint32_t>(4 * (p % 10)), {w, 0, 0, 1, 0, 1 - w, 0, 2, 0, 0, 1, -950});
    }
    const int n = 5 + static_cast<int>(next() % 5);
    std::vector<std::uint8_t> v;
    for (int k = 0; k < n; ++k) {
      v.push_back(static_cast<std::uint8_t>(3 * (p % 10)));  // position/normal matrix index
      v.push_back(0x1E);                                     // texture matrix 1 index
      v.push_back(static_cast<std::uint8_t>(next()));        // position
      v.push_back(static_cast<std::uint8_t>(next()));        // normal
      v.push_back(static_cast<std::uint8_t>(next()));        // color 0
      v.push_back(static_cast<std::uint8_t>(next()));        // texture coordinate 0
    }
    body.prim(cmd::kTriStrip, static_cast<std::uint16_t>(n), v);
    vertices += static_cast<std::uint64_t>(n);
  }

  NullBackend null_backend;
  PackBackend pack_backend;
  Backend* backend = pack ? static_cast<Backend*>(&pack_backend) : &null_backend;
  GxProcessor proc(backend);
  proc.set_memory(mem.data(), static_cast<std::uint32_t>(mem.size()));
  proc.set_reference_vertices(ref);
  GpMemory gpm;
  if (gp) {  // as the threaded engine's render thread: the arrays' entries are copies (Q6)
    gpm.attach(mem.data(), static_cast<std::uint32_t>(mem.size()));
    for (std::uint32_t at : {kPos, kNrm, kCol, kTex}) gpm.shadow(at, 0x600, mem.data() + at);
    proc.set_gp_memory(&gpm);
  }
  s.run(proc);
  body.run(proc);  // warm-up
  double best = 1e30;
  for (int r = 0; r < reps; ++r) {
    const auto t0 = std::chrono::steady_clock::now();
    body.run(proc);
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    if (ms < best) best = ms;
  }
  {
    HashBackend hb;
    GxProcessor hp(&hb);
    hp.set_memory(mem.data(), static_cast<std::uint32_t>(mem.size()));
    hp.set_reference_vertices(ref);
    s.run(hp);
    body.run(hp);
    std::printf("gx-bench output hash %016llx\n", static_cast<unsigned long long>(hb.h));
  }
  std::printf("gx-bench%s%s%s: %d primitives, %llu vertices: best %.3f ms (%.1f ns per primitive, %.1f ns per vertex)\n",
              ref ? " (reference path)" : "", gp ? " (GP memory)" : "", pack ? " (with packing)" : "", prims, static_cast<unsigned long long>(vertices), best,
              best * 1e6 / prims, best * 1e6 / static_cast<double>(vertices));
  return 0;
}
