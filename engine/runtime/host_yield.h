// Host yields (E2): rt::run() returns between VI fields, so that a browser can
// go back to its event loop (show the frame, read input) and call run() again
// for the next field.
//
// The guest cannot tell. A yield is an unwind like a thread switch's
// (runtime/native.h): native frames return, the next run() continues at c.pc
// and re-enters functions at their resume labels. It is taken at the end of a
// safe point, after the safe point's events and interrupts, and never inside
// an interrupt handler (it waits for the next safe point outside one). When
// the guest continues at that very safe point, the re-entered code calls the
// safe point again (resume labels sit before it): that first call, at the same
// PC with the same instruction count, returns at once, because the call the
// yield interrupted already did all of its work. So the guest runs exactly the
// same instructions with or without yields; `mvp-play --yield` and the browser
// shell check it against runs without them (docs/engine/WEB.md).
//
// A requested yield waits for a safe point the runtime stops at, that is one
// where an event is due (c.ticks >= c.next_event): the guest may pass many safe
// points, and run a lot of code, before that. stop_before() stops the run at
// once instead, at a hook called right after a function's entry safe point
// (E5c: the frame poll, when netplay waits for the next frame's input).
#pragma once

#include <cstdint>

#include "runtime/sys_hooks.h"

namespace mvp::rt {

class YieldingHooks : public SystemHooks {
 public:
  using SystemHooks::SystemHooks;

  // Asks run() to return after the current safe point (or the next one outside
  // interrupt handlers where an event is due, see above). Call it from inside the
  // run, e.g. from a GfxObserver's vi_field.
  void request_yield() { want_ = true; }
  bool yield_pending() const { return want_; }

  // Makes run() return right away, before the code at c.pc runs. Only from a
  // hook called by translated code right after a function's entry safe point
  // (runtime/hle.h), which returns at once, unwinding: c.pc is the entry, and the
  // next run() calls the function again. That call repeats the entry's safe
  // point, which is skipped (as after any yield): the stopped call already passed
  // it. The guest state is the same as without the stop. Returns false, and does
  // nothing, inside an interrupt handler (a run cannot stop there). A pending
  // request_yield() counts as served.
  bool stop_before(ppc::Cpu& c);

  void safepoint(ppc::Cpu& c) override;

  std::uint64_t yields = 0, repeats_skipped = 0, stops = 0;

  // Save states: whether the next safe point repeats the one a stop or yield
  // interrupted (a state saved at a stopped frame poll restores it too).
  struct YieldState {
    bool want = false, armed = false;
    std::uint32_t pc = 0;
    std::uint64_t ticks = 0;
  };
  YieldState yield_state() const { return {want_, armed_, pc_, ticks_}; }
  void set_yield_state(const YieldState& s) {
    want_ = s.want;
    armed_ = s.armed;
    pc_ = s.pc;
    ticks_ = s.ticks;
  }

 private:
  bool want_ = false;
  bool armed_ = false;  // the next safe point may be the one a yield interrupted
  std::uint32_t pc_ = 0;
  std::uint64_t ticks_ = 0;
};

}  // namespace mvp::rt
