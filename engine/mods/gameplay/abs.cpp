#include "mods/gameplay/abs.h"

#include <cstdio>
#include <cstring>

namespace mvp::mods {

namespace {

// Retail sites (runtime/mod_hooks.h; evidence in docs/mods/ABS_CHALLENGE.md §3).
constexpr u32 kUmpireCall = 0x801BB338u;  // entered with the call in r5 and at the caller's r1+8
constexpr u32 kCircleDraw = 0x802F893Cu;  // r3 = HUD; draws the circle while hud+0x90 != 0
constexpr u32 kNewPitch = 0x801BAFA4u;    // umpire reset for the next pitch
constexpr u32 kRelease = 0x801896E8u;     // posts event 0x3D (release)
constexpr u32 kWindup = 0x802321BCu;      // the pitch is chosen and the delivery starts (posts event 0x5A)
constexpr u32 kNewGame = 0x801BACA0u;     // the umpire's set-up for a game, also on Restart Game (§3.10)
constexpr u32 kSites[] = {kUmpireCall, kCircleDraw, kNewPitch, kRelease, kWindup, kNewGame};

// The late snapshot (docs/mods/ABS_CHALLENGE.md §7.2): a review rewinds to a
// snapshot just before its replay's first frame (release - pre), so the headless
// run to it is short. The delivery, windup to release, repeats pitch after pitch
// over long runs (T: 143-145, 135-137, 74-75 and 62-63 frame polls; §3.10), so it
// is learned per fielding team from the team's last pitch. kDefaultDelivery is
// the shortest seen, for a team's first pitch: a longer delivery only lengthens
// the rewind. kLateMargin covers the jitter of one or two polls. A delivery
// shorter than learned drops the late snapshot, and the review rewinds from the
// pitch's first one.
constexpr u32 kDefaultDelivery = 62, kLateMargin = 3;

// Guest layout (§3.1-§3.6). Umpire A: +0x10 play coordinator T, +0x14 game context
// P. T+0x3B0 the pitch, whose +0x4 points at the plate crossing (+0x28 x, +0x2C y,
// zone-normalized: |x|,|y| <= 1 is a strike); T+0x3A4 + 0x28C the swing state (+0x4:
// 1, 2, 4, 6, 7 = swung). P+0x6F8 game state G: +0x10 options (+0x6C + 4 * team:
// the team's controller port, -1 for the CPU; team 0 home, 1 away), +0x14 the count
// block (+0x1CD0 the batting team: 1 away, 0 home). 0x8061B884: the logic ticks
// (1/60 s) the frame loop ran this frame.
constexpr u32 kAPlay = 0x10, kAGame = 0x14;
constexpr u32 kTPitch = 0x3B0, kPitchLoc = 0x4, kLocX = 0x28, kLocY = 0x2C;
constexpr u32 kTBatter = 0x3A4, kBatterSwing = 0x28C;
constexpr u32 kPState = 0x6F8, kGOptions = 0x10, kGCount = 0x14;
constexpr u32 kOptPort = 0x6C, kCountBatting = 0x1CD0;
constexpr u32 kFrameTicks = 0x8061B884u;
constexpr u32 kHudCircleShown = 0x90;

// The challenge on a standard controller, native prototype (§7.6): L, R and Z
// pressed together. Netplay gets an engine button that never reaches the guest.
constexpr u16 kChallengeCombo = rt::pad::kL | rt::pad::kR | rt::pad::kZ;

constexpr Setting kSettings[] = {
    {"hide", 0, 1, 1, "hide the post-pitch circle"},
    {"miss", 0, 30, 8, "umpire misses, percent of called pitches"},
    {"edge", 0, 4, 3, "0: misses anywhere; 1-4: more and more concentrated at the zone's edges"},
    {"umpire", 0, 1, 0, "0: the engine's miss model; 1: the game's own variable zone (miss and edge unused)"},
    {"challenges", 0, 5, 2, "challenges per team, kept when the call is overturned"},
    {"window", 30, 180, 90, "time to challenge after the call, in 1/60 s of game time"},
    {"pre", 0, 60, 15, "replay: frame polls shown before the release"},
    {"post", 15, 120, 45, "replay: frame polls shown after the call"},
};

// Edge weighting: w(d) = exp(-|d| / sigma) over |d| in 1/256 units of the zone's
// half-size, built by repeated multiplication with exp(-1/(256 sigma)) in Q32 so
// every build computes the same table. sigma = 0.5, 0.25, 0.12, 0.06 for edge 1-4.
constexpr u32 kDecayQ32[5] = {0, 4261543595u, 4228380000u, 4157408227u, 4024254894u};
// Mean of w over called pitches for each edge level, Q16: 96 called pitches of
// traced games (84 CPU vs CPU, 12 with a batter who never swings), 2026-10-17
// (docs/mods/ABS_CHALLENGE.md §4.1). Over that sample the miss rate then equals
// the setting, except where P_miss saturates at 1 (edge 4 above about 10 %).
constexpr u32 kMeanWQ16[5] = {65536, 38421, 26156, 15363, 8729};

struct State {
  u32 started;
  u32 called, misses, swings, game_calls;  // statistics
  u32 challenges_made, upheld, overturned;
  u32 last_frame, release_frame;
  u32 last_x, last_y;  // raw float bits of the last called pitch's location
  u32 umpire;          // the umpire object (A) of the last new pitch or call
  u32 window_ticks;    // game ticks left to challenge the last call
  u32 pitch_frame;     // the frame poll of this pitch's first snapshot (new pitch)
  u32 windup_frame;    // this pitch's delivery start (kWindup), 0 before it
  u32 late_due;        // the frame at which to ask for the late snapshot, 0: none pending
  u32 delivery[2];     // windup to release in frames, learned per fielding team; 0: not yet
  u16 prev_buttons[4];
  u8 challenges[2];    // per team (0 home, 1 away)
  u8 window_open, last_truth, last_call, last_miss;
  u8 pitch_team;       // this pitch's fielding team (2: unknown)
  u8 late_asked;       // this pitch's late snapshot is scheduled (one per pitch)
};
static_assert(sizeof(State) <= 256);

bool swung(u32 st) { return st == 1 || st == 2 || st == 4 || st == 6 || st == 7; }

float bits_to_float(u32 b) {
  float f;
  std::memcpy(&f, &b, 4);
  return f;
}

const char* phase_tag(Phase p) {
  switch (p) {
    case Phase::Seeking: return "[seek] ";
    case Phase::Presenting: return "[replay] ";
    case Phase::Rerunning: return "[rerun] ";
    default: return "";
  }
}

class AbsMod final : public Mod {
 public:
  AbsMod() { info_ = ModInfo{"abs", "ABS challenge", 1, kSites, kSettings, sizeof(State)}; }
  const ModInfo& info() const override { return info_; }

