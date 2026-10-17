// Data-free tests for the netplay wire protocol (net/protocol.h), the state
// hashes (net/state_hash.h) and the identity helpers (net/identity.h).
#include <doctest/doctest.h>

#include <cstring>
#include <string>

#include "net/identity.h"
#include "net/protocol.h"
#include "net/state_hash.h"

using namespace mvp;
using namespace mvp::net;

namespace {

Hello sample_hello(u8 player) {
  Hello h;
  h.player = player;
  h.delay = 3;
  h.hash_interval = 60;
  h.nonce = 0x0123456789ABCDEFull;
  h.seed = 42;
  h.id.build = "test-build-1";
  for (int i = 0; i < 20; ++i) h.id.disc[i] = static_cast<u8>(i);
  h.id.packset = empty_packset_hash();
  h.id.settings = "ax_voices=1;ports=0x3";
  return h;
}

PadState pad(u16 buttons, rt::s8 sx = 0) {
  PadState p;
  p.connected = true;
  p.buttons = buttons;
  p.stick_x = sx;
  p.trigger_r = 200;
  return p;
}

}  // namespace

TEST_CASE("HELLO round trip and rejections") {
  Hello h = sample_hello(2);
  h.seen_peer = true;
  std::vector<u8> w = encode(h);
  CHECK(w[0] == 0x4D);
  CHECK(w[1] == 0x4E);
  CHECK(w[2] == kProtocolMajor);
  CHECK(w[3] == static_cast<u8>(MsgType::Hello));
  CHECK(peek_type(w.data(), w.size()) == MsgType::Hello);
  auto d = decode_hello(w.data(), w.size());
  REQUIRE(d);
  CHECK(*d == h);
  // Truncated, trailing bytes, another major version, bad player, reserved flags.
  CHECK_FALSE(decode_hello(w.data(), w.size() - 1));
  std::vector<u8> longer = w;
  longer.push_back(0);
  CHECK_FALSE(decode_hello(longer.data(), longer.size()));
  std::vector<u8> v2 = w;
  v2[2] = 2;
  CHECK_FALSE(peek_type(v2.data(), v2.size()));
  CHECK_FALSE(decode_hello(v2.data(), v2.size()));
  std::vector<u8> bad = w;
  bad[5] = 3;  // player 3
  CHECK_FALSE(decode_hello(bad.data(), bad.size()));
  bad = w;
  bad[6] |= 2;  // reserved flag
  CHECK_FALSE(decode_hello(bad.data(), bad.size()));
  CHECK_FALSE(decode_input(w.data(), w.size()));  // wrong type
}

TEST_CASE("INPUT round trip and rejections") {
  Input m;
  m.ack_frame = 1000;
  m.ack_hash = 959;
  m.cur_frame = 998;
  m.send_ms = 0xFFFFFFF0u;
  m.echo_ms = 12345;
  m.echo_hold_ms = 3;
  m.first_frame = 1001;
  m.pads = {pad(rt::pad::kA), pad(rt::pad::kStart | rt::pad::kUp, -128), pad(0, 127)};
  m.hashes = {{959, 0x1122334455667788ull}, {1019, 0x99AABBCCDDEEFF00ull}};
  std::vector<u8> w = encode(m);
  CHECK(w.size() == kHeaderSize + 7 * 4 + 1 + 3 * kPadWireSize + 1 + 2 * 12);
  auto d = decode_input(w.data(), w.size());
  REQUIRE(d);
  CHECK(*d == m);
  {
    // Repeated pads are one byte each; a full pad equal to the previous is refused.
    Input r = m;
    r.pads = {pad(rt::pad::kA), pad(rt::pad::kA), pad(rt::pad::kA), pad(rt::pad::kB), pad(rt::pad::kB)};
    std::vector<u8> wr = encode(r);
    CHECK(wr.size() == kHeaderSize + 28 + 1 + 2 * kPadWireSize + 3 + 1 + 2 * 12);
    CHECK(decode_input(wr.data(), wr.size()) == r);
    std::vector<u8> full = wr;
    const std::size_t at = kHeaderSize + 28 + 1 + kPadWireSize;  // the first repeat byte
    CHECK(full[at] == kPadRepeat);
    std::vector<u8> expanded(full.begin(), full.begin() + static_cast<std::ptrdiff_t>(at));
    put_pad(expanded, r.pads[1]);
    expanded.insert(expanded.end(), full.begin() + static_cast<std::ptrdiff_t>(at + 1), full.end());
    CHECK_FALSE(decode_input(expanded.data(), expanded.size()));
    // The first pad cannot be a repeat.
    Input one;
    one.pads = {pad(rt::pad::kA)};
    std::vector<u8> w1 = encode(one);
    w1[kHeaderSize + 28 + 1] = kPadRepeat;
    w1.resize(kHeaderSize + 28 + 2);
    w1.push_back(0);
    CHECK_FALSE(decode_input(w1.data(), w1.size()));
  }
  // Unknown button bit in a pad.
  std::vector<u8> bad = w;
  const std::size_t pad0 = kHeaderSize + 28 + 1;
  bad[pad0 + 1] |= 0x80;
  CHECK_FALSE(decode_input(bad.data(), bad.size()));
  // A disconnected pad with controls.
  bad = w;
  bad[pad0] = 0;
  CHECK_FALSE(decode_input(bad.data(), bad.size()));
  // Hash records out of order.
  Input m2 = m;
  std::swap(m2.hashes[0], m2.hashes[1]);
  std::vector<u8> w2 = encode(m2);
  CHECK_FALSE(decode_input(w2.data(), w2.size()));
  // Truncated anywhere.
  for (std::size_t n = 0; n < w.size(); ++n) CHECK_FALSE(decode_input(w.data(), n));
  // Empty packet (keepalive).
  Input e;
  std::vector<u8> we = encode(e);
  CHECK(we.size() == kHeaderSize + 30);
  CHECK(decode_input(we.data(), we.size()) == e);
}

