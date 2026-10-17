// Instruction vectors for the semantics library and the reference interpreter.
// Each expected value is worked out from the PowerPC / Gekko manuals; the last
// test case checks that every opcode has at least one vector here.
#include <doctest/doctest.h>

#include <cstring>
#include <string>

#include "encode.h"
#include "machine.h"
#include "ppc/fp.h"

using namespace mvp::ppc;
using namespace mvp::ppc::enc;
using mvp::ppc::test::Machine;

namespace {
u64 dbits(double d) {
  u64 b;
  std::memcpy(&b, &d, 8);
  return b;
}
u64 sbits(float f) {  // a single as FPR (double) bits
  u32 b;
  std::memcpy(&b, &f, 4);
  return fp::single_to_double(b);
}
constexpr u32 CR0(u32 bits) { return bits << 28; }
}  // namespace

// ---------------------------------------------------------------- integer

TEST_CASE("add family: carry, overflow, record") {
  Machine m;
  auto& c = m.c;
  c.gpr[4] = 5;
  c.gpr[5] = 7;
  m.run(XO(3, 4, 5, 0, 266));
  CHECK(c.gpr[3] == 12);
  // add. negative result: CR0 = LT, plus SO copied from XER.
  c.gpr[4] = 0xFFFFFFF0;
  c.xer_so = 1;
  m.run(XO(3, 4, 5, 0, 266, 1));
  CHECK(c.gpr[3] == 0xFFFFFFF7);
  CHECK(c.cr == CR0(0x9));
  // addo.: signed overflow sets OV and SO, and CR0 sees the new SO.
  c.xer_so = 0;
  c.gpr[4] = 0x7FFFFFFF;
  c.gpr[5] = 1;
  m.run(XO(3, 4, 5, 1, 266, 1));
  CHECK(c.gpr[3] == 0x80000000);
  CHECK(c.xer_ov == 1);
  CHECK(c.xer_so == 1);
  CHECK(c.cr == CR0(0x9));
  // addo without overflow clears OV, keeps SO.
  c.gpr[4] = 1;
  m.run(XO(3, 4, 5, 1, 266));
  CHECK(c.xer_ov == 0);
  CHECK(c.xer_so == 1);
  // addc
  c.gpr[4] = 0xFFFFFFFF;
  c.gpr[5] = 1;
  m.run(XO(3, 4, 5, 0, 10));
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ca == 1);
  c.gpr[4] = 1;
  m.run(XO(3, 4, 5, 0, 10));
  CHECK(c.gpr[3] == 2);
  CHECK(c.xer_ca == 0);
  // adde: 0xFFFFFFFF + 0 + CA
  c.gpr[4] = 0xFFFFFFFF;
  c.gpr[5] = 0;
  c.xer_ca = 1;
  m.run(XO(3, 4, 5, 0, 138));
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ca == 1);
  // addze
  c.gpr[4] = 5;
  c.xer_ca = 1;
  m.run(XO(3, 4, 0, 0, 202));
  CHECK(c.gpr[3] == 6);
  CHECK(c.xer_ca == 0);
  c.gpr[4] = 0xFFFFFFFF;
  c.xer_ca = 1;
  m.run(XO(3, 4, 0, 0, 202));
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ca == 1);
  // addme: rA + CA - 1
  c.gpr[4] = 5;
  c.xer_ca = 0;
  m.run(XO(3, 4, 0, 0, 234));
  CHECK(c.gpr[3] == 4);
  CHECK(c.xer_ca == 1);
  c.gpr[4] = 0;
  c.xer_ca = 0;
  m.run(XO(3, 4, 0, 0, 234));
  CHECK(c.gpr[3] == 0xFFFFFFFF);
  CHECK(c.xer_ca == 0);
}

TEST_CASE("immediate adds") {
  Machine m;
  auto& c = m.c;
  c.gpr[0] = 1000;  // rA = 0 means literal zero
  m.run(D(14, 3, 0, -1));
  CHECK(c.gpr[3] == 0xFFFFFFFF);
  c.gpr[4] = 0x100;
  m.run(D(14, 3, 4, -0x10));
  CHECK(c.gpr[3] == 0xF0);
  m.run(D(15, 3, 0, 0x8070));
  CHECK(c.gpr[3] == 0x80700000);
  c.gpr[4] = 0x10;
  m.run(D(15, 3, 4, -1));
  CHECK(c.gpr[3] == 0xFFFF0010);
  // addic: carry out
  c.gpr[4] = 0xFFFFFFFF;
  m.run(D(12, 3, 4, 1));
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ca == 1);
  c.gpr[4] = 5;
  m.run(D(12, 3, 4, -1));
  CHECK(c.gpr[3] == 4);
  CHECK(c.xer_ca == 1);  // 5 + 0xFFFFFFFF carries
  // addic.: records
  c.gpr[4] = 1;
  c.xer_so = 0;
  m.run(D(13, 3, 4, -1));
  CHECK(c.gpr[3] == 0);
  CHECK(c.cr == CR0(0x2));
  // subfic: simm - rA
  c.gpr[4] = 5;
  m.run(D(8, 3, 4, 0));
  CHECK(c.gpr[3] == 0xFFFFFFFB);
  CHECK(c.xer_ca == 0);
  c.gpr[4] = 0;
  m.run(D(8, 3, 4, 0));
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ca == 1);
  // mulli
  c.gpr[4] = 0xFFFFFFFD;
  m.run(D(7, 3, 4, 7));
  CHECK(c.gpr[3] == static_cast<u32>(-21));
}

TEST_CASE("subtract family") {
  Machine m;
  auto& c = m.c;
  c.gpr[4] = 3;
  c.gpr[5] = 10;
  m.run(XO(3, 4, 5, 0, 40));  // subf r3 = r5 - r4
  CHECK(c.gpr[3] == 7);
  c.gpr[4] = 1;
  c.gpr[5] = 0x80000000;
  m.run(XO(3, 4, 5, 1, 40));  // subfo overflow
  CHECK(c.gpr[3] == 0x7FFFFFFF);
  CHECK(c.xer_ov == 1);
  c.set_xer(0);
  c.gpr[4] = 3;
  c.gpr[5] = 10;
  m.run(XO(3, 4, 5, 0, 8));  // subfc: no borrow -> CA = 1
  CHECK(c.gpr[3] == 7);
  CHECK(c.xer_ca == 1);
  c.gpr[4] = 10;
  c.gpr[5] = 3;
  m.run(XO(3, 4, 5, 0, 8));
  CHECK(c.gpr[3] == 0xFFFFFFF9);
  CHECK(c.xer_ca == 0);
  c.gpr[4] = 3;
  c.gpr[5] = 10;
  c.xer_ca = 1;
  m.run(XO(3, 4, 5, 0, 136));  // subfe: ~a + b + CA
  CHECK(c.gpr[3] == 7);
  CHECK(c.xer_ca == 1);
  c.xer_ca = 0;
  m.run(XO(3, 4, 5, 0, 136));
  CHECK(c.gpr[3] == 6);
  c.gpr[4] = 0;
  c.xer_ca = 1;
  m.run(XO(3, 4, 0, 0, 200));  // subfze: ~a + CA
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ca == 1);
  c.gpr[4] = 5;
  c.xer_ca = 0;
  m.run(XO(3, 4, 0, 0, 200));
  CHECK(c.gpr[3] == 0xFFFFFFFA);
  CHECK(c.xer_ca == 0);
  c.gpr[4] = 0;
  c.xer_ca = 1;
  m.run(XO(3, 4, 0, 0, 232));  // subfme: ~a + CA - 1
  CHECK(c.gpr[3] == 0xFFFFFFFF);
  CHECK(c.xer_ca == 1);
  c.gpr[4] = 5;
  m.run(XO(3, 4, 0, 0, 104));  // neg
  CHECK(c.gpr[3] == static_cast<u32>(-5));
  c.set_xer(0);
  c.gpr[4] = 0x80000000;
  m.run(XO(3, 4, 0, 1, 104, 1));  // nego.
  CHECK(c.gpr[3] == 0x80000000);
  CHECK(c.xer_ov == 1);
  CHECK(c.cr == CR0(0x9));
}

