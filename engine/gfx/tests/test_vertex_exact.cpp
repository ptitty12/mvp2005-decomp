// V5: the GX processor's fast vertex path (specialized loaders, the transform
// unit's per-primitive plan, the GP memory window) against the reference path
// (GxProcessor::load_vertex / transform, E1-P1's generic code), bit for bit, on
// random command streams: every vertex format the VCD/VAT can describe (direct
// and indexed, every component and color format, NBT with one and three
// indices, matrix indices), random XF state (matrices, lights, channels, texture
// coordinate generation, post matrices, projections), XF and CP writes between
// primitives, and memory read through a GpMemory with cache write-back
// snapshots and copies over it. No game data.
#include <doctest/doctest.h>

#include <bit>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "gfx/gp_memory.h"
#include "gfx/gx_processor.h"
#include "tests/gx_stream.h"

using namespace mvp;
using namespace mvp::gfx;
using namespace mvp::gfx::test;

namespace {

struct Rng {
  std::uint64_t s;
  explicit Rng(std::uint64_t seed) : s(seed * 0x9E3779B97F4A7C15ull + 1) {}
  std::uint32_t next() {
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return static_cast<std::uint32_t>(s >> 11);
  }
  std::uint32_t below(std::uint32_t n) { return n ? next() % n : 0; }
  bool chance(int percent) { return below(100) < static_cast<std::uint32_t>(percent); }
  float range(float lo, float hi) { return lo + (hi - lo) * static_cast<float>(next() & 0xFFFFFF) / 16777216.0f; }
  // A float for XF state: usually tame, sometimes zero, one, negative zero or a large value.
  float xf_float() {
    switch (below(16)) {
      case 0: return 0.0f;
      case 1: return 1.0f;
      case 2: return -0.0f;
      case 3: return range(-5000.0f, 5000.0f);
      default: return range(-2.0f, 2.0f);
    }
  }
};

constexpr std::uint32_t kMemSize = 1u << 20;

// Both processors see the same commands; the second takes the reference path.
struct Pair {
  Capture fast_cap, ref_cap;
  GxProcessor fast{&fast_cap}, ref{&ref_cap};
  void run(const std::vector<std::uint8_t>& c) {
    fast.command(c.data(), c.size());
    ref.command(c.data(), c.size());
  }
  void cp(std::uint8_t reg, std::uint32_t v) {
    Stream s;
    s.cp(reg, v);
    run(s.cmds[0]);
  }
  void xf(std::uint32_t addr, const std::vector<std::uint32_t>& w) {
    Stream s;
    s.xf(addr, w);
    run(s.cmds[0]);
  }
  void bp(std::uint8_t reg, std::uint32_t v) {
    Stream s;
    s.bp(reg, v);
    run(s.cmds[0]);
  }
};

std::uint32_t fbits(float f) { return std::bit_cast<std::uint32_t>(f); }

// An input vertex's 36 floats (pos, nrm, bin, tan, colors, texture coordinates) as one array.
const float* floats(const InVertex& v) {
  return reinterpret_cast<const float*>(reinterpret_cast<const unsigned char*>(&v) + offsetof(InVertex, pos));
}

// A component format's value bytes (big-endian), tame for f32.
void put_comp(std::vector<std::uint8_t>& v, Rng& r, std::uint32_t fmt) {
  switch (fmt) {
    case kU8: case kS8: v.push_back(static_cast<std::uint8_t>(r.next())); break;
    case kU16: case kS16: u16be(v, static_cast<std::uint16_t>(r.next())); break;
    default: f32(v, r.chance(5) ? std::bit_cast<float>(r.next()) : r.range(-300.0f, 300.0f)); break;
  }
}

// Fills `n` bytes of memory at `at` as `count` elements of `stride` bytes, each
// `comps` components of format `fmt` (colors: raw bytes).
void fill_array(std::vector<std::uint8_t>& mem, Rng& r, std::uint32_t at, std::uint32_t stride, std::uint32_t fmt,
                std::uint32_t comps, bool color) {
  for (std::uint32_t e = 0; e < 256 && at + e * stride + 64 < mem.size(); ++e) {
    std::vector<std::uint8_t> v;
    for (std::uint32_t k = 0; k < comps; ++k) {
      if (color) v.push_back(static_cast<std::uint8_t>(r.next()));
      else put_comp(v, r, fmt);
    }
    for (std::size_t k = 0; k < v.size() && k < stride; ++k) mem[at + e * stride + k] = v[k];
  }
}

// Two floats the same: the same bits, or both NaN. (Which NaN an operation with
// two NaN operands passes on depends on the order the compiler puts commutative
// operands in, which C++ leaves to it; every other value is compared bit for bit.)
bool same_float(float x, float y) { return fbits(x) == fbits(y) || (x != x && y != y); }
std::uint64_t g_nan_payloads = 0;

std::size_t compare(const std::vector<DrawCall>& a, const std::vector<DrawCall>& b, const char* what) {
  REQUIRE(a.size() == b.size());
  std::size_t bad = 0;
  for (std::size_t k = 0; k < a.size(); ++k) {
    REQUIRE(a[k].verts.size() == b[k].verts.size());
    CHECK(a[k].index == b[k].index);
    if (std::memcmp(a[k].verts.data(), b[k].verts.data(), a[k].verts.size() * sizeof(OutVertex)) == 0) continue;
    bool differ = false;
    for (std::size_t v = 0; v < a[k].verts.size(); ++v) {
      const float* fa = &a[k].verts[v].pos[0];
      const float* fb = &b[k].verts[v].pos[0];
      for (int f = 0; f < 36; ++f) {
        if (fbits(fa[f]) == fbits(fb[f])) continue;
        if (same_float(fa[f], fb[f])) {
          ++g_nan_payloads;
          continue;
        }
        if (bad < 3 && !differ)
          std::printf("%s: draw %zu vertex %zu float %d: fast %08x (%g) ref %08x (%g)\n", what, k, v, f, fbits(fa[f]),
                      static_cast<double>(fa[f]), fbits(fb[f]), static_cast<double>(fb[f]));
        differ = true;
      }
    }
    if (differ) ++bad;
  }
  return bad;
}

void run_trial(std::uint64_t seed, bool use_gp, std::uint64_t& draws, std::uint64_t& verts) {
  Rng r(seed);
  std::vector<std::uint8_t> mem(kMemSize);
  for (auto& b : mem) b = static_cast<std::uint8_t>(r.next());
  Pair p;
  p.ref.set_reference_vertices(true);
  GpMemory gp;
  if (use_gp) {
    gp.attach(mem.data(), kMemSize);
    p.fast.set_gp_memory(&gp);
    p.ref.set_gp_memory(&gp);
  } else {
    p.fast.set_memory(mem.data(), kMemSize);
    p.ref.set_memory(mem.data(), kMemSize);
  }

  // XF memory: position/texture, normal and post matrices, lights.
  {
    std::vector<std::uint32_t> w;
    for (int k = 0; k < 0x100; ++k) w.push_back(fbits((k & 3) == 3 ? r.range(-1000.0f, 1000.0f) : r.xf_float()));
    for (int k = 0; k < 0x100; k += 16) p.xf(xf::kPosMtx + k, std::vector<std::uint32_t>(w.begin() + k, w.begin() + k + 16));
    w.clear();
    for (int k = 0; k < 0x60; ++k) w.push_back(fbits(r.xf_float()));
    for (int k = 0; k < 0x60; k += 16) p.xf(xf::kNrmMtx + k, std::vector<std::uint32_t>(w.begin() + k, w.begin() + k + 16));
    w.clear();
    for (int k = 0; k < 0x100; ++k) w.push_back(fbits(r.xf_float()));
    for (int k = 0; k < 0x100; k += 16) p.xf(xf::kPostMtx + k, std::vector<std::uint32_t>(w.begin() + k, w.begin() + k + 16));
    for (std::uint32_t l = 0; l < 8; ++l) {
      std::vector<std::uint32_t> lw(16);
      lw[3] = r.next();
      for (int k = 4; k < 16; ++k) lw[k] = fbits(k >= 10 && k < 13 ? r.range(-500.0f, 500.0f) : r.xf_float());
      p.xf(xf::kLights + 16 * l, lw);
    }
  }
  // Registers: random, then the fields the transform reads chosen to be meaningful most of the time.
  auto randomize_regs = [&] {
    p.xf(xf::kNumColors, {r.below(4)});
    p.xf(xf::kAmbient0, {r.next(), r.next()});
    p.xf(xf::kMaterial0, {r.next(), r.next()});
    std::vector<std::uint32_t> ctl(4);
    for (auto& c : ctl) c = r.chance(20) ? r.next() : (r.next() & 0x7FFF);
    p.xf(xf::kColor0Cntrl, ctl);
    p.xf(xf::kDualTex, {r.below(2)});
    std::vector<std::uint32_t> proj(7);
    for (int k = 0; k < 6; ++k) proj[k] = fbits(r.xf_float());
    proj[6] = r.chance(80) ? r.below(2) : r.next();
    p.xf(xf::kProjection, proj);
    p.xf(xf::kViewport, {fbits(r.range(100, 400)), fbits(r.range(-300, 300)), fbits(r.range(0, 16777215.0f)),
                         fbits(r.range(300, 700)), fbits(r.range(300, 700)), fbits(r.range(0, 16777215.0f))});
    p.xf(xf::kNumTexGens, {r.chance(90) ? r.below(9) : r.below(16)});
    std::vector<std::uint32_t> tg(8), post(8);
    for (auto& t : tg) {
      // type (4-6) mostly 0-3, source row (7-11) mostly 0-12, emboss source (12-14).
      t = r.next() & 0x7FFF;
      if (r.chance(85)) t = (t & ~0x70u) | (r.below(4) << 4);
      if (r.chance(85)) t = (t & ~0xF80u) | (r.below(13) << 7);
      if (r.chance(5)) t = r.next();
    }
    for (auto& t : post) t = r.chance(90) ? (r.next() & 0x13F) : r.next();
    p.xf(xf::kTexGen, tg);
    p.xf(xf::kPostInfo, post);
    p.bp(bp::kScissorOffset, r.next() & 0xFFFFF);
  };
  randomize_regs();

  // Arrays: positions, normals, colors, texture coordinates.
  std::uint32_t base[12], stride[12];
  for (int a = 0; a < 12; ++a) {
    base[a] = 0x1000 + static_cast<std::uint32_t>(a) * 0x10000 + (r.below(64) << 2);
    stride[a] = r.chance(20) ? r.below(64) : 4 * (1 + r.below(12));
    p.cp(static_cast<std::uint8_t>(cp::kArrayBase + a), base[a]);
    p.cp(static_cast<std::uint8_t>(cp::kArrayStride + a), stride[a]);
  }
  // Indexed XF loads (arrays 12, 13: position and normal matrices) from tame floats.
  const std::uint32_t kXfArr = 0xD0000;
  for (std::uint32_t k = 0; k < 0x4000; k += 4) {
    const std::uint32_t w = fbits(r.xf_float());
    mem[kXfArr + k] = static_cast<std::uint8_t>(w >> 24);
    mem[kXfArr + k + 1] = static_cast<std::uint8_t>(w >> 16);
    mem[kXfArr + k + 2] = static_cast<std::uint8_t>(w >> 8);
    mem[kXfArr + k + 3] = static_cast<std::uint8_t>(w);
  }
  for (int a = 12; a < 14; ++a) {
    p.cp(static_cast<std::uint8_t>(cp::kArrayBase + a), kXfArr);
    p.cp(static_cast<std::uint8_t>(cp::kArrayStride + a), 48);
  }
  auto randomize_format = [&](int fmt) {
    // VCD: each attribute none/direct/index8/index16; VAT values random, formats mostly valid.
    std::uint32_t lo = r.next() & 0x1FFFF, hi = r.next() & 0xFFFF;
    if (r.chance(90)) lo |= 1u << 9;  // a position
    std::uint32_t a = r.next(), b = r.next(), c = r.next();
    auto valid = [&](std::uint32_t& reg, int at, std::uint32_t maxv) {
      if (r.chance(90)) reg = (reg & ~(7u << at)) | (r.below(maxv + 1) << at);
    };
    valid(a, 1, 4);   // position format
    valid(a, 10, 4);  // normal format
    valid(a, 14, 5);  // color 0
    valid(a, 18, 5);  // color 1
    valid(a, 22, 4);  // tex 0
    valid(b, 1, 4);
    valid(b, 10, 4);
    valid(b, 19, 4);
    valid(b, 28, 4);
    valid(c, 6, 4);
    valid(c, 15, 4);
    valid(c, 24, 4);
    if (r.chance(70)) a &= ~(1u << 9);  // NBT mostly off
    p.cp(cp::kVcdLo, lo);
    p.cp(cp::kVcdHi, hi);
    p.cp(static_cast<std::uint8_t>(cp::kVatA + fmt), a);
    p.cp(static_cast<std::uint8_t>(cp::kVatB + fmt), b);
    p.cp(static_cast<std::uint8_t>(cp::kVatC + fmt), c);
    // The arrays in each attribute's format.
    fill_array(mem, r, base[0], stride[0], bits(a, 1, 3), 3, false);
    fill_array(mem, r, base[1], stride[1], bits(a, 10, 3), 9, false);
    fill_array(mem, r, base[2], stride[2], 0, 4, true);
    fill_array(mem, r, base[3], stride[3], 0, 4, true);
    const std::uint32_t tfmt[8] = {bits(a, 22, 3), bits(b, 1, 3), bits(b, 10, 3), bits(b, 19, 3),
                                   bits(b, 28, 3), bits(c, 6, 3),  bits(c, 15, 3), bits(c, 24, 3)};
    for (int k = 0; k < 8; ++k) fill_array(mem, r, base[4 + k], stride[4 + k], tfmt[k], 2, false);
  };
  for (int f = 0; f < 8; ++f) randomize_format(f);
  p.cp(cp::kMatIndexA, r.next());
  p.cp(cp::kMatIndexB, r.next());

  if (use_gp) {
    // Snapshots of some array ranges (then memory moves on under them) and copies.
    for (int k = 0; k < 6; ++k) {
      const std::uint32_t at = base[r.below(12)] + r.below(0x800), len = 1 + r.below(0x400);
      gp.flush(at, len);
      for (std::uint32_t i = 0; i < len && at + i < kMemSize; ++i)
        if (r.chance(30)) mem[at + i] = static_cast<std::uint8_t>(r.next());
    }
    for (int k = 0; k < 4; ++k) {
      const std::uint32_t at = (base[r.below(12)] + r.below(0x800)) & ~63u, len = 64 * (1 + r.below(16));
      std::vector<std::uint8_t> copy(mem.begin() + at, mem.begin() + at + len);
      for (auto& b : copy)
        if (r.chance(30)) b = static_cast<std::uint8_t>(r.next());
      gp.shadow(at, len, copy.data());
    }
    if (r.chance(50)) {
      const std::uint32_t at = base[r.below(12)], len = 1 + r.below(0x400);
      gp.end_shadow(at, len);
      gp.write_through(at, len);
    }
  }

  const int prims = 60;
  int loader_bad = 0;
  for (int n = 0; n < prims; ++n) {
    const int fmt = static_cast<int>(r.below(8));
    // Between primitives: XF matrix writes (the transform's caches), register and format changes.
    switch (r.below(14)) {
      case 12: case 13: {  // an indexed XF load: a matrix (or a part, or a range across matrices) from memory
        const bool nrm = r.below(13) == 12 ? false : r.chance(40);
        const std::uint32_t addr = nrm ? xf::kNrmMtx + r.below(0x60) : r.below(0x100);
        const std::uint32_t cnt = 1 + r.below(16), index = r.below(256);
        std::vector<std::uint8_t> c{static_cast<std::uint8_t>(nrm ? 0x28 : 0x20)};
        Stream::put32(c, index << 16 | (cnt - 1) << 12 | addr);
        p.run(c);
        break;
      }
      case 0: {
        std::vector<std::uint32_t> w;
        const std::uint32_t at = r.below(0x100), cnt = 1 + r.below(16);
        for (std::uint32_t k = 0; k < cnt; ++k) w.push_back(fbits(r.xf_float()));
        p.xf(at, w);
        break;
      }
      case 1: {
        std::vector<std::uint32_t> w;
        const std::uint32_t at = xf::kNrmMtx + r.below(0x60), cnt = 1 + r.below(9);
        for (std::uint32_t k = 0; k < cnt; ++k) w.push_back(fbits(r.xf_float()));
        p.xf(at, w);
        break;
      }
      case 2: randomize_regs(); break;
      case 3: randomize_format(static_cast<int>(r.below(8))); break;
      case 4: p.cp(cp::kMatIndexA, r.next()); p.cp(cp::kMatIndexB, r.next()); break;
      case 5: {
        std::vector<std::uint32_t> w;
        const std::uint32_t at = xf::kPostMtx + r.below(0x100), cnt = 1 + r.below(12);
        for (std::uint32_t k = 0; k < cnt; ++k) w.push_back(fbits(r.xf_float()));
        p.xf(at, w);
        break;
      }
      case 6: {
        std::vector<std::uint32_t> lw;
        const std::uint32_t at = xf::kLights + r.below(0x80), cnt = 1 + r.below(8);
        for (std::uint32_t k = 0; k < cnt; ++k) lw.push_back(r.chance(50) ? r.next() : fbits(r.xf_float()));
        p.xf(at, lw);
        break;
      }
      default: break;
    }
    // The GP's view changing between primitives (the loader's windows must follow):
    // memory written, new snapshots and copies, copies ended, a field.
    if (use_gp) {
      const std::uint32_t at = base[r.below(12)] + r.below(0x1000), len = 1 + r.below(0x300);
      switch (r.below(8)) {
        case 0:
          for (std::uint32_t i = 0; i < len && at + i < kMemSize; ++i) mem[at + i] = static_cast<std::uint8_t>(r.next());
          break;
        case 1:
          gp.flush(at, len);
          for (std::uint32_t i = 0; i < len && at + i < kMemSize; ++i)
            if (r.chance(50)) mem[at + i] = static_cast<std::uint8_t>(r.next());
          break;
        case 2: {
          const std::uint32_t a64 = at & ~63u, l64 = 64 * (1 + r.below(12));
          std::vector<std::uint8_t> copy(mem.begin() + a64, mem.begin() + a64 + l64);
          for (auto& b : copy)
            if (r.chance(50)) b = static_cast<std::uint8_t>(r.next());
          gp.shadow(a64, l64, copy.data());
          break;
        }
        case 3: gp.end_shadow(at, len); break;
        case 4: gp.write_through(at, len); break;
        case 5: gp.field(); break;
        default: break;
      }
    }
    const std::uint32_t vsize = p.ref.vertex_size(fmt);
    const std::uint16_t count = static_cast<std::uint16_t>(r.chance(5) ? r.below(400) : r.below(24));
    // Vertex bytes: random, with direct f32 components tame where the format says so.
    std::vector<std::uint8_t> data(static_cast<std::size_t>(vsize) * count);
    for (auto& b : data) b = static_cast<std::uint8_t>(r.next() % (r.chance(50) ? 256 : 32));
    // The loaders alone, on each vertex.
    for (std::uint32_t k = 0; k < count && k < 4; ++k) {
      const InVertex a = p.fast.load_vertex_fast(fmt, data.data() + k * vsize);
      const InVertex b = p.ref.load_vertex(fmt, data.data() + k * vsize);
      const bool same = a.pnmtx == b.pnmtx && std::memcmp(a.texmtx, b.texmtx, 8) == 0 &&
                        std::memcmp(floats(a), floats(b), sizeof(float) * 36) == 0;
      if (!same && loader_bad++ < 3) {
        std::printf("loader: fmt %d vcd %08x %08x\n", fmt, p.ref.cp_vcd_lo(), p.ref.cp_vcd_hi());
        for (int f = 0; f < 36; ++f)
          if (fbits(floats(a)[f]) != fbits(floats(b)[f]))
            std::printf("  float %d: fast %08x ref %08x\n", f, fbits(floats(a)[f]), fbits(floats(b)[f]));
      }
    }
    std::vector<std::uint8_t> c{static_cast<std::uint8_t>(0x80 | (r.below(8) << 3) | fmt),
                                static_cast<std::uint8_t>(count >> 8), static_cast<std::uint8_t>(count)};
    c.insert(c.end(), data.begin(), data.end());
    p.fast.command(c.data(), c.size());
    p.ref.command(c.data(), c.size());
  }
  CHECK(loader_bad == 0);
  draws += p.fast_cap.raw.size();
  for (const auto& d : p.fast_cap.raw) verts += d.verts.size();
  const std::size_t bad = compare(p.fast_cap.raw, p.ref_cap.raw, use_gp ? "gp" : "direct");
  if (bad) std::printf("  (seed %llu)\n", static_cast<unsigned long long>(seed));
  CHECK(bad == 0);
}

}  // namespace

TEST_CASE("fast vertex path equals the reference path bit for bit (random formats and state)") {
  std::setvbuf(stdout, nullptr, _IONBF, 0);
  std::uint64_t draws = 0, verts = 0;
  const char* only = std::getenv("VX_SEED");  // development: one trial
  for (std::uint64_t seed = 1; seed <= 400; ++seed) {
    if (only && seed != std::strtoull(only, nullptr, 10)) continue;
    run_trial(seed, (seed & 1) != 0, draws, verts);
  }
  std::printf("vertex exactness: %llu draws, %llu vertices compared (%llu NaN floats with another NaN's payload)\n",
              static_cast<unsigned long long>(draws), static_cast<unsigned long long>(verts),
              static_cast<unsigned long long>(g_nan_payloads));
  CHECK(verts > 100000);
}
