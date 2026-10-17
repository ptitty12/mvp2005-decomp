// Data-free tests of the device models added for E0b item 10: the GX command
// stream and FIFO (with the CP breakpoint), the PE interrupts, the DVD drive,
// the AI sample counter and DMA, and the DSP mailbox protocol.
#include <doctest/doctest.h>

#include <algorithm>
#include <cstring>
#include <vector>

#include "common/bytes.h"
#include "runtime/gx_fifo.h"
#include "runtime/hw.h"
#include "vfs/source.h"

using namespace mvp;
using namespace mvp::rt;

namespace {

void put32(std::vector<u8>& v, u32 x) {
  for (int k = 3; k >= 0; --k) v.push_back(static_cast<u8>(x >> (8 * k)));
}
void cp_load(std::vector<u8>& v, u8 reg, u32 x) {
  v.push_back(0x08);
  v.push_back(reg);
  put32(v, x);
}
void bp_load(std::vector<u8>& v, u32 x) {
  v.push_back(0x61);
  put32(v, x);
}

struct Rig {
  std::vector<u8> mem = std::vector<u8>(0x01800000 + 64, 0);
  ppc::Cpu cpu;
  Hardware hw{&cpu, mem.data(), 0x01800000};
  void w(u32 a, u32 v, int size = 4) { hw.write(a, v, size); }
  u32 r(u32 a, int size = 4) { return hw.read(a, size); }
  void cp32(u32 off, u32 v) {  // a CP register pair: low half, then high half
    w(0xCC000000 + off, v & 0xFFFF, 2);
    w(0xCC000000 + off + 2, v >> 16, 2);
  }
  u32 cp32r(u32 off) { return r(0xCC000000 + off, 2) | r(0xCC000000 + off + 2, 2) << 16; }
  void wgp(const std::vector<u8>& bytes) {
    for (u8 b : bytes) w(0xCC008000, b, 1);
  }
};

}  // namespace

TEST_CASE("GX stream: command lengths, vertex sizes and PE effects") {
  GxFifo gx;
  std::vector<u8> s;
  // VCD: position and color 0 direct, texture coordinate 0 as an 8-bit index.
  cp_load(s, 0x50, 0x2200);
  cp_load(s, 0x60, 0x2);
  // VAT A, format 3: position XYZ f32 (12 bytes), color 0 RGBA8 (4 bytes).
  cp_load(s, 0x73, 0x1 | 4u << 1 | 1u << 13 | 5u << 14);
  // An XF load of 3 words, then a triangle strip of 2 vertices in format 3.
  s.push_back(0x10);
  put32(s, (2u << 16) | 0x1000);
  for (int k = 0; k < 3; ++k) put32(s, 0);
  s.push_back(0x98 | 3);
  s.push_back(0);
  s.push_back(2);
  s.insert(s.end(), 2 * 17, 0x5A);
  bp_load(s, 0x47001234);
  bp_load(s, 0x48005678);
  bp_load(s, 0x45000002);
  // Fed in two pieces, split inside the primitive's vertex data.
  GxFifo::Effects a = gx.push(s.data(), 40);
  GxFifo::Effects b = gx.push(s.data() + 40, s.size() - 40);
  CHECK(gx.vertex_size(3) == 17);
  CHECK_FALSE(a.draw_done);
  CHECK(b.draw_done);
  CHECK(b.token_int);
  CHECK(b.token == 0x5678);
  CHECK(gx.commands() == 8);
  CHECK(gx.primitives() == 1);
  CHECK(gx.vertices() == 2);
  CHECK(gx.bp_loads() == 3);
  CHECK_FALSE(gx.broken());
  // A byte that starts no command loses the stream.
  u8 junk = 0x77;
  gx.push(&junk, 1);
  CHECK(gx.broken());
  CHECK(gx.broken_opcode() == 0x77);
}

