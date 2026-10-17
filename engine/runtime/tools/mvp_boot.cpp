// mvp-boot: runs the retail DOL natively (translated C++), for development.
//
//   mvp-boot lockstep   E0b item 6: run the translated code from __start and the
//                       reference interpreter side by side (runtime/lockstep.h)
//                       up to the first MMIO access.
//   mvp-boot run [--until main|none|NAME|0xADDR] [--mmio N] [--log FILE] [--fifo FILE]
//                [--max-entries N] [--max-ticks N] [--unnamed stop|log]
//                [--input SCRIPT] [--frames N] [--ports MASK] [--frame-log FILE]
//                [--ax-voices on|off] [--trace on|off] [--audio-wav FILE]
//                [--count NAME,...] [--peek ADDR:LEN,...] [--time-at F,...]
//                       E0b items 7 and 8: boot with the IPL environment
//                       (runtime/boot.h) and the hardware model (runtime/hw.h).
//                       Prints the calls __start makes; logs every call from game
//                       or EA code into the SDK and runtime libraries (K1's set)
//                       by name to FILE (default sdk_calls.log, in the current
//                       directory) and stops at a call into an SDK function that
//                       has no name yet, printing its address (input for K1).
//                       E0c: controller input comes from the input queue
//                       (runtime/input.h): --input feeds a script of
//                       frame-indexed events (default: port 1 connected, nothing
//                       pressed; --ports sets the connected ports), --frames N
//                       stops at the poll of frame N (after frames 0..N-1), and
//                       one line per frame poll goes to the frame log (default
//                       frames.log). --ax-voices off freezes the AX voices
//                       (E0b behaviour; runtime/ax.h).
//                       E2: --trace off runs without the per-entry trace hook
//                       (no SDK call log, --until and --max-entries unused): the
//                       speed of the translated code alone, for measurements.
//                       E3b: --audio-wav mixes
//                       the AX frames (audio/mixer.h) into a WAV file (keep it
//                       under build/); the mixer only reads guest state, so
//                       everything else in the report is the same without it.
//                       --count prints how often the listed functions (names
//                       or 0x addresses) were entered, --peek prints guest
//                       memory (hex, at most 256 bytes each) at the end of the
//                       run (for investigations). --time-at prints (on stderr)
//                       the host seconds spent in the run when the polls of the
//                       listed frames come (Q4: the cost of a stretch of play),
//                       and under Node the main thread's CPU seconds too (the
//                       cost on a busy machine).
//
// Needs the disc (MVP_DISC) and a build with the translated code. Prints register
// values, names and addresses only; the log stays under build/ (only its hash may
// be committed).
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "audio/mixer.h"
#include "audio/wav.h"
#include "common/bytes.h"
#include "common/file.h"
#include "common/sha1.h"
#include "ppc/isa.h"
#include "runtime/boot.h"
#include "runtime/hle.h"
#include "runtime/hw.h"
#ifdef MVP_BOOT_WASM
#include "runtime/wasm_host.h"
#endif
#include "runtime/input.h"
#include "runtime/lockstep.h"
#include "runtime/sdk_names.h"
#include "runtime/sys_hooks.h"
#include "translator/program.h"
#include "translator/units.h"

void mvp_register_game();

using namespace mvp;
using namespace mvp::rt;

