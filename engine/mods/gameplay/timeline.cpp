#include "mods/gameplay/timeline.h"

#include <chrono>
#include <cstdio>
#include <stdexcept>

#include "common/sha1.h"
#include "runtime/hle.h"

namespace mvp::mods {

namespace {

double ms_since(std::chrono::steady_clock::time_point t0) {
  return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

double now_ms() {
  return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

}  // namespace

Timeline::Timeline(rt::Machine& m, rt::YieldingHooks& hooks, rt::Hardware& hw, rt::InputQueue& input,
                   GameplayHost& mods, Tool& tool)
    : m_(m), hooks_(hooks), hw_(hw), input_(input), mods_(mods), tool_(tool) {}

bool Timeline::gate(ppc::Cpu& c) {
  const u64 poll = rt::g_frame_polls;  // the poll about to happen
  const bool stop = mods_.phase() == Phase::Live ? (mods_.snapshot_requested() || mods_.review_requested())
                                                 : poll >= target_;
  if (!stop || !hooks_.stop_before(c)) return true;
  due_ = true;
  return false;
}

double Timeline::progress() const {
  if (mods_.phase() == Phase::Live || target_ == ~0ull || target_ <= leg_start_) return 1.0;
  const double done = static_cast<double>(rt::g_frame_polls - leg_start_) / static_cast<double>(target_ - leg_start_);
  return done < 0 ? 0 : done > 1 ? 1 : done;
}

void Timeline::begin_leg(Phase phase, u64 target) {
  const double now = now_ms();
  const Phase was = mods_.phase();
  if (was != Phase::Live) leg_ms_[static_cast<int>(was)] += now - leg_t0_;
  leg_t0_ = now;
  leg_start_ = rt::g_frame_polls;
  target_ = target;
  mods_.set_phase(phase);
  tool_.present(phase == Phase::Presenting || phase == Phase::Live, phase);
}

void Timeline::after_poll(u64 frame) {
  ring_[frame % kRing] = input_.current();
  if (frame + 1 > recorded_to_) recorded_to_ = frame + 1;
}

void Timeline::push_inputs(u64 from, u64 to) {
  for (u64 f = from; f < to; ++f) {
    u64 at = f;
    if (at >= recorded_to_) at = recorded_to_ ? recorded_to_ - 1 : 0;  // past what was played: hold the last input
    if (recorded_to_ > kRing && at < recorded_to_ - kRing)
      throw rt::StopRun{"timeline: the review reaches back further than the recorded inputs"};
    input_.push(ring_[at % kRing]);
  }
}

void Timeline::save(int slot) {
  const auto t0 = std::chrono::steady_clock::now();
  rt::save_state(slot_[slot], m_, hooks_, hw_, &input_);
  slot_mods_[slot] = mods_.save_state();
  tool_.save_views(slot);
  ++snapshots;
  const double ms = ms_since(t0);
  if (ms > snapshot_ms_max) snapshot_ms_max = ms;
}

void Timeline::load(int slot) {
  const auto t0 = std::chrono::steady_clock::now();
  rt::load_state(slot_[slot], m_, hooks_, hw_, &input_);
  if (!mods_.load_state(slot_mods_[slot])) throw rt::StopRun{"timeline: the mods' state does not fit the snapshot"};
  tool_.load_views(slot);
  ++restores;
  const double ms = ms_since(t0);
  if (ms > restore_ms_max) restore_ms_max = ms;
}

int Timeline::slot_to_replace(const u64 polls[2]) {
  if (polls[0] == ~0ull) return 0;
  if (polls[1] == ~0ull) return 1;
  return polls[1] < polls[0] ? 1 : 0;
}

int Timeline::slot_for_review(const u64 polls[2], u64 from) {
  int k = -1;
  for (int j = 0; j < 2; ++j)
    if (polls[j] != ~0ull && polls[j] <= from && (k < 0 || polls[j] > polls[k])) k = j;
  return k;
}

void Timeline::slot_polls(u64 out[2]) const {
  for (int k = 0; k < 2; ++k) out[k] = slot_[k].valid ? slot_[k].frame_polls : ~0ull;
}

void Timeline::publish_snapshots() {
  u64 polls[2];
  slot_polls(polls);
  u64 latest = ~0ull, earliest = ~0ull;
  for (u64 p : polls) {
    if (p == ~0ull) continue;
    if (latest == ~0ull || p > latest) latest = p;
    if (p < earliest) earliest = p;
  }
  mods_.set_snapshots(latest, earliest);
}

std::string Timeline::mem1_sha1() {
  Sha1 h;
  h.update(m_.mem1(), rt::Machine::kMem1Size);
  return Sha1::hex(h.finish());
}

void Timeline::log(const std::string& s) {
  if (log_) log_(s);
  else std::printf("timeline: %s\n", s.c_str());
}

void Timeline::restart() {
  if (mods_.phase() != Phase::Live) throw std::logic_error("Timeline::restart: only in live play");
  for (rt::SaveState& s : slot_) s.valid = false;
  recorded_to_ = rt::g_frame_polls;  // polls before it were the other run's
  due_ = false;
  target_ = ~0ull;
  mods_.snapshot_taken();  // a request from the run before is met by the snapshot below
  save(0);
  publish_snapshots();
}

void Timeline::step() {
  if (!due_) return;
  due_ = false;
  const u64 poll = rt::g_frame_polls;
  switch (mods_.phase()) {
    case Phase::Live: {
      if (const Review* req = mods_.review_requested()) {
        const Review r = *req;
        u64 polls[2];
        slot_polls(polls);
        const int k = slot_for_review(polls, r.present_from);
        if (k < 0 || (recorded_to_ > kRing && polls[k] < recorded_to_ - kRing)) {
          // No snapshot before the replay, or it is older than the recorded inputs reach: no review.
          mods_.drop_review();
          log(k < 0 ? "review dropped: no snapshot before the replay"
                    : "review dropped: the snapshot is older than the recorded inputs");
          break;
        }
        resume_ = poll;
        live_sha1_ = mem1_sha1();
        mods_.begin_review();
        ++reviews;
        review_slot_ = k;
        load(k);
        const u64 p = rt::g_frame_polls;
        char b[200];
        std::snprintf(b, sizeof b, "review: back to poll %llu, replay %llu-%llu, continue at %llu",
                      static_cast<unsigned long long>(p), static_cast<unsigned long long>(r.present_from),
                      static_cast<unsigned long long>(r.present_to), static_cast<unsigned long long>(resume_));
        log(b);
        for (double& ms : leg_ms_) ms = 0;
        leg_t0_ = now_ms();
        if (r.present_from > p) {
          push_inputs(p, r.present_from);
          begin_leg(Phase::Seeking, r.present_from);
        } else {
          push_inputs(p, r.present_to);  // the slot holds the replay's first frame already
          begin_leg(Phase::Presenting, r.present_to);
        }
      } else if (mods_.snapshot_requested()) {
        u64 polls[2];
        slot_polls(polls);
        save(slot_to_replace(polls));
        mods_.snapshot_taken();
        publish_snapshots();
      }
      break;
    }
    case Phase::Seeking: {
      const Review& r = *mods_.review();
      save(review_slot_);  // the snapshot restored is not needed again
      publish_snapshots();
      push_inputs(r.present_from, r.present_to);
      begin_leg(Phase::Presenting, r.present_to);
      break;
    }
    case Phase::Presenting: {
      const Review& r = *mods_.review();
      load(review_slot_);
      push_inputs(r.present_from, resume_);
      begin_leg(Phase::Rerunning, resume_);
      break;
    }
    case Phase::Rerunning: {
      const std::string rerun = mem1_sha1();
      log("review: re-ran to poll " + std::to_string(poll) + ": MEM1 sha1 " + rerun +
          (rerun == live_sha1_ ? " (same as live)" : " (live was " + live_sha1_ + ")"));
      begin_leg(Phase::Live, ~0ull);
      char b[160];
      std::snprintf(b, sizeof b, "review: host time seek %.0f ms, replay %.0f ms, re-run %.0f ms",
                    leg_ms_[static_cast<int>(Phase::Seeking)], leg_ms_[static_cast<int>(Phase::Presenting)],
                    leg_ms_[static_cast<int>(Phase::Rerunning)]);
      log(b);
      mods_.end_review(m_.cpu());
      break;
    }
  }
}

}  // namespace mvp::mods