TEST_CASE("GX stream: display lists run from memory and may change the vertex format") {
  std::vector<u8> mem(0x1000, 0);
  std::vector<u8> list;
  cp_load(list, 0x50, 0x200);           // position direct only
  cp_load(list, 0x70, 0x1 | 4u << 1);   // XYZ f32
  bp_load(list, 0x45000002);            // draw done, from inside the list
  while (list.size() % 32) list.push_back(0);  // NOP padding
  std::memcpy(&mem[0x400], list.data(), list.size());
  GxFifo gx;
  gx.set_memory(mem.data(), static_cast<u32>(mem.size()));
  std::vector<u8> s{0x40};
  put32(s, 0x80000400);  // the list's address (physical bits only)
  put32(s, static_cast<u32>(list.size()));
  GxFifo::Effects fx = gx.push(s.data(), s.size());
  CHECK(fx.draw_done);
  CHECK(gx.display_lists() == 1);
  CHECK(gx.display_list_errors() == 0);
  CHECK(gx.vertex_size(0) == 12);
  // A list outside memory is skipped and counted.
  std::vector<u8> bad{0x40};
  put32(bad, 0x00FF0000);
  put32(bad, 0x20);
  gx.push(bad.data(), bad.size());
  CHECK(gx.display_list_errors() == 1);
}

// Q4: the observer gets the commands of each push at once (one gx_commands
// call), in order and with their bytes, display lists' commands included; an
// observer that takes them one by one (the default) sees the same sequence.
namespace {
struct Seen : GfxObserver {
  std::vector<std::vector<u8>> cmds;
  int batches = 0;
  void gx_command(const u8* c, std::size_t n) override { cmds.emplace_back(c, c + n); }
  void gx_commands(const GxCommand* c, std::size_t n) override {
    ++batches;
    GfxObserver::gx_commands(c, n);
  }
  void vi_field(std::uint64_t, std::uint32_t) override {}
};
struct OneByOne : GfxObserver {
  std::vector<std::vector<u8>> cmds;
  void gx_command(const u8* c, std::size_t n) override { cmds.emplace_back(c, c + n); }
  void vi_field(std::uint64_t, std::uint32_t) override {}
};
}  // namespace

TEST_CASE("GX stream: the observer gets each push's commands at once, in order") {
  std::vector<u8> mem(0x1000, 0);
  std::vector<u8> list;
  cp_load(list, 0x50, 0x200);
  bp_load(list, 0x66001234);
  while (list.size() % 32) list.push_back(0);
  std::memcpy(&mem[0x400], list.data(), list.size());
  std::vector<u8> s;
  cp_load(s, 0x50, 0x200);              // position direct
  cp_load(s, 0x70, 0x1 | 4u << 1);      // XYZ f32: 12 bytes a vertex
  s.push_back(0x90);                    // triangles, format 0, 7 vertices (84 bytes: over three bursts)
  s.push_back(0);
  s.push_back(7);
  for (int k = 0; k < 84; ++k) s.push_back(static_cast<u8>(k));
  s.push_back(0x40);  // a display list
  put32(s, 0x400);
  put32(s, static_cast<u32>(list.size()));
  bp_load(s, 0x45000002);
  while (s.size() % 32) s.push_back(0);
  Seen seen;
  OneByOne one;
  GxFifo a, b;
  a.set_memory(mem.data(), static_cast<u32>(mem.size()));
  b.set_memory(mem.data(), static_cast<u32>(mem.size()));
  a.set_observer(&seen);
  b.set_observer(&one);
  int pushes_with_commands = 0;
  for (std::size_t at = 0; at < s.size(); at += 32) {
    const std::size_t before = seen.cmds.size();
    a.push(s.data() + at, 32);
    b.push(s.data() + at, 32);
    if (seen.cmds.size() > before) ++pushes_with_commands;
  }
  CHECK(seen.batches == pushes_with_commands);
  CHECK(seen.cmds == one.cmds);
  REQUIRE(seen.cmds.size() >= 6);
  CHECK(seen.cmds[2].size() == 3 + 84);  // the primitive, whole
  CHECK(seen.cmds[2][3 + 83] == 83);
  CHECK(seen.cmds[3] == std::vector<u8>(list.begin(), list.begin() + 6));  // the list's first command
}

