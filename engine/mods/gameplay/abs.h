// The ABS challenge mod (docs/mods/ABS_CHALLENGE.md): the post-pitch circle is
// hidden, the plate umpire misses some ball/strike calls (weighted toward the
// edges of the zone), and a challenged call is reviewed and upheld or overturned.
#pragma once

#include <memory>

#include "mods/gameplay/api.h"

namespace mvp::mods {

std::unique_ptr<Mod> make_abs_mod();

// A pitch against the zone (docs/mods/ABS_CHALLENGE.md §4.1): a strike when any part
// of the ball touches the zone, corners included. `x`, `y`: where the ball's centre
// crossed, in the game's zone units (|x|, |y| <= 1 is inside the zone); `half_w_in`,
// `half_h_in`: the zone's half-width and half-height in inches.
struct AbsPitch {
  bool strike = false;
  double off_in = 0;  // a ball: inches between the ball's edge and the zone (0 for a strike)
  double margin = 0;  // how far the ball's edge is from the zone's edge, either way, in zone units
};
AbsPitch abs_judge(double x, double y, double half_w_in, double half_h_in);
inline constexpr double kAbsBallRadiusIn = 1.45;  // a regulation ball, 9 to 9.25 in around

}  // namespace mvp::mods
