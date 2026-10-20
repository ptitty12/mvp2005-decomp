#include "runtime/state_file.h"

#include <cstring>

#include "common/state_io.h"

namespace mvp::rt {

namespace {

constexpr char kMagic[8] = {'M', 'V', 'P', 'S', 'T', 'A', 'T', 'E'};
constexpr char kEnd[4] = {'E', 'N', 'D', '!'};
constexpr std::uint64_t kMaxHeader = 1u << 20;

// The CPU's registers and clock; not its host pointers (mem, hooks, env).
template <class Ar>
void cpu_fields(Ar& a, ppc::Cpu& c) {
  a(c.gpr);
  for (ppc::Fpr& f : c.fpr) {
    a(f.ps0);
    a(f.ps1);
  }
  a(c.cr);
  a(c.lr);
  a(c.ctr);
  a(c.xer_so);
  a(c.xer_ov);
  a(c.xer_ca);
  a(c.xer_low);
  a(c.fpscr);
  a(c.msr);
  a(c.pc);
  a(c.ticks);
  a(c.next_event);
  a(c.reserve);
  a(c.reserve_addr);
  a(c.sr);
  a(c.spr);
}

template <class Ar>
void clock_fields(Ar& a, SystemHooks::ClockState& k, YieldingHooks::YieldState& y) {
  a(k.clk.cpi_milli);
  a(k.tb_offset);
  a(k.dec_base);
  a(k.dec_fire_tb);
  a(k.dec_value);
  a(k.dec_pending);
  a(y.want);
  a(y.armed);
  a(y.pc);
  a(y.ticks);
}

}  // namespace

std::vector<u8> write_state_file(const std::string& header, const SaveState& s, std::span<const u8> mods,
                                 std::span<const u8> views) {
  if (!s.valid || !s.hw) throw std::logic_error("write_state_file: empty save state");
  StateWriter w;
  w.out().reserve(s.machine.mem1.size() + (24u << 20));
  w.bytes(kMagic, sizeof kMagic);
  w(kStateFileVersion);
  w(header);
  auto& st = const_cast<SaveState&>(s);  // the writer only reads
  cpu_fields(w, st.machine.cpu);
  w(s.machine.mem1);
  w(s.machine.extra_ram);
  clock_fields(w, st.clock, st.yield);
  s.hw->write_state(w);
  w(static_cast<u8>(s.input ? 1 : 0));
  if (s.input) {
    w(encode_frame(s.input->current()));
    w(s.input->frames());
  }
  w(s.frame_polls);
  w(std::vector<u8>(mods.begin(), mods.end()));
  w(std::vector<u8>(views.begin(), views.end()));
  s.hw->write_card_state(w);  // version 2
  w.bytes(kEnd, sizeof kEnd);
  return std::move(w.out());
}

std::optional<std::string> state_file_header(std::span<const u8> bytes, std::string* err) {
  try {
    StateReader r(bytes);
    char magic[8];
    r.bytes(magic, sizeof magic);
    if (std::memcmp(magic, kMagic, sizeof magic) != 0) throw StateError("not a save state");
    std::uint32_t version = 0;
    r(version);
    if (version < kStateFileOldestVersion || version > kStateFileVersion)
      throw StateError("a save state of version " + std::to_string(version) + "; this engine reads versions " +
                       std::to_string(kStateFileOldestVersion) + " to " + std::to_string(kStateFileVersion));
    std::uint64_t n = 0;
    r(n);
    if (n > kMaxHeader) throw StateError("save state: a header longer than any engine writes");
    auto h = r.take(static_cast<std::size_t>(n));
    return std::string(h.begin(), h.end());
  } catch (const StateError& e) {
    if (err) *err = e.what();
    return std::nullopt;
  }
}

bool read_state_file(std::span<const u8> bytes, const Hardware& hw, const InputQueue* input, SaveState& s,
                     std::vector<u8>& mods, std::vector<u8>& views, std::string* err) {
  const auto header = state_file_header(bytes, err);
  if (!header) return false;
  try {
    StateReader r(bytes);
    r.take(sizeof kMagic);
    std::uint32_t version = 0;
    r(version);
    r.take(8 + header->size());
    cpu_fields(r, s.machine.cpu);
    r(s.machine.mem1);
    r(s.machine.extra_ram);
    clock_fields(r, s.clock, s.yield);
    s.hw = hw.snapshot();
    s.hw->read_state(r);
    u8 has_input = 0;
    r(has_input);
    s.input.reset();
    if (has_input) {
      std::vector<u8> wire;
      u64 frames = 0;
      r(wire);
      r(frames);
      const auto current = decode_frame(wire.data(), wire.size());
      if (!current) throw StateError("save state: an input frame this engine cannot read");
      if (input) {
        s.input = std::make_unique<InputQueue>(*input);
        s.input->resume_at(*current, frames);
      }
    }
    r(s.frame_polls);
    r(mods);
    r(views);
    if (version >= 2) s.hw->read_card_state(r);
    else s.hw->reset_card_state();  // its machine had no card
    char end[4];
    r.bytes(end, sizeof end);
    if (std::memcmp(end, kEnd, sizeof end) != 0 || r.left() != 0) throw StateError("save state: damaged (no end mark)");
    s.valid = true;
    return true;
  } catch (const StateError& e) {
    s.valid = false;
    if (err) *err = e.what();
    return false;
  }
}

void prepare_machine(Machine& m, const SaveState& s) {
  for (const auto& [base, ram] : s.machine.extra_ram)  // each region's bytes end in Machine::kGuard spare ones
    if (!m.has_ram(base) && ram.size() > Machine::kGuard) m.map_ram(base, static_cast<u32>(ram.size() - Machine::kGuard));
}

}  // namespace mvp::rt
