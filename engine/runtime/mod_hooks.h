// Gameplay mod hooks: the site registry of hook API v1 (docs/mods/ABS_CHALLENGE.md §5).
//
// A mod binds host code to retail addresses from a fixed registry. The translator
// stubs every registry site the way it stubs an HLE Before hook (runtime/hle.h):
// the handler runs right after the function's entry, before its first
// instruction, with the registers its caller set up. Unlike the SDK hooks, a mod
// handler may change guest registers and memory: that is what a gameplay mod is
// for. It stays deterministic because every peer runs the same mods with the
// same settings and seed (mods/host.h), and handlers read nothing else.
//
// The registry is part of the engine build: hle_table() appends one entry per
// site, so the sites are baked into the translated code like any HLE entry. A
// site with no handler bound costs one host call per call of its function.
// Sites are only ever added (an engine minor version); the address is what
// binds, the label is the decomp's placeholder name for reading convenience.
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "runtime/hle.h"

namespace mvp::rt::mod {

struct Site {
  u32 address;        // retail main.dol (GV4E69 rev 0)
  const char* label;  // placeholder name in config/GV4E69/symbols.txt
  const char* why;    // what a mod uses the site for
};

// The registry, in a fixed order (the order of their hle_table() entries).
std::span<const Site> sites();
// Index of the site at `address`, or -1.
int site_index(u32 address);

// A handler: called at the site with the guest's CPU, and the pointer given to bind().
using Handler = void (*)(Cpu& c, void* user);

// Binds `h` at the registry site `address`. One handler per site: binding a
// second one, or binding an address outside the registry, throws std::logic_error.
void bind(u32 address, Handler h, void* user);
void unbind_all();

// hle.cpp: the HLE function of site `k` (a Before hook that calls the bound handler).
HleFn site_fn(std::size_t k);

}  // namespace mvp::rt::mod