  void start(Host& h) override {
    State& s = h.state_as<State>();
    s.started = 1;
    s.challenges[0] = s.challenges[1] = static_cast<u8>(h.setting("challenges"));
    // The weight table: derived from settings only, the same on every machine.
    const int edge = h.setting("edge");
    u64 w = 0xFFFFFFFFull;
    for (int b = 0; b < 256; ++b) {
      weight_[b] = static_cast<u32>(w >> 16);  // Q16
      if (edge) w = (w * kDecayQ32[edge]) >> 32;
    }
    const u64 rate = static_cast<u64>(h.setting("miss")) * 65536u / 100u;  // Q16
    k_q16_ = static_cast<u32>(rate * 65536u / kMeanWQ16[edge]);
  }

  void site(Host& h, ppc::Cpu& c, u32 address) override {
    State& s = h.state_as<State>();
    switch (address) {
      case kCircleDraw:
        // Hidden in play and in the re-runs (they must match play); shown in the replay.
        if (h.setting("hide") && h.phase() != Phase::Presenting) h.w32(c.gpr[3] + kHudCircleShown, 0);
        break;
      case kUmpireCall:
        umpire_call(h, c);
        break;
      case kNewPitch:
        s.window_open = 0;
        s.umpire = c.gpr[3];  // the umpire object the call is made on (T)
        s.pitch_frame = static_cast<u32>(h.frame()) + 1;
        s.windup_frame = s.late_due = 0;
        s.late_asked = 0;
        s.pitch_team = 2;
        h.request_snapshot();  // the rewind point for a challenge of this pitch (Live only)
        break;
      case kWindup:
        windup(h);
        break;
      case kNewGame:
        new_game(h);
        break;
      case kRelease:
        s.release_frame = static_cast<u32>(h.frame());
        s.late_due = 0;  // too late to help a review, and it would stall the pitch in flight
        if (s.windup_frame && s.pitch_team < 2) s.delivery[s.pitch_team] = s.release_frame - s.windup_frame;
        break;
      default:
        break;
    }
  }