TEST_CASE("GX stream: a lost stream's report shows the last commands' first bytes") {
  GxFifo gx;
  std::vector<u8> s;
  cp_load(s, 0x50, 0x200);
  bp_load(s, 0x47001234);
  s.push_back(0x77);  // starts no command
  gx.push(s.data(), s.size());
  REQUIRE(gx.broken());
  const std::string r = gx.broken_context();
  CHECK(r.find("len 6: 08 50 00 00 02 00") != std::string::npos);
  CHECK(r.find("len 5: 61 47 00 12 34") != std::string::npos);
  CHECK(r.find("lost at: 77") != std::string::npos);
}

TEST_CASE("GX FIFO: bursts to memory, the GP follows, and stops at a breakpoint") {
  Rig g;
  constexpr u32 kBase = 0x00100000, kEnd = 0x00100FFC;
  g.w(0xCC00300C, kBase);  // PI (CPU) FIFO
  g.w(0xCC003010, kEnd);
  g.w(0xCC003014, kBase);
  g.cp32(0x20, kBase);  // CP (GP) FIFO
  g.cp32(0x24, kEnd);
  g.cp32(0x28, 0xC00);  // high and low watermarks
  g.cp32(0x2C, 0x400);
  g.cp32(0x34, kBase);
  g.cp32(0x38, kBase);
  g.w(0xCC000002, 0x11, 2);  // read enable, linked to the CPU FIFO
  g.w(0xCC00100A, 0x3, 2);   // PE token and finish interrupts enabled
  g.w(0xCC003004, 0xE00);    // PI mask: PE token, PE finish, CP

  std::vector<u8> burst;
  bp_load(burst, 0x48001234);
  bp_load(burst, 0x47001234);
  burst.resize(32, 0);
  g.wgp(burst);
  CHECK(std::memcmp(&g.mem[kBase], burst.data(), 32) == 0);
  CHECK(g.r(0xCC003014) == kBase + 32);
  CHECK(g.cp32r(0x34) == kBase + 32);
  CHECK(g.cp32r(0x38) == kBase + 32);  // the GP read it all
  CHECK(g.r(0xCC00100E, 2) == 0x1234);
  CHECK((g.hw.pi_cause() & 0x200) != 0);
  g.w(0xCC00100A, 0x3 | 0x4, 2);  // acknowledge the token
  CHECK((g.hw.pi_cause() & 0x200) == 0);
  CHECK((g.r(0xCC000000, 2) & 0xC) == 0xC);  // idle

  // A breakpoint at the GP's position: reached at once.
  g.cp32(0x3C, kBase + 32);
  g.w(0xCC000002, 0x11 | 0x2 | 0x20, 2);
  CHECK((g.r(0xCC000000, 2) & 0x10) != 0);
  CHECK((g.hw.pi_cause() & 0x800) != 0);
  // While it is set the GP waits: the next burst stays in memory, unread.
  std::vector<u8> second;
  bp_load(second, 0x45000002);
  second.resize(32, 0);
  g.wgp(second);
  CHECK(g.cp32r(0x34) == kBase + 64);
  CHECK(g.cp32r(0x38) == kBase + 32);
  CHECK(g.cp32r(0x30) == 32);  // read-write distance
  CHECK(g.hw.draw_done_count() == 0);
  // The interrupt handler clears the breakpoint interrupt enable; the GP still
  // waits until the breakpoint itself is disabled.
  g.w(0xCC000002, 0x11 | 0x2, 2);
  CHECK((g.hw.pi_cause() & 0x800) == 0);
  CHECK(g.cp32r(0x38) == kBase + 32);
  g.w(0xCC000002, 0x11, 2);
  CHECK(g.cp32r(0x38) == kBase + 64);
  CHECK(g.hw.draw_done_count() == 1);
  CHECK((g.hw.pi_cause() & 0x400) != 0);

  // The write pointer wraps at the end, toggling the wrap bit.
  g.w(0xCC003014, kBase + 0xFE0);
  g.cp32(0x34, kBase + 0xFE0);
  g.cp32(0x38, kBase + 0xFE0);
  g.wgp(std::vector<u8>(32, 0));
  CHECK(g.r(0xCC003014) == (kBase | 0x04000000u));
  CHECK(g.cp32r(0x34) == kBase);
  CHECK(g.cp32r(0x38) == kBase);
}

