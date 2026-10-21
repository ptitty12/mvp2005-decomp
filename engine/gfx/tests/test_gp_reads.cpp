// What the renderer will read for each command (P2, gfx/gp_reads.h): the array
// elements a primitive's indices select (8- and 16-bit indices, NBT with three
// indices), indexed XF loads, TLUT loads, TMEM preloads, EFB copies to
// textures, and each texture the first time a draw samples it after an
// invalidation or a TLUT load.
#include <doctest/doctest.h>

#include <utility>
#include <vector>

#include "gfx/gp_reads.h"
#include "gfx/texture_decode.h"
#include "tests/gx_stream.h"

using namespace mvp;
using namespace mvp::gfx;
using namespace mvp::gfx::test;

namespace {

struct Recorder {
  std::vector<std::pair<u32, u32>> ranges;
  std::vector<ReadKind> kinds;
  GpReads reads{[this](u32 a, u32 n, ReadKind k) {
    ranges.emplace_back(a, n);
    kinds.push_back(k);
  }};
  void run(const Stream& s) {
    for (const auto& c : s.cmds) reads.command(c.data(), c.size());
  }
  std::vector<std::pair<u32, u32>> take() {
    auto r = std::move(ranges);
    ranges.clear();
    kinds.clear();
    return r;
  }
};

}  // namespace

TEST_CASE("indexed attributes: the elements the indices select") {
  Recorder r;
  Stream s;
  // pnmtx direct; position 16-bit index (s16 xyz: 6 bytes); color0 8-bit index (rgba8: 4 bytes).
  s.cp(cp::kVcdLo, 1u | 3u << 9 | 2u << 13);
  s.cp(cp::kVcdHi, 0);
  s.cp(cp::kVatA, 1u | 3u << 1 | 5u << 14);  // xyz, s16; color0 rgba8
  s.cp(cp::kArrayBase + 0, 0x80100000);       // a virtual address: physical 0x100000
  s.cp(cp::kArrayStride + 0, 6);
  s.cp(cp::kArrayBase + 2, 0x00200000);
  s.cp(cp::kArrayStride + 2, 8);
  // Three vertices: (pnmtx, pos index hi lo, color index).
  s.prim(cmd::kTriangles, 3, {0, 0x01, 0x00, 7, 0, 0x00, 0x10, 2, 0, 0x00, 0x20, 5});
  r.run(s);
  REQUIRE(r.kinds.size() == 2);
  CHECK(r.kinds[0] == ReadKind::kVertex);
  auto got = r.take();
  REQUIRE(got.size() == 2);
  CHECK(got[0] == std::pair<u32, u32>(0x100000 + 0x10 * 6, (0x100 - 0x10) * 6 + 6));
  CHECK(got[1] == std::pair<u32, u32>(0x200000 + 2 * 8, 5 * 8 + 4));

  // Normal, binormal, tangent with three 8-bit indices (s8: 3 bytes a vector), position direct.
  Stream t;
  t.cp(cp::kVcdLo, 1u << 9 | 2u << 11);
  t.cp(cp::kVatA, 1u | 1u << 1 | 1u << 9 | 1u << 10 | 1u << 31);  // pos xyz s8; NBT, s8, three indices
  t.cp(cp::kArrayBase + 1, 0x300000);
  t.cp(cp::kArrayStride + 1, 9);
  t.prim(cmd::kPoints, 1, {1, 2, 3, 4, 9, 6});  // position (3 bytes), then the indices 4, 9, 6
  r.run(t);
  got = r.take();
  REQUIRE(got.size() == 1);
  CHECK(got[0] == std::pair<u32, u32>(0x300000 + 4 * 9, 5 * 9 + 3));
}

TEST_CASE("XF indexed loads, TLUT loads, preloads, EFB copies to textures") {
  Recorder r;
  Stream s;
  s.cp(cp::kArrayBase + 12, 0x400000);
  s.cp(cp::kArrayStride + 12, 48);
  s.raw({0x20, 0x00, 0x05, 0xB0, 0x00});  // LOAD_INDX_A: index 5, 12 words to XF 0
  s.bp(bp::kLoadTlut0, 0x500000 >> 5);
  s.bp(bp::kLoadTlut1, 16u << 10);         // 16 x 32 bytes
  s.bp(bp::kPreloadAddr, 0x600000 >> 5);
  s.bp(bp::kPreloadMode, 4u | 3u << 15);  // 4 RGBA8 lines: 64 bytes each
  s.bp(bp::kEfbWH, 63u | 31u << 10);       // 64 x 32
  s.bp(bp::kXfbAddr, 0x700000 >> 5);
  s.bp(bp::kTriggerCopy, ((6u & 7) << 1 | 6u >> 3) << 3);  // RGBA8, to a texture
  s.bp(bp::kTriggerCopy, 1u << 14);                        // to the XFB: no hash, nothing read
  r.run(s);
  CHECK(r.kinds == std::vector<ReadKind>{ReadKind::kXf, ReadKind::kTlut, ReadKind::kPreload, ReadKind::kCopy});
  auto got = r.take();
  REQUIRE(got.size() == 4);
  CHECK(got[0] == std::pair<u32, u32>(0x400000 + 5 * 48, 48));
  CHECK(got[1] == std::pair<u32, u32>(0x500000, 512));
  CHECK(got[2] == std::pair<u32, u32>(0x600000, 256));
  CHECK(got[3] == std::pair<u32, u32>(0x700000, 64 * 32 * 4));
}