  void frame(Host& h, ppc::Cpu&) override {
    State& s = h.state_as<State>();
    if (s.late_due && h.frame() >= s.late_due) {
      s.late_due = 0;
      h.request_snapshot();  // taken at the next frame poll (Live only)
    }
    if (s.window_open) {
      const u32 ticks = h.r32(kFrameTicks);
      if (ticks >= s.window_ticks) s.window_open = 0;
      else s.window_ticks -= ticks;
    }
    const rt::FrameInput& in = h.input();
    for (int k = 0; k < rt::kPorts; ++k) {
      const u16 b = in.pad[k].connected ? in.pad[k].buttons : 0;
      const bool pressed = (b & kChallengeCombo) == kChallengeCombo &&
                           (s.prev_buttons[k] & kChallengeCombo) != kChallengeCombo;
      s.prev_buttons[k] = b;
      if (pressed && s.window_open && h.phase() == Phase::Live) challenge(h, k);
    }
  }

  void review_done(Host& h, ppc::Cpu&, const Review& r) override {
    State& s = h.state_as<State>();
    const u8 verdict = r.payload[0], original = r.payload[1], team = r.payload[2];
    s.window_open = 0;
    ++s.challenges_made;
    char line[160], note[96];
    if (verdict == original) {
      if (s.challenges[team]) --s.challenges[team];
      ++s.upheld;
      std::snprintf(line, sizeof line, "challenge by %s: the call stands (%s); %u challenge(s) left",
                    team ? "away" : "home", original ? "strike" : "ball", s.challenges[team]);
      std::snprintf(note, sizeof note, "The call stands: %s. %s: %u challenge%s left", original ? "strike" : "ball",
                    team ? "Away" : "Home", s.challenges[team], s.challenges[team] == 1 ? "" : "s");
    } else {
      ++s.overturned;
      std::snprintf(line, sizeof line, "challenge by %s: overturned, %s; %u challenge(s) left",
                    team ? "away" : "home", verdict ? "strike" : "ball", s.challenges[team]);
      std::snprintf(note, sizeof note, "Overturned: %s. %s keeps the challenge (%u left)", verdict ? "strike" : "ball",
                    team ? "Away" : "Home", s.challenges[team]);
    }
    h.log(line);
    h.notify(note);
    char ui[160];
    std::snprintf(ui, sizeof ui, "{\"abs\":\"verdict\",\"team\":\"%s\",\"overturned\":%s,\"call\":\"%s\",\"left\":%u}",
                  team ? "away" : "home", verdict == original ? "false" : "true", verdict ? "strike" : "ball",
                  s.challenges[team]);
    h.publish(ui);
  }

