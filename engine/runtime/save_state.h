// Save states v1 (docs/mods/ABS_CHALLENGE.md §6): the whole emulated machine at a
// stopped frame poll, kept in host memory, restored in the same process.
//
// When: only between rt::run() calls, after the run stopped at a frame poll
// (rt::g_poll_gate with YieldingHooks::stop_before). Then no guest frame lives on
// the host stack, and the guest continues at the frame poll's entry when the run
// does. A state saved there and restored later continues exactly as the saved run
// did with the same inputs (the round-trip check: same guest line).
//
// What: the CPU, MEM1 and the VM region, the virtual clock, every device (ARAM,
// the GX FIFO parser, DSP mailboxes, DVD), the input queue and the frame-poll
// count. Not here: host-side views a tool keeps (the renderer's GX state and
// textures, the GP's view of memory, the audio mixer), which the tool saves or
// resets itself, and the gameplay mods' state (mods/gameplay/host.h), which the
// tool saves alongside. A state is game-derived: it never leaves the machine
// that made it (ARCHITECTURE §3).
#pragma once

#include <cstdint>
#include <memory>

#include "runtime/host_yield.h"
#include "runtime/hw.h"
#include "runtime/input.h"
#include "runtime/machine.h"

namespace mvp::rt {

struct SaveState {
  bool valid = false;
  std::uint64_t frame_polls = 0;  // rt::g_frame_polls when saved
  Machine::Snapshot machine;
  SystemHooks::ClockState clock;
  YieldingHooks::YieldState yield;
  std::unique_ptr<Hardware> hw;
  std::unique_ptr<InputQueue> input;  // null when there is no input queue
};

// Both throw std::logic_error when called inside a run (rt::g_unwinding,
// g_resume, g_yield or an exception handler active). `input` may be null.
void save_state(SaveState& s, Machine& m, YieldingHooks& hooks, Hardware& hw, const InputQueue* input);
// V4: makes `s`'s buffers now (MEM1's, written through, and the hardware's
// snapshot object) so that its first save_state copies without allocating; `s`
// stays empty (valid false). Nothing of the machine changes.
void prepare_state(SaveState& s, Machine& m, const Hardware& hw);
void load_state(const SaveState& s, Machine& m, YieldingHooks& hooks, Hardware& hw, InputQueue* input);

}  // namespace mvp::rt
