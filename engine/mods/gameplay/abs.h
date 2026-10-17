// The ABS challenge mod (docs/mods/ABS_CHALLENGE.md): the post-pitch circle is
// hidden, the plate umpire misses some ball/strike calls (weighted toward the
// edges of the zone), and a challenged call is reviewed and upheld or overturned.
#pragma once

#include <memory>

#include "mods/gameplay/api.h"

namespace mvp::mods {

std::unique_ptr<Mod> make_abs_mod();

}  // namespace mvp::mods
