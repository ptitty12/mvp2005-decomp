#include "web/wasm/module_cache.h"

#include <cstring>

namespace mvp::web {

namespace {

constexpr char kMagic[8] = {'M', 'V', 'P', 'X', 'L', 'A', 'T', '\0'};
constexpr std::uint32_t kVersion = 3;  // V4: 2 carries WasmModule::fixed, 3 the structured count

// If this fails, xlat::WasmModule or WasmEmitStats changed: carry the new
// fields below and bump kVersion (the cache key changes with the engine build
// anyway; this keeps a warm start equal to a fresh translation).
static_assert(sizeof(xlat::WasmModule) == 196 && sizeof(xlat::WasmEmitStats) == 68,
              "update serialize_module/deserialize_module for the changed WasmModule");

struct Writer {
  std::vector<std::uint8_t> out;
  void u32(std::uint32_t v) {
    for (int k = 0; k < 4; ++k) out.push_back(static_cast<std::uint8_t>(v >> (8 * k)));
  }
  void u64(std::uint64_t v) {
    u32(static_cast<std::uint32_t>(v));
    u32(static_cast<std::uint32_t>(v >> 32));
  }
  void slots(const std::vector<xlat::WasmSlot>& v) {
    u32(static_cast<std::uint32_t>(v.size()));
    for (const auto& s : v) {
      u32(s.address);
      u32(s.slot);
    }
  }
  void words(const std::vector<std::uint32_t>& v) {
    u32(static_cast<std::uint32_t>(v.size()));
    for (std::uint32_t w : v) u32(w);
  }
};

struct Reader {
  const std::uint8_t* p;
  std::size_t n, at = 0;
  bool ok = true;
  std::uint32_t u32() {
    if (n - at < 4) {
      ok = false;
      return 0;
    }
    std::uint32_t v = 0;
    for (int k = 0; k < 4; ++k) v |= static_cast<std::uint32_t>(p[at + k]) << (8 * k);
    at += 4;
    return v;
  }
  std::uint64_t u64() {
    const std::uint64_t lo = u32();
    return lo | (static_cast<std::uint64_t>(u32()) << 32);
  }
  // A count of `each`-byte records that must fit in what is left.
  std::uint32_t count(std::size_t each) {
    const std::uint32_t c = u32();
    if (ok && static_cast<std::uint64_t>(c) * each > n - at) ok = false;
    return ok ? c : 0;
  }
  void slots(std::vector<xlat::WasmSlot>& v) {
    v.resize(count(8));
    for (auto& s : v) {
      s.address = u32();
      s.slot = u32();
    }
  }
  void words(std::vector<std::uint32_t>& v) {
    v.resize(count(4));
    for (auto& w : v) w = u32();
  }
};

}  // namespace

std::vector<std::uint8_t> serialize_module_header(const xlat::WasmModule& m) {
  return serialize_module_header(m, m.bytes.size());
}

std::vector<std::uint8_t> serialize_module_header(const xlat::WasmModule& m, std::uint64_t bytes_len) {
  Writer w;
  w.out.reserve(64 + 8 * (m.entries.size() + m.resume.size() + m.resume_index.size()) +
                4 * (m.safe.size() + m.idle.size()) + 8 * m.hle_sites.size() + 8 * m.stats.helper_by_op.size());
  w.out.insert(w.out.end(), kMagic, kMagic + 8);
  w.u32(kVersion);
  w.u32(m.slots);
  w.slots(m.entries);
  w.slots(m.resume);
  w.u32(static_cast<std::uint32_t>(m.resume_index.size()));
  for (const auto& [a, k] : m.resume_index) {
    w.u32(a);
    w.u32(k);
  }
  w.words(m.safe);
  w.words(m.idle);
  w.u32(static_cast<std::uint32_t>(m.hle_sites.size()));
  for (const auto& [a, k] : m.hle_sites) {
    w.u32(a);
    w.u32(static_cast<std::uint32_t>(k));
  }
  w.u32(static_cast<std::uint32_t>(m.hle_matched.size()));
  for (int k : m.hle_matched) w.u32(static_cast<std::uint32_t>(k));
  const auto& f = m.fixed;  // V4
  for (std::uint32_t v : {f.unwinding, f.resume, f.hook, f.wgp, f.gp_pages, f.mem1_base, f.mem1_limit}) w.u32(v);
  const auto& s = m.stats;
  for (std::size_t v : {s.functions, s.wasm_functions, s.entries, s.instructions, s.inline_insns, s.helper_insns,
                        s.branch_insns, s.resume_labels, s.dispatch_functions, s.spills, s.reloads, s.hle_stubs,
                        s.code_bytes, s.structured_functions})
    w.u64(v);
  w.u32(static_cast<std::uint32_t>(s.helper_by_op.size()));
  for (std::size_t v : s.helper_by_op) w.u64(v);
  w.u64(bytes_len);
  return std::move(w.out);
}

std::vector<std::uint8_t> serialize_module(const xlat::WasmModule& m) {
  std::vector<std::uint8_t> out = serialize_module_header(m);
  out.insert(out.end(), m.bytes.begin(), m.bytes.end());
  return out;
}

bool deserialize_module(const std::uint8_t* p, std::size_t n, xlat::WasmModule* m, std::string* error) {
  std::size_t at = 0;
  if (!deserialize_module_tables(p, n, m, &at, error)) return false;
  m->bytes.assign(p + at, p + n);
  return true;
}

bool deserialize_module_tables(const std::uint8_t* p, std::size_t n, xlat::WasmModule* m, std::size_t* bytes_at,
                               std::string* error) {
  auto fail = [&](const char* why) {
    if (error) *error = std::string("cached module: ") + why;
    return false;
  };
  if (n < 12 || std::memcmp(p, kMagic, 8) != 0) return fail("not a cached module");
  Reader r{p, n, 8};
  if (r.u32() != kVersion) return fail("other format version");
  xlat::WasmModule out;
  out.slots = r.u32();
  r.slots(out.entries);
  r.slots(out.resume);
  out.resume_index.resize(r.count(8));
  for (auto& [a, k] : out.resume_index) {
    a = r.u32();
    k = r.u32();
  }
  r.words(out.safe);
  r.words(out.idle);
  out.hle_sites.resize(r.count(8));
  for (auto& [a, k] : out.hle_sites) {
    a = r.u32();
    k = static_cast<int>(r.u32());
  }
  out.hle_matched.resize(r.count(4));
  for (int& k : out.hle_matched) k = static_cast<int>(r.u32());
  auto& f = out.fixed;  // V4
  for (std::uint32_t* v : {&f.unwinding, &f.resume, &f.hook, &f.wgp, &f.gp_pages, &f.mem1_base, &f.mem1_limit})
    *v = r.u32();
  auto& s = out.stats;
  for (std::size_t* v : {&s.functions, &s.wasm_functions, &s.entries, &s.instructions, &s.inline_insns,
                         &s.helper_insns, &s.branch_insns, &s.resume_labels, &s.dispatch_functions, &s.spills,
                         &s.reloads, &s.hle_stubs, &s.code_bytes, &s.structured_functions})
    *v = static_cast<std::size_t>(r.u64());
  s.helper_by_op.resize(r.count(8));
  for (std::size_t& v : s.helper_by_op) v = static_cast<std::size_t>(r.u64());
  const std::uint64_t len = r.u64();
  if (!r.ok || len != n - r.at) return fail("truncated or corrupt");
  *bytes_at = r.at;
  *m = std::move(out);
  return true;
}

}  // namespace mvp::web