TEST_CASE("GX FIFO: a redirected write-gather pipe writes memory, not commands") {
  // GXRedirectWriteGatherPipe: the GP unlinked, the PI FIFO set to base 0, end
  // 0x04000000 (the top of the physical space) and the write pointer at the
  // destination. The data (here not GX commands at all) lands in memory.
  Rig g;
  constexpr u32 kFifo = 0x00100000, kDest = 0x00200000;
  g.cp32(0x20, kFifo);
  g.cp32(0x24, kFifo + 0xFFC);
  g.cp32(0x34, kFifo);
  g.cp32(0x38, kFifo);
  g.w(0xCC000002, 0x01, 2);  // reads enabled, not linked
  // A partial burst left in the pipe (24 bytes of commands): writing WPAR, as the
  // redirect does, drops it (Hardware::wgp_reset), so the data starts at kDest.
  g.wgp(std::vector<u8>(24, 0x61));
  g.hw.wgp_reset();
  CHECK(g.hw.wgp_resets_dropping() == 1);
  g.w(0xCC00300C, 0);
  g.w(0xCC003010, 0x04000000);
  g.w(0xCC003014, kDest);
  std::vector<u8> pixels;
  for (int k = 0; k < 64; ++k) pixels.push_back(static_cast<u8>(k % 4 == 3 ? 0xFF : 0x53 + k));
  g.wgp(pixels);
  CHECK(std::memcmp(&g.mem[kDest], pixels.data(), pixels.size()) == 0);
  CHECK(g.r(0xCC003014) == kDest + 64);
  CHECK(g.cp32r(0x38) == kFifo);  // the GP read nothing
  CHECK_FALSE(g.hw.gx().broken());
  CHECK(g.hw.gx().commands() == 0);
}

TEST_CASE("DI: a disc read completes after its latency and transfer time") {
  Rig g;
  std::vector<u8> disc(0x10000);
  for (std::size_t k = 0; k < disc.size(); ++k) disc[k] = static_cast<u8>(k * 7 + 3);
  vfs::MemorySource src(disc);
  g.hw.set_disc(&src);
  g.w(0xCC006000, 0x2A);  // DVDInit: all three interrupt masks
  g.w(0xCC003004, 0x4);
  g.w(0xCC006008, 0xA8000000);
  g.w(0xCC00600C, 0x1000 >> 2);
  g.w(0xCC006010, 0x40);
  g.w(0xCC006014, 0x80002000);
  g.w(0xCC006018, 0x40);
  g.w(0xCC00601C, 3);  // DMA, start
  const HardwareConfig cfg;
  std::uint64_t done = cfg.di_latency_tb + 0x40ull * cfg.di_tb_per_kib / 1024;
  CHECK(g.hw.next_event_tb() == done);
  g.hw.advance(done - 1);
  CHECK((g.r(0xCC00601C) & 1) == 1);
  CHECK_FALSE(g.hw.interrupt_pending());
  g.hw.advance(done);
  CHECK((g.r(0xCC00601C) & 1) == 0);
  CHECK(g.r(0xCC006018) == 0);
  CHECK(std::memcmp(&g.mem[0x2000], &disc[0x1000], 0x40) == 0);
  CHECK((g.r(0xCC006000) & 0x10) != 0);  // TCINT
  CHECK((g.hw.pi_cause() & 0x4) != 0);
  CHECK(g.hw.interrupt_pending());
  g.w(0xCC006000, 0x2A | 0x10);  // acknowledge
  CHECK_FALSE(g.hw.interrupt_pending());
  CHECK(g.hw.di_bytes() == 0x40);
  // A read past the end of the disc stops the run.
  g.w(0xCC00600C, 0x20000 >> 2);
  g.w(0xCC006018, 0x20);
  g.w(0xCC00601C, 3);
  CHECK_THROWS_AS(g.hw.advance(done * 3), StopRun);
  // Commands the drive model does not know stop the run when started.
  g.w(0xCC006008, 0x55000000);
  CHECK_THROWS_AS(g.w(0xCC00601C, 1), StopRun);
}

