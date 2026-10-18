// mvp-web (E2): the engine in the player's browser. Emscripten builds this into
// mvp-web.js + mvp-web.wasm, which the web shell's Worker loads
// (engine/web/src/worker.ts). Everything game-derived stays inside that
// Worker: the disc is read from the File the player picked, the DOL is
// translated to WebAssembly here, and nothing is sent anywhere.
//
// The Worker calls the exports below in this order (docs/engine/WEB.md):
//   mvp_open_disc(size)          the picked image (ISO or CISO), read through
//                                Module.mvpDisc (a File) with FileReaderSync;
//                                checks GV4E69 rev 0 by the DOL's SHA-1
//   mvp_translate(text, len)     symbols.txt (repository metadata, served with
//                                the engine) -> CFG -> one wasm module ->
//                                compile -> instantiate into the runtime's table
//   mvp_boot(script, len, ...)   the machine, the hardware model, the input
//                                (an mvp-input script for tests, else live input)
//   mvp_gfx_init(sel, w, h)      asynchronous: adapter, device, the canvas the
//                                page registered as `sel`; calls
//                                Module.mvpOnGfx(ok, message) when done
//   mvp_run_fields(n)            runs n whole VI fields (rt::run returns at each
//                                field: runtime/host_yield.h)
//   mvp_present()                shows the frame the VI scans out
//   mvp_set_pad(...)             the host's controller state for the next frame polls
//   mvp_status() / mvp_guest_line()   JSON status, and the determinism line
//
// E2c adds: mvp_open_disc_url (the image from the server, web_disc.h), the
// translated module's cache (mvp_keep_module, mvp_module_blob,
// mvp_translate_cached: module_cache.h), audio (mvp_audio_*: web_audio.h) and
// read-ahead for the server's image (mvp_disc_want / mvp_disc_insert).
// E6 adds roster packs (packs_web.h: mvp_pack_*, mvp_packs_mount), mounted on the
// virtual disc between mvp_open_disc and mvp_boot.
// Gameplay mods (hook API v1, docs/mods/ABS_CHALLENGE.md): mvp_mod_enable("abs:k=v,...")
// and mvp_mod_seed before mvp_boot; mvp_mod_status() for the page (the timeline's
// phase and the mods' log lines); mvp_mod_intro(ms), the page's announcement before
// each review. They need the single-threaded engine.
// P1 times the drive model's reads on the host ("io" in mvp_status;
// mvp_io_log_enable / mvp_io_log for a per-read log).
// P2: the threaded build (mvp-web-mt, MVP_WEB_THREADS) runs the GX processor
// and the renderer on a render thread (render_thread.h) when the page asks for
// it (mvp_set_threaded before mvp_gfx_init); otherwise, and in the
// single-threaded build, they run inline (render_side.h). mvp_digest /
// mvp_digest_take: the renderer digest per VI field, to compare the two.
//
// Determinism: guest state depends only on the disc and the per-frame inputs.
// Wall time never reaches it: the shell decides *how many* fields to run per
// animation frame, never what happens inside one; the renderer only reads guest
// memory; yields are invisible to the guest.
#include <emscripten.h>
#include <emscripten/heap.h>

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "audio/mixer.h"
#include "common/sha1.h"
#include "gfx/gx_processor.h"
#include "gfx/wgpu/gpu.h"
#include "gfx/wgpu/renderer.h"
#include "gfx/wgpu/window.h"
#include "mods/gameplay/host.h"
#include "mods/gameplay/timeline.h"
#include "runtime/boot.h"
#include "runtime/hle.h"
#include "runtime/host_yield.h"
#include "runtime/hw.h"
#include "runtime/input.h"
#include "runtime/machine.h"
#include "runtime/native.h"
#include "runtime/wasm_host.h"
#include "translator/cfg.h"
#include "translator/symbols.h"
#include "translator/wasm_emit.h"
#include "vfs/disc_check.h"
#include "vfs/source.h"
#include "web/wasm/module_cache.h"
#include "web/wasm/web_audio.h"
#include "web/wasm/web_disc.h"
#include "web/wasm/net_web.h"
#include "web/wasm/packs_web.h"
#include "web/wasm/render_side.h"
#ifdef MVP_WEB_THREADS
#include "web/wasm/render_thread.h"
#endif

using namespace mvp;
using namespace mvp::rt;

// Reads the picked File synchronously (Workers only). Returns 1 on success.
EM_JS(int, mvp_js_disc_read, (double off, void* dst, double n), {
  try {
    var f = Module['mvpDisc'];
    var buf = new FileReaderSync().readAsArrayBuffer(f.slice(off, off + n));
    if (buf.byteLength != n) return 0;
    HEAPU8.set(new Uint8Array(buf), dst >>> 0);
    return 1;
  } catch (e) {
    err('mvp: disc read failed: ' + e);
    return 0;
  }
});

EM_JS(void, mvp_js_on_gfx, (int ok, const char* msg), {
  if (Module['mvpOnGfx']) Module['mvpOnGfx'](ok, UTF8ToString(msg));
});

// Loading progress for the page's status line (posted while the Worker is
// still busy: the page shows it at once).
EM_JS(void, mvp_js_progress, (const char* what, int step, int steps), {
  if (Module['mvpOnProgress']) Module['mvpOnProgress'](UTF8ToString(what), step, steps);
});

namespace {

double now_ms() { return emscripten_get_now(); }

// The player's disc image, as the File the page handed to the Worker.
class JsFileSource final : public vfs::Source {
 public:
  explicit JsFileSource(std::uint64_t size) : size_(size) {}
  std::uint64_t size() const override { return size_; }
  bool read(std::uint64_t off, void* dst, std::size_t n) override {
    if (off > size_ || n > size_ - off) return false;
    if (n == 0) return true;
    ++reads;
    bytes += n;
    if (trace) trace->push_back({off, static_cast<std::uint32_t>(n)});
    return mvp_js_disc_read(static_cast<double>(off), dst, static_cast<double>(n)) != 0;
  }
  std::uint64_t reads = 0, bytes = 0;
  // Development: every read's offset and size (mvp_disc_trace), to tune the
  // server image's block cache (block_cache.h) offline. Offsets only, no data.
  std::vector<std::pair<std::uint64_t, std::uint32_t>>* trace = nullptr;

