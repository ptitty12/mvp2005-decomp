// Data-free tests of save-state files (runtime/state_file.h): the stream's
// encoding (common/state_io.h), the hardware's fields, and a whole machine
// written by one process's objects and read into fresh ones.
#include <doctest/doctest.h>

#include <array>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "common/state_io.h"
#include "runtime/host_yield.h"
#include "runtime/hw.h"
#include "runtime/input.h"
#include "runtime/machine.h"
#include "runtime/save_state.h"
#include "runtime/state_file.h"

using namespace mvp;
using namespace mvp::rt;

namespace {

enum class Small : std::uint8_t { A, B, C };

struct Rig {
  Machine m;
  Hardware hw{&m.cpu(), m.mem1(), Machine::kMem1Size};
  YieldingHooks hooks{&m, &hw};
  Rig() {
    m.memory().io = &hw;
    m.cpu().hooks = &hooks;
  }
};

}  // namespace

TEST_CASE("state stream: fixed widths, both directions in one order") {
  StateWriter w;
  const std::size_t n = 7;  // 4 bytes in wasm32, 8 natively: always 8 in the stream
  const std::vector<std::uint32_t> words{1, 2, 0xDEADBEEF};
  const std::vector<std::pair<std::uint64_t, int>> events{{5, -1}, {9, 3}};
  const std::array<std::uint8_t, 3> bytes{7, 8, 9};
  const std::int32_t grid[2][2] = {{1, -2}, {3, -4}};
  const std::string text = "MVP";
  w(true);
  w(Small::C);
  w(n);
  w(-5.5);
  w(1.25f);
  w(words);
  w(events);
  w(bytes);
  w(grid);
  w(text);
  // bool 1, enum 1, size_t 8, double 8, float 4, (8 + 12), (8 + 2 x 12), 3, 16, (8 + 3)
  CHECK(w.out().size() == 1 + 1 + 8 + 8 + 4 + 20 + 32 + 3 + 16 + 11);

  StateReader r(w.out());
  bool b = false;
  Small e = Small::A;
  std::size_t n2 = 0;
  double d = 0;
  float f = 0;
  std::vector<std::uint32_t> words2;
  std::vector<std::pair<std::uint64_t, int>> events2;
  std::array<std::uint8_t, 3> bytes2{};
  std::int32_t grid2[2][2] = {};
  std::string text2;
  r(b);
  r(e);
  r(n2);
  r(d);
  r(f);
  r(words2);
  r(events2);
  r(bytes2);
  r(grid2);
  r(text2);
  CHECK(b);
  CHECK(e == Small::C);
  CHECK(n2 == 7);
  CHECK(d == -5.5);
  CHECK(f == 1.25f);
  CHECK(words2 == words);
  CHECK(events2 == events);
  CHECK(bytes2 == bytes);
  CHECK(std::memcmp(grid2, grid, sizeof grid) == 0);
  CHECK(text2 == text);
  CHECK(r.left() == 0);
  std::uint32_t past = 0;
  CHECK_THROWS_AS(r(past), StateError);
}

TEST_CASE("hardware state: written, read into a fresh machine's snapshot, written again the same") {
  Rig a;
  // Some device state: DI registers, an EXI channel, the PE token, ARAM through
  // a DSP DMA's registers, and an incomplete GX command waiting for its bytes.
  a.hw.write(0xCC006008, 0xA8000000, 4);  // DI command buffer 0
  a.hw.write(0xCC006814, 0x00000080, 4);  // EXI channel 1 CSR
  a.hw.write(0xCC005020, 0x0000, 2);      // AR main memory address (high)
  for (u8 byte : {u8(0x61), u8(0x47), u8(0x00)}) a.hw.write(0xCC008000, byte, 1);  // a BP load, 3 of its 5 bytes
  StateWriter wa;
  a.hw.write_state(wa);

  Rig b;
  StateReader r(wa.out());
  auto snap = b.hw.snapshot();
  snap->read_state(r);
  CHECK(r.left() == 0);
  b.hw.restore_from(*snap);
  StateWriter wb;
  b.hw.write_state(wb);
  CHECK(wb.out() == wa.out());
}

TEST_CASE("state file: a whole machine into fresh objects") {
  Rig a;
  a.m.map_ram(0x7E000000u, 0x00800000u);  // the VM region, as VMInit maps it
  std::memcpy(a.m.mem1() + 0x1000, "save me", 8);
  a.m.cpu().gpr[3] = 0x80001234;
  a.m.cpu().fpr[1].ps1 = 0x3FF0000000000000ull;
  a.m.cpu().pc = 0x8014865C;
  a.m.cpu().spr[912] = 0x00040004;
  a.m.cpu().ticks = 123456789;
  InputQueue qa;
  qa.push(neutral_input(1));
  qa.begin_frame();
  SaveState s;
  save_state(s, a.m, a.hooks, a.hw, &qa);
  const std::vector<u8> mods{1, 2, 3}, views{9};
  const auto file = write_state_file("{\"label\":\"test\"}", s, mods, views);

  std::string err;
  CHECK(state_file_header(file, &err) == std::optional<std::string>("{\"label\":\"test\"}"));

  Rig b;  // no VM region yet: prepare_machine maps it
  InputQueue qb;
  SaveState t;
  std::vector<u8> mods2, views2;
  REQUIRE(read_state_file(file, b.hw, &qb, t, mods2, views2, &err));
  CHECK(!b.m.has_ram(0x7E000000u));
  prepare_machine(b.m, t);
  CHECK(b.m.has_ram(0x7E000000u));
  load_state(t, b.m, b.hooks, b.hw, &qb);
  CHECK(std::memcmp(b.m.mem1() + 0x1000, "save me", 8) == 0);
  CHECK(b.m.cpu().gpr[3] == 0x80001234);
  CHECK(b.m.cpu().fpr[1].ps1 == 0x3FF0000000000000ull);
  CHECK(b.m.cpu().pc == 0x8014865C);
  CHECK(b.m.cpu().spr[912] == 0x00040004);
  CHECK(b.m.cpu().ticks == 123456789);
  CHECK(b.m.cpu().hooks == &b.hooks);  // the host pointers stay this machine's
  CHECK(qb.frames() == 1);
  CHECK(mods2 == mods);
  CHECK(views2 == views);
  // And the same file again from the second machine.
  SaveState s2;
  save_state(s2, b.m, b.hooks, b.hw, &qb);
  CHECK(write_state_file("{\"label\":\"test\"}", s2, mods, views) == file);

  SUBCASE("damaged files are refused") {
    auto cut = file;
    cut.resize(cut.size() - 1);
    CHECK(!read_state_file(cut, b.hw, &qb, t, mods2, views2, &err));
    CHECK(err.find("end") != std::string::npos);
    auto bad = file;
    bad[0] = 'X';
    CHECK(!state_file_header(bad, &err));
    CHECK(err == "not a save state");
  }
}
