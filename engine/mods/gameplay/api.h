// Gameplay mods: hook API v1, the side a mod is written against
// (docs/mods/ABS_CHALLENGE.md §5).
//
// A mod is host code that runs at retail-address sites (runtime/mod_hooks.h) and
// once per frame poll. It may read and write guest memory and registers. To stay
// deterministic across machines and in netplay, a mod reads only:
// - the guest (memory, registers),
// - its settings (validated, part of the netplay handshake),
// - its random streams (seeded from the match seed),
// - the frame's controller input,
// and keeps every piece of mutable state in its state block, which the engine
// owns, copies into save states and folds into the netplay state hash. No host
// clock, no host randomness, no statics, and no floating point that can differ
// between builds (no libm transcendental functions: integer or table math).
#pragma once

#include <cstdint>
#include <cstring>
#include <span>
#include <string>

#include "ppc/cpu.h"
#include "runtime/input.h"

namespace mvp::mods {

// 1.1: the host keeps the two latest snapshots, and a review restores the later one
// that fits (Host::request_snapshot).
inline constexpr std::uint32_t kHookApiMajor = 1, kHookApiMinor = 1;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// An integer setting: key, range, default. Settings are match settings: they are
// written canonically into the netplay handshake (GameplayHost::canonical_settings).
struct Setting {
  const char* key;
  int min, max, def;
  const char* help;
};

struct ModInfo {
  const char* id;                    // [a-z0-9_]+, also the settings prefix
  const char* name;
  u32 version;                       // the mod's own version; part of the handshake
  std::span<const u32> sites;        // registry sites it binds (runtime/mod_hooks.h)
  std::span<const Setting> settings;
  u32 state_size;                    // bytes of its state block (plain data, no pointers)
};

// Where the guest timeline is (docs/mods/ABS_CHALLENGE.md §7.2). A review
// restores a snapshot taken before the replay's first frame, runs headless to
// that frame (Seeking), shows the replay (Presenting: the only phase
// whose guest state is thrown away afterwards), goes back to the replay's first
// frame and runs headless to the frame poll after the request (Rerunning), then
// play continues (Live). Outside Presenting a mod must behave exactly as it did
// live, so that a re-run reproduces the live game except for what the review
// changes on purpose.
enum class Phase : u8 { Live, Seeking, Presenting, Rerunning };

// A review request. Frames are frame polls (Host::frame()). The payload is the
// mod's own: it survives the restores (Host::review()).
struct Review {
  u64 present_from = 0, present_to = 0;
  u8 payload[16] = {};
};

class Host;

class Mod {
 public:
  virtual ~Mod() = default;
  virtual const ModInfo& info() const = 0;
  // Once, when the match starts (after settings and seed are fixed): initialize the state block.
  virtual void start(Host& h) = 0;
  // At one of info().sites, before the function's first instruction.
  virtual void site(Host& h, ppc::Cpu& c, u32 address) = 0;
  // At every frame poll, after that frame's input was consumed.
  virtual void frame(Host&, ppc::Cpu&) {}
  // When a review this mod requested has re-run to the frame poll after the
  // request, before play continues (Phase::Live again): apply its outcome.
  virtual void review_done(Host&, ppc::Cpu&, const Review&) {}
};

// What the engine offers a mod during its callbacks.
class Host {
 public:
  virtual ~Host() = default;

  // Guest memory: MEM1 (0x80000000/0xC0000000) and the VM region. Anything else
  // (MMIO, unmapped) stops the run with a reason: a mod never touches devices.
  virtual u32 r8(u32 ea) = 0;
  virtual u32 r16(u32 ea) = 0;
  virtual u32 r32(u32 ea) = 0;
  virtual void w8(u32 ea, u32 v) = 0;
  virtual void w16(u32 ea, u32 v) = 0;
  virtual void w32(u32 ea, u32 v) = 0;
  // IEEE single as stored by the guest (bits); compare with integer math where possible.
  float rf32(u32 ea) {
    u32 b = r32(ea);
    float f;
    std::memcpy(&f, &b, 4);
    return f;
  }

  // The calling mod's random stream `stream` (0-3): xoshiro128**, seeded from the
  // match seed, the mod id and the stream; its state is in the save state.
  virtual u32 rng(u32 stream) = 0;
  // The calling mod's setting `key` (validated against its range).
  virtual int setting(const char* key) const = 0;
  // The calling mod's state block (info().state_size bytes, zeroed at start).
  virtual void* state() = 0;
  template <class T>
  T& state_as() {
    static_assert(std::is_trivially_copyable_v<T>, "mod state must be plain data");
    return *static_cast<T*>(state());
  }

  // Frame polls so far and this frame's controller input.
  virtual u64 frame() const = 0;
  virtual const rt::FrameInput& input() const = 0;

  // A line for the developer log (never game state).
  virtual void log(const std::string& line) = 0;
  // A short message for the player, shown over the game (never game state).
  virtual void notify(const std::string& text) = 0;
  // A small JSON object for the mod's own part of the page (its challenge scene,
  // a scoreboard): the page knows the mod's records by their keys. Never game state.
  virtual void publish(const std::string& json) = 0;

  // ---- The timeline: frame-boundary services, carried out at the next frame poll ----
  virtual Phase phase() const = 0;
  // Saves the machine and every mod's state at the next frame poll (Live only).
  // The host keeps the two latest snapshots (1.1; 1.0 kept one): a request
  // replaces the older. snapshot_frame(): the latest one's poll, ~0 before any.
  virtual void request_snapshot() = 0;
  virtual u64 snapshot_frame() const = 0;
  // Reviews from the latest kept snapshot at or before present_from (Live only;
  // needs one, and present_from <= the current frame). The nearer the snapshot,
  // the shorter the headless run to present_from. False when it cannot be done.
  virtual bool request_review(const Review& r) = 0;
  // The review in progress, or null when live.
  virtual const Review* review() const = 0;
};

}  // namespace mvp::mods