TEST_CASE("multiply and divide") {
  Machine m;
  auto& c = m.c;
  c.gpr[4] = 0x10000;
  c.gpr[5] = 0x10000;
  m.run(XO(3, 4, 5, 1, 235));  // mullwo
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ov == 1);
  c.set_xer(0);
  c.gpr[4] = static_cast<u32>(-2);
  c.gpr[5] = 3;
  m.run(XO(3, 4, 5, 1, 235, 1));
  CHECK(c.gpr[3] == static_cast<u32>(-6));
  CHECK(c.xer_ov == 0);
  CHECK(c.cr == CR0(0x8));
  c.gpr[4] = 0x10000;
  c.gpr[5] = 0x10000;
  m.run(XO(3, 4, 5, 0, 75));  // mulhw
  CHECK(c.gpr[3] == 1);
  c.gpr[4] = 0xFFFFFFFF;
  c.gpr[5] = 1;
  m.run(XO(3, 4, 5, 0, 75));
  CHECK(c.gpr[3] == 0xFFFFFFFF);
  c.gpr[5] = 2;
  m.run(XO(3, 4, 5, 0, 11));  // mulhwu
  CHECK(c.gpr[3] == 1);
  c.gpr[4] = static_cast<u32>(-7);
  c.gpr[5] = 2;
  m.run(XO(3, 4, 5, 0, 491));  // divw truncates toward zero
  CHECK(c.gpr[3] == static_cast<u32>(-3));
  c.gpr[4] = 5;
  c.gpr[5] = 0;
  m.run(XO(3, 4, 5, 1, 491));  // divwo by zero
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ov == 1);
  c.gpr[4] = static_cast<u32>(-5);
  m.run(XO(3, 4, 5, 0, 491));
  CHECK(c.gpr[3] == 0xFFFFFFFF);
  c.gpr[4] = 0x80000000;
  c.gpr[5] = 0xFFFFFFFF;
  m.run(XO(3, 4, 5, 0, 491));
  CHECK(c.gpr[3] == 0xFFFFFFFF);
  c.gpr[4] = 7;
  c.gpr[5] = 2;
  m.run(XO(3, 4, 5, 0, 459));  // divwu
  CHECK(c.gpr[3] == 3);
  c.set_xer(0);
  c.gpr[5] = 0;
  m.run(XO(3, 4, 5, 1, 459));
  CHECK(c.gpr[3] == 0);
  CHECK(c.xer_ov == 1);
}

TEST_CASE("compares") {
  Machine m;
  auto& c = m.c;
  c.gpr[3] = static_cast<u32>(-1);
  c.gpr[4] = 1;
  m.run(X(31, 0, 3, 4, 0));  // cmpw
  CHECK(c.cr == CR0(0x8));
  m.run(X(31, 1 << 2, 3, 4, 32));  // cmplw cr1
  CHECK(c.cr == (CR0(0x8) | 0x04000000));
  c.gpr[6] = static_cast<u32>(-3);
  m.run(D(11, 7 << 2, 6, -3));  // cmpwi cr7
  CHECK((c.cr & 0xF) == 0x2);
  c.xer_so = 1;
  c.gpr[3] = 1;
  m.run(D(10, 0, 3, 2));  // cmplwi: LT plus SO
  CHECK((c.cr >> 28) == 0x9);
}

TEST_CASE("logical") {
  Machine m;
  auto& c = m.c;
  c.gpr[4] = 0xF0F0F0F0;
  c.gpr[5] = 0xFF00FF00;
  m.run(X(31, 4, 3, 5, 28));
  CHECK(c.gpr[3] == 0xF000F000);  // and
  m.run(X(31, 4, 3, 5, 60));
  CHECK(c.gpr[3] == 0x00F000F0);  // andc
  m.run(X(31, 4, 3, 5, 444));
  CHECK(c.gpr[3] == 0xFFF0FFF0);  // or
  m.run(X(31, 4, 3, 5, 412));
  CHECK(c.gpr[3] == 0xF0FFF0FF);  // orc
  m.run(X(31, 4, 3, 5, 316));
  CHECK(c.gpr[3] == 0x0FF00FF0);  // xor
  m.run(X(31, 4, 3, 5, 476));
  CHECK(c.gpr[3] == 0x0FFF0FFF);  // nand
  m.run(X(31, 4, 3, 5, 124));
  CHECK(c.gpr[3] == 0x000F000F);  // nor
  m.run(X(31, 4, 3, 5, 284));
  CHECK(c.gpr[3] == 0xF00FF00F);  // eqv
  c.gpr[4] = 0x80;
  m.run(X(31, 4, 3, 0, 954));
  CHECK(c.gpr[3] == 0xFFFFFF80);  // extsb
  c.gpr[4] = 0x12348000;
  m.run(X(31, 4, 3, 0, 922, 1));
  CHECK(c.gpr[3] == 0xFFFF8000);  // extsh.
  CHECK((c.cr >> 28) == 0x8);
  c.gpr[4] = 0x00010000;
  m.run(X(31, 4, 3, 0, 26));
  CHECK(c.gpr[3] == 15);  // cntlzw
  c.gpr[4] = 0;
  m.run(X(31, 4, 3, 0, 26));
  CHECK(c.gpr[3] == 32);
  c.gpr[4] = 0x12340000;
  m.run(D(24, 4, 3, 0x5678));
  CHECK(c.gpr[3] == 0x12345678);  // ori
  m.run(D(25, 4, 3, 0x0001));
  CHECK(c.gpr[3] == 0x12350000);  // oris
  m.run(D(26, 4, 3, 0xFFFF));
  CHECK(c.gpr[3] == 0x1234FFFF);  // xori
  m.run(D(27, 4, 3, 0x1234));
  CHECK(c.gpr[3] == 0x00000000);  // xoris
  c.xer_so = 0;
  m.run(D(28, 4, 3, 0x0F0F));
  CHECK(c.gpr[3] == 0);  // andi.
  CHECK((c.cr >> 28) == 0x2);
  m.run(D(29, 4, 3, 0x1000));
  CHECK(c.gpr[3] == 0x10000000);  // andis.
  CHECK((c.cr >> 28) == 0x4);
}

TEST_CASE("rotate and shift") {
  Machine m;
  auto& c = m.c;
  c.gpr[4] = 0x12345678;
  m.run(M(21, 4, 3, 8, 24, 31));  // rlwinm: rotate 8, keep low byte
  CHECK(c.gpr[3] == 0x12);
  m.run(M(21, 4, 3, 4, 0, 27));  // slwi 4
  CHECK(c.gpr[3] == 0x23456780);
  m.run(M(21, 4, 3, 30, 2, 31));  // srwi 2
  CHECK(c.gpr[3] == 0x048D159E);
  c.gpr[5] = 36;  // rlwnm uses the low 5 bits: rotate by 4
  m.run(M(23, 4, 3, 5, 0, 31));
  CHECK(c.gpr[3] == 0x23456781);
  c.gpr[3] = 0xFFFFFFFF;
  m.run(M(20, 4, 3, 16, 8, 15));  // rlwimi: insert rotated byte into bits 8-15
  CHECK(c.gpr[3] == 0xFF78FFFF);  // rotl 16 = 0x56781234; bits 8-15 = 0x78
  c.gpr[5] = 4;
  m.run(X(31, 4, 3, 5, 24));  // slw
  CHECK(c.gpr[3] == 0x23456780);
  c.gpr[5] = 32;
  m.run(X(31, 4, 3, 5, 24));
  CHECK(c.gpr[3] == 0);
  c.gpr[5] = 4;
  m.run(X(31, 4, 3, 5, 536));  // srw
  CHECK(c.gpr[3] == 0x01234567);
  c.gpr[5] = 63;
  m.run(X(31, 4, 3, 5, 536));
  CHECK(c.gpr[3] == 0);
  c.gpr[4] = static_cast<u32>(-5);
  c.gpr[5] = 1;
  m.run(X(31, 4, 3, 5, 792));  // sraw: -5 >> 1 = -3, a 1 bit shifted out: CA
  CHECK(c.gpr[3] == static_cast<u32>(-3));
  CHECK(c.xer_ca == 1);
  c.gpr[4] = static_cast<u32>(-4);
  m.run(X(31, 4, 3, 5, 792));
  CHECK(c.gpr[3] == static_cast<u32>(-2));
  CHECK(c.xer_ca == 0);
  c.gpr[5] = 40;
  m.run(X(31, 4, 3, 5, 792));
  CHECK(c.gpr[3] == 0xFFFFFFFF);
  CHECK(c.xer_ca == 1);
  c.gpr[4] = 4;
  m.run(X(31, 4, 3, 1, 824, 1));  // srawi. 1
  CHECK(c.gpr[3] == 2);
  CHECK(c.xer_ca == 0);
  CHECK((c.cr >> 28) == 0x4);
  c.gpr[4] = 0xFFFFFFFF;
  m.run(X(31, 4, 3, 1, 824));
  CHECK(c.gpr[3] == 0xFFFFFFFF);
  CHECK(c.xer_ca == 1);
}

// ---------------------------------------------------------------- CR, SPR, system