 private:
  std::uint64_t size_;
};

// Live controller input: what the page reports when the game polls. A button
// pressed since the previous poll counts as pressed at this one even if it was
// already released: the game polls once per frame of its own, which can be
// several VI fields apart (the intro movie runs below real time), and a quick
// tap would otherwise fall between two polls. Only what each poll returns
// reaches the guest.
struct LiveInput : InputSource {
  FrameInput state = neutral_input(1);
  std::array<u16, kPorts> latched{};
  u16 last_buttons = 0;        // port 1's buttons at the last poll
  u64 polls_with_buttons = 0;  // polls that returned a pressed button on any port
  void set(int port, const PadState& p) {
    const std::size_t k = static_cast<std::size_t>(port);
    latched[k] |= static_cast<u16>(p.buttons & ~state.pad[k].buttons);
    state.pad[k] = p;
  }
  FrameInput input_for(u64) override {
    FrameInput in = state;
    for (std::size_t k = 0; k < kPorts; ++k) {
      in.pad[k].buttons |= latched[k];
      if (latched[k] & pad::kA) in.pad[k].analog_a = 255;
      if (latched[k] & pad::kB) in.pad[k].analog_b = 255;
      latched[k] = 0;
    }
    last_buttons = in.pad[0].buttons;
    for (const PadState& p : in.pad)
      if (p.buttons) {
        ++polls_with_buttons;
        break;
      }
    return in;
  }
};

// The inputs the guest consumed, as an mvp-input script (runtime/input.h), for
// tests (Q3): one line per change, at the frame poll that first saw it, against
// a start of "port 1 connected, neutral". A live run (a gamepad, the keyboard, a
// netplay peer's ports too) can then be replayed from its own log and must end in
// the same guest state: host input reaches the guest only through the frame
// poll. Host-side only; never read by the guest.
struct InputLog {
  bool on = false;
  FrameInput prev = neutral_input(1);
  std::string text = "mvp-input 1\n";
  void record(std::uint64_t frame, const FrameInput& in) {
    if (!on || text.size() > (1u << 20)) return;
    static const struct { const char* name; u16 bit; } kBits[] = {
        {"LEFT", pad::kLeft}, {"RIGHT", pad::kRight}, {"DOWN", pad::kDown}, {"UP", pad::kUp}, {"Z", pad::kZ},
        {"R", pad::kR},       {"L", pad::kL},         {"A", pad::kA},       {"B", pad::kB},   {"X", pad::kX},
        {"Y", pad::kY},       {"START", pad::kStart}};
    for (std::size_t k = 0; k < kPorts; ++k) {
      const PadState& a = prev.pad[k];
      const PadState& b = in.pad[k];
      if (a == b) continue;
      const std::string head = std::to_string(frame) + " " + std::to_string(k + 1) + " ";
      if (a.connected != b.connected) text += head + (b.connected ? "connect\n" : "disconnect\n");
      if (!b.connected) continue;  // an unplugged pad reads as zeros: nothing more to say
      std::string press, release;
      for (const auto& e : kBits) {
        if ((b.buttons & e.bit) && !(a.buttons & e.bit)) press += std::string(" ") + e.name;
        if (!(b.buttons & e.bit) && (a.buttons & e.bit)) release += std::string(" ") + e.name;
      }
      if (!press.empty()) text += head + "press" + press + "\n";
      if (!release.empty()) text += head + "release" + release + "\n";
      auto pair = [&](const char* cmd, int x, int y, int ox, int oy) {
        if (x != ox || y != oy) text += head + cmd + " " + std::to_string(x) + " " + std::to_string(y) + "\n";
      };
      pair("stick", b.stick_x, b.stick_y, a.stick_x, a.stick_y);
      pair("cstick", b.substick_x, b.substick_y, a.substick_x, a.substick_y);
      pair("trigger", b.trigger_l, b.trigger_r, a.trigger_l, a.trigger_r);
      pair("analog", b.analog_a, b.analog_b, a.analog_a, a.analog_b);
    }
    prev = in;
  }
};

struct Engine;

// The guest's DVD reads as the host serves them (P1): every read the drive
// model makes goes through here and is timed on the host clock, so a hitch from
// a slow read shows up in mvp_status ("io") and, when enabled, in a per-read log
// (mvp_io_log). Host-side only: the guest gets the same bytes at the same
// virtual time however long the host took.
class TimedSource final : public vfs::Source {
 public:
  explicit TimedSource(vfs::Source* inner) : inner_(inner) {}
  std::uint64_t size() const override { return inner_->size(); }
  bool read(std::uint64_t off, void* dst, std::size_t n) override;

  struct Stats {
    std::uint64_t reads = 0, bytes = 0;
    double ms = 0, max_ms = 0;                         // host time spent in reads
    std::uint64_t over16 = 0, over50 = 0, over100 = 0;  // reads longer than a field, 50 ms, 100 ms
  };
  Stats st;
  struct Record {
    std::uint64_t field;
    double at_ms;  // the Worker's clock (performance.now()) when the read started
    std::uint64_t off;
    std::uint32_t n;
    double ms;
  };
  std::vector<Record>* log = nullptr;

 private:
  vfs::Source* inner_;
};

// The renderer's view of the run, and the field boundary.
struct Observer : GfxObserver {
  Engine* e = nullptr;
  void gx_command(const std::uint8_t* cmd, std::size_t len) override;
  void gx_commands(const rt::GxCommand* cmds, std::size_t n) override;  // Q4: a push's commands at once
  void vi_field(std::uint64_t field, std::uint32_t xfb) override;
  void gp_sync() override;  // P2: the render thread catches up (threaded only)
  void gp_write_through(std::uint32_t addr, std::uint32_t len) override;  // Q1 (gfx/gp_memory.h)
};

struct Engine {
  std::string error;
  JsFileSource* file = nullptr;  // owned by disc (a picked File)
  web::BlockCacheSource* http = nullptr;  // owned by disc (the server's image)
  std::uint64_t want_first = 0;           // mvp_disc_want's run
  std::optional<vfs::Disc> disc;
  std::string image_kind;
  std::unique_ptr<TimedSource> timed;     // the drive model's view of the image (mvp_boot)

  // Translation.
  bool translated = false;
  double t_symbols = 0, t_cfg = 0, t_emit = 0;
  WasmLoadStats load;
  xlat::WasmEmitStats emit_stats;
  std::size_t module_bytes = 0;
  bool keep_module = false;      // keep the module for the cache (mvp_module_blob)
  bool from_cache = false;
  xlat::WasmModule kept;
  std::vector<std::uint8_t> blob;

  // The machine.
  std::unique_ptr<Machine> m;
  std::unique_ptr<Hardware> hw;
  std::unique_ptr<YieldingHooks> hooks;
  std::unique_ptr<InputQueue> input;
  std::unique_ptr<InputScript> script;
  LiveInput live;
  InputLog input_log;  // development: mvp_input_log
  Observer obs;
  std::uint64_t stop_fields = ~0ull, stop_frames = ~0ull;
  std::uint64_t frames = 0;
  bool stopped = false;
  std::string why;

  // Audio (E2c): the mixer observes the DSP; its frames wait in `audio` for the Worker.
  bool audio_on = false;
  std::unique_ptr<audio::AxMixer> mixer;
  web::PendingAudio audio;

  // Rendering: inline (rs), or on the render thread (threaded, P2).
  web::RenderSide rs;
  bool threaded = false;
  std::uint32_t xfb = 0;

  // Host time spent inside rt::run, for the frame-time numbers.
  double run_ms = 0, last_batch_ms = 0, max_field_ms = 0;
  std::uint64_t run_fields = 0;

