// mvp-play (E1): runs the retail game natively (translated code, as mvp-boot)
// and renders its GX command stream with WebGPU: a window shows the frame the
// video interface scans out at every VI field. For development.
//
//   mvp-play [--input SCRIPT] [--ports MASK] [--frames N] [--fields N] [--max-ticks N]
//            [--headless] [--present-every N]
//            [--capture DIR] [--capture-every N] [--capture-at F1,F2,...]
//
//   --input, --ports  controller input, as mvp-boot (runtime/input.h scripts)
//   --frames N        stop at the frame poll of frame N; --fields N: after N VI fields
//   --stop-file F     (E3c) stop at the first frame poll that is a multiple of 60 once
//                     file F exists (for long unattended runs; host-side only)
//   --hash-every N    (E3c) print the SHA-1 of MEM1, the instruction count and the time
//                     base at every Nth frame poll (determinism checkpoints)
//   --max-ticks N     instruction budget (default: none)
//   --headless        no window: render off-screen only (captures still work)
//   --no-render       no renderer at all (the guest runs alone; for the determinism check)
//   --yield           return from rt::run() at every VI field and call it again, as
//                     the browser does (runtime/host_yield.h); the guest must run
//                     the same (compare the "guest:" line with a run without it)
//   --present-every N show every Nth VI field (default 1)
//   --scale N         (Q1) internal resolution, N = 1-3 times the console's (captures too)
//   --gp-live         (Q1, comparisons) the renderer reads guest memory as it is, not as
//                     the GP sees it after the data cache's write-backs (gfx/gp_memory.h)
//   --capture DIR     write the scanned-out frame as DIR/field_N_frame_F.png (VI field N,
//                     game frame poll F, the number input scripts use) every
//                     --capture-every N fields (default 60) and at the --capture-at
//                     fields (--capture-from N: periodic captures from VI field N on);
//                     only frames that changed since the last capture
//   --draw-log FILE   one line per draw and EFB copy (state, textures) from VI field
//                     --draw-log-from N (default 0) to --draw-log-to N
//   --gx-log FILE     (E3c) every GX command with the guest PC that delivered it, from VI
//                     field --gx-log-from N to --gx-log-to N
//   --dump-textures DIR at VI field --dump-field N: every texture each draw
//                     samples, decoded from guest memory, as DIR/tex_F_D_M.png
//                     (--dump-to M: every field from N to M)
//   --perf-every N    print guest and host seconds every N VI fields (speed per stretch)
//   --profile N       sample the guest PC (a host thread); print the N busiest functions
//                     (--profile-from F: from VI field F on)
//   --host-profile N  (Windows) sample the host thread's instruction pointer; print the N
//                     busiest host functions (translated guest code as one bucket),
//                     from --profile-from on
//   --trace-entry A,B registers and memory at r3/r4 at each entry of those functions (hex
//                     addresses), from VI field --trace-from to --trace-to, to --trace-file;
//                     --trace-peek O1,O2: also the words at r3 + those (hex) offsets
//   --record-input F  (E3c) write port 1's input, as the guest received it, as an input
//                     script (with a window: the keyboard merged in; Enter START,
//                     X A, Z B, C X, V Y, arrows, I/J/K/L stick, Q L, E R, Space Z)
//   --pacing-from N   (E3c) frame pacing statistics (window) from VI field N on
//   --pace            (E3c, window) show each VI field at its own time (59.94 Hz) and
//                     present without waiting for the display (mailbox) when it can
//   --cpi X           (E3c, CLK2; experiments) CPU cycles charged per guest instruction, a
//                     decimal (1.25), instead of the default (rt::ClockConfig): changes
//                     guest timing (not the netplay default)
//   --cpi-switch F:X  (CLK2, experiments) from frame poll F on, charge X cycles (the time
//                     base goes on from where it is): a script made for one charge can
//                     reach a match, then the game runs at another
//   --perf-every N also prints the XFB copies (rendered frames), the logic ticks the
//                     frame loop ran (its tick count n per frame poll, read from the
//                     guest: 1, 2, 3), the instructions run (idle skips excluded) and
//                     the DVD commands and bytes so far
//   --threads-at F1,F2 (CLK2, diagnostics) the OS threads at those VI fields: state,
//                     priority, the queue each sleeps on, PC, LR and the stack's callers
//   --audio-wav FILE  (E3b) the AX mix as a 16-bit stereo WAV, exactly as virtual
//                     time produced it (keep it under build/; never commit it)
//   --no-audio        no playback (default: SDL3 playback when a window is open)
//   --audio-latency MS buffering between the mixer and the device (default 100)
//   --pack PATH       (E6) a roster pack (directory or .zip), repeatable, in set order:
//                     applied to the disc's tables and mounted on the virtual disc
//                     before boot (docs/MODDING.md, mods/mount.h)
//   --mod ID[:K=V,...] a gameplay mod (hook API v1, mods/gameplay/host.h), repeatable,
//                     with its settings, e.g. --mod abs:miss=20,edge=3 (docs/mods/);
//                     ABS challenge in the window: L+R+Z (Q+E+Space) after a called pitch
//   --mod-seed N      the match seed of the mods' random streams (default 1)
//   --capture-review DIR  captures of every review's replay (mods/gameplay/timeline.h),
//                     every --capture-review-every N fields (default 5)
//   --rewind-test F:N save states (runtime/save_state.h): save the machine at frame
//                     poll F, run N polls, restore, run them again; prints whether the
//                     guest state matched (scripted input only)
//   --state-save F:FILE  a save-state file (runtime/state_file.h) of frame poll F,
//                     with the mods' state and, when rendering, the renderer's GX
//                     state (save from a run that renders: --headless, not --no-render)
//   --state-label TEXT   what the state shows, for its header (the page shows it)
//   --state-load FILE    start from a save-state file: boot, stop at the first frame
//                     poll, load it; give the same --pack and --mod as the saving run.
//                     The script's frames stay absolute (the state's poll and on)
//   Save-state files are game-derived: keep them under build/ or a scratch folder.
//   --card FILE       a memory card in slot A (runtime/memcard.h): the file's 8 MiB, or a
//                     new card formatted as the game formats one when there is no file;
//                     written back once the game's writes have settled (60 fields) and
//                     at the end. Without it the slot is empty, as before. Game-derived.
//
// Determinism: the renderer and the mixer only read guest state, audio and
// window events never reach the guest (playback throttles the host, not the
// guest); with the same input script, the guest runs exactly as under
// mvp-boot. The report's "guest:" line (instructions, time base, SHA-1 of MEM1)
// is the same with and without --no-render (local test local_play_determinism).
// Captures are game-derived: they belong under build/ and are never committed.
//
// Exit status: 0 when the run ended by its budget with frames shown and no
// WebGPU errors (or with --no-render), 1 otherwise, 77 without the disc.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <SDL3/SDL_keyboard.h>

#include "audio/mixer.h"
#include "audio/sdl_out.h"
#include "audio/wav.h"
#include "common/file.h"
#include "common/sha1.h"
#include "gfx/gx_processor.h"
#include "gfx/png.h"
#include "gfx/texture_decode.h"
#include "gfx/wgpu/gpu.h"
#include "gfx/wgpu/renderer.h"
#include "gfx/wgpu/window.h"
#include "host_profiler.h"
#include "mods/gameplay/host.h"
#include "mods/gameplay/timeline.h"
#include "mods/mount.h"
#include "runtime/boot.h"
#include "runtime/hle.h"
#include "runtime/host_yield.h"
#include "runtime/hw.h"
#include "runtime/input.h"
#include "runtime/native.h"
#include "runtime/save_state.h"
#include "runtime/state_file.h"
#include "runtime/sys_hooks.h"
#include "common/state_io.h"
#include "translator/program.h"

void mvp_register_game();

using namespace mvp;
using namespace mvp::rt;

