// Data-free end-to-end test of the WebAssembly emitter (E2; Emscripten build
// only): the synthetic program (translator/tests/synth.h) is translated to a
// wasm module in-process, as the player's browser will translate the game,
// compiled and instantiated into the runtime, and run in lockstep with the
// reference interpreter. The checks are those of test_emit_lockstep (the C++
// emitter): same results, same unwinds, in-frame resumes, resume entries and
// interpreted instructions. Both helper modes run: inline code, and every
// non-branch instruction through the runtime. A second program checks the
// inline floating-point moves and conversions on edge-case values. The synthetic
// program also runs with stops at its frame polls (poll_stop.h, E5c).
#include <doctest/doctest.h>

#include <cstdio>
#include <cstring>
#include <iterator>
#include <map>

#include "../../ppc/tests/encode.h"
#include "../../translator/tests/synth.h"
#include "common/bytes.h"
#include "poll_stop.h"
#include "runtime/hle.h"
#include "runtime/hw.h"
#include "runtime/lockstep.h"
#include "runtime/machine.h"
#include "runtime/native.h"
#include "runtime/wasm_host.h"
#include "translator/wasm_emit.h"

namespace {
std::string g_reported;
void capture(const std::string& s) { g_reported += s; }
}  // namespace

using namespace mvp;
using namespace mvp::rt;

// The programs start with MSR[FP] clear (MSR 0), so their first FPU instruction
// raises floating-point unavailable (E3a). The handler at 0x800 sets FP in SRR1
// and returns: the reference interpreter runs it in line, the translated side
// runs it to its rfi from the translated frame (rt.fp_unavailable) and goes on,
// as in test_emit_lockstep.
static void install_fp_handler(Machine& m) {
  using namespace mvp::ppc::enc;
  const u32 handler[] = {
      SPR(467, 3, 272),     // mtsprg0 r3
      SPR(339, 3, 27),      // mfsrr1 r3
      D(24, 3, 3, 0x2000),  // ori r3, r3, MSR[FP]
      SPR(467, 3, 27),      // mtsrr1 r3
      SPR(339, 3, 272),     // mfsprg0 r3
      X(19, 0, 0, 0, 50),   // rfi
  };
  for (std::size_t k = 0; k < std::size(handler); ++k) put_be32(m.mem1() + 0x800 + 4 * k, handler[k]);
  m.set_real_mode(true);  // the vector runs at its physical address
}

// V4 (`fixed`): the module has the runtime's addresses as constants, MEM1's
// (the native machine's) included (xlat::WasmFixed), as the browser emits it.
// V4 (`fast`): calls reach functions without resume labels and resumes their
// resumable copies (WasmEmitOptions::fast_entries): no unwind ends in a frame,
// so the in-frame resumes become unwinds to the dispatcher and resume entries.
// V4 (`loops`): with fast entries, loops as wasm loops where they nest (structured_loops).
static void run_case(bool inline_ops, bool mem1 = false, bool fixed = false, bool fast = false, bool loops = false) {
  xlat::synth::Program prog = xlat::synth::make();
  auto syms = xlat::SymbolTable::parse(prog.symbols);
  auto dol = vfs::Dol::parse(prog.dol);
  REQUIRE(dol);
  xlat::ProgramCfg cfg = xlat::build_cfg(*dol, syms);
  REQUIRE(cfg.errors == 0);

  xlat::WasmEmitOptions opt;
  const auto& table = hle_table();
  for (std::size_t k = 0; k < table.size(); ++k)
    opt.hle.push_back({table[k].name, table[k].address, static_cast<int>(k), table[k].kind == HleKind::Before});
  opt.precise_io = true;  // exact registers at the MMIO stop
  opt.inline_ops = inline_ops;
  opt.fast_entries = fast;
  opt.structured_loops = loops;
  Machine ref, nat;
  if (fixed) {
    set_wasm_mem1(nat.mem1(), Machine::kMem1Size);
    opt.fixed = wasm_fixed_addrs();
    REQUIRE(opt.fixed.mem1_limit == Machine::kMem1Size - 7);
  }
  xlat::WasmModule mod;
  std::string err;
  REQUIRE_MESSAGE(xlat::emit_wasm(cfg, opt, &mod, &err), err);
  MESSAGE(mod.bytes.size(), " bytes, ", mod.stats.instructions, " instructions (", mod.stats.inline_insns, " inline, ",
          mod.stats.helper_insns, " through the runtime, ", mod.stats.branch_insns, " control), ",
          mod.stats.structured_functions, " with structured loops");
  if (loops) CHECK(mod.stats.structured_functions > 0);

  FixedHooks hr, hn;
  StopIo ir, in;
  ir.cpu = &ref.cpu();
  in.cpu = &nat.cpu();
  ref.memory().io = &ir;
  nat.memory().io = &in;
  ref.cpu().hooks = &hr;
  nat.cpu().hooks = &hn;
  ref.load_dol(*dol);
  nat.load_dol(*dol);
  install_fp_handler(ref);
  install_fp_handler(nat);
  hn.run_handlers = true;
  WasmLoadStats ls;
  REQUIRE_MESSAGE(load_wasm_module(mod, &ls, &err), err);
  REQUIRE_FALSE(g_native.empty());
  // Q6: the helpers' own MEM1 path (env.mem1), against the interpreter's page table.
  struct Mem1 {
    explicit Mem1(Machine* m) { if (m) set_wasm_mem1(m->mem1(), Machine::kMem1Size); }
    ~Mem1() { set_wasm_mem1(nullptr, 0); }
  } mem1_view(mem1 ? &nat : nullptr);

  std::uint64_t unwinds = g_unwinds, inframe = g_inframe_resumes, entries = g_resume_entries,
                interpreted = g_interpreted;
  g_reported.clear();
  g_report = &capture;
  LockstepResult r = run_lockstep(ref, nat, cfg, 1);
  INFO(r.message);
  REQUIRE(r.pass);
  CHECK(r.stop.write);
  CHECK(r.stop.addr == 0xCC000000u);
  CHECK(r.events > 20);
  if (fast) {
    CHECK(g_unwinds == unwinds + 4);
    CHECK(g_inframe_resumes == inframe);
    CHECK(g_resume_entries == entries + 7);  // the two in-frame resumes, each a level further up
  } else {
    CHECK(g_unwinds == unwinds + 2);
    CHECK(g_inframe_resumes == inframe + 2);
    CHECK(g_resume_entries == entries + 3);
  }
  // main's two instructions after jumper's return, and the six of the handler.
  CHECK(g_interpreted == interpreted + 2 + 6);
  CHECK(hr.exceptions == 1);
  CHECK(hn.exceptions == 1);
  CHECK_FALSE(g_unwinding);
  CHECK(g_reported.find("x=42") == 0);
  auto want = xlat::synth::expected_results();
  for (std::size_t k = 0; k < want.size(); ++k) {
    u32 got = be32(nat.mem1() + (prog.results & 0x01FFFFFF) + 4 * k);
    CHECK_MESSAGE(got == want[k], "result ", k);
  }
}