TEST_CASE("BYE round trip") {
  Bye b{ByeReason::Desync, 3600, "state hash differs"};
  std::vector<u8> w = encode(b);
  auto d = decode_bye(w.data(), w.size());
  REQUIRE(d);
  CHECK(*d == b);
  w[kHeaderSize] = 9;  // unknown reason
  CHECK_FALSE(decode_bye(w.data(), w.size()));
}

TEST_CASE("garbage never parses") {
  std::vector<u8> g(64);
  std::uint64_t x = 99;
  for (int round = 0; round < 2000; ++round) {
    for (u8& b : g) {
      x = x * 6364136223846793005ull + 1442695040888963407ull;
      b = static_cast<u8>(x >> 56);
    }
    const std::size_t n = (x >> 8) % g.size();
    // Random bytes behind a valid header: decoders must either reject or round-trip.
    g[0] = kMagic0, g[1] = kMagic1, g[2] = kProtocolMajor, g[3] = static_cast<u8>(1 + (x >> 20) % 3);
    if (auto h = decode_hello(g.data(), n)) CHECK(encode(*h).size() == n);
    if (auto i = decode_input(g.data(), n)) CHECK(encode(*i) == std::vector<u8>(g.begin(), g.begin() + n));
    if (auto b = decode_bye(g.data(), n)) CHECK(encode(*b) == std::vector<u8>(g.begin(), g.begin() + n));
  }
}

TEST_CASE("handshake compatibility") {
  Hello a = sample_hello(1), b = sample_hello(2);
  CHECK(incompatibility(a, b).empty());
  b.delay = 4;  // negotiated, not checked
  b.seed = 7;
  CHECK(incompatibility(a, b).empty());
  Hello c = sample_hello(1);
  CHECK(incompatibility(a, c).find("both players chose player 1") != std::string::npos);
  c = sample_hello(2);
  c.id.build = "other";
  CHECK(incompatibility(a, c).find("engine builds") != std::string::npos);
  c = sample_hello(2);
  c.id.disc[0] ^= 1;
  CHECK(incompatibility(a, c).find("disc") != std::string::npos);
  c = sample_hello(2);
  c.id.packset[0] ^= 1;
  // E6: in words a player understands, with both set hashes' prefixes.
  CHECK(incompatibility(a, c).find("different roster packs") != std::string::npos);
  CHECK(incompatibility(a, c).find("same packs in the same order") != std::string::npos);
  CHECK(incompatibility(a, c).find("player 2: " + hex(c.id.packset.data(), 32).substr(0, 16)) != std::string::npos);
  CHECK(incompatibility(c, a) == incompatibility(a, c));  // the same text on both sides
  c = sample_hello(2);
  c.id.cpi_milli = 3;
  CHECK(incompatibility(a, c).find("CLK2/DVD1") != std::string::npos);
  // CLK2: the identity carries the runtime's clock; a CLK1 build sent 2 (whole cycles) in the same field.
  CHECK(Identity{}.cpi_milli == rt::ClockConfig{}.cpi_milli);
  c = sample_hello(2);
  c.id.cpi_milli = 2;
  CHECK(incompatibility(a, c).find("CLK2/DVD1") != std::string::npos);
  CHECK(incompatibility(c, a).find("CLK2/DVD1") != std::string::npos);
  c = sample_hello(2);
  c.id.di_tb_per_kib = 1;
  CHECK(incompatibility(a, c).find("CLK2/DVD1") != std::string::npos);
  c = sample_hello(2);
  c.id.settings = "ax_voices=0;ports=0x3";
  CHECK(incompatibility(a, c).find("settings") != std::string::npos);
  // Gameplay mods (mods/gameplay/host.h): only the keys that differ, by player, in words.
  Hello m1 = sample_hello(1), m2 = sample_hello(2);
  m1.id.settings = "ports=0x3;mod.abs.miss=30;mod.abs=1";
  m2.id.settings = "ports=0x3;mod.abs.miss=20;mod.abs=1";
  CHECK(incompatibility(m1, m2) ==
        "the players chose different gameplay mod settings (mod.abs.miss: player 1 30, player 2 20); "
        "both must choose the same");
  CHECK(incompatibility(m2, m1) == incompatibility(m1, m2));  // the same text on both sides
  m2.id.settings = "ports=0x3";
  CHECK(incompatibility(m1, m2).find("mod.abs: player 1 1, player 2 (none)") != std::string::npos);
  m2.id.settings = "ports=0x1;mod.abs.miss=30;mod.abs=1";
  CHECK(incompatibility(m1, m2) == "different settings (ports: player 1 0x3, player 2 0x1)");
  c = sample_hello(2);
  c.hash_interval = 30;
  CHECK(incompatibility(a, c).find("desync-check") != std::string::npos);
}

