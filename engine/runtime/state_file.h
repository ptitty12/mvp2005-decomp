// Save-state files: a save state (runtime/save_state.h) as bytes, for another
// process to load: a later mvp-play, or the browser engine. Native and wasm32
// engines read each other's files (common/state_io.h).
//
// Loading one: boot the same disc, with the same roster packs and gameplay
// mods, run to the first stopped frame poll, then read_state_file(),
// prepare_machine() and rt::load_state(). The guest then continues exactly as
// the saving run did from that poll, given the same inputs.
//
// Layout (little-endian): "MVPSTATE", u32 version, u64 length + the header (a
// JSON object, below), then the parts in a fixed order: the CPU, MEM1, the
// other RAM regions, the virtual clock, the yield state, the hardware
// (Hardware::write_state), the input queue, the frame-poll count, the gameplay
// mods' state block and the renderer's GX state (both may be empty), "END!".
//
// The header is the tool's: what a loader must set up before boot and what the
// state shows. mvp-play and the browser write
//   {"game": "GV4E69-0", "frame_polls": N, "vi_fields": N,
//    "mods": "abs:miss=8,..." (the enabled mods, as the --mod / mod= spec),
//    "packs": {"set_hash": ..., "result_hash": ..., "ids": [...]},
//    "label": "..."}
// A state is game-derived (MEM1 and ARAM hold the disc's data): it stays on the
// machine that made it and is never committed; the one exception is the owner's
// own states on the owner's server, for the owner alone (ARCHITECTURE §3).
#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "runtime/save_state.h"

namespace mvp::rt {

inline constexpr std::uint32_t kStateFileVersion = 1;

std::vector<u8> write_state_file(const std::string& header, const SaveState& s, std::span<const u8> mods,
                                 std::span<const u8> views);

// The header of a state file, or nothing (and `err`) when `bytes` is not one
// of this version.
std::optional<std::string> state_file_header(std::span<const u8> bytes, std::string* err);

// Reads a whole state file into `s`. Its hardware and input queue start as
// copies of `hw` and `input` (this machine's observers, disc and input
// source), then take the file's state. False with `err` on a damaged file.
bool read_state_file(std::span<const u8> bytes, const Hardware& hw, const InputQueue* input, SaveState& s,
                     std::vector<u8>& mods, std::vector<u8>& views, std::string* err);

// Maps the RAM regions `s` holds that `m` has not mapped yet (the VM region,
// when the boot run stopped before the game's VMInit), so load_state() fits.
void prepare_machine(Machine& m, const SaveState& s);

}  // namespace mvp::rt
