#include "translator/program.h"

#include "common/file.h"

#ifndef MVP_DISC_DEFAULT
#define MVP_DISC_DEFAULT ""
#endif
#ifndef MVP_DECOMP_PIN_DIR
#define MVP_DECOMP_PIN_DIR ""
#endif

namespace mvp::xlat {

std::optional<Inputs> load_inputs(const std::string& disc_path, const std::string& symbols_path, std::string* error) {
  auto disc = vfs::open_disc(disc_path, true, error);
  if (!disc) return std::nullopt;
  auto text = read_text(symbols_path);
  if (!text) {
    if (error) *error = "cannot read " + symbols_path;
    return std::nullopt;
  }
  std::vector<std::string> errs;
  SymbolTable syms = SymbolTable::parse(*text, &errs);
  if (!errs.empty()) {
    if (error) *error = "symbols.txt: " + errs.front() + " (" + std::to_string(errs.size()) + " errors)";
    return std::nullopt;
  }
  return Inputs{std::move(*disc), std::move(syms)};
}

std::string default_disc_path() { return env_or("MVP_DISC", MVP_DISC_DEFAULT); }
// The engine's pinned copy of the decomp's files (decomp_pin/README.md), never config/GV4E69.
std::string default_symbols_path() { return std::string(MVP_DECOMP_PIN_DIR) + "/symbols.txt"; }
std::string default_splits_path() { return std::string(MVP_DECOMP_PIN_DIR) + "/splits.txt"; }

}  // namespace mvp::xlat