  // Gameplay mods and their timeline (snapshots, reviews). `fields_run` counts VI
  // fields as they happen (a review's restore takes the field counter back).
  mods::GameplayHost mods;
  std::unique_ptr<mods::Timeline> timeline;
  bool hold = false;  // a review runs without presenting
  std::uint64_t fields_run = 0;
  std::vector<std::string> mod_log, mod_notes;  // lines and player messages for the page (mvp_mod_status)
  std::vector<std::string> mod_ui;              // the mods' UI records (JSON objects) for the page
  bool mod_seed_set = false;                     // mvp_mod_seed was called (netplay: overrides the session's)
  // The page's announcement of a review (mvp_mod_intro): once one is requested, nothing
  // runs for `intro_ms` of host time before it starts. Host-side only: the guest waits at
  // a frame poll, as it does for the other player online.
  double intro_ms = 0, intro_until = 0;
};
Engine g;
std::vector<TimedSource::Record> g_io_log;
bool g_io_logging = false;

bool TimedSource::read(std::uint64_t off, void* dst, std::size_t n) {
  const double t0 = now_ms();
  const bool ok = inner_->read(off, dst, n);
  const double dt = now_ms() - t0;
  ++st.reads;
  st.bytes += n;
  st.ms += dt;
  if (dt > st.max_ms) st.max_ms = dt;
  if (dt > 1000.0 / 59.94) ++st.over16;
  if (dt > 50) ++st.over50;
  if (dt > 100) ++st.over100;
  if (log) log->push_back({g.hw ? g.hw->vi_fields() : 0, t0, off, static_cast<std::uint32_t>(n), dt});
  return ok;
}

void Observer::gx_command(const std::uint8_t* cmd, std::size_t len) {
#ifdef MVP_WEB_THREADS
  if (e->threaded) {
    web::render_gx(cmd, len);
    return;
  }
#endif
  e->rs.gx(cmd, len);
}

void Observer::gx_commands(const rt::GxCommand* cmds, std::size_t n) {
#ifdef MVP_WEB_THREADS
  if (e->threaded) {
    web::render_gx_batch(cmds, n);
    return;
  }
#endif
  for (std::size_t k = 0; k < n; ++k) e->rs.gx(cmds[k].p, cmds[k].len);
}

void Observer::gp_sync() {
#ifdef MVP_WEB_THREADS
  if (e->threaded) web::render_sync();
#endif
}

// Q1: what the GP sees of memory (gfx/gp_memory.h): written without the cache,
// or written back from it by the game (rt::g_dc_flush_hook).
bool g_gp_view = true;  // development (Q1): false = the renderer reads memory as it is (mvp_gp_view)

void Observer::gp_write_through(std::uint32_t addr, std::uint32_t len) {
  if (!g_gp_view) return;
#ifdef MVP_WEB_THREADS
  if (e->threaded) {
    web::render_gp_write_through(addr, len);
    return;
  }
#endif
  e->rs.gp_write_through(addr, len);
}

void dc_flush_hook(std::uint32_t ea, std::uint32_t len) {
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    web::render_gp_flush(ea, len);
    return;
  }
#endif
  g.rs.gp_flush(ea, len);
}

void Observer::vi_field(std::uint64_t field, std::uint32_t xfb) {
  ++e->fields_run;
#ifdef MVP_WEB_THREADS
  if (e->threaded) web::render_field(field, xfb);
  else
#endif
    e->rs.vi_field(field, xfb);
  e->xfb = xfb;
  if (field >= e->stop_fields) throw StopRun{"ran " + std::to_string(e->stop_fields) + " VI fields"};
  e->hooks->request_yield();  // run() returns after this field's safe point
}

// What a review does to the page's engine (mods/gameplay/timeline.h).
struct WebTimelineTool : mods::Timeline::Tool {
  void present(bool on, mods::Phase phase) override {
    g.hold = !on;
    if (g.hw && g.mixer) g.hw->set_ax_observer(phase == mods::Phase::Live ? g.mixer.get() : nullptr);  // silent replay
  }
  void save_views(int slot) override { g.rs.save_view(slot); }
  void load_views(int slot) override { g.rs.load_view(slot); }
};
WebTimelineTool g_timeline_tool;

bool poll_gate(ppc::Cpu& c) { return g.timeline->gate(c); }

void frame_hook(ppc::Cpu& c, std::uint64_t frame) {
  g.frames = frame;
  if (g.timeline) g.timeline->after_poll(frame);
  if (g.mods.any_enabled()) g.mods.on_frame(c, frame);
  if (g.input_log.on) g.input_log.record(frame, g.input->current());
  if (frame >= g.stop_frames) throw StopRun{"ran " + std::to_string(frame) + " frames"};
  // Netplay (E5): state hash, early yield. A review's re-run polls are not hashed
  // again (both players re-run them the same way; checking resumes after it).
  if (web::net_active() && g.mods.phase() == mods::Phase::Live) web::net_after_poll(frame);
}

std::string json_escape(const std::string& s) {
  std::string o;
  for (char ch : s) {
    if (ch == '"' || ch == '\\') o += '\\', o += ch;
    else if (static_cast<unsigned char>(ch) < 0x20) o += ' ';
    else o += ch;
  }
  return o;
}

std::string g_text;  // the last string handed to JS

// Attaches or detaches the mixer (a fresh one each time: its per-voice state
// starts over). Between run batches only, never inside an AX frame.
void apply_audio() {
  if (!g.hw) return;
  if (g.audio_on && !g.mixer) {
    g.mixer = std::make_unique<audio::AxMixer>(&g.audio);
    Hardware* hw = g.hw.get();
    g.mixer->set_rate([hw] { return hw->ai_dsp_rate(); });
    g.hw->set_ax_observer(g.mixer.get());
  } else if (!g.audio_on && g.mixer) {
    g.hw->set_ax_observer(nullptr);
    g.mixer.reset();
    g.audio.clear();
  }
}

// The renderer needs both the device and the machine's memory; whichever of
// mvp_gfx_init's callback and mvp_boot comes second attaches it (inline), or
// the render thread does once it has both (threaded).
void attach_renderer() {
  if (!g.m) return;
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    if (web::render_thread_running()) web::render_memory(g.m->mem1(), Machine::kMem1Size);
    return;
  }
#endif
  g.rs.set_memory(g.m->mem1(), Machine::kMem1Size);
}

}  // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE const char* mvp_last_error() { return g.error.c_str(); }

}  // extern "C"

namespace {

// CISO or ISO, then the identity check (GV4E69 rev 0, by the DOL's SHA-1).
bool open_image(std::unique_ptr<vfs::Source> src) {
  vfs::Source* raw = src.get();
  std::unique_ptr<vfs::Source> image;
  char magic[4] = {};
  if (raw->size() >= 4 && raw->read(0, magic, 4) && std::string(magic, 4) == "CISO") {
    image = vfs::CisoSource::open(std::move(src), &g.error);
    g.image_kind = "CISO";
    if (!image) return false;
  } else {
    image = std::move(src);
    g.image_kind = "ISO";
  }
  mvp_js_progress("Checking the disc (main.dol SHA-1)", 0, 0);
  auto disc = vfs::open_disc(std::move(image), true, &g.error);
  if (!disc) return false;
  g.disc = std::move(*disc);
  web::net_set_disc(&*g.disc);    // the netplay handshake's disc identity (E5), before any pack is mounted
  web::packs_set_disc(&*g.disc);  // roster packs are mounted on it (E6, mvp_packs_mount)
  return true;
}

}  // namespace

