// mvp-xlat: the translator's development CLI.
//
//   mvp-xlat cfg [--errors] [--tables]   recover functions and CFGs, print a summary
//   mvp-xlat emit-cpp <dir> [--shards N]  translate the whole DOL to C++ in <dir> (under build/)
//   mvp-xlat emit-wasm <file> [--precise] [--helpers] [--names] [--shared] [--fast] [--runs N]
//                                        translate the whole DOL to a WebAssembly module (E2) and print
//                                        how long it took; <file> under build/ only (--names: function
//                                        names for profilers and disassemblers; --shared: the threaded
//                                        engine's module)
//
// Reads the disc from MVP_DISC (or the build's default) and config/GV4E69/symbols.txt.
// Output describes the retail binary's structure only; it stays in the terminal
// or under build/.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "common/file.h"
#include "runtime/hle.h"
#include "translator/emit_cpp.h"
#include "translator/wasm_emit.h"
#include "translator/program.h"

using namespace mvp;
using namespace mvp::xlat;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::fprintf(stderr, "usage: mvp-xlat cfg [--errors] [--tables] | emit-cpp <dir> [--shards N] | emit-wasm <file> [--precise] "
                 "[--helpers] [--runs N]\n");
    return 2;
  }
  std::string cmd = argv[1];
  bool show_errors = false, show_tables = false;
  for (int k = 2; k < argc; ++k) {
    if (!std::strcmp(argv[k], "--errors")) show_errors = true;
    if (!std::strcmp(argv[k], "--tables")) show_tables = true;
  }
  std::string err;
  auto in = load_inputs(default_disc_path(), default_symbols_path(), &err);
  if (!in) {
    std::fprintf(stderr, "mvp-xlat: %s\n", err.c_str());
    return 1;
  }
  if (cmd == "cfg") {
    ProgramCfg p = build_cfg(in->disc.dol, in->symbols);
    std::printf("functions %zu, words %zu (reachable %zu), blocks %zu\n", p.functions.size(), p.words,
                p.reachable_words, p.blocks);
    std::printf("calls %zu, tail calls %zu, indirect calls %zu, jump tables %zu, indirect jumps %zu, "
                "absolute calls %zu\n",
                p.calls, p.tail_calls, p.indirect_calls, p.jump_tables, p.indirect_jumps, p.absolute_calls);
    std::printf("resume points %zu, back edges %zu, secondary entries %zu, runs into the next function %zu, "
                "expected traps %zu, pure spins %zu, errors %zu\n",
                p.resume_points, p.back_edges, p.secondary_entries, p.falls_into_next, p.expected_traps, p.idle_spins,
                p.errors);
    if (show_tables)
      for (const auto& f : p.functions)
        for (const auto& t : f.tables)
          std::printf("  %s: bctr 0x%08X table 0x%08X, %zu entries\n", f.sym->name.c_str(), t.site, t.table,
                      t.targets.size());
    if (show_errors)
      for (const auto& e : p.error_list) std::printf("  %s\n", e.c_str());
    return p.errors ? 1 : 0;
  }
  if (cmd == "emit-cpp") {
    if (argc < 3) return 2;
    EmitOptions opt;
    const auto& table = rt::hle_table();
    for (std::size_t k = 0; k < table.size(); ++k)
      opt.hle.push_back({table[k].name, table[k].address, static_cast<int>(k), table[k].kind == rt::HleKind::Before});
    for (int k = 3; k + 1 < argc; ++k)
      if (!std::strcmp(argv[k], "--shards")) opt.shards = std::atoi(argv[k + 1]);
    ProgramCfg p = build_cfg(in->disc.dol, in->symbols);
    if (p.errors) {
      std::fprintf(stderr, "mvp-xlat: the CFG has %zu errors; not emitting\n", p.errors);
      return 1;
    }
    EmitStats st;
    if (!emit_cpp(p, argv[2], opt, &st, &err)) {
      std::fprintf(stderr, "mvp-xlat: %s\n", err.c_str());
      return 1;
    }
    std::printf("emitted %zu functions (%zu entry points, %zu resume labels), %zu instructions, %zu bytes of C++ in "
                "%d shards\n",
                st.functions, st.entries, st.resume_labels, st.instructions, st.bytes, opt.shards);
    std::printf("FPU checks %zu; functions left unoptimized (straight-line runs over %zu instructions) %zu\n",
                st.fpu_checks, opt.max_optimized_run, st.unoptimized);
    // Every replacement and hook must find its function: a rename in symbols.txt
    // (or a wrong address) would otherwise drop it silently.
    int missing = 0;
    for (std::size_t k = 0; k < table.size(); ++k)
      if (std::count(st.hle_matched.begin(), st.hle_matched.end(), static_cast<int>(k)) != 1) {
        std::fprintf(stderr, "mvp-xlat: HLE entry %zu (%s, 0x%08X) matches %d functions, not 1\n", k, table[k].name,
                     table[k].address,
                     static_cast<int>(std::count(st.hle_matched.begin(), st.hle_matched.end(), static_cast<int>(k))));
        ++missing;
      }
    return missing ? 1 : 0;
  }
  if (cmd == "emit-wasm") {
    if (argc < 3) return 2;
    WasmEmitOptions opt;
    const auto& table = rt::hle_table();
    for (std::size_t k = 0; k < table.size(); ++k)
      opt.hle.push_back({table[k].name, table[k].address, static_cast<int>(k), table[k].kind == rt::HleKind::Before});
    int runs = 1;
    for (int k = 3; k < argc; ++k) {
      if (!std::strcmp(argv[k], "--precise")) opt.precise_io = true;
      if (!std::strcmp(argv[k], "--helpers")) opt.inline_ops = false;
      if (!std::strcmp(argv[k], "--names")) opt.names = true;            // a name section (profilers, disassembly)
      if (!std::strcmp(argv[k], "--shared")) opt.shared_memory = true;   // the threaded engine's module (P2)
      if (!std::strcmp(argv[k], "--fast")) opt.fast_entries = true;      // V4: fast entries, resumable copies
      if (!std::strcmp(argv[k], "--loops")) opt.structured_loops = true; // V4: loops without the dispatcher
      if (!std::strcmp(argv[k], "--runs") && k + 1 < argc) runs = std::atoi(argv[k + 1]);
    }
    using clock = std::chrono::steady_clock;
    auto ms = [](clock::time_point t) { return std::chrono::duration<double, std::milli>(clock::now() - t).count(); };
    WasmModule mod;
    for (int r = 0; r < runs; ++r) {
      auto t0 = clock::now();
      ProgramCfg p = build_cfg(in->disc.dol, in->symbols);
      double cfg_ms = ms(t0);
      if (p.errors) {
        std::fprintf(stderr, "mvp-xlat: the CFG has %zu errors; not emitting\n", p.errors);
        return 1;
      }
      auto t1 = clock::now();
      if (!emit_wasm(p, opt, &mod, &err)) {
        std::fprintf(stderr, "mvp-xlat: %s\n", err.c_str());
        return 1;
      }
      std::printf("run %d: CFG %.0f ms, emit %.0f ms\n", r, cfg_ms, ms(t1));
    }
    if (!write_file(argv[2], mod.bytes.data(), mod.bytes.size())) {
      std::fprintf(stderr, "mvp-xlat: cannot write %s\n", argv[2]);
      return 1;
    }
    const auto& s = mod.stats;
    auto pct = [&](std::size_t n) { return 100.0 * static_cast<double>(n) / static_cast<double>(s.instructions); };
    std::printf("module %zu bytes (code %zu), %zu wasm functions (%zu guest functions, %zu entries, %zu with a "
                "dispatcher, %zu resume labels, %zu replaced)\n",
                mod.bytes.size(), s.code_bytes, s.wasm_functions, s.functions, s.entries, s.dispatch_functions,
                s.resume_labels, s.hle_stubs);
    std::printf("%zu instructions: %zu inline (%.1f%%), %zu through the runtime (%.1f%%), %zu control (%.1f%%); "
                "%zu spills, %zu reloads\n",
                s.instructions, s.inline_insns, pct(s.inline_insns), s.helper_insns, pct(s.helper_insns),
                s.branch_insns, pct(s.branch_insns), s.spills, s.reloads);
    std::vector<std::pair<std::size_t, int>> by_op;
    for (std::size_t k = 0; k < s.helper_by_op.size(); ++k)
      if (s.helper_by_op[k]) by_op.push_back({s.helper_by_op[k], static_cast<int>(k)});
    std::sort(by_op.rbegin(), by_op.rend());
    std::printf("through the runtime, by opcode:");
    for (std::size_t k = 0; k < by_op.size() && k < 24; ++k)
      std::printf(" %s %zu", ppc::op_name(static_cast<ppc::Op>(by_op[k].second)), by_op[k].first);
    std::printf("\n");
    return 0;
  }
  std::fprintf(stderr, "mvp-xlat: unknown command %s\n", cmd.c_str());
  return 2;
}
