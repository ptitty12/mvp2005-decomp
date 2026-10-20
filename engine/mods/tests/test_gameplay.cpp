// Data-free tests for gameplay mods (hook API v1, docs/mods/ABS_CHALLENGE.md §5)
// and save states (§6): settings and their canonical handshake form, random
// streams, mod state, the site registry, and a machine's save/restore round trip.
#include <doctest/doctest.h>

#include <cstring>
#include <stdexcept>

#include "mods/gameplay/abs.h"
#include "mods/gameplay/host.h"
#include "mods/gameplay/timeline.h"
#include "runtime/hle.h"
#include "runtime/mod_hooks.h"
#include "runtime/save_state.h"

using namespace mvp;
using namespace mvp::mods;

namespace {

constexpr u32 kTestSites[] = {0x802F893Cu};
constexpr Setting kTestSettings[] = {
    {"rate", 0, 30, 8, "a percentage"},
    {"flag", 0, 1, 1, "a switch"},
};
struct TestState {
  u32 starts, frames, last_draw;
};

class TestMod final : public Mod {
 public:
  explicit TestMod(const char* id = "test") {
    info_ = ModInfo{id, "Test", 3, kTestSites, kTestSettings, sizeof(TestState)};
  }
  const ModInfo& info() const override { return info_; }
  void start(Host& h) override { ++h.state_as<TestState>().starts; }
  void site(Host&, ppc::Cpu&, u32) override {}
  void frame(Host& h, ppc::Cpu&) override {
    auto& s = h.state_as<TestState>();
    ++s.frames;
    s.last_draw = h.rng(0);
  }

 private:
  ModInfo info_;
};

// Asks for the review in `want` at its next frame and keeps the host's answer.
class ReviewMod final : public Mod {
 public:
  ReviewMod() { info_ = ModInfo{"review", "Review", 1, {}, {}, 4}; }
  const ModInfo& info() const override { return info_; }
  void start(Host&) override {}
  void site(Host&, ppc::Cpu&, u32) override {}
  void frame(Host& h, ppc::Cpu&) override { accepted = h.request_review(want); }
  Review want;
  bool accepted = false;

 private:
  ModInfo info_;
};

}  // namespace

TEST_CASE("ABS truth: a strike when any part of the ball touches the zone") {
  const double w = 9.0, h = 10.2;  // the zone's half sizes in inches (a 69 in batter)
  // The centre in the zone.
  CHECK(abs_judge(0.2, -0.5, w, h).strike);
  CHECK(abs_judge(0.2, -0.5, w, h).off_in == 0.0);
  // Beside the zone: the edge of a 1.45 in ball reaches 1.45 in past it.
  CHECK(abs_judge(1.0 + 1.40 / w, 0.0, w, h).strike);
  const AbsPitch wide = abs_judge(-(1.0 + 1.60 / w), 0.3, w, h);
  CHECK(!wide.strike);
  CHECK(wide.off_in == doctest::Approx(0.15));
  // Low: inches along y scale with the batter's half-height.
  const AbsPitch low = abs_judge(0.1, -1.3, w, h);
  CHECK(!low.strike);
  CHECK(low.off_in == doctest::Approx(0.3 * h - 1.45));
  // The same zone units are more inches for a taller batter: 1.38 in low, then 1.51 in.
  CHECK(abs_judge(0.1, -1.135, w, h).strike);
  CHECK(!abs_judge(0.1, -1.135, w, 1.1 * h).strike);
  // A corner: within one radius of the corner itself, not of the two edges' lines.
  CHECK(abs_judge(1.1, 1.1, w, h).strike);    // 0.90 and 1.02 in out: 1.36 in from the corner
  CHECK(!abs_judge(1.12, 1.1, w, h).strike);  // 1.08 and 1.02 in out: 1.49 in
  // The pitch of the first ring-up state (x -1.0443, y 0.8242): 0.40 in past the edge, so
  // it touches the zone; the centre rule had called it a ball.
  CHECK(abs_judge(-1.0443, 0.8242, w, h).strike);
  // The margin (the umpire model's edge weighting) is zero where the ball's edge meets the zone.
  CHECK(abs_judge(1.0 + 1.45 / w, 0.0, w, h).margin == doctest::Approx(0.0).epsilon(1e-9));
  CHECK(abs_judge(0.0, 0.0, w, h).margin == doctest::Approx((w + 1.45) / w));
}