namespace {

int lockstep(const xlat::Inputs& in) {
  xlat::ProgramCfg cfg = xlat::build_cfg(in.disc.dol, in.symbols);
  Machine ref, nat;
  FixedHooks ref_hooks, nat_hooks;
  StopIo ref_io, nat_io;
  ref_io.cpu = &ref.cpu();
  nat_io.cpu = &nat.cpu();
  ref.memory().io = &ref_io;
  nat.memory().io = &nat_io;
  ref.cpu().hooks = &ref_hooks;
  nat.cpu().hooks = &nat_hooks;
  ref.load_dol(in.disc.dol);
  nat.load_dol(in.disc.dol);
  mvp_register_game();
  LockstepResult r = run_lockstep(ref, nat, cfg, 16);
  auto name = [&](u32 a) {
    const xlat::Symbol* s = cfg.index.containing(a);
    return s ? s->name : std::string("?");
  };
  std::printf("%llu function-entry events compared, %llu reference instructions, %llu full MEM1 comparisons\n",
              static_cast<unsigned long long>(r.events), static_cast<unsigned long long>(r.ref_steps),
              static_cast<unsigned long long>(r.mem_checks));
  if (!r.pass) {
    std::printf("%s\n  recent entries:", r.message.c_str());
    for (u32 a : r.recent) std::printf(" %08X(%s)", a, name(a).c_str());
    std::printf("\nFAIL\n");
    return 1;
  }
  std::printf("first MMIO access: %s of %d bytes at 0x%08X, from 0x%08X (%s)\n", r.stop.write ? "write" : "read",
              r.stop.size, r.stop.addr, r.stop.pc, name(r.stop.pc).c_str());
  std::printf("PASS: zero divergences from __start to the first MMIO access\n");
  return 0;
}

struct RunTrace {
  const xlat::ProgramCfg* cfg = nullptr;
  const xlat::UnitMap* units = nullptr;
  u32 start_fn = 0, stop_at = 0;
  std::uint64_t events = 0, max_events = 20000000;
  // The SDK call log: one line per boundary call, streamed to a file and hashed.
  std::FILE* log = nullptr;
  Sha1 log_sha;
  std::uint64_t log_bytes = 0;
  void log_line(const std::string& s) {
    log_sha.update(s.data(), s.size());
    log_bytes += s.size();
    if (log) std::fwrite(s.data(), 1, s.size(), log);
  }
  std::uint64_t sdk_calls = 0, exception_entries = 0, thread_switches = 0;
  u32 thread = 0;
  bool stop_on_unnamed = true;
  std::map<u32, std::uint64_t> unnamed;  // unnamed SDK callees (input for K1)
  std::map<u32, u32> unnamed_first_caller;
  std::map<std::string, std::uint64_t> per_function;
  std::vector<std::string> first_order;
  std::map<u32, std::uint64_t> counted;  // --count: entries per function address

