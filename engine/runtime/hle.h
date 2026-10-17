// HLE: SDK functions the engine replaces with host code, by symbols.txt name
// (E0b item 8, ARCHITECTURE §5), and runtime hooks at game functions, by their
// retail address (E0c).
//
// Replacements: the translator emits, for every function whose name is in
// hle_table() with kind Replace, a stub `f_X` that calls rt::hle_call with the
// table index and a pointer to the translated original (`f_X_orig`). Direct
// calls, indirect calls and the interpreter all reach the stub, so the
// replacement applies everywhere. After the replacement runs, hle_call returns as
// the guest `blr` would (c.pc = LR).
//
// Hooks (kind Before): the function is translated as usual and calls
// rt::hle_before right after its entry (prologue and entry safe point), before
// its first instruction; resuming inside the function does not run it. A hook
// changes host state only (the input queue, logs), never guest state. A hook may
// also stop the run there (YieldingHooks::stop_before, runtime/host_yield.h: the
// frame poll does when netplay waits for an input): the translated function
// returns at once, unwinding, and is entered afresh, hook included, when the run
// continues. Game functions are keyed by address: the retail binary's addresses
// are a stable contract, their placeholder names are not.
//
// Replacements read their arguments from the guest registers and memory and
// must change guest state only as the original would (determinism invariant).
//
// Native implementations (kind Native, Q4, runtime/hle_game.h): replacements of
// game functions, keyed by address, equal to the original in all guest state
// (registers, memory, hardware accesses and their guest times), for speed. They
// call the original when the state they are written for does not hold. The
// interpreter (hle_interpret) runs the original instead, so the lockstep checker
// compares the two.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "runtime/native.h"

namespace mvp::rt {

class InputQueue;

using HleFn = void (*)(Cpu& c, GuestFn original);

enum class HleKind { Replace, Before, Native };

struct HleEntry {
  const char* name;  // symbols.txt name (a label only, when `address` is set)
  HleFn fn;
  const char* why;   // what the replacement or hook does
  u32 address = 0;   // match the function at this address instead of by name
  HleKind kind = HleKind::Replace;
};

// The registry. Its order is part of the translated code (indices are baked in),
// so entries are only ever appended.
const std::vector<HleEntry>& hle_table();
int hle_index(const std::string& name);  // -1 when not replaced

void hle_call(Cpu& c, int index, GuestFn original);
void hle_before(Cpu& c, int index);

// Replaced functions by guest address (registered by the translated code's
// table), so the reference interpreter can apply the same replacements.
struct HleSite {
  u32 address;
  int index;
};
void set_hle_sites(const HleSite* sites, std::size_t count);
int hle_site(u32 address);  // index, or -1
// Runs the replacement at c.pc for the interpreter: as if called (LR already holds
// the return address), then returns to LR. Returns false when c.pc has none.
bool hle_interpret(Cpu& c);

// Where OSReport output goes (a log for the developer, never guest state).
extern void (*g_report)(const std::string& line);
extern std::uint64_t g_report_count;

// Controller input (runtime/input.h, R14): the queue PADRead reads and the frame
// poll hook advances. Null: no controllers are connected.
extern InputQueue* g_input;
// Called at every frame poll after its input was consumed (tools: frame logs,
// frame budgets). Host side only.
extern void (*g_frame_hook)(Cpu& c, std::uint64_t frame);
// Asked at every frame poll before anything is consumed (save states, gameplay
// mods' frame-boundary services): whether the poll may go ahead now. A gate that
// needs the run to return first stops it there (YieldingHooks::stop_before) and
// returns false: the poll then changes nothing, and the guest makes it again,
// unchanged, when the run continues. Null: always.
extern bool (*g_poll_gate)(Cpu& c);
extern std::uint64_t g_frame_polls, g_pad_reads;
// Q1: DCFlushRange, DCStoreRange and their NoSync forms are about to write the
// data cache's lines over [ea, ea + len) back to memory (hooks before them; the
// guest functions run unchanged). The host's model of what the GP reads follows
// (gfx/gp_memory.h). Host side only; null: nothing.
extern void (*g_dc_flush_hook)(u32 ea, u32 len);

// printf-style formatting of a guest format string with the PowerPC EABI
// variadic arguments of the current call (r3 = format when first_gpr = 3).
std::string format_guest(Cpu& c, u32 fmt, int first_gpr);

}  // namespace mvp::rt