namespace {

// Logs every draw and copy, then hands it on (--draw-log).
struct DrawLog : gfx::Backend {
  gfx::Backend* next = nullptr;
  std::FILE* f = nullptr;
  const std::uint64_t* field = nullptr;
  const gfx::GxProcessor* proc = nullptr;
  std::uint64_t from = 0, to = ~0ull;
  bool on() const { return f && *field >= from && *field <= to; }
  // Texture dumps (--dump-textures).
  const std::uint8_t* mem = nullptr;
  std::uint32_t mem_size = 0;
  std::string dump_dir;
  std::uint64_t dump_field = ~0ull, dump_to = 0, dump_draw = 0;
  void dump(const gfx::DrawCall& d) {
    for (int m = 0; m < 8; ++m) {
      const gfx::TextureDesc& t = d.tex[static_cast<std::size_t>(m)];
      if (!t.valid || !gfx::texture_format_known(t.format)) continue;
      const std::uint32_t a = t.addr & 0x01FFFFFFu;
      const std::uint32_t bytes = gfx::texture_level_size(t.format, t.width, t.height);
      const std::uint8_t* src = mem + a;
      std::vector<std::uint8_t> tm;
      if (t.tmem) {  // preloaded: from TMEM
        if (!gfx::gather_tmem_texture(t.tmem, t.tmem_even, t.tmem_odd, t.format, t.width, t.height, 1, tm)) continue;
        src = tm.data();
      } else if (a > mem_size || bytes > mem_size - a) {
        continue;
      }
      std::vector<std::uint8_t> rgba(static_cast<std::size_t>(t.width) * t.height * 4);
      gfx::decode_texture(t.format, t.width, t.height, src, rgba.data(), t.tlut, t.tlut_format, t.tlut_entries);
      char name[96];
      std::snprintf(name, sizeof name, "/tex_%llu_%llu_%d.png", static_cast<unsigned long long>(*field),
                    static_cast<unsigned long long>(dump_draw), m);
      gfx::write_png_rgba(dump_dir + name, t.width, t.height, rgba.data());
    }
    ++dump_draw;
  }
  void draw(const gfx::DrawCall& d) override {
    if (on()) {
      const gfx::PipelineConfig& c = d.cfg;
      std::fprintf(f, "field %llu draw %zu verts %zu indices topo %u stages %u texgens %u chans %u ind %u blend %u/%u/%u/%u z %u/%u/%u "
                   "cull %u fmt %u acmp %u/%u/%u ref %d/%d logic %u/%u fog %u scissor %u,%u-%u,%u\n",
                   static_cast<unsigned long long>(*field), d.verts.size(), d.index.size(), c.topology, c.num_stages, c.num_texgens,
                   c.num_chans, c.num_ind, c.blend_enable, c.blend_src, c.blend_dst, c.blend_sub, c.z_enable, c.z_func,
                   c.z_update, c.cull, c.pixel_format, c.alpha_comp0, c.alpha_comp1, c.alpha_logic, d.u.alpha_ref[0],
                   d.u.alpha_ref[1], c.logic_enable, c.logic_op, c.fog_type, d.scissor[0], d.scissor[1], d.scissor[2],
                   d.scissor[3]);
      if (proc) {
        const auto& pi = proc->last_primitive();
        std::fprintf(f, "  prim %02X x%u size %u vcd %08X %08X vat %08X %08X %08X matidx %08X %08X texgen %08X %08X n %u dual %u vtx",
                     pi.op, pi.count, pi.vertex_size, pi.vcd_lo, pi.vcd_hi, pi.vat[0], pi.vat[1], pi.vat[2],
                     pi.matidx[0], pi.matidx[1], pi.texgen[0], pi.texgen[1], pi.num_texgens, pi.dual_tex);
        for (auto b : pi.first_vertex) std::fprintf(f, " %02X", b);
        std::fprintf(f, "\n  arrays");
        for (int k = 0; k < 6; ++k) std::fprintf(f, " %08X/%u", pi.array_base[k], pi.array_stride[k]);
        std::fprintf(f, "\n");
      }
      for (int i = 0; i < c.num_stages; ++i) {
        const gfx::TevStage& t = c.stages[i];
        std::fprintf(f, "  stage %d color %06X alpha %06X ind %06X tex %u map %u coord %u ras %u kc %u ka %u\n", i,
                     t.color_env, t.alpha_env, t.ind_cmd, t.tex_enable, t.texmap, t.texcoord, t.ras_chan, t.kcsel,
                     t.kasel);
      }
      for (int i = 0; i < c.num_ind; ++i)
        std::fprintf(f, "  ind %d map %u coord %u scale %02X\n", i, c.iref_map[i], c.iref_coord[i], c.ind_scale[i]);
      for (int m = 0; m < 8; ++m) {
        const gfx::TextureDesc& t = d.tex[static_cast<std::size_t>(m)];
        if (!t.valid) continue;
        std::fprintf(f, "  map %d addr %08X fmt %u %ux%u levels %u wrap %u/%u filter %u/%u tlut %u@%X", m, t.addr,
                     t.format, t.width, t.height, t.levels, t.wrap_s, t.wrap_t, t.mag_linear, t.min_filter,
                     t.tlut_format, t.tlut_offset);
        if (t.tmem) std::fprintf(f, " tmem %05X/%05X", t.tmem_even, t.tmem_odd);
        std::fprintf(f, "\n");
      }
      for (int r = 0; r < 4; ++r)
        std::fprintf(f, "  reg%d %d %d %d %d konst %d %d %d %d\n", r, d.u.tev_regs[r][0], d.u.tev_regs[r][1],
                     d.u.tev_regs[r][2], d.u.tev_regs[r][3], d.u.konst[r][0], d.u.konst[r][1], d.u.konst[r][2],
                     d.u.konst[r][3]);
      for (std::size_t k = 0; k < d.verts.size() && k < 4; ++k) {
        const gfx::OutVertex& v = d.verts[k];
        std::fprintf(f, "  v%zu pos %.3f %.3f %.3f %.3f c0 %.3f %.3f %.3f %.3f t0 %.3f %.3f %.3f t1 %.3f %.3f %.3f\n", k,
                     v.pos[0], v.pos[1], v.pos[2], v.pos[3], v.color[0][0], v.color[0][1], v.color[0][2],
                     v.color[0][3], v.tex[0][0], v.tex[0][1], v.tex[0][2], v.tex[1][0], v.tex[1][1], v.tex[1][2]);
      }
    }
    if (!dump_dir.empty() && *field >= dump_field && *field <= std::max(dump_field, dump_to)) dump(d);
    next->draw(d);
  }
  void efb_copy(const gfx::EfbCopy& c) override {
    if (on())
      std::fprintf(f, "field %llu copy %s %u,%u %ux%u -> %08X fmt %u%s half %u clear %u rgba %02X%02X%02X%02X z %06X stride %u\n",
                   static_cast<unsigned long long>(*field), c.to_xfb ? "xfb" : "tex", c.src_x, c.src_y, c.width,
                   c.height, c.dest, c.tex_format, c.intensity ? "i" : "", c.half_scale, c.clear, c.clear_rgba[0],
                   c.clear_rgba[1], c.clear_rgba[2], c.clear_rgba[3], c.clear_z, c.dest_stride);
    next->efb_copy(c);
  }
  void invalidate_textures() override { next->invalidate_textures(); }
};

// Port 1 from the keyboard in the window (E3c), mapped as the browser maps it
// (web/src/input.ts): Enter = START, X = A, Z = B, C = X, V = Y, arrows = D-pad,
// I/J/K/L = main stick, Q = L, E = R, Space = Z. Read at each VI field after the
// window's events; it reaches the guest only through the frame poll.
PadState keyboard_pad() {
  int n = 0;
  const bool* k = SDL_GetKeyboardState(&n);
  auto on = [&](SDL_Scancode s) { return k && static_cast<int>(s) < n && k[s]; };
  PadState p;
  p.connected = true;
  const struct {
    SDL_Scancode key;
    u16 bit;
  } map[] = {{SDL_SCANCODE_RETURN, pad::kStart}, {SDL_SCANCODE_X, pad::kA},     {SDL_SCANCODE_Z, pad::kB},
             {SDL_SCANCODE_C, pad::kX},          {SDL_SCANCODE_V, pad::kY},     {SDL_SCANCODE_UP, pad::kUp},
             {SDL_SCANCODE_DOWN, pad::kDown},    {SDL_SCANCODE_LEFT, pad::kLeft}, {SDL_SCANCODE_RIGHT, pad::kRight},
             {SDL_SCANCODE_Q, pad::kL},          {SDL_SCANCODE_E, pad::kR},     {SDL_SCANCODE_SPACE, pad::kZ}};
  for (const auto& m : map)
    if (on(m.key)) p.buttons = static_cast<u16>(p.buttons | m.bit);
  int sx = 0, sy = 0;
  if (on(SDL_SCANCODE_I)) sy += 100;
  if (on(SDL_SCANCODE_K)) sy -= 100;
  if (on(SDL_SCANCODE_J)) sx -= 100;
  if (on(SDL_SCANCODE_L)) sx += 100;
  p.stick_x = static_cast<s8>(sx);
  p.stick_y = static_cast<s8>(sy);
  if (p.buttons & pad::kL) p.trigger_l = 255;
  if (p.buttons & pad::kR) p.trigger_r = 255;
  return p;
}

// The script's input with the keyboard merged into port 1 (window mode), and
// optionally every change of port 1 written as an input script (--record-input),
// so a window session replays headless.
struct HostInputSource : InputSource {
  InputSource* base = nullptr;
  bool keyboard = false;
  PadState host;  // the latest keyboard state
  std::FILE* rec = nullptr;
  PadState last;
  bool have_last = false;
  FrameInput input_for(u64 frame) override {
    FrameInput in = base->input_for(frame);
    PadState& p = in.pad[0];
    if (keyboard && p.connected) {
      p.buttons = static_cast<u16>(p.buttons | host.buttons);
      p.stick_x = static_cast<s8>(std::clamp(p.stick_x + host.stick_x, -128, 127));
      p.stick_y = static_cast<s8>(std::clamp(p.stick_y + host.stick_y, -128, 127));
      p.trigger_l = std::max(p.trigger_l, host.trigger_l);
      p.trigger_r = std::max(p.trigger_r, host.trigger_r);
    }
    if (rec) record(frame, p);
    return in;
  }
  void record(u64 frame, const PadState& p) {
    static const char* names[] = {"LEFT", "RIGHT", "DOWN", "UP", "Z", "R", "L", "", "A", "B", "X", "Y", "START"};
    const PadState prev = have_last ? last : PadState{true};
    const unsigned long long f = static_cast<unsigned long long>(frame);
    for (int b = 0; b < 13; ++b) {
      if (!names[b][0]) continue;
      const bool now = p.buttons >> b & 1, was = prev.buttons >> b & 1;
      if (now != was) std::fprintf(rec, "%llu 1 %s %s\n", f, now ? "press" : "release", names[b]);
    }
    if (p.stick_x != prev.stick_x || p.stick_y != prev.stick_y)
      std::fprintf(rec, "%llu 1 stick %d %d\n", f, p.stick_x, p.stick_y);
    if (p.trigger_l != prev.trigger_l || p.trigger_r != prev.trigger_r)
      std::fprintf(rec, "%llu 1 trigger %u %u\n", f, p.trigger_l, p.trigger_r);
    last = p;
    have_last = true;
  }
};

struct Play : GfxObserver {
  YieldingHooks* yield_hooks = nullptr;  // --yield
  // Gameplay mods' reviews (mods/gameplay/timeline.h): `hold` while one runs
  // without presenting (nothing shown or captured, no pacing); `replay` while
  // its replay is shown (captured to --capture-review).
  bool hold = false, replay = false;
  std::string review_capture_dir;
  std::uint64_t review_capture_every = 5, review_captures = 0, replays = 0;
  gfx::GxProcessor* proc = nullptr;
  gfx::Renderer* renderer = nullptr;
  gfx::Window* window = nullptr;
  audio::SdlAudioOut* audio_out = nullptr;
  std::uint64_t present_every = 1, max_fields = ~0ull;
  std::string capture_dir;
  std::string stop_file;  // --stop-file
  HostInputSource* host_input = nullptr;  // window mode: the keyboard on port 1
  // Frame pacing (window mode): host time between presents.
  std::vector<double> present_ms;
  std::uint64_t pacing_from = 0;  // --pacing-from
  // --card: the memory card's file (in `hw`), written back once the game's writes settle (60 VI fields without
  // another).
  std::string card_path;
  std::uint64_t card_saved = 0, card_seen = 0, card_seen_field = 0;
  void save_card(std::uint64_t field, bool now) {
    const rt::MemoryCard* c = hw ? hw->card() : nullptr;
    if (!c || card_path.empty()) return;
    if (c->writes() != card_seen) {
      card_seen = c->writes();
      card_seen_field = field;
    }
    if (card_seen == card_saved || (!now && field < card_seen_field + 60)) return;
    const std::string tmp = card_path + ".tmp";
    if (!write_file(tmp, c->image().data(), c->image().size())) {
      std::fprintf(stderr, "mvp-play: cannot write %s\n", tmp.c_str());
      return;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, card_path, ec);
    if (ec) std::fprintf(stderr, "mvp-play: cannot replace %s: %s\n", card_path.c_str(), ec.message().c_str());
    else std::printf("card: wrote %s at field %llu (%llu writes)\n", card_path.c_str(),
                     static_cast<unsigned long long>(field), static_cast<unsigned long long>(card_seen));
    card_saved = card_seen;
  }
  double wait_s = 0, present_s = 0;  // host time waiting for the schedule, and in window events + present
  // --pace (window): each VI field is shown at its own time, 1001/60000 s apart
  // (59.94 Hz), measured from the first field shown; a field that is late runs
  // at once, and one more than 100 ms late restarts the schedule. Host only.
  bool pace = false;
  bool paced = false;
  std::uint64_t pace_field0 = 0, late_fields = 0, restarts = 0, overslept = 0;
  double overslept_s = 0;
  std::chrono::steady_clock::time_point pace_t0;
#ifdef _WIN32
  HANDLE timer = nullptr;
#endif
  void pace_field(std::uint64_t field) {
    using clock = std::chrono::steady_clock;
    const auto now = clock::now();
    if (!paced) {
      paced = true;
      pace_field0 = field;
      pace_t0 = now;
      return;
    }
    const auto due = pace_t0 + std::chrono::nanoseconds((field - pace_field0) * 1001000000000ull / 60000ull);
    if (now > due + std::chrono::milliseconds(100)) {
      ++restarts;
      pace_field0 = field;
      pace_t0 = now;
      return;
    }
    if (now > due) {
      ++late_fields;
      return;
    }
    // Sleep to about a millisecond before, then spin. On Windows a
    // high-resolution waitable timer: Windows 11 ignores timeBeginPeriod for a
    // window that is occluded, and a plain sleep then oversleeps by up to 15 ms.
    const auto wake = due - std::chrono::microseconds(1500);
    if (now < wake) {
#ifdef _WIN32
      if (!timer) timer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
      if (timer) {
        LARGE_INTEGER rel;
        rel.QuadPart = -std::chrono::duration_cast<std::chrono::nanoseconds>(wake - now).count() / 100;
        if (SetWaitableTimer(timer, &rel, 0, nullptr, nullptr, FALSE)) WaitForSingleObject(timer, INFINITE);
      } else {
        std::this_thread::sleep_until(wake);
      }
#else
      std::this_thread::sleep_until(wake);
#endif
      const auto woke = clock::now();
      if (woke > due) {
        ++overslept;
        overslept_s += std::chrono::duration<double>(woke - due).count();
      }
    }
    while (clock::now() < due) std::this_thread::yield();
  }
  std::chrono::steady_clock::time_point last_present;
  std::uint64_t hash_every = 0;  // --hash-every
  const std::uint8_t* mem1 = nullptr;
  std::uint32_t mem1_size = 0;
  gfx::GpMemory* gp_memory = nullptr;  // Q1: what the GP reads (null: --gp-live)
  std::uint64_t capture_every = 60, capture_from = 0;
  std::set<std::uint64_t> capture_at;
  std::uint64_t last_capture_serial = 0, captures = 0, fields = 0, presents = 0;
  std::uint64_t frames = 0, max_frames = ~0ull;
  // --perf-every N: host and guest seconds every N fields (speed per stretch).
  std::uint64_t perf_every = 0;
  std::chrono::steady_clock::time_point t0;
  rt::SystemHooks* hooks = nullptr;
  const rt::Hardware* hw = nullptr;
  ppc::Cpu* cpu = nullptr;