TEST_CASE("wasm-emitted synthetic program matches the interpreter in lockstep (inline code)") { run_case(true); }

TEST_CASE("wasm-emitted synthetic program matches the interpreter in lockstep (runtime helpers only)") {
  run_case(false);
}

TEST_CASE("Q6: the same with the memory helpers' own MEM1 path") {
  run_case(true, true);
  run_case(false, true);
}

TEST_CASE("V4: the same with the runtime's addresses and MEM1 fixed in the module") {
  run_case(true, true, true);
  run_case(false, true, true);
}

TEST_CASE("V4: the same with fast entries and resumable copies (as the browser emits it)") {
  run_case(true, true, true, true);
  run_case(false, true, true, true);
  run_case(true, false, false, true);
}

TEST_CASE("V4: the same with fast entries and structured loops") {
  run_case(true, true, true, true, true);
  run_case(false, true, true, true, true);
  run_case(true, false, false, true, true);
}

TEST_CASE("V4: a module made for other runtime addresses is refused") {
  xlat::synth::Program prog = xlat::synth::make();
  auto syms = xlat::SymbolTable::parse(prog.symbols);
  auto dol = vfs::Dol::parse(prog.dol);
  REQUIRE(dol);
  xlat::ProgramCfg cfg = xlat::build_cfg(*dol, syms);
  REQUIRE(cfg.errors == 0);
  set_wasm_mem1(nullptr, 0);
  const xlat::WasmFixed here = wasm_fixed_addrs();
  CHECK(here.mem1_limit == 0);  // no MEM1 given: not fixed
  for (int k = 0; k < 3; ++k) {
    xlat::WasmEmitOptions opt;
    opt.fixed = here;
    if (k == 0) opt.fixed.unwinding += 4;
    if (k == 1) opt.fixed.gp_pages += 64;
    if (k == 2) {
      opt.fixed.mem1_base = 0x100000;  // MEM1 fixed where the runtime has none
      opt.fixed.mem1_limit = Machine::kMem1Size - 7;
    }
    xlat::WasmModule mod;
    std::string err;
    REQUIRE_MESSAGE(xlat::emit_wasm(cfg, opt, &mod, &err), err);
    CHECK(mod.fixed == opt.fixed);
    WasmLoadStats ls;
    CHECK_FALSE(load_wasm_module(mod, &ls, &err));
    CHECK(err.find("other runtime addresses") != std::string::npos);
  }
}

