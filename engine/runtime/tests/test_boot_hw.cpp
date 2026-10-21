// Data-free tests for the boot environment and the hardware models: a synthetic
// disc for the low-memory layout; EXI (RTC, SRAM over imm and DMA), the DSP init
// handshake and the ARAM DMA driven through their registers as the SDK does; the
// memory card in slot A (its unlock, reads, programs and erases; a new card's format).
#include <doctest/doctest.h>

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <memory>

#include "common/bytes.h"
#include "common/state_io.h"
#include "runtime/boot.h"
#include "runtime/hw.h"
#include "runtime/sys_hooks.h"

using namespace mvp;
using namespace mvp::rt;

namespace {

// A minimal disc: header, BI2, a one-file FST, a one-section DOL.
std::unique_ptr<vfs::Source> synthetic_disc() {
  std::vector<u8> d(0x8000, 0);
  std::memcpy(&d[0], "GTST01", 6);
  put_be32(&d[0x1C], vfs::Gcm::kDiscMagic);
  for (u32 k = 0; k < 0x2000; ++k) d[0x440 + k] = static_cast<u8>(k);  // recognizable BI2
  // FST at 0x2440: root + one file "f" at 0x5000, 4 bytes.
  u32 fst = 0x2440;
  d[fst] = 1;
  put_be32(&d[fst + 8], 2);
  put_be32(&d[fst + 12], 0);
  put_be32(&d[fst + 16], 0x5000);
  put_be32(&d[fst + 20], 4);
  d[fst + 24] = 'f';
  put_be32(&d[0x424], fst);
  put_be32(&d[0x428], 26);
  put_be32(&d[0x42C], 64);
  // DOL at 0x6000: text at 0x80003100, 4 bytes (blr).
  u32 dol = 0x6000;
  put_be32(&d[0x420], dol);
  put_be32(&d[dol + 0], 0x100);
  put_be32(&d[dol + 0x48], 0x80003100);
  put_be32(&d[dol + 0x90], 4);
  put_be32(&d[dol + 0xE0], 0x80003100);
  put_be32(&d[dol + 0x100], 0x4E800020);
  return std::make_unique<vfs::MemorySource>(std::move(d));
}

u32 m32(Machine& m, u32 a) { return be32(m.mem1() + (a & 0x01FFFFFF)); }

}  // namespace

TEST_CASE("boot environment: low memory, FST and BI2 copies, CPU state") {
  std::string err;
  auto disc = vfs::open_disc(synthetic_disc(), false, &err);
  REQUIRE_MESSAGE(disc, err);
  Machine m;
  m.load_dol(disc->dol);
  BootLayout L = setup_boot_environment(m, *disc);
  CHECK(std::memcmp(m.mem1(), "GTST01", 6) == 0);
  CHECK(m32(m, 0x8000001C) == vfs::Gcm::kDiscMagic);
  CHECK(m32(m, 0x80000020) == 0x0D15EA5E);
  CHECK(m32(m, 0x80000028) == 0x01800000);
  CHECK(m32(m, 0x8000002C) == 3);
  CHECK(m32(m, 0x80000034) == L.arena_hi);
  CHECK(m32(m, 0x80000038) == L.fst);
  CHECK(m32(m, 0x8000003C) == 64);
  CHECK(m32(m, 0x800000F4) == L.bi2);
  CHECK(m32(m, 0x800000F8) == 162000000);
  CHECK(m32(m, 0x800000FC) == 486000000);
  CHECK(L.fst == 0x817FFFC0);  // 64-byte FST maximum, at the top of MEM1
  CHECK(L.bi2 == L.fst - 0x2000);
  CHECK(m.mem1()[(L.fst & 0x01FFFFFF) + 24] == 'f');
  CHECK(m.mem1()[(L.bi2 & 0x01FFFFFF) + 0x13] == 0x13);
  CHECK(m.cpu().pc == 0x80003100);
  CHECK((m.cpu().msr & (ppc::msr::IR | ppc::msr::DR)) == (ppc::msr::IR | ppc::msr::DR));
  CHECK(m.cpu().spr[536] == 0x80001FFF);
}