extern "C" {

std::vector<std::pair<std::uint64_t, std::uint32_t>> g_trace;
bool g_tracing = false;

// Opens the picked image (Module.mvpDisc, `size` bytes) and checks that it is
// GV4E69 rev 0. Returns 1, or 0 with mvp_last_error() set.
EMSCRIPTEN_KEEPALIVE int mvp_open_disc(double size) {
  g.error.clear();
  auto src = std::make_unique<JsFileSource>(static_cast<std::uint64_t>(size));
  JsFileSource* raw = src.get();
  if (g_tracing) raw->trace = &g_trace;
  if (!open_image(std::move(src))) return 0;
  g.file = raw;
  return 1;
}

// Development: record the picked image's reads (call before mvp_open_disc),
// and return them as "offset size" lines, with "F field" lines where a run
// batch starts (the image's offsets and sizes, no data).
EMSCRIPTEN_KEEPALIVE void mvp_disc_trace_enable() { g_tracing = true; }
EMSCRIPTEN_KEEPALIVE const char* mvp_disc_trace() {
  g_text.clear();
  for (const auto& [off, n] : g_trace)
    g_text += (off == ~0ull ? "F " + std::to_string(n) : std::to_string(off) + " " + std::to_string(n)) + '\n';
  return g_text.c_str();
}

// Development (P1): log every guest DVD read with its host timing (call before
// mvp_boot); mvp_io_log() returns "field at_ms offset size ms" lines (at_ms on
// the Worker's performance.now() clock; offsets and sizes only, no data).
EMSCRIPTEN_KEEPALIVE void mvp_io_log_enable() { g_io_logging = true; }
EMSCRIPTEN_KEEPALIVE const char* mvp_io_log() {
  g_text.clear();
  char b[96];
  for (const auto& r : g_io_log) {
    std::snprintf(b, sizeof b, "%llu %.1f %llu %u %.2f\n", static_cast<unsigned long long>(r.field), r.at_ms,
                  static_cast<unsigned long long>(r.off), r.n, r.ms);
    g_text += b;
  }
  return g_text.c_str();
}

// The same for the server's image (Module.mvpDiscUrl, `size` bytes), read by
// Range requests through a block cache (block_cache.h): `block_kib` KiB
// blocks, at most `max_blocks` of them, runs of at most `max_run` blocks per
// request, `ahead` blocks of read-ahead (0 or less: the defaults, 256 KiB x
// 256, runs of 4, 8 ahead; ahead < 0: the default, 0: none).
EMSCRIPTEN_KEEPALIVE int mvp_open_disc_url(double size, int block_kib, int max_blocks, int max_run, int ahead) {
  g.error.clear();
  web::BlockCacheSource::Options opt;
  if (block_kib > 0) opt.block = static_cast<std::uint32_t>(block_kib) << 10;
  if (max_blocks > 0) opt.max_blocks = static_cast<std::uint32_t>(max_blocks);
  if (max_run > 0) opt.max_run = static_cast<std::uint32_t>(max_run);
  if (ahead >= 0) opt.ahead = static_cast<std::uint32_t>(ahead);
  auto src = web::open_http_disc(static_cast<std::uint64_t>(size), opt);
  web::BlockCacheSource* raw = src.get();
  if (!open_image(std::move(src))) {
    if (!web::last_http_error().empty()) g.error += " (" + web::last_http_error() + ")";
    return 0;
  }
  g.http = raw;
  return 1;
}

// Read-ahead for the server's image: the run of blocks a sequential stream
// wants next (0: none). mvp_disc_want_first() and mvp_disc_block() give the
// first block and the block size; mvp_disc_insert() hands the bytes back
// (ok = 0: the fetch failed, the blocks are released).
EMSCRIPTEN_KEEPALIVE int mvp_disc_want() {
  if (!g.http) return 0;
  const auto w = g.http->want();
  g.want_first = w.first;
  return static_cast<int>(w.count);
}
EMSCRIPTEN_KEEPALIVE double mvp_disc_want_first() { return static_cast<double>(g.want_first); }
EMSCRIPTEN_KEEPALIVE double mvp_disc_block() { return g.http ? g.http->options().block : 0; }
EMSCRIPTEN_KEEPALIVE void mvp_disc_insert(double first, int count, const std::uint8_t* data, double n, int ok) {
  if (g.http)
    g.http->insert(static_cast<std::uint64_t>(first), static_cast<std::uint32_t>(count), data,
                   static_cast<std::size_t>(n), ok != 0);
}

EMSCRIPTEN_KEEPALIVE const char* mvp_disc_info() {
  if (!g.disc) return "{}";
  g_text = "{\"game\":\"" + json_escape(g.disc->gcm.game_id()) + "\",\"revision\":" +
           std::to_string(g.disc->gcm.revision()) + ",\"title\":\"" + json_escape(g.disc->gcm.title()) +
           "\",\"dol_sha1\":\"" + g.disc->dol_sha1 + "\",\"image\":\"" + g.image_kind + "\"}";
  return g_text.c_str();
}

// Translates the DOL with the symbol map `text` (symbols.txt) into one module
// and loads it into the runtime. Returns 1, or 0 with mvp_last_error() set.
EMSCRIPTEN_KEEPALIVE int mvp_translate(const char* text, int len) {
  g.error.clear();
  if (!g.disc) {
    g.error = "no disc";
    return 0;
  }
  double t0 = now_ms();
  mvp_js_progress("Translating: reading the symbol map", 1, 5);
  std::vector<std::string> errs;
  xlat::SymbolTable syms = xlat::SymbolTable::parse(std::string(text, static_cast<std::size_t>(len)), &errs);
  if (!errs.empty()) {
    g.error = "symbols.txt: " + errs.front() + " (" + std::to_string(errs.size()) + " errors)";
    return 0;
  }
  double t1 = now_ms();
  xlat::WasmModule mod;
  {
    mvp_js_progress("Translating: finding the control flow of every function", 2, 5);
    xlat::ProgramCfg cfg = xlat::build_cfg(g.disc->dol, syms);
    double t2 = now_ms();
    mvp_js_progress("Translating: emitting WebAssembly", 3, 5);
    xlat::WasmEmitOptions opt;
#ifdef MVP_WEB_NAMES
    opt.names = true;  // profiling builds: guest symbols in the browser's profiler
#endif
#ifdef MVP_WEB_THREADS
    opt.shared_memory = true;  // P2: the threaded engine's memory is shared
#endif
    const auto& table = hle_table();
    for (std::size_t k = 0; k < table.size(); ++k)
      opt.hle.push_back({table[k].name, table[k].address, static_cast<int>(k), table[k].kind == HleKind::Before});
    if (!xlat::emit_wasm(cfg, opt, &mod, &g.error)) return 0;
    double t3 = now_ms();
    g.t_symbols = t1 - t0;
    g.t_cfg = t2 - t1;
    g.t_emit = t3 - t2;
  }  // the CFG is freed here
  g.module_bytes = mod.bytes.size();
  g.emit_stats = mod.stats;
  mvp_js_progress("Translating: compiling the module", 4, 5);
  if (!load_wasm_module(mod, &g.load, &g.error)) return 0;
  g.translated = true;
  if (g.keep_module) g.kept = std::move(mod);  // serialized later, off the critical path
  return 1;
}

// Keep the translated module after mvp_translate, for the browser's cache.
EMSCRIPTEN_KEEPALIVE void mvp_keep_module(int keep) { g.keep_module = keep != 0; }

// The kept module for the cache (module_cache.h), in two pieces the Worker
// writes one after the other: the header (its tables; mvp_module_blob returns
// its size, mvp_module_blob_ptr its address) and the wasm bytes
// (mvp_module_bytes_ptr, mvp_module_bytes_len); valid until mvp_module_release().
EMSCRIPTEN_KEEPALIVE double mvp_module_blob() {
  if (g.kept.bytes.empty()) return 0;
  g.blob = web::serialize_module_header(g.kept);
  return static_cast<double>(g.blob.size());
}
EMSCRIPTEN_KEEPALIVE const std::uint8_t* mvp_module_blob_ptr() { return g.blob.data(); }
EMSCRIPTEN_KEEPALIVE const std::uint8_t* mvp_module_bytes_ptr() { return g.kept.bytes.data(); }
EMSCRIPTEN_KEEPALIVE double mvp_module_bytes_len() { return static_cast<double>(g.kept.bytes.size()); }
EMSCRIPTEN_KEEPALIVE void mvp_module_release() {
  g.kept = xlat::WasmModule{};
  std::vector<std::uint8_t>().swap(g.blob);
}

// Loads a module from the browser's cache (a blob mvp_module_blob made for a
// disc with the same DOL, by the same engine build) instead of translating.
// Returns 1, or 0 with mvp_last_error() set (the Worker then translates).
EMSCRIPTEN_KEEPALIVE int mvp_translate_cached(const std::uint8_t* blob, double len) {
  g.error.clear();
  if (!g.disc) {
    g.error = "no disc";
    return 0;
  }
  xlat::WasmModule mod;
  const double t0 = now_ms();
  if (!web::deserialize_module(blob, static_cast<std::size_t>(len), &mod, &g.error)) return 0;
  g.t_symbols = g.t_cfg = 0;
  g.t_emit = now_ms() - t0;  // unpacking the blob stands in for the emitter
  g.module_bytes = mod.bytes.size();
  g.emit_stats = mod.stats;
  mvp_js_progress("Loading the cached translation: compiling", 4, 5);
  if (!load_wasm_module(mod, &g.load, &g.error)) return 0;
  g.translated = true;
  g.from_cache = true;
  return 1;
}

EMSCRIPTEN_KEEPALIVE const char* mvp_translate_info() {
  const auto& s = g.emit_stats;
  char b[640];
  std::snprintf(b, sizeof b,
                "{\"symbols_ms\":%.1f,\"cfg_ms\":%.1f,\"emit_ms\":%.1f,\"compile_ms\":%.1f,\"instantiate_ms\":%.1f,"
                "\"module_bytes\":%zu,\"functions\":%zu,\"wasm_functions\":%zu,\"instructions\":%zu,"
                "\"inline\":%zu,\"helper\":%zu,\"branch\":%zu,\"cached\":%s}",
                g.t_symbols, g.t_cfg, g.t_emit, g.load.compile_ms, g.load.instantiate_ms, g.module_bytes, s.functions,
                s.wasm_functions, s.instructions, s.inline_insns, s.helper_insns, s.branch_insns,
                g.from_cache ? "true" : "false");
  g_text = b;
  return g_text.c_str();
}

// Builds the machine. `script` (len > 0): an mvp-input script drives the pads
// (tests); otherwise live input from mvp_set_pad. Stops after `stop_fields` VI
// fields or at the poll of frame `stop_frames` (0: never).
EMSCRIPTEN_KEEPALIVE int mvp_boot(const char* script, int len, double stop_fields, double stop_frames) {
  g.error.clear();
  if (!g.translated) {
    g.error = "not translated";
    return 0;
  }
  // Netplay (E5): players 1 and 2 on ports 1 and 2, once the handshake is done.
  const bool net = web::net_active();
  if (net && !web::net_running()) {
    g.error = "the netplay session is not running";
    return 0;
  }
  FrameInput boot_input = neutral_input(net ? 0x3 : 1);
  g.input = std::make_unique<InputQueue>(boot_input);
  if (len > 0) {
    auto s = InputScript::parse(std::string(script, static_cast<std::size_t>(len)), &g.error, boot_input);
    if (!s) {
      g.error = "input script: " + g.error;
      return 0;
    }
    g.script = std::make_unique<InputScript>(std::move(*s));
    g.input->set_source(g.script.get());
  } else {
    g.input->set_source(&g.live);
  }
  if (stop_fields > 0) g.stop_fields = static_cast<std::uint64_t>(stop_fields);
  if (stop_frames > 0) g.stop_frames = static_cast<std::uint64_t>(stop_frames);

  g.m = std::make_unique<Machine>();
  g.m->load_dol(g.disc->dol);
  setup_boot_environment(*g.m, *g.disc);
  g.hw = std::make_unique<Hardware>(&g.m->cpu(), g.m->mem1(), Machine::kMem1Size);
  g.hw->set_wgp_state(wasm_wgp_state());  // P2: the translated code appends to the write-gather pipe itself
  set_wasm_mem1(g.m->mem1(), Machine::kMem1Size);  // Q6: and reads and writes MEM1 directly
  g.timed = std::make_unique<TimedSource>(g.disc->image.get());
  if (g_io_logging) g.timed->log = &g_io_log;
  g.hw->set_disc(g.timed.get());
  g.hooks = std::make_unique<YieldingHooks>(g.m.get(), g.hw.get());
  g.m->memory().io = g.hw.get();
  g.m->cpu().hooks = g.hooks.get();
  g.hooks->reschedule(g.m->cpu());
  g.obs.e = &g;
  g.hw->set_gfx_observer(&g.obs);
  if (net) {
    // This side's controller: the script's port for this player, or the live pad (port 1 on the page).
    InputSource* local = g.script ? static_cast<InputSource*>(g.script.get()) : &g.live;
    g.input->set_source(web::net_attach(*g.m, *g.hooks, local, g.script ? web::net_player() - 1 : 0));
  }
  g_input = g.input.get();
  g_frame_hook = &frame_hook;
  g_dc_flush_hook = g_gp_view ? &dc_flush_hook : nullptr;
  if (g.mods.any_enabled()) {
    if (g.threaded) {
      g.error = "gameplay mods need the single-threaded engine";
      return 0;
    }
    auto to_page = [](const std::string& s) {
      if (g.mod_log.size() < 256) g.mod_log.push_back(s);
    };
    if (net && !g.mod_seed_set) g.mods.set_seed(web::net_seed());  // both players draw the same numbers
    g.mods.set_log(to_page);
    g.mods.set_notify([](const std::string& s) {
      if (g.mod_notes.size() < 64) g.mod_notes.push_back(s);
    });
    g.mods.set_publish([](const std::string& s) {
      if (g.mod_ui.size() < 64) g.mod_ui.push_back(s);
    });
    if (!g.mods.start(g.m->cpu(), &g.error)) return 0;
    g.timeline = std::make_unique<mods::Timeline>(*g.m, *g.hooks, *g.hw, *g.input, g.mods, g_timeline_tool);
    g.timeline->set_log([to_page](const std::string& s) { to_page("timeline: " + s); });
    g_poll_gate = &poll_gate;
  }
  attach_renderer();
  apply_audio();
  return 1;
}

// Gameplay mods (before mvp_boot): "id" or "id:key=value,...". 0 and mvp_last_error() on a bad spec.
EMSCRIPTEN_KEEPALIVE int mvp_mod_enable(const char* spec) {
  g.error.clear();
  if (g.m) {
    g.error = "mods are chosen before mvp_boot";
    return 0;
  }
  if (g.mods.available().empty())
    for (auto& mod : mods::builtin_mods()) g.mods.add(std::move(mod));
  const std::string s = spec ? spec : "";
  const auto colon = s.find(':');
  if (!g.mods.enable(s.substr(0, colon), colon == std::string::npos ? "" : s.substr(colon + 1), &g.error)) return 0;
  web::net_set_mod_settings(g.mods.canonical_settings());  // netplay: part of the handshake (before mvp_net_begin)
  return 1;
}

// The match seed of the mods' random streams. Netplay uses the session's shared
// seed unless one is set here (tests: both pages must then pass the same).
EMSCRIPTEN_KEEPALIVE void mvp_mod_seed(double seed) {
  g.mods.set_seed(static_cast<std::uint64_t>(seed));
  g.mod_seed_set = true;
}

// How long the page announces a review before it starts (ms; 0, the default: none).
// The ABS challenge scene shows "ABS challenge" meanwhile (abs-scene.ts).
EMSCRIPTEN_KEEPALIVE void mvp_mod_intro(double ms) { g.intro_ms = ms > 0 ? ms : 0; }

// The mods' canonical settings (the netplay handshake's part), "" without mods.
EMSCRIPTEN_KEEPALIVE const char* mvp_mod_settings() {
  g_text = g.mods.canonical_settings();
  return g_text.c_str();
}

// {"phase":"live|seeking|presenting|rerunning","progress":0-1,"log":[...],"notify":[...],
// "ui":[{...}]}: the timeline's phase and how far its current review leg is, the
// mods' log lines, their messages for the player and their UI records (JSON
// objects, mods::Host::publish) since the last call.
EMSCRIPTEN_KEEPALIVE const char* mvp_mod_status() {
  static const char* kPhase[] = {"live", "seeking", "presenting", "rerunning"};
  char progress[32];
  std::snprintf(progress, sizeof progress, "%.3f", g.timeline ? g.timeline->progress() : 1.0);
  g_text = std::string("{\"phase\":\"") + kPhase[static_cast<int>(g.mods.phase())] + "\",\"progress\":" + progress +
           ",\"log\":[";
  for (std::size_t k = 0; k < g.mod_log.size(); ++k)
    g_text += (k ? ",\"" : "\"") + json_escape(g.mod_log[k]) + "\"";
  g_text += "],\"notify\":[";
  for (std::size_t k = 0; k < g.mod_notes.size(); ++k)
    g_text += (k ? ",\"" : "\"") + json_escape(g.mod_notes[k]) + "\"";
  g_text += "],\"ui\":[";
  for (std::size_t k = 0; k < g.mod_ui.size(); ++k) g_text += (k ? "," : "") + g.mod_ui[k];
  g_text += "]}";
  g.mod_log.clear();
  g.mod_notes.clear();
  g.mod_ui.clear();
  return g_text.c_str();
}

// Audio on or off (the mixer attached or not), before or after mvp_boot.
// Guest state is the same either way (E3b-MIX1).
EMSCRIPTEN_KEEPALIVE void mvp_audio_enable(int on) {
  g.audio_on = on != 0;
  apply_audio();
}

// The frames mixed since the last call (interleaved 16-bit stereo at
// mvp_audio_rate() Hz); mvp_audio_ptr() is valid until the next call.
EMSCRIPTEN_KEEPALIVE int mvp_audio_take() { return static_cast<int>(g.audio.take()); }
// Q2, the threaded engine (shared memory): the mixer writes into a ring of
// `capacity` frames (a power of two) in this module's memory as it mixes, which
// the AudioWorklet reads directly (web_audio.h); returns the ring's header
// address, 0 where the memory is not shared. mvp_audio_take() then returns 0.
EMSCRIPTEN_KEEPALIVE std::uintptr_t mvp_audio_ring(int capacity) {
#ifdef MVP_WEB_THREADS
  if (capacity < 1024 || (capacity & (capacity - 1))) return 0;
  return reinterpret_cast<std::uintptr_t>(g.audio.make_ring(static_cast<std::uint32_t>(capacity)));
#else
  (void)capacity;
  return 0;
#endif
}
EMSCRIPTEN_KEEPALIVE const std::int16_t* mvp_audio_ptr() { return g.audio.taken(); }
EMSCRIPTEN_KEEPALIVE int mvp_audio_rate() { return static_cast<int>(g.audio.rate()); }

// Starts the renderer on the canvas the page registered under `selector`
// (Module.specialHTMLTargets). Asynchronous; Module.mvpOnGfx(ok, message) runs
// when it is ready or has failed. It may start before mvp_boot (the requests
// overlap the translation). The game runs without a renderer until then (or
// for good if WebGPU is missing): rendering never changes guest state.
EMSCRIPTEN_KEEPALIVE void mvp_gfx_init(const char* selector, int width, int height) {
#ifdef MVP_WEB_THREADS
  // P2: the render thread takes the canvas (Module.mvpCanvas) and the device.
  if (g.threaded) {
    if (web::render_thread_start(selector, width, height, [](int ok, const char* msg) {
          if (!ok) g.rs.error = msg;
          mvp_js_on_gfx(ok, msg);
        })) {
      attach_renderer();  // if the machine is already there
      return;
    }
    g.threaded = false;  // no thread: render inline
  }
#endif
  std::string sel = selector;
  gfx::Gpu::create_async([sel, width, height](std::unique_ptr<gfx::Gpu> gpu, std::string err) {
    if (!gpu) {
      g.rs.error = err;
      mvp_js_on_gfx(0, err.c_str());
      return;
    }
    g.rs.gpu = std::move(gpu);
    g.rs.window = gfx::Window::open_canvas(sel.c_str(), width, height, &err);
    if (!g.rs.window || !g.rs.window->attach(*g.rs.gpu, &err)) {
      g.rs.error = err;
      g.rs.window.reset();
      mvp_js_on_gfx(0, err.c_str());
      return;
    }
    g.rs.attach();
    std::string msg = g.rs.gpu->adapter_name();
    mvp_js_on_gfx(1, msg.c_str());
  });
}

// P2: 1 in the threaded build (mvp-web-mt). mvp_set_threaded(1) before
// mvp_gfx_init runs the GX processor and the renderer on the render thread.
EMSCRIPTEN_KEEPALIVE int mvp_threads_available() {
#ifdef MVP_WEB_THREADS
  return 1;
#else
  return 0;
#endif
}
EMSCRIPTEN_KEEPALIVE void mvp_set_threaded(int on) {
#ifdef MVP_WEB_THREADS
  if (!web::render_thread_running()) g.threaded = on != 0;
#else
  (void)on;
#endif
}
EMSCRIPTEN_KEEPALIVE int mvp_threaded() { return g.threaded ? 1 : 0; }
// Development (P2): the render thread's waiting (render_thread.h, render_tune).
EMSCRIPTEN_KEEPALIVE void mvp_render_tune(double fence_spin_us, double render_spin_ms, int publish_kib,
                                          int sync_fences, int copies) {
#ifdef MVP_WEB_THREADS
  web::render_tune(fence_spin_us, render_spin_ms, publish_kib, sync_fences, copies);
#else
  (void)fence_spin_us, (void)render_spin_ms, (void)publish_kib, (void)sync_fences, (void)copies;
#endif
}

EMSCRIPTEN_KEEPALIVE void mvp_canvas_resize(int width, int height) {
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    web::render_resize(width, height);
    return;
  }
#endif
  g.rs.resize(width, height);
}