  // CLK2 statistics (host side, read only): XFB copies (BP 0x52 with bit 14 set:
  // a rendered frame) and the frame loop's tick count per frame poll
  // (lbl_8061B884, DETERMINISM §2.4: n of the frame before the poll).
  std::uint64_t xfb_copies = 0;
  std::uint64_t tick_frames[4] = {};  // frame polls whose previous frame ran n = 0 (none yet), 1, 2, 3 ticks
  void perf(std::uint64_t field) {
    if (!perf_every || field % perf_every) return;
    const double host = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    const double guest = static_cast<double>(hooks->time_base(*cpu)) / 40500000.0;
    std::printf("perf: field %llu frame %llu guest %.3f s host %.3f s copies %llu n1 %llu n2 %llu n3 %llu run %llu "
                "dvd %llu %llu\n",
                static_cast<unsigned long long>(field), static_cast<unsigned long long>(frames), guest, host,
                static_cast<unsigned long long>(xfb_copies), static_cast<unsigned long long>(tick_frames[1]),
                static_cast<unsigned long long>(tick_frames[2]), static_cast<unsigned long long>(tick_frames[3]),
                static_cast<unsigned long long>(cpu->ticks - hooks->idle_ticks_skipped),
                static_cast<unsigned long long>(hw ? hw->di_commands() : 0),
                static_cast<unsigned long long>(hw ? hw->di_bytes() : 0));
    std::fflush(stdout);  // long runs are watched while they run
  }

  // Without a renderer (--no-render) the observer only counts fields.
  void gx_command(const std::uint8_t* cmd, std::size_t len) override {
    if (len >= 5 && cmd[0] == 0x61 && cmd[1] == 0x52 && (cmd[3] & 0x40)) ++xfb_copies;  // bit 14 of the BP value
    if (gx_log && fields >= gx_log_from && fields <= gx_log_to) log_command(cmd, len);
    if (proc) proc->command(cmd, len);
  }
  // --gx-log FILE (E3c): every GX command the GP executes, with the guest PC of
  // the store that delivered it, from VI field --gx-log-from to --gx-log-to.
  std::FILE* gx_log = nullptr;
  std::uint64_t gx_log_from = 0, gx_log_to = ~0ull;
  void log_command(const std::uint8_t* c, std::size_t len) {
    auto be32 = [](const std::uint8_t* p) {
      return std::uint32_t(p[0]) << 24 | std::uint32_t(p[1]) << 16 | std::uint32_t(p[2]) << 8 | p[3];
    };
    std::fprintf(gx_log, "%llu pc %08X ", static_cast<unsigned long long>(fields), cpu ? cpu->pc : 0u);
    const std::uint8_t op = c[0];
    if (op == 0x61 && len >= 5) std::fprintf(gx_log, "BP %02X %06X", c[1], be32(c + 1) & 0xFFFFFF);
    else if (op == 0x08 && len >= 6) std::fprintf(gx_log, "CP %02X %08X", c[1], be32(c + 2));
    else if (op == 0x10 && len >= 5) {
      const std::uint32_t w = be32(c + 1);
      std::fprintf(gx_log, "XF %04X x%u", w & 0xFFFF, (w >> 16) + 1);
      for (std::size_t k = 0; k < ((w >> 16) + 1) && k < 4 && 5 + 4 * k + 4 <= len; ++k)
        std::fprintf(gx_log, " %08X", be32(c + 5 + 4 * k));
    } else if (op >= 0x20 && op < 0x40 && len >= 5) std::fprintf(gx_log, "XF indexed %02X %08X", op, be32(c + 1));
    else if (op >= 0x80 && op < 0xC0 && len >= 3) {
      std::fprintf(gx_log, "DRAW %02X x%u", op, std::uint32_t(c[1]) << 8 | c[2]);
      for (std::size_t k = 3; k < len && k < 3 + 24; ++k) std::fprintf(gx_log, " %02X", c[k]);
    } else std::fprintf(gx_log, "op %02X len %zu", op, len);
    std::fprintf(gx_log, "\n");
  }

  // --threads-at F1,F2,... (CLK2, diagnostics): the OS's threads at those VI fields.
  std::set<std::uint64_t> threads_at;
  const xlat::SymbolTable* syms = nullptr;
  std::uint32_t rd32(std::uint32_t a) const {
    if ((a & 0xFE000000u) != 0x80000000u || (a & 0x01FFFFFFu) + 4 > mem1_size) return 0;
    const std::uint8_t* p = mem1 + (a & 0x01FFFFFFu);
    return std::uint32_t(p[0]) << 24 | std::uint32_t(p[1]) << 16 | std::uint32_t(p[2]) << 8 | p[3];
  }
  std::string where(std::uint32_t a) const {
    const xlat::Symbol* f = syms ? syms->function_containing(a) : nullptr;
    char b[160];
    if (f) std::snprintf(b, sizeof b, "%08X %s+0x%X", a, f->name.c_str(), a - f->address);
    else std::snprintf(b, sizeof b, "%08X", a);
    return b;
  }
  void dump_threads(std::uint64_t field) const {
    // Dolphin OS: the active-thread queue at 0x800000DC, the current thread at 0x800000E4;
    // OSThread: context (gpr at 0, lr 0x84, srr0 0x198), state 0x2C8 (u16), priority 0x2D0,
    // queue 0x2DC (the queue it sleeps on), linkActive.next 0x2FC.
    const std::uint32_t cur = rd32(0x800000E4);
    std::printf("threads at field %llu (current %08X):\n", static_cast<unsigned long long>(field), cur);
    int n = 0;
    for (std::uint32_t t = rd32(0x800000DC); t && n < 16; t = rd32(t + 0x2FC), ++n) {
      const bool running = t == cur;
      const std::uint32_t state = rd32(t + 0x2C8) >> 16, prio = rd32(t + 0x2D0), queue = rd32(t + 0x2DC);
      const std::uint32_t pc = running ? cpu->pc : rd32(t + 0x198), lr = running ? cpu->lr : rd32(t + 0x84);
      std::uint32_t sp = running ? cpu->gpr[1] : rd32(t + 4);
      std::printf("  thread %08X state %u prio %u queue %08X%s\n    pc %s\n    lr %s\n", t, state, prio, queue,
                  running ? " (running)" : "", where(pc).c_str(), where(lr).c_str());
      for (int k = 0; k < 24 && sp; ++k) {  // the back chain: each frame's caller saved its LR at [back chain + 4]
        const std::uint32_t up = rd32(sp);
        if (!up || up <= sp) break;
        std::printf("    <- %s\n", where(rd32(up + 4)).c_str());
        sp = up;
      }
    }
    std::fflush(stdout);
  }