TEST_CASE("xxh64 matches the reference vectors") {
  CHECK(xxh64("", 0) == 0xEF46DB3751D8E999ull);
  CHECK(xxh64("a", 1) == 0xD24EC4F1A98C6E5Bull);
  CHECK(xxh64("abc", 3) == 0x44BC2CF5AD770999ull);
  const char* s = "Nobody inspects the spammish repetition";  // 39 bytes: the 32-byte stripe path
  CHECK(xxh64(s, std::strlen(s)) == 0xFBCEA83C8A378BF1ull);
  // Seeds change the result; every length up to 100 differs from its neighbours.
  CHECK(xxh64("abc", 3, 1) != xxh64("abc", 3, 0));
  std::vector<u8> buf(100);
  for (std::size_t i = 0; i < buf.size(); ++i) buf[i] = static_cast<u8>(i * 7 + 1);
  for (std::size_t n = 1; n < buf.size(); ++n) CHECK(xxh64(buf.data(), n) != xxh64(buf.data(), n - 1));
}

TEST_CASE("state hasher: windows, slices, sensitivity") {
  std::vector<u8> mem(1 << 16);
  for (std::size_t i = 0; i < mem.size(); ++i) mem[i] = static_cast<u8>(i * 31 + 7);
  auto window = [&](StateHasher& h, u64 first, std::vector<u8>& m, u64 ticks) {
    std::optional<u64> r;
    for (u64 f = first; f < first + h.interval(); ++f) {
      auto x = h.at_frame(f, m.data(), m.size(), ticks + f);
      if (f + 1 < first + h.interval()) CHECK_FALSE(x);
      r = x;
    }
    return r;
  };
  StateHasher a(6), b(6);
  // Slices cover the memory exactly once, 64-byte aligned.
  std::size_t covered = 0;
  for (u32 s = 0; s < 6; ++s) {
    auto [lo, hi] = a.slice(s, mem.size());
    CHECK(lo == covered);
    CHECK(lo % 64 == 0);
    covered = hi;
  }
  CHECK(covered == mem.size());
  auto ha = window(a, 0, mem, 100), hb = window(b, 0, mem, 100);
  REQUIRE(ha);
  CHECK(ha == hb);
  CHECK(a.last_slices().size() == 6);
  // The same memory in the next window hashes differently (frame numbers are in it).
  CHECK(window(a, 6, mem, 100) != ha);
  // One byte changed anywhere changes the window hash and exactly one slice hash.
  for (std::size_t pos : {std::size_t(0), mem.size() / 2, mem.size() - 1}) {
    StateHasher x(6), y(6);
    std::vector<u8> m2 = mem;
    m2[pos] ^= 0x40;
    auto hx = window(x, 0, mem, 100), hy = window(y, 0, m2, 100);
    CHECK(hx != hy);
    int differ = 0;
    for (int s = 0; s < 6; ++s) differ += x.last_slices()[s] != y.last_slices()[s];
    CHECK(differ == 1);
  }
  // The instruction count is part of it.
  StateHasher z(6);
  CHECK(window(z, 0, mem, 101) != ha);
}

TEST_CASE("identity helpers") {
  rt::HardwareConfig hw;
  rt::BootConfig boot;
  const std::string s = settings_string(hw, boot, kNetplayPorts);
  CHECK(s.find("ax_voices=1") != std::string::npos);
  CHECK(s.find("ports=0x3") != std::string::npos);
  CHECK(s.find("pad=v1") != std::string::npos);
  // Sorted keys.
  std::vector<std::string> keys;
  for (std::size_t at = 0; at < s.size();) {
    std::size_t semi = s.find(';', at);
    if (semi == std::string::npos) semi = s.size();
    keys.push_back(s.substr(at, s.find('=', at) - at));
    at = semi + 1;
  }
  for (std::size_t k = 1; k < keys.size(); ++k) CHECK(keys[k - 1] < keys[k]);
  hw.ax_voices = false;
  CHECK(settings_string(hw, boot, kNetplayPorts) != s);
  CHECK(hex(empty_packset_hash().data(), 32) == "f5e1298911db13adf522a1b34d4d4f9291b42fa7a77f63a5f6f07bc322dcc66e");
}