TEST_CASE("real mode maps MEM1 at 0, and the icbi check guards the DOL's code") {
  std::string err;
  auto disc = vfs::open_disc(synthetic_disc(), false, &err);
  REQUIRE(disc);
  Machine m;
  m.load_dol(disc->dol);
  SystemHooks h(&m);
  m.cpu().hooks = &h;
  CHECK(m.memory().host(0x00003100) == nullptr);
  u32 old = m.cpu().msr = ppc::msr::IR | ppc::msr::DR;
  m.cpu().msr = 0;
  h.msr_changed(m.cpu(), old);
  REQUIRE(m.memory().host(0x00003100) != nullptr);
  CHECK(be32(m.memory().host(0x00003100)) == 0x4E800020);
  m.cpu().msr = ppc::msr::IR | ppc::msr::DR;
  h.msr_changed(m.cpu(), 0);
  CHECK(m.memory().host(0x00003100) == nullptr);
  h.icbi(m.cpu(), 0x80003100);  // unchanged: fine
  m.mem1()[0x3100] ^= 1;
  CHECK_THROWS_AS(h.icbi(m.cpu(), 0x80003104), ppc::GuestFault);
  h.icbi(m.cpu(), 0x80000100);  // outside the DOL's code: not checked
}

namespace {

struct Rig {
  std::vector<u8> mem = std::vector<u8>(0x01800000 + 64, 0);
  ppc::Cpu cpu;
  Hardware hw{&cpu, mem.data(), 0x01800000};
  void w(u32 a, u32 v, int size = 4) { hw.write(a, v, size); }
  u32 r(u32 a, int size = 4) { return hw.read(a, size); }
  // EXI channel 0 transfer as EXIImm/EXIDma do it: select device 1, start, wait.
  void exi_select(u32 dev) { w(0xCC006800, (1u << (7 + dev)) | (3u << 4)); }
  void exi_deselect() { w(0xCC006800, 0); }
  void exi_imm(u32 data, u32 len, u32 rw) {
    w(0xCC006810, data);
    w(0xCC00680C, ((len - 1) << 4) | (rw << 2) | 1);
  }
};

}  // namespace

TEST_CASE("EXI: the IPL chip's SRAM and RTC") {
  Rig g;
  // RTC read.
  g.exi_select(1);
  g.exi_imm(0x20000000, 4, 1);
  CHECK((g.r(0xCC00680C) & 1) == 0);  // transfer complete
  CHECK((g.r(0xCC006800) & 0x8) != 0);  // TCINT
  g.exi_imm(0, 4, 0);
  CHECK(g.r(0xCC006810) == HardwareConfig{}.rtc_seconds);
  g.exi_deselect();
  // SRAM read by DMA into MEM1 0x1000.
  g.exi_select(1);
  g.exi_imm(0x20000100, 4, 1);
  g.w(0xCC006804, 0x00001000);
  g.w(0xCC006808, 64);
  g.w(0xCC00680C, 2 | 1);  // DMA read
  g.exi_deselect();
  CHECK(std::memcmp(&g.mem[0x1000], g.hw.sram().data(), 64) == 0);
  // The checksums cover counterBias..flags.
  u16 sum = 0, inv = 0;
  for (u32 k = 0x0C; k < 0x14; k += 2) {
    u16 v = static_cast<u16>(be16(&g.mem[0x1000 + k]));
    sum = static_cast<u16>(sum + v);
    inv = static_cast<u16>(inv + static_cast<u16>(~v));
  }
  CHECK(be16(&g.mem[0x1000]) == sum);
  CHECK(be16(&g.mem[0x1002]) == inv);
  CHECK(g.mem[0x1013] == 0x2C);
  // SRAM write of one byte at offset 0x12 (language), as WriteSram encodes offsets.
  g.exi_select(1);
  g.exi_imm(0xA0000100 | (0x12 << 6), 4, 1);
  g.exi_imm(0x05000000, 1, 1);
  g.exi_deselect();
  CHECK(g.hw.sram()[0x12] == 5);
  // Another device on channel 0 (the memory card slot) is absent: reads are 0.
  g.exi_select(0);
  g.exi_imm(0, 4, 1);
  g.exi_imm(0, 4, 0);
  CHECK(g.r(0xCC006810) == 0);
  CHECK((g.r(0xCC006800) & 0x1000) == 0);  // EXT: no card
  // TCINT is write-one-to-clear.
  g.w(0xCC006800, 0x8);
  CHECK((g.r(0xCC006800) & 0x8) == 0);
}

