#include "net/protocol.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <string>

#include "common/bytes.h"

namespace mvp::net {

namespace {

void header(std::vector<u8>& out, MsgType t) {
  out.push_back(kMagic0);
  out.push_back(kMagic1);
  out.push_back(kProtocolMajor);
  out.push_back(static_cast<u8>(t));
}
void u16_(std::vector<u8>& out, u16 v) {
  out.push_back(static_cast<u8>(v >> 8));
  out.push_back(static_cast<u8>(v));
}
void u32_(std::vector<u8>& out, u32 v) {
  for (int s = 24; s >= 0; s -= 8) out.push_back(static_cast<u8>(v >> s));
}
void u64_(std::vector<u8>& out, u64 v) {
  u32_(out, static_cast<u32>(v >> 32));
  u32_(out, static_cast<u32>(v));
}

// A bounds-checked reader: every get fails (and stays failed) past the end.
struct Reader {
  const u8* p;
  std::size_t n, at = kHeaderSize;
  bool ok = true;
  bool need(std::size_t k) {
    if (!ok || n - at < k) ok = false;
    return ok;
  }
  u8 b() { return need(1) ? p[at++] : 0; }
  u16 w() {
    if (!need(2)) return 0;
    u16 v = be16(p + at);
    at += 2;
    return v;
  }
  u32 d() {
    if (!need(4)) return 0;
    u32 v = be32(p + at);
    at += 4;
    return v;
  }
  u64 q() {
    if (!need(8)) return 0;
    u64 v = be64(p + at);
    at += 8;
    return v;
  }
  const u8* bytes(std::size_t k) {
    if (!need(k)) return nullptr;
    const u8* r = p + at;
    at += k;
    return r;
  }
  bool done() const { return ok && at == n; }
};

bool header_ok(const u8* d, std::size_t n, MsgType t) {
  return n >= kHeaderSize && d[0] == kMagic0 && d[1] == kMagic1 && d[2] == kProtocolMajor && d[3] == static_cast<u8>(t);
}

}  // namespace

std::string hex(const u8* data, std::size_t size) {
  static const char* k = "0123456789abcdef";
  std::string s;
  s.reserve(size * 2);
  for (std::size_t i = 0; i < size; ++i) {
    s.push_back(k[data[i] >> 4]);
    s.push_back(k[data[i] & 15]);
  }
  return s;
}

void put_pad(std::vector<u8>& out, const PadState& p) {
  out.push_back(p.connected ? 1 : 0);
  out.push_back(static_cast<u8>(p.buttons >> 8));
  out.push_back(static_cast<u8>(p.buttons));
  for (rt::s8 v : {p.stick_x, p.stick_y, p.substick_x, p.substick_y}) out.push_back(static_cast<u8>(v));
  for (u8 v : {p.trigger_l, p.trigger_r, p.analog_a, p.analog_b}) out.push_back(v);
}

std::optional<PadState> get_pad(const u8* q) {
  PadState p;
  if (q[0] & ~1u) return std::nullopt;  // reserved flag bits
  p.connected = q[0] & 1;
  p.buttons = static_cast<u16>(q[1] << 8 | q[2]);
  if (p.buttons & ~rt::pad::kAll) return std::nullopt;
  p.stick_x = static_cast<rt::s8>(q[3]);
  p.stick_y = static_cast<rt::s8>(q[4]);
  p.substick_x = static_cast<rt::s8>(q[5]);
  p.substick_y = static_cast<rt::s8>(q[6]);
  p.trigger_l = q[7];
  p.trigger_r = q[8];
  p.analog_a = q[9];
  p.analog_b = q[10];
  if (!p.connected && !(p == PadState{})) return std::nullopt;  // a missing pad has no controls
  return p;
}

// HELLO body: minor, player, flags (bit 0: seen_peer), delay, hash_interval (2),
// reserved (2, zero), nonce (8), seed (8), disc (20), packset (32), cpi_milli (4),
// di_latency_tb (4), di_tb_per_kib (4), build length (1) + bytes, settings
// length (2) + bytes.
std::vector<u8> encode(const Hello& m) {
  std::vector<u8> out;
  header(out, MsgType::Hello);
  out.push_back(m.minor);
  out.push_back(m.player);
  out.push_back(m.seen_peer ? 1 : 0);
  out.push_back(m.delay);
  u16_(out, m.hash_interval);
  u16_(out, 0);
  u64_(out, m.nonce);
  u64_(out, m.seed);
  out.insert(out.end(), m.id.disc.begin(), m.id.disc.end());
  out.insert(out.end(), m.id.packset.begin(), m.id.packset.end());
  u32_(out, m.id.cpi_milli);
  u32_(out, m.id.di_latency_tb);
  u32_(out, m.id.di_tb_per_kib);
  const std::size_t bl = std::min(m.id.build.size(), kMaxBuildId);
  out.push_back(static_cast<u8>(bl));
  out.insert(out.end(), m.id.build.begin(), m.id.build.begin() + static_cast<std::ptrdiff_t>(bl));
  const std::size_t sl = std::min(m.id.settings.size(), kMaxSettings);
  u16_(out, static_cast<u16>(sl));
  out.insert(out.end(), m.id.settings.begin(), m.id.settings.begin() + static_cast<std::ptrdiff_t>(sl));
  return out;
}

std::optional<Hello> decode_hello(const u8* d, std::size_t n) {
  if (!header_ok(d, n, MsgType::Hello)) return std::nullopt;
  Reader r{d, n};
  Hello m;
  m.minor = r.b();
  m.player = r.b();
  u8 flags = r.b();
  m.seen_peer = flags & 1;
  m.delay = r.b();
  m.hash_interval = r.w();
  u16 reserved = r.w();
  m.nonce = r.q();
  m.seed = r.q();
  if (const u8* p = r.bytes(20)) std::memcpy(m.id.disc.data(), p, 20);
  if (const u8* p = r.bytes(32)) std::memcpy(m.id.packset.data(), p, 32);
  m.id.cpi_milli = r.d();
  m.id.di_latency_tb = r.d();
  m.id.di_tb_per_kib = r.d();
  u8 bl = r.b();
  if (bl > kMaxBuildId) return std::nullopt;
  if (const u8* p = r.bytes(bl)) m.id.build.assign(reinterpret_cast<const char*>(p), bl);
  u16 sl = r.w();
  if (sl > kMaxSettings) return std::nullopt;
  if (const u8* p = r.bytes(sl)) m.id.settings.assign(reinterpret_cast<const char*>(p), sl);
  if (!r.done() || (flags & ~1u) || reserved != 0) return std::nullopt;
  if (m.player != 1 && m.player != 2) return std::nullopt;
  return m;
}

// INPUT body: ack_frame, ack_hash, cur_frame, send_ms, echo_ms, echo_hold_ms,
// first_frame (4 each), pad count (1) + the pads, hash count (1) + 12 bytes each
// (frame 4, hash 8). A pad is 11 bytes, or the single byte kPadRepeat when it
// equals the previous pad of the packet (never for the first).
std::vector<u8> encode(const Input& m) {
  std::vector<u8> out;
  out.reserve(kHeaderSize + 30 + m.pads.size() * kPadWireSize + m.hashes.size() * 12);
  header(out, MsgType::Input);
  for (u32 v : {m.ack_frame, m.ack_hash, m.cur_frame, m.send_ms, m.echo_ms, m.echo_hold_ms, m.first_frame}) u32_(out, v);
  const std::size_t np = std::min(m.pads.size(), kMaxInputsPerPacket);
  out.push_back(static_cast<u8>(np));
  for (std::size_t k = 0; k < np; ++k) {
    if (k > 0 && m.pads[k] == m.pads[k - 1])
      out.push_back(kPadRepeat);  // the same as the previous pad: one byte
    else
      put_pad(out, m.pads[k]);
  }
  const std::size_t nh = std::min(m.hashes.size(), kMaxHashesPerPacket);
  out.push_back(static_cast<u8>(nh));
  for (std::size_t k = 0; k < nh; ++k) {
    u32_(out, m.hashes[k].frame);
    u64_(out, m.hashes[k].hash);
  }
  return out;
}

std::optional<Input> decode_input(const u8* d, std::size_t n) {
  if (!header_ok(d, n, MsgType::Input)) return std::nullopt;
  Reader r{d, n};
  Input m;
  m.ack_frame = r.d();
  m.ack_hash = r.d();
  m.cur_frame = r.d();
  m.send_ms = r.d();
  m.echo_ms = r.d();
  m.echo_hold_ms = r.d();
  m.first_frame = r.d();
  u8 np = r.b();
  if (np > kMaxInputsPerPacket) return std::nullopt;
  for (u8 k = 0; k < np; ++k) {
    if (k > 0 && r.need(1) && d[r.at] == kPadRepeat) {
      ++r.at;
      m.pads.push_back(m.pads.back());
      continue;
    }
    const u8* p = r.bytes(kPadWireSize);
    if (!p) return std::nullopt;
    auto pad = get_pad(p);
    if (!pad) return std::nullopt;
    if (k > 0 && *pad == m.pads.back()) return std::nullopt;  // must have been a repeat byte
    m.pads.push_back(*pad);
  }
  u8 nh = r.b();
  if (nh > kMaxHashesPerPacket) return std::nullopt;
  for (u8 k = 0; k < nh; ++k) {
    HashRecord h;
    h.frame = r.d();
    h.hash = r.q();
    if (!m.hashes.empty() && h.frame <= m.hashes.back().frame) return std::nullopt;  // ascending
    m.hashes.push_back(h);
  }
  if (!r.done()) return std::nullopt;
  if (static_cast<u64>(m.first_frame) + np > 0xFFFFFFFFull) return std::nullopt;
  return m;
}

// BYE body: reason (1), frame (4), text length (1) + bytes.
std::vector<u8> encode(const Bye& m) {
  std::vector<u8> out;
  header(out, MsgType::Bye);
  out.push_back(static_cast<u8>(m.reason));
  u32_(out, m.frame);
  const std::size_t tl = std::min(m.text.size(), kMaxReason);
  out.push_back(static_cast<u8>(tl));
  out.insert(out.end(), m.text.begin(), m.text.begin() + static_cast<std::ptrdiff_t>(tl));
  return out;
}

std::optional<Bye> decode_bye(const u8* d, std::size_t n) {
  if (!header_ok(d, n, MsgType::Bye)) return std::nullopt;
  Reader r{d, n};
  Bye m;
  u8 reason = r.b();
  if (reason > static_cast<u8>(ByeReason::Error)) return std::nullopt;
  m.reason = static_cast<ByeReason>(reason);
  m.frame = r.d();
  u8 tl = r.b();
  if (tl > kMaxReason) return std::nullopt;
  if (const u8* p = r.bytes(tl)) m.text.assign(reinterpret_cast<const char*>(p), tl);
  if (!r.done()) return std::nullopt;
  return m;
}

std::optional<MsgType> peek_type(const u8* d, std::size_t n) {
  if (n < kHeaderSize || d[0] != kMagic0 || d[1] != kMagic1 || d[2] != kProtocolMajor) return std::nullopt;
  if (d[3] < static_cast<u8>(MsgType::Hello) || d[3] > static_cast<u8>(MsgType::Bye)) return std::nullopt;
  return static_cast<MsgType>(d[3]);
}

namespace {

// "k=v;k=v" (net/identity.h) as a map.
std::map<std::string, std::string> parse_settings(const std::string& s) {
  std::map<std::string, std::string> m;
  for (std::size_t at = 0; at < s.size();) {
    std::size_t end = s.find(';', at);
    if (end == std::string::npos) end = s.size();
    const std::string kv = s.substr(at, end - at);
    const std::size_t eq = kv.find('=');
    if (!kv.empty()) m[kv.substr(0, eq)] = eq == std::string::npos ? "" : kv.substr(eq + 1);
    at = end + 1;
  }
  return m;
}

// The settings that differ, by player ("key: player 1 v1, player 2 v2, ..."), and
// whether they are all gameplay mods' (mods/gameplay/host.h: "mod" keys).
std::string settings_difference(const Hello& p1, const Hello& p2, bool* mods_only) {
  const auto a = parse_settings(p1.id.settings), b = parse_settings(p2.id.settings);
  std::set<std::string> keys;
  for (const auto& [k, v] : a) keys.insert(k);
  for (const auto& [k, v] : b) keys.insert(k);
  std::string out;
  *mods_only = true;
  for (const std::string& k : keys) {
    const auto ia = a.find(k), ib = b.find(k);
    const std::string va = ia == a.end() ? "(none)" : ia->second, vb = ib == b.end() ? "(none)" : ib->second;
    if (va == vb) continue;
    if (k.rfind("mod.", 0) != 0) *mods_only = false;
    out += (out.empty() ? "" : ", ") + k + ": player 1 " + va + ", player 2 " + vb;
  }
  return out;
}

}  // namespace

std::string incompatibility(const Hello& a, const Hello& b) {
  if (a.player == b.player)
    return "both players chose player " + std::to_string(a.player) + " (one must be player 1, the other player 2)";
  if (a.id.build != b.id.build) return "different engine builds (" + a.id.build + " vs " + b.id.build + ")";
  if (a.id.disc != b.id.disc)
    return "different disc identities (" + hex(a.id.disc.data(), 20) + " vs " + hex(b.id.disc.data(), 20) + ")";
  if (a.id.packset != b.id.packset) {
    // Named by player, not "here"/"there": the other side shows this text too (BYE).
    const Hello& p1 = a.player == 1 ? a : b;
    const Hello& p2 = a.player == 1 ? b : a;
    return "the players chose different roster packs (player 1: pack set " + hex(p1.id.packset.data(), 32).substr(0, 16) +
           ", player 2: " + hex(p2.id.packset.data(), 32).substr(0, 16) +
           "); both must pick the same packs in the same order";
  }
  if (a.id.cpi_milli != b.id.cpi_milli || a.id.di_latency_tb != b.id.di_latency_tb ||
      a.id.di_tb_per_kib != b.id.di_tb_per_kib)
    return "different engine timing constants (CLK2/DVD1)";
  if (a.id.settings != b.id.settings) {
    const Hello& p1 = a.player == 1 ? a : b;
    const Hello& p2 = a.player == 1 ? b : a;
    bool mods_only = false;
    const std::string diff = settings_difference(p1, p2, &mods_only);
    if (mods_only) return "the players chose different gameplay mod settings (" + diff + "); both must choose the same";
    return "different settings (" + diff + ")";
  }
  if (a.hash_interval != b.hash_interval)
    return "different desync-check intervals (" + std::to_string(a.hash_interval) + " vs " +
           std::to_string(b.hash_interval) + ")";
  return "";
}

}  // namespace mvp::net
