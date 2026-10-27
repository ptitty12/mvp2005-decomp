// Local-only check (E0b item 3 exit): disassemble every code word of the retail
// main.dol and compare it, word for word, with decomp-toolkit's split listing.
//
// Needs MVP_DISC (the disc) and MVP_DTK_ASM (dtk's build/GV4E69/asm); exits 77
// (skipped) without them. dtk prints relocated operands symbolically
// (`lbl_805A3048@l(r11)`, `fn_80003300`, `.L_800031A4`); those are evaluated with
// the symbols.txt dtk split with and compared by value: MVP_DTK_SYMBOLS, else the
// engine's pinned copy (decomp_pin/), which fits a listing built at the decomp
// commit pin.json names. Mismatches are printed (at most 40) to the terminal only.
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/bytes.h"
#include "common/file.h"
#include "ppc/disasm.h"
#include "vfs/disc_check.h"

#ifndef MVP_DISC_DEFAULT
#define MVP_DISC_DEFAULT ""
#endif
#ifndef MVP_DTK_ASM_DEFAULT
#define MVP_DTK_ASM_DEFAULT ""
#endif
#ifndef MVP_DECOMP_PIN_DIR
#define MVP_DECOMP_PIN_DIR ""
#endif

namespace fs = std::filesystem;
using namespace mvp;

namespace {

struct Listed {
  fs::file_time_type mtime;
  u32 word;
  std::string text;
};

std::unordered_multimap<std::string, u32> load_symbols(const std::string& path) {
  std::unordered_multimap<std::string, u32> m;
  std::ifstream in(path);
  std::string line;
  while (std::getline(in, line)) {
    auto eq = line.find(" = ");
    auto colon = line.find(":0x", eq);
    if (eq == std::string::npos || colon == std::string::npos) continue;
    m.emplace(line.substr(0, eq), static_cast<u32>(std::strtoul(line.c_str() + colon + 3, nullptr, 16)));
  }
  return m;
}

std::vector<std::string> split_operands(const std::string& s) {
  std::vector<std::string> out;
  std::size_t start = 0;
  bool quoted = false;
  for (std::size_t i = 0; i <= s.size(); ++i) {
    if (i < s.size() && s[i] == '"') quoted = !quoted;
    if (i == s.size() || (!quoted && s[i] == ',' && i + 1 < s.size() && s[i + 1] == ' ')) {
      out.push_back(s.substr(start, i - start));
      start = i + 2;
      ++i;
    }
  }
  return out;
}

std::optional<long long> parse_num(const std::string& s) {
  if (s.empty()) return std::nullopt;
  bool neg = s[0] == '-';
  std::string t = neg ? s.substr(1) : s;
  if (t.size() < 3 || t[0] != '0' || t[1] != 'x') return std::nullopt;
  char* end = nullptr;
  long long v = std::strtoll(t.c_str() + 2, &end, 16);
  if (*end) return std::nullopt;
  return neg ? -v : v;
}

class Checker {
 public:
  Checker(std::unordered_multimap<std::string, u32> syms, u32 sda, u32 sda2)
      : syms_(std::move(syms)), sda_(sda), sda2_(sda2) {}

  // Addresses a dtk symbol operand can denote.
  std::vector<u32> resolve(std::string name) const {
    if (name.size() >= 2 && name.front() == '"' && name.back() == '"') name = name.substr(1, name.size() - 2);
    std::vector<u32> out;
    if (name.rfind(".L_", 0) == 0) {
      out.push_back(static_cast<u32>(std::strtoul(name.c_str() + 3, nullptr, 16)));
      return out;
    }
    auto [a, b] = syms_.equal_range(name);
    for (auto it = a; it != b; ++it) out.push_back(it->second);
    // dtk disambiguates duplicated local names by appending _<ADDRESS>.
    if (out.empty() && name.size() > 9 && name[name.size() - 9] == '_') {
      char* end = nullptr;
      unsigned long v = std::strtoul(name.c_str() + name.size() - 8, &end, 16);
      if (!*end) out.push_back(static_cast<u32>(v));
    }
    return out;
  }

