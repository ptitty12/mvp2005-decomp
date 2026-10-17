// What the GP reads of guest memory (Q1, gfx/gp_memory.h): memory as the CPU's
// data cache has written it back. A flush snapshots its lines until the next
// VI field; stores after it stay invisible to the GP until then (the front
// end's pattern: flush, draw, write the next sprite's vertices, wait); memory
// written without the cache ends the snapshot; lines never flushed read as
// memory is. Also the GX processor reading vertices through it.
#include <doctest/doctest.h>

#include <bit>
#include <cstring>
#include <vector>

#include "gfx/gp_memory.h"
#include "gfx/gx_processor.h"
#include "tests/gx_stream.h"

using namespace mvp;
using namespace mvp::gfx;
using namespace mvp::gfx::test;

TEST_CASE("a flush snapshots its lines until the next field") {
  std::vector<std::uint8_t> mem(1 << 16, 0);
  GpMemory g;
  g.attach(mem.data(), static_cast<std::uint32_t>(mem.size()));
  // Nothing flushed: the GP reads memory itself.
  mem[0x100] = 1;
  CHECK(g.view(0x100, 4) == mem.data() + 0x100);
  // Flushed, then written again (still in the cache): the GP sees the flushed value.
  g.flush(0x100, 4);
  mem[0x100] = 2;
  mem[0x11F] = 9;  // the same 32-byte line
  mem[0x120] = 7;  // the next line, never flushed: as memory is
  const std::uint8_t* v = g.view(0x100, 0x24);
  REQUIRE(v);
  CHECK(v[0] == 1);
  CHECK(v[0x1F] == 0);
  CHECK(v[0x20] == 7);
  CHECK(g.stats().overlays == 1);
  // A view that only partly covers the flushed line.
  v = g.view(0x110, 0x20);
  CHECK(v[0x0F] == 0);   // 0x11F from the snapshot
  CHECK(v[0x10] == 7);   // 0x120 from memory
  // Flushed again: the new contents.
  g.flush(0x100, 1);
  CHECK(g.view(0x100, 1)[0] == 2);
  // The next field: written back by now, whatever the cache did.
  mem[0x100] = 3;
  g.field();
  CHECK(g.view(0x100, 1) == mem.data() + 0x100);
  CHECK(g.view(0x100, 1)[0] == 3);
}

TEST_CASE("memory written without the cache ends a snapshot") {
  std::vector<std::uint8_t> mem(1 << 16, 0);
  GpMemory g;
  g.attach(mem.data(), static_cast<std::uint32_t>(mem.size()));
  g.flush(0x2000, 0x40);
  mem[0x2000] = 5;  // DMA wrote it
  mem[0x2020] = 6;  // the CPU wrote it (in the cache)
  g.write_through(0x2000, 0x20);
  const std::uint8_t* v = g.view(0x2000, 0x40);
  CHECK(v[0] == 5);
  CHECK(v[0x20] == 0);
  CHECK(g.stats().drops == 1);
}

TEST_CASE("the render thread's form: activate a range, contents for some lines") {
  std::vector<std::uint8_t> mem(1 << 16, 0);
  GpMemory g;
  g.attach(mem.data(), static_cast<std::uint32_t>(mem.size()));
  std::uint8_t line[32];
  std::memset(line, 0xAB, sizeof line);
  g.set_lines(0x3000 >> 5, 1, line);  // the emulation thread took these at the flush
  g.activate(0x3000, 0x40);
  std::memset(mem.data() + 0x3000, 0x11, 0x40);  // the CPU's later stores
  const std::uint8_t* v = g.view(0x3000, 0x40);
  CHECK(v[0] == 0xAB);
  CHECK(v[0x20] == 0);  // unchanged since its last snapshot (zeros)
  CHECK(g.view(0x2FFF, 1)[0] == 0);
  CHECK(g.view(0x3040, 1) == mem.data() + 0x3040);
  CHECK(g.view(0xFFFF, 2) == nullptr);  // outside memory
}