// E5c: in wasm-emitted code too, the frame poll stops the run before it consumes
// anything, and the guest runs exactly as without the stops (poll_stop.h).
TEST_CASE("wasm-emitted frame poll that must wait for its input stops the run there, invisibly") {
  xlat::synth::Program prog = xlat::synth::make();
  auto syms = xlat::SymbolTable::parse(prog.symbols);
  auto dol = vfs::Dol::parse(prog.dol);
  REQUIRE(dol);
  xlat::ProgramCfg cfg = xlat::build_cfg(*dol, syms);
  REQUIRE(cfg.errors == 0);
  xlat::WasmEmitOptions opt;
  const auto& table = hle_table();
  for (std::size_t k = 0; k < table.size(); ++k)
    opt.hle.push_back({table[k].name, table[k].address, static_cast<int>(k), table[k].kind == HleKind::Before});
  xlat::WasmModule mod;
  std::string err;
  REQUIRE_MESSAGE(xlat::emit_wasm(cfg, opt, &mod, &err), err);
  WasmLoadStats ls;
  REQUIRE_MESSAGE(load_wasm_module(mod, &ls, &err), err);
  poll_stop::Outcome plain = poll_stop::run(*dol, prog.results, false);
  poll_stop::Outcome stopped = poll_stop::run(*dol, prog.results, true);
  CHECK(plain.runs == 0);
  CHECK(plain.frames == 4);
  CHECK(plain.results == xlat::synth::expected_results());
  CHECK(stopped.stops == 4);
  CHECK(stopped.runs == 4);
  CHECK(stopped.unwinds == plain.unwinds + 4);
  CHECK(stopped.polls == plain.polls);
  CHECK(stopped.ticks == plain.ticks);
  CHECK(stopped.regs == plain.regs);
  CHECK(stopped.results == plain.results);
  CHECK_FALSE(g_unwinding);
}

// The emitter's inline floating-point moves and conversions (lfs's single to
// double, stfs's double to single with its denormalizing and NaN cases, lfd,
// stfd, stfiwx, fneg, fnabs, ps_merge10) on edge-case values, against the
// interpreter: one loop over records of inputs, every result stored, and the
// whole of MEM1 and every register compared where both stop (an MMIO store).
namespace {

struct Rng {
  std::uint64_t s;
  std::uint64_t next() {
    s ^= s >> 12;
    s ^= s << 25;
    s ^= s >> 27;
    return s * 0x2545F4914F6CDD1Dull;
  }
};

struct FpProgram {
  std::vector<std::uint8_t> dol;
  std::string symbols;
};

FpProgram make_fp_program() {
  using namespace mvp::ppc::enc;
  constexpr u32 kText = 0x80003100, kIn = 0x80200000, kRecords = 96;
  const u32 singles[] = {0x00000000, 0x80000000, 0x3F800000, 0xBF800000, 0x00000001, 0x80000001, 0x007FFFFF,
                         0x00400000, 0x00800000, 0x7F7FFFFF, 0xFF7FFFFF, 0x7F800000, 0xFF800000, 0x7FC00000,
                         0xFFC00000, 0x7F800001, 0x7FBFFFFF, 0xFFFFFFFF, 0x7FFFFFFF, 0x34000000};
  const std::uint64_t doubles[] = {
      0x0000000000000000ull, 0x8000000000000000ull, 0x3FF0000000000000ull, 0x7FF0000000000000ull,
      0xFFF0000000000000ull, 0x7FF8000000000000ull, 0x7FF0000000000001ull, 0xFFF4000012345678ull,
      0x0000000000000001ull, 0x800FFFFFFFFFFFFFull, 0x3690000000000000ull, 0x36A0000000000000ull,
      0x36A8000000000000ull, 0x36B0000000000001ull, 0x3700000000000000ull, 0x37F0000000000000ull,
      0x3800000000000000ull, 0x380FFFFFFFFFFFFFull, 0x3810000000000000ull, 0x47EFFFFFE0000000ull,
      0x47EFFFFFFFFFFFFFull, 0x47F0000000000000ull, 0xB80FFFFFFFFFFFFFull, 0xC7EFFFFFF0000000ull};
  Rng r{0x5EEDF10A7ull};
  std::vector<u32> data;
  for (u32 k = 0; k < kRecords; ++k) {
    u32 s = k < std::size(singles) ? singles[k] : static_cast<u32>(r.next() >> 32);
    std::uint64_t d = k < std::size(doubles) ? doubles[k] : r.next();
    if (k >= std::size(doubles) && (k & 1)) {
      // Exponents around the single denormal and normal limits.
      std::uint64_t e = 860 + (r.next() % 50);
      d = (d & 0x800FFFFFFFFFFFFFull) | (e << 52);
    }
    data.push_back(s);
    data.push_back(0);
    data.push_back(static_cast<u32>(d >> 32));
    data.push_back(static_cast<u32>(d));
  }
  std::vector<u32> w = {
      D(15, 3, 0, 0x8020),   // lis r3, 0x8020 (inputs)
      D(15, 4, 0, 0x8030),   // lis r4, 0x8030 (results, plain RAM)
      D(14, 5, 0, kRecords), // li r5, records
      SPR(467, 5, 9),        // mtctr r5
      D(14, 6, 0, 12),       // li r6, 12
  };
  const u32 loop = kText + 4 * static_cast<u32>(w.size());
  for (u32 x : {D(48, 1, 3, 0),          // lfs f1, 0(r3)
                D(54, 1, 4, 0),          // stfd f1, 0(r4)
                D(50, 2, 3, 8),          // lfd f2, 8(r3)
                D(52, 2, 4, 8),          // stfs f2, 8(r4)
                X(31, 2, 4, 6, 983),     // stfiwx f2, r4, r6 (12)
                X(63, 3, 0, 2, 40),      // fneg f3, f2
                D(54, 3, 4, 16),         // stfd f3, 16(r4)
                X(63, 4, 0, 1, 136),     // fnabs f4, f1
                D(54, 4, 4, 24),         // stfd f4, 24(r4)
                X(4, 5, 1, 2, 592),      // ps_merge10 f5, f1, f2
                PSQ(60, 5, 4, 32, 0, 0), // psq_st f5, 32(r4), 0, qr0
                D(14, 3, 3, 16),         // addi r3, r3, 16
                D(14, 4, 4, 48)})        // addi r4, r4, 48
    w.push_back(x);
  w.push_back(B(16, 0, static_cast<s32>(loop - (kText + 4 * static_cast<u32>(w.size())))));  // bdnz loop
  w.push_back(D(15, 9, 0, static_cast<s32>(static_cast<std::int16_t>(0xCC00))));  // lis r9, 0xCC00
  w.push_back(D(36, 0, 9, 0));            // stw r0, 0(r9): MMIO, the stop
  w.push_back(X(19, 20, 0, 0, 16));       // blr
  const std::size_t text_bytes = 4 * w.size(), data_bytes = 4 * data.size();
  std::vector<std::uint8_t> d(0x100 + text_bytes + data_bytes, 0);
  auto set = [&](int slot, u32 off, u32 addr, u32 size) {
    put_be32(&d[4 * static_cast<std::size_t>(slot)], off);
    put_be32(&d[0x48 + 4 * static_cast<std::size_t>(slot)], addr);
    put_be32(&d[0x90 + 4 * static_cast<std::size_t>(slot)], size);
  };
  set(0, 0x100, kText, static_cast<u32>(text_bytes));
  set(7, static_cast<u32>(0x100 + text_bytes), kIn, static_cast<u32>(data_bytes));
  put_be32(&d[0xE0], kText);
  for (std::size_t k = 0; k < w.size(); ++k) put_be32(&d[0x100 + 4 * k], w[k]);
  for (std::size_t k = 0; k < data.size(); ++k) put_be32(&d[0x100 + text_bytes + 4 * k], data[k]);
  char line[128];
  std::snprintf(line, sizeof line, "main = .text:0x%08X; // type:function size:0x%zX\n", kText, text_bytes);
  return FpProgram{std::move(d), line};
}

}  // namespace

