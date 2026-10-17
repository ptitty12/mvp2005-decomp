#include "runtime/mod_hooks.h"

#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>

namespace mvp::rt::mod {

namespace {

// Hook API v1 sites. Evidence for each one: docs/mods/ABS_CHALLENGE.md §3.
constexpr Site kSites[] = {
    {0x801BB338u, "fn_801BB338",
     "plate umpire: entered with the ball/strike call in r5 and in the caller's slot at r1+8 (ABS)"},
    {0x802F893Cu, "fn_802F893C", "HUD: draws the post-pitch circle while hud+0x90 is set (ABS)"},
    {0x801BAFA4u, "fn_801BAFA4", "plate umpire: reset for the next pitch (ABS)"},
    {0x801896E8u, "fn_801896E8", "pitch: release, posts event 0x3D (ABS)"},
    {0x802321BCu, "fn_802321BC", "pitch: chosen, the delivery starts; posts event 0x5A (ABS)"},
    {0x801BACA0u, "fn_801BACA0", "plate umpire: set up for a new game (also Restart Game); rolls the variable zone (ABS)"},
};
constexpr std::size_t kCount = sizeof kSites / sizeof kSites[0];

std::array<std::pair<Handler, void*>, kCount> g_bound{};

template <std::size_t K>
void site_hook(Cpu& c, GuestFn) {
  if (auto [h, user] = g_bound[K]; h) h(c, user);
}

template <std::size_t... K>
constexpr std::array<HleFn, kCount> make_fns(std::index_sequence<K...>) {
  return {&site_hook<K>...};
}
constexpr std::array<HleFn, kCount> kFns = make_fns(std::make_index_sequence<kCount>{});

}  // namespace

std::span<const Site> sites() { return kSites; }

int site_index(u32 address) {
  for (std::size_t k = 0; k < kCount; ++k)
    if (kSites[k].address == address) return static_cast<int>(k);
  return -1;
}

void bind(u32 address, Handler h, void* user) {
  int k = site_index(address);
  if (k < 0) {
    char a[16];
    std::snprintf(a, sizeof a, "0x%08X", static_cast<unsigned>(address));
    throw std::logic_error(std::string("mod hook: ") + a + " is not a registry site");
  }
  auto& slot = g_bound[static_cast<std::size_t>(k)];
  if (slot.first) throw std::logic_error(std::string("mod hook: ") + kSites[k].label + " is already bound");
  slot = {h, user};
}

void unbind_all() { g_bound.fill({}); }

HleFn site_fn(std::size_t k) { return kFns.at(k); }

}  // namespace mvp::rt::mod
