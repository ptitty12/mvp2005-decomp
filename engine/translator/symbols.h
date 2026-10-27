// decomp-toolkit symbol map (the decomp's config/GV4E69/symbols.txt, which the
// engine reads from its pinned copy, decomp_pin/symbols.txt): metadata about the
// retail binary (names, addresses, sizes), not game data.
#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace mvp::xlat {

using u32 = std::uint32_t;

struct Symbol {
  enum class Type { Function, Label, Object, Unknown };
  std::string name;
  std::string section;  // ".text", ".feoverlay", ...
  u32 address = 0;
  u32 size = 0;  // 0 when not given
  Type type = Type::Unknown;
  bool local = false;
  bool data = false;  // has a `data:` attribute (a label in a code section that marks data)
};

class SymbolTable {
 public:
  SymbolTable() = default;
  SymbolTable(SymbolTable&&) = default;
  SymbolTable& operator=(SymbolTable&&) = default;
  SymbolTable(const SymbolTable&) = delete;  // the indexes point into symbols_
  SymbolTable& operator=(const SymbolTable&) = delete;

  // Parses symbols.txt lines: `name = section:0xADDR; // type:function size:0x10 scope:local ...`.
  // Lines that don't parse are reported through `errors` (if given) and skipped.
  static SymbolTable parse(const std::string& text, std::vector<std::string>* errors = nullptr);

  static bool is_code_section(const std::string& s) {
    return s == ".init" || s == ".text" || s == ".feoverlay" || s == ".dboverlay";
  }

  const std::vector<Symbol>& all() const { return symbols_; }
  // Functions by start address (each code address belongs to at most one).
  const std::map<u32, const Symbol*>& functions() const { return functions_; }
  const Symbol* function_at(u32 start) const;        // exact start
  const Symbol* function_containing(u32 addr) const;  // start <= addr < start + size
  // First symbol with this name, or null.
  const Symbol* find(const std::string& name) const;
  // Objects named jumptable_* by start address.
  const std::map<u32, const Symbol*>& jump_tables() const { return jump_tables_; }
  // The data object starting at `addr` (any name), or null.
  const Symbol* object_at(u32 addr) const;
  // The code label at `addr` (type:label in a code section, not marked as data), or null.
  const Symbol* code_label_at(u32 addr) const;

 private:
  std::vector<Symbol> symbols_;
  std::map<u32, const Symbol*> functions_;
  std::map<u32, const Symbol*> jump_tables_;
  std::map<u32, const Symbol*> objects_;
  std::map<u32, const Symbol*> code_labels_;
  std::map<std::string, const Symbol*> by_name_;
};

}  // namespace mvp::xlat
