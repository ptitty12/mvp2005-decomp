// Netplay in the browser engine (E5): see net_web.h. The exports below are
// called by the Worker (engine/web/src/worker.ts):
//   mvp_net_begin(player, delay, hash_every, build, len, nonce)   the session; its handshake starts
//   mvp_net_recv(data, len)      a datagram from the data channel
//   mvp_net_pump()               timers, resends, received datagrams; returns the state
//   mvp_net_status()             JSON for the page
//   mvp_net_diagnostic()         the local desync diagnostic (text, no game data)
//   mvp_net_quit()               says goodbye to the peer
//   mvp_net_lead()               Q5: the input lead (ms, the median of the last frames; net/driver.h)
#include "net_web.h"

#include <array>

#include <emscripten.h>

#include <cstdio>
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "net/driver.h"
#include "net/identity.h"
#include "net/session.h"
#include "runtime/boot.h"
#include "runtime/hw.h"
#include "runtime/sys_hooks.h"
#include "web/wasm/packs_web.h"

using namespace mvp;

EM_JS(void, mvp_js_net_send, (const void* data, int len), {
  if (Module['mvpNetSend']) Module['mvpNetSend'](HEAPU8.slice(data >>> 0, (data >>> 0) + len));
});

namespace {

std::uint64_t now_ms() { return static_cast<std::uint64_t>(emscripten_get_now()); }

// The data channel, seen from the Worker.
class ChannelTransport final : public net::Transport {
 public:
  void send(const std::uint8_t* data, std::size_t size) override { mvp_js_net_send(data, static_cast<int>(size)); }
  bool receive(std::vector<std::uint8_t>& out) override {
    if (inbox.empty()) return false;
    out = std::move(inbox.front());
    inbox.pop_front();
    return true;
  }
  std::deque<std::vector<std::uint8_t>> inbox;
};

struct Net {
  const vfs::Disc* disc = nullptr;
  std::array<std::uint8_t, 20> disc_id{};  // computed when the disc is checked, before packs are mounted
  ChannelTransport channel;
  std::unique_ptr<net::Session> session;
  std::unique_ptr<net::NetplayDriver> driver;
  rt::InputSource* local = nullptr;
  int local_port = 0;
  std::string text;
  std::string mod_settings;  // gameplay mods' canonical settings, part of the handshake
};
Net n;

}  // namespace

namespace mvp::web {

void net_set_disc(const vfs::Disc* disc) {
  n.disc = disc;
  if (disc) n.disc_id = net::disc_identity(*disc);
}
bool net_active() { return n.session != nullptr; }
bool net_running() { return n.session && n.session->running(); }
int net_player() { return n.session ? n.session->config().player : 0; }

rt::InputSource* net_attach(rt::Machine& m, rt::YieldingHooks& hooks, rt::InputSource* local, int port) {
  n.local = local;
  n.local_port = port;
  n.driver = std::make_unique<net::NetplayDriver>(*n.session, n.channel, m, &hooks, &now_ms);
  return n.driver.get();
}

bool net_may_run() { return n.driver && n.driver->may_run(now_ms()); }

void net_after_poll(std::uint64_t frame) {
  if (n.driver) n.driver->after_poll(frame);
}

void net_set_mod_settings(const std::string& s) { n.mod_settings = s; }
std::uint64_t net_seed() { return n.session ? n.session->seed() : 0; }

}  // namespace mvp::web