TEST_CASE("wasm-emitted floating-point moves and conversions match the interpreter on edge cases") {
  FpProgram prog = make_fp_program();
  auto syms = xlat::SymbolTable::parse(prog.symbols);
  auto dol = vfs::Dol::parse(prog.dol);
  REQUIRE(dol);
  xlat::ProgramCfg cfg = xlat::build_cfg(*dol, syms);
  REQUIRE(cfg.errors == 0);
  xlat::WasmEmitOptions opt;
  opt.precise_io = true;
  xlat::WasmModule mod;
  std::string err;
  REQUIRE_MESSAGE(xlat::emit_wasm(cfg, opt, &mod, &err), err);
  CHECK(mod.stats.helper_insns == 0);  // psq_st too is inline since E2d

  Machine ref, nat;
  FixedHooks hr, hn;
  StopIo ir, in;
  ir.cpu = &ref.cpu();
  in.cpu = &nat.cpu();
  ref.memory().io = &ir;
  nat.memory().io = &in;
  ref.cpu().hooks = &hr;
  nat.cpu().hooks = &hn;
  ref.load_dol(*dol);
  nat.load_dol(*dol);
  install_fp_handler(ref);
  install_fp_handler(nat);
  hn.run_handlers = true;
  WasmLoadStats ls;
  REQUIRE_MESSAGE(load_wasm_module(mod, &ls, &err), err);
  LockstepResult r = run_lockstep(ref, nat, cfg, 1);
  INFO(r.message);
  REQUIRE(r.pass);
  CHECK(r.stop.addr == 0xCC000000u);
  CHECK(hr.exceptions == 1);  // the first lfs; MSR[FP] stays set after
  CHECK(hn.exceptions == 1);
  // Spot checks against the conversion rules (fp.h): lfs of the smallest
  // single denormal, stfs of a NaN keeps the top fraction bits.
  CHECK(be64(nat.mem1() + 0x300000 + 48 * 4) == 0x36A0000000000000ull);
  CHECK(be32(nat.mem1() + 0x300000 + 48 * 7 + 8) == 0xFFA00000u);
}