TEST_CASE("settings: defaults, overrides, ranges and the canonical handshake form") {
  GameplayHost h;
  h.add(std::make_unique<TestMod>());
  std::string err;
  CHECK_FALSE(h.enable("nope", "", &err));
  CHECK(err.find("no gameplay mod") != std::string::npos);
  CHECK_FALSE(h.enable("test", "rate=31", &err));
  CHECK_FALSE(h.enable("test", "speed=1", &err));
  CHECK_FALSE(h.enable("test", "rate", &err));
  CHECK_FALSE(h.enable("test", "rate=x", &err));
  CHECK_FALSE(h.any_enabled());
  REQUIRE(h.enable("test", "rate=20", &err));
  CHECK_FALSE(h.enable("test", "", &err));  // twice
  CHECK(h.canonical_settings() == "mod.test.flag=1;mod.test.rate=20;mod.test=3");
}

TEST_CASE("random streams: xoshiro128** seeded from the match seed, mod id and stream") {
  // Vectors from an independent implementation (SHA-256 seeding, xoshiro128**).
  ppc::Cpu cpu;
  auto first_draws = [&](u64 seed) {
    GameplayHost h;
    h.add(std::make_unique<TestMod>());
    std::string err;
    REQUIRE(h.enable("test", "", &err));
    h.set_seed(seed);
    REQUIRE(h.start(cpu, &err));
    std::vector<u32> v;
    for (int k = 0; k < 3; ++k) {
      h.on_frame(cpu, static_cast<u64>(k));
      TestState s;
      auto bytes = h.save_state();
      REQUIRE(bytes.size() == 64 + sizeof(TestState));
      std::memcpy(&s, bytes.data() + 64, sizeof s);
      v.push_back(s.last_draw);
    }
    h.stop();
    return v;
  };
  CHECK(first_draws(1) == std::vector<u32>{0x55158a8fu, 0x1135c77bu, 0x3f3146deu});
  CHECK(first_draws(2) == std::vector<u32>{0xa88a3bf3u, 0x1f702b85u, 0x5faaff30u});
}

TEST_CASE("mod state and streams round-trip through save_state/load_state") {
  ppc::Cpu cpu;
  GameplayHost h;
  h.add(std::make_unique<TestMod>());
  std::string err;
  REQUIRE(h.enable("test", "", &err));
  REQUIRE(h.start(cpu, &err));
  for (int k = 0; k < 5; ++k) h.on_frame(cpu, static_cast<u64>(k));
  const auto saved = h.save_state();
  for (int k = 5; k < 9; ++k) h.on_frame(cpu, static_cast<u64>(k));
  const auto later = h.save_state();
  CHECK(later != saved);
  REQUIRE(h.load_state(saved));
  for (int k = 5; k < 9; ++k) h.on_frame(cpu, static_cast<u64>(k));
  CHECK(h.save_state() == later);  // the same draws again
  CHECK_FALSE(h.load_state(std::vector<u8>(3)));
  h.stop();
}

TEST_CASE("the site registry: one handler per site, registry addresses only") {
  CHECK(rt::mod::site_index(0x801BB338u) >= 0);
  CHECK(rt::mod::site_index(0x80000000u) < 0);
  auto noop = [](ppc::Cpu&, void*) {};
  CHECK_THROWS_AS(rt::mod::bind(0x80000000u, noop, nullptr), std::logic_error);
  rt::mod::bind(0x802F893Cu, noop, nullptr);
  CHECK_THROWS_AS(rt::mod::bind(0x802F893Cu, noop, nullptr), std::logic_error);
  rt::mod::unbind_all();
  // Every site is a Before hook in the HLE table, at its address.
  for (const auto& s : rt::mod::sites()) {
    bool found = false;
    for (const auto& e : rt::hle_table())
      if (e.address == s.address && e.kind == rt::HleKind::Before) found = true;
    CHECK(found);
  }
  // Two mods wanting one site cannot start together.
  ppc::Cpu cpu;
  GameplayHost h;
  h.add(std::make_unique<TestMod>("one"));
  h.add(std::make_unique<TestMod>("two"));
  std::string err;
  REQUIRE(h.enable("one", "", &err));
  REQUIRE(h.enable("two", "", &err));
  CHECK_FALSE(h.start(cpu, &err));
  CHECK(err.find("already bound") != std::string::npos);
}

