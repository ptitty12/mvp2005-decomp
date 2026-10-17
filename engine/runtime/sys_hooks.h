// The runtime's answers to the CPU hooks (ppc::Hooks): the virtual clock (time
// base and decrementer), interrupt delivery at safe points, the idle skip, MSR
// changes, icbi and sc (E0b items 7 and 9).
//
// Virtual clock (DETERMINISM R1): the time base is derived only from the number
// of guest instructions run (c.ticks) at a fixed charge of CPU cycles per
// instruction (ClockConfig); the CPU runs at 486 MHz and the time base at 1/12 of
// that. Host time never enters. Interrupts (decrementer, and PI causes from the hardware
// model) are delivered only at safe points (function entries and loop back
// edges) while MSR[EE] is set (R4), by running the OS's own exception vector code.
// The floating-point unavailable exception is synchronous: it is taken at the
// FPU instruction that raises it (MSR[FP] clear, ppc/isa.h uses_fpu).
#pragma once

#include <cstdint>

#include "ppc/cpu.h"
#include "runtime/machine.h"

namespace mvp::rt {

class Hardware;

// The charge per guest instruction, in thousandths of a CPU cycle: the time base
// advances ticks * cpi_milli / 12000 (12 CPU cycles per time-base tick). Part of
// the netplay handshake. E0b-CLK1 charged 2 cycles (2000); CLK2 charges 1.25
// (388.8 million instructions per guest second), with which the game's heaviest
// play still fits a 60 Hz frame (docs/engine/CLK2_LOG.md).
struct ClockConfig {
  static constexpr std::uint32_t kMilliPerTb = 12000;  // thousandths of a CPU cycle per time-base tick
  std::uint32_t cpi_milli = 1250;
};

class SystemHooks : public ppc::Hooks {
 public:
  explicit SystemHooks(Machine* m, Hardware* hw = nullptr, ClockConfig clk = {}) : m_(m), hw_(hw), clk_(clk) {}

  // Time base for the instruction count `ticks`.
  std::uint64_t tb_at(std::uint64_t ticks) const { return tb_offset_ + ticks * clk_.cpi_milli / ClockConfig::kMilliPerTb; }
  // First instruction count at which the time base reaches `tb`.
  std::uint64_t ticks_for(std::uint64_t tb) const;

  std::uint64_t time_base(ppc::Cpu& c) override { return tb_at(c.ticks); }
  void set_time_base(ppc::Cpu& c, std::uint64_t v) override;
  const ClockConfig& clock() const { return clk_; }
  // Charges `clk` from the current instruction on; the time base stays where it
  // is (experiments only, mvp-play --cpi-switch: a run's charge is otherwise fixed).
  void set_clock(ppc::Cpu& c, ClockConfig clk);
  u32 decrementer(ppc::Cpu& c) override;
  void set_decrementer(ppc::Cpu& c, u32 v) override;
  // Translation off (IR or DR clear) maps physical MEM1 at 0 (real mode); EE set
  // may make a pending interrupt deliverable at the next safe point.
  void msr_changed(ppc::Cpu& c, u32 old) override;
  // Translated code is only valid for the DOL's bytes: invalidating a changed
  // code block is a hard error (the vector area below 0x80003000 is not DOL code).
  void icbi(ppc::Cpu& c, u32 ea) override;
  // The OS's system call only flushes write buffers (HID0[ABE] toggle, rfi):
  // nothing to do here.
  void system_call(ppc::Cpu&) override { ++syscalls; }
  // mtspr WPAR: the write-gather pipe restarts empty (Hardware::wgp_reset).
  void write_pipe_reset(ppc::Cpu&) override;
  // Runs due events, delivers a pending interrupt, applies the idle skip, and
  // stops the run (StopRun) once `tick_limit` instructions have run.
  void safepoint(ppc::Cpu& c) override;
  // A synchronous exception (floating-point unavailable, 0x800) raised by the
  // instruction at c.pc: delivered like an interrupt, and its handler runs until
  // it returns to that instruction (or switches threads, as an interrupt may).
  void exception(ppc::Cpu& c, u32 vector) override;

  // Recomputes c.next_event from the scheduled events.
  void reschedule(ppc::Cpu& c);

 private:
  // Q4: the same, where the caller knows whether a hardware interrupt is pending.
  void reschedule(ppc::Cpu& c, bool external_pending);

 public:
  // Save states (runtime/save_state.h): the clock's own state (time-base offset,
  // decrementer, charge). The counters below are statistics and are not included.
  struct ClockState {
    ClockConfig clk;
    std::uint64_t tb_offset = 0, dec_base = 0, dec_fire_tb = ~0ull;
    u32 dec_value = 0;
    bool dec_pending = false;
  };
  ClockState clock_state() const { return {clk_, tb_offset_, dec_base_, dec_fire_tb_, dec_value_, dec_pending_}; }
  void set_clock_state(const ClockState& s) {
    clk_ = s.clk;
    tb_offset_ = s.tb_offset;
    dec_base_ = s.dec_base;
    dec_fire_tb_ = s.dec_fire_tb;
    dec_value_ = s.dec_value;
    dec_pending_ = s.dec_pending;
  }

  std::uint64_t tick_limit = ~0ull;
  std::uint64_t syscalls = 0, icbi_blocks = 0, real_mode_switches = 0;
  std::uint64_t external_interrupts = 0, decrementer_interrupts = 0, idle_skips = 0, idle_ticks_skipped = 0;
  std::uint64_t fp_unavailable = 0;  // floating-point unavailable exceptions (lazy FPU context switches)

 private:
  void deliver(ppc::Cpu& c, u32 vector);

  Machine* m_;
  Hardware* hw_;
  ClockConfig clk_;
  std::uint64_t tb_offset_ = 0;
  // Decrementer: value `dec_value_` written at time base `dec_base_`.
  u32 dec_value_ = 0;
  std::uint64_t dec_base_ = 0;
  std::uint64_t dec_fire_tb_ = ~0ull;  // when it next turns negative
  bool dec_pending_ = false;
};

}  // namespace mvp::rt