  bool operand_matches(const std::string& mine, const std::string& theirs) const {
    if (mine == theirs) return true;
    // Split a trailing "(rN)" off both.
    auto base_reg = [](const std::string& s, std::string* head) {
      std::string reg;
      if (!s.empty() && s.back() == ')') {
        auto p = s.rfind('(');
        if (p != std::string::npos) {
          reg = s.substr(p + 1, s.size() - p - 2);
          *head = s.substr(0, p);
          return reg;
        }
      }
      *head = s;
      return reg;
    };
    std::string mh, th;
    std::string mreg = base_reg(mine, &mh), treg = base_reg(theirs, &th);
    auto mval = parse_num(mh);
    if (!mval) return false;
    auto at = th.rfind('@');
    if (at == std::string::npos || th.find('"', at) != std::string::npos) {
      // Bare symbol: a branch target.
      if (!treg.empty() || !mreg.empty()) return false;
      for (u32 a : resolve(th))
        if (static_cast<u32>(*mval) == a) return true;
      return false;
    }
    std::string kind = th.substr(at + 1), sym = th.substr(0, at);
    long long addend = 0;
    auto pm = sym.find_last_of("+-");
    if (pm != std::string::npos && pm > 0 && sym.compare(pm + 1, 2, "0x") == 0 && sym.back() != '"') {
      auto v = parse_num(sym.substr(pm + (sym[pm] == '+' ? 1 : 0)));
      if (v) {
        addend = *v;
        sym = sym.substr(0, pm);
      }
    }
    for (u32 a : resolve(sym)) {
      u32 s = static_cast<u32>(a + addend);
      u32 m = static_cast<u32>(*mval) & 0xFFFF;
      if (kind == "sda21") {
        if (treg != "r0") return false;
        u32 base = mreg == "r13" ? sda_ : mreg == "r2" ? sda2_ : mreg == "r0" ? 0 : 1;
        if (base == 1) return false;
        if (static_cast<u32>(base + static_cast<s32>(*mval)) == s) return true;
        continue;
      }
      if (mreg != treg) return false;
      if (kind == "h" && m == (s >> 16)) return true;
      if (kind == "ha" && m == (((s + 0x8000) >> 16) & 0xFFFF)) return true;
      if (kind == "l" && m == (s & 0xFFFF)) return true;
    }
    return false;
  }

  // How a word compared. Everything except kMismatch counts as matching.
  enum Result { kExact, kRelocValue, kRelocAlias, kBranchDisp, kDtkNop, kMismatch };

  Result compare(const std::string& mine, const std::string& theirs, u32 addr) const {
    if (mine == theirs) return kExact;
    auto ms = mine.find(' '), ts = theirs.find(' ');
    std::string mm = mine.substr(0, ms), tm = theirs.substr(0, ts);
    std::string margs = ms == std::string::npos ? "" : mine.substr(ms + 1);
    std::string targs = ts == std::string::npos ? "" : theirs.substr(ts + 1);
    // dtk quirk: it prints any `ori r0, r0, imm` as `nop`.
    if (theirs == "nop" && mine.rfind("ori r0, r0, ", 0) == 0) return kDtkNop;
    if (mm == tm) {
      auto mo = split_operands(margs), to = split_operands(targs);
      if (mo.size() != to.size()) return kMismatch;
      bool all = true;
      for (std::size_t i = 0; i < mo.size() && all; ++i) all = operand_matches(mo[i], to[i]);
      if (all) return kRelocValue;
      // Branch targets dtk cannot label print as the raw displacement.
      if (is_branch(mm) && !mo.empty()) {
        auto mv = parse_num(mo.back()), tv = parse_num(to.back());
        if (mv && tv && static_cast<u32>(*mv) - addr == static_cast<u32>(*tv)) {
          bool rest = true;
          for (std::size_t i = 0; i + 1 < mo.size(); ++i) rest &= mo[i] == to[i];
          if (rest) return kBranchDisp;
        }
      }
      return kMismatch;
    }
    // With a relocated operand dtk keeps the base mnemonic: subi/subis/subic print
    // as addi/addis/addic, and an sda21 addi prints as `li rD, sym@sda21`.
    auto mo = split_operands(margs), to = split_operands(targs);
    static const std::pair<const char*, const char*> kAlias[] = {
        {"subi", "addi"}, {"subis", "addis"}, {"subic", "addic"}, {"subic.", "addic."}};
    for (auto [sub, add] : kAlias) {
      if (mm == sub && tm == add && mo.size() == 3 && to.size() == 3 && mo[0] == to[0] && mo[1] == to[1] &&
          to[2].find('@') != std::string::npos) {
        auto v = parse_num(mo[2]);
        if (v && operand_matches(neg_hex(*v), to[2])) return kRelocAlias;
      }
    }
    if (tm == "li" && (mm == "addi" || mm == "subi") && mo.size() == 3 && to.size() == 2 && mo[0] == to[0] &&
        to[1].size() > 6 && to[1].compare(to[1].size() - 6, 6, "@sda21") == 0) {
      auto v = parse_num(mo[2]);
      if (v && operand_matches(neg_hex(mm == "subi" ? *v : -*v) + "(" + mo[1] + ")", to[1] + "(r0)"))
        return kRelocAlias;
    }
    return kMismatch;
  }

