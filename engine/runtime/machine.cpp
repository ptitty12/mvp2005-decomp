#include "runtime/machine.h"

#include <cstring>

namespace mvp::rt {

Machine::Machine() : own_mem1_(kMem1Size + kGuard, 0), mem1_(own_mem1_.data()), mem_(std::make_unique<ppc::Memory>()) {
  // MEM1 through the cached (0x80000000) and uncached (0xC0000000) windows.
  mem_->map(0x80000000u, mem1_, kMem1Size);
  mem_->map(0xC0000000u, mem1_, kMem1Size);
  cpu_.mem = mem_.get();
  cpu_.env = this;
}

Machine::Machine(u8* storage) : mem1_(storage), mem_(std::make_unique<ppc::Memory>()) {
  std::memset(mem1_, 0, kMem1Size + kGuard);
  mem_->map(0x80000000u, mem1_, kMem1Size);
  mem_->map(0xC0000000u, mem1_, kMem1Size);
  cpu_.mem = mem_.get();
  cpu_.env = this;
}

void Machine::map_ram(u32 base, u32 size) {
  if (has_ram(base)) return;
  extra_ram_.emplace_back(base, std::vector<u8>(size + kGuard, 0));
  mem_->map(base, extra_ram_.back().second.data(), size);
}

bool Machine::has_ram(u32 base) const {
  for (const auto& r : extra_ram_)
    if (r.first == base) return true;
  return false;
}

void Machine::set_real_mode(bool on) {
  if (on) {
    mem_->map(0x00000000u, mem1_, kMem1Size);
  } else {
    for (u32 off = 0; off < kMem1Size; off += ppc::Memory::kPageSize) mem_->page[off >> ppc::Memory::kPageShift] = nullptr;
  }
}

void Machine::save(Snapshot& s) const {
  s.cpu = cpu_;
  s.mem1.assign(mem1_, mem1_ + kMem1Size + kGuard);  // keeps the snapshot's capacity: no allocation after the first
  s.extra_ram = extra_ram_;
}

bool Machine::load(const Snapshot& s) {
  if (s.mem1.size() != kMem1Size + kGuard || s.extra_ram.size() != extra_ram_.size()) return false;
  for (std::size_t k = 0; k < extra_ram_.size(); ++k)
    if (s.extra_ram[k].first != extra_ram_[k].first || s.extra_ram[k].second.size() != extra_ram_[k].second.size())
      return false;
  ppc::Memory* mem = cpu_.mem;
  ppc::Hooks* hooks = cpu_.hooks;
  void* env = cpu_.env;
  cpu_ = s.cpu;
  cpu_.mem = mem;
  cpu_.hooks = hooks;
  cpu_.env = env;
  std::memcpy(mem1_, s.mem1.data(), kMem1Size + kGuard);
  for (std::size_t k = 0; k < extra_ram_.size(); ++k)
    std::memcpy(extra_ram_[k].second.data(), s.extra_ram[k].second.data(), extra_ram_[k].second.size());
  constexpr u32 kXlat = ppc::msr::IR | ppc::msr::DR;
  set_real_mode((cpu_.msr & kXlat) != kXlat);
  return true;
}

bool Machine::code_intact(u32 ea) const {
  if (!dol_) return true;
  u32 block = ea & ~31u;
  for (const auto& s : dol_->sections()) {
    if (!s.text || block + 32 <= s.address || block >= s.address + s.size) continue;
    u32 lo = block < s.address ? s.address : block;
    u32 hi = block + 32 > s.address + s.size ? s.address + s.size : block + 32;
    if (std::memcmp(&mem1_[lo & 0x01FFFFFFu], dol_->bytes().data() + s.offset + (lo - s.address), hi - lo) != 0)
      return false;
  }
  return true;
}

void Machine::load_dol(const vfs::Dol& dol) {
  dol_ = &dol;
  for (const auto& s : dol.sections()) {
    u32 off = s.address & 0x3FFFFFFFu;
    if (off + s.size <= kMem1Size) std::memcpy(&mem1_[off], dol.bytes().data() + s.offset, s.size);
  }
  u32 bss = dol.bss_address() & 0x3FFFFFFFu;
  if (bss + dol.bss_size() <= kMem1Size) {
    // The bss range covers small-data sections that the DOL also loads; zero only
    // the bytes no loaded section supplies.
    for (u32 a = 0; a < dol.bss_size(); ++a) {
      u32 addr = dol.bss_address() + a;
      if (!dol.section_at(addr)) mem1_[bss + a] = 0;
    }
  }
  cpu_.pc = dol.entry();
}

}  // namespace mvp::rt