TEST_CASE("AI: the sample counter counts at the stream rate while playing") {
  Rig g;
  CHECK(g.r(0xCC006C08) == 0);
  CHECK(g.hw.next_event_tb() == 1351350);  // nothing but the next VI frame
  g.w(0xCC006C00, 0x1 | 0x20);  // play at 32 kHz, counter reset
  CHECK(g.hw.next_event_tb() == 1266);  // 40.5 MHz / 32 kHz = 1265.6 ticks
  g.hw.advance(1265);
  CHECK(g.r(0xCC006C08) == 0);
  g.hw.advance(1266);
  CHECK(g.r(0xCC006C08) == 1);
  g.w(0xCC006C00, 0x1 | 0x2);  // 48 kHz from here, counting on from 1
  CHECK(g.hw.next_event_tb() == 1266 + 844);
  g.hw.advance(1266 + 844);
  CHECK(g.r(0xCC006C08) == 2);
  g.w(0xCC006C00, 0x2);  // stopped: the counter holds
  g.hw.advance(100000);
  CHECK(g.r(0xCC006C08) == 2);
}

TEST_CASE("DSP: boot ROM, task loading and the AX frame handshake") {
  Rig g;
  auto mail_from_dsp = [&] {
    u32 hi = g.r(0xCC005004, 2);
    u32 lo = g.r(0xCC005006, 2);
    return hi << 16 | lo;
  };
  auto mail_to_dsp = [&](u32 m) {
    g.w(0xCC005000, m >> 16, 2);
    g.w(0xCC005002, m & 0xFFFF, 2);
    CHECK((g.r(0xCC005000, 2) & 0x8000) == 0);  // taken at once
  };
  // The IPL's init microcode, then the boot ROM after the next reset.
  g.w(0xCC00500A, 0x1 | 0x4, 2);
  g.w(0xCC00500A, 0x0, 2);
  CHECK(mail_from_dsp() == 0x80544348u);
  g.w(0xCC00500A, 0x1 | 0x4, 2);
  g.w(0xCC00500A, 0x100, 2);  // unhalt, DSP interrupt unmasked
  CHECK(mail_from_dsp() == 0x8071FEEDu);
  CHECK((g.r(0xCC005004, 2) & 0x8000) == 0);
  // __DSP_boot_task's ten mails.
  const u32 boot[10] = {0x80F3A001, 0x80631560, 0x80F3C002, 0, 0x80F3A002, 0x1F00, 0x80F3B002, 0, 0x80F3D001, 0x10};
  for (u32 m : boot) mail_to_dsp(m);
  g.w(0xCC003004, 0x40);
  CHECK_FALSE(g.hw.interrupt_pending());
  g.hw.advance(g.hw.next_event_tb());
  CHECK(g.hw.interrupt_pending());
  CHECK(mail_from_dsp() == 0xDCD10000u);
  g.w(0xCC00500A, 0x100 | 0x80, 2);  // acknowledge DSPINT
  CHECK_FALSE(g.hw.interrupt_pending());
  // An AX frame: command list length and address; the voices advance
  // (runtime/ax.h); the microcode yields. The list names one PB: a running PCM16
  // voice of 64 samples, without rate conversion, which ends in this frame.
  const u16 cl[] = {0x0002, 0x8070, 0x0100, 0x0003};
  for (u32 k = 0; k < 4; ++k) put_be16(&g.mem[0x700000 + 2 * k], cl[k]);
  put_be16(&g.mem[0x700100 + 0x0E], 1);       // state: run
  put_be16(&g.mem[0x700100 + 0x08], 2);       // srcSelect: none
  put_be16(&g.mem[0x700100 + 0x70], 0x000A);  // format: PCM16
  put_be32(&g.mem[0x700100 + 0x76], 63);      // end address
  std::uint64_t t0 = 1000000;
  g.hw.advance(t0);
  mail_to_dsp(0xBABE0180);
  mail_to_dsp(0x80700000);
  CHECK(g.hw.ax_stats().voice_frames == 1);
  CHECK(g.hw.ax_stats().samples == 64);
  CHECK(g.hw.ax_stats().ended == 1);
  CHECK(be16(&g.mem[0x700100 + 0x0E]) == 0);   // stopped
  CHECK(be32(&g.mem[0x700100 + 0x7A]) == 63);  // at its end
  CHECK(g.hw.next_event_tb() == t0 + 20250);
  g.hw.advance(t0 + 20250);
  CHECK(mail_from_dsp() == 0xDCD10002u);
  CHECK(g.hw.dsp_frames() == 1);
  mail_to_dsp(0xCDD10003);  // resume the same task
  // A command list the model does not understand stops the run.
  put_be16(&g.mem[0x700200], 0x0009);
  mail_to_dsp(0xBABE0180);
  CHECK_THROWS_AS(mail_to_dsp(0x80700200), StopRun);
  CHECK_THROWS_AS(mail_to_dsp(0x12345678), StopRun);
}