TEST_CASE("condition register logic") {
  Machine m;
  auto& c = m.c;
  c.cr = 0xA0000000;  // bits 0 (lt) and 2 (eq) of cr0
  m.run(X(19, 1, 0, 2, 257));  // crand gt = lt & eq
  CHECK(c.cr == 0xE0000000);
  m.run(X(19, 3, 0, 1, 129));  // crandc un = lt & ~gt
  CHECK(c.cr == 0xE0000000);
  m.run(X(19, 3, 0, 4, 289));  // creqv un = ~(lt ^ cr1lt)
  CHECK(c.cr == 0xE0000000);
  m.run(X(19, 4, 0, 1, 225));  // crnand cr1lt = ~(lt & gt)
  CHECK(c.cr == 0xE0000000);
  m.run(X(19, 5, 3, 3, 33));  // crnot cr1gt = ~un
  CHECK(c.cr == 0xE4000000);
  m.run(X(19, 6, 3, 5, 449));  // cror cr1eq = un | cr1gt
  CHECK(c.cr == 0xE6000000);
  m.run(X(19, 7, 3, 4, 417));  // crorc cr1un = un | ~cr1lt
  CHECK(c.cr == 0xE7000000);
  m.run(X(19, 7, 0, 1, 193));  // crxor cr1un = lt ^ gt
  CHECK(c.cr == 0xE6000000);
  c.cr = 0x12345678;
  m.run(X(19, 7 << 2, 0, 0, 0));  // mcrf cr7, cr0
  CHECK(c.cr == 0x12345671);
  c.set_xer(0xF000007F);
  m.run(X(31, 1 << 2, 0, 0, 512));  // mcrxr cr1: copies XER[0-3], then clears them
  CHECK(c.cr == 0x1F345671);
  CHECK(c.xer() == 0x0000007F);
  m.run(X(31, 3, 0, 0, 19));  // mfcr
  CHECK(c.gpr[3] == 0x1F345671);
  c.gpr[12] = 0xAAAAAAAA;
  m.run(31u << 26 | 12u << 21 | 0x81u << 12 | 144u << 1);  // mtcrf cr0 and cr7
  CHECK(c.cr == 0xAF34567A);
}

TEST_CASE("special registers") {
  Machine m;
  auto& c = m.c;
  c.gpr[3] = 0x80003000;
  m.run(SPR(467, 3, 8));  // mtlr
  CHECK(c.lr == 0x80003000);
  m.run(SPR(339, 4, 8));  // mflr
  CHECK(c.gpr[4] == 0x80003000);
  c.gpr[3] = 7;
  m.run(SPR(467, 3, 9));
  CHECK(c.ctr == 7);
  c.gpr[3] = 0xE000007F;
  m.run(SPR(467, 3, 1));  // mtxer
  CHECK(c.xer_so == 1);
  CHECK(c.xer_ov == 1);
  CHECK(c.xer_ca == 1);
  m.run(SPR(339, 5, 1));
  CHECK(c.gpr[5] == 0xE000007F);
  c.gpr[3] = 0x00070007;
  m.run(SPR(467, 3, 914));  // mtspr GQR2
  CHECK(c.spr[914] == 0x00070007);
  m.run(SPR(339, 6, 914));
  CHECK(c.gpr[6] == 0x00070007);
  m.hooks.tb = 0x0000000500000009ull;
  m.run(SPR(371, 7, 268));  // mftb (TBL)
  CHECK(c.gpr[7] == 9);
  m.run(SPR(371, 7, 269));  // mftbu
  CHECK(c.gpr[7] == 5);
  m.run(SPR(339, 7, 285));  // mfspr TBU via 285
  CHECK(c.gpr[7] == 5);
  c.gpr[3] = 0x1234;
  m.run(SPR(467, 3, 284));  // mtspr TBL
  CHECK(m.hooks.tb == 0x0000000500001234ull);
  m.run(SPR(467, 3, 22));  // mtdec
  CHECK(m.hooks.dec == 0x1234);
  m.hooks.dec = 99;
  m.run(SPR(339, 8, 22));  // mfdec
  CHECK(c.gpr[8] == 99);
  c.gpr[3] = 0x0C008000;
  m.run(SPR(467, 3, 921));  // mtspr WPAR: the write-gather pipe restarts empty
  CHECK(c.spr[spr::WPAR] == 0x0C008000);
  CHECK(m.hooks.pipe_resets == 1);
  m.run(SPR(339, 8, 921));  // mfspr WPAR: the buffer reads empty (BNE clear)
  CHECK(c.gpr[8] == 0x0C008000);
  c.msr = 0x2032;
  c.gpr[3] = 0xA032;
  m.run(X(31, 3, 0, 0, 146));  // mtmsr
  CHECK(c.msr == 0xA032);
  CHECK(m.hooks.last_old_msr == 0x2032);
  m.run(X(31, 9, 0, 0, 83));  // mfmsr
  CHECK(c.gpr[9] == 0xA032);
  c.gpr[3] = 0x80000001;
  m.run(X(31, 3, 7, 0, 210));  // mtsr 7
  CHECK(c.sr[7] == 0x80000001);
  m.run(X(31, 4, 7, 0, 595));  // mfsr
  CHECK(c.gpr[4] == 0x80000001);
  c.gpr[5] = 0x30000000;
  m.run(X(31, 3, 0, 5, 242));  // mtsrin (sr[3])
  CHECK(c.sr[3] == 0x80000001);
  m.run(X(31, 6, 0, 5, 659));  // mfsrin
  CHECK(c.gpr[6] == 0x80000001);
}

TEST_CASE("cache, sync and TLB instructions") {
  Machine m;
  auto& c = m.c;
  for (u32 k = 0; k < 64; ++k) m.w8(0x80002000 + k, 0xEE);
  c.gpr[3] = 0x80002011;
  m.run(X(31, 0, 0, 3, 1014));  // dcbz: zero the aligned 32-byte block
  CHECK(m.r32(0x80002000) == 0);
  CHECK(m.r32(0x8000201C) == 0);
  CHECK(m.r8(0x80002020) == 0xEE);
  m.run(X(4, 0, 0, 3, 1014));  // dcbz_l (locked cache, plain RAM here)
  CHECK(m.r32(0x80002000) == 0);
  m.run(X(31, 0, 0, 3, 982));  // icbi
  CHECK(m.hooks.icbis.back() == 0x80002011);
  u32 before = c.gpr[3];
  for (u32 xo : {86u, 470u, 54u, 278u, 246u}) m.run(X(31, 0, 0, 3, xo));  // dcbf dcbi dcbst dcbt dcbtst
  m.run(X(31, 0, 0, 0, 598));                                           // sync
  m.run(X(31, 0, 0, 0, 854));                                           // eieio
  m.run(X(19, 0, 0, 0, 150));                                           // isync
  m.run(X(31, 0, 0, 3, 306));                                           // tlbie
  m.run(X(31, 0, 0, 0, 566));                                           // tlbsync
  CHECK(c.gpr[3] == before);
  CHECK(m.r8(0x80002020) == 0xEE);
}

// ---------------------------------------------------------------- loads and stores

TEST_CASE("integer loads") {
  Machine m;
  auto& c = m.c;
  m.w32(0x80002000, 0x89ABCDEF);
  c.gpr[4] = 0x80002000;
  c.gpr[5] = 2;
  m.run(D(34, 3, 4, 1));  // lbz
  CHECK(c.gpr[3] == 0xAB);
  m.run(X(31, 3, 4, 5, 87));  // lbzx
  CHECK(c.gpr[3] == 0xCD);
  m.run(D(35, 3, 4, 3));  // lbzu
  CHECK(c.gpr[3] == 0xEF);
  CHECK(c.gpr[4] == 0x80002003);
  c.gpr[4] = 0x80002000;
  c.gpr[5] = 1;
  m.run(X(31, 3, 4, 5, 119));  // lbzux
  CHECK(c.gpr[3] == 0xAB);
  CHECK(c.gpr[4] == 0x80002001);
  c.gpr[4] = 0x80002000;
  c.gpr[5] = 2;
  m.run(D(40, 3, 4, 0));  // lhz
  CHECK(c.gpr[3] == 0x89AB);
  m.run(X(31, 3, 4, 5, 279));  // lhzx
  CHECK(c.gpr[3] == 0xCDEF);
  m.run(D(41, 3, 4, 2));  // lhzu
  CHECK(c.gpr[3] == 0xCDEF);
  CHECK(c.gpr[4] == 0x80002002);
  m.run(X(31, 3, 4, 5, 311));  // lhzux (r4 = 0x80002002 + 2 is past the word: reads 0)
  CHECK(c.gpr[3] == 0);
  CHECK(c.gpr[4] == 0x80002004);
  c.gpr[4] = 0x80002000;
  m.run(D(42, 3, 4, 0));  // lha sign-extends
  CHECK(c.gpr[3] == 0xFFFF89AB);
  m.run(X(31, 3, 4, 5, 343));  // lhax
  CHECK(c.gpr[3] == 0xFFFFCDEF);
  m.run(D(43, 3, 4, 2));  // lhau
  CHECK(c.gpr[3] == 0xFFFFCDEF);
  CHECK(c.gpr[4] == 0x80002002);
  c.gpr[4] = 0x80002000;
  m.run(X(31, 3, 4, 5, 375));  // lhaux
  CHECK(c.gpr[3] == 0xFFFFCDEF);
  CHECK(c.gpr[4] == 0x80002002);
  c.gpr[4] = 0x80002000;
  c.gpr[5] = 0;
  m.run(X(31, 3, 4, 5, 790));  // lhbrx
  CHECK(c.gpr[3] == 0xAB89);
  m.run(D(32, 3, 4, 0));  // lwz
  CHECK(c.gpr[3] == 0x89ABCDEF);
  m.run(X(31, 3, 4, 5, 23));  // lwzx
  CHECK(c.gpr[3] == 0x89ABCDEF);
  m.run(X(31, 3, 4, 5, 534));  // lwbrx
  CHECK(c.gpr[3] == 0xEFCDAB89);
  m.w32(0x80002004, 0x11223344);
  m.run(D(33, 3, 4, 4));  // lwzu
  CHECK(c.gpr[3] == 0x11223344);
  CHECK(c.gpr[4] == 0x80002004);
  c.gpr[5] = static_cast<u32>(-4);
  m.run(X(31, 3, 4, 5, 55));  // lwzux
  CHECK(c.gpr[3] == 0x89ABCDEF);
  CHECK(c.gpr[4] == 0x80002000);
  // Uncached mirror reads the same RAM.
  c.gpr[6] = 0xC0002000;
  m.run(D(32, 3, 6, 0));
  CHECK(c.gpr[3] == 0x89ABCDEF);
  // Unaligned word.
  m.run(D(32, 3, 4, 1));
  CHECK(c.gpr[3] == 0xABCDEF11);
}

