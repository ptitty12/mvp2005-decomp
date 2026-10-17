#include "mods/gameplay/host.h"

#include <algorithm>
#include <cstdio>
#include <sstream>

#include "common/sha256.h"
#include "ppc/mem_access.h"
#include "runtime/hle.h"
#include "runtime/hw.h"
#include "runtime/mod_hooks.h"

namespace mvp::mods {

namespace {

bool valid_id(const std::string& s) {
  if (s.empty()) return false;
  for (char ch : s)
    if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_')) return false;
  return true;
}

u32 rotl(u32 x, int k) { return (x << k) | (x >> (32 - k)); }

// xoshiro128** (Blackman, Vigna), state s[0..3].
u32 xoshiro_next(u32* s) {
  const u32 result = rotl(s[1] * 5, 7) * 9;
  const u32 t = s[1] << 9;
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = rotl(s[3], 11);
  return result;
}

// Stream seed: the first 16 bytes of SHA-256("mvp-mod-rng/1" | seed (8, BE) | id | stream (4, BE)).
void seed_stream(u32* s, u64 seed, const std::string& id, u32 stream) {
  Sha256 h;
  const char tag[] = "mvp-mod-rng/1";
  h.update(tag, sizeof tag - 1);
  u8 b[8];
  for (int k = 0; k < 8; ++k) b[k] = static_cast<u8>(seed >> (56 - 8 * k));
  h.update(b, 8);
  h.update(id.data(), id.size());
  for (int k = 0; k < 4; ++k) b[k] = static_cast<u8>(stream >> (24 - 8 * k));
  h.update(b, 4);
  auto d = h.finish();
  for (int k = 0; k < 4; ++k)
    s[k] = u32(d[4 * k]) << 24 | u32(d[4 * k + 1]) << 16 | u32(d[4 * k + 2]) << 8 | d[4 * k + 3];
  if (!(s[0] | s[1] | s[2] | s[3])) s[0] = 1;  // xoshiro's only invalid state
}

}  // namespace

GameplayHost::GameplayHost() = default;
GameplayHost::~GameplayHost() { stop(); }

void GameplayHost::add(std::unique_ptr<Mod> m) { available_.push_back(std::move(m)); }

bool GameplayHost::enable(const std::string& id, const std::string& settings, std::string* err) {
  auto fail = [&](const std::string& why) {
    if (err) *err = why;
    return false;
  };
  Mod* mod = nullptr;
  for (auto& m : available_)
    if (id == m->info().id) mod = m.get();
  if (!mod) return fail("no gameplay mod '" + id + "'");
  for (auto& a : active_)
    if (a.mod == mod) return fail("mod '" + id + "' is enabled twice");
  const ModInfo& info = mod->info();
  if (!valid_id(info.id)) return fail(std::string("bad mod id '") + info.id + "'");
  Active a{mod, {}, {}, {}};
  for (const Setting& s : info.settings) a.settings[s.key] = s.def;
  std::istringstream in(settings);
  for (std::string kv; std::getline(in, kv, ',');) {
    if (kv.empty()) continue;
    auto eq = kv.find('=');
    if (eq == std::string::npos) return fail("mod '" + id + "': expected key=value, got '" + kv + "'");
    std::string key = kv.substr(0, eq), val = kv.substr(eq + 1);
    const Setting* s = nullptr;
    for (const Setting& t : info.settings)
      if (key == t.key) s = &t;
    if (!s) return fail("mod '" + id + "': no setting '" + key + "'");
    char* end = nullptr;
    long v = std::strtol(val.c_str(), &end, 10);
    if (val.empty() || *end || v < s->min || v > s->max)
      return fail("mod '" + id + "': " + key + " must be an integer in " + std::to_string(s->min) + ".." +
                  std::to_string(s->max));
    a.settings[key] = static_cast<int>(v);
  }
  active_.push_back(std::move(a));
  return true;
}

bool GameplayHost::start(ppc::Cpu& c, std::string* err) {
  stop();
  cpu_ = &c;
  frame_ = 0;
  phase_ = Phase::Live;
  want_snapshot_ = want_review_ = in_review_ = false;
  snapshot_frame_ = earliest_snapshot_ = ~0ull;
  for (std::size_t k = 0; k < active_.size(); ++k) {
    Active& a = active_[k];
    const ModInfo& info = a.mod->info();
    a.state.assign(info.state_size, 0);
    for (u32 s = 0; s < 4; ++s) seed_stream(&a.rng[4 * s], seed_, info.id, s);
    for (u32 addr : info.sites) {
      bindings_.push_back(std::make_unique<Binding>(Binding{this, k, addr}));
      try {
        rt::mod::bind(addr, &GameplayHost::on_site, bindings_.back().get());
      } catch (const std::exception& e) {
        if (err) *err = std::string("mod '") + info.id + "': " + e.what();
        stop();
        return false;
      }
    }
  }
  for (Active& a : active_) {
    cur_ = &a;
    a.mod->start(*this);
  }
  cur_ = nullptr;
  return true;
}

void GameplayHost::stop() {
  rt::mod::unbind_all();
  bindings_.clear();
  cur_ = nullptr;
}

void GameplayHost::on_site(ppc::Cpu& c, void* user) {
  auto* b = static_cast<Binding*>(user);
  GameplayHost& h = *b->host;
  Active* prev = h.cur_;
  h.cur_ = &h.active_[b->mod];
  h.cur_->mod->site(h, c, b->address);
  h.cur_ = prev;
}

void GameplayHost::on_frame(ppc::Cpu& c, u64 frame) {
  frame_ = frame;
  for (Active& a : active_) {
    cur_ = &a;
    a.mod->frame(*this, c);
  }
  cur_ = nullptr;
}

std::string GameplayHost::canonical_settings() const {
  std::vector<std::string> kv;
  for (const Active& a : active_) {
    const ModInfo& info = a.mod->info();
    kv.push_back(std::string("mod.") + info.id + "=" + std::to_string(info.version));
    for (const auto& [k, v] : a.settings) kv.push_back(std::string("mod.") + info.id + "." + k + "=" + std::to_string(v));
  }
  std::sort(kv.begin(), kv.end());
  std::string s;  // "k=v;k=v", the form of the handshake's settings string (net/identity.h)
  for (const auto& e : kv) s += (s.empty() ? "" : ";") + e;
  return s;
}

std::vector<u8> GameplayHost::save_state() const {
  std::vector<u8> out;
  for (const Active& a : active_) {
    for (u32 w : a.rng)
      for (int k = 0; k < 4; ++k) out.push_back(static_cast<u8>(w >> (24 - 8 * k)));
    out.insert(out.end(), a.state.begin(), a.state.end());
  }
  return out;
}

bool GameplayHost::load_state(std::span<const u8> bytes) {
  std::size_t need = 0;
  for (const Active& a : active_) need += 64 + a.state.size();
  if (bytes.size() != need) return false;
  std::size_t at = 0;
  for (Active& a : active_) {
    for (u32& w : a.rng) {
      w = u32(bytes[at]) << 24 | u32(bytes[at + 1]) << 16 | u32(bytes[at + 2]) << 8 | bytes[at + 3];
      at += 4;
    }
    std::copy_n(bytes.begin() + static_cast<std::ptrdiff_t>(at), a.state.size(), a.state.begin());
    at += a.state.size();
  }
  return true;
}

u8* GameplayHost::span(u32 ea, u32 n, bool write) {
  u8* p = cpu_ ? ppc::host_span(*cpu_, ea, n) : nullptr;
  if (!p) {
    char b[96];
    std::snprintf(b, sizeof b, "gameplay mod %s: %s of %u bytes at 0x%08X outside guest RAM",
                  cur_ ? cur_->mod->info().id : "?", write ? "write" : "read", n, static_cast<unsigned>(ea));
    throw rt::StopRun{b};
  }
  if (write) ppc::gp_check(ea, n);
  return p;
}

u32 GameplayHost::r8(u32 ea) { return *span(ea, 1, false); }
u32 GameplayHost::r16(u32 ea) {
  const u8* p = span(ea, 2, false);
  return u32(p[0]) << 8 | p[1];
}
u32 GameplayHost::r32(u32 ea) {
  const u8* p = span(ea, 4, false);
  return u32(p[0]) << 24 | u32(p[1]) << 16 | u32(p[2]) << 8 | p[3];
}
void GameplayHost::w8(u32 ea, u32 v) { *span(ea, 1, true) = static_cast<u8>(v); }
void GameplayHost::w16(u32 ea, u32 v) {
  u8* p = span(ea, 2, true);
  p[0] = static_cast<u8>(v >> 8);
  p[1] = static_cast<u8>(v);
}
void GameplayHost::w32(u32 ea, u32 v) {
  u8* p = span(ea, 4, true);
  p[0] = static_cast<u8>(v >> 24);
  p[1] = static_cast<u8>(v >> 16);
  p[2] = static_cast<u8>(v >> 8);
  p[3] = static_cast<u8>(v);
}

u32 GameplayHost::rng(u32 stream) {
  if (!cur_ || stream > 3) throw rt::StopRun{"gameplay mod: rng() outside a callback or stream > 3"};
  return xoshiro_next(&cur_->rng[4 * stream]);
}

int GameplayHost::setting(const char* key) const {
  if (!cur_) throw rt::StopRun{"gameplay mod: setting() outside a callback"};
  auto it = cur_->settings.find(key);
  if (it == cur_->settings.end()) throw rt::StopRun{std::string("gameplay mod: no setting '") + key + "'"};
  return it->second;
}

void* GameplayHost::state() {
  if (!cur_) throw rt::StopRun{"gameplay mod: state() outside a callback"};
  return cur_->state.data();
}

const rt::FrameInput& GameplayHost::input() const {
  static const rt::FrameInput none = rt::neutral_input(0);
  return rt::g_input ? rt::g_input->current() : none;
}

void GameplayHost::request_snapshot() {
  if (phase_ == Phase::Live) want_snapshot_ = true;
}

bool GameplayHost::request_review(const Review& r) {
  if (phase_ != Phase::Live || want_review_ || in_review_ || !cur_) return false;
  if (earliest_snapshot_ == ~0ull || r.present_from < earliest_snapshot_ || r.present_from > frame_ ||
      r.present_to < r.present_from)
    return false;
  requested_ = r;
  requested_mod_ = static_cast<std::size_t>(cur_ - active_.data());
  want_review_ = true;
  return true;
}

void GameplayHost::end_review(ppc::Cpu& c) {
  phase_ = Phase::Live;
  if (!in_review_) return;
  in_review_ = false;
  Active& a = active_[active_mod_];
  cur_ = &a;
  a.mod->review_done(*this, c, review_);
  cur_ = nullptr;
}

void GameplayHost::notify(const std::string& text) {
  if (notify_) notify_(text);
  else std::printf("notify: %s\n", text.c_str());
}

void GameplayHost::publish(const std::string& json) {
  if (publish_) publish_(json);
  else std::printf("ui: %s\n", json.c_str());
}

void GameplayHost::log(const std::string& line) {
  std::string s = std::string("mod ") + (cur_ ? cur_->mod->info().id : "?") + ": " + line;
  if (log_) log_(s);
  else std::printf("%s\n", s.c_str());
}

}  // namespace mvp::mods