TEST_CASE("AI DMA: a block every 5 ms at 32 kHz, AIDINT at each block start") {
  Rig g;
  g.w(0xCC006C00, 0x40);  // DSP sample rate 32 kHz
  g.w(0xCC005030, 0x8070, 2);
  g.w(0xCC005032, 0x0000, 2);
  g.w(0xCC00500A, 0x10, 2);       // AIDINT unmasked
  g.w(0xCC005036, 0x280 / 32, 2);  // 20 blocks of 32 bytes: 160 stereo samples
  g.w(0xCC003004, 0x40);
  CHECK_FALSE(g.hw.interrupt_pending());
  g.w(0xCC005036, 0x8000 | (0x280 / 32), 2);  // start
  CHECK(g.hw.interrupt_pending());
  CHECK(g.hw.ai_dma_blocks() == 1);
  g.w(0xCC00500A, 0x10 | 0x8, 2);  // acknowledge
  CHECK_FALSE(g.hw.interrupt_pending());
  CHECK(g.hw.next_event_tb() == 202500);  // 5 ms
  g.hw.advance(101250);
  CHECK(g.r(0xCC00503A, 2) == 10);  // half the blocks left
  g.hw.advance(202500);
  CHECK(g.hw.interrupt_pending());
  CHECK(g.hw.ai_dma_blocks() == 2);
  g.w(0xCC005036, 0x280 / 32, 2);  // stop
  g.hw.advance(1000000);
  CHECK(g.hw.ai_dma_blocks() == 2);
}

TEST_CASE("Q4: Hardware::wgp_store does what write does for the write-gather pipe") {
  // Two machines with a CPU FIFO in memory (the GP unlinked): one gets every
  // store through write(), the other through wgp_store(); before and after the
  // access log is full (wgp_store's inline path only applies then).
  Rig a, b;
  constexpr u32 kBase = 0x00100000, kEnd = 0x0010FFE0;
  for (Rig* g : {&a, &b}) {
    g->w(0xCC00300C, kBase);
    g->w(0xCC003010, kEnd);
    g->w(0xCC003014, kBase);
  }
  std::uint64_t seed = 0x9E3779B97F4A7C15ull;
  auto next = [&] {
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return static_cast<u32>(seed >> 16);
  };
  u32 pc = 0x80003000;
  std::uint64_t ticks = 1000;
  auto stores = [&](int n) {
    for (int k = 0; k < n; ++k) {
      const u32 r = next();
      const int size = (r & 3) == 0 ? 1 : (r & 3) == 1 ? 2 : 4;
      const u32 v = size == 4 ? next() : next() & ((1u << (8 * size)) - 1);
      const u32 addr = 0xCC008000u + ((r >> 2) & 1) * 4;
      pc += 4;
      ticks += 1 + (r >> 3 & 7);
      a.cpu.pc = pc;
      a.cpu.ticks = ticks;
      a.hw.write(addr, v, size);
      b.cpu.pc = 0;  // wgp_store sets them where write() would read them
      b.cpu.ticks = 0;
      b.hw.wgp_store(addr, v, size, pc, ticks);
    }
  };
  stores(300);
  for (Rig* g : {&a, &b})
    for (int k = 0; k < 100000; ++k) g->r(0xCC003000);
  stores(20000);
  CHECK(a.hw.gp_bursts() > 1000);
  CHECK(a.hw.gp_bursts() == b.hw.gp_bursts());
  CHECK(a.hw.write_counts() == b.hw.write_counts());
  CHECK(a.r(0xCC003014) == b.r(0xCC003014));
  // The partial burst under way: the next 32 bytes complete it the same way.
  std::vector<u8> fill(32, 0x5A);
  a.wgp(fill);
  b.wgp(fill);
  CHECK(a.r(0xCC003014) == b.r(0xCC003014));
  CHECK(std::memcmp(&a.mem[kBase], &b.mem[kBase], kEnd - kBase) == 0);
}