TEST_CASE("integer stores") {
  Machine m;
  auto& c = m.c;
  c.gpr[3] = 0x11223344;
  c.gpr[4] = 0x80002000;
  c.gpr[5] = 4;
  m.run(D(38, 3, 4, 0));  // stb
  CHECK(m.r8(0x80002000) == 0x44);
  m.run(X(31, 3, 4, 5, 215));  // stbx
  CHECK(m.r8(0x80002004) == 0x44);
  m.run(D(39, 3, 4, 1));  // stbu
  CHECK(m.r8(0x80002001) == 0x44);
  CHECK(c.gpr[4] == 0x80002001);
  m.run(X(31, 3, 4, 5, 247));  // stbux
  CHECK(m.r8(0x80002005) == 0x44);
  CHECK(c.gpr[4] == 0x80002005);
  c.gpr[4] = 0x80002010;
  m.run(D(44, 3, 4, 0));  // sth
  CHECK(m.r16(0x80002010) == 0x3344);
  m.run(X(31, 3, 4, 5, 407));  // sthx
  CHECK(m.r16(0x80002014) == 0x3344);
  m.run(D(45, 3, 4, 2));  // sthu
  CHECK(c.gpr[4] == 0x80002012);
  m.run(X(31, 3, 4, 5, 439));  // sthux
  CHECK(c.gpr[4] == 0x80002016);
  CHECK(m.r16(0x80002016) == 0x3344);
  c.gpr[0] = 0;
  m.run(X(31, 3, 4, 0, 918));  // sthbrx (rB = r0 = 0)
  CHECK(m.r16(0x80002016) == 0x4433);
  c.gpr[4] = 0x80002020;
  m.run(D(36, 3, 4, 0));  // stw
  CHECK(m.r32(0x80002020) == 0x11223344);
  m.run(X(31, 3, 4, 5, 151));  // stwx
  CHECK(m.r32(0x80002024) == 0x11223344);
  m.run(X(31, 3, 4, 5, 662));  // stwbrx
  CHECK(m.r32(0x80002024) == 0x44332211);
  m.run(D(37, 3, 4, 8));  // stwu
  CHECK(c.gpr[4] == 0x80002028);
  CHECK(m.r32(0x80002028) == 0x11223344);
  m.run(X(31, 3, 4, 5, 183));  // stwux
  CHECK(c.gpr[4] == 0x8000202C);
  CHECK(m.r32(0x8000202C) == 0x11223344);
}

TEST_CASE("multiple, string and reservation") {
  Machine m;
  auto& c = m.c;
  for (u32 r = 29; r < 32; ++r) c.gpr[r] = 0x100 + r;
  c.gpr[1] = 0x80003000;
  m.run(D(47, 29, 1, 0x10));  // stmw r29
  CHECK(m.r32(0x80003010) == 0x11D);
  CHECK(m.r32(0x80003018) == 0x11F);
  for (u32 r = 29; r < 32; ++r) c.gpr[r] = 0;
  m.run(D(46, 30, 1, 0x14));  // lmw r30
  CHECK(c.gpr[29] == 0);
  CHECK(c.gpr[30] == 0x11E);
  CHECK(c.gpr[31] == 0x11F);
  // lswi / stswi: 6 bytes into r5, r6 (r6 left-justified, zero padded).
  for (u32 k = 0; k < 8; ++k) m.w8(0x80003100 + k, 0xA0 + k);
  c.gpr[4] = 0x80003100;
  m.run(X(31, 5, 4, 6, 597));  // lswi r5, r4, 6
  CHECK(c.gpr[5] == 0xA0A1A2A3);
  CHECK(c.gpr[6] == 0xA4A50000);
  c.gpr[7] = 0x80003200;
  m.run(X(31, 5, 7, 6, 725));  // stswi r5, r7, 6
  CHECK(m.r32(0x80003200) == 0xA0A1A2A3);
  CHECK(m.r16(0x80003204) == 0xA4A5);
  CHECK(m.r16(0x80003206) == 0);
  c.xer_low = 5;
  c.gpr[8] = 0;
  m.run(X(31, 5, 4, 8, 533));  // lswx r5, r4, r8 (5 bytes)
  CHECK(c.gpr[5] == 0xA0A1A2A3);
  CHECK(c.gpr[6] == 0xA4000000);
  c.gpr[9] = 0x80003300;
  m.run(X(31, 5, 9, 8, 661));  // stswx
  CHECK(m.r32(0x80003300) == 0xA0A1A2A3);
  CHECK(m.r8(0x80003304) == 0xA4);
  CHECK(m.r8(0x80003305) == 0);
  // lwarx / stwcx.
  m.w32(0x80003400, 5);
  c.gpr[4] = 0x80003400;
  c.gpr[0] = 0;
  m.run(X(31, 3, 0, 4, 20));  // lwarx r3, 0, r4
  CHECK(c.gpr[3] == 5);
  c.gpr[3] = 6;
  c.xer_so = 0;
  m.run(X(31, 3, 0, 4, 150, 1));  // stwcx. succeeds
  CHECK(m.r32(0x80003400) == 6);
  CHECK((c.cr >> 28) == 0x2);
  c.gpr[3] = 7;
  m.run(X(31, 3, 0, 4, 150, 1));  // no reservation: fails
  CHECK(m.r32(0x80003400) == 6);
  CHECK((c.cr >> 28) == 0x0);
}

TEST_CASE("floating-point loads and stores") {
  Machine m;
  auto& c = m.c;
  m.w32(0x80002000, 0x3F800000);  // 1.0f
  m.w32(0x80002004, 0x00000001);  // smallest denormal single
  m.w64(0x80002008, dbits(2.5));
  c.gpr[4] = 0x80002000;
  c.gpr[5] = 4;
  c.fpr[1].ps1 = 0x1234;
  m.run(D(48, 1, 4, 0));  // lfs fills both halves
  CHECK(c.fpr[1].ps0 == dbits(1.0));
  CHECK(c.fpr[1].ps1 == dbits(1.0));
  m.run(X(31, 2, 4, 5, 535));  // lfsx: denormal single becomes a normal double
  CHECK(c.fpr[2].ps0 == 0x36A0000000000000ull);
  m.run(D(49, 3, 4, 4));  // lfsu
  CHECK(c.gpr[4] == 0x80002004);
  CHECK(c.fpr[3].ps0 == 0x36A0000000000000ull);
  m.run(X(31, 3, 4, 5, 567));  // lfsux
  CHECK(c.gpr[4] == 0x80002008);
  CHECK(c.fpr[3].ps0 == 0x4000800000000000ull);  // the high word of 2.5 read as a single: 2.0625
  c.gpr[4] = 0x80002000;
  c.fpr[6].ps1 = 0x55;
  m.run(D(50, 6, 4, 8));  // lfd: ps0 only
  CHECK(c.fpr[6].ps0 == dbits(2.5));
  CHECK(c.fpr[6].ps1 == 0x55);
  c.gpr[5] = 8;
  m.run(X(31, 7, 4, 5, 599));  // lfdx
  CHECK(c.fpr[7].ps0 == dbits(2.5));
  m.run(D(51, 7, 4, 8));  // lfdu
  CHECK(c.gpr[4] == 0x80002008);
  c.gpr[5] = 0;
  m.run(X(31, 8, 4, 5, 631));  // lfdux
  CHECK(c.fpr[8].ps0 == dbits(2.5));
  // Stores.
  c.gpr[4] = 0x80002100;
  c.gpr[5] = 4;
  m.run(D(52, 1, 4, 0));  // stfs 1.0
  CHECK(m.r32(0x80002100) == 0x3F800000);
  m.run(X(31, 2, 4, 5, 663));  // stfsx: denormalizes 2^-149
  CHECK(m.r32(0x80002104) == 0x00000001);
  m.run(D(53, 1, 4, 8));  // stfsu
  CHECK(c.gpr[4] == 0x80002108);
  m.run(X(31, 1, 4, 5, 695));  // stfsux
  CHECK(c.gpr[4] == 0x8000210C);
  CHECK(m.r32(0x8000210C) == 0x3F800000);
  m.run(D(54, 6, 4, 4));  // stfd
  CHECK(m.r64(0x80002110) == dbits(2.5));
  m.run(X(31, 6, 4, 5, 727));  // stfdx
  CHECK(m.r64(0x80002110) == dbits(2.5));
  m.run(D(55, 6, 4, 0x14));  // stfdu
  CHECK(c.gpr[4] == 0x80002120);
  m.run(X(31, 6, 4, 5, 759));  // stfdux
  CHECK(c.gpr[4] == 0x80002124);
  CHECK(m.r64(0x80002124) == dbits(2.5));
  c.fpr[9].ps0 = 0xFFF80000DEADBEEFull;
  m.run(X(31, 9, 4, 5, 983));  // stfiwx: low word
  CHECK(m.r32(0x80002128) == 0xDEADBEEF);
}