  void gp_write_through(std::uint32_t addr, std::uint32_t len) override {
    if (gp_memory) gp_memory->write_through(addr, len);
  }

  void vi_field(std::uint64_t field, std::uint32_t xfb) override {
    fields = field;
    save_card(field, false);
    if (gp_memory) gp_memory->field();
    perf(field);
    if (threads_at.count(field)) dump_threads(field);
    if (yield_hooks) yield_hooks->request_yield();
    if (!renderer) {
      if (field >= max_fields) throw StopRun{"ran " + std::to_string(max_fields) + " VI fields (--fields)"};
      return;
    }
    renderer->flush();
    renderer->scan_field(xfb);  // P1: what the VI scans out (the window shows scanout())
    const gfx::Renderer::Frame* f = renderer->xfb(xfb);
    if (!f) f = renderer->latest_xfb();
    if (replay && !review_capture_dir.empty() && f && field % review_capture_every == 0) {
      auto px = renderer->read_frame(*f);
      char name[80];
      std::snprintf(name, sizeof name, "/review%02llu_field_%06llu_frame_%06llu.png",
                    static_cast<unsigned long long>(replays), static_cast<unsigned long long>(field),
                    static_cast<unsigned long long>(frames));
      if (gfx::write_png_rgba(review_capture_dir + name, f->width, f->height, px.data())) ++review_captures;
    }
    if (hold) {
      if (window && !window->pump()) throw StopRun{"window closed"};
      return;
    }
    if (window && field % present_every == 0) {
      if (audio_out) audio_out->throttle();  // keep the host near real time (host pacing only)
      const auto c0 = std::chrono::steady_clock::now();
      if (pace) pace_field(field);
      const auto c1 = std::chrono::steady_clock::now();
      if (!window->pump()) throw StopRun{"window closed"};
      if (host_input) host_input->host = keyboard_pad();
      const gfx::Renderer::Frame* shown = renderer->scanout();
      if (!shown) shown = f;
      if (shown) {
        window->present(shown->view, shown->width, shown->height);
        const auto c2 = std::chrono::steady_clock::now();
        if (field >= pacing_from) {
          wait_s += std::chrono::duration<double>(c1 - c0).count();
          present_s += std::chrono::duration<double>(c2 - c1).count();
        }
        ++presents;
        const auto now = std::chrono::steady_clock::now();
        if (presents > 1 && field >= pacing_from)
          present_ms.push_back(std::chrono::duration<double, std::milli>(now - last_present).count());
        last_present = now;
      }
    }
    if (!replay && !capture_dir.empty() && f && f->serial != last_capture_serial &&
        ((field >= capture_from && field % capture_every == 0) || capture_at.count(field))) {
      auto px = renderer->read_frame(*f);
      char name[64];
      std::snprintf(name, sizeof name, "/field_%06llu_frame_%06llu.png", static_cast<unsigned long long>(field),
                    static_cast<unsigned long long>(frames));
      if (gfx::write_png_rgba(capture_dir + name, f->width, f->height, px.data())) ++captures;
      last_capture_serial = f->serial;
    }
    if (field >= max_fields) throw StopRun{"ran " + std::to_string(max_fields) + " VI fields (--fields)"};
  }
};
Play* g_play = nullptr;

// --cpi-switch F:X (CLK2, experiments): the charge from frame poll F on.
std::uint64_t g_cpi_switch_frame = ~0ull;
ClockConfig g_cpi_switch_to;

mods::GameplayHost* g_mods = nullptr;  // --mod: gameplay mods, null without any
mods::Timeline* g_timeline = nullptr;   // their snapshots and reviews

// --rewind-test F:N: the save-state round trip (docs/mods/ABS_CHALLENGE.md §6.5).
struct RewindTest {
  std::uint64_t at = ~0ull, n = 0;
  int stage = 0;  // 0: run to F; 1: saved, run to F+N; 2: restored, run to F+N again; 3: done
  bool due = false;
  rt::SaveState state;
  std::string first;
  Machine* m = nullptr;
  YieldingHooks* hooks = nullptr;
  Hardware* hw = nullptr;
  InputQueue* input = nullptr;
};
RewindTest g_rewind;

std::string guest_line(Machine& m, YieldingHooks& hooks) {
  Sha1 h;
  h.update(m.mem1(), Machine::kMem1Size);
  char b[160];
  std::snprintf(b, sizeof b, "instructions %llu time base %llu MEM1 sha1 %s",
                static_cast<unsigned long long>(m.cpu().ticks),
                static_cast<unsigned long long>(hooks.time_base(m.cpu())), Sha1::hex(h.finish()).c_str());
  return b;
}

// --state-save F:FILE, --state-load FILE: save-state files (runtime/state_file.h).
struct StateFiles {
  std::uint64_t save_at = ~0ull;
  std::string save_path, load_path, label;
  bool loaded = false, due = false;
  mods::Json header;  // what main() knows of the run: the mods and packs
  std::string mod_settings, pack_result_hash;
  Machine* m = nullptr;
  YieldingHooks* hooks = nullptr;
  Hardware* hw = nullptr;
  InputQueue* input = nullptr;
  gfx::GxProcessor* proc = nullptr;  // null without rendering: the file then has no GX state
  gfx::Renderer* renderer = nullptr;
  gfx::GpMemory* gp_memory = nullptr;
};
StateFiles g_states;

bool poll_gate(ppc::Cpu& c) {
  StateFiles& s = g_states;
  const std::uint64_t poll = rt::g_frame_polls;
  if (((!s.load_path.empty() && !s.loaded) || poll == s.save_at) &&
      (!g_mods || g_mods->phase() == mods::Phase::Live) && s.hooks->stop_before(c)) {
    s.due = true;
    return false;
  }
  RewindTest& t = g_rewind;
  if (t.stage < 3 && ((t.stage == 0 && poll == t.at) || (t.stage > 0 && poll == t.at + t.n)) &&
      t.hooks->stop_before(c)) {
    t.due = true;
    return false;
  }
  return g_timeline ? g_timeline->gate(c) : true;
}

// After rt::run() returned for the rewind test.
void rewind_step() {
  RewindTest& t = g_rewind;
  t.due = false;
  if (t.stage == 0) {
    const auto t0 = std::chrono::steady_clock::now();
    rt::save_state(t.state, *t.m, *t.hooks, *t.hw, t.input);
    std::printf("rewind test: saved at poll %llu in %.1f ms\n", static_cast<unsigned long long>(t.at),
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count());
    t.stage = 1;
  } else if (t.stage == 1) {
    t.first = guest_line(*t.m, *t.hooks);
    const auto t0 = std::chrono::steady_clock::now();
    rt::load_state(t.state, *t.m, *t.hooks, *t.hw, t.input);
    std::printf("rewind test: poll %llu: %s; restored poll %llu in %.1f ms\n",
                static_cast<unsigned long long>(t.at + t.n), t.first.c_str(), static_cast<unsigned long long>(t.at),
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count());
    t.stage = 2;
  } else if (t.stage == 2) {
    const std::string again = guest_line(*t.m, *t.hooks);
    std::printf("rewind test: poll %llu again: %s: %s\n", static_cast<unsigned long long>(t.at + t.n), again.c_str(),
                again == t.first ? "IDENTICAL" : "DIFFERENT");
    t.stage = 3;
  }
}

double ms_since(std::chrono::steady_clock::time_point t0) {
  return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

void state_load(StateFiles& s) {
  const auto t0 = std::chrono::steady_clock::now();
  const auto bytes = read_file(s.load_path);
  if (!bytes) throw StopRun{"--state-load: cannot read " + s.load_path};
  std::string err;
  const auto header_text = rt::state_file_header(*bytes, &err);
  if (!header_text) throw StopRun{"--state-load: " + s.load_path + ": " + err};
  const auto header = mods::parse_json(*header_text, &err);
  if (!header || !header->is_obj()) throw StopRun{"--state-load: the header is not JSON: " + err};
  auto str = [&](const mods::Json* j) { return j && j->is_str() ? j->s : std::string(); };
  const std::string mods_then = str(header->get("mod_settings"));
  const mods::Json* packs = header->get("packs");
  const std::string packs_then = str(packs ? packs->get("result_hash") : nullptr);
  // The same mods (ids and versions: the "mod.<id>=<version>" entries); their settings may
  // differ, as on the page (e.g. another umpire miss rate from the state on).
  auto mod_ids = [](const std::string& canonical) {
    std::string ids;
    for (std::size_t at = 0; at < canonical.size();) {
      std::size_t end = canonical.find(';', at);
      if (end == std::string::npos) end = canonical.size();
      const std::string entry = canonical.substr(at, end - at);
      const std::size_t eq = entry.find('=');
      if (eq != std::string::npos && entry.find('.', 4) >= eq) ids += entry + ";";  // "mod.abs=1", not "mod.abs.miss=8"
      at = end + 1;
    }
    return ids;
  };
  if (mod_ids(mods_then) != mod_ids(s.mod_settings))
    throw StopRun{"--state-load: saved with the gameplay mods \"" + mods_then + "\"; this run has \"" + s.mod_settings +
                  "\" (give the same --mod ids)"};
  if (mods_then != s.mod_settings) std::printf("state: the mods' settings differ from the saving run's: %s\n", s.mod_settings.c_str());
  if (packs_then != s.pack_result_hash)
    throw StopRun{"--state-load: saved with other roster packs (result " + (packs_then.empty() ? "none" : packs_then) +
                  "; this run " + (s.pack_result_hash.empty() ? "none" : s.pack_result_hash) + ")"};
  rt::SaveState st;
  std::vector<u8> mods_state, views;
  if (!rt::read_state_file(*bytes, *s.hw, s.input, st, mods_state, views, &err))
    throw StopRun{"--state-load: " + s.load_path + ": " + err};
  rt::prepare_machine(*s.m, st);
  rt::load_state(st, *s.m, *s.hooks, *s.hw, s.input);
  if (g_mods ? !g_mods->load_state(mods_state) : !mods_state.empty())
    throw StopRun{"--state-load: the gameplay mods' state does not fit this run's mods"};
  if (s.proc) {
    if (!views.empty()) {
      StateReader r(views);
      s.proc->read_state(r);
    } else {
      s.proc->reset();  // the saving run did not render: the game's next frames set it up again
    }
  }
  if (s.renderer) s.renderer->invalidate_textures();
  if (s.gp_memory) s.gp_memory->field();
  if (g_timeline) g_timeline->restart();
  std::printf("state: loaded %s (%s) at poll %llu in %.1f ms: %s\n", s.load_path.c_str(),
              str(header->get("label")).c_str(), static_cast<unsigned long long>(rt::g_frame_polls), ms_since(t0),
              guest_line(*s.m, *s.hooks).c_str());
}

void state_save(StateFiles& s) {
  const auto t0 = std::chrono::steady_clock::now();
  rt::SaveState st;
  rt::save_state(st, *s.m, *s.hooks, *s.hw, s.input);
  const std::vector<u8> mods_state = g_mods ? g_mods->save_state() : std::vector<u8>();
  std::vector<u8> views;
  if (s.proc) {
    StateWriter w;
    s.proc->write_state(w);
    views = std::move(w.out());
  }
  mods::Json header = s.header;
  header.set("frame_polls", mods::Json::integer(static_cast<std::int64_t>(rt::g_frame_polls)));
  header.set("vi_fields", mods::Json::integer(static_cast<std::int64_t>(s.hw->vi_fields())));
  header.set("label", mods::Json::string(s.label));
  const auto file = rt::write_state_file(mods::canonical_json(header), st, mods_state, views);
  if (!write_file(s.save_path, file.data(), file.size())) throw StopRun{"--state-save: cannot write " + s.save_path};
  std::printf("state: saved poll %llu to %s (%.1f MiB%s) in %.1f ms: %s\n", static_cast<unsigned long long>(rt::g_frame_polls),
              s.save_path.c_str(), file.size() / 1048576.0, views.empty() ? ", no GX state" : "", ms_since(t0),
              guest_line(*s.m, *s.hooks).c_str());
}

// After rt::run() returned for a save-state file. A load comes first; a save
// due at the poll loaded then happens when the gate stops again.
void state_step() {
  StateFiles& s = g_states;
  s.due = false;
  if (!s.load_path.empty() && !s.loaded) {
    s.loaded = true;
    state_load(s);
  } else if (rt::g_frame_polls == s.save_at) {
    s.save_at = ~0ull;
    state_save(s);
  }
}

// What a review does to mvp-play (mods/gameplay/timeline.h): presenting, guest
// audio, and the renderer's GX state saved and restored with the snapshots.
struct PlayTimelineTool : mods::Timeline::Tool {
  Play* play = nullptr;
  gfx::GxProcessor* proc = nullptr;
  gfx::Renderer* renderer = nullptr;
  gfx::GpMemory* gp_memory = nullptr;
  Hardware* hw = nullptr;
  ax::FrameObserver* mixer = nullptr;  // null when no audio is mixed
  std::unique_ptr<gfx::GxProcessor> views[2];
  void present(bool on, mods::Phase phase) override {
    play->hold = !on;
    play->replay = phase == mods::Phase::Presenting;
    if (play->replay) ++play->replays;
    if (mixer) hw->set_ax_observer(phase == mods::Phase::Live ? mixer : nullptr);  // the replay is silent
  }
  void save_views(int slot) override {
    if (!proc) return;
    if (views[slot]) *views[slot] = *proc;
    else views[slot] = std::make_unique<gfx::GxProcessor>(*proc);
  }
  void load_views(int slot) override {
    if (proc && views[slot]) *proc = *views[slot];
    if (renderer) renderer->invalidate_textures();
    if (gp_memory) gp_memory->field();
  }
};

void frame_hook(ppc::Cpu& c, std::uint64_t frame) {
  g_play->frames = frame;
  if (frame >= g_play->max_frames) throw StopRun{"ran " + std::to_string(frame) + " frames (--frames)"};
  if (g_timeline) g_timeline->after_poll(frame);
  if (g_mods) g_mods->on_frame(c, frame);
  {
    const std::uint8_t* n = g_play->mem1 + (0x8061B884u & 0x01FFFFFFu);  // the frame loop's tick count
    const std::uint32_t ticks = std::uint32_t(n[0]) << 24 | std::uint32_t(n[1]) << 16 | std::uint32_t(n[2]) << 8 | n[3];
    ++g_play->tick_frames[ticks < 4 ? ticks : 0];
  }
  if (frame == g_cpi_switch_frame) {
    g_play->hooks->set_clock(c, g_cpi_switch_to);
    std::printf("clock: %.3f cycles per instruction from frame %llu\n", g_cpi_switch_to.cpi_milli / 1000.0,
                static_cast<unsigned long long>(frame));
  }
  // --hash-every N: the guest state at every Nth frame poll (compare two runs line by line).
  if (g_play->hash_every && frame % g_play->hash_every == 0) {
    Sha1 h;
    h.update(g_play->mem1, g_play->mem1_size);
    std::printf("hash: frame %llu instructions %llu time base %llu MEM1 sha1 %s\n",
                static_cast<unsigned long long>(frame), static_cast<unsigned long long>(c.ticks),
                static_cast<unsigned long long>(g_play->hooks->time_base(c)), Sha1::hex(h.finish()).c_str());
    std::fflush(stdout);
  }
  // --stop-file: checked every 60 frames; the run then ends at a frame poll,
  // so `--frames` with the reported count reproduces the same stopping point.
  if (!g_play->stop_file.empty() && frame % 60 == 0 && std::filesystem::exists(g_play->stop_file))
    throw StopRun{"ran " + std::to_string(frame) + " frames (--stop-file)"};
}

// --trace-entry ADDR[,ADDR...] (debugging): at each translated entry of those
// functions, one line with the VI field, LR, MSR and r3-r8, then 128 bytes at r3
// and at r4, the words at r3 + each --trace-peek offset, r13 and the GQRs, to
// --trace-file (default stdout). Read-only.
struct EntryTrace {
  std::set<std::uint32_t> addrs;
  std::vector<std::uint32_t> peeks;
  std::FILE* f = stdout;
  const std::uint64_t* field = nullptr;
  std::uint64_t from = 0, to = ~0ull;
  std::uint64_t max_lines = 100000, lines = 0;
};
EntryTrace g_trace;

std::vector<std::uint32_t> parse_hex_list(const std::string& s) {
  std::vector<std::uint32_t> v;
  for (std::size_t at = 0; at < s.size();) {
    std::size_t comma = s.find(',', at);
    if (comma == std::string::npos) comma = s.size();
    v.push_back(static_cast<std::uint32_t>(std::strtoul(s.substr(at, comma - at).c_str(), nullptr, 16)));
    at = comma + 1;
  }
  return v;
}

void trace_hook(ppc::Cpu& c, std::uint32_t address) {
  if (!g_trace.addrs.count(address) || *g_trace.field < g_trace.from || *g_trace.field > g_trace.to) return;
  if (g_trace.lines++ >= g_trace.max_lines) return;
  std::fprintf(g_trace.f, "field %llu entry %08X lr %08X msr %08X r3 %08X r4 %08X r5 %08X r6 %08X r7 %08X r8 %08X\n",
               static_cast<unsigned long long>(*g_trace.field), address, c.lr, c.msr, c.gpr[3], c.gpr[4], c.gpr[5],
               c.gpr[6], c.gpr[7], c.gpr[8]);
  auto byte = [&](std::uint32_t a) -> unsigned {
    const std::uint8_t* p = c.mem->host(a);
    return p ? *p : 0u;
  };
  for (int r = 3; r <= 4; ++r) {
    std::fprintf(g_trace.f, "  [r%d]", r);
    for (std::uint32_t k = 0; k < 128; ++k) std::fprintf(g_trace.f, k % 32 == 0 ? "\n   %02X" : " %02X", byte(c.gpr[r] + k));
    std::fprintf(g_trace.f, "\n");
  }
  for (std::uint32_t off : g_trace.peeks) {
    const std::uint32_t a = c.gpr[3] + off;
    std::fprintf(g_trace.f, "  [r3+%X] %02X%02X%02X%02X\n", off, byte(a), byte(a + 1), byte(a + 2), byte(a + 3));
  }
  std::fprintf(g_trace.f, "  r13 %08X gqr", c.gpr[13]);
  for (int q = 0; q < 8; ++q) std::fprintf(g_trace.f, " %08X", c.spr[ppc::spr::GQR0 + q]);
  std::fprintf(g_trace.f, "\n");
}

// --profile N: a host thread samples the guest PC (c.pc, stored before every
// translated instruction) at the host timer's pace (a sleep of 1 ms: about 15 ms
// on Windows) and prints the N functions with the most samples. Read-only: a
// racy read of one u32 the guest never sees.
struct GuestProfiler {
  std::atomic<bool> stop{false};
  std::thread thread;
  std::map<std::uint32_t, std::uint64_t> pcs;
  std::uint64_t samples = 0;
  // Samples from VI field `from` on (--profile-from).
  void start(const ppc::Cpu* cpu, const std::uint64_t* field, std::uint64_t from) {
    thread = std::thread([this, cpu, field, from] {
      while (!stop.load(std::memory_order_relaxed)) {
        if (reinterpret_cast<const volatile std::uint64_t&>(*field) >= from) {
          const std::uint32_t pc = reinterpret_cast<const volatile std::uint32_t&>(cpu->pc);
          ++pcs[pc];
          ++samples;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    });
  }
  void finish(const xlat::SymbolTable& syms, int top) {
    stop = true;
    if (thread.joinable()) thread.join();
    std::map<std::string, std::uint64_t> by_fn;
    for (const auto& [pc, n] : pcs) {
      const xlat::Symbol* f = syms.function_containing(pc);
      char b[32];
      std::snprintf(b, sizeof b, "?%08X", pc & ~0xFFFu);
      by_fn[f ? f->name : std::string(b)] += n;
    }
    std::vector<std::pair<std::uint64_t, std::string>> v;
    for (const auto& [name, n] : by_fn) v.push_back({n, name});
    std::sort(v.rbegin(), v.rend());
    std::printf("profile: %llu samples\n", static_cast<unsigned long long>(samples));
    for (int k = 0; k < top && k < static_cast<int>(v.size()); ++k) {
      const auto& [n, name] = v[static_cast<std::size_t>(k)];
      std::printf("  %6.2f%%  %s\n", samples ? 100.0 * static_cast<double>(n) / static_cast<double>(samples) : 0.0,
                  name.c_str());
    }
  }
};

}  // namespace

int main(int argc, char** argv) {
  std::setvbuf(stdout, nullptr, _IOLBF, 1 << 16);
  std::string input_path, capture_at, draw_log_path;
  std::uint64_t draw_log_from = 0, draw_log_to = ~0ull, dump_field = ~0ull, dump_to = 0;
  std::string dump_dir, audio_wav, record_input;
  bool no_audio = false;
  std::uint32_t audio_latency = 100;
  std::uint32_t cpi_milli = 0;  // --cpi (0: the default, rt::ClockConfig)
  // A decimal number of cycles per instruction in thousandths (0: not a positive number).
  auto parse_cpi = [](const std::string& s) -> std::uint32_t {
    const double x = std::strtod(s.c_str(), nullptr);
    return x > 0 && x < 1000 ? static_cast<std::uint32_t>(std::lround(x * 1000)) : 0;
  };
  std::uint64_t max_ticks = ~0ull;
  u32 ports = 1;
  bool headless = false, no_render = false, yield = false, gp_live = false;
  std::uint32_t scale = 1;
  int profile_top = 0, host_profile_top = 0;
  std::uint64_t profile_from = 0;
  std::string trace_entry, trace_file;
  std::vector<std::string> packs;
  std::vector<std::string> mod_args;  // --mod ID[:K=V,...]
  std::uint64_t mod_seed = 1;
  Play play;
  for (int k = 1; k < argc; ++k) {
    std::string a = argv[k];
    auto next = [&]() -> std::string { return k + 1 < argc ? argv[++k] : std::string(); };
    if (a == "--input") input_path = next();
    else if (a == "--ports") ports = static_cast<u32>(std::strtoul(next().c_str(), nullptr, 0)) & 0xF;
    else if (a == "--frames") play.max_frames = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--fields") play.max_fields = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--max-ticks") max_ticks = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--headless") headless = true;
    else if (a == "--no-render") no_render = headless = true;
    else if (a == "--yield") yield = true;
    else if (a == "--gp-live") gp_live = true;
    else if (a == "--scale") scale = static_cast<std::uint32_t>(std::strtoul(next().c_str(), nullptr, 10));
    else if (a == "--present-every") play.present_every = std::max<std::uint64_t>(1, std::strtoull(next().c_str(), nullptr, 10));
    else if (a == "--capture") play.capture_dir = next();
    else if (a == "--capture-every") play.capture_every = std::max<std::uint64_t>(1, std::strtoull(next().c_str(), nullptr, 10));
    else if (a == "--capture-at") capture_at = next();
    else if (a == "--capture-from") play.capture_from = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--stop-file") play.stop_file = next();
    else if (a == "--record-input") record_input = next();
    else if (a == "--pacing-from") play.pacing_from = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--cpi") {
      cpi_milli = parse_cpi(next());
      if (!cpi_milli) {
        std::fprintf(stderr, "mvp-play: --cpi wants a positive number of cycles\n");
        return 2;
      }
    } else if (a == "--threads-at") {
      const std::string v = next();
      for (std::size_t at = 0; at < v.size();) {
        std::size_t comma = v.find(',', at);
        if (comma == std::string::npos) comma = v.size();
        play.threads_at.insert(std::strtoull(v.substr(at, comma - at).c_str(), nullptr, 10));
        at = comma + 1;
      }
    } else if (a == "--cpi-switch") {
      const std::string v = next();
      const std::size_t colon = v.find(':');
      g_cpi_switch_to.cpi_milli = colon == std::string::npos ? 0 : parse_cpi(v.substr(colon + 1));
      if (!g_cpi_switch_to.cpi_milli) {
        std::fprintf(stderr, "mvp-play: --cpi-switch wants FRAME:CYCLES\n");
        return 2;
      }
      g_cpi_switch_frame = std::strtoull(v.substr(0, colon).c_str(), nullptr, 10);
    }
    else if (a == "--pace") play.pace = true;
    else if (a == "--gx-log") play.gx_log = std::fopen(next().c_str(), "w");
    else if (a == "--gx-log-from") play.gx_log_from = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--gx-log-to") play.gx_log_to = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--hash-every") play.hash_every = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--draw-log") draw_log_path = next();
    else if (a == "--draw-log-from") draw_log_from = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--draw-log-to") draw_log_to = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--dump-textures") dump_dir = next();
    else if (a == "--dump-field") dump_field = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--dump-to") dump_to = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--perf-every") play.perf_every = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--profile") profile_top = std::atoi(next().c_str());
    else if (a == "--profile-from") profile_from = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--host-profile") host_profile_top = std::atoi(next().c_str());
    else if (a == "--trace-entry") trace_entry = next();
    else if (a == "--trace-file") trace_file = next();
    else if (a == "--trace-from") g_trace.from = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--trace-to") g_trace.to = std::strtoull(next().c_str(), nullptr, 10);
    else if (a == "--trace-peek") g_trace.peeks = parse_hex_list(next());
    else if (a == "--audio-wav") audio_wav = next();
    else if (a == "--no-audio") no_audio = true;
    else if (a == "--audio-latency") audio_latency = static_cast<std::uint32_t>(std::strtoul(next().c_str(), nullptr, 10));
    else if (a == "--pack") packs.push_back(next());
    else if (a == "--mod") mod_args.push_back(next());
    else if (a == "--mod-seed") mod_seed = std::strtoull(next().c_str(), nullptr, 0);
    else if (a == "--capture-review") play.review_capture_dir = next();
    else if (a == "--rewind-test") {
      const std::string v = next();
      const auto colon = v.find(':');
      g_rewind.at = std::strtoull(v.c_str(), nullptr, 10);
      g_rewind.n = colon == std::string::npos ? 60 : std::strtoull(v.c_str() + colon + 1, nullptr, 10);
    }
    else if (a == "--state-save") {
      const std::string v = next();
      const auto colon = v.find(':');
      if (colon == std::string::npos || colon + 1 >= v.size()) {
        std::fprintf(stderr, "mvp-play: --state-save wants F:FILE\n");
        return 2;
      }
      g_states.save_at = std::strtoull(v.c_str(), nullptr, 10);
      g_states.save_path = v.substr(colon + 1);
    }
    else if (a == "--state-label") g_states.label = next();
    else if (a == "--card") play.card_path = next();
    else if (a == "--state-load") g_states.load_path = next();
    else if (a == "--capture-review-every")
      play.review_capture_every = std::max<std::uint64_t>(1, std::strtoull(next().c_str(), nullptr, 10));
    else {
      std::fprintf(stderr,
                   "usage: mvp-play [--input SCRIPT] [--ports MASK] [--frames N] [--fields N] [--stop-file F] [--hash-every N] [--record-input F] [--pacing-from N] [--pace] [--cpi X] [--cpi-switch F:X] [--max-ticks N] "
                   "[--headless] [--no-render] [--yield] [--present-every N] [--capture DIR] [--capture-every N] [--capture-from N] [--capture-at F1,F2,...] "
                   "[--draw-log FILE] [--draw-log-from N] [--draw-log-to N] [--dump-textures DIR --dump-field N [--dump-to M]] "
                   "[--perf-every N] [--profile N] [--host-profile N] [--profile-from F] [--trace-entry HEX,... [--trace-file F] [--trace-from N] "
                   "[--trace-to N] [--trace-peek HEX,...]] [--audio-wav FILE] [--no-audio] [--audio-latency MS] [--pack PATH]... [--mod ID[:K=V,...]]... [--mod-seed N] [--capture-review DIR [--capture-review-every N]] [--rewind-test F:N] "
                   "[--state-save F:FILE [--state-label TEXT]] [--state-load FILE] [--card FILE]\n");
      return 2;
    }
  }
  for (std::size_t at = 0; at < capture_at.size();) {
    std::size_t comma = capture_at.find(',', at);
    if (comma == std::string::npos) comma = capture_at.size();
    play.capture_at.insert(std::strtoull(capture_at.substr(at, comma - at).c_str(), nullptr, 10));
    at = comma + 1;
  }

  std::string err;
  auto in = xlat::load_inputs(xlat::default_disc_path(), xlat::default_symbols_path(), &err);
  if (!in) {
    std::fprintf(stderr, "mvp-play: %s\n", err.c_str());
    return 77;
  }
  g_states.header = mods::Json::object();
  g_states.header.set("game", mods::Json::string("GV4E69-0"));
  g_states.header.set("saved_by", mods::Json::string("mvp-play"));
  if (!packs.empty()) {  // E6: roster packs on the virtual disc, before boot
    std::string log, set_hash;
    const auto mounted = mods::mount_pack_paths(in->disc, packs, &log, &set_hash);
    std::printf("%s", log.c_str());
    if (!mounted.ok) {
      std::fprintf(stderr, "mvp-play: the roster packs cannot be used\n");
      return 1;
    }
    g_states.pack_result_hash = mounted.result_hash;
    mods::Json p = mods::Json::object(), files = mods::Json::array();
    for (const std::string& path : packs) files.arr.push_back(mods::Json::string(std::filesystem::path(path).filename().string()));
    p.set("set_hash", mods::Json::string(set_hash));
    p.set("result_hash", mods::Json::string(mounted.result_hash));
    p.set("files", std::move(files));
    g_states.header.set("packs", std::move(p));
  }
  std::string script_text = "mvp-input 1\n";
  if (!input_path.empty()) {
    auto text = read_text(input_path);
    if (!text) {
      std::fprintf(stderr, "mvp-play: cannot read %s\n", input_path.c_str());
      return 2;
    }
    script_text = *text;
  }
  FrameInput boot_input = neutral_input(ports);
  auto script = InputScript::parse(script_text, &err, boot_input);
  if (!script) {
    std::fprintf(stderr, "mvp-play: %s: %s\n", input_path.c_str(), err.c_str());
    return 2;
  }
  InputQueue input(boot_input);
  HostInputSource host_input;
  host_input.base = &*script;
  host_input.keyboard = !headless;
  if (!record_input.empty()) {
    host_input.rec = std::fopen(record_input.c_str(), "w");
    if (!host_input.rec) {
      std::fprintf(stderr, "mvp-play: cannot write %s\n", record_input.c_str());
      return 2;
    }
    std::fprintf(host_input.rec, "mvp-input 1\n# recorded by mvp-play (port 1)\n");
  }
  input.set_source(&host_input);
  if (!headless) play.host_input = &host_input;

  // Renderer.
  std::unique_ptr<gfx::Window> window;
  if (!headless) {
    window = gfx::Window::open("MVP Baseball 2005 (mvp-play)", 1280, 960, &err);
    if (!window) {
      std::fprintf(stderr, "mvp-play: %s\n", err.c_str());
      return 1;
    }
  }
  std::unique_ptr<gfx::Gpu> gpu;
  if (!no_render) {
    gpu = gfx::Gpu::create(&err);
    if (!gpu) {
      std::fprintf(stderr, "mvp-play: %s\n", err.c_str());
      return 1;
    }
    std::printf("gpu: %s\n", gpu->adapter_name().c_str());
  }
  if (window) window->prefer_mailbox(play.pace);
  if (window && !window->attach(*gpu, &err)) {
    std::fprintf(stderr, "mvp-play: %s\n", err.c_str());
    return 1;
  }

  Machine m;
  m.load_dol(in->disc.dol);
  setup_boot_environment(m, in->disc);
  Hardware hw(&m.cpu(), m.mem1(), Machine::kMem1Size);
  hw.set_disc(in->disc.image.get());
  if (!play.card_path.empty()) {  // --card: the file's card, or a new one
    auto file = read_file(play.card_path);
    const bool have = file.has_value();
    std::vector<std::uint8_t> img = have ? std::move(*file) : std::vector<std::uint8_t>{};
    if (!have || img.size() != rt::MemoryCard::kSize) {
      const auto& s = hw.sram();  // the counter bias and the language this console formats with
      const std::uint32_t bias = std::uint32_t(s[0x0C]) << 24 | std::uint32_t(s[0x0D]) << 16 | std::uint32_t(s[0x0E]) << 8 | s[0x0F];
      img = rt::MemoryCard::formatted(bias, s[0x12], std::uint64_t(rt::HardwareConfig{}.rtc_seconds) * 40500000ull);
      if (have) std::printf("card: %s is not an 8 MiB card; a new one replaces it\n", play.card_path.c_str());
    }
    hw.insert_card(std::move(img));
    play.hw = &hw;
    std::printf("card: %s in slot A (%s)\n", play.card_path.c_str(), have ? "loaded" : "new, formatted");
  }
  ClockConfig clock;
  if (cpi_milli) clock.cpi_milli = cpi_milli;
  std::printf("clock: %.3f cycles per instruction\n", clock.cpi_milli / 1000.0);
  YieldingHooks hooks(&m, &hw, clock);
  m.memory().io = &hw;
  m.cpu().hooks = &hooks;
  hooks.tick_limit = max_ticks;
  hooks.reschedule(m.cpu());
  mvp_register_game();

  // Gameplay mods (--mod): enabled with their settings, sites bound before the first instruction.
  mods::GameplayHost gameplay;
  for (auto& mod : mods::builtin_mods()) gameplay.add(std::move(mod));
  for (const std::string& arg : mod_args) {
    const auto colon = arg.find(':');
    std::string id = arg.substr(0, colon), settings = colon == std::string::npos ? "" : arg.substr(colon + 1);
    if (!gameplay.enable(id, settings, &err)) {
      std::fprintf(stderr, "mvp-play: --mod %s: %s\n", arg.c_str(), err.c_str());
      return 2;
    }
  }
  if (gameplay.any_enabled()) {
    gameplay.set_seed(mod_seed);
    if (!gameplay.start(m.cpu(), &err)) {
      std::fprintf(stderr, "mvp-play: %s\n", err.c_str());
      return 2;
    }
    g_mods = &gameplay;
    std::printf("mods: %s seed %llu\n", gameplay.canonical_settings().c_str(),
                static_cast<unsigned long long>(mod_seed));
  }
  g_states.mod_settings = gameplay.canonical_settings();
  {
    mods::Json specs = mods::Json::array();
    for (const std::string& arg : mod_args) specs.arr.push_back(mods::Json::string(arg));
    g_states.header.set("mods", std::move(specs));
    g_states.header.set("mod_settings", mods::Json::string(g_states.mod_settings));
  }

  std::unique_ptr<gfx::Renderer> renderer_owner;
  if (gpu) renderer_owner = std::make_unique<gfx::Renderer>(*gpu, m.mem1(), Machine::kMem1Size, scale);
  gfx::Renderer* renderer = renderer_owner.get();
  // Q1: the renderer reads memory as the GP sees it (gfx/gp_memory.h): the game's
  // cache write-backs (rt::g_dc_flush_hook), DMA and the write-gather pipe.
  static gfx::GpMemory gp_memory;
  if (renderer && !gp_live) {
    gp_memory.attach(m.mem1(), Machine::kMem1Size);
    renderer->set_gp_memory(&gp_memory);
    play.gp_memory = &gp_memory;
    rt::g_dc_flush_hook = [](std::uint32_t ea, std::uint32_t len) { gp_memory.flush(ea, len); };
  }
  DrawLog draw_log;
  draw_log.next = renderer;
  draw_log.field = &play.fields;
  draw_log.from = draw_log_from;
  draw_log.to = draw_log_to;
  draw_log.mem = m.mem1();
  draw_log.mem_size = Machine::kMem1Size;
  draw_log.dump_dir = dump_dir;
  draw_log.dump_field = dump_field;
  draw_log.dump_to = dump_to;
  if (!draw_log_path.empty()) draw_log.f = std::fopen(draw_log_path.c_str(), "w");
  const bool wrap = draw_log.f || !dump_dir.empty();
  gfx::GxProcessor proc(wrap ? static_cast<gfx::Backend*>(&draw_log) : renderer);
  draw_log.proc = &proc;
  proc.set_keep_primitive_info(draw_log.f != nullptr);
  proc.set_memory(m.mem1(), Machine::kMem1Size);
  proc.set_gp_memory(play.gp_memory);
  play.proc = &proc;
  play.renderer = renderer;
  play.window = window.get();
  if (yield) play.yield_hooks = &hooks;
  if (!renderer) play.proc = nullptr;
  hw.set_gfx_observer(&play);
  g_play = &play;

  // Audio (E3b): the mixer observes the DSP's frames; it feeds a WAV, a level
  // meter and, with a window, SDL3 playback.
  audio::AxMixer mixer;
  audio::TeeSink tee;
  audio::WavWriter wav;
  audio::LevelMeter levels;
  std::unique_ptr<audio::SdlAudioOut> audio_out;
  if (!audio_wav.empty()) {
    if (!wav.open(audio_wav)) {
      std::fprintf(stderr, "mvp-play: cannot write %s\n", audio_wav.c_str());
      return 2;
    }
    tee.add(&wav);
  }
  if (window && !no_audio) {
    audio_out = audio::SdlAudioOut::open(audio::kDspRate, audio_latency, &err);
    if (audio_out) {
      std::printf("audio: SDL %s, device \"%s\" at %u Hz, %u ms buffer\n", audio_out->driver().c_str(),
                  audio_out->device().c_str(), audio_out->device_rate(), audio_latency);
      tee.add(audio_out.get());
      play.audio_out = audio_out.get();
    } else {
      std::fprintf(stderr, "mvp-play: no audio: %s\n", err.c_str());
    }
  }
  if (!audio_wav.empty() || audio_out) {
    tee.add(&levels);
    mixer.set_sink(&tee);
    mixer.set_rate([&hw] { return hw.ai_dsp_rate(); });
    hw.set_ax_observer(&mixer);
  }
  rt::g_input = &input;
  rt::g_frame_hook = &frame_hook;
  if (!trace_entry.empty()) {
    for (std::uint32_t a : parse_hex_list(trace_entry)) g_trace.addrs.insert(a);
    if (!trace_file.empty()) g_trace.f = std::fopen(trace_file.c_str(), "w");
    if (!g_trace.f) g_trace.f = stdout;
    g_trace.field = &play.fields;
    rt::g_entry_hook = &trace_hook;
  }

  const auto t0 = std::chrono::steady_clock::now();
  play.t0 = t0;
  play.hooks = &hooks;
  play.hw = &hw;
  play.syms = &in->symbols;
  play.mem1 = m.mem1();
  play.mem1_size = Machine::kMem1Size;
  play.cpu = &m.cpu();
  std::string why;
  int rc = 0;
  GuestProfiler profiler;
  if (profile_top > 0) profiler.start(&m.cpu(), &play.fields, profile_from);
  tools::HostProfiler host_profiler;
  if (host_profile_top > 0) host_profiler.start(&play.fields, profile_from);
  // Gameplay mods' snapshots and reviews: the run stops at a frame poll when one is due.
  PlayTimelineTool timeline_tool;
  timeline_tool.play = &play;
  timeline_tool.proc = play.proc;
  timeline_tool.renderer = renderer;
  timeline_tool.gp_memory = play.gp_memory;
  timeline_tool.hw = &hw;
  if (!audio_wav.empty() || audio_out) timeline_tool.mixer = &mixer;
  mods::Timeline timeline(m, hooks, hw, input, gameplay, timeline_tool);
  if (g_mods) {
    g_timeline = &timeline;
    timeline.prepare();  // V4: the slots' buffers now, not at the first pitches
  }
  g_rewind.m = &m;
  g_rewind.hooks = &hooks;
  g_rewind.hw = &hw;
  g_rewind.input = &input;
  g_states.m = &m;
  g_states.hooks = &hooks;
  g_states.hw = &hw;
  g_states.input = &input;
  g_states.proc = play.proc;
  g_states.renderer = renderer;
  g_states.gp_memory = play.gp_memory;
  const bool states = g_states.save_at != ~0ull || !g_states.load_path.empty();
  if (g_mods || g_rewind.at != ~0ull || states) rt::g_poll_gate = &poll_gate;
  try {
    for (;;) {
      rt::run(m.cpu());  // returns at each VI field with --yield, and when the timeline stops it
      if (g_states.due) state_step();
      if (g_rewind.due) rewind_step();
      if (g_timeline)
        while (g_timeline->due()) g_timeline->step();
    }
  } catch (const StopRun& s) {
    why = s.why;
  } catch (const ppc::GuestFault& f) {
    why = std::string("guest fault: ") + f.what();
    rc = 1;
  }
  play.save_card(play.fields, true);
  const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  if (profile_top > 0) profiler.finish(in->symbols, profile_top);
  if (host_profile_top > 0) host_profiler.finish(host_profile_top);
  hw.set_gfx_observer(nullptr);
  hw.set_ax_observer(nullptr);
  rt::g_input = nullptr;
  rt::g_frame_hook = nullptr;
  rt::g_poll_gate = nullptr;
  if (g_timeline)
    std::printf("timeline: %llu snapshots (longest %.1f ms, mean %.1f ms), %llu reviews, %llu restores (longest %.1f ms), "
                "%llu replay captures\n",
                static_cast<unsigned long long>(timeline.snapshots), timeline.snapshot_ms_max,
                timeline.snapshots ? timeline.snapshot_ms_total / static_cast<double>(timeline.snapshots) : 0.0,
                static_cast<unsigned long long>(timeline.reviews), static_cast<unsigned long long>(timeline.restores),
                timeline.restore_ms_max, static_cast<unsigned long long>(play.review_captures));
  g_timeline = nullptr;
  g_mods = nullptr;
  gameplay.stop();
  rt::g_dc_flush_hook = nullptr;
  rt::g_entry_hook = nullptr;
  if (g_trace.f && g_trace.f != stdout) std::fclose(g_trace.f);
  if (renderer) {
    renderer->flush();
    gpu->wait_idle();
  }
  if (draw_log.f) std::fclose(draw_log.f);
  if (play.gx_log) std::fclose(play.gx_log);
  if (host_input.rec) std::fclose(host_input.rec);

  auto ull = [](std::uint64_t v) { return static_cast<unsigned long long>(v); };
  const double guest = static_cast<double>(hooks.time_base(m.cpu())) / 40500000.0;
  std::printf("stopped: %s\n", why.c_str());
  {
    Sha1 mem;
    mem.update(m.mem1(), Machine::kMem1Size);
    std::printf("guest: %llu instructions, time base %llu, %llu frame polls, MEM1 sha1 %s\n", ull(m.cpu().ticks),
                ull(hooks.time_base(m.cpu())), ull(rt::g_frame_polls), Sha1::hex(mem.finish()).c_str());
  }
  if (yield)
    std::printf("yield: %llu host yields, %llu repeated safe points skipped\n", ull(hooks.yields),
                ull(hooks.repeats_skipped));
  std::printf("time: %.3f s of guest time in %.1f s (%.2fx real time); %llu VI fields, %llu frame polls\n", guest, secs,
              secs > 0 ? guest / secs : 0.0, ull(hw.vi_fields()), ull(play.frames));
  std::printf("runtime: %llu unwinds, %llu in-frame resumes, %llu resume entries; %llu external and %llu decrementer "
              "interrupts, %llu floating-point unavailable; idle skips %llu (%llu instructions skipped); interpreted %llu "
              "instructions\n",
              ull(rt::g_unwinds), ull(rt::g_inframe_resumes), ull(rt::g_resume_entries), ull(hooks.external_interrupts),
              ull(hooks.decrementer_interrupts), ull(hooks.fp_unavailable), ull(hooks.idle_skips),
              ull(hooks.idle_ticks_skipped), ull(rt::g_interpreted));
  if (!audio_wav.empty() || audio_out) {
    const audio::MixerStats& ms = mixer.stats();
    std::printf("audio: %llu frames mixed (%u Hz), %llu voice-ms, at most %u voices, aux A returns in %llu frames; %s\n",
                ull(ms.frames), hw.ai_dsp_rate(), ull(ms.voice_ms), ms.max_voices, ull(ms.aux_return_frames[0]),
                levels.summary().c_str());
    if (!audio_wav.empty()) std::printf("audio: WAV %s%s\n", audio_wav.c_str(), wav.close() ? "" : " (WRITE FAILED)");
    if (audio_out) {
      audio_out->close();
      const audio::SdlAudioOut::Stats as = audio_out->stats();
      std::printf("audio: played %llu frames (%llu non-silent, peak %d), %llu silent, %llu underruns, %llu skipped, "
                  "%llu dropped (ring full), ratio %.4f..%.4f, throttled %llu ms\n",
                  ull(as.drift.out_frames), ull(as.played_nonsilent), as.played_peak, ull(as.drift.silent_frames),
                  ull(as.drift.underruns), ull(as.drift.skipped_frames), ull(as.ring_full), as.drift.min_ratio,
                  as.drift.max_ratio, ull(as.throttle_waits));
    }
  }
  if (!renderer) return rc;
  const auto& ps = proc.stats();
  std::printf("gx: %llu commands, %llu draws, %llu vertices, %llu triangles, %llu EFB copies (%llu to the XFB), "
              "%llu TLUT loads, %llu unknown; %llu state snapshots, %llu TMEM preloads\n",
              ull(ps.commands), ull(ps.draws), ull(ps.vertices), ull(ps.triangles), ull(ps.copies), ull(ps.xfb_copies),
              ull(ps.tlut_loads), ull(ps.unknown), ull(ps.snapshots), ull(ps.preloads));
  const auto& rs = renderer->stats();
  std::printf("renderer: %llu draws (%llu skipped, %llu merged into the one before; %llu encoded), %llu passes, "
              "%llu pipelines, %llu textures decoded, %llu hits, "
              "%llu EFB-copy textures used, %llu texture copies, %llu clears; %llu unknown formats, "
              "%llu indirect draws, %llu fog draws; %llu with the previous draw's state; %llu preloaded textures; uploaded %.1f MB of vertices, "
              "%.1f MB of indices; EFB-copy textures dropped %llu, size misses %llu, tall %llu, inside %llu\n",
              ull(rs.draws), ull(rs.skipped), ull(rs.merged), ull(rs.encoded), ull(rs.passes), ull(rs.pipelines),
              ull(rs.textures_decoded), ull(rs.texture_hits), ull(rs.copy_textures_used), ull(rs.tex_copies),
              ull(rs.clears), ull(rs.unknown_formats), ull(rs.indirect_draws), ull(rs.fog_draws), ull(rs.same_state), ull(rs.tmem_textures),
              static_cast<double>(rs.vertex_bytes) / 1e6, static_cast<double>(rs.index_bytes) / 1e6,
              ull(rs.copies_dropped), ull(rs.copy_size_misses), ull(rs.copy_tall), ull(rs.copy_inside));
  if (play.gp_memory) {
    const auto& gs = play.gp_memory->stats();
    std::printf("gp memory: %llu cache write-backs (%.1f MB), %llu reads with lines from them (%llu lines), %llu ended by DMA\n",
                ull(gs.flushes), static_cast<double>(gs.flushed_bytes) / 1e6, ull(gs.overlays), ull(gs.overlaid_lines),
                ull(gs.drops));
  }
  if (!play.present_ms.empty()) {
    std::vector<double> v = play.present_ms;
    std::sort(v.begin(), v.end());
    double sum = 0, sq = 0;
    for (double x : v) sum += x;
    const double mean = sum / static_cast<double>(v.size());
    for (double x : v) sq += (x - mean) * (x - mean);
    auto pct = [&](double q) { return v[std::min(v.size() - 1, static_cast<std::size_t>(q * static_cast<double>(v.size())))]; };
    std::size_t late = 0;
    for (double x : v) late += x > 25.0 ? 1 : 0;
    std::printf("pacing: %zu present intervals: mean %.3f ms (%.2f per second), sd %.3f ms, p1 %.2f, p50 %.2f, p99 %.2f, "
                "max %.2f ms; %zu over 25 ms; present mode %s%s\n",
                v.size(), mean, mean > 0 ? 1000.0 / mean : 0.0, std::sqrt(sq / static_cast<double>(v.size())), pct(0.01),
                pct(0.5), pct(0.99), v.back(), late, window ? window->present_mode_name() : "-",
                play.pace ? "" : " (not paced: --pace)");
    std::printf("pacing: %.1f s waiting for the field schedule, %.1f s in window events and present\n", play.wait_s,
                play.present_s);
    if (play.pace)
      std::printf("pacing: %llu fields shown late, %llu schedule restarts; %llu sleeps woke after the field's time "
                  "(%.1f s in all)%s\n",
                  ull(play.late_fields), ull(play.restarts), ull(play.overslept), play.overslept_s,
#ifdef _WIN32
                  play.timer ? "" : " (no high-resolution timer)"
#else
                  ""
#endif
      );
  }
  std::printf("display: %llu presents, %llu captures%s%s; WebGPU errors %llu\n", ull(play.presents), ull(play.captures),
              play.capture_dir.empty() ? "" : " in ", play.capture_dir.c_str(), ull(gpu->errors()));
  if (gpu->errors() || ps.xfb_copies == 0) rc = 1;
  return rc;
}