TEST_CASE("Q6: copies stand for memory until they end") {
  std::vector<std::uint8_t> mem(1 << 16, 0);
  GpMemory g;
  g.attach(mem.data(), static_cast<std::uint32_t>(mem.size()));
  std::vector<std::uint8_t> copy(0x80, 0x22);
  g.shadow(0x4000, 0x80, copy.data());  // two entries, as memory was when the command was issued
  std::memset(mem.data() + 0x4000, 0x33, 0xC0);  // the guest's later writes (after the commands)
  // A view inside the copies is the copy itself.
  const std::uint8_t* v = g.view(0x4010, 0x40);
  CHECK(v[0] == 0x22);
  CHECK(v[0x3F] == 0x22);
  // Partly: the copies laid over the view without them.
  v = g.view(0x4070, 0x20);
  CHECK(v[0x0F] == 0x22);  // 0x407F
  CHECK(v[0x10] == 0x33);  // 0x4080: memory
  // With a snapshot under the rest: both.
  std::uint8_t line[32];
  std::memset(line, 0x44, sizeof line);
  g.set_lines(0x4080 >> 5, 1, line);
  g.activate(0x4080, 0x20);
  v = g.view(0x4060, 0x40);
  CHECK(v[0x1F] == 0x22);
  CHECK(v[0x20] == 0x44);
  // The first entry ends (the guest writes there next): memory again; the second stays.
  g.end_shadow(0x4000, 0x40);
  CHECK(g.view(0x4000, 0x40) == mem.data() + 0x4000);
  CHECK(g.view(0x4040, 1)[0] == 0x22);
  g.end_shadow(0x4040, 0x40);
  CHECK(g.view(0x4040, 1) == mem.data() + 0x4040);
  CHECK(g.stats().shadows == 1);
  CHECK(g.stats().shadow_bytes == 0x80);
}

TEST_CASE("the GX processor reads vertex arrays as the GP sees them") {
  // Positions f32 xyz by 8-bit index at 0x1000; the game flushed (1, 1, 0) for
  // each vertex, then wrote (2, 2, 0) (the next sprite's, still in the cache).
  std::vector<std::uint8_t> mem(1 << 20, 0), flushed;
  auto put = [&](std::uint32_t a, float x) {
    const std::uint32_t w = std::bit_cast<std::uint32_t>(x);
    for (int b = 0; b < 4; ++b) mem[a + b] = static_cast<std::uint8_t>(w >> (24 - 8 * b));
  };
  for (int v = 0; v < 3; ++v) {
    put(0x1000 + 12 * v, 1.0f + v);
    put(0x1004 + 12 * v, 1.0f);
  }
  flushed = mem;
  GpMemory g;
  g.attach(mem.data(), static_cast<std::uint32_t>(mem.size()));
  g.flush(0x1000, 36);
  for (int v = 0; v < 3; ++v) put(0x1004 + 12 * v, 2.0f);
  Stream s = setup_2d();
  s.cp(cp::kVcdLo, 2u << 9);       // position by 8-bit index
  s.cp(cp::kVcdHi, 0);
  s.cp(cp::kVatA, 1u | 4u << 1);   // xyz, f32
  s.cp(cp::kArrayBase + 0, 0x1000);
  s.cp(cp::kArrayStride + 0, 12);
  s.prim(cmd::kTriangles, 3, {0, 1, 2});
  Capture a, b, c;
  GxProcessor pa(&a), pb(&b), pc(&c);
  pa.set_memory(mem.data(), static_cast<std::uint32_t>(mem.size()));
  pa.set_gp_memory(&g);                                                       // as the GP sees it
  pb.set_memory(flushed.data(), static_cast<std::uint32_t>(flushed.size()));  // what was flushed
  pc.set_memory(mem.data(), static_cast<std::uint32_t>(mem.size()));          // memory as it is
  s.run(pa);
  s.run(pb);
  s.run(pc);
  REQUIRE(a.draws.size() == 1);
  REQUIRE(b.draws.size() == 1);
  REQUIRE(c.draws.size() == 1);
  REQUIRE(a.draws[0].verts.size() == 3);
  for (int v = 0; v < 3; ++v) {
    CHECK(a.draws[0].verts[v].pos[1] == b.draws[0].verts[v].pos[1]);
    CHECK(a.draws[0].verts[v].pos[1] != c.draws[0].verts[v].pos[1]);
  }
  // The next field: the stores are written back.
  g.field();
  Capture d;
  GxProcessor pd(&d);
  pd.set_memory(mem.data(), static_cast<std::uint32_t>(mem.size()));
  pd.set_gp_memory(&g);
  s.run(pd);
  CHECK(d.draws[0].verts[0].pos[1] == c.draws[0].verts[0].pos[1]);
}