// ---------------------------------------------------------------- floating point

TEST_CASE("fp arithmetic: double and single") {
  Machine m;
  auto& c = m.c;
  c.fpr[1].ps0 = dbits(1.5);
  c.fpr[2].ps0 = dbits(2.25);
  c.fpr[3].ps0 = dbits(3.0);
  m.run(A(63, 4, 1, 2, 0, 21));  // fadd
  CHECK(c.fpr[4].ps0 == dbits(3.75));
  m.run(A(63, 4, 1, 2, 0, 20));  // fsub
  CHECK(c.fpr[4].ps0 == dbits(-0.75));
  m.run(A(63, 4, 1, 0, 3, 25));  // fmul
  CHECK(c.fpr[4].ps0 == dbits(4.5));
  c.fpr[5].ps0 = dbits(1.0);
  m.run(A(63, 4, 5, 3, 0, 18));  // fdiv 1/3
  CHECK(c.fpr[4].ps0 == 0x3FD5555555555555ull);
  m.run(A(59, 4, 5, 3, 0, 18));  // fdivs 1/3: single, both halves
  CHECK(c.fpr[4].ps0 == sbits(1.0f / 3.0f));
  CHECK(c.fpr[4].ps1 == sbits(1.0f / 3.0f));
  // fadds rounds once to single: 1 + 2^-24 is a tie -> even (1.0).
  c.fpr[6].ps0 = dbits(0x1p-24);
  m.run(A(59, 4, 5, 6, 0, 21));
  CHECK(c.fpr[4].ps0 == dbits(1.0));
  c.fpr[6].ps0 = dbits(0x1p-24 + 0x1p-40);
  m.run(A(59, 4, 5, 6, 0, 21));
  CHECK(c.fpr[4].ps0 == dbits(1.0 + 0x1p-23));
  m.run(A(59, 4, 5, 6, 0, 20));  // fsubs
  CHECK(c.fpr[4].ps0 == dbits(1.0 - 0x1p-24));
  // fmuls rounds frC to a 25-bit significand first: (1+2^-23)*(1+2^-25 -> 1+2^-24).
  c.fpr[7].ps0 = dbits(1.0 + 0x1p-23);
  c.fpr[8].ps0 = dbits(1.0 + 0x1p-25);
  m.run(A(59, 4, 7, 0, 8, 25));
  CHECK(c.fpr[4].ps0 == dbits(1.0 + 0x1p-22));
  // Without the frC rounding (fmul, double) the product is exact.
  m.run(A(63, 4, 7, 0, 8, 25));
  CHECK(c.fpr[4].ps0 == dbits((1.0 + 0x1p-23) * (1.0 + 0x1p-25)));
}

TEST_CASE("fp fused multiply-add") {
  Machine m;
  auto& c = m.c;
  c.fpr[1].ps0 = dbits(2.0);
  c.fpr[2].ps0 = dbits(3.0);
  c.fpr[3].ps0 = dbits(0.5);
  m.run(A(63, 4, 1, 3, 2, 29));  // fmadd 2*3+0.5
  CHECK(c.fpr[4].ps0 == dbits(6.5));
  m.run(A(63, 4, 1, 3, 2, 28));  // fmsub
  CHECK(c.fpr[4].ps0 == dbits(5.5));
  m.run(A(63, 4, 1, 3, 2, 31));  // fnmadd
  CHECK(c.fpr[4].ps0 == dbits(-6.5));
  m.run(A(63, 4, 1, 3, 2, 30));  // fnmsub
  CHECK(c.fpr[4].ps0 == dbits(-5.5));
  m.run(A(59, 4, 1, 3, 2, 29));  // fmadds
  CHECK(c.fpr[4].ps1 == dbits(6.5));
  m.run(A(59, 4, 1, 3, 2, 28));  // fmsubs
  CHECK(c.fpr[4].ps0 == dbits(5.5));
  m.run(A(59, 4, 1, 3, 2, 31));  // fnmadds
  CHECK(c.fpr[4].ps0 == dbits(-6.5));
  m.run(A(59, 4, 1, 3, 2, 30));  // fnmsubs
  CHECK(c.fpr[4].ps0 == dbits(-5.5));
  // Fused: one rounding. a*c = -(1-2^-36)*2^-24, b = 1+2^-23: the exact sum
  // 1 + 2^-24 + 2^-60 is just above a single tie, so it rounds up. (Rounding to
  // double first would land on the tie and round to even, 1.0.)
  c.fpr[5].ps0 = dbits((1.0 - 0x1p-18) * 0x1p-12);
  c.fpr[6].ps0 = dbits(-(1.0 + 0x1p-18) * 0x1p-12);
  c.fpr[7].ps0 = dbits(1.0 + 0x1p-23);
  m.run(A(59, 4, 5, 7, 6, 29));
  CHECK(c.fpr[4].ps0 == dbits(1.0 + 0x1p-23));
  // Double fmadd is fused too: (1+2^-30)^2 - 1 keeps the 2^-60 term.
  c.fpr[8].ps0 = dbits(1.0 + 0x1p-30);
  c.fpr[9].ps0 = dbits(-1.0);
  m.run(A(63, 4, 8, 9, 8, 29));
  CHECK(c.fpr[4].ps0 == dbits(0x1p-29 + 0x1p-60));
}

TEST_CASE("fp NaN, infinity and NI") {
  // The Gekko's NI flush and NaN propagation: the exact model's (Q6: the fast
  // model, the engine's default with NI set, flushes nothing; test_fp checks it).
  struct ExactModel {
    ExactModel() { fp::g_model = fp::Model::Exact; }
    ~ExactModel() { fp::g_model = fp::Model::Fast; }
  } exact;
  Machine m;
  auto& c = m.c;
  c.fpr[1].ps0 = 0x7FF0000000000001ull;  // SNaN
  c.fpr[2].ps0 = dbits(1.0);
  m.run(A(63, 4, 1, 2, 0, 21));  // fadd: SNaN becomes quiet
  CHECK(c.fpr[4].ps0 == 0x7FF8000000000001ull);
  c.fpr[3].ps0 = 0x7FF8123456789ABCull;
  m.run(A(59, 4, 2, 3, 0, 21));  // fadds: QNaN operand, payload truncated to single
  CHECK(c.fpr[4].ps0 == 0x7FF8123440000000ull);
  c.fpr[5].ps0 = 0x7FF0000000000000ull;
  m.run(A(63, 4, 5, 5, 0, 20));  // inf - inf
  CHECK(c.fpr[4].ps0 == fp::kDefaultNaN);
  c.fpr[6].ps0 = 0;
  m.run(A(63, 4, 5, 0, 6, 25));  // inf * 0
  CHECK(c.fpr[4].ps0 == fp::kDefaultNaN);
  m.run(A(63, 4, 6, 6, 0, 18));  // 0 / 0
  CHECK(c.fpr[4].ps0 == fp::kDefaultNaN);
  m.run(A(63, 4, 2, 6, 0, 18));  // 1 / 0
  CHECK(c.fpr[4].ps0 == 0x7FF0000000000000ull);
  c.fpr[7].ps0 = 0xFFF8000000000042ull;  // negative QNaN
  m.run(A(63, 4, 2, 7, 2, 31));  // fnmadd: a NaN result keeps its sign
  CHECK(c.fpr[4].ps0 == 0xFFF8000000000042ull);
  // NaN precedence: frA, then frB, then frC.
  c.fpr[8].ps0 = 0x7FF8000000000008ull;
  m.run(A(63, 4, 2, 8, 7, 29));
  CHECK(c.fpr[4].ps0 == 0x7FF8000000000008ull);
  // NI: a single result below the normal range flushes to zero.
  c.fpr[9].ps0 = dbits(0x1p-70);
  m.run(A(59, 4, 9, 0, 9, 25));  // 2^-140: a single denormal
  CHECK(c.fpr[4].ps0 == dbits(0x1p-140));
  c.fpscr = fpscr::NI;
  m.run(A(59, 4, 9, 0, 9, 25));
  CHECK(c.fpr[4].ps0 == 0);
  c.fpr[10].ps0 = dbits(-0x1p-600);
  c.fpr[12].ps0 = dbits(0x1p-450);
  c.fpscr = 0;
  m.run(A(63, 4, 10, 0, 12, 25));  // double: -2^-1050 is a double denormal
  CHECK(c.fpr[4].ps0 == 0x8000000001000000ull);
  c.fpscr = fpscr::NI;
  m.run(A(63, 4, 10, 0, 12, 25));  // NI flushes it to -0
  CHECK(c.fpr[4].ps0 == fp::kSign);
  c.fpr[11].ps0 = dbits(-0x1p-500);
  m.run(A(63, 4, 11, 0, 9, 25));  // -2^-570: normal, kept
  CHECK(c.fpr[4].ps0 == dbits(-0x1p-570));
}

