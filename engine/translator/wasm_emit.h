// WebAssembly emitter (E2, D2): translates the recovered functions straight to a
// wasm32 binary module, with no C compiler in the loop, so the player's browser
// can translate the game itself.
//
// The module imports the runtime's linear memory and function table, a few
// addresses (g_unwinding, g_resume, g_entry_hook) and the runtime services it
// calls ("rt" imports; runtime/wasm_host.h implements them). Guest memory stays
// big-endian (D8): in-module helpers byte-swap through the 64 KiB page table,
// and anything off the fast path (MMIO, page-crossing, unmapped) goes to the
// runtime. The calling convention, safe points, unwinding and resume labels
// are those of the C++ emitter (runtime/native.h, E0c), so translated
// functions of either kind are interchangeable `void f(Cpu&)` table entries.
//
// Code shape:
// - GPRs, CR, LR, CTR, XER[CA] and XER[SO] live in wasm locals; FPRs and
//   everything else stay in the Cpu struct. Dataflow over each function decides
//   which locals are spilled before a call, a helper or a return ("dirty") and
//   which are reloaded after one ("live"), using ppc::effects();
// - instructions the emitter knows (integer ALU, compares, rotates, CR logic,
//   loads and stores, branches, SPR moves) are inline; every other one calls the
//   runtime's semantics (rt.exec: the interpreter's execute_plain), with only its
//   own registers spilled and reloaded;
// - floating point (E2d): FPRs stay in the Cpu struct as raw bits; arithmetic,
//   compares, conversions and paired singles have an inline fast path on IEEE
//   f64 arithmetic, used only where it provably equals the exact model
//   (ppc/fp.h), else the whole instruction goes to rt.exec (rules: wasm_emit.cpp,
//   "floating point"); quantized loads and stores, lmw and stmw are in-module
//   helpers with a one-page fast path;
// - control flow: one block per label, nested so forward branches are direct
//   `br`s; backward branches, jump tables, resume labels and secondary entries
//   go through a loop with a br_table dispatcher;
// - the virtual clock (c.ticks) is a local, stored exactly where the runtime
//   can observe it; `return_call` is used only for guest tail calls.
//
// The output is derived from the player's disc: it is never written outside
// build/ (or the browser's own storage).
#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "translator/cfg.h"
#include "translator/emit_cpp.h"

