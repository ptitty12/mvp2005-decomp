#include "runtime/hle_game.h"

#include "common/bytes.h"
#include "ppc/fp.h"
#include "ppc/mem_access.h"
#include "ppc/sem.h"
#include "runtime/hw.h"

namespace mvp::rt {

namespace {

using ppc::fp::Prec;

// A paired-single register's halves (double bits, as ppc::Fpr).
struct Ps {
  u64 p0, p1;
};

// The fast model's steps (ppc/fp.h; sem.h's ps_muls0 and ps_madds0 with fp.cpp's
// fast paths, force25 not applied): one IEEE double operation each, the product
// then the sum, every result rounded to single. V4: a NaN result keeps whatever
// bits the host gave it here, and canon() makes it the default NaN where a
// computed value leaves for guest state (an FPR, the pipe): every one of these
// operations gives a NaN for a NaN operand, so a value is a NaN exactly where
// the model's (canonical at every step) is, and at the exits the bits are the
// same. Loaded values never pass through canon(): their bits stay as loaded.
inline u64 single_round(double r) { return ppc::fp::to_bits(static_cast<double>(static_cast<float>(r))); }
inline u64 mul_s(u64 a, u64 c) { return single_round(ppc::fp::to_f64(a) * ppc::fp::to_f64(c)); }
inline u64 madd_s(u64 a, u64 c, u64 b) {
  const double prod = ppc::fp::to_f64(a) * ppc::fp::to_f64(c);
  return single_round(prod + ppc::fp::to_f64(b));
}
inline u64 canon(u64 x) { return ppc::fp::is_nan(x) ? ppc::fp::kDefaultNaN : x; }
inline Ps muls0(Ps a, Ps c) { return {mul_s(a.p0, c.p0), mul_s(a.p1, c.p0)}; }
inline Ps madds0(Ps a, Ps c, Ps b) { return {madd_s(a.p0, c.p0, b.p0), madd_s(a.p1, c.p0, b.p1)}; }

// Loads from plain memory `p` (a span ppc::host_span gave): lfs fills both
// halves; psq_l with a float GQR (type 0-3) reads two singles.
inline Ps lfs(const u8* p) {
  const u64 v = ppc::fp::single_to_double_inline(be32(p));
  return {v, v};
}
inline Ps psq_l(const u8* p) {
  return {ppc::fp::single_to_double_inline(be32(p)), ppc::fp::single_to_double_inline(be32(p + 4))};
}

// The hardware model a Cpu's MMIO goes to, if it is one.
inline Hardware* hardware_of(Cpu& c) {
  return c.mem->io ? static_cast<Hardware*>(c.mem->io->hardware()) : nullptr;
}

// Stores by one instruction at `pc` with c.ticks == `t`: straight to the
// write-gather pipe's model when the address goes there (Hardware::wgp_store),
// else through ppc::write32 / write8 as the interpreter's.
struct Stores {
  Cpu& c;
  Hardware* pipe;  // set when the stores' address [ea, ea + 8) is the pipe's
  void word(u32 ea, u32 v, u32 pc, std::uint64_t t) {
    if (pipe) return pipe->wgp_store(ea, v, 4, pc, t);
    c.pc = pc;
    c.ticks = t;
    ppc::write32(c, ea, v);
  }
  void byte(u32 ea, u32 v, u32 pc, std::uint64_t t) {
    if (pipe) return pipe->wgp_store(ea, v & 0xFF, 1, pc, t);
    c.pc = pc;
    c.ticks = t;
    ppc::write8(c, ea, v);
  }
};
// Whether stores to [ea, ea + 8) go to the write-gather pipe (ppc::slow_write's
// way to Hardware::write: an unmapped page, the pipe's addresses).
inline bool pipe_stores(Cpu& c, Hardware* hw, u32 ea) {
  return hw && ea - 0xCC008000u <= 0x1000u - 8 && !c.mem->page[ea >> ppc::Memory::kPageShift];
}

}  // namespace

// fn_803EB000 (hle_game.h), from its first instruction to its blr (c.pc is left
// as it is; the caller returns to LR). Its two paths share their last 29
// instructions (the tail from `lwz r12, 4(r14)` to the blr), at 0x803EB14C after
// 83 instructions or at 0x803EB238 after 37. False, with nothing changed: the
// state is not the one this is written for (the original must run).
static bool run_803EB000(Cpu& c) {
  const u32 g = c.spr[ppc::spr::GQR0];
  if (!(c.msr & ppc::msr::FP) || !ppc::fp::fast_model(c.fpscr) || ((g >> 16) & 7) >= 4 || (g & 7) >= 4) {
    return false;
  }
  // Every load before the first store is from plain memory (no side effects), so
  // they may run in any order; anything else runs the original.
  const u32 r3 = c.gpr[3], r5 = c.gpr[5], r6 = c.gpr[6], r14 = c.gpr[14];
  const u8* p3 = ppc::host_span(c, r3, 12);
  const u8* p6 = ppc::host_span(c, r6, 0x3C);
  const u8* p14 = ppc::host_span(c, r14, 8);
  if (!p3 || !p6 || !p14) {
    return false;
  }
  const u32 i0 = (be32(p3) & 0xFF) << 6, i1 = (be32(p3 + 4) & 0xFF) << 6, i2 = (be32(p3 + 8) & 0xFF) << 6;
  // fsub f0, f0, f0; three lfs; fcmpu cr1, f30, f0; beq cr1 (one matrix).
  const u64 zero = ppc::fp::sub(c.fpr[0].ps0, c.fpr[0].ps0, Prec::Double, c.fpscr);
  const Ps f31 = lfs(p3), f30 = lfs(p3 + 4), f29 = lfs(p3 + 8);
  const u32 cr1 = ppc::sem::fcmp_bits(f30.p0, zero);
  const bool one = (cr1 & 2) != 0;
  const u8* m0 = ppc::host_span(c, r5 + i0, 64);
  const u8* m1 = one ? m0 : ppc::host_span(c, r5 + i1, 64);
  const u8* m2 = one ? m0 : ppc::host_span(c, r5 + i2, 64);
  if (!m0 || !m1 || !m2) {
    return false;
  }
  const std::uint64_t t0 = c.ticks;
  ppc::sem::set_cr_field(c, 1, cr1);
  c.fpr[29] = {f29.p0, f29.p1};
  c.fpr[30] = {f30.p0, f30.p1};
  c.fpr[31] = {f31.p0, f31.p1};
  // The blend's rows (f0-f7: row k in f(2k), f(2k+1)).
  Ps row[8];
  if (one) {
    for (u32 k = 0; k < 8; ++k) row[k] = psq_l(m0 + 8 * k);
    c.gpr[7] = r5 + i0;
  } else {
    Ps last8{}, last9{}, last24{};
    for (u32 k = 0; k < 8; ++k) {
      const Ps a = psq_l(m0 + 8 * k), b = psq_l(m1 + 8 * k), d = psq_l(m2 + 8 * k);
      row[k] = madds0(d, f29, madds0(b, f30, muls0(a, f31)));
      if (k == 6) last8 = a;
      if (k == 7) last9 = a, last24 = d;
    }
    c.gpr[7] = r5 + i0;
    c.gpr[8] = r5 + i1;
    c.gpr[9] = r5 + i2;
    c.fpr[8] = {last8.p0, last8.p1};
    c.fpr[9] = {last9.p0, last9.p1};
    c.fpr[24] = {last24.p0, last24.p1};
  }
  for (u32 k = 0; k < 8; ++k)  // loaded (one matrix) or computed (the blend)
    c.fpr[k] = one ? ppc::Fpr{row[k].p0, row[k].p1} : ppc::Fpr{canon(row[k].p0), canon(row[k].p1)};
  // The matrix at r6: columns 0 and 1 (f10-f13, f16-f19) and its products.
  const Ps f10 = lfs(p6), f11 = lfs(p6 + 0x10), f12 = lfs(p6 + 0x20), f13 = lfs(p6 + 0x30);
  const Ps f16 = lfs(p6 + 4), f17 = lfs(p6 + 0x14), f18 = lfs(p6 + 0x24), f19 = lfs(p6 + 0x34);
  Ps f14 = madds0(row[4], f12, madds0(row[2], f11, muls0(row[0], f10)));
  Ps f15 = madds0(row[5], f12, madds0(row[3], f11, muls0(row[1], f10)));
  Ps f20 = madds0(row[4], f18, madds0(row[2], f17, muls0(row[0], f16)));
  Ps f21 = madds0(row[5], f18, madds0(row[3], f17, muls0(row[1], f16)));
  // The tail, in program order from its first instruction (index j from 1).
  const u32 at = one ? 0x803EB238u : 0x803EB14Cu;
  const std::uint64_t tail = t0 + (one ? 37 : 83);
  auto pc = [&](u32 j) { return at + 4 * (j - 1); };  // the original's instruction j: its address
  auto tk = [&](u32 j) { return tail + j; };           // and c.ticks while it runs
  const u32 r12 = be32(p14 + 4), r4 = be32(p14);
  const Ps f10b = lfs(p6 + 8);
  f14 = madds0(row[6], f13, f14);
  const Ps f11b = lfs(p6 + 0x18);
  f20 = madds0(row[6], f19, f20);
  const Ps f12b = lfs(p6 + 0x28);
  f15 = madds0(row[7], f13, f15);
  f21 = madds0(row[7], f19, f21);
  const u32 r10 = c.gpr[10];
  Hardware* hw = hardware_of(c);
  Stores st{c, pipe_stores(c, hw, r4) ? hw : nullptr};
  auto psq_st = [&](u32 j, Ps v) {  // computed values (V4: canon)
    st.word(r4, ppc::fp::double_to_single_ftz(canon(v.p0)), pc(j), tk(j));
    st.word(r4 + 4, ppc::fp::double_to_single_ftz(canon(v.p1)), pc(j), tk(j));
  };
  st.word(r4, r12, pc(11), tk(11));       // stw r12, 0(r4)
  st.byte(r4, r10 * 12, pc(12), tk(12));  // stb r11, 0(r4)
  psq_st(13, f14);
  Ps f22 = muls0(row[0], f10b), f23 = muls0(row[1], f10b);
  psq_st(16, f15);
  const Ps f13b = lfs(p6 + 0x38);  // after two stores, as the original reads it
  f22 = madds0(row[2], f11b, f22);
  f23 = madds0(row[3], f11b, f23);
  psq_st(20, f20);
  f22 = madds0(row[4], f12b, f22);
  f23 = madds0(row[5], f12b, f23);
  f22 = madds0(row[6], f13b, f22);
  f23 = madds0(row[7], f13b, f23);
  psq_st(26, f21);
  psq_st(27, f22);
  psq_st(28, f23);
  c.gpr[4] = r4;
  c.gpr[11] = r10 * 9;
  c.gpr[12] = r12;
  c.fpr[10] = {f10b.p0, f10b.p1};
  c.fpr[11] = {f11b.p0, f11b.p1};
  c.fpr[12] = {f12b.p0, f12b.p1};
  c.fpr[13] = {f13b.p0, f13b.p1};
  c.fpr[14] = {canon(f14.p0), canon(f14.p1)};
  c.fpr[15] = {canon(f15.p0), canon(f15.p1)};
  c.fpr[16] = {f16.p0, f16.p1};
  c.fpr[17] = {f17.p0, f17.p1};
  c.fpr[18] = {f18.p0, f18.p1};
  c.fpr[19] = {f19.p0, f19.p1};
  c.fpr[20] = {canon(f20.p0), canon(f20.p1)};
  c.fpr[21] = {canon(f21.p0), canon(f21.p1)};
  c.fpr[22] = {canon(f22.p0), canon(f22.p1)};
  c.fpr[23] = {canon(f23.p0), canon(f23.p1)};
  c.ticks = tail + 29;  // through the blr
  return true;
}

void hle_fn_803EB000(Cpu& c, GuestFn original) {
  Hardware* const hw = hardware_of(c);
  if (hw) hw->gp_defer(true);  // Q6: the GP reads its bursts in one piece (Hardware::gp_defer)
  const bool done = run_803EB000(c);
  if (hw) hw->gp_defer(false);
  if (!done) original(c);
}

namespace {

// Q6: a safe point where translated code has one (before a back edge's branch,
// taken or not; a function's entry): rt.safepoint when c.ticks >= c.next_event,
// with c.pc there and every register in c.
enum class Safe { Go, Ran, Leave };
Safe safe_point(Cpu& c, u32 at) {
  if (c.ticks < c.next_event) return Safe::Go;
  const std::uint64_t t = c.ticks;
  c.pc = at;
  c.hooks->safepoint(c);
  if (g_unwinding || c.pc != at) {
    // Translated code would leave or resume elsewhere: leave, and the dispatcher
    // goes on at c.pc (the original's remains run in the interpreter).
    g_unwinding = true;
    return Safe::Leave;
  }
  return c.ticks == t ? Safe::Go : Safe::Ran;  // Ran: an interrupt handler ran and came back here
}

// Whether the state fn_803EB000 and its loop are written for holds.
bool fp_state_fits(const Cpu& c) {
  const u32 g = c.spr[ppc::spr::GQR0];
  return !g_entry_hook && (c.msr & ppc::msr::FP) && ppc::fp::fast_model(c.fpscr) && ((g >> 16) & 7) < 4 &&
         (g & 7) < 4;
}

}  // namespace

// fn_803EA3EC (hle_game.h): its prologue, the loop (fn_803EB000 natively, then
// the normal and the second matrix loads), its epilogue; the safe points before
// the bdnz (every iteration) and at fn_803EB000's entry where translated code has
// them. The loop head is instruction 23, its body 28 instructions plus the
// callee's, the epilogue 19.
void hle_fn_803EA3EC(Cpu& c, GuestFn original) {
  constexpr u32 kBack = 0x803EA4B0u, kCallee = 0x803EB000u, kRet = 0x803EA448u;
  const u32 n = c.gpr[4], r1 = c.gpr[1];
  if (!fp_state_fits(c) || n == 0 || n > 4096 || !ppc::host_span(c, r1 - 0x7C, 0x84)) {
    original(c);
    return;
  }
  static constexpr u8 kSaved[14] = {14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 31, 30, 29};
  // mflr r0; stw r0, 4(r1); stwu r1, -0x7c(r1); stfd f14-f24, f31, f30, f29;
  // stw r14, 0x78(r1); lis/addi r14 = 0x8062C750; li r10, 0; mtctr r4.
  const std::uint64_t t0 = c.ticks;
  c.gpr[0] = c.lr;
  ppc::write32(c, r1 + 4, c.gpr[0]);
  const u32 sp = r1 - 0x7C;
  ppc::write32(c, sp, r1);
  c.gpr[1] = sp;
  for (u32 k = 0; k < 14; ++k) ppc::write64(c, sp + 8 + 8 * k, c.fpr[kSaved[k]].ps0);
  ppc::write32(c, sp + 0x78, c.gpr[14]);
  c.gpr[14] = 0x8062C750u;
  c.gpr[10] = 0;
  c.ctr = n;
  c.ticks = t0 + 22;
  Hardware* const hw = hardware_of(c);
  for (;;) {
    // bl fn_803EB000, and the callee's entry's safe point.
    c.ticks += 1;
    c.lr = kRet;
    switch (safe_point(c, kCallee)) {
      case Safe::Leave: return;
      case Safe::Ran:
        if (!fp_state_fits(c)) return void(g_unwinding = true);
        break;
      case Safe::Go: break;
    }
    if (hw) hw->gp_defer(true);  // Q6: the GP reads this pass's bursts before the back edge's safe point
    if (!run_803EB000(c)) {  // the original runs there instead, from the dispatcher
      if (hw) hw->gp_defer(false);
      c.pc = kCallee;
      g_unwinding = true;
      return;
    }
    // Back at 0x803EA448: 27 instructions, j from 1, in program order.
    const std::uint64_t tb = c.ticks;
    auto pc = [&](u32 j) { return kRet + 4 * (j - 1); };
    auto tk = [&](u32 j) { return tb + j; };
    const u32 r14 = c.gpr[14], r4 = c.gpr[4];
    Stores st{c, pipe_stores(c, hw, r4) ? hw : nullptr};
    auto psq_st = [&](u32 j, u32 f) {
      st.word(r4, ppc::fp::double_to_single_ftz(c.fpr[f].ps0), pc(j), tk(j));
      st.word(r4 + 4, ppc::fp::double_to_single_ftz(c.fpr[f].ps1), pc(j), tk(j));
    };
    auto stfs = [&](u32 j, u32 f) { st.word(r4, ppc::fp::double_to_single(c.fpr[f].ps0), pc(j), tk(j)); };
    c.pc = pc(1);
    c.ticks = tk(1);
    c.gpr[12] = ppc::read32(c, r14 + 8);  // lwz r12, 8(r14)
    st.word(r4, c.gpr[12], pc(2), tk(2));  // stw r12, 0(r4)
    st.byte(r4, c.gpr[11], pc(3), tk(3));  // stb r11, 0(r4)
    psq_st(4, 14);
    stfs(5, 15);
    psq_st(6, 20);
    stfs(7, 21);
    psq_st(8, 22);
    stfs(9, 23);
    c.gpr[11] = c.gpr[10] * 12 + 0x78;  // mulli r11, r10, 12; addi r11, r11, 0x78
    c.fpr[4].ps0 = ppc::fp::sub(c.fpr[4].ps0, c.fpr[4].ps0, Prec::Double, c.fpscr);  // fsub f4, f4, f4
    c.pc = pc(13);
    c.ticks = tk(13);
    c.gpr[12] = ppc::read32(c, r14 + 4);  // lwz r12, 4(r14)
    st.word(r4, c.gpr[12], pc(14), tk(14));
    st.byte(r4, c.gpr[11], pc(15), tk(15));
    psq_st(16, 14);
    stfs(17, 15);
    stfs(18, 4);
    psq_st(19, 20);
    stfs(20, 21);
    stfs(21, 4);
    psq_st(22, 22);
    stfs(23, 23);
    stfs(24, 4);
    c.gpr[3] += 0x10;
    c.gpr[10] += 1;
    if (hw) hw->gp_defer(false);
    c.ticks = tk(26);
    switch (safe_point(c, kBack)) {  // the back edge's (an interrupt handler that ran counted its own)
      case Safe::Leave: return;
      case Safe::Ran:
        if (!fp_state_fits(c)) return void(g_unwinding = true);
        break;
      case Safe::Go: break;
    }
    c.ticks += 1;
    if (--c.ctr == 0) break;  // bdnz
  }
  // lwz r0, 0x80(r1); mtlr r0; lfd f14-f24, f31, f30, f29; lwz r14, 0x78(r1);
  // addi r1, r1, 0x7c; blr (hle_call returns to LR).
  const std::uint64_t te = c.ticks;
  c.pc = 0x803EA4B4u;
  c.gpr[0] = ppc::read32(c, sp + 0x80);
  c.lr = c.gpr[0];
  for (u32 k = 0; k < 14; ++k) c.fpr[kSaved[k]].ps0 = ppc::read64(c, sp + 8 + 8 * k);
  c.gpr[14] = ppc::read32(c, sp + 0x78);
  c.gpr[1] = sp + 0x7C;
  c.ticks = te + 19;
}

}  // namespace mvp::rt