TEST_CASE("fp rounding, conversion, select, moves") {
  Machine m;
  auto& c = m.c;
  c.fpr[1].ps0 = dbits(1.0 + 0x1p-24);
  m.run(X(63, 4, 0, 1, 12));  // frsp: tie to even
  CHECK(c.fpr[4].ps0 == dbits(1.0));
  CHECK(c.fpr[4].ps1 == dbits(1.0));
  c.fpr[1].ps0 = dbits(1.0 + 0x1p-24 + 0x1p-30);
  m.run(X(63, 4, 0, 1, 12));
  CHECK(c.fpr[4].ps0 == dbits(1.0 + 0x1p-23));
  c.fpr[1].ps0 = dbits(1e300);
  m.run(X(63, 4, 0, 1, 12));
  CHECK(c.fpr[4].ps0 == 0x7FF0000000000000ull);
  c.fpr[2].ps0 = dbits(2.7);
  m.run(X(63, 4, 0, 2, 15));  // fctiwz
  CHECK(c.fpr[4].ps0 == 0xFFF8000000000002ull);
  c.fpr[2].ps0 = dbits(-2.7);
  m.run(X(63, 4, 0, 2, 15));
  CHECK(c.fpr[4].ps0 == 0xFFF80000FFFFFFFEull);
  c.fpr[2].ps0 = dbits(-0.5);
  m.run(X(63, 4, 0, 2, 15));  // zero from a negative source sets bit 32
  CHECK(c.fpr[4].ps0 == 0xFFF8000100000000ull);
  c.fpr[2].ps0 = dbits(3e9);
  m.run(X(63, 4, 0, 2, 15));
  CHECK(c.fpr[4].ps0 == 0xFFF800007FFFFFFFull);
  c.fpr[2].ps0 = dbits(-3e9);
  m.run(X(63, 4, 0, 2, 15));
  CHECK(c.fpr[4].ps0 == 0xFFF8000080000000ull);
  c.fpr[2].ps0 = fp::kDefaultNaN;
  m.run(X(63, 4, 0, 2, 15));
  CHECK(c.fpr[4].ps0 == 0xFFF8000080000000ull);
  c.fpr[2].ps0 = dbits(2.5);
  m.run(X(63, 4, 0, 2, 14));  // fctiw, round to nearest even
  CHECK(c.fpr[4].ps0 == 0xFFF8000000000002ull);
  c.fpr[2].ps0 = dbits(3.5);
  m.run(X(63, 4, 0, 2, 14));
  CHECK(c.fpr[4].ps0 == 0xFFF8000000000004ull);
  // fsel
  c.fpr[5].ps0 = dbits(-0.0);
  c.fpr[6].ps0 = dbits(10.0);  // frC: chosen when frA >= 0
  c.fpr[7].ps0 = dbits(20.0);  // frB
  m.run(A(63, 4, 5, 7, 6, 23));
  CHECK(c.fpr[4].ps0 == dbits(10.0));
  c.fpr[5].ps0 = dbits(-1.0);
  m.run(A(63, 4, 5, 7, 6, 23));
  CHECK(c.fpr[4].ps0 == dbits(20.0));
  c.fpr[5].ps0 = fp::kDefaultNaN;
  m.run(A(63, 4, 5, 7, 6, 23));
  CHECK(c.fpr[4].ps0 == dbits(20.0));
  // moves and sign operations touch ps0 only
  c.fpr[4].ps1 = 0x77;
  c.fpr[8].ps0 = dbits(-3.0);
  m.run(X(63, 4, 0, 8, 72));  // fmr
  CHECK(c.fpr[4].ps0 == dbits(-3.0));
  CHECK(c.fpr[4].ps1 == 0x77);
  m.run(X(63, 4, 0, 8, 40));  // fneg
  CHECK(c.fpr[4].ps0 == dbits(3.0));
  m.run(X(63, 4, 0, 8, 264));  // fabs
  CHECK(c.fpr[4].ps0 == dbits(3.0));
  m.run(X(63, 4, 0, 6, 136));  // fnabs
  CHECK(c.fpr[4].ps0 == dbits(-10.0));
  // estimates (correctly rounded, see fp.h)
  c.fpr[9].ps0 = dbits(4.0);
  m.run(X(59, 4, 0, 9, 24));  // fres
  CHECK(c.fpr[4].ps0 == dbits(0.25));
  CHECK(c.fpr[4].ps1 == dbits(0.25));
  m.run(X(63, 4, 0, 9, 26));  // frsqrte
  CHECK(c.fpr[4].ps0 == dbits(0.5));
}

TEST_CASE("fp compare and FPSCR") {
  Machine m;
  auto& c = m.c;
  c.fpr[1].ps0 = dbits(1.0);
  c.fpr[2].ps0 = dbits(2.0);
  m.run(X(63, 0, 1, 2, 0));  // fcmpu cr0
  CHECK((c.cr >> 28) == 0x8);
  m.run(X(63, 1 << 2, 2, 1, 32));  // fcmpo cr1
  CHECK(((c.cr >> 24) & 0xF) == 0x4);
  c.fpr[3].ps0 = fp::kDefaultNaN;
  m.run(X(63, 0, 1, 3, 0));
  CHECK((c.cr >> 28) == 0x1);
  c.fpr[4].ps0 = dbits(-0.0);
  c.fpr[5].ps0 = 0;
  m.run(X(63, 0, 4, 5, 0));
  CHECK((c.cr >> 28) == 0x2);
  c.fpr[6].ps0 = dbits(-2.0);
  c.fpr[7].ps0 = dbits(-1.0);
  m.run(X(63, 0, 6, 7, 0));
  CHECK((c.cr >> 28) == 0x8);
  // FPSCR
  c.fpscr = 0;
  m.run(X(63, 29, 0, 0, 38));  // mtfsb1 29: NI
  CHECK(c.fpscr == fpscr::NI);
  m.run(X(63, 9, 0, 0, 583));  // mffs
  CHECK(c.fpr[9].ps0 == (0xFFF8000000000000ull | fpscr::NI));
  m.run(X(63, 29, 0, 0, 70));  // mtfsb0
  CHECK(c.fpscr == 0);
  c.fpr[10].ps0 = 0xFFF8000012345678ull;
  m.run(63u << 26 | 0x03u << 17 | 10u << 11 | 711u << 1);  // mtfsf fields 6 and 7
  CHECK(c.fpscr == 0x00000078);
  m.run(63u << 26 | 7u << 23 | 0x5u << 12 | 134u << 1);  // mtfsfi field 7 = 5
  CHECK(c.fpscr == 0x00000075);
  c.fpscr = 0x12345678;
  m.run(X(63, 2 << 2, 6 << 2, 0, 64));  // mcrfs cr2, field 6
  CHECK(((c.cr >> 20) & 0xF) == 0x7);
}