// Development (Q2): a slower machine, simulated on this one. After each VI field
// (and each inline present) the emulation spins (factor - 1) times the host time
// it worked (its waits for the render thread excluded), and so does the render
// thread after each stretch of its work; 1: off. Host time only: guest state is
// the same at any factor. (Chrome's CPU throttling through DevTools slows only
// the page's main thread, not its Workers.)
double g_host_slow = 1.0;
void host_slow_spin(double work_ms) {
  if (g_host_slow <= 1 || work_ms <= 0) return;
  const double until = now_ms() + (g_host_slow - 1) * work_ms;
  while (now_ms() < until) {
  }
}
EMSCRIPTEN_KEEPALIVE void mvp_host_slow(double factor) {
  g_host_slow = factor > 1 ? factor : 1.0;
#ifdef MVP_WEB_THREADS
  web::render_host_slow(g_host_slow);
#endif
}

// A requested review waits out the page's announcement (mvp_mod_intro): true while it
// does. The run has stopped at the frame poll for it, and nothing runs meanwhile.
bool intro_holds() {
  if (!g.timeline || !g.timeline->due() || g.mods.phase() != mods::Phase::Live || !g.mods.review_requested())
    return false;
  const double now = now_ms();
  if (g.intro_until == 0) g.intro_until = now + g.intro_ms;
  if (now < g.intro_until) return true;
  g.intro_until = 0;
  return false;
}

