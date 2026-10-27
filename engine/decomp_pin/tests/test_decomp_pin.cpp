// Data-free checks of the engine's pinned copy of the decomp's metadata
// (decomp_pin/README.md): every name and address the engine binds must resolve in
// the pinned symbols.txt. The translator matches HLE entries silently (translator/
// emit_cpp.cpp find_hle), so a sync that renamed or dropped one would otherwise lose
// a replacement or a hook without an error, and only mvp-xlat with the disc noticed.
#include <doctest/doctest.h>

#include <string>
#include <vector>

#include "common/file.h"
#include "runtime/hle.h"
#include "runtime/mod_hooks.h"
#include "runtime/sdk_names.h"
#include "translator/symbols.h"
#include "translator/units.h"

using namespace mvp;

namespace {

const xlat::SymbolTable& pinned_symbols() {
  static const xlat::SymbolTable t = [] {
    auto text = read_text(MVP_DECOMP_PIN_DIR "/symbols.txt");
    REQUIRE(text.has_value());
    std::vector<std::string> errors;
    xlat::SymbolTable s = xlat::SymbolTable::parse(*text, &errors);
    CHECK(errors.empty());
    return s;
  }();
  return t;
}

int functions_named(const std::string& name) {
  int n = 0;
  for (const auto& [start, sym] : pinned_symbols().functions()) n += sym->name == name;
  return n;
}

}  // namespace

TEST_CASE("the pinned symbols.txt parses") {
  CHECK(pinned_symbols().functions().size() > 20000);
}

TEST_CASE("every HLE entry finds exactly one function in the pinned symbols.txt") {
  for (const auto& e : rt::hle_table()) {
    const std::string name = e.name;  // captured as text (doctest shows a const char* as a bool)
    CAPTURE(name);
    CAPTURE(e.address);
    if (e.address)
      CHECK(pinned_symbols().function_at(e.address) != nullptr);  // game functions and mod sites: by retail address
    else
      CHECK(functions_named(e.name) == 1);  // SDK replacements and hooks: by name
  }
}

TEST_CASE("every gameplay mod site is a function start") {
  for (const auto& s : rt::mod::sites()) {
    const std::string label = s.label;
    CAPTURE(label);
    CHECK(pinned_symbols().function_at(s.address) != nullptr);
  }
}

TEST_CASE("the SDK aliases name function starts the pin has not named otherwise") {
  for (const auto& a : rt::kSdkAliases) {
    const std::string alias = a.name;
    CAPTURE(alias);
    const xlat::Symbol* s = pinned_symbols().function_at(a.address);
    REQUIRE(s != nullptr);
    // A placeholder, or already the alias (then the alias is redundant: sdk_names.h).
    CHECK((s->name.rfind("fn_", 0) == 0 || s->name == a.name));
  }
}

TEST_CASE("the pinned splits.txt puts the replaced SDK functions in runtime-library units") {
  auto text = read_text(MVP_DECOMP_PIN_DIR "/splits.txt");
  REQUIRE(text.has_value());
  xlat::UnitMap units = xlat::UnitMap::parse(*text);
  CHECK(units.size() > 500);
  for (const auto& e : rt::hle_table()) {
    if (e.address) continue;
    const std::string name = e.name;
    CAPTURE(name);
    const xlat::Symbol* s = pinned_symbols().find(name);
    REQUIRE(s != nullptr);
    const std::string unit = units.unit_at(s->address);
    CAPTURE(unit);
    CHECK(units.is_runtime(s->address));
  }
}