TEST_CASE("paired singles: arithmetic") {
  Machine m;
  auto& c = m.c;
  auto set = [&](u32 r, double a, double b) {
    c.fpr[r].ps0 = dbits(a);
    c.fpr[r].ps1 = dbits(b);
  };
  set(1, 1.0, 2.0);
  set(2, 3.0, 5.0);
  set(3, 0.5, 4.0);
  m.run(A(4, 4, 1, 2, 0, 21));  // ps_add
  CHECK(c.fpr[4].ps0 == dbits(4.0));
  CHECK(c.fpr[4].ps1 == dbits(7.0));
  m.run(A(4, 4, 1, 2, 0, 20));  // ps_sub
  CHECK(c.fpr[4].ps1 == dbits(-3.0));
  m.run(A(4, 4, 2, 1, 0, 18));  // ps_div
  CHECK(c.fpr[4].ps0 == dbits(3.0));
  CHECK(c.fpr[4].ps1 == dbits(2.5));
  m.run(A(4, 4, 1, 0, 3, 25));  // ps_mul
  CHECK(c.fpr[4].ps0 == dbits(0.5));
  CHECK(c.fpr[4].ps1 == dbits(8.0));
  m.run(A(4, 4, 2, 0, 3, 12));  // ps_muls0: both halves times c.ps0
  CHECK(c.fpr[4].ps0 == dbits(1.5));
  CHECK(c.fpr[4].ps1 == dbits(2.5));
  m.run(A(4, 4, 2, 0, 3, 13));  // ps_muls1: times c.ps1
  CHECK(c.fpr[4].ps0 == dbits(12.0));
  CHECK(c.fpr[4].ps1 == dbits(20.0));
  m.run(A(4, 4, 1, 2, 3, 29));  // ps_madd: a*c + b
  CHECK(c.fpr[4].ps0 == dbits(3.5));
  CHECK(c.fpr[4].ps1 == dbits(13.0));
  m.run(A(4, 4, 1, 2, 3, 28));  // ps_msub
  CHECK(c.fpr[4].ps0 == dbits(-2.5));
  CHECK(c.fpr[4].ps1 == dbits(3.0));
  m.run(A(4, 4, 1, 2, 3, 31));  // ps_nmadd
  CHECK(c.fpr[4].ps0 == dbits(-3.5));
  m.run(A(4, 4, 1, 2, 3, 30));  // ps_nmsub
  CHECK(c.fpr[4].ps1 == dbits(-3.0));
  m.run(A(4, 4, 1, 2, 3, 14));  // ps_madds0: a*c.ps0 + b
  CHECK(c.fpr[4].ps0 == dbits(3.5));
  CHECK(c.fpr[4].ps1 == dbits(6.0));
  m.run(A(4, 4, 1, 2, 3, 15));  // ps_madds1: a*c.ps1 + b
  CHECK(c.fpr[4].ps0 == dbits(7.0));
  CHECK(c.fpr[4].ps1 == dbits(13.0));
  m.run(A(4, 4, 1, 2, 3, 10));  // ps_sum0: ps0 = a.ps0 + b.ps1, ps1 = c.ps1
  CHECK(c.fpr[4].ps0 == dbits(6.0));
  CHECK(c.fpr[4].ps1 == dbits(4.0));
  m.run(A(4, 4, 1, 2, 3, 11));  // ps_sum1: ps0 = c.ps0, ps1 = a.ps0 + b.ps1
  CHECK(c.fpr[4].ps0 == dbits(0.5));
  CHECK(c.fpr[4].ps1 == dbits(6.0));
  set(5, -1.0, 2.0);
  m.run(A(4, 4, 5, 2, 3, 23));  // ps_sel: a >= 0 ? c : b
  CHECK(c.fpr[4].ps0 == dbits(3.0));
  CHECK(c.fpr[4].ps1 == dbits(4.0));
  set(6, 4.0, 0.25);
  m.run(X(4, 4, 0, 6, 24));  // ps_res
  CHECK(c.fpr[4].ps0 == dbits(0.25));
  CHECK(c.fpr[4].ps1 == dbits(4.0));
  m.run(X(4, 4, 0, 6, 26));  // ps_rsqrte
  CHECK(c.fpr[4].ps0 == dbits(0.5));
  CHECK(c.fpr[4].ps1 == dbits(2.0));
}

TEST_CASE("paired singles: moves, merges, compares") {
  Machine m;
  auto& c = m.c;
  c.fpr[1] = {dbits(1.0), dbits(2.0)};
  c.fpr[2] = {dbits(3.0), dbits(-4.0)};
  m.run(X(4, 4, 1, 2, 528));  // ps_merge00
  CHECK(c.fpr[4].ps0 == dbits(1.0));
  CHECK(c.fpr[4].ps1 == dbits(3.0));
  m.run(X(4, 4, 1, 2, 560));  // ps_merge01
  CHECK(c.fpr[4].ps1 == dbits(-4.0));
  m.run(X(4, 4, 1, 2, 592));  // ps_merge10
  CHECK(c.fpr[4].ps0 == dbits(2.0));
  CHECK(c.fpr[4].ps1 == dbits(3.0));
  m.run(X(4, 4, 1, 2, 624));  // ps_merge11
  CHECK(c.fpr[4].ps0 == dbits(2.0));
  CHECK(c.fpr[4].ps1 == dbits(-4.0));
  m.run(X(4, 4, 0, 2, 72));  // ps_mr
  CHECK(c.fpr[4].ps1 == dbits(-4.0));
  m.run(X(4, 4, 0, 2, 40));  // ps_neg
  CHECK(c.fpr[4].ps0 == dbits(-3.0));
  CHECK(c.fpr[4].ps1 == dbits(4.0));
  m.run(X(4, 4, 0, 2, 264));  // ps_abs
  CHECK(c.fpr[4].ps1 == dbits(4.0));
  m.run(X(4, 4, 0, 2, 136));  // ps_nabs
  CHECK(c.fpr[4].ps0 == dbits(-3.0));
  m.run(X(4, 0, 1, 2, 0));  // ps_cmpu0: 1 < 3
  CHECK((c.cr >> 28) == 0x8);
  m.run(X(4, 0, 1, 2, 64));  // ps_cmpu1: 2 > -4
  CHECK((c.cr >> 28) == 0x4);
  m.run(X(4, 1 << 2, 2, 1, 32));  // ps_cmpo0 cr1: 3 > 1
  CHECK(((c.cr >> 24) & 0xF) == 0x4);
  m.run(X(4, 1 << 2, 2, 1, 96));  // ps_cmpo1: -4 < 2
  CHECK(((c.cr >> 24) & 0xF) == 0x8);
}

TEST_CASE("paired singles: quantized loads and stores") {
  Machine m;
  auto& c = m.c;
  c.spr[spr::GQR0 + 1] = 0x00040004;  // u8, scale 0
  c.spr[spr::GQR0 + 2] = 0x08070807;  // s16, scale 8 (load and store)
  c.spr[spr::GQR0 + 3] = 0x00060006;  // s8
  m.w8(0x80002000, 0x10);
  m.w8(0x80002001, 0x20);
  m.w16(0x80002010, 0x0100);
  m.w16(0x80002012, 0xFF80);
  m.w32(0x80002020, 0x3F800000);
  m.w32(0x80002024, 0x40000000);
  c.gpr[3] = 0x80002000;
  m.run(PSQ(56, 1, 3, 0, 0, 1));  // psq_l u8
  CHECK(c.fpr[1].ps0 == dbits(16.0));
  CHECK(c.fpr[1].ps1 == dbits(32.0));
  m.run(PSQ(56, 2, 3, 0x10, 0, 2));  // s16 scale 8: 256/256, -128/256
  CHECK(c.fpr[2].ps0 == dbits(1.0));
  CHECK(c.fpr[2].ps1 == dbits(-0.5));
  m.run(PSQ(56, 3, 3, 0x20, 1, 0));  // W=1: ps1 = 1.0; type 0 float
  CHECK(c.fpr[3].ps0 == dbits(1.0));
  CHECK(c.fpr[3].ps1 == dbits(1.0));
  m.run(PSQ(57, 4, 3, 0x20, 0, 0));  // psq_lu
  CHECK(c.gpr[3] == 0x80002020);
  CHECK(c.fpr[4].ps1 == dbits(2.0));
  c.gpr[3] = 0x80002000;
  c.gpr[5] = 0x10;
  m.run(PSQX(5, 3, 5, 0, 2, 6));  // psq_lx
  CHECK(c.fpr[5].ps1 == dbits(-0.5));
  m.run(PSQX(5, 3, 5, 0, 1, 38));  // psq_lux u8 at 0x80002010
  CHECK(c.gpr[3] == 0x80002010);
  CHECK(c.fpr[5].ps0 == dbits(1.0));
  // Stores
  c.gpr[3] = 0x80002100;
  c.fpr[6] = {dbits(1.5), dbits(-2.0)};
  m.run(PSQ(60, 6, 3, 0, 0, 2));  // psq_st s16 scale 8
  CHECK(m.r16(0x80002100) == 0x0180);
  CHECK(m.r16(0x80002102) == 0xFE00);
  c.fpr[7] = {dbits(300.0), dbits(-5.0)};
  m.run(PSQ(60, 7, 3, 4, 0, 1));  // u8 saturates
  CHECK(m.r8(0x80002104) == 255);
  CHECK(m.r8(0x80002105) == 0);
  c.fpr[8] = {dbits(-200.0), dbits(2.9)};
  m.run(PSQ(60, 8, 3, 6, 0, 3));  // s8: saturate, truncate toward zero
  CHECK(m.r8(0x80002106) == 0x80);
  CHECK(m.r8(0x80002107) == 2);
  c.fpr[9] = {dbits(0x1p-140), dbits(1.0)};
  m.run(PSQ(60, 9, 3, 8, 0, 0));  // float type: a denormal single stores as zero
  CHECK(m.r32(0x80002108) == 0);
  CHECK(m.r32(0x8000210C) == 0x3F800000);
  m.run(PSQ(61, 6, 3, 0x10, 1, 2));  // psq_stu, W=1: one element
  CHECK(c.gpr[3] == 0x80002110);
  CHECK(m.r16(0x80002110) == 0x0180);
  CHECK(m.r16(0x80002112) == 0);
  c.gpr[5] = 4;
  m.run(PSQX(6, 3, 5, 0, 2, 7));  // psq_stx
  CHECK(m.r16(0x80002114) == 0x0180);
  m.run(PSQX(6, 3, 5, 1, 2, 39));  // psq_stux
  CHECK(c.gpr[3] == 0x80002114);
}