extern "C" {

// Creates the session (player 1 or 2, proposed delay, desync window in frames,
// the engine build id the page computed). Returns 1, or 0 without a checked disc.
EMSCRIPTEN_KEEPALIVE int mvp_net_begin(int player, int delay, int hash_every, const char* build, int build_len,
                                       double nonce) {
  if (!n.disc || n.session) return 0;
  net::SessionConfig cfg;
  cfg.player = player == 2 ? 2 : 1;
  cfg.delay = static_cast<std::uint32_t>(delay < 0 ? 0 : delay);
  cfg.hash_interval = static_cast<std::uint32_t>(hash_every < 0 ? 0 : hash_every);
  cfg.nonce = static_cast<std::uint64_t>(nonce);
  // The same configuration mvp_boot() uses (defaults), with both player ports.
  cfg.id = net::make_identity(std::string(build, static_cast<std::size_t>(build_len)), *n.disc, rt::HardwareConfig{},
                              rt::BootConfig{}, rt::ClockConfig{}, net::kNetplayPorts);
  // The disc as the player's image has it (mounting roster packs rewrites the FST),
  // and the packs by their ordered set hash (E6, MODDING.md §10).
  cfg.id.disc = n.disc_id;
  cfg.id.packset = web::packs_set_hash();
  // Gameplay mods and their settings must match too (a mismatch refuses the game),
  // and player 1's random nonce is the seed proposal for their random streams.
  if (!n.mod_settings.empty()) cfg.id.settings += ";" + n.mod_settings;
  cfg.seed = cfg.nonce;
  n.session = std::make_unique<net::Session>(n.channel, cfg, [](std::uint64_t f) {
    if (!n.local) return net::Session::neutral_pad();
    return n.local->input_for(f).pad[static_cast<std::size_t>(n.local_port)];
  });
  n.session->update(now_ms());
  return 1;
}

EMSCRIPTEN_KEEPALIVE void mvp_net_recv(const std::uint8_t* data, int len) {
  if (len > 0 && len <= 2048) n.channel.inbox.emplace_back(data, data + len);
}

// 0 handshake, 1 running, 2 desynced, 3 disconnected, 4 rejected, 5 ended, -1 no session.
EMSCRIPTEN_KEEPALIVE int mvp_net_pump() {
  if (!n.session) return -1;
  n.session->update(now_ms());
  return static_cast<int>(n.session->state());
}

EMSCRIPTEN_KEEPALIVE void mvp_net_quit() {
  if (n.session) n.session->quit(now_ms(), "the other player closed the game");
}

EMSCRIPTEN_KEEPALIVE const char* mvp_net_status() {
  if (!n.session) return "{}";
  const net::Session& s = *n.session;
  const net::SessionStats& st = s.stats();
  std::string reason;
  for (char ch : s.reason()) {
    if (ch == '"' || ch == '\\') reason += '\\';
    reason += static_cast<unsigned char>(ch) < 0x20 ? ' ' : ch;
  }
  char last[40] = "";
  if (n.driver && !n.driver->windows().empty()) {
    const auto& w = n.driver->windows().back();
    std::snprintf(last, sizeof last, "%u:%016llx", w.frame, static_cast<unsigned long long>(w.hash));
  }
  auto ull = [](std::uint64_t v) { return static_cast<unsigned long long>(v); };
  const net::NetplayDriver* d = n.driver.get();
  char b[1600];
  std::snprintf(
      b, sizeof b,
      "{\"state\":\"%s\",\"reason\":\"%s\",\"player\":%d,\"delay\":%u,\"frame\":%llu,\"peerFrame\":%u,"
      "\"localHorizon\":%llu,\"remoteHorizon\":%llu,\"rttLast\":%u,\"rttMin\":%u,\"rttAvg\":%.1f,\"rttMax\":%u,"
      "\"packetsSent\":%llu,\"bytesSent\":%llu,\"packetsReceived\":%llu,\"bytesReceived\":%llu,\"badPackets\":%llu,"
      "\"hashesCompared\":%llu,\"stalls\":%llu,\"stallMs\":%llu,\"longestStallMs\":%llu,\"gateMisses\":%llu,"
      "\"earlyYields\":%llu,\"pollStops\":%llu,\"latencyAvg\":%.1f,\"latencyMax\":%llu,\"latencySum\":%llu,\"latencySamples\":%llu,\"windows\":%llu,\"lastWindow\":\"%s\",\"desync\":%s,"
      "\"rttRecent\":%.1f,\"rttJitter\":%.1f,\"leadMs\":%.1f,\"leadSamples\":%llu}",
      net::state_name(s.state()), reason.c_str(), s.config().player, s.delay(), ull(s.next_frame()), s.peer_frame(),
      ull(s.local_horizon()), ull(s.remote_horizon()), st.rtt_last, st.rtt_min, st.rtt_avg(), st.rtt_max,
      ull(st.packets_sent), ull(st.bytes_sent), ull(st.packets_received), ull(st.bytes_received), ull(st.bad_packets),
      ull(st.hashes_compared), ull(d ? d->stalls : 0), ull(d ? d->stall_ms : 0), ull(d ? d->longest_stall_ms : 0),
      ull(d ? d->gate_misses : 0), ull(d ? d->early_yields : 0), ull(d ? d->poll_stops : 0), st.latency_avg(), ull(st.latency_max), ull(st.latency_sum), ull(st.latency_samples),
      ull(d ? d->windows().size() : 0), last,
      s.state() == net::State::Desynced ? "true" : "false", st.rtt_recent, st.rtt_jitter, d ? d->lead_ms() : 0.0,
      ull(d ? d->lead_samples : 0));
  n.text = b;
  return n.text.c_str();
}

// Q5: the input lead (net/driver.h lead_ms()): the median, over the last frames, of how
// long before this side needed the peer's input it arrived. The Worker's pacing reads it
// several times a second; NaN before any sample.
EMSCRIPTEN_KEEPALIVE double mvp_net_lead() {
  if (!n.driver || !n.driver->lead_samples) return __builtin_nan("");
  return n.driver->lead_ms();
}

// Every window hash this side computed, "frame:hash" per line (tests compare two pages).
EMSCRIPTEN_KEEPALIVE const char* mvp_net_windows() {
  n.text.clear();
  if (n.driver)
    for (const auto& w : n.driver->windows()) {
      char b[40];
      std::snprintf(b, sizeof b, "%u:%016llx\n", w.frame, static_cast<unsigned long long>(w.hash));
      n.text += b;
    }
  return n.text.c_str();
}

EMSCRIPTEN_KEEPALIVE const char* mvp_net_diagnostic() {
  n.text = n.driver ? n.driver->diagnostic() : (n.session ? n.session->diagnostic() : std::string());
  return n.text.c_str();
}

}  // extern "C"