// Runs `n` VI fields (fewer if the run stops). Returns the fields run, or -1
// once stopped (mvp_status() says why).
EMSCRIPTEN_KEEPALIVE int mvp_run_fields(int n) {
  if (!g.m || g.stopped) return -1;
  if (intro_holds()) return 0;
  // Netplay (Q5): the next frame's input is not here yet: nothing runs, so no
  // batch starts (the render thread is not woken for nothing). A gameplay mod's
  // review replays recorded inputs instead and is not held up; one that waited
  // for its announcement starts below.
  const bool step_due = g.timeline && g.timeline->due();
  if (!step_due && web::net_active() && g.mods.phase() == mods::Phase::Live && !web::net_may_run()) return 0;
  const std::uint64_t start = g.hw->vi_fields();
  const std::uint64_t run_start = g.fields_run, want = static_cast<std::uint64_t>(n);
  if (g_tracing) g_trace.push_back({~0ull, static_cast<std::uint32_t>(start)});
  double t0 = now_ms();
#ifdef MVP_WEB_THREADS
  if (g.threaded) web::render_batch_begin();
#endif
  // The timeline's due steps; true when they start a review's replay. The batch then ends,
  // so the page paces the replay from its first frame (at half speed: worker.ts).
  auto steps = [] {
    const mods::Phase was = g.mods.phase();
    while (g.timeline->due()) g.timeline->step();
    return g.mods.phase() == mods::Phase::Presenting && was != mods::Phase::Presenting;
  };
  try {
    bool replay_starts = step_due && steps();  // the review announced meanwhile starts
    // A review's headless legs (g.hold) run unpaced, within a budget per batch.
    while (!replay_starts && (g.fields_run - run_start < want || (g.hold && now_ms() - t0 < 40))) {
      // Netplay: the next frame's input is not here yet (a review replays recorded inputs instead).
      if (web::net_active() && g.mods.phase() == mods::Phase::Live && !web::net_may_run()) break;
      double f0 = now_ms();
#ifdef MVP_WEB_THREADS
      const double waited0 = g.threaded ? web::render_waited_ms() : 0;
#endif
      rt::run(g.m->cpu());
      double dt = now_ms() - f0;
      if (g_host_slow > 1) {
        double work = dt;
#ifdef MVP_WEB_THREADS
        if (g.threaded) work -= web::render_waited_ms() - waited0;
#endif
        host_slow_spin(work);
        dt = now_ms() - f0;
      }
      if (dt > g.max_field_ms) g.max_field_ms = dt;
      if (g.timeline) {
        if (intro_holds()) break;  // the page announces the review first
        replay_starts = steps();
      }
    }
  } catch (const StopRun& s) {
    g.stopped = true;
    g.why = s.why;
    if (g.http && !g.http->error().empty()) g.why += ": " + g.http->error();
  } catch (const ppc::GuestFault& f) {
    g.stopped = true;
    g.why = std::string("guest fault: ") + f.what();
  }
  g.last_batch_ms = now_ms() - t0;
  g.run_ms += g.last_batch_ms;
  const std::uint64_t ran = g.fields_run - run_start;
  g.run_fields += ran;
  if (g.stopped) {
#ifdef MVP_WEB_THREADS
    if (g.threaded) web::render_flush();
    else
#endif
      g.rs.flush();
  }
  return static_cast<int>(ran);
}

