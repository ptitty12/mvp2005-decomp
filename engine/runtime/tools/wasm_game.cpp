// mvp-boot-wasm's game registration (E2, Emscripten only): instead of linking
// the C++-emitted translation, translate main.dol to WebAssembly in-process at
// startup, exactly as the player's browser will, then compile, instantiate and
// register the module (runtime/wasm_host.h). Linked with tools/mvp_boot.cpp,
// whose commands (run, lockstep) are unchanged.
//
// Environment (development switches, all optional):
//   MVP_WASM_PRECISE=1   spill registers before every guest memory access
//                        (exact state at an MMIO stop: lockstep)
//   MVP_WASM_HELPERS=1   every non-branch instruction through the runtime
//   MVP_WASM_OUT=FILE    also write the module (only ever under build/)
//   MVP_WASM_NAMES=1     add a name section (function names for profilers)
//   MVP_WASM_FIXED=0     the runtime's addresses as imported globals (V4: constants by default)
//   MVP_WASM_FAST=0      one function per guest function (V4: fast entries and resumable copies)
//   MVP_WASM_LOOPS=0     loops through the dispatcher (V4: structured loops)
//   MVP_WASM_HEAP=0      heap code interpreted (V4: translated on its third call)
#include <emscripten.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "common/file.h"
#include "runtime/hle.h"
#include "runtime/wasm_host.h"
#include "translator/program.h"
#include "translator/wasm_emit.h"

using namespace mvp;

namespace {
bool env_on(const char* name) {
  const char* v = std::getenv(name);
  return v && *v && *v != '0';
}
double ms_since(std::chrono::steady_clock::time_point t) {
  return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t).count();
}
}  // namespace

void mvp_register_game() {
  auto t0 = std::chrono::steady_clock::now();
  std::string err;
  auto in = xlat::load_inputs(xlat::default_disc_path(), xlat::default_symbols_path(), &err);
  if (!in) {
    std::fprintf(stderr, "mvp-boot-wasm: %s\n", err.c_str());
    std::exit(1);
  }
  double load_ms = ms_since(t0);
  auto t1 = std::chrono::steady_clock::now();
  xlat::ProgramCfg cfg = xlat::build_cfg(in->disc.dol, in->symbols);
  double cfg_ms = ms_since(t1);
  xlat::WasmEmitOptions opt;
  const auto& table = rt::hle_table();
  for (std::size_t k = 0; k < table.size(); ++k)
    opt.hle.push_back({table[k].name, table[k].address, static_cast<int>(k), table[k].kind == rt::HleKind::Before});
  opt.precise_io = env_on("MVP_WASM_PRECISE");
  opt.inline_ops = !env_on("MVP_WASM_HELPERS");
  opt.names = env_on("MVP_WASM_NAMES");
  // V4: the runtime's addresses (MEM1's included: the machine exists by now) as
  // constants, as the browser's engine emits them; MVP_WASM_FIXED=0 keeps the globals.
  if (const char* v = std::getenv("MVP_WASM_FIXED"); !v || *v != '0') opt.fixed = rt::wasm_fixed_addrs();
  // V4: Fast entries with resumable copies, as the browser's engine; MVP_WASM_FAST=0: one function each.
  if (const char* v = std::getenv("MVP_WASM_FAST"); !v || *v != '0') opt.fast_entries = true;
  // V4: loops as wasm loops where the dispatcher is not needed; MVP_WASM_LOOPS=0: the dispatcher.
  if (const char* v = std::getenv("MVP_WASM_LOOPS"); !v || *v != '0') opt.structured_loops = true;
  auto t2 = std::chrono::steady_clock::now();
  xlat::WasmModule mod;
  if (!xlat::emit_wasm(cfg, opt, &mod, &err)) {
    std::fprintf(stderr, "mvp-boot-wasm: %s\n", err.c_str());
    std::exit(1);
  }
  double emit_ms = ms_since(t2);
  if (const char* out = std::getenv("MVP_WASM_OUT"))
    if (!write_file(out, mod.bytes.data(), mod.bytes.size())) std::fprintf(stderr, "mvp-boot-wasm: cannot write %s\n", out);
  rt::WasmLoadStats ls;
  if (!rt::load_wasm_module(mod, &ls, &err)) {
    std::fprintf(stderr, "mvp-boot-wasm: %s\n", err.c_str());
    std::exit(1);
  }
  // V4: heap code translated while the game runs, as in the browser; MVP_WASM_HEAP=0: interpreted.
  if (const char* v = std::getenv("MVP_WASM_HEAP"); !v || *v != '0') rt::set_wasm_heap_code(true);
  const auto& s = mod.stats;
  std::printf("wasm: disc+symbols %.0f ms, CFG %.0f ms, emit %.0f ms, compile %.0f ms, instantiate %.0f ms\n", load_ms,
              cfg_ms, emit_ms, ls.compile_ms, ls.instantiate_ms);
  std::printf("wasm: module %zu bytes, %zu wasm functions (%zu guest functions, %zu entries, %zu resume labels), "
              "%zu instructions: %zu inline, %zu through the runtime, %zu control; %zu spills, %zu reloads%s%s\n",
              mod.bytes.size(), s.wasm_functions, s.functions, s.entries, s.resume_labels, s.instructions,
              s.inline_insns, s.helper_insns, s.branch_insns, s.spills, s.reloads, opt.precise_io ? "; precise I/O" : "",
              opt.inline_ops ? "" : "; helpers only");
  // The process's resident memory once the module is loaded (Node only).
  EM_ASM({
    if (typeof process != 'undefined')
      console.log('wasm: resident memory after loading ' + (process.memoryUsage().rss / 1048576).toFixed(0) + ' MiB');
  });
}
