// The translated module as bytes (E2c): what the browser keeps in its own
// storage (OPFS) so a second visit skips the translation. The blob holds the
// emitter's whole output (xlat::WasmModule: the wasm binary and the tables the
// runtime fills from it), so loading it is exactly loading a fresh translation.
//
// The Worker stores it under a key made of the DOL's SHA-1 and the engine
// build's identity (the hashes of mvp-web.wasm and symbols.txt, written by the
// build next to the engine), so a different disc or a different engine never
// reads it. The blob is derived from the player's disc: it stays in the
// player's browser (ARCHITECTURE.md §3), never on a server.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "translator/wasm_emit.h"

namespace mvp::web {

// Format: "MVPXLAT\0", a version, the module's tables, then its bytes.
std::vector<std::uint8_t> serialize_module(const xlat::WasmModule& m);
// Everything before the bytes: header + m.bytes is serialize_module(m), so the
// Worker writes the two pieces without a second copy of the module in the heap.
std::vector<std::uint8_t> serialize_module_header(const xlat::WasmModule& m);
// V4: the same for a module whose bytes are kept elsewhere (`bytes_len` of them).
std::vector<std::uint8_t> serialize_module_header(const xlat::WasmModule& m, std::uint64_t bytes_len);
bool deserialize_module(const std::uint8_t* p, std::size_t n, xlat::WasmModule* m, std::string* error);
// V4: the tables only (m->bytes stays empty); the wasm bytes are p[*bytes_at, n),
// for loading without a copy of them (rt::load_wasm_module with a span).
bool deserialize_module_tables(const std::uint8_t* p, std::size_t n, xlat::WasmModule* m, std::size_t* bytes_at,
                               std::string* error);

}  // namespace mvp::web