// Q4: stores to the write-gather pipe from translated code. The module appends a
// store that leaves the burst unfinished itself, sends one that completes it (or
// reaches into the next) to rt.wgp_store, takes every address of the pipe's 4 KiB
// block (a psq_st writes its second word to 0xCC008004), and psq_st and stfd
// write their pairs through their own fast paths. The FIFO in memory and its
// write pointer must be those of the same bytes written through Hardware::write.
namespace {

struct PipeRig {
  Machine m;
  FixedHooks hooks;
  Hardware hw{&m.cpu(), m.mem1(), Machine::kMem1Size};
  static constexpr u32 kFifo = 0x00100000, kFifoEnd = 0x0010FFE0;
  PipeRig() {
    m.memory().io = &hw;
    m.cpu().hooks = &hooks;
    hw.write(0xCC00300C, kFifo, 4);  // the CPU FIFO; the GP not linked, reads off
    hw.write(0xCC003010, kFifoEnd, 4);
    hw.write(0xCC003014, kFifo, 4);
    for (int k = 0; k < 100000; ++k) hw.read(0xCC003000, 4);  // a full access log: translated code may append
  }
};

}  // namespace

TEST_CASE("wasm-emitted stores to the write-gather pipe equal Hardware::write's") {
  using namespace mvp::ppc::enc;
  constexpr u32 kText = 0x80003100, kData = 0x80200000;
  std::vector<u32> w = {
      D(15, 4, 0, static_cast<s32>(static_cast<std::int16_t>(0xCC01))),  // lis r4, 0xCC01
      D(14, 4, 4, -0x8000),                                              // addi r4, r4, -0x8000: the pipe
      D(15, 3, 0, 0x8020),                                               // lis r3, 0x8020 (data)
      D(48, 1, 3, 0),                                                    // lfs f1, 0(r3)
      D(48, 2, 3, 4),                                                    // lfs f2, 4(r3)
      X(4, 3, 1, 2, 528),                                                // ps_merge00 f3, f1, f2
      D(14, 5, 0, 0x61),                                                 // li r5, 0x61
      D(38, 5, 4, 0),                                                    // stb r5, 0(r4)
      D(15, 5, 0, 0x1234),                                               // lis r5, 0x1234
      D(24, 5, 5, 0x5678),                                               // ori r5, r5, 0x5678
      D(36, 5, 4, 0),                                                    // stw r5, 0(r4)
      D(44, 5, 4, 0),                                                    // sth r5, 0(r4)
      D(36, 5, 4, 4),                                                    // stw r5, 4(r4): 0xCC008004
      D(14, 6, 0, 40),                                                   // li r6, 40
      SPR(467, 6, 9),                                                    // mtctr r6
  };
  const u32 loop = kText + 4 * static_cast<u32>(w.size());
  for (u32 x : {D(36, 5, 4, 0),             // stw r5, 0(r4)
                D(14, 5, 5, 0x0101),        // addi r5, r5, 0x101
                D(44, 5, 4, 0),             // sth r5, 0(r4)
                D(38, 5, 4, 0),             // stb r5, 0(r4)
                PSQ(60, 3, 4, 0, 0, 0),     // psq_st f3, 0(r4), 0, qr0 (float pair)
                D(52, 1, 4, 0),             // stfs f1, 0(r4)
                D(54, 2, 4, 0),             // stfd f2, 0(r4)
                PSQ(60, 3, 4, 0, 1, 0)})    // psq_st f3, 0(r4), 1, qr0 (ps0 only)
    w.push_back(x);
  w.push_back(B(16, 0, static_cast<s32>(loop - (kText + 4 * static_cast<u32>(w.size())))));  // bdnz loop
  w.push_back(D(15, 9, 0, static_cast<s32>(static_cast<std::int16_t>(0xCC01))));  // lis r9, 0xCC01
  w.push_back(D(36, 0, 9, -0x1000));      // stw r0, -0x1000(r9): 0xCC00F000, outside the blocks: the stop
  w.push_back(X(19, 20, 0, 0, 16));       // blr
  const u32 floats[] = {0x3FC00000, 0xC2F6E979};  // 1.5, -123.456
  const std::size_t text_bytes = 4 * w.size();
  std::vector<std::uint8_t> d(0x100 + text_bytes + 8, 0);
  auto set = [&](int slot, u32 off, u32 addr, u32 size) {
    put_be32(&d[4 * static_cast<std::size_t>(slot)], off);
    put_be32(&d[0x48 + 4 * static_cast<std::size_t>(slot)], addr);
    put_be32(&d[0x90 + 4 * static_cast<std::size_t>(slot)], size);
  };
  set(0, 0x100, kText, static_cast<u32>(text_bytes));
  set(7, static_cast<u32>(0x100 + text_bytes), kData, 8);
  put_be32(&d[0xE0], kText);
  for (std::size_t k = 0; k < w.size(); ++k) put_be32(&d[0x100 + 4 * k], w[k]);
  put_be32(&d[0x100 + text_bytes], floats[0]);
  put_be32(&d[0x100 + text_bytes + 4], floats[1]);
  char line[128];
  std::snprintf(line, sizeof line, "main = .text:0x%08X; // type:function size:0x%zX\n", kText, text_bytes);
  auto syms = xlat::SymbolTable::parse(line);
  auto dol = vfs::Dol::parse(d);
  REQUIRE(dol);
  xlat::ProgramCfg cfg = xlat::build_cfg(*dol, syms);
  REQUIRE(cfg.errors == 0);
  xlat::WasmModule mod;
  std::string err;
  REQUIRE_MESSAGE(xlat::emit_wasm(cfg, {}, &mod, &err), err);
  WasmLoadStats ls;
  REQUIRE_MESSAGE(load_wasm_module(mod, &ls, &err), err);

  // The translated code, with the module's own pipe state (as the engines run it).
  PipeRig nat;
  nat.m.load_dol(*dol);
  nat.hw.set_wgp_state(wasm_wgp_state());
  ppc::Cpu& c = nat.m.cpu();
  c.msr = ppc::msr::FP;
  c.lr = 0;
  c.pc = kText;
  std::string why;
  try {
    run(c);
  } catch (const StopRun& s) {
    why = s.why;
  }
  CHECK(why.find("outside the hardware blocks") != std::string::npos);

  // The same values, byte by byte through Hardware::write.
  PipeRig ref;
  auto put = [&](u32 v, int size) { ref.hw.write(0xCC008000, v, size); };
  auto ftz = [](std::uint64_t b) {  // fp::double_to_single_ftz of an exact single
    return static_cast<u32>(b >> 32 & 0xC0000000u) | static_cast<u32>(b >> 29 & 0x3FFFFFFFu);
  };
  auto dbl = [](u32 s) {  // fp::single_to_double of a normal single
    float f;
    std::memcpy(&f, &s, 4);
    double x = f;
    std::uint64_t b;
    std::memcpy(&b, &x, 8);
    return b;
  };
  const std::uint64_t d1 = dbl(floats[0]), d2 = dbl(floats[1]);
  u32 r5 = 0x61;
  put(r5, 1);
  r5 = 0x12345678;
  put(r5, 4);
  put(r5 & 0xFFFF, 2);
  put(r5, 4);
  for (int k = 0; k < 40; ++k) {
    put(r5, 4);
    r5 += 0x101;
    put(r5 & 0xFFFF, 2);
    put(r5 & 0xFF, 1);
    put(ftz(d1), 4);
    put(ftz(d2), 4);
    put(ftz(d1), 4);
    put(static_cast<u32>(d2 >> 32), 4);
    put(static_cast<u32>(d2), 4);
    put(ftz(d1), 4);
  }
  const u32 wp = nat.hw.read(0xCC003014, 4);
  CHECK(wp == ref.hw.read(0xCC003014, 4));
  CHECK(wp == PipeRig::kFifo + 32 * 39);  // 11 + 40 x 31 bytes: 39 bursts
  CHECK(std::memcmp(nat.m.mem1() + PipeRig::kFifo, ref.m.mem1() + PipeRig::kFifo, wp - PipeRig::kFifo) == 0);
  CHECK(nat.hw.gp_bursts() == ref.hw.gp_bursts());
  // Every store counted once, wherever in the pipe's block it went.
  auto pipe_writes = [](const Hardware& hw) {
    std::uint64_t n = 0;
    for (const auto& [a, k] : hw.write_counts())
      if (a - 0xCC008000u < 0x1000u) n += k;
    return n;
  };
  CHECK(pipe_writes(nat.hw) == pipe_writes(ref.hw));
  CHECK(wasm_wgp_state()->fast_stores > 40 * 8);  // the module appended most of them itself
}