namespace {

// The SDK's side of the card (src/dolphin/card): its unlock arithmetic (CARDUnlock.c), its checksum and
// VerifyID's serial check (tu_8041C2D8.c), to check the card against what the game computes.
u32 sdk_exnor_1st(u32 data, u32 rshift) {
  u32 work = data;
  for (u32 i = 0; i < rshift; i++) {
    u32 wk = ~(work ^ (work >> 7) ^ (work >> 15) ^ (work >> 23));
    work = (work >> 1) | ((wk << 30) & 0x40000000u);
  }
  return work;
}
u32 sdk_exnor(u32 data, u32 lshift) {
  u32 work = data;
  for (u32 i = 0; i < lshift; i++) {
    u32 wk = ~(work ^ (work << 7) ^ (work << 15) ^ (work << 23));
    work = (work << 1) | ((wk >> 30) & 0x00000002u);
  }
  return work;
}
u32 sdk_bitrev(u32 data) {
  u32 wk = 0, k = 0, j = 1;
  for (u32 i = 0; i < 32; i++) {
    if (i > 15) {
      if (i == 31) wk |= ((data & (1u << 31)) >> 31) & 1u;
      else {
        wk |= (data & (1u << i)) >> j;
        j += 2;
      }
    } else {
      wk |= (data & (1u << i)) << (31 - i - k);
      k++;
    }
  }
  return wk;
}
u32 sdk_next(u32 s, u32 shift) {
  u32 wk = sdk_exnor(s, shift);
  u32 wk1 = ~(wk ^ (wk << 7) ^ (wk << 15) ^ (wk << 23));
  return wk | ((wk1 >> 31) & 1u);
}
void sdk_checksum(const u8* p, u32 length, u16* sum, u16* inv) {
  *sum = *inv = 0;
  for (u32 k = 0; k + 1 < length; k += 2) {
    u16 w = static_cast<u16>(p[k] << 8 | p[k + 1]);
    *sum = static_cast<u16>(*sum + w);
    *inv = static_cast<u16>(*inv + static_cast<u16>(~w));
  }
  if (*sum == 0xFFFF) *sum = 0;
  if (*inv == 0xFFFF) *inv = 0;
}

// The card on channel 0, through the EXI registers as EXIImmEx does it (chunks of up to 4 bytes).
struct CardRig : Rig {
  void send(std::initializer_list<u8> bytes) {
    std::vector<u8> b(bytes);
    for (std::size_t k = 0; k < b.size(); k += 4) {
      u32 n = static_cast<u32>(std::min<std::size_t>(4, b.size() - k)), d = 0;
      for (u32 i = 0; i < n; ++i) d |= u32(b[k + i]) << (24 - 8 * i);
      exi_imm(d, n, 1);
    }
  }
  std::vector<u8> receive(u32 len) {
    std::vector<u8> out;
    while (out.size() < len) {
      u32 n = std::min<u32>(4, len - static_cast<u32>(out.size()));
      exi_imm(0, n, 0);
      u32 d = r(0xCC006810);
      for (u32 i = 0; i < n; ++i) out.push_back(static_cast<u8>(d >> (24 - 8 * i)));
    }
    return out;
  }
  u8 status() {
    exi_select(0);
    send({0x83, 0x00});
    u8 s = receive(1)[0];
    exi_deselect();
    return s;
  }
  // ReadArrayUnlock: the command, the latency's dummy bytes (4), then `rlen` bytes read.
  std::vector<u8> unlock_read(u32 data, u32 rlen, int mode) {
    data &= 0xFFFFF000u;
    exi_select(0);
    if (mode == 0)
      send({0x52, static_cast<u8>((data >> 29) & 3), static_cast<u8>(data >> 21), static_cast<u8>((data >> 19) & 3),
            static_cast<u8>((data >> 12) & 0x7F)});
    else
      send({0x52, static_cast<u8>(data >> 24), static_cast<u8>(data >> 16), 0, 0});
    send({0, 0, 0, 0});
    auto out = receive(rlen);
    exi_deselect();
    return out;
  }
};

}  // namespace

