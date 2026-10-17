#include "mods/gameplay/abs.h"
#include "mods/gameplay/host.h"

namespace mvp::mods {

std::vector<std::unique_ptr<Mod>> builtin_mods() {
  std::vector<std::unique_ptr<Mod>> v;
  v.push_back(make_abs_mod());
  return v;
}

}  // namespace mvp::mods
