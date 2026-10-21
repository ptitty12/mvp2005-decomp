// What translated (native C++) guest code calls into, and how the runtime runs it.
//
// Calling convention of a translated function `void f_XXXXXXXX(Cpu&)`:
// - the caller sets LR (as the guest `bl` would) and calls;
// - the guest `blr` sets c.pc = LR & ~3 and returns;
// - after the call the caller checks c.pc against its own return address, and
//   g_unwinding. Either one off means guest code changed the return path (a
//   context switch, a longjmp, an rfi): the native frames are *unwinding*.
//
// Unwinding (D4, E0c) is plain C++ returns, no exceptions. A frame that sees an
// unwinding return goes to its resume block: if the new guest state continues
// in this very activation (same stack pointer as at the call, and c.pc is one of
// this function's resume labels: a call's return address or a loop back edge),
// it clears g_unwinding and jumps there ("resumes in frame"). Otherwise it
// returns too, with g_unwinding set, and the next frame up tries. A dispatcher
// (run, call_indirect's interpreter loop, run_handler) ends the unwind. The top
// level then continues at c.pc: a function entry runs natively; a resume label
// re-enters its function's body there (the function is called with g_resume
// set); anything else runs in the reference interpreter. Native frames hold no
// guest state (only the C++ stack), so whichever frame runs guest code, the
// result is the same: these rules change speed, never behaviour.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "ppc/cpu.h"

namespace mvp::rt {

using ppc::Cpu;
using u32 = std::uint32_t;
using GuestFn = void (*)(Cpu&);

// A translated entry point (or resume label): guest address and its native function.
struct NativeEntry {
  u32 address;
  GuestFn fn;
};

// Set while native frames return because the guest's return path changed (see
// above). Translated code and the dispatchers keep it; nothing guest-visible.
extern bool g_unwinding;
// Set by a dispatcher right before calling a function at a resume label: the
// function skips its prologue and continues at c.pc.
extern bool g_resume;

// Statistics (D4): unwinds that reached the top-level dispatcher (native frames
// abandoned), unwinds that ended in a live frame of the same activation, and
// re-entries of a function body at a resume label.
extern std::uint64_t g_unwinds;
extern std::uint64_t g_inframe_resumes;
extern std::uint64_t g_resume_entries;

// Optional hook run at every translated function entry (tracing, lockstep).
using EntryHook = void (*)(Cpu&, u32 address);
extern EntryHook g_entry_hook;

inline void on_entry(Cpu& c, u32 address) {
  if (g_entry_hook) g_entry_hook(c, address);
}

// Safe points (D4): every translated entry point and every loop back edge. The
// interpreter checks the same addresses, so interrupts land at the same guest
// instruction whichever path runs the code.
class SafeSet {
 public:
  void set(const u32* addrs, std::size_t count);
  bool has(u32 a) const {
    if (a < base_ || (a & 3)) return false;
    std::size_t k = (a - base_) / 4;
    return k < bits_.size() && bits_[k];
  }

 private:
  u32 base_ = 0;
  std::vector<bool> bits_;
};
extern SafeSet g_safe;
// Back edges of pure spin loops: the safe point runs on every pass there (the
// runtime may skip virtual time to the next event), not only when an event is due.
extern SafeSet g_idle;

inline void safepoint_check(Cpu& c) {
  if (c.ticks >= c.next_event) c.hooks->safepoint(c);
}

// An unwinding return starts here: translated code calls this and returns.
inline void leave() { g_unwinding = true; }

[[noreturn]] void trap(Cpu& c, u32 address, const char* why);

// Floating-point unavailable (E3a): the FPU instruction at `at` found MSR[FP]
// clear. Takes the exception through c.hooks (the runtime runs the OS handler,
// which switches the FPU context and returns to `at` with MSR[FP] set). The
// caller then checks c.pc and g_unwinding as after a call.
void fp_unavailable(Cpu& c, u32 at);

// Calls the guest function at `target` (LR already set): native if translated,
// otherwise in the interpreter until it returns to LR (or an unwind passes).
void call_indirect(Cpu& c, u32 target);
// V4: code outside the translated program (code the game writes into its heap)
// may have a translation made while the game runs (runtime/wasm_host.h): when set,
// call_indirect asks it for one before it interprets. It returns a function only
// where that is the same as interpreting from `target` (null: interpret).
using HeapCodeHook = GuestFn (*)(Cpu& c, u32 target);
extern HeapCodeHook g_heap_code_hook;
// bla to an address outside the DOL (code the OS installs in low memory).
void absolute_call(Cpu& c, u32 target);

// The table of translated entry points (set by the translated code's
// registration, see translator/emit_cpp.h).
class NativeTable {
 public:
  void set(const NativeEntry* entries, std::size_t count);
  GuestFn find(u32 address) const;
  bool empty() const { return dense_.empty(); }

 private:
  u32 base_ = 0;
  std::vector<GuestFn> dense_;  // (address - base) / 4
};
extern NativeTable g_native;
// Resume labels: address -> the function (its first entry) whose body has it.
extern NativeTable g_resume_labels;

// A translated function's resume block: whether to continue at c.pc in this
// frame. Returns c.pc when it should (a dispatcher's g_resume, or an unwind that
// arrives at one of `self`'s resume labels with the stack pointer `sp` the frame
// had at its call or safe point), else 0 with g_unwinding set (keep returning).
u32 resume_target(Cpu& c, u32 sp, GuestFn self);

// Runs guest code from c.pc. Whenever c.pc is a translated entry point, the
// native function runs; a resume label re-enters its function's body; anything
// else runs in the reference interpreter. Returns by exception (GuestFault, or
// whatever an Io handler or hook throws to stop the run), or after yield_run().
void run(Cpu& c);

// Host yield (E2): makes run() return so the host can go back to its own event
// loop (a browser between VI fields), by an unwind like a thread switch's: every
// native frame returns (resume_target never resumes in frame while it is set)
// and the top-level dispatcher returns. The next run() continues at c.pc. Call
// it from a safe point outside interrupt handlers (runtime/host_yield.h does).
extern bool g_yield;
inline void yield_run() {
  g_yield = true;
  g_unwinding = true;
}

// Runs an exception handler entered at c.pc (SRR0/SRR1 set) until the exception
// ends: an rfi in the interpreter that sets MSR[EE] or lands at `resume_pc` with
// the stack pointer `sp` (a synchronous exception in code that runs with EE
// clear), or an rfi in translated code (OSLoadContext), whose unwind ends here. Back at the interrupted point (an rfi
// to `resume_pc` with the stack pointer `sp`) it returns normally; anywhere else
// (the scheduler switched threads) it returns with g_unwinding set and c.pc at
// the new thread, and the interrupted frames unwind from their safe point.
void run_handler(Cpu& c, u32 resume_pc, u32 sp);

// Instructions run by the interpreter in run() and run_handler(), for the
// interpreted-share statistic (D4, E0b item 9).
extern std::uint64_t g_interpreted;
// While an interrupt is being handled: the depth (1 inside a handler) and the
// interrupted code's LR. Function entries reached with LR still equal to it were
// reached from the vector by rfi and by branches, not by calls (tools that
// attribute entries to call sites use this).
extern int g_exception_depth;
extern u32 g_exception_lr;

}  // namespace mvp::rt