 private:
  // A game starts (from the menus, or the pause menu's Restart Game): both teams get
  // their challenges again, and nothing of the last game's pitches carries over. The
  // statistics stay (they count the whole session).
  void new_game(Host& h) {
    State& s = h.state_as<State>();
    s.challenges[0] = s.challenges[1] = static_cast<u8>(h.setting("challenges"));
    s.window_open = 0;
    s.windup_frame = s.late_due = 0;
    s.late_asked = 0;
    s.delivery[0] = s.delivery[1] = 0;
    s.pitch_team = 2;
    char line[80];
    std::snprintf(line, sizeof line, "new game: %u challenge(s) per team", s.challenges[0]);
    h.log(line);
  }

  // The delivery starts: schedule the late snapshot to land kLateMargin polls
  // before the replay's first frame (release - pre), with the delivery learned
  // for this fielding team. A snapshot asked for at frame F is taken at poll F + 1.
  void windup(Host& h) {
    State& s = h.state_as<State>();
    const u32 frame = static_cast<u32>(h.frame());
    s.windup_frame = frame;
    if (s.late_asked) return;
    s.late_asked = 1;
    const int team = fielding_team(h, s.umpire);
    s.pitch_team = static_cast<u8>(team < 0 ? 2 : team);
    const u32 delivery = team >= 0 && s.delivery[team] ? s.delivery[team] : kDefaultDelivery;
    const u32 before = static_cast<u32>(h.setting("pre")) + 1 + kLateMargin;
    if (delivery > before) {
      s.late_due = frame + delivery - before;
    } else {
      h.request_snapshot();  // the replay starts before the delivery is under way
    }
  }

  // The fielding team (0 home, 1 away) from umpire A, or -1 while the chain is
  // not set up (§3.5): A+0x14 -> P, P+0x6F8 -> G, G+0x14 -> count, +0x1CD0 batting.
  static int fielding_team(Host& h, u32 a) {
    auto ram = [](u32 p) { return p >= 0x80000000u && p < 0x81800000u; };
    if (!ram(a)) return -1;
    const u32 p = h.r32(a + kAGame);
    if (!ram(p)) return -1;
    const u32 g = h.r32(p + kPState);
    if (!ram(g)) return -1;
    const u32 count = h.r32(g + kGCount);
    if (!ram(count)) return -1;
    return h.r32(count + kCountBatting) ? 0 : 1;  // away (1) batting: home (0) fields
  }

  void umpire_call(Host& h, ppc::Cpu& c) {
    State& s = h.state_as<State>();
    const u32 a = c.gpr[3];
    const u32 play = h.r32(a + kAPlay);
    const u32 loc = h.r32(h.r32(play + kTPitch) + kPitchLoc);
    const u32 xb = h.r32(loc + kLocX), yb = h.r32(loc + kLocY);
    const u32 swing = h.r32(h.r32(play + kTBatter) + kBatterSwing + 4);
    const u32 game_call = c.gpr[5] ? 1 : 0;
    if (swung(swing)) {
      ++s.swings;
      return;  // a swing is a strike whatever the umpire thinks
    }
    // Exact in double: |x| and 1.0 are floats, their difference and its x256 are exact.
    const double ax = bits_to_float(xb & 0x7FFFFFFFu), ay = bits_to_float(yb & 0x7FFFFFFFu);
    const u32 truth = (ax <= 1.0 && ay <= 1.0) ? 1 : 0;
    double d = (ax > ay ? ax : ay) - 1.0;
    if (d < 0) d = -d;
    const double scaled = d * 256.0;
    const int bucket = scaled >= 255.0 ? 255 : static_cast<int>(scaled);
    u32 call = truth, miss = 0;
    if (h.setting("umpire") == 1) {
      call = game_call;
      ++s.game_calls;
    } else {
      const u64 p = static_cast<u64>(k_q16_) * weight_[bucket];  // Q32
      const u32 draw = h.rng(0);
      call = truth ^ (p > 0xFFFFFFFFull || draw < p ? 1u : 0u);
    }
    const Review* r = h.review();
    if (r && h.phase() == Phase::Rerunning) call = r->payload[0];  // the review's verdict
    miss = call != truth;
    c.gpr[5] = call;
    h.w32(c.gpr[1] + 8, call);  // fn_801BB1E0's local, read again after the swing test
    ++s.called;
    s.misses += miss;
    s.last_frame = static_cast<u32>(h.frame());
    s.last_x = xb;
    s.last_y = yb;
    s.last_truth = static_cast<u8>(truth);
    s.last_call = static_cast<u8>(call);
    s.last_miss = static_cast<u8>(miss);
    s.umpire = a;
    s.window_open = 1;
    s.window_ticks = static_cast<u32>(h.setting("window"));
    char line[200];
    std::snprintf(line, sizeof line, "%sframe %u called %s (truth %s, game %s) x %.4f y %.4f d %.4f%s", phase_tag(h.phase()),
                  s.last_frame, call ? "strike" : "ball", truth ? "strike" : "ball", game_call ? "strike" : "ball",
                  static_cast<double>(bits_to_float(xb)), static_cast<double>(bits_to_float(yb)), d,
                  miss ? " MISSED" : "");
    h.log(line);
  }