TEST_CASE("timeline slots: a live snapshot replaces the older; a review restores the latest that fits") {
  const u64 none = ~0ull;
  {
    const u64 p[2] = {none, none};
    CHECK(Timeline::slot_to_replace(p) == 0);
    CHECK(Timeline::slot_for_review(p, 100) == -1);
  }
  {
    const u64 p[2] = {600, none};  // the pitch's first snapshot only
    CHECK(Timeline::slot_to_replace(p) == 1);
    CHECK(Timeline::slot_for_review(p, 950) == 0);
    CHECK(Timeline::slot_for_review(p, 599) == -1);
  }
  {
    const u64 p[2] = {600, 940};  // and the late one, near the release
    CHECK(Timeline::slot_to_replace(p) == 0);
    CHECK(Timeline::slot_for_review(p, 943) == 1);  // a short rewind
    CHECK(Timeline::slot_for_review(p, 940) == 1);  // none: the snapshot is the replay's first frame
    CHECK(Timeline::slot_for_review(p, 939) == 0);  // the late one came too late: the long rewind
  }
  {
    const u64 p[2] = {1400, 940};  // the next pitch's first snapshot replaced the older
    CHECK(Timeline::slot_to_replace(p) == 1);
    CHECK(Timeline::slot_for_review(p, 1600) == 0);
  }
}

TEST_CASE("reviews: accepted from the earliest snapshot kept up to the current frame") {
  ppc::Cpu cpu;
  GameplayHost h;
  h.add(std::make_unique<ReviewMod>());
  auto* mod = static_cast<ReviewMod*>(h.available().back().get());
  std::string err;
  REQUIRE(h.enable("review", "", &err));
  REQUIRE(h.start(cpu, &err));
  auto ask = [&](u64 from, u64 frame) {
    mod->want.present_from = from;
    mod->want.present_to = from + 10;
    h.on_frame(cpu, frame);
    if (mod->accepted) h.drop_review();
    return mod->accepted;
  };
  CHECK_FALSE(ask(100, 200));  // no snapshot yet
  h.set_snapshots(940, 600);
  CHECK(h.snapshot_frame() == 940);
  CHECK(ask(943, 1000));
  CHECK(ask(700, 1000));        // before the late snapshot: from the pitch's first one
  CHECK_FALSE(ask(599, 1000));  // before both
  CHECK_FALSE(ask(1001, 1000)); // not played yet
  h.stop();
}

TEST_CASE("save states: a machine round-trips (CPU, MEM1, devices, clock, input)") {
  rt::Machine m;
  rt::Hardware hw(&m.cpu(), m.mem1(), rt::Machine::kMem1Size);
  rt::YieldingHooks hooks(&m, &hw);
  m.cpu().hooks = &hooks;
  rt::InputQueue input;
  m.cpu().gpr[3] = 0x1234;
  m.cpu().pc = 0x8014865Cu;
  m.cpu().ticks = 1000;
  m.mem1()[0x100] = 0xAB;
  input.push(rt::neutral_input(1));
  input.begin_frame();
  rt::g_frame_polls = 7;
  rt::SaveState st;
  rt::save_state(st, m, hooks, hw, &input);
  hw.write(0xCC003004u, 0x5A5A, 4);  // PI interrupt mask
  m.cpu().gpr[3] = 0;
  m.cpu().ticks = 5000;
  m.mem1()[0x100] = 0;
  input.push(rt::neutral_input(3));
  input.begin_frame();
  rt::g_frame_polls = 9;
  rt::load_state(st, m, hooks, hw, &input);
  CHECK(m.cpu().gpr[3] == 0x1234);
  CHECK(m.cpu().ticks == 1000);
  CHECK(m.cpu().hooks == &hooks);  // host pointers stay
  CHECK(m.mem1()[0x100] == 0xAB);
  CHECK(hw.pi_mask() == 0);
  CHECK(input.frames() == 1);
  CHECK(rt::g_frame_polls == 7);
  rt::g_frame_polls = 0;
  // Not inside a run.
  rt::g_unwinding = true;
  CHECK_THROWS_AS(rt::save_state(st, m, hooks, hw, &input), std::logic_error);
  rt::g_unwinding = false;
}