  std::string name(u32 a) const {
    const xlat::Symbol* s = cfg->index.containing(a);
    if (!s) return "?";
    if (s->name.rfind("fn_", 0) == 0)
      if (const char* alias = sdk_alias(s->address)) return alias;
    return s->name;
  }
  bool named(u32 a) const {
    const xlat::Symbol* s = cfg->index.at(a);
    return s && (s->name.rfind("fn_", 0) != 0 || sdk_alias(a));
  }
};
RunTrace* g_trace = nullptr;

// The frame log: one line per frame poll (frame index, time base, SDK boundary
// calls so far, and the frame's input in wire form), streamed and hashed.
struct FrameTrace {
  SystemHooks* hooks = nullptr;
  std::FILE* log = nullptr;
  Sha1 sha;
  std::uint64_t bytes = 0, stop_at = ~0ull;
  std::vector<std::uint64_t> time_at;  // --time-at: frames whose host time is printed
  std::chrono::steady_clock::time_point start;
};
FrameTrace* g_frames = nullptr;

// Q4: the calling thread's CPU seconds where the host reports them (Node), else -1.
double thread_cpu_seconds() {
#ifdef __EMSCRIPTEN__
  return EM_ASM_DOUBLE({
    if (typeof process == 'undefined' || !process.threadCpuUsage) return -1;
    var u = process.threadCpuUsage();
    return (u.user + u.system) / 1e6;
  });
#else
  return -1;
#endif
}

void frame_hook(ppc::Cpu& c, std::uint64_t frame) {
  FrameTrace& f = *g_frames;
  if (frame >= f.stop_at) throw StopRun{"ran " + std::to_string(f.stop_at) + " frames (--frames)"};
  if (std::find(f.time_at.begin(), f.time_at.end(), frame) != f.time_at.end())
    std::fprintf(stderr, "time-at frame %llu: %.3f s, thread cpu %.3f s\n", static_cast<unsigned long long>(frame),
                 std::chrono::duration<double>(std::chrono::steady_clock::now() - f.start).count(),
                 thread_cpu_seconds());
  char b[160];
  int n = std::snprintf(b, sizeof b, "frame %llu tb %llu calls %llu in ", static_cast<unsigned long long>(frame),
                        static_cast<unsigned long long>(f.hooks->time_base(c)),
                        static_cast<unsigned long long>(g_trace->sdk_calls));
  std::string line(b, static_cast<std::size_t>(n));
  for (u8 v : encode_frame(g_input->current())) {
    std::snprintf(b, sizeof b, "%02x", v);
    line += b;
  }
  line += "\n";
  f.sha.update(line.data(), line.size());
  f.bytes += line.size();
  if (f.log) std::fwrite(line.data(), 1, line.size(), f.log);
}

void trace_hook(ppc::Cpu& c, u32 addr) {
  RunTrace& t = *g_trace;
  ++t.events;
  if (!t.counted.empty()) {
    auto it = t.counted.find(addr);
    if (it != t.counted.end()) ++it->second;
  }
  // Thread switches: the OS's current-thread pointer (0x800000E4) changed.
  if (const u8* p = c.mem->host(0x800000E4u)) {
    u32 cur = be32(p);
    if (cur != t.thread) {
      if (t.thread && cur) ++t.thread_switches;
      if (cur) t.thread = cur;
    }
  }
  u32 call_site = c.lr - 4;
  const xlat::Symbol* caller = t.cfg->index.containing(call_site);
  if (caller && caller->address == t.start_fn)
    std::printf("  event %llu: __start (lr 0x%08X) calls %s (0x%08X)\n", static_cast<unsigned long long>(t.events),
                c.lr, t.name(addr).c_str(), addr);
  // An SDK boundary call: game or EA code (or crt0) calling into the runtime libraries.
  // LR names the call site only when the word there is a call to this entry (a
  // direct bl to it, or an indirect call); otherwise this entry was reached by a
  // tail call further down the chain (PCinit's `b` into its body, for example).
  bool direct = false;
  if (rt::g_exception_depth > 0 && c.lr == rt::g_exception_lr) {
    ++t.exception_entries;  // the OS's exception code, entered from its vector, not called
  } else if (const u8* p = c.mem->host(call_site)) {
    ppc::Insn w{be32(p)};
    ppc::Op op = ppc::decode(w.w);
    if (op == ppc::Op::b && w.lk()) direct = (w.aa() ? static_cast<u32>(w.li()) : call_site + static_cast<u32>(w.li())) == addr;
    else if (op == ppc::Op::bcctr && w.lk()) direct = (c.ctr & ~3u) == addr;
    else if (op == ppc::Op::bclr && w.lk()) {
      // blrl: LR no longer holds the target, but the register moved into it by
      // the usual `mtlr rX` just before still does (the callee has not run yet).
      direct = true;
      for (u32 back = 4; back <= 16; back += 4) {
        const u8* q = c.mem->host(call_site - back);
        if (!q) break;
        ppc::Insn m{be32(q)};
        if (ppc::decode(m.w) == ppc::Op::mtspr && m.spr() == 8) {
          direct = (c.gpr[m.rS()] & ~3u) == addr;
          break;
        }
      }
    }
  }
  if (direct && t.units->is_runtime(addr) && !t.units->is_runtime(call_site)) {
    if (!t.named(addr) && t.stop_on_unnamed) {
      char b[160];
      std::snprintf(b, sizeof b, "call into unnamed SDK function 0x%08X (%s, unit %s) from 0x%08X (%s)", addr,
                    t.name(addr).c_str(), t.units->unit_at(addr).c_str(), call_site, t.name(call_site).c_str());
      throw StopRun{b};
    }
    std::string callee = t.name(addr);
    if (!t.named(addr)) {
      if (t.unnamed[addr]++ == 0) t.unnamed_first_caller[addr] = call_site;
      callee += " (unnamed)";
    }
    ++t.sdk_calls;
    if (t.per_function[callee]++ == 0) t.first_order.push_back(callee);
    t.log_line(callee + " <- " + t.name(call_site) + "\n");
  }
  if (addr == t.stop_at) throw StopRun{"reached " + t.name(addr)};
  if (t.events >= t.max_events) throw StopRun{"function-entry budget used up (--max-entries)"};
}

int run_boot(const xlat::Inputs& in, int argc, char** argv) {
  std::string until = "main", log_path = "sdk_calls.log", fifo_path, unnamed_mode = "stop";
  std::string input_path, frame_log_path = "frames.log", audio_wav, count_list, peek_list;
  std::size_t mmio_lines = 40;
  std::uint64_t max_entries = 20000000, max_ticks = 2000000000ull, max_frames = ~0ull;
  std::vector<std::uint64_t> time_at;
  bool trace = true;
  u32 ports = 1;
  HardwareConfig hw_cfg;
  for (int k = 2; k + 1 < argc; ++k) {
    std::string a = argv[k];
    if (a == "--until") until = argv[k + 1];
    if (a == "--mmio") mmio_lines = static_cast<std::size_t>(std::atoi(argv[k + 1]));
    if (a == "--log") log_path = argv[k + 1];
    if (a == "--fifo") fifo_path = argv[k + 1];
    if (a == "--unnamed") unnamed_mode = argv[k + 1];
    if (a == "--max-entries") max_entries = std::strtoull(argv[k + 1], nullptr, 10);
    if (a == "--max-ticks") max_ticks = std::strtoull(argv[k + 1], nullptr, 10);
    if (a == "--input") input_path = argv[k + 1];
    if (a == "--frames") max_frames = std::strtoull(argv[k + 1], nullptr, 10);
    if (a == "--time-at")
      for (const char* p = argv[k + 1]; *p;) {
        char* end = nullptr;
        time_at.push_back(std::strtoull(p, &end, 10));
        p = *end == ',' ? end + 1 : end;
        if (end == p && *p) break;
      }
    if (a == "--ports") ports = static_cast<u32>(std::strtoul(argv[k + 1], nullptr, 0)) & 0xF;
    if (a == "--frame-log") frame_log_path = argv[k + 1];
    if (a == "--ax-voices") hw_cfg.ax_voices = std::string(argv[k + 1]) != "off";
    if (a == "--trace") trace = std::string(argv[k + 1]) != "off";
    if (a == "--audio-wav") audio_wav = argv[k + 1];
    if (a == "--count") count_list = argv[k + 1];
    if (a == "--peek") peek_list = argv[k + 1];
  }
  // Controller input (R14): a script, or port(s) connected with nothing pressed.
  std::string script_text = "mvp-input 1\n";
  if (!input_path.empty()) {
    auto text = read_text(input_path);
    if (!text) {
      std::fprintf(stderr, "mvp-boot: cannot read %s\n", input_path.c_str());
      return 2;
    }
    script_text = *text;
  }
  std::string script_err;
  FrameInput boot_input = neutral_input(ports);
  auto script = InputScript::parse(script_text, &script_err, boot_input);
  if (!script) {
    std::fprintf(stderr, "mvp-boot: %s: %s\n", input_path.c_str(), script_err.c_str());
    return 2;
  }
  InputQueue input(boot_input);
  input.set_source(&*script);
  auto splits = read_text(xlat::default_splits_path());
  if (!splits) {
    std::fprintf(stderr, "mvp-boot: cannot read %s\n", xlat::default_splits_path().c_str());
    return 1;
  }
  xlat::UnitMap units = xlat::UnitMap::parse(*splits);
  xlat::ProgramCfg cfg = xlat::build_cfg(in.disc.dol, in.symbols);
  Machine m;
  m.load_dol(in.disc.dol);
  BootLayout L = setup_boot_environment(m, in.disc);
  std::printf("boot: FST at 0x%08X (%u bytes), BI2 at 0x%08X, arena hi 0x%08X\n", L.fst, L.fst_size, L.bi2,
              L.arena_hi);
  Hardware hw(&m.cpu(), m.mem1(), Machine::kMem1Size, hw_cfg);
#ifdef MVP_BOOT_WASM
  hw.set_wgp_state(wasm_wgp_state());  // P2: the translated code appends to the write-gather pipe itself
  set_wasm_mem1(m.mem1(), Machine::kMem1Size);  // Q6: and reads and writes MEM1 directly
#endif
  hw.set_disc(in.disc.image.get());
  SystemHooks hooks(&m, &hw);
  m.memory().io = &hw;
  m.cpu().hooks = &hooks;
  hooks.tick_limit = max_ticks;
  hooks.reschedule(m.cpu());
  mvp_register_game();
  // The mixer (E3b): an observer of the DSP's frames; it writes only the WAV.
  audio::AxMixer mixer;
  audio::WavWriter wav;
  audio::LevelMeter levels;
  audio::TeeSink tee;
  if (!audio_wav.empty()) {
    if (!wav.open(audio_wav)) {
      std::fprintf(stderr, "mvp-boot: cannot write %s\n", audio_wav.c_str());
      return 2;
    }
    tee.add(&wav);
    tee.add(&levels);
    mixer.set_sink(&tee);
    mixer.set_rate([&hw] { return hw.ai_dsp_rate(); });
    hw.set_ax_observer(&mixer);
  }
  RunTrace t;
  t.cfg = &cfg;
  t.units = &units;
  t.start_fn = in.disc.dol.entry();
  t.max_events = max_entries;
  t.stop_on_unnamed = unnamed_mode != "log";
  for (std::size_t at = 0; at < count_list.size();) {
    std::size_t comma = count_list.find(',', at);
    if (comma == std::string::npos) comma = count_list.size();
    const std::string name = count_list.substr(at, comma - at);
    at = comma + 1;
    const xlat::Symbol* s = name.rfind("0x", 0) == 0 ? nullptr : in.symbols.find(name);
    const u32 a = s ? s->address : static_cast<u32>(std::strtoul(name.c_str(), nullptr, 16));
    if (!a) {
      std::fprintf(stderr, "mvp-boot: --count: no symbol %s\n", name.c_str());
      return 2;
    }
    t.counted[a] = 0;
  }
  const xlat::Symbol* mainsym = in.symbols.find("main");
  t.stop_at = until == "main" ? (mainsym ? mainsym->address : 0x8014D63Cu) : 0;
  if (until.rfind("0x", 0) == 0) {  // a function entry by address
    t.stop_at = static_cast<u32>(std::strtoul(until.c_str(), nullptr, 16));
  } else if (until != "main" && until != "none") {  // a function by its symbols.txt name
    const xlat::Symbol* s = in.symbols.find(until);
    if (!s) {
      std::fprintf(stderr, "mvp-boot: no symbol %s\n", until.c_str());
      return 2;
    }
    t.stop_at = s->address;
  }
  t.log = std::fopen(log_path.c_str(), "wb");
  if (!t.log) std::fprintf(stderr, "mvp-boot: cannot write %s\n", log_path.c_str());
  g_trace = &t;
  rt::g_entry_hook = trace ? &trace_hook : nullptr;
  FrameTrace ft;
  ft.hooks = &hooks;
  ft.stop_at = max_frames;
  ft.time_at = time_at;
  ft.log = std::fopen(frame_log_path.c_str(), "wb");
  if (!ft.log) std::fprintf(stderr, "mvp-boot: cannot write %s\n", frame_log_path.c_str());
  g_frames = &ft;
  rt::g_input = &input;
  rt::g_frame_hook = &frame_hook;
  std::string why;
  int rc = 1;
  // Host time spent running guest code (E2 speed measurements): printed only,
  // never visible to the guest.
  auto run_start = std::chrono::steady_clock::now();
  ft.start = run_start;
  try {
    rt::run(m.cpu());
  } catch (const StopRun& s) {
    why = s.why;
    rc = s.why.rfind("reached", 0) == 0 || s.why.rfind("ran ", 0) == 0 ? 0 : 1;
  } catch (const ppc::GuestFault& f) {
    why = std::string("guest fault: ") + f.what() + " in " + t.name(f.pc());
  }
  double run_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - run_start).count();
  rt::g_entry_hook = nullptr;
  rt::g_frame_hook = nullptr;
  rt::g_input = nullptr;
  hw.set_ax_observer(nullptr);
  if (ft.log) std::fclose(ft.log);
  std::printf("stopped: %s (pc 0x%08X in %s) after %llu function entries, %llu unwinds\n", why.c_str(),
              m.cpu().pc, t.name(m.cpu().pc).c_str(), static_cast<unsigned long long>(t.events),
              static_cast<unsigned long long>(rt::g_unwinds));
  std::printf("SDK calls: %llu boundary calls to %zu functions, %s; OSReport lines %llu; %llu exception entries\n",
              static_cast<unsigned long long>(t.sdk_calls), t.per_function.size(),
              t.unnamed.empty() ? "all named" : "SOME UNNAMED", static_cast<unsigned long long>(g_report_count),
              static_cast<unsigned long long>(t.exception_entries));
  for (const auto& [a, n] : t.unnamed)
    std::printf("  unnamed SDK function 0x%08X (unit %s): %llu calls, first from 0x%08X (%s)\n", a,
                t.units->unit_at(a).c_str(), static_cast<unsigned long long>(n), t.unnamed_first_caller[a],
                t.name(t.unnamed_first_caller[a]).c_str());
  for (const auto& n : t.first_order) std::printf("  %-28s %llu\n", n.c_str(), static_cast<unsigned long long>(t.per_function[n]));
  if (t.log) std::fclose(t.log);
  std::printf("SDK call log: %s, %llu bytes, sha1 %s\n", log_path.c_str(), static_cast<unsigned long long>(t.log_bytes),
              Sha1::hex(t.log_sha.finish()).c_str());
  for (const auto& [a, n] : t.counted)
    std::printf("count: %s (0x%08X) entered %llu times\n", t.name(a).c_str(), a, static_cast<unsigned long long>(n));
  for (std::size_t at = 0; at < peek_list.size();) {
    std::size_t comma = peek_list.find(',', at);
    if (comma == std::string::npos) comma = peek_list.size();
    const std::string item = peek_list.substr(at, comma - at);
    at = comma + 1;
    char* end = nullptr;
    const u32 addr = static_cast<u32>(std::strtoul(item.c_str(), &end, 16));
    const u32 len = std::min<u32>(*end == ':' ? static_cast<u32>(std::strtoul(end + 1, nullptr, 0)) : 4u, 256u);
    std::printf("peek: %08X:", addr);
    for (u32 k = 0; k < len; ++k) {
      const u8* b = m.memory().host(addr + k);
      if (k && k % 32 == 0) std::printf("\npeek: %08X:", addr + k);
      std::printf(k % 4 ? "%02x" : " %02x", b ? *b : 0);
    }
    std::printf("\n");
  }
  auto ull = [](std::uint64_t v) { return static_cast<unsigned long long>(v); };
  // stderr: host time differs between runs; the reports on stdout must not (local_boot_twice).
  std::fprintf(stderr, "host: %.3f s in rt::run, %.0f million guest instructions executed per second\n", run_seconds,
               run_seconds > 0 ? static_cast<double>(m.cpu().ticks - hooks.idle_ticks_skipped) / run_seconds / 1e6 : 0.0);
  if (const double cpu = thread_cpu_seconds(); cpu >= 0) std::fprintf(stderr, "thread cpu: %.3f s\n", cpu);
  std::printf("clock: %llu instructions, time base %llu (%.3f s of guest time), %llu VI fields\n", ull(m.cpu().ticks),
              ull(hooks.time_base(m.cpu())), static_cast<double>(hooks.time_base(m.cpu())) / 40500000.0,
              ull(hw.vi_fields()));
  {
    Sha1 mem;
    mem.update(m.mem1(), Machine::kMem1Size);
    std::printf("memory: MEM1 sha1 %s\n", Sha1::hex(mem.finish()).c_str());
  }
  std::printf("threads: %llu switches (%.2f per VI field), %llu unwinds (%.3f per switch), %llu in-frame resumes, "
              "%llu resume entries\n",
              ull(t.thread_switches),
              hw.vi_fields() ? static_cast<double>(t.thread_switches) / static_cast<double>(hw.vi_fields()) : 0.0,
              ull(rt::g_unwinds),
              t.thread_switches ? static_cast<double>(rt::g_unwinds) / static_cast<double>(t.thread_switches) : 0.0,
              ull(rt::g_inframe_resumes), ull(rt::g_resume_entries));
  std::printf("interrupts: %llu external, %llu decrementer; idle skips %llu (%llu instructions skipped); "
              "unwinds %llu; interpreted %llu instructions (%.4f%% of those run)\n",
              ull(hooks.external_interrupts), ull(hooks.decrementer_interrupts), ull(hooks.idle_skips),
              ull(hooks.idle_ticks_skipped), ull(rt::g_unwinds), ull(rt::g_interpreted),
              m.cpu().ticks > hooks.idle_ticks_skipped
                  ? 100.0 * static_cast<double>(rt::g_interpreted) /
                        static_cast<double>(m.cpu().ticks - hooks.idle_ticks_skipped)
                  : 0.0);
  {
    Sha1 ss;
    ss.update(script_text.data(), script_text.size());
    std::printf("input: %s (%zu events, sha1 %s), ports 0x%X; %llu frame polls, %llu PADReads; frame log %s, %llu "
                "bytes, sha1 %s\n",
                input_path.empty() ? "no script" : input_path.c_str(), script->events(), Sha1::hex(ss.finish()).c_str(),
                ports, ull(rt::g_frame_polls), ull(rt::g_pad_reads), frame_log_path.c_str(), ull(ft.bytes),
                Sha1::hex(ft.sha.finish()).c_str());
  }
  std::printf("MMIO: %zu accesses logged, %llu sc, %llu icbi blocks\n", hw.log().size(), ull(hooks.syscalls),
              ull(hooks.icbi_blocks));
  std::printf("DI: %llu drive commands, %llu bytes read; DSP: %llu AX frames; AI: %llu DMA blocks\n", ull(hw.di_commands()), ull(hw.di_bytes()),
              ull(hw.dsp_frames()), ull(hw.ai_dma_blocks()));
  const AxStats& ax = hw.ax_stats();
  std::printf("AX voices (%s): %llu voice frames (at most %llu voices at once), %llu input samples, %llu ended, "
              "%llu loop wraps, %llu PB updates\n",
              hw_cfg.ax_voices ? "advanced" : "frozen", ull(ax.voice_frames), ull(ax.max_running), ull(ax.samples),
              ull(ax.ended), ull(ax.looped), ull(ax.updates));
  if (!audio_wav.empty()) {
    const bool ok = wav.close();
    const audio::MixerStats& ms = mixer.stats();
    std::printf("audio: %s%s, %u Hz (%llu rate changes); %s\n", audio_wav.c_str(), ok ? "" : " (WRITE FAILED)",
                wav.rate(), ull(wav.rate_changes()), levels.summary().c_str());
    std::printf("audio: mixer %llu frames, %llu voice-ms (src %llu/%llu/%llu/%llu), at most %u voices, %llu mix "
                "reloads, %llu ramp ms, %llu lpf ms, %llu itd ms, %llu envelope ms, max envelope 0x%04X, %llu depops; "
                "aux A %llu frames (%llu with sends, %llu with returns), aux B %llu (%llu, %llu); stereo %llu, "
                "surround %llu, DPL2 %llu, compressor %llu frames; %llu unknown commands; %llu clipped, %llu "
                "non-silent frames\n",
                ull(ms.frames), ull(ms.voice_ms), ull(ms.src_ms[0]), ull(ms.src_ms[1]), ull(ms.src_ms[2]),
                ull(ms.src_ms[3]), ms.max_voices, ull(ms.mix_reloads), ull(ms.ramp_ms), ull(ms.lpf_ms), ull(ms.itd_ms),
                ull(ms.env_ms), ms.max_ve, ull(ms.depops), ull(ms.aux_frames[0]), ull(ms.aux_send_frames[0]),
                ull(ms.aux_return_frames[0]), ull(ms.aux_frames[1]), ull(ms.aux_send_frames[1]),
                ull(ms.aux_return_frames[1]), ull(ms.stereo_mode_frames), ull(ms.surround_mode_frames),
                ull(ms.dpl2_frames), ull(ms.compressor_frames), ull(ms.unknown_commands), ull(ms.clipped),
                ull(ms.nonsilent_frames));
  }
  std::printf("DSP: state %d, %zu mails waiting, mailbox 0x%08X, control 0x%04X; PI cause 0x%05X mask 0x%05X\n",
              hw.dsp_state(), hw.dsp_mails_waiting(), hw.dsp_mail_to_cpu(), hw.dsp_control(), hw.pi_cause(),
              hw.pi_mask());
  if (!fifo_path.empty() && !write_file(fifo_path, hw.fifo().data(), hw.fifo().size()))
    std::fprintf(stderr, "mvp-boot: cannot write %s\n", fifo_path.c_str());
  const GxFifo& gx = hw.gx();
  std::printf("GX: %llu bytes to the write-gather pipe, %llu commands (%llu BP loads, %llu primitives, %llu vertices, "
              "%llu display list calls, %llu bad), %llu draw-done%s\n",
              ull(hw.fifo_bytes()), ull(gx.commands()), ull(gx.bp_loads()), ull(gx.primitives()), ull(gx.vertices()),
              ull(gx.display_lists()), ull(gx.display_list_errors()), ull(hw.draw_done_count()), gx.broken() ? "; PARSE LOST" : "");
  if (gx.broken())
    std::printf("GX: byte 0x%02X at stream offset %llu starts no command; the commands before it:\n%s",
                gx.broken_opcode(), ull(gx.broken_offset()), gx.broken_context().c_str());
  std::size_t shown = 0;
  for (const auto& a : hw.log()) {
    if (shown++ >= mmio_lines) break;
    std::printf("  %s %-3s 0x%08X %d bytes %s 0x%08X  (pc 0x%08X %s)\n", a.write ? "W" : "R",
                Hardware::block_name(a.addr), a.addr, a.size, a.write ? "<-" : "->", a.value, a.pc,
                t.name(a.pc).c_str());
  }
  return rc;
}

}  // namespace

int main(int argc, char** argv) {
  std::setvbuf(stdout, nullptr, _IOLBF, 1 << 16);
  std::string cmd = argc > 1 ? argv[1] : "";
  std::string err;
  auto in = xlat::load_inputs(xlat::default_disc_path(), xlat::default_symbols_path(), &err);
  if (!in) {
    std::fprintf(stderr, "mvp-boot: %s\n", err.c_str());
    return 77;
  }
  if (cmd == "lockstep") return lockstep(*in);
  if (cmd == "run") return run_boot(*in, argc, argv);
  std::fprintf(stderr, "usage: mvp-boot lockstep | run [--until main|none|NAME|0xADDR] [--mmio N] [--log FILE] [--fifo FILE] "
                       "[--max-entries N] [--max-ticks N] [--unnamed stop|log] [--input SCRIPT] [--frames N] "
                       "[--ports MASK] [--frame-log FILE] [--ax-voices on|off] [--trace on|off]\n");
  return 2;
}
