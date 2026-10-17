// The engine side of gameplay mods (hook API v1, docs/mods/ABS_CHALLENGE.md §5):
// which mods run in a match, with which settings and seed; binding their sites;
// the services of mods::Host; and their state, for save states and the netplay
// state hash.
#pragma once

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "mods/gameplay/api.h"

namespace mvp::mods {

class GameplayHost final : public Host {
 public:
  GameplayHost();
  ~GameplayHost() override;
  GameplayHost(const GameplayHost&) = delete;
  GameplayHost& operator=(const GameplayHost&) = delete;

  // The mods this engine build ships (owned). Only enabled ones run.
  void add(std::unique_ptr<Mod> m);
  const std::vector<std::unique_ptr<Mod>>& available() const { return available_; }

  // Enables mod `id` with "key=value,key=value" settings (others at their
  // defaults). Unknown mods or keys and out-of-range values: false and `err`.
  bool enable(const std::string& id, const std::string& settings, std::string* err);
  bool any_enabled() const { return !active_.empty(); }

  // The match seed (netplay: the handshake's; offline: the recording's).
  void set_seed(u64 seed) { seed_ = seed; }
  u64 seed() const { return seed_; }

  // Binds every enabled mod's sites (runtime/mod_hooks.h), zeroes their state,
  // seeds their random streams and calls start(). False with `err` if two mods
  // want one site. `c` is the machine's CPU (its memory is what mods read).
  bool start(ppc::Cpu& c, std::string* err);
  // Unbinds every site.
  void stop();

  // At every frame poll, after its input was consumed (the engine's frame hook).
  void on_frame(ppc::Cpu& c, u64 frame);

  // "mod.<id>.<key>=<value>;...;mod.<id>=<version>" sorted, in the form of the
  // netplay handshake's settings string (net/identity.h). Empty with no mod enabled.
  std::string canonical_settings() const;

  // Every enabled mod's state block and random streams, in a fixed layout: what a
  // save state stores and the netplay state hash folds in.
  std::vector<u8> save_state() const;
  // Restores save_state()'s bytes (false if the layout differs).
  bool load_state(std::span<const u8> bytes);

  void set_log(std::function<void(const std::string&)> f) { log_ = std::move(f); }
  // Where notify() goes (the page's banner, mvp-play's output).
  void set_notify(std::function<void(const std::string&)> f) { notify_ = std::move(f); }
  // Where publish() goes (the page's mod UI; mvp-play prints it).
  void set_publish(std::function<void(const std::string&)> f) { publish_ = std::move(f); }

  // ---- For the timeline (mods/gameplay/timeline.h) ----
  bool snapshot_requested() const { return want_snapshot_; }
  void snapshot_taken() { want_snapshot_ = false; }
  // The polls of the snapshots the timeline keeps: the latest (snapshot_frame())
  // and the earliest (how far back a review can start). ~0 when none.
  void set_snapshots(u64 latest, u64 earliest) {
    snapshot_frame_ = latest;
    earliest_snapshot_ = earliest;
  }
  const Review* review_requested() const { return want_review_ ? &requested_ : nullptr; }
  // The timeline starts the requested review (it is then review()) or drops it.
  void begin_review() {
    review_ = requested_;
    active_mod_ = requested_mod_;
    in_review_ = true;
    want_review_ = false;
  }
  void drop_review() { want_review_ = false; }
  void set_phase(Phase p) { phase_ = p; }
  // The review re-ran to its end: the requesting mod's review_done(), then live.
  void end_review(ppc::Cpu& c);

  // ---- Host (valid during a mod's callbacks) ----
  u32 r8(u32 ea) override;
  u32 r16(u32 ea) override;
  u32 r32(u32 ea) override;
  void w8(u32 ea, u32 v) override;
  void w16(u32 ea, u32 v) override;
  void w32(u32 ea, u32 v) override;
  u32 rng(u32 stream) override;
  int setting(const char* key) const override;
  void* state() override;
  u64 frame() const override { return frame_; }
  const rt::FrameInput& input() const override;
  void log(const std::string& line) override;
  void notify(const std::string& text) override;
  void publish(const std::string& json) override;
  Phase phase() const override { return phase_; }
  void request_snapshot() override;
  u64 snapshot_frame() const override { return snapshot_frame_; }
  bool request_review(const Review& r) override;
  const Review* review() const override { return in_review_ ? &review_ : nullptr; }

 private:
  struct Active {
    Mod* mod;
    std::map<std::string, int> settings;
    std::vector<u8> state;
    std::array<u32, 16> rng{};  // 4 streams x xoshiro128** state
  };
  struct Binding {
    GameplayHost* host;
    std::size_t mod;  // index into active_
    u32 address;
  };
  static void on_site(ppc::Cpu& c, void* user);
  u8* span(u32 ea, u32 n, bool write);

  std::vector<std::unique_ptr<Mod>> available_;
  std::vector<Active> active_;
  std::vector<std::unique_ptr<Binding>> bindings_;
  Active* cur_ = nullptr;  // the mod whose callback runs
  ppc::Cpu* cpu_ = nullptr;
  u64 seed_ = 1, frame_ = 0;
  std::function<void(const std::string&)> log_, notify_, publish_;
  Phase phase_ = Phase::Live;
  bool want_snapshot_ = false, want_review_ = false, in_review_ = false;
  u64 snapshot_frame_ = ~0ull, earliest_snapshot_ = ~0ull;
  Review requested_{}, review_{};
  std::size_t requested_mod_ = 0, active_mod_ = 0;
};

// The mods this engine ships, ready to add() (mods/gameplay/abs.h, ...).
std::vector<std::unique_ptr<Mod>> builtin_mods();

}  // namespace mvp::mods
