// Local-only check (Q4): each native implementation of a game function
// (runtime/hle_game.h, HleKind::Native) against the interpreter running the
// original from the retail main.dol, on random operands, specials included:
// registers, ticks, memory and every MMIO store (address, value, size, and the
// guest PC and ticks it was made at) must be the same. Also checks that a state
// the implementation is not written for runs the original.
//
// Needs MVP_DISC; exits 77 (skipped) without it.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "common/bytes.h"
#include "ppc/fp.h"
#include "ppc/interp.h"
#include "ppc/isa.h"
#include "runtime/hle.h"
#include "runtime/hle_game.h"
#include "runtime/machine.h"
#include "vfs/disc_check.h"

#ifndef MVP_DISC_DEFAULT
#define MVP_DISC_DEFAULT ""
#endif

using namespace mvp;
using namespace mvp::rt;

namespace {

struct Store {
  u32 addr, value;
  int size;
  u32 pc;
  std::uint64_t ticks;
  bool operator==(const Store&) const = default;
};

// MMIO: every store recorded with the guest PC and ticks it was made at.
struct Recorder : ppc::Io {
  ppc::Cpu* c = nullptr;
  std::uint64_t bias = 0;  // ticks that "interrupt handlers" added (QuietHooks)
  std::vector<Store> log;
  u32 read(u32, int) override { return 0; }
  void write(u32 a, u32 v, int size) override { log.push_back({a, v, size, c->pc, c->ticks - bias}); }
};

std::uint64_t g_seed = 0x2545F4914F6CDD1Dull;
u32 rnd() {
  g_seed ^= g_seed << 13;
  g_seed ^= g_seed >> 7;
  g_seed ^= g_seed << 17;
  return static_cast<u32>(g_seed >> 11);
}
float rnd_float(float lo, float hi) { return lo + (hi - lo) * static_cast<float>(rnd() & 0xFFFFFF) / 16777216.0f; }
// A single's bits: mostly ordinary values, sometimes zeros, denormals, infinities,
// NaNs and values whose products overflow or underflow.
u32 rnd_single() {
  const u32 r = rnd() % 100;
  if (r < 80) {
    float f = rnd_float(-4.0f, 4.0f);
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
  }
  const u32 sign = rnd() & 0x80000000u;
  switch (r % 7) {
    case 0: return sign;                                    // +-0
    case 1: return sign | (rnd() & 0x7FFFFF);               // denormal
    case 2: return sign | 0x7F800000u;                      // infinity
    case 3: return sign | 0x7FC00000u | (rnd() & 0x3FFFFF);  // quiet NaN
    case 4: return sign | 0x7F800001u | (rnd() & 0x1FFFFF);  // signaling NaN
    case 5: return sign | ((0xF0u + rnd() % 15) << 23) | (rnd() & 0x7FFFFF);  // huge
    default: return sign | ((1u + rnd() % 20) << 23) | (rnd() & 0x7FFFFF);    // tiny
  }
}
std::uint64_t rnd_double() {  // an FPR half: a single as lfs converts it, or any bits
  if (rnd() & 1) return ppc::fp::single_to_double(rnd_single());
  return std::uint64_t(rnd()) << 32 | rnd();
}

constexpr u32 kRet = 0x80001800;  // where the call returns (blr), outside the function

// Runs the original in the interpreter, from c.pc until it returns to LR.
void interpret(ppc::Cpu& c) {
  const u32 ret = c.lr & ~3u;
  int steps = 0;
  while (c.pc != ret) {
    ppc::step(c);
    if (++steps > 100000) {
      std::printf("FAIL: the original did not return\n");
      std::exit(1);
    }
  }
}
int g_fallbacks = 0;
void counted_original(ppc::Cpu& c) {
  ++g_fallbacks;
  interpret(c);
}

struct Snapshot {
  ppc::Cpu cpu;
  std::vector<u8> mem;
  std::vector<Store> stores;
};

int hle_index_at(u32 address) {
  const auto& t = hle_table();
  for (std::size_t k = 0; k < t.size(); ++k)
    if (t[k].address == address && t[k].kind == HleKind::Native) return static_cast<int>(k);
  return -1;
}

bool same(const ppc::Cpu& a, const ppc::Cpu& b, std::string* what) {
  char buf[160];
  for (int r = 0; r < 32; ++r) {
    if (a.gpr[r] != b.gpr[r]) {
      std::snprintf(buf, sizeof buf, "r%d %08X vs %08X", r, a.gpr[r], b.gpr[r]);
      *what = buf;
      return false;
    }
    if (a.fpr[r].ps0 != b.fpr[r].ps0 || a.fpr[r].ps1 != b.fpr[r].ps1) {
      std::snprintf(buf, sizeof buf, "f%d %016llX/%016llX vs %016llX/%016llX", r,
                    static_cast<unsigned long long>(a.fpr[r].ps0), static_cast<unsigned long long>(a.fpr[r].ps1),
                    static_cast<unsigned long long>(b.fpr[r].ps0), static_cast<unsigned long long>(b.fpr[r].ps1));
      *what = buf;
      return false;
    }
  }
  if (a.cr != b.cr || a.lr != b.lr || a.ctr != b.ctr || a.pc != b.pc || a.ticks != b.ticks || a.fpscr != b.fpscr ||
      a.msr != b.msr || a.xer() != b.xer()) {
    std::snprintf(buf, sizeof buf, "cr %08X/%08X pc %08X/%08X ticks %llu/%llu", a.cr, b.cr, a.pc, b.pc,
                  static_cast<unsigned long long>(a.ticks), static_cast<unsigned long long>(b.ticks));
    *what = buf;
    return false;
  }
  return true;
}

// fn_803EB000: weights and bone indices at r3, bones at r5, a matrix at r6, the
// destination and header at r14 (the write-gather pipe, or memory).
int check_803EB000(Machine& m, Recorder& io, int trials) {
  constexpr u32 kFn = 0x803EB000;
  const int index = hle_index_at(kFn);
  if (index < 0) {
    std::printf("FAIL: no native implementation registered at %08X\n", kFn);
    return 1;
  }
  constexpr u32 kArgs = 0x81000000, kMatrix = 0x81000100, kDest = 0x81000200, kBones = 0x81010000,
                kOut = 0x81020000, kWin = 0x81000000, kWinEnd = 0x81030000;
  ppc::Cpu& c = m.cpu();
  u8* mem1 = m.mem1();
  auto host = [&](u32 ea) { return mem1 + (ea - 0x80000000u); };
  int failures = 0, one = 0, fallbacks_expected = 0;
  g_fallbacks = 0;
  for (int t = 0; t < trials; ++t) {
    // Operands.
    for (u32 k = 0; k < 3; ++k) put_be32(host(kArgs + 4 * k), rnd_single());
    if (rnd() % 4 == 0) put_be32(host(kArgs + 4), rnd() & 0x80000000u);  // the second weight +-0: one matrix
    for (u32 k = 0; k < 16; ++k) put_be32(host(kMatrix + 4 * k), rnd_single());
    for (u32 k = 0; k < 256 * 16; ++k) put_be32(host(kBones + 4 * k), rnd() % 8 ? rnd_single() : rnd());
    const bool to_memory = rnd() % 8 == 0;
    put_be32(host(kDest), to_memory ? kOut + 4 * (rnd() % 8) : 0xCC008000u);
    put_be32(host(kDest + 4), rnd());
    for (int r = 0; r < 32; ++r) c.gpr[r] = rnd();
    for (int r = 0; r < 32; ++r) c.fpr[r] = {rnd_double(), rnd_double()};
    if (rnd() % 2) c.fpr[0].ps0 = 0;  // usually finite
    c.gpr[1] = 0x81200000;
    c.gpr[3] = kArgs;
    c.gpr[5] = kBones;
    c.gpr[6] = kMatrix;
    c.gpr[14] = kDest;
    c.cr = rnd();
    c.lr = kRet;
    c.ctr = rnd();
    c.msr = ppc::msr::FP;
    c.fpscr = ppc::fpscr::NI;
    c.spr[ppc::spr::GQR0] = 0;  // floats
    // States it is not written for: the original runs (and must give the same).
    bool expect_fallback = false;
    switch (rnd() % 64) {
      case 0: c.fpscr = ppc::fpscr::NI | 1; expect_fallback = true; break;  // round toward zero
      case 1: c.fpscr = 0; expect_fallback = true; break;                    // NI clear
      case 2: c.spr[ppc::spr::GQR0] = 0x00070007; expect_fallback = true; break;  // s16 quantization
      default: break;
    }
    fallbacks_expected += expect_fallback;
    c.ticks = 1000000 + rnd() % 1000;
    c.pc = kFn;

    const ppc::Cpu before = c;
    std::vector<u8> win(host(kWin), host(kWinEnd));
    // The original, in the interpreter.
    io.log.clear();
    interpret(c);
    Snapshot ref{c, std::vector<u8>(host(kWin), host(kWinEnd)), io.log};
    // The native implementation, called as translated code calls it.
    c = before;
    std::memcpy(host(kWin), win.data(), win.size());
    io.log.clear();
    hle_call(c, index, &counted_original);
    std::string what;
    bool ok = same(ref.cpu, c, &what);
    if (ok && ref.stores != io.log) {
      ok = false;
      what = "stores differ (" + std::to_string(ref.stores.size()) + " vs " + std::to_string(io.log.size()) + ")";
    }
    if (ok && std::memcmp(ref.mem.data(), host(kWin), ref.mem.size()) != 0) {
      ok = false;
      what = "memory differs";
    }
    if (!ok && ++failures <= 10) std::printf("trial %d: %s\n", t, what.c_str());
    if (ref.cpu.ticks - before.ticks == 66) ++one;
  }
  std::printf("fn_803EB000: %d trials (%d with one matrix), %d ran the original (%d expected), %d differences\n", trials,
              one, g_fallbacks, fallbacks_expected, failures);
  if (g_fallbacks != fallbacks_expected) {
    std::printf("FAIL: the original ran %d times, expected %d\n", g_fallbacks, fallbacks_expected);
    return 1;
  }
  return failures ? 1 : 0;
}

// Safe points that count, make the next one due a few instructions later, and
// now and then stand for an interrupt handler that ran and came back (the time
// it took added to c.ticks, nothing else changed). The reference interpreter has
// none, so the state must not depend on them but for those ticks.
struct QuietHooks : ppc::Hooks {
  std::uint64_t safepoints = 0, handlers = 0;
  Recorder* io = nullptr;
  std::uint64_t time_base(ppc::Cpu&) override { return 0; }
  void set_time_base(ppc::Cpu&, std::uint64_t) override {}
  u32 decrementer(ppc::Cpu&) override { return 0; }
  void set_decrementer(ppc::Cpu&, u32) override {}
  void msr_changed(ppc::Cpu&, u32) override {}
  void icbi(ppc::Cpu&, u32) override {}
  void system_call(ppc::Cpu&) override {}
  void safepoint(ppc::Cpu& c) override {
    ++safepoints;
    if (rnd() % 4 == 0) {
      const std::uint64_t ran = 1 + rnd() % 5000;
      c.ticks += ran;
      io->bias += ran;
      ++handlers;
    }
    c.next_event = c.ticks + 1 + rnd() % 200;
  }
};

// fn_803EA3EC (Q6): the skinning loop, n matrices from the weights at r3, with
// fn_803EB000 inside; the pipe's address and the two headers at 0x8062C750.
int check_803EA3EC(Machine& m, Recorder& io, int trials) {
  constexpr u32 kFn = 0x803EA3EC, kTable = 0x8062C750;
  const int index = hle_index_at(kFn);
  if (index < 0) {
    std::printf("FAIL: no native implementation registered at %08X\n", kFn);
    return 1;
  }
  constexpr u32 kArgs = 0x81000000, kMatrix = 0x81000400, kBones = 0x81010000, kOut = 0x81020000,
                kStack = 0x81028000, kWin = 0x81000000, kWinEnd = 0x81030000;
  ppc::Cpu& c = m.cpu();
  u8* mem1 = m.mem1();
  auto host = [&](u32 ea) { return mem1 + (ea - 0x80000000u); };
  QuietHooks hooks;
  hooks.io = &io;
  int failures = 0, fallbacks_expected = 0;
  std::uint64_t matrices = 0;
  g_fallbacks = 0;
  for (int t = 0; t < trials; ++t) {
    const u32 n = 1 + rnd() % 8;
    matrices += n;
    for (u32 k = 0; k < 4 * n; ++k) put_be32(host(kArgs + 4 * k), rnd_single());
    for (u32 k = 0; k < n; ++k)
      if (rnd() % 4 == 0) put_be32(host(kArgs + 16 * k + 4), rnd() & 0x80000000u);  // one matrix
    for (u32 k = 0; k < 16; ++k) put_be32(host(kMatrix + 4 * k), rnd_single());
    for (u32 k = 0; k < 256 * 16; ++k) put_be32(host(kBones + 4 * k), rnd() % 8 ? rnd_single() : rnd());
    const bool to_memory = rnd() % 8 == 0;
    put_be32(host(kTable), to_memory ? kOut + 4 * (rnd() % 8) : 0xCC008000u);
    put_be32(host(kTable + 4), rnd());
    put_be32(host(kTable + 8), rnd());
    for (int r = 0; r < 32; ++r) c.gpr[r] = rnd();
    for (int r = 0; r < 32; ++r) c.fpr[r] = {rnd_double(), rnd_double()};
    if (rnd() % 2) c.fpr[0].ps0 = 0;
    if (rnd() % 2) c.fpr[4].ps0 = 0;
    c.gpr[1] = kStack;
    c.gpr[3] = kArgs;
    c.gpr[4] = n;
    c.gpr[5] = kBones;
    c.gpr[6] = kMatrix;
    c.cr = rnd();
    c.lr = kRet;
    c.ctr = rnd();
    c.msr = ppc::msr::FP;
    c.fpscr = ppc::fpscr::NI;
    c.spr[ppc::spr::GQR0] = 0;
    bool expect_fallback = false;
    if (rnd() % 64 == 0) {
      c.fpscr = ppc::fpscr::NI | 1;
      expect_fallback = true;
    }
    fallbacks_expected += expect_fallback;
    c.ticks = 1000000 + rnd() % 1000;
    c.pc = kFn;
    c.hooks = nullptr;
    const ppc::Cpu before = c;
    std::vector<u8> win(host(kWin), host(kWinEnd));
    io.log.clear();
    interpret(c);
    Snapshot ref{c, std::vector<u8>(host(kWin), host(kWinEnd)), io.log};
    c = before;
    c.hooks = &hooks;
    c.next_event = rnd() % 2 ? c.ticks : ~0ull;  // safe points due at once (then every few instructions), or never
    std::memcpy(host(kWin), win.data(), win.size());
    io.log.clear();
    io.bias = 0;
    hle_call(c, index, &counted_original);
    c.hooks = nullptr;
    c.next_event = ref.cpu.next_event;
    c.ticks -= io.bias;  // the handlers' time
    io.bias = 0;
    std::string what;
    bool ok = same(ref.cpu, c, &what);
    if (ok && ref.stores != io.log) {
      ok = false;
      what = "stores differ (" + std::to_string(ref.stores.size()) + " vs " + std::to_string(io.log.size()) + ")";
    }
    if (ok && std::memcmp(ref.mem.data(), host(kWin), ref.mem.size()) != 0) {
      ok = false;
      what = "memory differs";
    }
    if (!ok && ++failures <= 10) std::printf("trial %d (n %u): %s\n", t, n, what.c_str());
  }
  std::printf("fn_803EA3EC: %d trials (%llu matrices, %llu safe points, %llu with a handler), %d ran the original "
              "(%d expected), %d differences\n",
              trials, static_cast<unsigned long long>(matrices), static_cast<unsigned long long>(hooks.safepoints),
              static_cast<unsigned long long>(hooks.handlers), g_fallbacks, fallbacks_expected, failures);
  if (g_fallbacks != fallbacks_expected) {
    std::printf("FAIL: the original ran %d times, expected %d\n", g_fallbacks, fallbacks_expected);
    return 1;
  }
  return failures ? 1 : 0;
}

}  // namespace

int main(int argc, char** argv) {
  const char* env = std::getenv("MVP_DISC");
  std::string disc_path = env && *env ? env : MVP_DISC_DEFAULT;
  std::error_code ec;
  if (disc_path.empty() || !std::filesystem::exists(disc_path, ec)) {
    std::printf("SKIP: needs MVP_DISC\n");
    return 77;
  }
  std::string err;
  auto disc = vfs::open_disc(disc_path, true, &err);
  if (!disc) {
    std::printf("FAIL: %s\n", err.c_str());
    return 1;
  }
  const int trials = argc > 1 ? std::atoi(argv[1]) : 20000;
  Machine m;
  m.load_dol(disc->dol);
  Recorder io;
  io.c = &m.cpu();
  m.memory().io = &io;
  int rc = check_803EB000(m, io, trials);
  rc |= check_803EA3EC(m, io, trials / 10);
  std::printf(rc ? "FAIL\n" : "PASS\n");
  return rc;
}