namespace mvp::xlat {

// V4: the runtime's addresses as constants in the code instead of loads of the
// imported globals (env.unwinding, env.resume, env.entry_hook, env.wgp,
// env.gp_pages, env.mem1). Each zero field keeps its global. A module emitted
// with any of them works only in a runtime whose addresses they are: the
// browser's engine translates in its own process (and its cache key names the
// engine build), and rt::load_wasm_module refuses a module whose fixed
// addresses differ from the runtime's (WasmModule::fixed). With mem1_limit set
// (MEM1 at a fixed host address, mem1_base), a MEM1 access is a compare with a
// constant and a load or store with mem1_base as its offset.
struct WasmFixed {
  std::uint32_t unwinding = 0, resume = 0, hook = 0, wgp = 0, gp_pages = 0;
  std::uint32_t mem1_base = 0, mem1_limit = 0;  // as WasmMem1Layout's {base, limit}; limit 0: not fixed
  bool any() const { return unwinding || resume || hook || wgp || gp_pages || mem1_limit; }
  bool operator==(const WasmFixed&) const = default;
};

struct WasmEmitOptions {
  std::vector<HleSpec> hle;  // replacements and hooks, as for emit_cpp
  // Spill dirty registers before every guest memory access too, so the CPU
  // state is exact when an MMIO handler stops the run (lockstep tests).
  bool precise_io = false;
  // Run every non-branch instruction through the runtime (rt.exec): for tests
  // of the helper path and for measuring what inlining buys.
  bool inline_ops = true;
  // Add a "name" custom section (function names: the guest symbol, the helpers
  // and imports), for profilers. Off by default: it only costs bytes.
  bool names = false;
  // Check MSR[FP] before FPU instructions (floating-point unavailable, E3a): the
  // first of each block, as the C++ emitter does. Off only for tests that compare
  // single instructions with execute_plain(), which has no such check.
  bool fpu_checks = true;
  // Import the memory as shared, with a maximum of 4 GiB (P2: the threaded
  // browser engine, whose memory is a SharedArrayBuffer), and check each store
  // to guest memory against env.gp_pages first (WasmGpLayout): only there can a
  // render thread still have to read what the store overwrites.
  bool shared_memory = false;
  // Q4, F1: floating-point arithmetic in the exact Gekko model (ppc/fp.h) with
  // the fast paths E2d proved equal to it. Off (the default): the fast model of
  // ppc/fp.h, which the runtime's semantics library must use too
  // (ppc::fp::g_model); on, it must be Model::Exact.
  bool fp_exact = false;
  // V4: constants for the runtime's addresses (WasmFixed); all zero: the globals.
  WasmFixed fixed;
};

// A guest address and the table slot (relative to the module's table base) of
// the function it belongs to.
struct WasmSlot {
  std::uint32_t address = 0;
  std::uint32_t slot = 0;
};

struct WasmEmitStats {
  std::size_t functions = 0, wasm_functions = 0, entries = 0, instructions = 0, inline_insns = 0, helper_insns = 0,
              branch_insns = 0, resume_labels = 0, dispatch_functions = 0, spills = 0, reloads = 0, hle_stubs = 0;
  std::size_t code_bytes = 0;
  std::vector<std::size_t> helper_by_op;  // per ppc::Op: instructions that go through rt.exec
};

struct WasmModule {
  std::vector<std::uint8_t> bytes;  // the binary module
  std::uint32_t slots = 0;          // table entries it fills, from its imported table base
  std::vector<WasmSlot> entries;    // every entry point -> its function's slot (rt::g_native)
  std::vector<WasmSlot> resume;     // resume labels -> the slot of their function's first entry (rt::g_resume_labels)
  // Resume labels -> the dispatcher index rt.resume returns for them.
  std::vector<std::pair<std::uint32_t, std::uint32_t>> resume_index;
  std::vector<std::uint32_t> safe, idle;            // safe points and idle spins (as emit_cpp's table)
  std::vector<std::pair<std::uint32_t, int>> hle_sites;  // replaced functions for the interpreter
  std::vector<int> hle_matched;                    // HleSpec::index of every spec that found its function
  WasmFixed fixed;                                 // V4: the constants it was emitted with (WasmEmitOptions::fixed)
  WasmEmitStats stats;
};

// Import names, in import order ("rt" module). The host must provide each one.
const std::vector<std::string>& wasm_import_names();

// P2: the write-gather pipe's state the module's store helpers append to
// directly (env.wgp, the address of runtime/hw.h's WgpState): a store of 1, 2
// or 4 bytes to 0xCC008000 (Q4: to its 4 KiB block, as Hardware::write takes
// them) that fits in the 32-byte burst being gathered, while
// `fast` is set, is appended big-endian at buf + n without calling the runtime,
// which then counts it (fast_stores) and ends the poll detector's run
// (last_read = 0), as Hardware::write would. Q4: a store that completes the
// burst (or reaches into the next one) is counted and ends the run the same way,
// then goes to rt.wgp_store, which gathers it as Hardware::write would (the burst
// included). Offsets (the runtime checks them):
struct WasmWgpLayout {
  static constexpr std::uint32_t kBuf = 0, kN = 32, kFast = 36, kLastRead = 40, kFastStores = 48;
};

// Q6: env.mem1, the address of the runtime's view of MEM1 (runtime/wasm_host.h,
// set_wasm_mem1): {u32 base, u32 limit}. A guest access of up to 8 bytes at
// 0x80000000 + t with t < limit is at host address base + t (MEM1 is mapped
// there linearly, runtime/machine.h); limit 0 turns the path off. Offsets (the
// runtime checks them):
struct WasmMem1Layout {
  static constexpr std::uint32_t kBase = 0, kLimit = 4;
};

// P2: env.gp_pages (ppc::g_gp_pages): one 32-bit entry per 2^kShift bytes of
// the 32 MiB physical space, nonzero while the render thread has still to read
// them (the runtime checks that it agrees: ppc/mem_access.h).
struct WasmGpLayout {
  static constexpr std::uint32_t kShift = 6, kMask = (1u << (25 - kShift)) - 1;
};

// Trap codes passed to rt.trap.
enum class WasmTrap : std::uint32_t {
  InvalidInstruction = 0,
  UnconditionalTrap = 1,
  RanOffEnd = 2,
  JumpOutsideTable = 3,
  ExternalControl = 4,
  BadEntry = 5,
};
const char* wasm_trap_text(std::uint32_t code);

bool emit_wasm(const ProgramCfg& p, const WasmEmitOptions& opt, WasmModule* out, std::string* error);

}  // namespace mvp::xlat