// V4: loop shapes for WasmEmitOptions::structured_loops, against the interpreter
// in lockstep, with fast entries: nested loops with an inner forward branch; a
// loop with a continue from its middle and a break out of it; a loop that calls
// a function; and two shapes that keep the dispatcher (a loop entered at its
// condition, and two loops that overlap).
namespace {

struct LoopProgram {
  std::vector<std::uint8_t> dol;
  std::string symbols;
};

LoopProgram make_loop_program() {
  using namespace mvp::ppc::enc;
  constexpr u32 kText = 0x80003100;
  std::vector<u32> w;
  std::map<std::string, u32> at;
  std::vector<std::pair<std::size_t, std::string>> fix;  // b/bl (26-bit) or bc (16-bit) to a label
  std::vector<std::pair<std::string, u32>> fns;
  auto here = [&] { return kText + 4 * static_cast<u32>(w.size()); };
  auto L = [&](const std::string& n) { at[n] = here(); };
  auto fn = [&](const std::string& n) {
    L(n);
    fns.push_back({n, here()});
  };
  auto b = [&](const std::string& n, u32 lk = 0) {
    fix.push_back({w.size(), n});
    w.push_back(I(0, 0, lk));
  };
  auto bc = [&](u32 bo, u32 bi, const std::string& n) {
    fix.push_back({w.size(), n});
    w.push_back(B(bo, bi, 0));
  };
  auto cmpwi = [&](u32 ra, s32 v) { w.push_back(D(11, 0, ra, v)); };  // cmpwi cr0, ra, v
  auto li = [&](u32 rt, s32 v) { w.push_back(D(14, rt, 0, v)); };
  auto addi = [&](u32 rt, u32 ra, s32 v) { w.push_back(D(14, rt, ra, v)); };
  auto add = [&](u32 rt, u32 ra, u32 rb) { w.push_back(XO(rt, ra, rb, 0, 266)); };
  auto blr = [&] { w.push_back(X(19, 20, 0, 0, 16)); };
  constexpr u32 kLt = 12, kGt = 12, kEq = 12, kNe = 4;  // BO: branch if true (12) / false (4)
  constexpr u32 biLt = 0, biGt = 1, biEq = 2;
  (void)kGt;
  (void)kEq;

  fn("main");
  w.push_back(D(15, 1, 0, 0x8040));   // lis r1, 0x8040 (stack)
  w.push_back(D(15, 20, 0, 0x8030));  // lis r20, results
  for (int k = 0; k < 5; ++k) {
    static const char* const names[] = {"nested", "brk", "rotated", "calls_in", "overlap"};
    b(names[k], 1);
    w.push_back(D(36, 3, 20, 4 * k));  // stw r3, 4k(r20)
  }
  w.push_back(D(15, 9, 0, static_cast<s32>(static_cast<std::int16_t>(0xCC00))));  // lis r9, 0xCC00
  w.push_back(D(36, 0, 9, 0));  // stw r0, 0(r9): MMIO, the stop
  L("spin");
  b("spin");

  // nested: r3 = sum over i < 5, j <= i of i*j, plus 1 for each odd j.
  fn("nested");
  li(3, 0);
  li(4, 0);
  L("n_outer");
  li(5, 0);
  L("n_inner");
  w.push_back(XO(6, 4, 5, 0, 235));  // mullw r6, r4, r5
  add(3, 3, 6);
  w.push_back(D(28, 5, 7, 1));       // andi. r7, r5, 1
  bc(kLt, biEq, "n_skip");           // beq n_skip
  addi(3, 3, 1);
  L("n_skip");
  addi(5, 5, 1);
  w.push_back(X(31, 0, 5, 4, 0));    // cmpw r5, r4
  bc(kNe, biGt, "n_inner");          // ble n_inner
  addi(4, 4, 1);
  cmpwi(4, 5);
  bc(kLt, biLt, "n_outer");          // blt n_outer
  blr();

  // brk: r3 = sum of the r4 in 1..20 that are not multiples of 4, plus 100 for each above 10.
  fn("brk");
  li(3, 0);
  li(4, 0);
  L("k_top");
  addi(4, 4, 1);
  cmpwi(4, 20);
  bc(kLt, biGt, "k_done");           // bgt k_done (break)
  w.push_back(D(28, 4, 5, 3));       // andi. r5, r4, 3
  bc(kLt, biEq, "k_top");            // beq k_top (continue, from the middle)
  cmpwi(4, 10);
  bc(kNe, biGt, "k_add");            // ble k_add (a label inside the loop)
  addi(3, 3, 100);
  L("k_add");
  add(3, 3, 4);
  b("k_top");
  L("k_done");
  blr();

  // rotated: r3 = 10 + 9 + ... + 1, the loop entered at its condition (dispatcher).
  fn("rotated");
  li(3, 0);
  li(4, 10);
  b("r_cond");
  L("r_body");
  add(3, 3, 4);
  addi(4, 4, -1);
  L("r_cond");
  cmpwi(4, 0);
  bc(kLt, biGt, "r_body");           // bgt r_body
  blr();

  // calls_in: r3 = sum of sq(i), i < 6, plus 1000 for each square of 10 or more, with a frame
  // and a call in the loop.
  fn("calls_in");
  w.push_back(D(37, 1, 1, -32));     // stwu r1, -32(r1)
  w.push_back(SPR(339, 0, 8));       // mflr r0
  w.push_back(D(36, 0, 1, 36));      // stw r0, 36(r1)
  w.push_back(D(36, 31, 1, 28));     // stw r31, 28(r1)
  w.push_back(D(36, 30, 1, 24));     // stw r30, 24(r1)
  li(31, 0);
  li(30, 0);
  L("c_loop");
  w.push_back(X(31, 31, 3, 31, 444));  // mr r3, r31
  b("sq", 1);
  cmpwi(3, 10);
  bc(kLt, biLt, "c_small");          // blt c_small (a label inside the loop)
  addi(30, 30, 1000);
  L("c_small");
  add(30, 30, 3);
  addi(31, 31, 1);
  cmpwi(31, 6);
  bc(kLt, biLt, "c_loop");           // blt c_loop
  w.push_back(X(31, 30, 3, 30, 444));  // mr r3, r30
  w.push_back(D(32, 0, 1, 36));      // lwz r0, 36(r1)
  w.push_back(SPR(467, 0, 8));       // mtlr r0
  w.push_back(D(32, 31, 1, 28));     // lwz r31, 28(r1)
  w.push_back(D(32, 30, 1, 24));     // lwz r30, 24(r1)
  addi(1, 1, 32);
  blr();

  fn("sq");
  w.push_back(XO(3, 3, 3, 0, 235));  // mullw r3, r3, r3
  blr();

  // overlap: loops [o_a, o_c] and [o_b, o_d] share segments without nesting (dispatcher).
  fn("overlap");
  li(3, 0);
  li(4, 0);
  li(5, 0);
  L("o_a");
  addi(3, 3, 1);
  L("o_b");
  addi(4, 4, 1);
  b("o_c");                          // (to the next instruction: o_c starts a segment)
  L("o_c");
  w.push_back(D(28, 4, 6, 1));       // andi. r6, r4, 1
  bc(kNe, biEq, "o_a");              // bne o_a
  b("o_d");
  L("o_d");
  addi(5, 5, 1);
  cmpwi(5, 5);
  bc(kLt, biLt, "o_b");              // blt o_b
  add(3, 3, 4);
  add(3, 3, 5);
  blr();
  fn("end");

  for (const auto& [idx, n] : fix) {
    const u32 t = at.at(n), from = kText + 4 * static_cast<u32>(idx);
    u32& x = w[idx];
    if ((x >> 26) == 18) x = (x & ~0x03FFFFFCu) | ((t - from) & 0x03FFFFFCu);
    else x = (x & ~0xFFFCu) | ((t - from) & 0xFFFCu);
  }
  const std::size_t text_bytes = 4 * w.size();
  std::vector<std::uint8_t> d(0x100 + text_bytes, 0);
  put_be32(&d[0], 0x100);
  put_be32(&d[0x48], kText);
  put_be32(&d[0x90], static_cast<u32>(text_bytes));
  put_be32(&d[0xE0], kText);
  for (std::size_t k = 0; k < w.size(); ++k) put_be32(&d[0x100 + 4 * k], w[k]);
  std::string syms;
  char line[160];
  for (std::size_t k = 0; k + 1 < fns.size(); ++k) {
    std::snprintf(line, sizeof line, "%s = .text:0x%08X; // type:function size:0x%X\n", fns[k].first.c_str(),
                  fns[k].second, fns[k + 1].second - fns[k].second);
    syms += line;
  }
  return LoopProgram{std::move(d), std::move(syms)};
}

}  // namespace

