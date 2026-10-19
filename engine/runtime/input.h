// Controller input (DETERMINISM R14, E0c): the only way host input reaches guest
// state.
//
// The unit is the *frame poll*: one pass of the game's frame loop (fn_80148930),
// which polls the controllers once at its start (fn_8014865C, then fn_802D7588 ->
// the pad manager -> fn_8000A44C -> PADRead). The runtime numbers the polls 0, 1,
// 2, ... from power-on. At each poll the input queue consumes exactly one frame's
// input, and every PADRead until the next poll returns it. So a frame's input is
// part of the deterministic state: two runs with the same inputs per frame run the
// same, wherever the inputs come from (a script, a local pad, the other netplay
// peer).
//
// PadState is a versioned engine API (CLAUDE.md): the netplay protocol and input
// recordings carry it in its wire form (encode_frame / decode_frame).
#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace mvp::ppc {
struct Cpu;
}

namespace mvp::rt {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using s8 = std::int8_t;

// Button bits, as the SDK's PADStatus.button has them.
namespace pad {
inline constexpr u16 kLeft = 0x0001, kRight = 0x0002, kDown = 0x0004, kUp = 0x0008;
inline constexpr u16 kZ = 0x0010, kR = 0x0020, kL = 0x0040;
inline constexpr u16 kA = 0x0100, kB = 0x0200, kX = 0x0400, kY = 0x0800, kStart = 0x1000;
inline constexpr u16 kAll = 0x1F7F;
// The name of a button ("A", "START", "LEFT", ...), or 0 for an unknown name.
u16 button_bit(const std::string& name);
}  // namespace pad

inline constexpr int kPorts = 4;

// One controller port in one frame (version 1). Values are what PADRead reports
// after the SDK's origin correction: sticks centred on 0, triggers and analog
// buttons from 0. A disconnected port reads as PAD_ERR_NO_CONTROLLER with zeros.
struct PadState {
  static constexpr u8 kVersion = 1;
  bool connected = false;
  u16 buttons = 0;  // pad::k* bits only
  s8 stick_x = 0, stick_y = 0, substick_x = 0, substick_y = 0;
  u8 trigger_l = 0, trigger_r = 0, analog_a = 0, analog_b = 0;
  bool operator==(const PadState&) const = default;
};

struct FrameInput {
  std::array<PadState, kPorts> pad{};
  bool operator==(const FrameInput&) const = default;
};

// Connected ports with neutral controls (bit k = port k + 1).
FrameInput neutral_input(u32 port_mask);

// Wire form, version 1: one version byte, then per port 11 bytes: flags (bit 0
// connected), buttons (big-endian), stick x, y, substick x, y, trigger l, r,
// analog a, b. 45 bytes per frame.
inline constexpr std::size_t kFrameWireSize = 1 + kPorts * 11;
std::vector<u8> encode_frame(const FrameInput& in);
std::optional<FrameInput> decode_frame(const u8* data, std::size_t size);

// Where frames come from when nothing was pushed for them (a script, a recording,
// the netplay session).
class InputSource {
 public:
  virtual ~InputSource() = default;
  virtual FrameInput input_for(u64 frame) = 0;
  // Asked at the frame poll, before anything is consumed: whether frame `frame`
  // may be polled now. A source whose input may not be known yet (netplay,
  // net/driver.h) can instead stop the run right there
  // (YieldingHooks::stop_before) and return false: the poll then changes
  // nothing, and the guest makes it again, unchanged, when the run continues.
  // The default: always.
  virtual bool before_poll(ppc::Cpu&, u64 /*frame*/) { return true; }
};

// The per-frame input queue.
class InputQueue {
 public:
  // `boot` is what PADRead returns before the first frame poll.
  explicit InputQueue(const FrameInput& boot = neutral_input(1)) : current_(boot) {}

  // Supplies frames that were not pushed (not owned; may be null).
  void set_source(InputSource* s) { source_ = s; }
  // Queues the input for the next frame that has none queued yet.
  void push(const FrameInput& in) { pending_.push_back(in); }
  std::size_t pending() const { return pending_.size(); }
  // Whether the next frame poll has an input (queued, or from the source).
  bool ready() const { return !pending_.empty() || source_; }

  // The frame poll's first step: false when the source stopped the run to wait
  // for the next frame's input (InputSource::before_poll); then nothing changed.
  bool may_begin_frame(ppc::Cpu& c) { return !pending_.empty() || !source_ || source_->before_poll(c, frames_); }
  // A frame poll: consumes frame `frames()`'s input (the queued one, else the
  // source's, else the previous frame's again) and returns that frame's index.
  u64 begin_frame();
  // The input of the current frame (the boot input before the first poll).
  const FrameInput& current() const { return current_; }
  // Frame polls so far.
  u64 frames() const { return frames_; }
  // Save-state files (runtime/state_file.h): continue as a queue that has made
  // `frames` polls, the last with `current`, and has nothing queued. The source
  // stays this queue's.
  void resume_at(const FrameInput& current, u64 frames) {
    pending_.clear();
    current_ = current;
    frames_ = frames;
  }

 private:
  std::deque<FrameInput> pending_;
  InputSource* source_ = nullptr;
  FrameInput current_;
  u64 frames_ = 0;
};

// A scripted input source: a text file of frame-indexed controller events.
//
//   mvp-input 1                 the header line (format version)
//   # comment                   anything after '#' is ignored
//   <frame> <port> <command> [args]
//
// <frame> is a frame poll index (0 = the first poll), <port> 1-4. Commands:
//   press B...        hold buttons (A B X Y Z L R START UP DOWN LEFT RIGHT)
//   release B...      let them go
//   tap N B...        press now, release N frames later
//   stick X Y         main stick (-128..127)
//   cstick X Y        C stick
//   trigger L R       analog triggers (0..255)
//   analog A B        analog A/B (0..255)
//   connect           plug a controller in (neutral)
//   disconnect        unplug it
// State holds until changed. Before frame 0 the state is `initial` (port 1
// connected unless the script says otherwise at frame 0).
class InputScript : public InputSource {
 public:
  static std::optional<InputScript> parse(const std::string& text, std::string* error,
                                          const FrameInput& initial = neutral_input(1));
  FrameInput input_for(u64 frame) override;
  std::size_t events() const { return events_.size(); }
  u64 last_frame() const { return events_.empty() ? 0 : events_.back().frame; }

 private:
  struct Event {
    u64 frame;
    int port;   // 0-3
    int op;     // see input.cpp
    int a, b;   // arguments
    u16 bits;   // buttons
    std::size_t line;  // source order, for a stable sort
  };
  std::vector<Event> events_;  // sorted by (frame, line)
  FrameInput initial_;
  // Incremental evaluation: the state after all events up to `at_frame_`.
  FrameInput state_;
  std::size_t next_ = 0;
  std::optional<u64> at_frame_;
};

}  // namespace mvp::rt