// Shows the frame the video interface scans out (the XFB copy at the VI's
// address, else the latest copy). Returns 1 if a frame was drawn.
// Threaded (P2): the render thread shows it once it has drawn the batch; 1 once
// it has shown a frame.
EMSCRIPTEN_KEEPALIVE int mvp_present() {
  if (g.hold) return 0;  // a review runs without presenting: the canvas keeps the last frame
#ifdef MVP_WEB_THREADS
  if (g.threaded) return web::render_present(g_frame_polls) ? 1 : 0;
#endif
  const double t0 = now_ms();
  const bool shown = g.rs.present(g_frame_polls, t0);
  host_slow_spin(now_ms() - t0);
  return shown ? 1 : 0;
}

// Q5: ends a batch that made no new field (mvp_run_fields returned 0: a netplay
// run that waited at a frame poll) without presenting the same picture again.
EMSCRIPTEN_KEEPALIVE void mvp_batch_end() {
#ifdef MVP_WEB_THREADS
  if (g.threaded) web::render_batch_end();
#endif
}

// Q5 (development): 0 = the render passes are not timed on the GPU (gpuTimer=0;
// for comparisons). Call before mvp_gfx_init.
EMSCRIPTEN_KEEPALIVE void mvp_gpu_timing(int on) { web::g_gpu_timing = on != 0; }

// Q5 (development): extra GPU work at every present, in proportion to the
// picture's internal size (`gpuLoad=N`: N passes; gfx::Window::burn), to try the
// picture's Auto setting on a GPU that cannot keep up.
EMSCRIPTEN_KEEPALIVE void mvp_gpu_load(int passes) {
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    if (web::render_thread_running()) web::render_gpu_load(passes);
    return;
  }
#endif
  g.rs.set_gpu_load(passes);
}

// Development (P1): the presentation trace on or off, and its records since
// the last call as a JSON array.
EMSCRIPTEN_KEEPALIVE void mvp_trace(int on) {
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    web::render_trace(on != 0);
    return;
  }
#endif
  g.rs.set_trace(on != 0);
}
EMSCRIPTEN_KEEPALIVE const char* mvp_trace_take() {
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    if (g.stopped) web::render_sync();  // the render thread has drawn everything by now
    g_text = web::render_trace_take();
    return g_text.c_str();
  }
#endif
  g_text = g.rs.trace_take();
  return g_text.c_str();
}

// Q1: the renderer's internal resolution, 1-3 times the console's (the page's
// Picture setting); it applies from the next frame the game starts.
EMSCRIPTEN_KEEPALIVE void mvp_render_scale(int scale) {
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    if (web::render_thread_running()) web::render_scale(scale);
    return;
  }
#endif
  g.rs.set_scale(scale);
}

// Development (Q1), before mvp_boot: 0 = the renderer reads guest memory as it
// is, without the model of the CPU's data cache (gfx/gp_memory.h), for comparisons.
EMSCRIPTEN_KEEPALIVE void mvp_gp_view(int on) { g_gp_view = on != 0; }

// Development (P2): the renderer digest (gfx/digest_tap.h) on or off, and its
// "field digest draws" lines since the last call. Equal lines from the inline
// and the threaded renderer mean the render thread read the same guest memory.
EMSCRIPTEN_KEEPALIVE void mvp_digest(int on, double detail_from, double detail_to) {
  const auto from = static_cast<std::uint64_t>(detail_from), to = static_cast<std::uint64_t>(detail_to);
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    web::render_digest(on != 0, from, to);
    return;
  }
#endif
  g.rs.set_digest(on != 0, from, to);
}
EMSCRIPTEN_KEEPALIVE const char* mvp_digest_take() {
#ifdef MVP_WEB_THREADS
  if (g.threaded) {
    if (g.stopped) web::render_sync();
    g_text = web::render_digest_take();
    return g_text.c_str();
  }
#endif
  g_text = g.rs.digest_take();
  return g_text.c_str();
}

// Development (Q3): record the inputs the guest consumes (InputLog); mvp_input_log()
// returns them as an mvp-input script. Enable before mvp_boot or any time after.
EMSCRIPTEN_KEEPALIVE void mvp_input_log_enable() { g.input_log.on = true; }
EMSCRIPTEN_KEEPALIVE const char* mvp_input_log() {
  g_text = g.input_log.text;
  return g_text.c_str();
}