  static bool is_branch(const std::string& m) { return !m.empty() && m[0] == 'b'; }
  static std::string neg_hex(long long v) {
    char b[24];
    long long n = -v;
    if (n < 0)
      std::snprintf(b, sizeof b, "-0x%llx", -n);
    else
      std::snprintf(b, sizeof b, "0x%llx", n);
    return b;
  }

 private:
  std::unordered_multimap<std::string, u32> syms_;
  u32 sda_, sda2_;
};

bool parse_listing_line(const std::string& l, u32* addr, u32* word, std::string* text) {
  // "/* 80003100 00000100  3C 20 80 70 */\tlis r1, 0x8070"
  if (l.size() < 38 || l.compare(0, 3, "/* ") != 0 || l.compare(33, 4, " */\t") != 0) return false;
  char* end = nullptr;
  *addr = static_cast<u32>(std::strtoul(l.c_str() + 3, &end, 16));
  if (end != l.c_str() + 11) return false;
  u32 w = 0;
  for (int k = 0; k < 4; ++k) {
    unsigned b = static_cast<unsigned>(std::strtoul(l.substr(22 + 3 * k, 2).c_str(), nullptr, 16));
    w = (w << 8) | b;
  }
  *word = w;
  *text = l.substr(37);
  while (!text->empty() && (text->back() == '\r' || text->back() == ' ')) text->pop_back();
  return true;
}

}  // namespace

