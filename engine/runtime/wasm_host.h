// The runtime side of the WebAssembly emitter (E2; translator/wasm_emit.h):
// the "rt" functions a translated module imports, and loading a module into
// the running engine. Emscripten builds only.
//
// Loading compiles the module, grows the runtime's own function table by the
// module's slots and instantiates it there, so translated functions are plain
// `void (*)(Cpu&)` pointers: rt::run, call_indirect and the C++ runtime call
// them exactly like C++-emitted ones. Then it fills the same tables the C++
// emitter's registration fills (entry points, resume labels, safe points, idle
// spins, replaced functions).
#pragma once

#include <string>

#include "translator/wasm_emit.h"

namespace mvp::rt {

struct WasmLoadStats {
  double compile_ms = 0;      // new WebAssembly.Module (synchronous)
  double instantiate_ms = 0;  // new WebAssembly.Instance
  std::uint32_t table_base = 0;
};

bool load_wasm_module(const xlat::WasmModule& m, WasmLoadStats* stats, std::string* error);
// V4: the same with the wasm bytes at [bytes, bytes + len) instead of m.bytes (a
// cached blob, not copied). `keep_js_copy`: the JavaScript side keeps a copy of
// the bytes as Module.mvpModuleBytes (the browser's cache writes it later), so the
// caller can free its own at once and the engine's heap does not hold them.
bool load_wasm_module(const xlat::WasmModule& m, const std::uint8_t* bytes, std::size_t len, bool keep_js_copy,
                      WasmLoadStats* stats, std::string* error);

// P2: the write-gather pipe's buffer translated modules append to directly
// (xlat::WasmWgpLayout). Give it to the machine's Hardware (set_wgp_state).
struct WgpState;
WgpState* wasm_wgp_state();

// Q6: MEM1 of the machine whose Cpu runs translated modules (xlat::WasmMem1Layout):
// the modules' memory helpers read and write [0x80000000, +size) there directly.
// Call once the machine exists, before its code runs; null turns it off.
void set_wasm_mem1(const std::uint8_t* base, std::uint32_t size);

// V4: the runtime's addresses a module may hold as constants (xlat::WasmFixed):
// g_unwinding, g_resume, g_entry_hook, the pipe's state, ppc::g_gp_pages, and
// MEM1's once set_wasm_mem1 gave it. load_wasm_module refuses a module whose
// fixed (nonzero) fields differ from these, and set_wasm_mem1 must not move MEM1
// away from where a loaded module has it fixed.
xlat::WasmFixed wasm_fixed_addrs();

}  // namespace mvp::rt