TEST_CASE("Q6: the GP reading deferred bursts in one piece leaves what reading each one does") {
  // Two machines, the GP linked to a small CPU FIFO that wraps often, PE
  // interrupts on: one lets the GP read every burst at once, the other holds
  // the GP (gp_defer) over groups of 1-9 bursts. After each group: the same
  // pointers, PE token and interrupt state, finishes, commands seen, memory.
  Rig a, b;
  Seen sa, sb;
  constexpr u32 kBase = 0x00100000, kEnd = 0x001003E0;  // 1 KiB
  for (Rig* g : {&a, &b}) {
    g->w(0xCC00300C, kBase);
    g->w(0xCC003010, kEnd);
    g->w(0xCC003014, kBase);
    g->cp32(0x20, kBase);
    g->cp32(0x24, kEnd);
    g->cp32(0x28, 0x300);
    g->cp32(0x2C, 0x100);
    g->cp32(0x34, kBase);
    g->cp32(0x38, kBase);
    g->w(0xCC000002, 0x11, 2);  // read enable, linked
    g->w(0xCC00100A, 0x3, 2);   // PE token and finish interrupts
    g->w(0xCC003004, 0xE00);
  }
  a.hw.set_gfx_observer(&sa);
  b.hw.set_gfx_observer(&sb);
  std::uint64_t seed = 0x2545F4914F6CDD1Dull;
  auto next = [&] {
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return static_cast<u32>(seed >> 16);
  };
  std::vector<u8> s;  // a stream: NOPs, XF loads, CP loads, tokens, finishes
  while (s.size() < 256 * 1024) {
    switch (next() % 6) {
      case 0: s.push_back(0); break;
      case 1: {
        const u32 n = next() % 16;
        s.push_back(0x10);
        put32(s, (n << 16) | (next() & 0x3FF));
        for (u32 k = 0; k <= n; ++k) put32(s, next());
        break;
      }
      case 2: cp_load(s, static_cast<u8>(0x30 + next() % 2), next()); break;
      case 3: bp_load(s, 0x47000000u | (next() & 0xFFFF)); break;
      case 4: bp_load(s, 0x48000000u | (next() & 0xFFFF)); break;
      default: bp_load(s, 0x45000002u); break;
    }
  }
  s.resize(s.size() & ~std::size_t(31));
  int groups = 0;
  for (std::size_t at = 0; at < s.size();) {
    const std::size_t n = std::min<std::size_t>(s.size() - at, 32 * (1 + next() % 9));
    const std::vector<u8> part(s.begin() + static_cast<std::ptrdiff_t>(at), s.begin() + static_cast<std::ptrdiff_t>(at + n));
    a.wgp(part);
    b.hw.gp_defer(true);
    b.wgp(part);
    b.hw.gp_defer(false);
    at += n;
    ++groups;
    REQUIRE(a.cp32r(0x34) == b.cp32r(0x34));
    REQUIRE(a.cp32r(0x38) == b.cp32r(0x38));
    REQUIRE(a.r(0xCC003014) == b.r(0xCC003014));
    REQUIRE(a.r(0xCC00100E, 2) == b.r(0xCC00100E, 2));
    REQUIRE(a.r(0xCC00100A, 2) == b.r(0xCC00100A, 2));
    REQUIRE(a.hw.pi_cause() == b.hw.pi_cause());
    REQUIRE(a.hw.draw_done_count() == b.hw.draw_done_count());
    if (next() % 4 == 0)  // the handler acknowledges both, sometimes
      for (Rig* g : {&a, &b}) g->w(0xCC00100A, 0x3 | 0xC, 2);
  }
  CHECK(groups > 1000);
  CHECK(a.hw.draw_done_count() > 100);
  CHECK(sa.cmds == sb.cmds);
  CHECK(sb.batches < sa.batches);  // fewer, larger pushes
  CHECK(std::memcmp(&a.mem[kBase], &b.mem[kBase], kEnd + 32 - kBase) == 0);
}