  void challenge(Host& h, int port) {
    State& s = h.state_as<State>();
    const u32 game = h.r32(h.r32(s.umpire + kAGame) + kPState);
    const u32 options = h.r32(game + kGOptions);
    const u32 batting = h.r32(h.r32(game + kGCount) + kCountBatting) ? 1u : 0u;  // team index: 1 away, 0 home
    const u32 team = s.last_call ? batting : 1u - batting;  // strike: the batting team; ball: the fielding team
    const int team_port = static_cast<int>(h.r32(options + kOptPort + 4 * team));
    if (team_port != port) return;  // not the team allowed to challenge this call
    char line[200];
    if (!s.challenges[team]) {
      std::snprintf(line, sizeof line, "challenge by %s refused: no challenges left", team ? "away" : "home");
      h.log(line);
      h.notify(team ? "Away has no challenges left" : "Home has no challenges left");
      return;
    }
    Review r;
    const u64 snap = s.pitch_frame;  // the pitch's first snapshot; the host picks the nearest one kept
    const u64 pre = static_cast<u64>(h.setting("pre"));
    const u64 from = s.release_frame >= snap + pre ? s.release_frame - pre : snap;
    r.present_from = from;
    r.present_to = s.last_frame + static_cast<u64>(h.setting("post"));
    r.payload[0] = s.last_truth;  // the verdict: what the pitch was
    r.payload[1] = s.last_call;   // what the umpire called
    r.payload[2] = static_cast<u8>(team);
    if (!h.request_review(r)) {
      h.log("challenge: the review cannot start (no snapshot of this pitch)");
      return;
    }
    s.window_open = 0;
    std::snprintf(line, sizeof line, "challenge by %s (port %d) of a called %s: reviewing", team ? "away" : "home",
                  port + 1, s.last_call ? "strike" : "ball");
    h.log(line);
    // The page's challenge scene (mods-panel.ts): who, which call, and (shown after the
    // replay) where the pitch crossed, zone-normalized (|x|, |y| <= 1 is a strike).
    char ui[200];
    std::snprintf(ui, sizeof ui,
                  "{\"abs\":\"challenge\",\"team\":\"%s\",\"call\":\"%s\",\"truth\":\"%s\",\"x\":%.3f,\"y\":%.3f}",
                  team ? "away" : "home", s.last_call ? "strike" : "ball", s.last_truth ? "strike" : "ball",
                  static_cast<double>(bits_to_float(s.last_x)), static_cast<double>(bits_to_float(s.last_y)));
    h.publish(ui);
    h.notify(s.last_call ? "ABS challenge: reviewing the strike call" : "ABS challenge: reviewing the ball call");
  }

  ModInfo info_;
  // Derived from settings at start (the same on every machine), not state.
  u32 weight_[256] = {};
  u32 k_q16_ = 0;
};

}  // namespace

std::unique_ptr<Mod> make_abs_mod() { return std::make_unique<AbsMod>(); }

}  // namespace mvp::mods