TEST_CASE("EXI: the memory card in slot A (runtime/memcard.h)") {
  CardRig g;
  // Without a card: no EXT (the test above). With one:
  g.hw.insert_card(MemoryCard::formatted(0, 0, 0x0123456789ABCDEFull));
  CHECK((g.r(0xCC006800) & 0x1000) != 0);  // EXT: a card in slot A
  CHECK((g.r(0xCC006814) & 0x1000) == 0);  // channel 1 (slot B): empty
  // EXIGetID: 0x0000, then the ID (64 Mbit, 8 KiB sectors: IsCard and the EA layer take it).
  g.exi_select(0);
  g.send({0x00, 0x00});
  auto id = g.receive(4);
  g.exi_deselect();
  CHECK(be32(id.data()) == 0x00000040u);
  // __CARDReadVendorID: not 0xEC.. (fast mode's 512-byte pages).
  g.exi_select(0);
  g.send({0x85, 0x00});
  auto vendor = g.receive(2);
  g.exi_deselect();
  CHECK(vendor[0] == 0xC2);
  // Locked at power-on: DoMount runs __CARDUnlock.
  CHECK((g.status() & 0x40) == 0);

  // __CARDUnlock: the SDK recovers the flash ID (zeros) from the second read, whatever its random value and
  // dummy lengths.
  for (u32 rnd : {0x123u, 0x7FFFu, 0u}) {
    CardRig c;
    c.hw.insert_card(MemoryCard::formatted(0, 0, 1));
    const u32 init_val = (0x7FEC8000u | rnd) & 0xFFFFF000u;
    const u32 dummy1 = 4 + rnd % 29, dummy2 = 4 + (rnd >> 3) % 29;
    c.unlock_read(init_val, dummy1, 0);
    u32 wk = sdk_exnor_1st(init_val, dummy1 * 8 + 1);
    u32 wk1 = ~(wk ^ (wk >> 7) ^ (wk >> 15) ^ (wk >> 23));
    u32 scramble = sdk_bitrev(wk | ((wk1 << 31) & 0x80000000u));
    auto rbuf = c.unlock_read(0, 20 + dummy2, 1);
    u32 words[3];
    for (int k = 0; k < 3; ++k) {
      words[k] = be32(rbuf.data() + 4 * k) ^ scramble;
      scramble = sdk_next(scramble, 32);
    }
    CHECK(words[0] == 0);
    CHECK(words[1] == 0);
    CHECK(words[2] == 0);
    CHECK((c.status() & 0x40) == 0);
    c.unlock_read(0x12340000u, dummy1, 1);  // DoneCallback's two reads
    c.unlock_read(0x56780000u, dummy2, 1);
    CHECK((c.status() & 0x40) != 0);  // unlocked
  }

  // Unlocked: reads, a page program and a sector erase, each ending with the EXI interrupt when it is on.
  CardRig c;
  const auto img = MemoryCard::formatted(0, 0, 1);
  c.hw.insert_card(img);
  c.unlock_read(0x7FEC8000u, 4, 0);
  for (int k = 0; k < 3; ++k) c.unlock_read(0, 24, 1);
  REQUIRE((c.status() & 0x40) != 0);
  auto read = [&](u32 addr, u32 len) {
    c.exi_select(0);
    c.send({0x52, static_cast<u8>((addr >> 17) & 0x7F), static_cast<u8>(addr >> 9), static_cast<u8>((addr >> 7) & 3),
            static_cast<u8>(addr & 0x7F)});
    c.send({0, 0, 0, 0});
    auto out = c.receive(len);
    c.exi_deselect();
    return out;
  };
  auto head = read(0, 0x200);
  CHECK(std::memcmp(head.data(), img.data(), 0x200) == 0);
  c.exi_select(0);
  c.send({0x81, 0x01});  // __CARDEnableInterrupt
  c.exi_deselect();
  c.w(0xCC006800, 0x1);  // EXIINTMASK
  const u32 page = 5 * 0x2000 + 0x80;
  c.exi_select(0);
  c.send({0xF2, static_cast<u8>((page >> 17) & 0x7F), static_cast<u8>(page >> 9), static_cast<u8>((page >> 7) & 3),
          static_cast<u8>(page & 0x7F)});
  for (u32 k = 0; k < 128; k += 4)
    c.send({static_cast<u8>(k), static_cast<u8>(k + 1), static_cast<u8>(k + 2), static_cast<u8>(k + 3)});
  c.w(0xCC006800, 0x1);  // deselect: the program starts
  CHECK((c.r(0xCC006800) & 0x2) == 0);
  CHECK(c.hw.next_event_tb() <= MemoryCard::kProgramTb);
  c.hw.advance(MemoryCard::kProgramTb);
  CHECK((c.r(0xCC006800) & 0x2) != 0);  // EXIINT
  CHECK((c.hw.pi_cause() & 0x10) != 0);
  c.w(0xCC006800, 0x3);  // clear it (write one), mask stays
  CHECK((c.r(0xCC006800) & 0x2) == 0);
  auto back = read(page, 128);
  for (u32 k = 0; k < 128; ++k) CHECK(back[k] == k);
  CHECK(c.hw.card()->writes() == 1);
  c.exi_select(0);
  c.send({0xF1, static_cast<u8>((page >> 17) & 0x7F), static_cast<u8>(page >> 9)});
  c.w(0xCC006800, 0x1);
  c.hw.advance(MemoryCard::kProgramTb + MemoryCard::kEraseTb);
  CHECK((c.r(0xCC006800) & 0x2) != 0);
  back = read(page - 0x80, 0x200);
  for (u8 b : back) CHECK(b == 0xFF);
  CHECK(c.hw.card()->writes() == 2);
}