int main() {
  std::string disc_path = env_or("MVP_DISC", MVP_DISC_DEFAULT);
  std::string asm_dir = env_or("MVP_DTK_ASM", MVP_DTK_ASM_DEFAULT);
  std::string symbols = env_or("MVP_DTK_SYMBOLS", std::string(MVP_DECOMP_PIN_DIR) + "/symbols.txt");
  std::error_code ec;
  if (disc_path.empty() || asm_dir.empty() || !fs::is_directory(asm_dir, ec) || !fs::exists(disc_path, ec) ||
      !fs::exists(symbols, ec)) {
    std::printf("SKIP: needs MVP_DISC, MVP_DTK_ASM and %s\n", symbols.c_str());
    return 77;
  }
  std::string err;
  auto disc = vfs::open_disc(disc_path, true, &err);
  if (!disc) {
    std::printf("FAIL: %s\n", err.c_str());
    return 1;
  }

  // Code sections of the listing; for an address listed twice (stale files from an
  // earlier split), the newest file wins.
  std::map<u32, Listed> listed;
  // The listing's own symbol names (the "# .sec:0xOFF | 0xADDR | size" comment
  // before each .fn/.obj/.sym), so the check does not depend on symbols.txt
  // being from the same commit as the listing.
  std::unordered_multimap<std::string, u32> listing_syms;
  std::size_t files = 0;
  for (auto& e : fs::recursive_directory_iterator(asm_dir)) {
    if (!e.is_regular_file() || e.path().extension() != ".s") continue;
    ++files;
    auto mt = e.last_write_time();
    std::ifstream in(e.path());
    std::string l, sec;
    u32 header_addr = 0;
    bool have_header = false;
    while (std::getline(in, l)) {
      if (l.rfind("# .", 0) == 0) {
        auto bar = l.find("| 0x");
        if (bar != std::string::npos) {
          header_addr = static_cast<u32>(std::strtoul(l.c_str() + bar + 4, nullptr, 16));
          have_header = true;
        }
        continue;
      }
      if (have_header && (l.rfind(".fn ", 0) == 0 || l.rfind(".obj ", 0) == 0 || l.rfind(".sym ", 0) == 0)) {
        std::size_t s0 = l.find(' ') + 1, s1 = l.find(',', s0);
        listing_syms.emplace(l.substr(s0, s1 == std::string::npos ? std::string::npos : s1 - s0), header_addr);
        have_header = false;
        continue;
      }
      if (l.rfind(".section ", 0) == 0) {
        sec = l.substr(9, l.find(',') - 9);
        continue;
      }
      if (l.rfind(".text", 0) == 0) sec = ".text";
      if (sec != ".init" && sec != ".text" && sec != ".feoverlay" && sec != ".dboverlay") continue;
      u32 addr, word;
      std::string text;
      if (!parse_listing_line(l, &addr, &word, &text)) continue;
      auto it = listed.find(addr);
      if (it == listed.end() || it->second.mtime < mt) listed[addr] = Listed{mt, word, text};
    }
  }

  // r13/r2 bases from __start's own lis/ori pairs (0x80003108..0x80003114).
  auto lis_ori = [&](u32 at) {
    const u8* p = disc->dol.at(at, 8);
    return p ? ((be32(p) & 0xFFFF) << 16) | (be32(p + 4) & 0xFFFF) : 0u;
  };
  u32 sda2 = lis_ori(0x80003108), sda = lis_ori(0x80003110);
  auto syms = load_symbols(symbols);
  for (const auto& [n, a] : listing_syms) syms.emplace(n, a);
  syms.emplace("_SDA_BASE_", sda);  // linker-defined, not in symbols.txt
  syms.emplace("_SDA2_BASE_", sda2);
  Checker chk(std::move(syms), sda, sda2);

  std::size_t total = 0, same = 0, byte_mismatch = 0, invalid = 0;
  std::size_t by_result[Checker::kMismatch + 1] = {};
  std::map<std::string, std::size_t> bad_by_mnemonic;
  int shown = 0;
  for (const auto& [addr, l] : listed) {
    ++total;
    const u8* p = disc->dol.at(addr, 4);
    if (!p || be32(p) != l.word) {
      ++byte_mismatch;
      continue;
    }
    std::string mine = ppc::disasm(l.word, addr);
    if (ppc::decode(l.word) == ppc::Op::invalid) ++invalid;
    auto res = chk.compare(mine, l.text, addr);
    ++by_result[res];
    if (res != Checker::kMismatch) {
      ++same;
      continue;
    }
    auto& nbad = bad_by_mnemonic[l.text.substr(0, l.text.find(' '))];
    if (nbad++ < 4 && shown < 40) {
      std::printf("  %08X %08X  mine: %-40s dtk: %s\n", addr, l.word, mine.c_str(), l.text.c_str());
      ++shown;
    }
  }
  for (const auto& [m, n] : bad_by_mnemonic) std::printf("  mismatched %-12s %zu\n", m.c_str(), n);
  std::printf("identical text %zu; relocated operand equal by value %zu; relocated, base mnemonic kept by dtk %zu; "
              "unlabeled branch shown as displacement %zu; dtk prints ori r0,r0,imm as nop %zu\n",
              by_result[Checker::kExact], by_result[Checker::kRelocValue], by_result[Checker::kRelocAlias],
              by_result[Checker::kBranchDisp], by_result[Checker::kDtkNop]);
  std::printf("%zu .s files, %zu code words listed, %zu match, %zu differ, %zu bytes differ from the DOL, "
              "%zu invalid words (listed as .4byte by both)\n",
              files, total, same, total - same - byte_mismatch, byte_mismatch, invalid);
  bool pass = total > 0 && same == total;
  std::printf("%s\n", pass ? "PASS" : "FAIL");
  return pass ? 0 : 1;
}