TEST_CASE("textures: the first draw after an invalidation or a TLUT load") {
  Recorder r;
  Stream s;
  s.cp(cp::kVcdLo, 1u << 9);  // position direct
  s.cp(cp::kVatA, 1u | 4u << 1);  // xyz f32
  s.bp(bp::kGenMode, 1u);  // one texgen, one stage
  s.bp(bp::kTref, 1u << 6);  // stage 0 samples map 0
  s.bp(bp::tex_reg(bp::kTexImage0, 0), 63u | 63u << 10 | 6u << 20);  // 64 x 64 RGBA8
  s.bp(bp::tex_reg(bp::kTexImage3, 0), 0x800000 >> 5);
  std::vector<u8> v(12, 0);
  s.prim(cmd::kPoints, 1, v);
  s.prim(cmd::kPoints, 1, v);  // the same texture, the same epoch: read once
  r.run(s);
  auto got = r.take();
  REQUIRE(got.size() == 1);
  CHECK(got[0] == std::pair<u32, u32>(0x800000, texture_size(6, 64, 64, 1)));
  Stream t;
  t.bp(bp::kTexInvalidate, 0);
  t.prim(cmd::kPoints, 1, v);
  t.bp(bp::kLoadTlut0, 0x900000 >> 5);
  t.bp(bp::kLoadTlut1, 1u << 10);
  t.prim(cmd::kPoints, 1, v);
  r.run(t);
  got = r.take();
  REQUIRE(got.size() == 3);  // the texture, the palette, the texture again
  CHECK(got[0].first == 0x800000);
  CHECK(got[1] == std::pair<u32, u32>(0x900000, 32));
  CHECK(got[2].first == 0x800000);
}

TEST_CASE("indexed attributes: within an epoch, each array's elements are reported once") {
  Recorder r;
  Stream s;
  s.cp(cp::kVcdLo, 2u << 9);  // position: 8-bit index
  s.cp(cp::kVcdHi, 0);
  s.cp(cp::kVatA, 1u | 3u << 1);  // xyz, s16: 6 bytes
  s.cp(cp::kArrayBase + 0, 0x00100000);
  s.cp(cp::kArrayStride + 0, 6);
  r.run(s);
  r.take();
  auto draw = [&](std::vector<u8> idx) {
    Stream p;
    p.prim(cmd::kPoints, static_cast<std::uint16_t>(idx.size()), idx);
    r.run(p);
    return r.take();
  };
  using R = std::vector<std::pair<u32, u32>>;
  // Without an epoch: every primitive reports what it reads.
  CHECK(draw({10, 20}) == R{{0x100000 + 60, 10 * 6 + 6}});
  CHECK(draw({12, 14}) == R{{0x100000 + 72, 2 * 6 + 6}});
  r.reads.set_epoch(7);
  CHECK(draw({10, 20}) == R{{0x100000 + 60, 10 * 6 + 6}});
  CHECK(draw({12, 14}).empty());                                          // inside what the epoch reported
  CHECK(draw({18, 25}) == R{{0x100000 + 21 * 6, 4 * 6 + 6}});              // only the new elements
  CHECK(draw({5, 12}) == R{{0x100000 + 5 * 6, 4 * 6 + 6}});
  CHECK(r.reads.stats().skipped == 1);
  CHECK(draw({40, 41}) == R{{0x100000 + 40 * 6, 1 * 6 + 6}});              // elsewhere: reported, and now the one kept
  CHECK(draw({10, 12}) == R{{0x100000 + 10 * 6, 2 * 6 + 6}});
  r.reads.forget();  // the owner cleared marks: everything again
  CHECK(draw({10, 12}) == R{{0x100000 + 10 * 6, 2 * 6 + 6}});
  CHECK(draw({10, 12}).empty());
  r.reads.set_epoch(8);  // a new epoch
  CHECK(draw({10, 12}) == R{{0x100000 + 10 * 6, 2 * 6 + 6}});
  s.cp(cp::kArrayBase + 0, 0x00200000);  // another array base: reported
  r.run(s);
  r.take();
  CHECK(draw({10, 12}) == R{{0x200000 + 10 * 6, 2 * 6 + 6}});
}

TEST_CASE("V4: commands reads_something leaves out report nothing") {
  Recorder r;
  Stream s;
  // State under which every primitive and indexed load would read memory.
  s.cp(cp::kVcdLo, 1u | 3u << 9 | 2u << 13);
  s.cp(cp::kVatA, 1u | 3u << 1 | 5u << 14);
  s.cp(cp::kArrayBase + 0, 0x100000);
  s.cp(cp::kArrayStride + 0, 6);
  s.cp(cp::kArrayBase + 2, 0x200000);
  s.cp(cp::kArrayStride + 2, 8);
  for (int a = 12; a < 16; ++a) {
    s.cp(static_cast<u8>(cp::kArrayBase + a), 0x400000);
    s.cp(static_cast<u8>(cp::kArrayStride + a), 16);
  }
  r.run(s);
  r.take();
  int skipped = 0;
  for (int op = 0; op < 256; ++op) {
    if (GpReads::reads_something(static_cast<u8>(op))) continue;
    ++skipped;
    std::vector<u8> c(64, 0x37);
    c[0] = static_cast<u8>(op);
    c[1] = 0;
    c[2] = 3;  // a count, were it a primitive
    r.reads.command(c.data(), c.size());
    CHECK_MESSAGE(r.take().empty(), "op ", op);
  }
  CHECK(skipped == 256 - (1 + 4 + 1 + 64));  // CP, the four indexed XF loads, BP, primitives 0x80-0xBF
}