TEST_CASE("a new memory card is formatted as the game formats one, and passes its checks") {
  const std::uint64_t time = 0x0000002F12345678ull;
  const auto img = MemoryCard::formatted(0x11223344u, 0, time);
  REQUIRE(img.size() == MemoryCard::kSize);
  // VerifyID: device 0, 64 Mbit, the checksums, the serial against the flash ID (zeros) from the format time.
  const u8* id = img.data();
  CHECK(be16(id + 0x20) == 0);
  CHECK(be16(id + 0x22) == 64);
  CHECK(be16(id + 0x24) == 0);
  u16 sum, inv;
  sdk_checksum(id, 0x1FC, &sum, &inv);
  CHECK(be16(id + 0x1FC) == sum);
  CHECK(be16(id + 0x1FE) == inv);
  std::int64_t rand = 0;
  for (int k = 0; k < 8; ++k) rand = static_cast<std::int64_t>((static_cast<std::uint64_t>(rand) << 8) | id[12 + k]);
  CHECK(static_cast<std::uint64_t>(rand) == time);
  for (int i = 0; i < 12; i++) {
    rand = static_cast<std::int64_t>(static_cast<std::uint64_t>(rand) * 1103515245u + 12345u) >> 16;
    CHECK(id[i] == static_cast<u8>(rand + 0));
    rand = (static_cast<std::int64_t>(static_cast<std::uint64_t>(rand) * 1103515245u + 12345u) >> 16) & 0x7FFF;
  }
  CHECK(be32(id + 20) == 0x11223344u);
  // VerifyDir: both directories empty, their checksums right, check codes 0 and 1.
  for (u32 i = 0; i < 2; ++i) {
    const u8* dir = img.data() + (1 + i) * 0x2000;
    sdk_checksum(dir, 0x2000 - 4, &sum, &inv);
    CHECK(be16(dir + 0x1FFC) == sum);
    CHECK(be16(dir + 0x1FFE) == inv);
    CHECK(be16(dir + 0x1FFA) == i);
    CHECK(dir[0] == 0xFF);  // no file
  }
  // VerifyFAT: the checksums, and 1019 free blocks counted and recorded.
  for (u32 i = 0; i < 2; ++i) {
    const u8* fat = img.data() + (3 + i) * 0x2000;
    sdk_checksum(fat + 4, 0x2000 - 4, &sum, &inv);
    CHECK(be16(fat + 0) == sum);
    CHECK(be16(fat + 2) == inv);
    CHECK(be16(fat + 4) == i);
    u32 free = 0;
    for (u32 b = 5; b < 1024; ++b) free += be16(fat + 2 * b) == 0;
    CHECK(free == 1019);
    CHECK(be16(fat + 6) == 1019);
  }
  CHECK(img[5 * 0x2000] == 0xFF);  // the files' blocks: erased
}