// The host controller state for port `port` (0-3), used from the next frame
// poll on (live input only; scripts ignore it).
EMSCRIPTEN_KEEPALIVE void mvp_set_pad(int port, int connected, int buttons, int sx, int sy, int cx, int cy, int tl,
                                      int tr) {
  if (port < 0 || port >= kPorts) return;
  PadState p;
  p.connected = connected != 0;
  p.buttons = static_cast<u16>(buttons) & pad::kAll;
  auto s8c = [](int v) { return static_cast<s8>(v < -128 ? -128 : v > 127 ? 127 : v); };
  auto u8c = [](int v) { return static_cast<u8>(v < 0 ? 0 : v > 255 ? 255 : v); };
  p.stick_x = s8c(sx);
  p.stick_y = s8c(sy);
  p.substick_x = s8c(cx);
  p.substick_y = s8c(cy);
  p.trigger_l = u8c(tl);
  p.trigger_r = u8c(tr);
  p.analog_a = (p.buttons & pad::kA) ? 255 : 0;
  p.analog_b = (p.buttons & pad::kB) ? 255 : 0;
  g.live.set(port, p);
}

EMSCRIPTEN_KEEPALIVE const char* mvp_status() {
  if (!g.m) return "{}";
  ppc::Cpu& c = g.m->cpu();
  const double guest_s = static_cast<double>(g.hooks->time_base(c)) / 40500000.0;
  std::uint64_t presents = g.rs.presents();
#ifdef MVP_WEB_THREADS
  if (g.threaded) presents = web::render_presents();
#endif
  char b[1024];
  int n = std::snprintf(
      b, sizeof b,
      "{\"fields\":%llu,\"frames\":%llu,\"instructions\":%llu,\"guest_s\":%.4f,\"run_ms\":%.1f,\"run_fields\":%llu,"
      "\"last_batch_ms\":%.2f,\"max_field_ms\":%.2f,\"yields\":%llu,\"presents\":%llu,\"stopped\":%s",
      static_cast<unsigned long long>(g.hw->vi_fields()), static_cast<unsigned long long>(g_frame_polls),
      static_cast<unsigned long long>(c.ticks), guest_s, g.run_ms, static_cast<unsigned long long>(g.run_fields),
      g.last_batch_ms, g.max_field_ms, static_cast<unsigned long long>(g.hooks->yields),
      static_cast<unsigned long long>(presents), g.stopped ? "true" : "false");
  std::string s(b, static_cast<std::size_t>(n));
  s += ",\"heap_mb\":" + std::to_string(emscripten_get_heap_size() >> 20);
  if (g.stopped) s += ",\"why\":\"" + json_escape(g.why) + "\"";
  if (g.timed) {
    const auto& io = g.timed->st;
    std::snprintf(b, sizeof b,
                  ",\"io\":{\"reads\":%llu,\"bytes\":%llu,\"ms\":%.1f,\"max_ms\":%.1f,\"over16\":%llu,\"over50\":%llu,"
                  "\"over100\":%llu}",
                  static_cast<unsigned long long>(io.reads), static_cast<unsigned long long>(io.bytes), io.ms,
                  io.max_ms, static_cast<unsigned long long>(io.over16), static_cast<unsigned long long>(io.over50),
                  static_cast<unsigned long long>(io.over100));
    s += b;
  }
  if (g.file)
    s += ",\"disc_reads\":" + std::to_string(g.file->reads) + ",\"disc_bytes\":" + std::to_string(g.file->bytes);
  if (g.http) {
    const auto& h = g.http->stats();
    std::snprintf(b, sizeof b,
                  ",\"disc_reads\":%llu,\"disc_bytes\":%llu,\"net\":{\"requests\":%llu,\"fetched\":%llu,\"hits\":%llu,"
                  "\"misses\":%llu,\"prefetched\":%llu,\"prefetch_used\":%llu,\"prefetch_wasted\":%llu,"
                  "\"blocked_ms\":%.1f}",
                  static_cast<unsigned long long>(h.reads), static_cast<unsigned long long>(h.bytes),
                  static_cast<unsigned long long>(h.requests), static_cast<unsigned long long>(h.fetched),
                  static_cast<unsigned long long>(h.hits), static_cast<unsigned long long>(h.misses),
                  static_cast<unsigned long long>(h.prefetched), static_cast<unsigned long long>(h.prefetch_used),
                  static_cast<unsigned long long>(h.prefetch_wasted), h.blocked_ms);
    s += b;
  }
  if (g.mixer) {
    const auto& ms = g.mixer->stats();
    std::snprintf(b, sizeof b,
                  ",\"audio\":{\"frames\":%llu,\"rate\":%u,\"voice_ms\":%llu,\"aux_returns\":%llu,\"nonsilent\":%llu,"
                  "\"peak\":%d,\"dropped\":%llu}",
                  static_cast<unsigned long long>(ms.frames), g.audio.rate(),
                  static_cast<unsigned long long>(ms.voice_ms),
                  static_cast<unsigned long long>(ms.aux_return_frames[0]),
                  static_cast<unsigned long long>(ms.nonsilent_frames), ms.peak,
                  static_cast<unsigned long long>(g.audio.dropped()));
    s += b;
  }
  if (g.timeline) {  // gameplay mods: snapshots and reviews, with the longest host time of each
    const auto& t = *g.timeline;
    std::snprintf(b, sizeof b,
                  ",\"timeline\":{\"snapshots\":%llu,\"snapshot_ms_max\":%.2f,\"reviews\":%llu,\"restores\":%llu,"
                  "\"restore_ms_max\":%.2f}",
                  static_cast<unsigned long long>(t.snapshots), t.snapshot_ms_max,
                  static_cast<unsigned long long>(t.reviews), static_cast<unsigned long long>(t.restores),
                  t.restore_ms_max);
    s += b;
  }
  s += ",\"wgp_fast\":" + std::to_string(wasm_wgp_state()->fast_stores);  // P2: stores the translated code gathered itself
  s += ",\"pad_last\":" + std::to_string(g.live.last_buttons) +
       ",\"pad_polls\":" + std::to_string(g.live.polls_with_buttons);
#ifdef MVP_WEB_THREADS
  if (g.threaded) s += web::render_status_json();  // the render thread's snapshot, and the ring's numbers
  else
#endif
    s += ",\"resizes\":" + std::to_string(g.rs.resizes()) + g.rs.status_json();
  s += g.threaded ? ",\"threaded\":true}" : ",\"threaded\":false}";
  g_text = s;
  return g_text.c_str();
}

// The determinism line, in mvp-play's format: instructions, time base, frame
// polls and the SHA-1 of MEM1 (compare with a native run of the same input).
EMSCRIPTEN_KEEPALIVE const char* mvp_guest_line() {
  if (!g.m) return "";
  Sha1 mem;
  mem.update(g.m->mem1(), Machine::kMem1Size);
  char b[256];
  std::snprintf(b, sizeof b, "guest: %llu instructions, time base %llu, %llu frame polls, MEM1 sha1 %s",
                static_cast<unsigned long long>(g.m->cpu().ticks),
                static_cast<unsigned long long>(g.hooks->time_base(g.m->cpu())),
                static_cast<unsigned long long>(g_frame_polls), Sha1::hex(mem.finish()).c_str());
  g_text = b;
  return g_text.c_str();
}

}  // extern "C"