// ---------------------------------------------------------------- branches and system

TEST_CASE("branches") {
  Machine m;
  auto& c = m.c;
  constexpr u32 pc = Machine::kPc;
  m.run(I(0x100));  // b
  CHECK(c.pc == pc + 0x100);
  m.run(I(-0x100, 0, 1));  // bl
  CHECK(c.pc == pc - 0x100);
  CHECK(c.lr == pc + 4);
  m.run(I(0x3000, 1, 0));  // ba
  CHECK(c.pc == 0x3000);
  c.cr = CR0(0x2);
  m.run(B(12, 2, 0x20));  // beq taken
  CHECK(c.pc == pc + 0x20);
  m.run(B(4, 2, 0x20));  // bne not taken
  CHECK(c.pc == pc + 4);
  c.ctr = 2;
  m.run(B(16, 0, -8));  // bdnz: ctr 2 -> 1, taken
  CHECK(c.pc == pc - 8);
  CHECK(c.ctr == 1);
  m.run(B(16, 0, -8));  // ctr 1 -> 0, falls through
  CHECK(c.pc == pc + 4);
  m.run(B(18, 0, 8));  // bdz: ctr 0 -> 0xFFFFFFFF, not taken
  CHECK(c.pc == pc + 4);
  CHECK(c.ctr == 0xFFFFFFFF);
  c.ctr = 2;
  m.run(B(8, 2, 8));  // bdnzt eq: ctr -> 1 and eq set: taken
  CHECK(c.pc == pc + 8);
  m.run(B(12, 2, 8, 0, 1));  // beql: LR set even when... taken here
  CHECK(c.lr == pc + 4);
  c.lr = 0x80005000;
  m.run(X(19, 20, 0, 0, 16));  // blr
  CHECK(c.pc == 0x80005000);
  m.run(X(19, 20, 0, 0, 16, 1));  // blrl: jumps to the old LR, then sets LR
  CHECK(c.pc == 0x80005000);
  CHECK(c.lr == pc + 4);
  c.cr = 0;
  c.lr = 0x80005000;
  m.run(X(19, 12, 2, 0, 16));  // beqlr not taken
  CHECK(c.pc == pc + 4);
  c.ctr = 0x80006003;  // low bits ignored
  m.run(X(19, 20, 0, 0, 528));  // bctr
  CHECK(c.pc == 0x80006000);
  m.run(X(19, 20, 0, 0, 528, 1));  // bctrl
  CHECK(c.pc == 0x80006000);
  CHECK(c.lr == pc + 4);
  CHECK_THROWS_AS(m.run(X(19, 16, 0, 0, 528)), GuestFault);  // bcctr that decrements CTR
}

TEST_CASE("system: sc, rfi, traps, invalid") {
  Machine m;
  auto& c = m.c;
  m.run(0x44000002);  // sc
  CHECK(m.hooks.syscalls == 1);
  CHECK(c.pc == Machine::kPc + 4);
  c.spr[spr::SRR0] = 0x80004001;
  c.spr[spr::SRR1] = 0x0000B032;
  c.msr = 0x00002000;
  m.run(X(19, 0, 0, 0, 50));  // rfi
  CHECK(c.pc == 0x80004000);
  CHECK(c.msr == 0x0000B032);
  CHECK(m.hooks.last_old_msr == 0x00002000);
  c.gpr[3] = 1;
  c.gpr[4] = 2;
  m.run(X(31, 4, 3, 4, 4));  // tw eq: no trap
  CHECK(c.pc == Machine::kPc + 4);
  CHECK_THROWS_AS(m.run(X(31, 16, 3, 4, 4)), GuestFault);  // tw lt: 1 < 2 traps
  m.run(D(3, 4, 3, 5));  // twi eq 1 == 5: no trap
  CHECK_THROWS_AS(m.run(D(3, 8, 4, 1)), GuestFault);  // twi gt 2 > 1
  CHECK_THROWS_AS(m.run(0), GuestFault);
  CHECK_THROWS_AS(m.run(X(31, 3, 4, 5, 310)), GuestFault);  // eciwx
  CHECK_THROWS_AS(m.run(X(31, 3, 4, 5, 438)), GuestFault);  // ecowx
  c.gpr[4] = 0x90000000;  // unmapped, no Io
  CHECK_THROWS_AS(m.run(D(32, 3, 4, 0)), GuestFault);
}

TEST_CASE("floating-point unavailable: FPU instructions with MSR[FP] clear raise 0x800") {
  // The class is exactly the FPU instructions, by mnemonic.
  for (int k = 1; k < static_cast<int>(Op::count); ++k) {
    Op op = static_cast<Op>(k);
    std::string n = op_name(op);
    bool fpu = n.rfind("lf", 0) == 0 || n.rfind("stf", 0) == 0 || n.rfind("ps", 0) == 0 || n[0] == 'f' ||
               n == "mcrfs" || n == "mffs" || n.rfind("mtfs", 0) == 0;
    CHECK_MESSAGE(uses_fpu(op) == fpu, n);
  }
  Machine m;
  auto& c = m.c;
  c.msr = 0x00009032;  // EE ME IR DR RI, FP clear
  c.gpr[4] = 0x80002000;
  m.w32(0x80002000, 0x3F800000);  // 1.0f
  c.fpr[1].ps0 = 0;
  const auto ticks = c.ticks;
  m.run(D(48, 1, 4, 0));  // lfs f1, 0(r4): does not run
  CHECK(c.fpr[1].ps0 == 0);
  CHECK(c.pc == 0x00000800);  // the vector, in real mode
  CHECK(c.spr[spr::SRR0] == Machine::kPc);
  CHECK(c.spr[spr::SRR1] == 0x00009032);
  CHECK(c.msr == 0x00001000);  // only ME stays
  CHECK(m.hooks.last_old_msr == 0x00009032);
  CHECK(c.ticks == ticks);  // the instruction did not start
  // The handler returns with FP set in SRR1 (rfi placed at the vector's cached
  // address, since this machine maps no real mode): the load runs this time.
  c.spr[spr::SRR1] |= msr::FP;
  m.run(X(19, 0, 0, 0, 50), 0x80000800);  // rfi
  CHECK(c.pc == Machine::kPc);
  m.run(D(48, 1, 4, 0));
  CHECK(c.fpr[1].ps0 == 0x3FF0000000000000ull);
  CHECK(c.ticks == ticks + 2);
  // Integer instructions and dcbz_l never need the FPU.
  c.msr = 0;
  c.gpr[3] = 1;
  m.run(D(14, 3, 3, 1));  // addi r3, r3, 1
  CHECK(c.gpr[3] == 2);
  CHECK_FALSE(uses_fpu(Op::dcbz_l));
}

// Must stay last in this file: every opcode has a vector above.
TEST_CASE("coverage: every opcode executed by some vector") {
  auto& cov = mvp::ppc::test::covered();
  int missing = 0;
  for (int k = 1; k < static_cast<int>(Op::count); ++k) {
    Op op = static_cast<Op>(k);
    // Faulting instructions are recorded only when they complete; these are
    // covered by the CHECK_THROWS cases above.
    if (op == Op::eciwx || op == Op::ecowx) continue;
    if (!cov.count(op)) {
      ++missing;
      MESSAGE("no vector for ", op_name(op));
    }
  }
  CHECK(missing == 0);
}