TEST_CASE("DSP: the init handshake of __OSInitAudioSystem and the ARAM DMA") {
  Rig g;
  for (u32 k = 0; k < 0x20; ++k) g.mem[0x01000000 + k] = static_cast<u8>(0xA0 + k);
  g.w(0xCC00500A, 0x8AC, 2);
  g.w(0xCC00500A, 0x8AD, 2);  // reset
  CHECK((g.r(0xCC00500A, 2) & 1) == 0);
  CHECK((g.r(0xCC005004, 2) & 0x8000) == 0);  // no mail yet
  g.w(0xCC005020, 0x01000000);                 // ARAM DMA: MEM1 0x01000000 -> ARAM 0, 0x20 bytes
  g.w(0xCC005024, 0);
  g.w(0xCC005028, 0x20);
  u16 cr = static_cast<u16>(g.r(0xCC00500A, 2));
  CHECK((cr & 0x20) != 0);  // ARINT
  g.w(0xCC00500A, cr, 2);  // acknowledge
  CHECK((g.r(0xCC00500A, 2) & 0x20) == 0);
  // DMA back from ARAM to MEM1 0x2000.
  g.w(0xCC005020, 0x2000);
  g.w(0xCC005024, 0);
  g.w(0xCC005028, 0x80000020u);
  CHECK(g.mem[0x2000] == 0xA0);
  CHECK(g.mem[0x201F] == 0xBF);
  // Unhalt: the init microcode's mail arrives.
  u16 c2 = static_cast<u16>(g.r(0xCC00500A, 2) & ~0x800 & ~0x4);
  g.w(0xCC00500A, c2, 2);
  u32 hi = g.r(0xCC005004, 2), lo = g.r(0xCC005006, 2);
  CHECK(((hi << 16) | lo) == 0x80544348u);
  CHECK((g.r(0xCC005004, 2) & 0x8000) == 0);  // taken
}

TEST_CASE("hardware log, write-gather pipe and the poll limit") {
  Rig g;
  // The write-gather pipe passes data on in 32-byte bursts.
  g.w(0xCC008000, 0x61, 1);
  g.w(0xCC008000, 0x12345678);
  CHECK(g.hw.fifo().empty());
  for (int k = 0; k < 6; ++k) g.w(0xCC008000, 0);
  for (int k = 0; k < 3; ++k) g.w(0xCC008000, 0, 1);
  REQUIRE(g.hw.fifo().size() == 32);
  CHECK(std::vector<u8>(g.hw.fifo().begin(), g.hw.fifo().begin() + 5) == std::vector<u8>{0x61, 0x12, 0x34, 0x56, 0x78});
  CHECK(g.hw.gx().bp_loads() == 1);
  g.w(0xCC003004, 0xF0);
  CHECK(g.r(0xCC003004) == 0xF0);
  CHECK(g.r(0xCC003006, 2) == 0xF0);
  g.hw.poll_limit = 10;
  for (int k = 0; k < 9; ++k) g.r(0xCC002000, 2);
  CHECK_THROWS_AS(g.r(0xCC002000, 2), StopRun);
  CHECK_THROWS_AS(g.r(0xCD000000), StopRun);
  CHECK(g.hw.log().size() > 10);
}

