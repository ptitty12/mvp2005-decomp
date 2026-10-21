#include "runtime/native.h"

#include <algorithm>
#include <cstdio>

#include "ppc/interp.h"

namespace mvp::rt {

EntryHook g_entry_hook = nullptr;
bool g_unwinding = false;
bool g_resume = false;
bool g_yield = false;
std::uint64_t g_unwinds = 0;
std::uint64_t g_inframe_resumes = 0;
std::uint64_t g_resume_entries = 0;
std::uint64_t g_interpreted = 0;
int g_exception_depth = 0;
u32 g_exception_lr = 0;
NativeTable g_native;
NativeTable g_resume_labels;
SafeSet g_safe;
SafeSet g_idle;

void SafeSet::set(const u32* addrs, std::size_t count) {
  bits_.clear();
  if (!count) return;
  u32 lo = addrs[0], hi = addrs[0];
  for (std::size_t k = 0; k < count; ++k) {
    lo = std::min(lo, addrs[k]);
    hi = std::max(hi, addrs[k]);
  }
  base_ = lo;
  bits_.assign((hi - lo) / 4 + 1, false);
  for (std::size_t k = 0; k < count; ++k) bits_[(addrs[k] - lo) / 4] = true;
}

void trap(Cpu& c, u32 address, const char* why) {
  char b[128];
  std::snprintf(b, sizeof b, "guest trap at 0x%08X: %s", address, why);
  c.pc = address;
  throw ppc::GuestFault(address, b);
}

void NativeTable::set(const NativeEntry* entries, std::size_t count) {
  dense_.clear();
  if (!count) return;
  u32 lo = entries[0].address, hi = entries[0].address;
  for (std::size_t k = 0; k < count; ++k) {
    lo = std::min(lo, entries[k].address);
    hi = std::max(hi, entries[k].address);
  }
  base_ = lo;
  dense_.assign((hi - lo) / 4 + 1, nullptr);
  for (std::size_t k = 0; k < count; ++k) dense_[(entries[k].address - lo) / 4] = entries[k].fn;
}

GuestFn NativeTable::find(u32 address) const {
  if (address < base_ || (address & 3)) return nullptr;
  std::size_t k = (address - base_) / 4;
  return k < dense_.size() ? dense_[k] : nullptr;
}

u32 resume_target(Cpu& c, u32 sp, GuestFn self) {
  if (g_resume) {  // a dispatcher re-enters this body at c.pc
    g_resume = false;
    return c.pc;
  }
  if (!g_yield && c.gpr[1] == sp && g_resume_labels.find(c.pc) == self) {
    // The guest continues in this activation (same function, same stack frame):
    // the unwind ends here, and this frame goes on natively.
    g_unwinding = false;
    ++g_inframe_resumes;
    return c.pc;
  }
  g_unwinding = true;
  return 0;
}

namespace {

// Runs from c.pc, entering native code at translated entries, until c.pc == stop
// at an instruction boundary (nested: an interpreted call made by translated
// code), or forever (the top level). A nested loop returns as soon as an unwind
// passes through it, so the translated frames below it unwind too; the top level
// ends the unwind and goes on at c.pc, re-entering native code at resume labels,
// or returns if the unwind was a host yield (g_yield).
void dispatch(Cpu& c, bool nested, u32 stop) {
  auto unwound = [&]() {
    if (!g_unwinding) return false;
    if (nested) return true;
    g_unwinding = false;
    ++g_unwinds;  // native frames are gone; continue at c.pc
    if (g_yield) {  // ... in the next run()
      g_yield = false;
      return true;
    }
    return false;
  };
  for (;;) {
    if (nested && c.pc == stop) return;
    if (GuestFn fn = g_native.find(c.pc)) {
      fn(c);
      if (unwound()) return;
      continue;
    }
    if (!nested) {
      if (GuestFn fn = g_resume_labels.find(c.pc)) {
        ++g_resume_entries;
        g_resume = true;
        fn(c);
        if (unwound()) return;
        continue;
      }
    }
    if ((c.ticks >= c.next_event && g_safe.has(c.pc)) || g_idle.has(c.pc)) {
      u32 at = c.pc;
      c.hooks->safepoint(c);
      if (unwound()) return;
      if (c.pc != at) continue;  // an interrupt moved on to other code
    }
    ppc::step(c);
    ++g_interpreted;
    if (unwound()) return;  // a floating-point unavailable handler switched threads
  }
}

}  // namespace

HeapCodeHook g_heap_code_hook = nullptr;

void call_indirect(Cpu& c, u32 target) {
  u32 ret = c.lr;
  c.pc = target;
  if (GuestFn fn = g_native.find(target)) {
    fn(c);
    return;
  }
  if (g_heap_code_hook)
    if (GuestFn fn = g_heap_code_hook(c, target)) {  // V4: heap code, translated
      fn(c);
      return;
    }
  dispatch(c, true, ret & ~3u);
}

void fp_unavailable(Cpu& c, u32 at) {
  c.pc = at;
  c.hooks->exception(c, 0x800);
}

void absolute_call(Cpu& c, u32 target) {
  // Code the OS copies into low memory at run time. Not translated: interpret it.
  call_indirect(c, target < 0x01800000u ? target | 0x80000000u : target);
}

void run_handler(Cpu& c, u32 resume_pc, u32 sp) {
  // The OS's exception code itself uses rfi (from the real-mode vector into the
  // handler), always with MSR[EE] clear. An rfi that sets EE ends the exception:
  // back to the interrupted point, or into another thread. An rfi into a thread
  // that runs with EE clear keeps running here, in the interpreter, until that
  // thread's own rfi or a translated OSLoadContext (an unwind) settles it.
  auto at_resume = [&] { return c.pc == resume_pc && c.gpr[1] == sp; };
  for (;;) {
    if (GuestFn fn = g_native.find(c.pc)) {
      fn(c);
      if (g_unwinding) {
        // An rfi in translated code (or a switch below it) ended the exception.
        if (at_resume()) g_unwinding = false;
        return;
      }
      if (at_resume()) return;
      continue;
    }
    ppc::Op op = ppc::step(c);
    ++g_interpreted;
    if (g_unwinding) {  // a nested exception (floating-point unavailable) switched threads
      if (at_resume()) g_unwinding = false;
      return;
    }
    if (op == ppc::Op::rfi && ((c.msr & ppc::msr::EE) || at_resume())) {
      if (!at_resume()) g_unwinding = true;  // another thread resumes
      return;
    }
  }
}

void run(Cpu& c) { dispatch(c, false, 0); }

}  // namespace mvp::rt
