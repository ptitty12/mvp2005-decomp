// Netplay in the browser engine (E5): the lockstep session (engine/net) over
// the page's WebRTC data channel. The page owns the channel (RTCPeerConnection
// is not available in Workers); datagrams cross the Worker boundary as
// messages: received ones arrive through mvp_net_recv(), sent ones leave
// through Module.mvpNetSend(bytes). mvp_web.cpp calls the functions below at
// the disc check, the boot, every frame poll and before every rt::run().
#pragma once

#include <cstdint>
#include <string>

#include "runtime/host_yield.h"
#include "runtime/input.h"
#include "runtime/machine.h"
#include "vfs/disc_check.h"

namespace mvp::web {

// The disc whose identity the handshake carries (set when the disc is checked).
void net_set_disc(const vfs::Disc* disc);
// A session exists (mvp_net_begin was called) / it finished its handshake.
bool net_active();
bool net_running();
int net_player();  // 1 or 2
// At boot: where this side's controller comes from (`port` of `src`), and the
// machine the driver hashes. Returns the input queue's source.
rt::InputSource* net_attach(rt::Machine& m, rt::YieldingHooks& hooks, rt::InputSource* local, int port);
// Before every rt::run(): whether the guest may run up to the next frame poll.
bool net_may_run();
// At every frame poll, after its input was consumed.
void net_after_poll(std::uint64_t frame);
// Gameplay mods (mods/gameplay/host.h): their canonical settings join the
// handshake's settings string (set before mvp_net_begin), and they draw from the
// session's shared seed (player 1's proposal, known once the handshake is done).
void net_set_mod_settings(const std::string& s);
std::uint64_t net_seed();

}  // namespace mvp::web