// V4: snapshots copy ARAM and the card's image by pages (Hardware::save_into,
// restore_from): after every save the snapshot holds what a whole copy would, and
// every restore brings back exactly the snapshot, through DMA writes, card programs
// and erases, two snapshots taken in turn, restores of either, and a snapshot read
// from a state file (no pages to compare with).
TEST_CASE("V4: snapshots by pages hold what whole copies hold") {
  CardRig c;
  c.hw.insert_card(MemoryCard::formatted(0, 0, 1));
  c.unlock_read(0x7FEC8000u, 4, 0);
  for (int k = 0; k < 3; ++k) c.unlock_read(0, 24, 1);
  REQUIRE((c.status() & 0x40) != 0);
  std::uint64_t tb = 0;
  std::uint32_t seed = 1;
  auto rnd = [&] {
    seed = seed * 1664525u + 1013904223u;
    return seed;
  };
  auto dma = [&](u32 ar, u32 n) {  // n bytes of fresh data to ARAM at `ar`
    for (u32 k = 0; k < n; ++k) c.mem[0x00100000 + k] = static_cast<u8>(rnd() >> 24);
    c.w(0xCC005020, 0x00100000);
    c.w(0xCC005024, ar);
    c.w(0xCC005028, n);
  };
  auto program = [&](u32 page) {  // a 128-byte card page program
    c.exi_select(0);
    c.send({0xF2, static_cast<u8>((page >> 17) & 0x7F), static_cast<u8>(page >> 9), static_cast<u8>((page >> 7) & 3),
            static_cast<u8>(page & 0x7F)});
    for (u32 k = 0; k < 128; k += 4) {
      const u32 v = rnd();
      c.send({static_cast<u8>(v), static_cast<u8>(v >> 8), static_cast<u8>(v >> 16), static_cast<u8>(v >> 24)});
    }
    c.exi_deselect();
    tb += MemoryCard::kProgramTb + MemoryCard::kEraseTb;
    c.hw.advance(tb);
  };
  auto erase = [&](u32 sector) {
    c.exi_select(0);
    c.send({0xF1, static_cast<u8>((sector >> 17) & 0x7F), static_cast<u8>(sector >> 9)});
    c.exi_deselect();
    tb += MemoryCard::kProgramTb + MemoryCard::kEraseTb;
    c.hw.advance(tb);
  };
  auto state = [](const Hardware& h) {
    StateWriter w;
    h.write_state(w);  // every device's state, ARAM included (not the card's contents)
    return w.out();
  };
  auto same = [&](const Hardware& a, const Hardware& b) {
    return state(a) == state(b) && a.card() && b.card() && a.card()->image() == b.card()->image();
  };
  dma(0x000000, 0x4000);
  dma(0x800020, 0x60);
  program(3 * 0x2000);
  auto s1 = c.hw.snapshot();
  CHECK(same(*s1, c.hw));
  dma(0x001000, 0x2000);  // within what s1 has
  program(3 * 0x2000 + 0x80);
  program(40 * 0x2000);
  auto s2 = c.hw.snapshot();
  CHECK(same(*s2, c.hw));
  dma(0x7FF000, 0x3000);
  erase(3 * 0x2000);
  c.hw.save_into(*s1);
  CHECK(same(*s1, c.hw));
  dma(0x000800, 0x100);
  program(41 * 0x2000);
  c.hw.save_into(*s2);
  CHECK(same(*s2, c.hw));
  const auto want1 = state(*s1);
  const auto card1 = s1->card()->image();
  dma(0x200000, 0x10000);
  program(1000 * 0x2000);
  c.hw.restore_from(*s1);
  CHECK(state(c.hw) == want1);
  CHECK(c.hw.card()->image() == card1);
  dma(0x800000, 0x1000);
  program(40 * 0x2000 + 0x100);
  c.hw.save_into(*s2);  // after a restore: what it brought back is new to s2
  CHECK(same(*s2, c.hw));
  c.hw.restore_from(*s2);
  CHECK(same(*s2, c.hw));
  c.hw.restore_from(*s1);
  CHECK(state(c.hw) == want1);
  CHECK(c.hw.card()->image() == card1);
  c.hw.save_into(*s1);
  CHECK(same(*s1, c.hw));
  // A snapshot read from a state file: ARAM from the file, then a whole copy back.
  dma(0x400000, 0x800);
  const auto file_state = state(c.hw);
  dma(0x400000, 0x800);
  auto f = c.hw.snapshot();
  StateReader r(file_state);
  f->read_state(r);
  c.hw.restore_from(*f);
  CHECK(state(c.hw) == file_state);
  c.hw.save_into(*s1);
  CHECK(same(*s1, c.hw));
}
