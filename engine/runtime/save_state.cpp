#include "runtime/save_state.h"

#include <stdexcept>

#include "runtime/hle.h"
#include "runtime/native.h"

namespace mvp::rt {

namespace {

void check_idle(const char* what) {
  if (g_unwinding || g_resume || g_yield || g_exception_depth)
    throw std::logic_error(std::string(what) + ": only between rt::run() calls, outside handlers");
}

}  // namespace

void save_state(SaveState& s, Machine& m, YieldingHooks& hooks, Hardware& hw, const InputQueue* input) {
  check_idle("save_state");
  m.save(s.machine);
  s.clock = hooks.clock_state();
  s.yield = hooks.yield_state();
  if (s.hw) hw.save_into(*s.hw);
  else s.hw = hw.snapshot();
  if (input) {
    if (s.input) *s.input = *input;
    else s.input = std::make_unique<InputQueue>(*input);
  } else {
    s.input.reset();
  }
  s.frame_polls = g_frame_polls;
  s.valid = true;
}

void load_state(const SaveState& s, Machine& m, YieldingHooks& hooks, Hardware& hw, InputQueue* input) {
  check_idle("load_state");
  if (!s.valid) throw std::logic_error("load_state: empty save state");
  if (!m.load(s.machine)) throw std::logic_error("load_state: the machine's RAM regions changed since the save");
  hooks.set_clock_state(s.clock);
  hooks.set_yield_state(s.yield);
  hw.restore_from(*s.hw);
  if (input && s.input) *input = *s.input;
  g_frame_polls = s.frame_polls;
}

}  // namespace mvp::rt
