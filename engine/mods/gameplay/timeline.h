// The gameplay mods' timeline services (docs/mods/ABS_CHALLENGE.md §7.2): save
// states on request, and reviews that rewind, present and re-run, all at frame
// polls. A tool (mvp-play, the browser shell) installs gate() as rt::g_poll_gate,
// calls after_poll() from its frame hook, and after every rt::run() that returns,
// calls step() while due() is true.
//
// Two save-state slots. Live, they hold the mods' two latest snapshots (a
// request replaces the older). A review, requested at poll K
// (mods::Host::request_review):
// 1. at poll K+1 the run stops; restore the latest snapshot at or before
//    `present_from` (poll P);
// 2. Seeking: run headless to `present_from`, replaying the recorded inputs;
//    save the replay's first frame into the slot just restored (nothing to do
//    when P is `present_from`);
// 3. Presenting: run to `present_to` with the tool presenting (frames shown,
//    guest audio muted); inputs past K repeat K's;
// 4. restore that slot; Rerunning: run headless to poll K+1;
// 5. the requesting mod's review_done(), and play continues live.
// Both slots stay valid afterwards: they precede the call the review changed.
// The guest state at the end of 4 is what live play would have reached had the
// mods decided differently on purpose (the umpire's call); when nothing was
// changed it is exactly the state before the review (logged for checking).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "mods/gameplay/host.h"
#include "runtime/save_state.h"

namespace mvp::mods {

class Timeline {
 public:
  // What the tool does around the steps.
  class Tool {
   public:
    virtual ~Tool() = default;
    // Show frames and play guest audio (true), or run without presenting (false).
    virtual void present(bool on, Phase phase) = 0;
    // Host-side views of the guest to save with a slot and restore with it (the
    // renderer's GX state; caches to drop).
    virtual void save_views(int slot) = 0;
    virtual void load_views(int slot) = 0;
  };

  Timeline(rt::Machine& m, rt::YieldingHooks& hooks, rt::Hardware& hw, rt::InputQueue& input,
           GameplayHost& mods, Tool& tool);

  // rt::g_poll_gate: whether the poll may go ahead (false: the run stopped before it).
  bool gate(ppc::Cpu& c);
  // At each frame poll after its input was consumed: records it for re-runs.
  void after_poll(u64 frame);
  // A step is due (the run stopped for it).
  bool due() const { return due_; }
  // Performs the due step; only between rt::run() calls.
  void step();
  // The tool loaded a save-state file (runtime/state_file.h) in live play: the
  // snapshots and recorded inputs belong to the run before it, so they are
  // dropped, and the state loaded becomes the first snapshot (a review of the
  // next pitch can start there). Only between rt::run() calls, while live.
  void restart();

  Phase phase() const { return mods_.phase(); }
  // How far the current review leg is, 0-1 (1 when live): for the page's challenge scene.
  double progress() const;
  void set_log(std::function<void(const std::string&)> f) { log_ = std::move(f); }

  std::uint64_t snapshots = 0, reviews = 0, restores = 0;
  double snapshot_ms_max = 0, restore_ms_max = 0;

  // The slots by the polls of their snapshots (~0: empty). A live snapshot
  // replaces an empty slot, else the older; a review restores the latest at or
  // before `from` (-1: none).
  static int slot_to_replace(const u64 polls[2]);
  static int slot_for_review(const u64 polls[2], u64 from);

 private:
  static constexpr std::size_t kRing = 4096;  // frame polls of inputs kept for re-runs
  void push_inputs(u64 from, u64 to);  // queue the recorded inputs of polls [from, to)
  void save(int slot);
  void load(int slot);
  void slot_polls(u64 out[2]) const;
  void publish_snapshots();  // tells the mods' host which polls the slots hold
  void begin_leg(Phase phase, u64 target);  // starts a review leg (and times the one before)
  void log(const std::string& s);
  std::string mem1_sha1();

  rt::Machine& m_;
  rt::YieldingHooks& hooks_;
  rt::Hardware& hw_;
  rt::InputQueue& input_;
  GameplayHost& mods_;
  Tool& tool_;
  std::function<void(const std::string&)> log_;

  rt::SaveState slot_[2];
  std::vector<u8> slot_mods_[2];
  std::vector<rt::FrameInput> ring_ = std::vector<rt::FrameInput>(kRing);
  u64 recorded_to_ = 0;  // polls [recorded_to_ - kRing, recorded_to_) are in the ring

  bool due_ = false;
  u64 target_ = ~0ull;    // the poll that ends the current review leg
  u64 leg_start_ = 0;     // the poll it started at
  double leg_t0_ = 0;     // and the host time (ms), for the review's timing line
  double leg_ms_[4] = {};  // host ms per phase (Seeking, Presenting, Rerunning) in this review
  u64 resume_ = 0;      // review: the poll after the request
  int review_slot_ = 0;  // review: the slot restored, then holding the replay's first frame
  std::string live_sha1_;
};

}  // namespace mvp::mods
