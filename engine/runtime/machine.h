// A GameCube machine for running the retail DOL: MEM1, the CPU, and the DOL
// loaded where the apploader would put it.
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "ppc/cpu.h"
#include "vfs/dol.h"

namespace mvp::rt {

using u8 = std::uint8_t;
using u32 = std::uint32_t;

class Machine {
 public:
  static constexpr u32 kMem1Size = 0x01800000;  // 24 MiB
  static constexpr u32 kGuard = 64;             // bytes past the end, for unaligned accesses

  Machine();
  // V4: MEM1 in `storage` (kMem1Size + kGuard bytes, zeroed here) instead of a
  // buffer of its own, which the caller keeps for the machine's lifetime: the
  // browser's engine keeps MEM1 at a fixed address that its translated module
  // has as a constant (xlat::WasmFixed).
  explicit Machine(u8* storage);
  Machine(const Machine&) = delete;
  Machine& operator=(const Machine&) = delete;

  // Copies the DOL's sections into MEM1, zeroes .bss, sets the PC to the entry.
  // Remembers the DOL for code_intact().
  void load_dol(const vfs::Dol& dol);

  // Whether the 32-byte block at `ea` still holds the DOL's bytes (true outside
  // the DOL's text sections). Translated code is only valid while it does: the
  // icbi hook checks this for every block the guest invalidates (overlays the
  // game reloads must be byte-identical to the DOL image).
  bool code_intact(u32 ea) const;

  // Real mode (MSR[IR]/[DR] clear) addresses physical memory directly: MEM1 is
  // then also visible at 0x00000000. With translation on it is not (only the BAT
  // windows at 0x80000000 and 0xC0000000 are), as on the console.
  void set_real_mode(bool on);

  // Plain RAM at a guest address range (the VM region, DETERMINISM R10). Part of
  // the machine state like MEM1. Idempotent for the same range.
  void map_ram(u32 base, u32 size);
  bool has_ram(u32 base) const;

  // Save states (runtime/save_state.h): the CPU and every RAM region. load()
  // needs the same RAM regions as save() saw and keeps the CPU's host pointers.
  struct Snapshot {
    ppc::Cpu cpu;
    std::vector<u8> mem1;
    std::vector<std::pair<u32, std::vector<u8>>> extra_ram;
  };
  void save(Snapshot& s) const;
  bool load(const Snapshot& s);

  ppc::Cpu& cpu() { return cpu_; }
  ppc::Memory& memory() { return *mem_; }
  u8* mem1() { return mem1_; }
  const u8* mem1() const { return mem1_; }

 private:
  const vfs::Dol* dol_ = nullptr;
  std::vector<u8> own_mem1_;  // MEM1 unless the caller gave storage
  u8* mem1_ = nullptr;        // kMem1Size + kGuard bytes
  std::vector<std::pair<u32, std::vector<u8>>> extra_ram_;
  std::unique_ptr<ppc::Memory> mem_;
  ppc::Cpu cpu_;
};

}  // namespace mvp::rt