TEST_CASE("V4: loop shapes, structured where they nest, match the interpreter") {
  LoopProgram prog = make_loop_program();
  auto syms = xlat::SymbolTable::parse(prog.symbols);
  auto dol = vfs::Dol::parse(prog.dol);
  REQUIRE(dol);
  xlat::ProgramCfg cfg = xlat::build_cfg(*dol, syms);
  REQUIRE(cfg.errors == 0);
  for (int loops = 0; loops < 2; ++loops) {
    xlat::WasmEmitOptions opt;
    opt.precise_io = true;
    opt.fast_entries = true;
    opt.structured_loops = loops != 0;
    xlat::WasmModule mod;
    std::string err;
    REQUIRE_MESSAGE(xlat::emit_wasm(cfg, opt, &mod, &err), err);
    MESSAGE("loops ", loops, ": ", mod.stats.structured_functions, " structured, ", mod.stats.dispatch_functions,
            " with the dispatcher");
    // nested, brk and calls_in structured; rotated and overlap (and every resumable copy) keep the dispatcher.
    CHECK(mod.stats.structured_functions == (loops ? 3u : 0u));
    Machine ref, nat;
    FixedHooks hr, hn;
    StopIo ir, in;
    ir.cpu = &ref.cpu();
    in.cpu = &nat.cpu();
    ref.memory().io = &ir;
    nat.memory().io = &in;
    ref.cpu().hooks = &hr;
    nat.cpu().hooks = &hn;
    ref.load_dol(*dol);
    nat.load_dol(*dol);
    WasmLoadStats ls;
    REQUIRE_MESSAGE(load_wasm_module(mod, &ls, &err), err);
    LockstepResult r = run_lockstep(ref, nat, cfg, 1);
    INFO(r.message);
    REQUIRE(r.pass);
    CHECK(r.stop.addr == 0xCC000000u);
    const u32 want[] = {0, 0, 55, 2055, 0};  // nested and brk computed below; overlap as the interpreter
    u32 nested = 0;
    for (u32 i = 0; i < 5; ++i)
      for (u32 j = 0; j <= i; ++j) nested += i * j + (j & 1);
    u32 brk = 0;
    for (u32 v = 1; v <= 20; ++v)
      if (v & 3) brk += v + (v > 10 ? 100 : 0);
    CHECK(be32(nat.mem1() + 0x300000) == nested);
    CHECK(be32(nat.mem1() + 0x300004) == brk);
    CHECK(be32(nat.mem1() + 0x300008) == want[2]);
    CHECK(be32(nat.mem1() + 0x30000C) == want[3]);
    CHECK(be32(nat.mem1() + 0x300010) == be32(ref.mem1() + 0x300010));
  }
}
