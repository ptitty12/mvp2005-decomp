#include "translator/wasm_emit.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <unordered_map>

#include "ppc/cpu.h"
#include "ppc/effects.h"
#include "translator/wasm_binary.h"

namespace mvp::xlat {

using namespace wasm;
using ppc::Effects;
using ppc::Insn;
using ppc::Op;
using ppc::RegSet;
namespace reg = ppc::reg;

namespace {

// ---- the runtime's Cpu layout (wasm32) ----
// Every field before the pointers is laid out identically on x64 and wasm32, so
// a native build of the emitter (the CLI) writes the same module.
constexpr u32 kGpr = offsetof(ppc::Cpu, gpr);
constexpr u32 kFpr = offsetof(ppc::Cpu, fpr);  // Fpr: ps0 then ps1, 8 bytes each
constexpr u32 kCr = offsetof(ppc::Cpu, cr);
constexpr u32 kLr = offsetof(ppc::Cpu, lr);
constexpr u32 kCtr = offsetof(ppc::Cpu, ctr);
constexpr u32 kSo = offsetof(ppc::Cpu, xer_so);
constexpr u32 kCa = offsetof(ppc::Cpu, xer_ca);
constexpr u32 kPc = offsetof(ppc::Cpu, pc);
constexpr u32 kTicks = offsetof(ppc::Cpu, ticks);
constexpr u32 kNextEvent = offsetof(ppc::Cpu, next_event);
constexpr u32 kFpscr = offsetof(ppc::Cpu, fpscr);
constexpr u32 kMsr = offsetof(ppc::Cpu, msr);
constexpr u32 kOv = offsetof(ppc::Cpu, xer_ov);
constexpr u32 kXerLow = offsetof(ppc::Cpu, xer_low);
constexpr u32 kSpr = offsetof(ppc::Cpu, spr);
constexpr u32 kGqr = offsetof(ppc::Cpu, spr) + 4 * ppc::spr::GQR0;
// Cpu::mem, the Memory whose first member is the page table.
constexpr u32 kMem = offsetof(ppc::Cpu, spr) + sizeof(ppc::Cpu::spr);
static_assert(kMem % 8 == 0, "Cpu::mem must sit at the same offset on 32- and 64-bit hosts");
#if defined(__EMSCRIPTEN__)
static_assert(offsetof(ppc::Cpu, mem) == kMem);
#endif
static_assert(offsetof(ppc::Memory, page) == 0);
static_assert(sizeof(ppc::Fpr) == 16 && offsetof(ppc::Fpr, ps1) == 8);
u32 ps0(u32 r) { return kFpr + 16 * r; }
u32 ps1(u32 r) { return kFpr + 16 * r + 8; }

u32 reg_offset(int r) {
  if (r < 32) return kGpr + 4 * static_cast<u32>(r);
  switch (r) {
    case reg::CR: return kCr;
    case reg::LR: return kLr;
    case reg::CTR: return kCtr;
    case reg::CA: return kCa;
    default: return kSo;
  }
}

// ---- module interface ----
enum Ty : u32 {
  tV_I, tV_II, tI_IIIL, tV_IIIIL, tV_IIIL, tI_III, tV_III, tI_II, tL_I, tI_L, tV_IIIIIL, tL_IIIL, tV_IILIL, kTypes
};
enum Imp : u32 {
  iOnEntry, iSafepoint, iRead8, iRead16, iRead32, iWrite8, iWrite16, iWrite32, iExec, iCallIndirect,
  iAbsoluteCall, iResume, iHleCall, iHleBefore, iTrap, iRfi, iSystemCall, iTw, iFpUnavailable, iGpWait, iWgpStore,
  kImports
};
constexpr Ty kImportType[kImports] = {tV_II,   tV_I,  tI_IIIL, tI_IIIL, tI_IIIL, tV_IIIIL, tV_IIIIL, tV_IIIIL, tV_IIIL,
                                      tV_II,   tV_II, tI_III,  tV_III,  tV_II,   tV_III,   tV_I,     tV_I,     tV_III,
                                      tV_II,   tV_III, tV_IIIIL};
// Helpers defined in every module, after the imports.
enum Hlp : u32 {
  hRd8 = kImports, hRd16, hRd32, hWr8, hWr16, hWr32, hCmps, hCmpu, hDivw, hDivwu, hS2D, hD2S,
  hLmw, hStmw, hPsqL, hPsqSt, hRd64, hWr64, hLfs, hStfs,
  // Q6: the page-table paths behind the memory helpers' fronts (the helpers above
  // of the same name try MEM1 first, then tail-call these).
  hRd8P, hRd16P, hRd32P, hWr8P, hWr16P, hWr32P, hRd64P, hWr64P, hLfsP, hStfsP, kFirstGuest
};
constexpr u32 kHelperCount = static_cast<u32>(kFirstGuest) - static_cast<u32>(kImports);
constexpr Ty kHelperType[kHelperCount] = {tI_IIIL,  tI_IIIL,  tI_IIIL,   tV_IIIIL,  tV_IIIIL, tV_IIIIL,
                                          tI_III,   tI_III,   tI_II,     tI_II,     tL_I,     tI_L,
                                          tV_IIIIL, tV_IIIIL, tV_IIIIIL, tV_IIIIIL, tL_IIIL, tV_IILIL,
                                          tL_IIIL,  tV_IILIL,
                                          tI_IIIL,  tI_IIIL,  tI_IIIL,   tV_IIIIL,  tV_IIIIL, tV_IIIIL,
                                          tL_IIIL,  tV_IILIL, tL_IIIL,   tV_IILIL};
constexpr const char* kHelperName[kHelperCount] = {"rd8",  "rd16", "rd32", "wr8",   "wr16", "wr32", "cmps", "cmpu",
                                                   "divw", "divwu", "s2d", "d2s",   "lmw",  "stmw", "psq_l", "psq_st",
                                                   "rd64", "wr64", "lfs", "stfs",
                                                   "rd8.page", "rd16.page", "rd32.page", "wr8.page", "wr16.page",
                                                   "wr32.page", "rd64.page", "wr64.page", "lfs.page", "stfs.page"};
// Imported globals (immutable i32): the table base and three runtime addresses.
enum Glb : u32 { gTableBase, gUnwinding, gResume, gHook, gWgp, gGpPages, gMem1, kGlobals };
// V4: WasmEmitOptions::fixed of the module being emitted (emit_wasm sets it).
thread_local WasmFixed t_fixed;
// Pushes the address the imported global `g` holds: the constant when it is fixed.
void glb(Fn& b, Glb g) {
  u32 c = 0;
  switch (g) {
    case gUnwinding: c = t_fixed.unwinding; break;
    case gResume: c = t_fixed.resume; break;
    case gHook: c = t_fixed.hook; break;
    case gWgp: c = t_fixed.wgp; break;
    case gGpPages: c = t_fixed.gp_pages; break;
    default: break;
  }
  if (c) b.i32c(c);
  else b.global(g);
}

// ---- per-instruction classification ----
enum Kind : u8 {
  kInline, kHelper, kNop, kSc, kTw, kTrapOp, kJump, kCJump, kCall, kTail, kRet, kIJump, kJTab, kRfi, kTrap,
  kFp,    // floating-point arithmetic: an exact inline fast path, else the runtime (E2d)
  kMemH,  // lmw/stmw: an in-module helper on the Cpu struct (helper dataflow, E2d)
};
enum Grp : u8 { gInline, gHelper, gCall, gExit, gEnd, gFall };
enum Pre : u8 { pNone, pBack, pIdle };

bool is_inline(Op op, Insn i) {
  switch (op) {
    case Op::add: case Op::addc: case Op::adde: case Op::addme: case Op::addze: case Op::divw: case Op::divwu:
    case Op::mullw: case Op::neg: case Op::subf: case Op::subfc: case Op::subfe: case Op::subfme: case Op::subfze:
      return !i.oe();
    case Op::addi: case Op::addis: case Op::addic: case Op::addic_rc: case Op::subfic: case Op::mulli: case Op::mulhw:
    case Op::mulhwu: case Op::cmp: case Op::cmpi: case Op::cmpl: case Op::cmpli: case Op::and_: case Op::andc:
    case Op::eqv: case Op::nand: case Op::nor: case Op::or_: case Op::orc: case Op::xor_: case Op::extsb:
    case Op::extsh: case Op::cntlzw: case Op::ori: case Op::oris: case Op::xori: case Op::xoris: case Op::andi_rc:
    case Op::andis_rc: case Op::rlwinm: case Op::rlwnm: case Op::rlwimi: case Op::slw: case Op::srw: case Op::srawi:
    case Op::crand: case Op::crandc: case Op::creqv: case Op::crnand: case Op::crnor: case Op::cror: case Op::crorc:
    case Op::crxor: case Op::mcrf: case Op::mfcr: case Op::mtcrf:
    case Op::lbz: case Op::lbzu: case Op::lbzx: case Op::lbzux: case Op::lhz: case Op::lhzu: case Op::lhzx:
    case Op::lhzux: case Op::lha: case Op::lhau: case Op::lhax: case Op::lhaux: case Op::lwz: case Op::lwzu:
    case Op::lwzx: case Op::lwzux: case Op::stb: case Op::stbu: case Op::stbx: case Op::stbux: case Op::sth:
    case Op::sthu: case Op::sthx: case Op::sthux: case Op::stw: case Op::stwu: case Op::stwx: case Op::stwux:
    case Op::sraw:
      return true;
    // Quantized loads and stores (E2d): in-module helpers; FPRs stay in the Cpu struct.
    case Op::psq_l: case Op::psq_lu: case Op::psq_lx: case Op::psq_lux: case Op::psq_st: case Op::psq_stu:
    case Op::psq_stx: case Op::psq_stux:
      return true;
    case Op::mfspr: case Op::mtspr: {
      // Every SPR but the decrementer, the time base and WPAR (runtime hooks; a
      // WPAR write resets the write-gather pipe): LR and CTR are locals, XER is
      // SO and CA locals plus the Cpu struct, the rest are plain Cpu::spr words
      // (sem::read_spr / write_spr).
      const u32 n = i.spr();
      if (n == ppc::spr::DEC || n == ppc::spr::TBL_W || n == ppc::spr::TBU_W) return false;
      if (op == Op::mtspr && n == ppc::spr::WPAR) return false;
      return op == Op::mtspr || (n != ppc::spr::TBL_R && n != ppc::spr::TBU_R);
    }
    case Op::mfmsr:
      return true;
    // Floating point that only moves bits (FPRs stay in the Cpu struct).
    case Op::lfs: case Op::lfsu: case Op::lfsx: case Op::lfsux: case Op::lfd: case Op::lfdu: case Op::lfdx:
    case Op::lfdux: case Op::stfs: case Op::stfsu: case Op::stfsx: case Op::stfsux: case Op::stfd: case Op::stfdu:
    case Op::stfdx: case Op::stfdux: case Op::stfiwx: case Op::fmr: case Op::fneg: case Op::fabs: case Op::fnabs:
    case Op::ps_mr: case Op::ps_neg: case Op::ps_abs: case Op::ps_nabs: case Op::ps_merge00: case Op::ps_merge01:
    case Op::ps_merge10: case Op::ps_merge11:
      return true;
    default:
      return false;
  }
}

bool is_nop(Op op) {
  switch (op) {
    case Op::dcbf: case Op::dcbi: case Op::dcbst: case Op::dcbt: case Op::dcbtst: case Op::eieio: case Op::isync:
    case Op::sync: case Op::tlbie: case Op::tlbsync:
      return true;
    default:
      return false;
  }
}

bool is_memory(Op op) {
  switch (op) {
    case Op::lbz: case Op::lbzu: case Op::lbzx: case Op::lbzux: case Op::lhz: case Op::lhzu: case Op::lhzx:
    case Op::lhzux: case Op::lha: case Op::lhau: case Op::lhax: case Op::lhaux: case Op::lwz: case Op::lwzu:
    case Op::lwzx: case Op::lwzux: case Op::stb: case Op::stbu: case Op::stbx: case Op::stbux: case Op::sth:
    case Op::sthu: case Op::sthx: case Op::sthux: case Op::stw: case Op::stwu: case Op::stwx: case Op::stwux:
    case Op::lfs: case Op::lfsu: case Op::lfsx: case Op::lfsux: case Op::lfd: case Op::lfdu: case Op::lfdx:
    case Op::lfdux: case Op::stfs: case Op::stfsu: case Op::stfsx: case Op::stfsux: case Op::stfd: case Op::stfdu:
    case Op::stfdx: case Op::stfdux: case Op::stfiwx: case Op::psq_l: case Op::psq_lu: case Op::psq_lx:
    case Op::psq_lux: case Op::psq_st: case Op::psq_stu: case Op::psq_stx: case Op::psq_stux: case Op::lmw:
    case Op::stmw:
      return true;
    default:
      return false;
  }
}

// Floating-point arithmetic emitted inline (E2d; FuncGen::emit_fp). The FPSCR
// moves stay in the runtime.
bool is_fp_inline(Op op) {
  switch (op) {
    case Op::fadd: case Op::fsub: case Op::fmul: case Op::fdiv: case Op::fadds: case Op::fsubs: case Op::fmuls:
    case Op::fdivs: case Op::fmadd: case Op::fmsub: case Op::fnmadd: case Op::fnmsub: case Op::fmadds: case Op::fmsubs: case Op::fnmadds: case Op::fnmsubs: case Op::frsp:
    case Op::fctiw: case Op::fctiwz: case Op::fcmpu: case Op::fcmpo: case Op::fsel: case Op::fres:
    case Op::frsqrte: case Op::ps_add: case Op::ps_sub: case Op::ps_mul: case Op::ps_div: case Op::ps_muls0:
    case Op::ps_muls1: case Op::ps_madd: case Op::ps_msub: case Op::ps_nmadd: case Op::ps_nmsub:
    case Op::ps_madds0: case Op::ps_madds1: case Op::ps_sum0: case Op::ps_sum1: case Op::ps_sel: case Op::ps_cmpu0:
    case Op::ps_cmpu1: case Op::ps_cmpo0: case Op::ps_cmpo1: case Op::ps_res: case Op::ps_rsqrte:
      return true;
    default:
      return false;
  }
}

// V4: whether a single-entry function has resume labels (as FuncGen::run finds them).
bool has_resume_labels(const FunctionCfg& f) {
  auto label = [&](u32 a) { return f.contains(a) && f.reachable[(a - f.start) / 4] && a != f.start; };
  for (u32 a : f.resume_points)
    if (label(a)) return true;
  for (u32 a : f.back_edges)
    if (label(a)) return true;
  return false;
}

const HleSpec* find_spec(const WasmEmitOptions& o, const FunctionCfg& f) {
  for (const HleSpec& h : o.hle)
    if (h.address ? h.address == f.start : h.name == f.sym->name) return &h;
  return nullptr;
}

// A wasm function of the module.
// V4: Fast, a Plain function without resume labels (calls and the entry table
// reach it); Slow, its copy with them (resume entries only).
enum class WKind : u8 { Plain, Body, Stub, HleStub, Orig, Fast, Slow };
struct WFunc {
  WKind kind;
  std::size_t cfg;   // index in ProgramCfg::functions
  u32 entry = 0;     // Stub, HleStub, Plain: the entry address
  int slot = -1;     // table slot, for the callable ones
  int hle = -1;      // HleStub: replacement index; Plain: hook index (with `hook`)
  bool hook = false;
  u32 target = 0;    // Stub: its Body's function index; HleStub: its Orig's
};

struct Ctx {
  const ProgramCfg& p;
  const WasmEmitOptions& opt;
  std::unordered_map<u32, u32> callable;  // entry address -> function index (what a direct call calls)
  std::vector<int> start_slot;            // per cfg function: slot of its first entry's callable function
  WasmModule* out;
};

// ---- one translated function body ----
class FuncGen {
 public:
  enum class Mode { Plain, Body, Orig, Fast };

  FuncGen(Ctx& ctx, std::size_t cfg_index, Mode mode, int hook)
      : ctx_(ctx), f_(ctx.p.functions[cfg_index]), mode_(mode), hook_(hook), self_slot_(ctx.start_slot[cfg_index]) {}

  bool run(Fn& body, std::string* error);
  // Resume labels and their dispatcher index (valid after run).
  const std::vector<std::pair<u32, u32>>& resume() const { return resume_out_; }
  // The dispatcher index of an entry point (Body mode).
  u32 entry_segment(u32 e) const { return static_cast<u32>(seg_at_[(e - f_.start) / 4]); }

 private:
  struct Node {
    u8 kind = kNop, grp = gInline, pre = pNone;
    bool cond = false, mem = false;
    bool fpchk = false;  // checks MSR[FP] before it starts (floating-point unavailable)
    RegSet R = 0, W = 0;
    RegSet Din = 0, Lm = 0, Lin = 0, Lout = 0;
    int s0 = -1, s1 = -1, jt = -1;
  };

  // locals
  static constexpr u32 kCpu = 0;
  u32 L_label = 0, L_sp = 0, L_tc = 0, L_tgt = 0, L_tmp = 0, L_t = 0, L_w = 0, L_w2 = 0;
  // f64 temporaries (only in functions with inline floating-point arithmetic).
  u32 L_fa = 0, L_fb = 0, L_fc = 0, L_r = 0, L_f0 = 0, L_f1 = 0;
  bool uses_fp_ = false;
  // Q4: FPR halves (2 r + h) cached in f64 locals within a run of straight-line
  // code. Memory stays current (every write goes to both), so nothing is ever
  // spilled; a local is used only while it is known to hold the half's value
  // (fvalid_), which ends wherever something else may write FPRs (a call, a
  // safe point, the MSR[FP] check, rt.exec, psq_l's helper) and at every label.
  // FPSCR[RN, NI] == (0, 1), the fast paths' condition, is kept the same way.
  int fpr_local_[64];
  std::uint64_t fvalid_ = 0;
  bool fpok_known_ = false, fp_cond_ = false;
  u32 L_fpok = 0;
  void fp_reset() {
    fvalid_ = 0;
    fpok_known_ = false;
  }
  void fp_drop(u32 r) { fvalid_ &= ~(3ull << (2 * r)); }
  static u32 fp_half(u32 off) { return (off - kFpr) / 8; }
  void fp_get(u32 off);                  // pushes the half at Cpu offset `off` as an f64
  void fp_get_i64(u32 off) {             // ... as its i64 bits
    fp_get(off);
    body_->op(op::i64_reinterpret_f64);
  }
  void fp_load(u32 off);                 // makes the half's local current (on every path)
  void fp_put_f64(u32 off, u32 local);   // writes the half from an f64 local
  void fp_put_i64(u32 off, u32 local);   // ... from an i64 local
  void fp_reload(std::uint64_t halves);  // after rt.exec wrote them in memory
  void fpscr_ok();                       // pushes FPSCR[RN, NI] == (0, 1)
  // Q4: an integer compare whose CR field only the conditional branch right
  // after it reads (the field is dead after the branch): the compare sets no CR
  // bits, the branch tests the operands (cond_expr). fuse_[k] marks the compare.
  std::vector<bool> fuse_;
  int fused_ = -1;  // the compare at the node before the one being emitted, when fused
  void cr_liveness();
  int regloc_[reg::kCount];

  bool classify(std::string* error);
  void dataflow();
  RegSet dm(const Node& n) const { return n.pre == pIdle ? 0 : n.Din; }

  // emission helpers
  void reg_get(int r) { body_->get(static_cast<u32>(regloc_[r])); }
  void reg_set(int r) { body_->set(static_cast<u32>(regloc_[r])); }
  void flush(RegSet s);
  void reload(RegSet s);
  void materialize() {
    if (!p_) return;
    body_->get(L_t);
    body_->i64c(p_);
    body_->op(op::i64_add);
    body_->set(L_t);
    p_ = 0;
  }
  void ticks_expr() {
    body_->get(L_t);
    if (p_) {
      body_->i64c(p_);
      body_->op(op::i64_add);
    }
  }
  void store_ticks() {  // after materialize()
    body_->get(kCpu);
    body_->get(L_t);
    body_->mem(op::i64_store, 3, kTicks);
  }
  void load_ticks() {
    body_->get(kCpu);
    body_->mem(op::i64_load, 3, kTicks);
    body_->set(L_t);
  }
  void store_pc(u32 a) {
    body_->get(kCpu);
    body_->i32c(a);
    body_->mem(op::i32_store, 2, kPc);
  }
  void load_sp() {
    body_->get(kCpu);
    body_->mem(op::i32_load, 2, kGpr + 4);
    body_->set(L_sp);
  }
  void leave() {  // rt::leave(); return;
    glb(*body_, gUnwinding);
    body_->i32c(1);
    body_->mem(op::i32_store8, 0, 0);
    body_->op(op::return_);
  }
  void miss();               // an unwinding return arrived: resume block, or leave
  void check(u32 expect);    // if (c.pc != expect || g_unwinding) miss
  void jump(int seg);        // to a code segment, from the current position
  void cond_expr(const InsnInfo& x);
  void cr0_from(int r);      // CR0 from the value in register local r (record forms)
  void cmp_bits(bool is_signed, const std::function<void()>& x, const std::function<void()>& y);
  void set_cr_field(u32 field, const std::function<void()>& bits);
  void ea_d(Insn i);         // (rA|0) + d
  void ea_x(Insn i);         // (rA|0) + rB
  void emit_inline(const InsnInfo& x);
  void emit_fp(const InsnInfo& x);
  void emit_fp_fast(const InsnInfo& x, std::uint64_t dst);  // Q4, F1
  void emit_memh(const InsnInfo& x, RegSet flush_set);
  void exec_call(const InsnInfo& x);  // rt.exec of the instruction (no spills: callers decide)
  void emit_node(std::size_t k);
  void emit_fall();
  void emit_resume_block();
  void prologue(u32 e);

  Ctx& ctx_;
  const FunctionCfg& f_;
  Mode mode_;
  int hook_;
  int self_slot_;
  Fn* body_ = nullptr;
  std::vector<Node> nodes_;              // per word, plus the fall-off-the-end node
  std::vector<std::vector<int>> jt_succ_;
  std::vector<int> seg_at_;              // per word: its code segment
  std::vector<u32> seg_start_;           // per segment: first address (index 0 = resume block when present)
  std::vector<int> level_;               // per segment: the level of its block
  std::vector<std::pair<u32, u32>> resume_out_;
  std::vector<u32> resume_labels_;
  bool has_resume_ = false, dispatch_ = false;
  int loop_level_ = -1, pre_level_ = -1;
  int self_loop_ = -1;               // the current segment's own loop, if any
  std::vector<bool> self_loops_;     // per segment: it branches back to its own start
  // V4: structured loops instead of the dispatcher (a function without resume
  // labels: Fast, Orig): a segment that later segments branch back to starts a
  // wasm loop that ends after the last of them (loop_end_), and its backward
  // branches go to that loop (loop_at_: its level). Only when the loops nest and
  // nothing branches into one but at its start (plan_loops); else the dispatcher.
  bool structured_ = false;
  std::vector<int> loop_end_, loop_at_;
  bool plan_loops();
  void emit_region(int a, int b, bool is_loop, const std::function<void(int)>& emit_segment);
  int cur_seg_ = 0;
  s64 p_ = 0;  // instructions started since the ticks local was last updated
  RegSet used_ = 0;
};


bool falls_through(const InsnInfo& x) {
  switch (x.flow) {
    case Flow::Jump: case Flow::JumpTable: case Flow::Rfi: case Flow::Trap: return false;
    case Flow::TailCall: case Flow::IndirectJump: case Flow::Return: case Flow::CondReturn: return x.conditional;
    default: return true;
  }
}

// rt::on_entry, the entry safe point, and the return-path check of a function
// entry (runtime/native.h), on the in-memory state. `miss` runs when the entry
// hook or an interrupt at the entry safe point moved the guest elsewhere. The
// fast path (no hook, no event due) is one test (E2d): the check is needed only
// after the hook or the safe point ran, since a function is never entered while
// unwinding and nothing else changes c.pc there.
void emit_prologue(Fn& b, u32 e, const std::function<void()>& miss) {
  auto event_due = [&] {
    b.get(0);
    b.mem(op::i64_load, 3, kTicks);
    b.get(0);
    b.mem(op::i64_load, 3, kNextEvent);
    b.op(op::i64_ge_u);
  };
  b.get(0);
  b.i32c(e);
  b.mem(op::i32_store, 2, kPc);
  glb(b, gHook);
  b.mem(op::i32_load, 2, 0);
  event_due();
  b.op(op::i32_or);
  b.if_();
  glb(b, gHook);
  b.mem(op::i32_load, 2, 0);
  b.if_();
  b.get(0);
  b.i32c(e);
  b.call(iOnEntry);
  b.end();
  event_due();
  b.if_();
  b.get(0);
  b.call(iSafepoint);
  b.end();
  b.get(0);
  b.mem(op::i32_load, 2, kPc);
  b.i32c(e);
  b.op(op::i32_ne);
  glb(b, gUnwinding);
  b.mem(op::i32_load8_u, 0, 0);
  b.op(op::i32_or);
  b.if_();
  miss();
  b.end();
  b.end();
}

void emit_leave(Fn& b) {
  glb(b, gUnwinding);
  b.i32c(1);
  b.mem(op::i32_store8, 0, 0);
  b.op(op::return_);
}

// ---- FuncGen ----

bool FuncGen::classify(std::string* error) {
  const std::size_t n = f_.insns.size();
  nodes_.assign(n + 1, Node{});
  nodes_[n].grp = gFall;
  std::vector<bool> back(n, false), idle(n, false);
  for (u32 a : f_.back_edges) back[(a - f_.start) / 4] = true;
  for (u32 a : f_.idle_spins) idle[(a - f_.start) / 4] = true;
  auto local = [&](u32 target, int* out) {
    if (!f_.contains(target) || !f_.reachable[(target - f_.start) / 4]) return false;
    *out = static_cast<int>((target - f_.start) / 4);
    return true;
  };
  for (std::size_t k = 0; k < n; ++k) {
    if (!f_.reachable[k]) continue;
    const InsnInfo& x = f_.insns[k];
    Node& N = nodes_[k];
    Insn i{x.word};
    Effects e = ppc::effects(x.op, i);
    N.cond = x.conditional;
    N.pre = idle[k] ? pIdle : back[k] ? pBack : pNone;
    int next = static_cast<int>(k + 1);
    if (k + 1 < n && !f_.reachable[k + 1] && falls_through(x)) {
      if (error) *error = f_.sym->name + ": falls through into unreachable code";
      return false;
    }
    auto set = [&](u8 kind, u8 grp, bool with_effects) {
      N.kind = kind;
      N.grp = grp;
      if (with_effects) {
        N.R = e.reads;
        N.W = e.writes;
      }
    };
    switch (x.flow) {
      case Flow::Plain:
        if (x.op == Op::sc) {
          set(kSc, gCall, false);
          N.cond = false;
        } else if (x.op == Op::tw || x.op == Op::twi) {
          set(kTw, gHelper, true);
        } else if (x.op == Op::eciwx || x.op == Op::ecowx) {
          set(kTrapOp, gEnd, false);
        } else if (is_nop(x.op)) {
          set(kNop, gInline, false);
        } else if (ctx_.opt.inline_ops && is_fp_inline(x.op)) {
          set(kFp, gInline, true);
          uses_fp_ = true;
        } else if (ctx_.opt.inline_ops && (x.op == Op::lmw || x.op == Op::stmw)) {
          set(kMemH, gHelper, true);
          N.mem = true;
        } else if (ctx_.opt.inline_ops && is_inline(x.op, i)) {
          set(kInline, gInline, true);
          N.mem = is_memory(x.op);
        } else {
          set(kHelper, gHelper, true);
        }
        N.s0 = next;
        break;
      case Flow::Jump:
      case Flow::CondJump:
        set(x.flow == Flow::Jump ? kJump : kCJump, gInline, true);
        if (!local(x.target, &N.s0)) {
          if (error) *error = f_.sym->name + ": jump outside the function";
          return false;
        }
        if (x.flow == Flow::CondJump) N.s1 = next;
        break;
      case Flow::Call: case Flow::IndirectCall: case Flow::AbsoluteCall:
        set(kCall, gCall, true);
        N.s0 = next;
        break;
      case Flow::TailCall: case Flow::Return: case Flow::CondReturn: case Flow::IndirectJump:
        set(x.flow == Flow::TailCall ? kTail : x.flow == Flow::IndirectJump ? kIJump : kRet, gExit, true);
        if (N.cond) N.s0 = next;
        break;
      case Flow::JumpTable: {
        set(kJTab, gInline, true);
        N.jt = static_cast<int>(jt_succ_.size());
        jt_succ_.emplace_back();
        for (const auto& t : f_.tables)
          if (t.site == x.addr)
            for (u32 a : t.targets) {
              int s;
              if (!local(a, &s)) {
                if (error) *error = f_.sym->name + ": jump table target outside the function";
                return false;
              }
              jt_succ_.back().push_back(s);
            }
        break;
      }
      case Flow::Rfi:
        set(kRfi, gEnd, false);
        break;
      case Flow::Trap:
        set(kTrap, gEnd, false);
        break;
    }
    used_ |= N.R | N.W;
  }
  return true;
}

void FuncGen::dataflow() {
  const std::size_t n = f_.insns.size();
  auto each_succ = [&](const Node& N, auto&& fn) {
    if (N.s0 >= 0) fn(N.s0);
    if (N.s1 >= 0) fn(N.s1);
    if (N.jt >= 0)
      for (int s : jt_succ_[static_cast<std::size_t>(N.jt)]) fn(s);
  };
  // Forward: the registers whose local may differ from memory ("dirty").
  for (bool changed = true; changed;) {
    changed = false;
    for (std::size_t k = 0; k < n; ++k) {
      if (!f_.reachable[k]) continue;
      Node& N = nodes_[k];
      RegSet D = dm(N);
      switch (N.grp) {
        case gInline: D |= N.W; break;
        case gHelper: D &= ~(N.W | N.R); break;
        case gCall: D = N.cond ? (D | N.W) : 0; break;
        case gExit: D |= N.W; break;  // only the not-taken path continues
        default: break;
      }
      each_succ(N, [&](int s) {
        RegSet& in = nodes_[static_cast<std::size_t>(s)].Din;
        if ((in | D) != in) {
          in |= D;
          changed = true;
        }
      });
    }
  }
  // Backward: the registers whose local must hold the guest value ("live").
  // Spilling reads a local, so a register that may be dirty at a spill is live
  // there: every path to the spill then loads or writes it.
  const bool precise = ctx_.opt.precise_io;
  nodes_[n].Lin = nodes_[n].Din;
  for (bool changed = true; changed;) {
    changed = false;
    for (std::size_t kk = n; kk-- > 0;) {
      if (!f_.reachable[kk]) continue;
      Node& N = nodes_[kk];
      RegSet out = 0;
      each_succ(N, [&](int s) { out |= nodes_[static_cast<std::size_t>(s)].Lin; });
      RegSet D = dm(N), L;
      switch (N.grp) {
        case gInline: L = (out & ~N.W) | N.R | (precise && N.mem ? D : 0); break;
        case gHelper: L = (out & ~N.W) | (N.R & D) | (precise && N.mem ? D : 0); break;
        case gCall: case gExit: L = N.cond ? (((out | D) & ~N.W) | N.R) : ((D & ~N.W) | N.R); break;
        default: L = D; break;
      }
      // A safe point or an MSR[FP] check spills the dirty registers (so they are
      // live there) and reloads the instruction's own live set after its call.
      RegSet in = N.pre == pIdle ? N.Din : (N.pre == pBack || N.fpchk) ? (L | N.Din) : L;
      if (in != N.Lin) changed = true;
      N.Lin = in;
      N.Lout = out;
      N.Lm = L;
    }
  }
}

// Q4: which CR fields are live after each node (8 bits, field f = bit f), and
// so which integer compares can feed the next conditional branch directly.
// Conservative: calls, returns, helpers, safe points and the MSR[FP] check read
// every field; an instruction that changes CR in a way not listed here reads
// and keeps them all.
void FuncGen::cr_liveness() {
  const std::size_t n = f_.insns.size();
  constexpr unsigned kAll = 0xFF;
  std::vector<unsigned> R(n + 1, 0), W(n + 1, 0), in(n + 1, 0), out(n + 1, 0);
  R[n] = kAll;  // running off the end: the next function
  for (std::size_t k = 0; k < n; ++k) {
    if (!f_.reachable[k]) continue;
    const Node& N = nodes_[k];
    const InsnInfo& x = f_.insns[k];
    const Insn i{x.word};
    unsigned r = 0, w = 0;
    switch (N.kind) {
      case kJump: case kNop: case kMemH: case kJTab: break;
      case kCJump:
        if (!(i.bo() & 16)) r = 1u << (i.bi() >> 2);
        break;
      case kInline: case kFp: {
        const Op o = x.op;
        if (o == Op::cmp || o == Op::cmpi || o == Op::cmpl || o == Op::cmpli || o == Op::fcmpu || o == Op::fcmpo ||
            o == Op::ps_cmpu0 || o == Op::ps_cmpo0 || o == Op::ps_cmpu1 || o == Op::ps_cmpo1) {
          w = 1u << i.crfD();
        } else if (o == Op::mtcrf) {
          for (u32 f = 0; f < 8; ++f)
            if (i.crm() & (0x80u >> f)) w |= 1u << f;
        } else if (o == Op::mcrf) {
          r = 1u << i.crfS();
          w = 1u << i.crfD();
        } else if (N.R & ppc::bit(reg::CR)) {
          r = kAll;  // mfcr, the CR logical ops, ...
        } else if (N.W & ppc::bit(reg::CR)) {
          const bool int_rc = o == Op::andi_rc || o == Op::andis_rc || o == Op::addic_rc ||
                              (N.kind == kInline && i.rc() && o != Op::addi && o != Op::addis);
          if (int_rc && N.kind == kInline) w = 1u;  // CR0 of a record form
          else r = kAll;
        }
        break;
      }
      default: r = kAll; break;  // calls, returns, rt.exec, sc, traps, ...
    }
    if (N.pre != pNone || N.fpchk) r = kAll;  // an interrupt or the FPU exception sees CR
    R[k] = r;
    W[k] = w & ~r;
  }
  auto each_succ = [&](const Node& N, auto&& fn) {
    if (N.s0 >= 0) fn(N.s0);
    if (N.s1 >= 0) fn(N.s1);
    if (N.jt >= 0)
      for (int s : jt_succ_[static_cast<std::size_t>(N.jt)]) fn(s);
  };
  in[n] = kAll;
  for (bool changed = true; changed;) {
    changed = false;
    for (std::size_t k = n; k-- > 0;) {
      if (!f_.reachable[k]) continue;
      unsigned o = 0;
      each_succ(nodes_[k], [&](int s) { o |= in[static_cast<std::size_t>(s)]; });
      const unsigned v = (o & ~W[k]) | R[k];
      if (v != in[k] || o != out[k]) changed = true;
      in[k] = v;
      out[k] = o;
    }
  }
  fuse_.assign(n, false);
  for (std::size_t k = 0; k + 1 < n; ++k) {
    if (!f_.reachable[k] || !f_.reachable[k + 1]) continue;
    const Node& N = nodes_[k];
    const Node& B = nodes_[k + 1];
    const InsnInfo& x = f_.insns[k];
    const InsnInfo& y = f_.insns[k + 1];
    const Op o = x.op;
    if (N.kind != kInline || !(o == Op::cmp || o == Op::cmpi || o == Op::cmpl || o == Op::cmpli)) continue;
    if (N.pre != pNone || B.pre != pNone || B.fpchk) continue;
    if (seg_at_[k + 1] != seg_at_[k]) continue;  // a label: other paths reach the branch
    const Insn bi{y.word};
    const bool uses_cond = B.kind == kCJump || (B.cond && (B.kind == kCall || B.kind == kTail || B.kind == kRet ||
                                                           B.kind == kIJump));
    if (!uses_cond || y.op == Op::b) continue;
    if (!(bi.bo() & 4) || (bi.bo() & 16)) continue;  // CTR counted, or no CR test
    const u32 f = Insn{x.word}.crfD();
    if ((bi.bi() >> 2) != f || (out[k + 1] >> f & 1)) continue;
    fuse_[k] = true;
  }
}

void FuncGen::flush(RegSet s) {
  for (int r = 0; r < reg::kCount; ++r) {
    if (!(s & ppc::bit(r)) || regloc_[r] < 0) continue;
    body_->get(kCpu);
    reg_get(r);
    body_->mem(op::i32_store, 2, reg_offset(r));
    ++ctx_.out->stats.spills;
  }
}

void FuncGen::reload(RegSet s) {
  for (int r = 0; r < reg::kCount; ++r) {
    if (!(s & ppc::bit(r)) || regloc_[r] < 0) continue;
    body_->get(kCpu);
    body_->mem(op::i32_load, 2, reg_offset(r));
    reg_set(r);
    ++ctx_.out->stats.reloads;
  }
}

void FuncGen::miss() {
  if (has_resume_) {
    body_->i32c(0);
    body_->set(L_label);
    body_->br(loop_level_);
  } else {
    leave();
  }
}

void FuncGen::check(u32 expect) {
  body_->get(kCpu);
  body_->mem(op::i32_load, 2, kPc);
  body_->i32c(expect);
  body_->op(op::i32_ne);
  glb(*body_, gUnwinding);
  body_->mem(op::i32_load8_u, 0, 0);
  body_->op(op::i32_or);
  body_->if_();
  miss();
  body_->end();
}

void FuncGen::jump(int seg) {
  if (seg > cur_seg_) {
    body_->br(level_[static_cast<std::size_t>(seg)]);
  } else if (seg == cur_seg_ && self_loop_ >= 0) {
    body_->br(self_loop_);  // back to the start of this segment: its own loop (E2d)
  } else if (structured_) {
    body_->br(loop_at_[static_cast<std::size_t>(seg)]);  // V4: the loop that starts there
  } else {
    body_->i32c(seg);
    body_->set(L_label);
    body_->br(loop_level_);
  }
}

void FuncGen::cond_expr(const InsnInfo& x) {
  Insn i{x.word};
  if (x.op == Op::b) {
    body_->i32c(1);
    return;
  }
  if (fused_ >= 0) {  // Q4: the compare before this branch, tested directly
    const InsnInfo& cx = f_.insns[static_cast<std::size_t>(fused_)];
    const Insn ci{cx.word};
    fused_ = -1;
    const bool sgn = cx.op == Op::cmp || cx.op == Op::cmpi;
    const u32 bit = i.bi() & 3;  // LT GT EQ SO
    if (bit == 3) {
      reg_get(reg::SO);
    } else {
      reg_get(static_cast<int>(ci.rA()));
      if (cx.op == Op::cmp || cx.op == Op::cmpl) reg_get(static_cast<int>(ci.rB()));
      else body_->i32c(cx.op == Op::cmpi ? static_cast<u32>(ci.simm()) : ci.uimm());
      body_->op(bit == 0 ? (sgn ? op::i32_lt_s : op::i32_lt_u)
                         : bit == 1 ? (sgn ? op::i32_gt_s : op::i32_gt_u) : op::i32_eq);
    }
    if (!((i.bo() >> 3) & 1)) body_->op(op::i32_eqz);
    return;
  }
  u32 bo = i.bo();
  bool dec = !(bo & 4), test = !(bo & 16);
  if (dec) {
    reg_get(reg::CTR);
    body_->i32c(1);
    body_->op(op::i32_sub);
    body_->tee(static_cast<u32>(regloc_[reg::CTR]));
    if (bo & 2) {
      body_->op(op::i32_eqz);
    } else {
      body_->i32c(0);
      body_->op(op::i32_ne);
    }
  }
  if (test) {
    reg_get(reg::CR);
    u32 sh = 31 - i.bi();
    if (sh) {
      body_->i32c(sh);
      body_->op(op::i32_shr_u);
    }
    body_->i32c(1);
    body_->op(op::i32_and);
    if (!((bo >> 3) & 1)) body_->op(op::i32_eqz);
  }
  if (dec && test) body_->op(op::i32_and);
  if (!dec && !test) body_->i32c(1);
}

void FuncGen::cr0_from(int r) {
  // CR0 = LT GT EQ SO of r against 0 (sem::set_cr0), inline.
  Fn& b = *body_;
  reg_get(reg::CR);
  b.i32c(0x0FFFFFFFu);
  b.op(op::i32_and);
  reg_get(r);
  b.i32c(0x80000000u);
  b.op(op::i32_and);
  b.op(op::i32_or);
  reg_get(r);
  b.i32c(0);
  b.op(op::i32_gt_s);
  b.i32c(30);
  b.op(op::i32_shl);
  b.op(op::i32_or);
  reg_get(r);
  b.op(op::i32_eqz);
  b.i32c(29);
  b.op(op::i32_shl);
  b.op(op::i32_or);
  reg_get(reg::SO);
  b.i32c(28);
  b.op(op::i32_shl);
  b.op(op::i32_or);
  reg_set(reg::CR);
}

// LT GT EQ SO for a comparison of the values `x` and `y` push (sem::cmp_bits).
void FuncGen::cmp_bits(bool is_signed, const std::function<void()>& x, const std::function<void()>& y) {
  Fn& b = *body_;
  auto rel = [&](u8 opc, u32 sh) {
    x();
    y();
    b.op(opc);
    if (sh) {
      b.i32c(sh);
      b.op(op::i32_shl);
    }
  };
  rel(is_signed ? op::i32_lt_s : op::i32_lt_u, 3);
  rel(is_signed ? op::i32_gt_s : op::i32_gt_u, 2);
  b.op(op::i32_or);
  rel(op::i32_eq, 1);
  b.op(op::i32_or);
  reg_get(reg::SO);
  b.op(op::i32_or);
}

void FuncGen::set_cr_field(u32 field, const std::function<void()>& bits) {
  u32 sh = 28 - 4 * field;
  reg_get(reg::CR);
  body_->i32c(~(0xFu << sh));
  body_->op(op::i32_and);
  bits();
  if (sh) {
    body_->i32c(sh);
    body_->op(op::i32_shl);
  }
  body_->op(op::i32_or);
  reg_set(reg::CR);
}

void FuncGen::ea_d(Insn i) {
  if (i.rA()) {
    reg_get(static_cast<int>(i.rA()));
    if (i.simm()) {
      body_->i32c(i.simm());
      body_->op(op::i32_add);
    }
  } else {
    body_->i32c(i.simm());
  }
}

void FuncGen::ea_x(Insn i) {
  if (i.rA()) {
    reg_get(static_cast<int>(i.rA()));
    reg_get(static_cast<int>(i.rB()));
    body_->op(op::i32_add);
  } else {
    reg_get(static_cast<int>(i.rB()));
  }
}

void FuncGen::emit_inline(const InsnInfo& x) {
  Fn& b = *body_;
  const Insn i{x.word};
  const Op o = x.op;
  const u32 a = x.addr;
  const int rD = static_cast<int>(i.rD()), rA = static_cast<int>(i.rA()), rB = static_cast<int>(i.rB()),
            rS = static_cast<int>(i.rS());
  auto G = [&](int r) { reg_get(r); };
  auto S = [&](int r) { reg_set(r); };
  auto rc = [&](int r) {
    if (i.rc()) cr0_from(r);
  };
  auto bin = [&](u8 opcode) { b.op(opcode); };
  auto not_ = [&] {
    b.i32c(-1);
    b.op(op::i32_xor);
  };
  // rD = A + B + carry-in, XER[CA] = the carry out (sem::add_with).
  auto wide = [&](const std::function<void()>& A, const std::function<void()>& B, int cin /*0, 1, -1 = CA*/) {
    A();
    b.op(op::i64_extend_i32_u);
    B();
    b.op(op::i64_extend_i32_u);
    b.op(op::i64_add);
    if (cin == 1) {
      b.i64c(1);
      b.op(op::i64_add);
    } else if (cin < 0) {
      G(reg::CA);
      b.op(op::i64_extend_i32_u);
      b.op(op::i64_add);
    }
    b.tee(L_w);
    b.op(op::i32_wrap_i64);
    b.set(L_tmp);
    b.get(L_w);
    b.i64c(32);
    b.op(op::i64_shr_u);
    b.op(op::i32_wrap_i64);
    S(reg::CA);
    b.get(L_tmp);
    S(rD);
  };
  auto rA_ = [&] { G(rA); };
  auto rB_ = [&] { G(rB); };
  auto nrA = [&] {
    G(rA);
    not_();
  };
  auto zero = [&] { b.i32c(0); };
  auto ones = [&] { b.i32c(-1); };
  auto simm = [&] { b.i32c(i.simm()); };
  // Effective address on the stack after the cpu argument. Update forms keep
  // it in L_tmp for the base register write that follows the access.
  auto address = [&](int mode /*0 d, 1 du, 2 x, 3 xu*/) {
    if (mode == 1 || mode == 3) {
      G(rA);
      if (mode == 1) b.i32c(i.simm());
      else G(rB);
      b.op(op::i32_add);
      b.set(L_tmp);
      b.get(kCpu);
      b.get(L_tmp);
    } else {
      b.get(kCpu);
      if (mode == 0) ea_d(i);
      else ea_x(i);
    }
  };
  auto update = [&](int mode) {
    if (mode == 1 || mode == 3) {
      b.get(L_tmp);
      S(rA);
    }
  };
  auto load = [&](u32 helper, bool sext16, int mode) {
    address(mode);
    b.i32c(a);
    ticks_expr();
    b.call(helper);
    if (sext16) b.op(op::i32_extend16_s);
    S(rD);
    update(mode);
  };
  auto store = [&](u32 helper, int mode) {
    address(mode);
    G(rS);
    b.i32c(a);
    ticks_expr();
    b.call(helper);
    update(mode);
  };
  // Effective address into L_tmp (FP loads and stores).
  auto ea_tmp = [&](int mode) {
    if (mode == 1 || mode == 3) {
      G(rA);
      if (mode == 1) b.i32c(i.simm());
      else G(rB);
      b.op(op::i32_add);
    } else if (mode == 0) {
      ea_d(i);
    } else {
      ea_x(i);
    }
    b.set(L_tmp);
  };
  auto rd32_tmp = [&](u32 off) {  // pushes the big-endian word at L_tmp + off
    b.get(kCpu);
    b.get(L_tmp);
    if (off) {
      b.i32c(off);
      b.op(op::i32_add);
    }
    b.i32c(a);
    ticks_expr();
    b.call(hRd32);
  };
  auto wr32_tmp = [&](u32 off, const std::function<void()>& value) {
    b.get(kCpu);
    b.get(L_tmp);
    if (off) {
      b.i32c(off);
      b.op(op::i32_add);
    }
    value();
    b.i32c(a);
    ticks_expr();
    b.call(hWr32);
  };
  auto sign_op = [&](int sop /*0 move, 1 negate, 2 clear (abs), 3 set (nabs)*/) {
    if (sop == 0) return;
    if (sop == 2) b.i64c(0x7FFFFFFFFFFFFFFFll);
    else b.i64c(static_cast<s64>(0x8000000000000000ull));
    b.op(sop == 1 ? op::i64_xor : sop == 2 ? op::i64_and : op::i64_or);
  };
  auto cr_bit = [&](u32 bit) {
    G(reg::CR);
    if (31 - bit) {
      b.i32c(31 - bit);
      b.op(op::i32_shr_u);
    }
    b.i32c(1);
    b.op(op::i32_and);
  };
  switch (o) {
    case Op::addi: case Op::addis: {
      s32 v = o == Op::addi ? i.simm() : static_cast<s32>(i.uimm() << 16);
      if (rA) {
        G(rA);
        if (v) {
          b.i32c(v);
          bin(op::i32_add);
        }
      } else {
        b.i32c(v);
      }
      S(rD);
      return;
    }
    case Op::add: G(rA); G(rB); bin(op::i32_add); S(rD); rc(rD); return;
    case Op::subf: G(rB); G(rA); bin(op::i32_sub); S(rD); rc(rD); return;
    case Op::neg: b.i32c(0); G(rA); bin(op::i32_sub); S(rD); rc(rD); return;
    case Op::mullw: G(rA); G(rB); bin(op::i32_mul); S(rD); rc(rD); return;
    case Op::mulli: G(rA); b.i32c(i.simm()); bin(op::i32_mul); S(rD); return;
    case Op::mulhw: case Op::mulhwu: {
      u8 ext = o == Op::mulhw ? op::i64_extend_i32_s : op::i64_extend_i32_u;
      G(rA);
      bin(ext);
      G(rB);
      bin(ext);
      bin(op::i64_mul);
      b.i64c(32);
      bin(op::i64_shr_u);
      bin(op::i32_wrap_i64);
      S(rD);
      rc(rD);
      return;
    }
    case Op::divw: G(rA); G(rB); b.call(hDivw); S(rD); rc(rD); return;
    case Op::divwu: G(rA); G(rB); b.call(hDivwu); S(rD); rc(rD); return;
    case Op::addc: wide(rA_, rB_, 0); rc(rD); return;
    case Op::adde: wide(rA_, rB_, -1); rc(rD); return;
    case Op::addze: wide(rA_, zero, -1); rc(rD); return;
    case Op::addme: wide(rA_, ones, -1); rc(rD); return;
    case Op::subfc: wide(nrA, rB_, 1); rc(rD); return;
    case Op::subfe: wide(nrA, rB_, -1); rc(rD); return;
    case Op::subfze: wide(nrA, zero, -1); rc(rD); return;
    case Op::subfme: wide(nrA, ones, -1); rc(rD); return;
    case Op::addic: wide(rA_, simm, 0); return;
    case Op::addic_rc: wide(rA_, simm, 0); cr0_from(rD); return;
    case Op::subfic: wide(nrA, simm, 1); return;
    case Op::cmp:
      set_cr_field(i.crfD(), [&] { cmp_bits(true, rA_, rB_); });
      return;
    case Op::cmpi:
      set_cr_field(i.crfD(), [&] { cmp_bits(true, rA_, [&] { b.i32c(i.simm()); }); });
      return;
    case Op::cmpl:
      set_cr_field(i.crfD(), [&] { cmp_bits(false, rA_, rB_); });
      return;
    case Op::cmpli:
      set_cr_field(i.crfD(), [&] { cmp_bits(false, rA_, [&] { b.i32c(i.uimm()); }); });
      return;
    case Op::and_: G(rS); G(rB); bin(op::i32_and); S(rA); rc(rA); return;
    case Op::andc: G(rS); G(rB); not_(); bin(op::i32_and); S(rA); rc(rA); return;
    case Op::or_: G(rS); G(rB); bin(op::i32_or); S(rA); rc(rA); return;
    case Op::orc: G(rS); G(rB); not_(); bin(op::i32_or); S(rA); rc(rA); return;
    case Op::xor_: G(rS); G(rB); bin(op::i32_xor); S(rA); rc(rA); return;
    case Op::nand: G(rS); G(rB); bin(op::i32_and); not_(); S(rA); rc(rA); return;
    case Op::nor: G(rS); G(rB); bin(op::i32_or); not_(); S(rA); rc(rA); return;
    case Op::eqv: G(rS); G(rB); bin(op::i32_xor); not_(); S(rA); rc(rA); return;
    case Op::extsb: G(rS); bin(op::i32_extend8_s); S(rA); rc(rA); return;
    case Op::extsh: G(rS); bin(op::i32_extend16_s); S(rA); rc(rA); return;
    case Op::cntlzw: G(rS); bin(op::i32_clz); S(rA); rc(rA); return;
    case Op::ori: case Op::oris: case Op::xori: case Op::xoris: {
      u32 v = (o == Op::oris || o == Op::xoris) ? i.uimm() << 16 : i.uimm();
      G(rS);
      if (v) {
        b.i32c(v);
        bin(o == Op::ori || o == Op::oris ? op::i32_or : op::i32_xor);
      }
      S(rA);
      return;
    }
    case Op::andi_rc: case Op::andis_rc:
      G(rS);
      b.i32c(o == Op::andi_rc ? i.uimm() : i.uimm() << 16);
      bin(op::i32_and);
      S(rA);
      cr0_from(rA);
      return;
    case Op::rlwinm: case Op::rlwnm: case Op::rlwimi: {
      u32 m = ppc::rot_mask(i.mb(), i.me());
      G(rS);
      if (o == Op::rlwnm) {
        G(rB);  // wasm rotates modulo 32, like the & 31
        bin(op::i32_rotl);
      } else if (i.sh()) {
        b.i32c(i.sh());
        bin(op::i32_rotl);
      }
      if (m != 0xFFFFFFFFu) {
        b.i32c(m);
        bin(op::i32_and);
      }
      if (o == Op::rlwimi) {
        G(rA);
        b.i32c(~m);
        bin(op::i32_and);
        bin(op::i32_or);
      }
      S(rA);
      rc(rA);
      return;
    }
    case Op::slw: case Op::srw:
      // n = rB & 63: 0 when n > 31, else rS shifted by n (wasm shifts modulo 32).
      b.i32c(0);
      G(rS);
      G(rB);
      bin(o == Op::slw ? op::i32_shl : op::i32_shr_u);
      G(rB);
      b.i32c(32);
      bin(op::i32_and);
      bin(op::select);
      S(rA);
      rc(rA);
      return;
    case Op::srawi: {
      u32 n = i.sh();
      if (n == 0) {
        b.i32c(0);
        S(reg::CA);
        G(rS);
      } else {
        G(rS);
        b.i32c(0);
        bin(op::i32_lt_s);
        G(rS);
        b.i32c((1u << n) - 1);
        bin(op::i32_and);
        b.i32c(0);
        bin(op::i32_ne);
        bin(op::i32_and);
        S(reg::CA);
        G(rS);
        b.i32c(n);
        bin(op::i32_shr_s);
      }
      S(rA);
      rc(rA);
      return;
    }
    case Op::crand: case Op::crandc: case Op::creqv: case Op::crnand: case Op::crnor: case Op::cror:
    case Op::crorc: case Op::crxor: {
      cr_bit(i.crbA());
      cr_bit(i.crbB());
      auto flip = [&] {
        b.i32c(1);
        bin(op::i32_xor);
      };
      switch (o) {
        case Op::crand: bin(op::i32_and); break;
        case Op::crandc: flip(); bin(op::i32_and); break;
        case Op::creqv: bin(op::i32_xor); flip(); break;
        case Op::crnand: bin(op::i32_and); flip(); break;
        case Op::crnor: bin(op::i32_or); flip(); break;
        case Op::cror: bin(op::i32_or); break;
        case Op::crorc: flip(); bin(op::i32_or); break;
        default: bin(op::i32_xor); break;
      }
      b.set(L_tmp);
      u32 sh = 31 - i.crbD();
      G(reg::CR);
      b.i32c(~(1u << sh));
      bin(op::i32_and);
      b.get(L_tmp);
      if (sh) {
        b.i32c(sh);
        bin(op::i32_shl);
      }
      bin(op::i32_or);
      S(reg::CR);
      return;
    }
    case Op::mcrf:
      set_cr_field(i.crfD(), [&] {
        G(reg::CR);
        u32 sh = 28 - 4 * i.crfS();
        if (sh) {
          b.i32c(sh);
          bin(op::i32_shr_u);
        }
        b.i32c(0xF);
        bin(op::i32_and);
      });
      return;
    case Op::mfcr: G(reg::CR); S(rD); return;
    case Op::mtcrf: {
      u32 m = 0;
      for (u32 f = 0; f < 8; ++f)
        if (i.crm() & (0x80u >> f)) m |= 0xF0000000u >> (4 * f);
      G(reg::CR);
      b.i32c(~m);
      bin(op::i32_and);
      G(rS);
      b.i32c(m);
      bin(op::i32_and);
      bin(op::i32_or);
      S(reg::CR);
      return;
    }
    case Op::mfspr: {
      const u32 n = i.spr();
      if (n == ppc::spr::LR || n == ppc::spr::CTR) {
        G(n == ppc::spr::LR ? reg::LR : reg::CTR);
      } else if (n == ppc::spr::XER) {  // Cpu::xer()
        G(reg::SO);
        b.i32c(31);
        bin(op::i32_shl);
        b.get(kCpu);
        b.mem(op::i32_load, 2, kOv);
        b.i32c(30);
        bin(op::i32_shl);
        bin(op::i32_or);
        G(reg::CA);
        b.i32c(29);
        bin(op::i32_shl);
        bin(op::i32_or);
        b.get(kCpu);
        b.mem(op::i32_load, 2, kXerLow);
        b.i32c(0x1FFFFFFF);
        bin(op::i32_and);
        bin(op::i32_or);
      } else {
        b.get(kCpu);
        b.mem(op::i32_load, 2, kSpr + 4 * (n & 1023));
      }
      S(rD);
      return;
    }
    case Op::mtspr: {
      const u32 n = i.spr();
      if (n == ppc::spr::LR || n == ppc::spr::CTR) {
        G(rS);
        S(n == ppc::spr::LR ? reg::LR : reg::CTR);
      } else if (n == ppc::spr::XER) {  // Cpu::set_xer()
        G(rS);
        b.i32c(31);
        bin(op::i32_shr_u);
        S(reg::SO);
        b.get(kCpu);
        G(rS);
        b.i32c(30);
        bin(op::i32_shr_u);
        b.i32c(1);
        bin(op::i32_and);
        b.mem(op::i32_store, 2, kOv);
        G(rS);
        b.i32c(29);
        bin(op::i32_shr_u);
        b.i32c(1);
        bin(op::i32_and);
        S(reg::CA);
        b.get(kCpu);
        G(rS);
        b.i32c(0x1FFFFFFF);
        bin(op::i32_and);
        b.mem(op::i32_store, 2, kXerLow);
      } else {
        b.get(kCpu);
        G(rS);
        b.mem(op::i32_store, 2, kSpr + 4 * (n & 1023));
      }
      return;
    }
    case Op::mfmsr:
      b.get(kCpu);
      b.mem(op::i32_load, 2, kMsr);
      S(rD);
      return;
    case Op::lbz: load(hRd8, false, 0); return;
    case Op::lbzu: load(hRd8, false, 1); return;
    case Op::lbzx: load(hRd8, false, 2); return;
    case Op::lbzux: load(hRd8, false, 3); return;
    case Op::lhz: load(hRd16, false, 0); return;
    case Op::lhzu: load(hRd16, false, 1); return;
    case Op::lhzx: load(hRd16, false, 2); return;
    case Op::lhzux: load(hRd16, false, 3); return;
    case Op::lha: load(hRd16, true, 0); return;
    case Op::lhau: load(hRd16, true, 1); return;
    case Op::lhax: load(hRd16, true, 2); return;
    case Op::lhaux: load(hRd16, true, 3); return;
    case Op::lwz: load(hRd32, false, 0); return;
    case Op::lwzu: load(hRd32, false, 1); return;
    case Op::lwzx: load(hRd32, false, 2); return;
    case Op::lwzux: load(hRd32, false, 3); return;
    case Op::stb: store(hWr8, 0); return;
    case Op::stbu: store(hWr8, 1); return;
    case Op::stbx: store(hWr8, 2); return;
    case Op::stbux: store(hWr8, 3); return;
    case Op::sth: store(hWr16, 0); return;
    case Op::sthu: store(hWr16, 1); return;
    case Op::sthx: store(hWr16, 2); return;
    case Op::sthux: store(hWr16, 3); return;
    case Op::stw: store(hWr32, 0); return;
    case Op::stwu: store(hWr32, 1); return;
    case Op::stwx: store(hWr32, 2); return;
    case Op::stwux: store(hWr32, 3); return;
    // ---- floating point: loads, stores and moves of FPR bits ----
    case Op::lfs: case Op::lfsu: case Op::lfsx: case Op::lfsux: {
      int mode = o == Op::lfs ? 0 : o == Op::lfsu ? 1 : o == Op::lfsx ? 2 : 3;
      ea_tmp(mode);
      b.get(kCpu);  // Q4: read32 and fp::single_to_double in one helper
      b.get(L_tmp);
      b.i32c(a);
      ticks_expr();
      b.call(hLfs);
      b.set(L_w);
      fp_put_i64(ps0(i.rD()), L_w);
      fp_put_i64(ps1(i.rD()), L_w);
      update(mode);
      return;
    }
    case Op::lfd: case Op::lfdu: case Op::lfdx: case Op::lfdux: {
      int mode = o == Op::lfd ? 0 : o == Op::lfdu ? 1 : o == Op::lfdx ? 2 : 3;
      ea_tmp(mode);
      b.get(kCpu);  // read64 (h.rd64: the high word, then the low word, where they are not plain memory)
      b.get(L_tmp);
      b.i32c(a);
      ticks_expr();
      b.call(hRd64);
      b.set(L_w);
      fp_put_i64(ps0(i.rD()), L_w);
      update(mode);
      return;
    }
    case Op::stfs: case Op::stfsu: case Op::stfsx: case Op::stfsux: {
      int mode = o == Op::stfs ? 0 : o == Op::stfsu ? 1 : o == Op::stfsx ? 2 : 3;
      ea_tmp(mode);
      b.get(kCpu);  // Q4: fp::double_to_single and write32 in one helper
      b.get(L_tmp);
      fp_get_i64(ps0(i.rS()));
      b.i32c(a);
      ticks_expr();
      b.call(hStfs);
      update(mode);
      return;
    }
    case Op::stfd: case Op::stfdu: case Op::stfdx: case Op::stfdux: case Op::stfiwx: {
      int mode = (o == Op::stfd) ? 0 : o == Op::stfdu ? 1 : (o == Op::stfdx || o == Op::stfiwx) ? 2 : 3;
      ea_tmp(mode);
      if (o != Op::stfiwx) {  // write64 (h.wr64: the high word, then the low word, where they are not plain memory)
        b.get(kCpu);
        b.get(L_tmp);
        fp_get_i64(ps0(i.rS()));
        b.i32c(a);
        ticks_expr();
        b.call(hWr64);
      } else {
        wr32_tmp(0, [&] {
          fp_get_i64(ps0(i.rS()));
          b.op(op::i32_wrap_i64);
        });
      }
      update(mode);
      return;
    }
    case Op::fmr: case Op::fneg: case Op::fabs: case Op::fnabs:
      fp_get_i64(ps0(i.rB()));
      sign_op(o == Op::fmr ? 0 : o == Op::fneg ? 1 : o == Op::fabs ? 2 : 3);
      b.set(L_w);
      fp_put_i64(ps0(i.rD()), L_w);
      return;
    case Op::ps_mr: case Op::ps_neg: case Op::ps_abs: case Op::ps_nabs: case Op::ps_merge00: case Op::ps_merge01:
    case Op::ps_merge10: case Op::ps_merge11: {
      // Both halves are read before either is written (rD may be rA or rB).
      u32 src0 = ps0(i.rB()), src1 = ps1(i.rB());
      int sop = 0;
      switch (o) {
        case Op::ps_neg: sop = 1; break;
        case Op::ps_abs: sop = 2; break;
        case Op::ps_nabs: sop = 3; break;
        case Op::ps_merge00: src0 = ps0(i.rA()); src1 = ps0(i.rB()); break;
        case Op::ps_merge01: src0 = ps0(i.rA()); src1 = ps1(i.rB()); break;
        case Op::ps_merge10: src0 = ps1(i.rA()); src1 = ps0(i.rB()); break;
        case Op::ps_merge11: src0 = ps1(i.rA()); src1 = ps1(i.rB()); break;
        default: break;
      }
      fp_get_i64(src0);
      sign_op(sop);
      b.set(L_w);
      fp_get_i64(src1);
      sign_op(sop);
      b.set(L_w2);
      fp_put_i64(ps0(i.rD()), L_w);
      fp_put_i64(ps1(i.rD()), L_w2);
      return;
    }
    case Op::sraw:
      // sem::sra_by with n = rB & 63: CA = rS < 0 and (n > 31 or a 1 bit shifted out).
      G(rB);
      b.i32c(63);
      bin(op::i32_and);
      b.set(L_tmp);
      G(rS);
      b.i32c(0);
      bin(op::i32_lt_s);
      b.get(L_tmp);
      b.i32c(31);
      bin(op::i32_gt_u);
      G(rS);
      b.i32c(1);
      b.get(L_tmp);
      bin(op::i32_shl);
      b.i32c(1);
      bin(op::i32_sub);
      bin(op::i32_and);
      b.i32c(0);
      bin(op::i32_ne);
      bin(op::i32_or);
      bin(op::i32_and);
      S(reg::CA);
      G(rS);
      b.i32c(31);
      b.get(L_tmp);
      b.get(L_tmp);
      b.i32c(31);
      bin(op::i32_gt_u);
      bin(op::select);
      bin(op::i32_shr_s);
      S(rA);
      rc(rA);
      return;
    case Op::psq_l: case Op::psq_lu: case Op::psq_lx: case Op::psq_lux: case Op::psq_st: case Op::psq_stu:
    case Op::psq_stx: case Op::psq_stux: {
      // In-module helpers (h.psq_l, h.psq_st) on the effective address; update
      // forms write rA after the access, as sem does.
      const bool st = o == Op::psq_st || o == Op::psq_stu || o == Op::psq_stx || o == Op::psq_stux;
      const bool upd = o == Op::psq_lu || o == Op::psq_lux || o == Op::psq_stu || o == Op::psq_stux;
      const bool xf = o == Op::psq_lx || o == Op::psq_lux || o == Op::psq_stx || o == Op::psq_stux;
      const u32 w = xf ? i.psx_w() : i.ps_w(), q = xf ? i.psx_i() : i.ps_i();
      if (upd || rA) G(rA);
      if (xf) {
        G(rB);
        if (upd || rA) bin(op::i32_add);
      } else if (i.ps_d() || !(upd || rA)) {
        b.i32c(i.ps_d());
        if (upd || rA) bin(op::i32_add);
      }
      b.set(L_tmp);
      b.get(kCpu);
      b.get(L_tmp);
      b.i32c(kFpr + 16 * static_cast<u32>(rD));
      b.i32c(q | (w << 3));
      b.i32c(a);
      ticks_expr();
      b.call(st ? hPsqSt : hPsqL);
      if (!st) fp_drop(static_cast<u32>(rD));  // Q4: the helper wrote it in memory
      if (upd) {
        b.get(L_tmp);
        S(rA);
      }
      return;
    }
    default:
      return;  // is_inline() and this switch list the same opcodes
  }
}

// ---- floating point (E2d) ----
//
// FPRs stay in the Cpu struct as raw IEEE bits. Every arithmetic instruction
// gets an inline fast path on WebAssembly's IEEE f64 arithmetic, taken only where
// its result is provably the exact model's (ppc/fp.h); anything else calls the
// runtime (rt.exec: the interpreter's semantics), which recomputes the whole
// instruction from the registers the fast path has not touched. The rules:
// - FPSCR[RN] = 0 and FPSCR[NI] = 1 (the game sets NI in OSInit): the fast path
//   is written for round to nearest with NI; any other FPSCR goes to the runtime.
// - A NaN made by WebAssembly arithmetic never reaches guest state (its payload is
//   not deterministic): every NaN result goes to the runtime, which applies the
//   PowerPC NaN rules. NaN operands produce NaN results, so they go there too.
//   Only exact bit patterns are stored (loads, stores, locals and the sign
//   operations abs/neg/copysign preserve bits).
// - Double results (fadd, fsub, fmul, fdiv): r is the correctly rounded double;
//   a subnormal r flushes to a signed zero (NI), exactly where the model flushes.
// - Single results: r = RN53(x) of the exact value x, as a double. |r| < 2^-126
//   implies |x| < 2^-126 (rounding is monotonic and 2^-126 is a double): flush to
//   a signed zero. |r| = 2^-126 is ambiguous: runtime. Otherwise f32.demote
//   (round to nearest even, overflow to infinity) gives RN24(r), which is RN24(x)
//   unless r is a midpoint between two singles that x is not (a midpoint is a
//   double, so rounding x to double can reach one but never cross it). On a
//   midpoint: if r = x (an exact sum or product) r is demoted as is; a sum's
//   exact error comes from TwoSum (Knuth; exact in binary round-to-nearest without
//   overflow, which a finite midpoint below 2^128 rules out), and a nonzero
//   error moves r one double ulp towards x, off the midpoint and onto x's side,
//   so the demotion rounds as x would; a quotient that lands on one: runtime.
// - Products a * force25(c): force25(c) has at most 26 significant bits, so the
//   product is exact in double when a has at most 27 (its low 26 fraction bits
//   are zero; always so for single-precision a). Fused single multiply-adds need
//   that (else runtime); then r = RN53(p + b) is one rounding of the exact value.
//   A product that underflows double is off by less than 2^-1074; that only
//   matters if x is within that distance of a single midpoint, and then r is that
//   midpoint, where an underflowed product goes to the runtime; so does one that
//   underflowed to zero (zero sums take their sign from the exact operands). Double
//   fused multiply-adds need sig(a) + sig(c) <= 53 and no underflow (a product
//   below 2^-1022 from nonzero operands goes to the runtime).
// - Compares, fsel, fctiw(z), fres/frsqrte and the paired-single forms follow
//   the same rules; see each case.
namespace {
constexpr u64 kF64MinSingleNormal = 0x3810000000000000ull;  // 2^-126
constexpr u64 kF64MinNormal = 0x0010000000000000ull;        // 2^-1022
constexpr u64 kF64One = 0x3FF0000000000000ull;
constexpr u64 kDefaultNaNBits = 0x7FF8000000000000ull;
}  // namespace

void FuncGen::fp_get(u32 off) {
  const u32 h = fp_half(off);
  const int l = fpr_local_[h];
  if (l >= 0 && (fvalid_ >> h & 1)) {
    body_->get(static_cast<u32>(l));
    return;
  }
  body_->get(kCpu);
  body_->mem(op::f64_load, 3, off);
  if (l >= 0 && !fp_cond_) {  // inside a conditional path a load stays a plain load
    body_->tee(static_cast<u32>(l));
    fvalid_ |= 1ull << h;
  }
}

void FuncGen::fp_load(u32 off) {
  const u32 h = fp_half(off);
  const int l = fpr_local_[h];
  if (l < 0 || (fvalid_ >> h & 1)) return;
  body_->get(kCpu);
  body_->mem(op::f64_load, 3, off);
  body_->set(static_cast<u32>(l));
  fvalid_ |= 1ull << h;
}

void FuncGen::fp_put_f64(u32 off, u32 local) {
  body_->get(kCpu);
  body_->get(local);
  body_->mem(op::f64_store, 3, off);
  const u32 h = fp_half(off);
  if (const int l = fpr_local_[h]; l >= 0) {
    body_->get(local);
    body_->set(static_cast<u32>(l));
    fvalid_ |= 1ull << h;
  }
}

void FuncGen::fp_put_i64(u32 off, u32 local) {
  body_->get(kCpu);
  body_->get(local);
  body_->mem(op::i64_store, 3, off);
  const u32 h = fp_half(off);
  if (const int l = fpr_local_[h]; l >= 0) {
    body_->get(local);
    body_->op(op::f64_reinterpret_i64);
    body_->set(static_cast<u32>(l));
    fvalid_ |= 1ull << h;
  }
}

void FuncGen::fp_reload(std::uint64_t halves) {
  for (u32 h = 0; h < 64; ++h) {
    if (!(halves >> h & 1) || fpr_local_[h] < 0) continue;
    body_->get(kCpu);
    body_->mem(op::f64_load, 3, kFpr + 8 * h);
    body_->set(static_cast<u32>(fpr_local_[h]));
  }
}

// Called first thing in an instruction's fast path, before any branch: what it
// computes holds on every path out of the instruction.
void FuncGen::fpscr_ok() {
  if (fpok_known_) {
    body_->get(L_fpok);
    return;
  }
  body_->get(kCpu);
  body_->mem(op::i32_load, 2, kFpscr);
  body_->i32c(7);
  body_->op(op::i32_and);
  body_->i32c(4);
  body_->op(op::i32_eq);
  body_->tee(L_fpok);
  fpok_known_ = true;
}

// Q4: the FPR halves (2 r + h) an instruction emit_fp computes reads and writes.
static void fp_halves(Op o, Insn i, std::uint64_t* src, std::uint64_t* dst) {
  auto H = [](u32 r, u32 h) { return 1ull << (2 * r + h); };
  const u32 a = i.rA(), b = i.rB(), c = i.rC(), d = i.rD();
  const std::uint64_t d0 = H(d, 0), dd = H(d, 0) | H(d, 1);
  std::uint64_t S = 0, D = 0;
  switch (o) {
    case Op::fcmpu: case Op::fcmpo: case Op::ps_cmpu0: case Op::ps_cmpo0: S = H(a, 0) | H(b, 0); break;
    case Op::ps_cmpu1: case Op::ps_cmpo1: S = H(a, 1) | H(b, 1); break;
    case Op::fsel: S = H(a, 0) | H(b, 0) | H(c, 0); D = d0; break;
    case Op::ps_sel: S = H(a, 0) | H(a, 1) | H(b, 0) | H(b, 1) | H(c, 0) | H(c, 1); D = dd; break;
    case Op::fctiwz: case Op::fctiw: case Op::frsqrte: S = H(b, 0); D = d0; break;
    case Op::fadd: case Op::fsub: case Op::fdiv: S = H(a, 0) | H(b, 0); D = d0; break;
    case Op::fmul: S = H(a, 0) | H(c, 0); D = d0; break;
    case Op::fmadd: case Op::fmsub: case Op::fnmadd: case Op::fnmsub: S = H(a, 0) | H(b, 0) | H(c, 0); D = d0; break;
    case Op::fadds: case Op::fsubs: case Op::fdivs: S = H(a, 0) | H(b, 0); D = dd; break;
    case Op::fmuls: S = H(a, 0) | H(c, 0); D = dd; break;
    case Op::fres: case Op::frsp: S = H(b, 0); D = dd; break;
    case Op::fmadds: case Op::fmsubs: case Op::fnmadds: case Op::fnmsubs: S = H(a, 0) | H(b, 0) | H(c, 0); D = dd; break;
    case Op::ps_rsqrte: case Op::ps_res: S = H(b, 0) | H(b, 1); D = dd; break;
    case Op::ps_add: case Op::ps_sub: case Op::ps_div: S = H(a, 0) | H(a, 1) | H(b, 0) | H(b, 1); D = dd; break;
    case Op::ps_mul: S = H(a, 0) | H(a, 1) | H(c, 0) | H(c, 1); D = dd; break;
    case Op::ps_muls0: S = H(a, 0) | H(a, 1) | H(c, 0); D = dd; break;
    case Op::ps_muls1: S = H(a, 0) | H(a, 1) | H(c, 1); D = dd; break;
    case Op::ps_madd: case Op::ps_msub: case Op::ps_nmadd: case Op::ps_nmsub:
      S = H(a, 0) | H(a, 1) | H(b, 0) | H(b, 1) | H(c, 0) | H(c, 1);
      D = dd;
      break;
    case Op::ps_madds0: S = H(a, 0) | H(a, 1) | H(b, 0) | H(b, 1) | H(c, 0); D = dd; break;
    case Op::ps_madds1: S = H(a, 0) | H(a, 1) | H(b, 0) | H(b, 1) | H(c, 1); D = dd; break;
    case Op::ps_sum0: S = H(a, 0) | H(b, 1) | H(c, 1); D = dd; break;
    case Op::ps_sum1: S = H(a, 0) | H(b, 1) | H(c, 0); D = dd; break;
    default: break;
  }
  *src = S;
  *dst = D;
}

void FuncGen::exec_call(const InsnInfo& x) {
  body_->get(kCpu);
  body_->i32c(x.word);
  body_->i32c(x.addr);
  ticks_expr();
  body_->call(iExec);
}

void FuncGen::emit_fp(const InsnInfo& x) {
  Fn& b = *body_;
  const Insn i{x.word};
  const Op o = x.op;
  const u32 rD = i.rD(), rA = i.rA(), rB = i.rB(), rC = i.rC();
  // Q4: the sources go to their locals first (on every path) and the code reads
  // them there; results go to memory and the locals; rt.exec's are reloaded.
  std::uint64_t src = 0, dst = 0;
  fp_halves(o, i, &src, &dst);
  for (u32 h = 0; h < 64; ++h)
    if (src >> h & 1) fp_load(kFpr + 8 * h);
  struct Cond {
    bool& f;
    ~Cond() { f = false; }
  } cond{fp_cond_};
  fp_cond_ = true;
  auto ld = [&](u32 off) { fp_get(off); };
  auto ldi = [&](u32 off) { fp_get_i64(off); };
  auto st = [&](u32 off, u32 local) { fp_put_f64(off, local); };  // an f64 local that holds no NaN
  // force25(c) as an f64 (fp::force25 on the bits).
  auto force25 = [&](u32 off) {
    ldi(off);
    b.tee(L_w);
    b.i64c(static_cast<s64>(0xFFFFFFFFF8000000ull));
    b.op(op::i64_and);
    b.get(L_w);
    b.i64c(0x8000000);
    b.op(op::i64_and);
    b.op(op::i64_add);
    b.op(op::f64_reinterpret_i64);
  };
  // i32: the FPR half at `off` has at most 27 significant bits (low 26 fraction bits zero).
  auto short_sig = [&](u32 off) {
    ldi(off);
    b.i64c(0x3FFFFFF);
    b.op(op::i64_and);
    b.op(op::i64_eqz);
  };
  // The single-precision result for r = RN53(x) (rules above), into `out`.
  // `midpoint` runs when r is a midpoint between two singles: it pushes the f64
  // to demote instead of r, or branches to `slow`.
  auto finish_single = [&](u32 r, u32 out, int slow, const std::function<void()>& midpoint) {
    b.get(r);
    b.op(op::f64_abs);
    b.f64c_bits(kF64MinSingleNormal);
    b.op(op::f64_le);
    b.if_();
    b.get(r);
    b.op(op::f64_abs);
    b.f64c_bits(kF64MinSingleNormal);
    b.op(op::f64_eq);
    b.br_if(slow);
    b.f64c_bits(0);
    b.get(r);
    b.op(op::f64_copysign);
    b.set(out);
    b.else_();
    b.get(r);
    b.get(r);
    b.op(op::f64_ne);  // NaN
    b.br_if(slow);
    b.get(r);
    b.op(op::i64_reinterpret_f64);
    b.i64c(0x1FFFFFFF);
    b.op(op::i64_and);
    b.i64c(0x10000000);
    b.op(op::i64_eq);  // a midpoint between two singles
    b.if_f64();
    midpoint();
    b.else_();
    b.get(r);
    b.end();
    b.op(op::f32_demote_f64);
    b.op(op::f64_promote_f32);
    b.set(out);
    b.end();
  };
  // Midpoint handler for r = fl(x + y) (x, y in f64 locals): r nudged one ulp
  // towards the exact sum when TwoSum's error is not zero.
  auto twosum_nudge = [&](u32 r, u32 x, u32 y) {
    return [&b, r, x, y, this] {
      b.get(r);
      b.get(x);
      b.op(op::f64_sub);
      b.set(L_fc);  // bb = r - x
      b.get(x);
      b.get(r);
      b.get(L_fc);
      b.op(op::f64_sub);
      b.op(op::f64_sub);
      b.get(y);
      b.get(L_fc);
      b.op(op::f64_sub);
      b.op(op::f64_add);
      b.set(L_fc);  // err = (x - (r - bb)) + (y - bb)
      b.get(r);
      b.op(op::i64_reinterpret_f64);
      b.i64c(1);
      b.i64c(-1);
      b.get(L_fc);
      b.f64c_bits(0);
      b.op(op::f64_gt);
      b.get(r);
      b.f64c_bits(0);
      b.op(op::f64_gt);
      b.op(op::i32_eq);
      b.op(op::select);  // +1 ulp of magnitude when err has r's sign, else -1
      b.i64c(0);
      b.get(L_fc);
      b.f64c_bits(0);
      b.op(op::f64_ne);
      b.op(op::select);
      b.op(op::i64_add);
      b.op(op::f64_reinterpret_i64);
    };
  };
  // The double-precision result for the correctly rounded r, into `out`.
  auto finish_double = [&](u32 r, u32 out, int slow) {
    b.get(r);
    b.get(r);
    b.op(op::f64_ne);
    b.br_if(slow);
    b.f64c_bits(0);
    b.get(r);
    b.op(op::f64_copysign);
    b.get(r);
    b.get(r);
    b.op(op::f64_abs);
    b.f64c_bits(kF64MinNormal);
    b.op(op::f64_lt);
    b.op(op::select);
    b.set(out);
  };
  enum class FA { Add, Sub, Mul, Div, Madd, Msub, Recip };
  // One single-precision arithmetic result into `out` (a NaN-free f64).
  auto single = [&](FA k, u32 oa, u32 ob, u32 oc, bool negate, u32 out, int slow) {
    std::function<void()> midpoint;
    switch (k) {
      case FA::Add: case FA::Sub: case FA::Madd: case FA::Msub:
        // r = x + y with y = +-b; a subtraction is the addition of the negation.
        if (k == FA::Madd || k == FA::Msub) {
          short_sig(oa);  // the product must be exact
          b.op(op::i32_eqz);
          b.br_if(slow);
          ld(oa);
          force25(oc);
          b.op(op::f64_mul);
          b.tee(L_fa);
          // A zero product of nonzero operands underflowed: the exact sum's sign
          // (a zero result's sign) may differ from p + b's. Runtime.
          b.f64c_bits(0);
          b.op(op::f64_eq);
          b.if_();
          ldi(oa);
          b.i64c(1);
          b.op(op::i64_shl);
          b.op(op::i64_eqz);
          force25(oc);
          b.f64c_bits(0);
          b.op(op::f64_eq);
          b.op(op::i32_or);
          b.op(op::i32_eqz);
          b.br_if(slow);
          b.end();
        } else {
          ld(oa);
          b.set(L_fa);
        }
        ld(ob);
        if (k == FA::Sub || k == FA::Msub) b.op(op::f64_neg);
        b.tee(L_fb);
        b.get(L_fa);
        b.op(op::f64_add);
        midpoint = twosum_nudge(L_r, L_fa, L_fb);
        if (k == FA::Madd || k == FA::Msub) {
          // TwoSum sees only the sum's error: on a midpoint the product must be
          // exact too, so one that underflowed double goes to the runtime.
          midpoint = [&, slow, nudge = midpoint] {
            b.get(L_fa);
            b.op(op::f64_abs);
            b.f64c_bits(kF64MinNormal);
            b.op(op::f64_lt);
            b.br_if(slow);
            nudge();
          };
        }
        break;
      case FA::Mul:
        ld(oa);
        force25(oc);
        b.op(op::f64_mul);
        midpoint = [&, oa, slow] {  // exact product: demote r as it is
          short_sig(oa);
          b.op(op::i32_eqz);
          b.br_if(slow);
          b.get(L_r);
        };
        break;
      case FA::Div: case FA::Recip:
        if (k == FA::Recip) b.f64c_bits(kF64One);
        else ld(oa);
        ld(ob);
        b.op(op::f64_div);
        midpoint = [&, slow] { b.br(slow); };
        break;
    }
    b.set(L_r);
    finish_single(L_r, out, slow, midpoint);
    if (negate) {
      b.get(out);
      b.op(op::f64_neg);
      b.set(out);
    }
  };
  // Double fused multiply-add into `out`: exact product (sig(a) + sig(c) <= 53,
  // no underflow from nonzero operands), then one rounding.
  auto dmadd = [&](bool sub, bool negate, u32 out, int slow) {
    ldi(ps0(rA));
    b.set(L_w);
    ldi(ps0(rC));
    b.set(L_w2);
    b.get(L_w);
    b.i64c(1ll << 52);
    b.op(op::i64_or);
    b.op(op::i64_ctz);
    b.get(L_w2);
    b.i64c(1ll << 52);
    b.op(op::i64_or);
    b.op(op::i64_ctz);
    b.op(op::i64_add);
    b.i64c(53);
    b.op(op::i64_lt_u);
    b.br_if(slow);
    b.get(L_w);
    b.op(op::f64_reinterpret_i64);
    b.get(L_w2);
    b.op(op::f64_reinterpret_i64);
    b.op(op::f64_mul);
    b.tee(L_fa);
    b.op(op::f64_abs);
    b.f64c_bits(kF64MinNormal);
    b.op(op::f64_lt);
    b.get(L_w);
    b.i64c(1);
    b.op(op::i64_shl);
    b.op(op::i64_eqz);
    b.get(L_w2);
    b.i64c(1);
    b.op(op::i64_shl);
    b.op(op::i64_eqz);
    b.op(op::i32_or);
    b.op(op::i32_eqz);
    b.op(op::i32_and);
    b.br_if(slow);  // an underflowing product of nonzero operands
    b.get(L_fa);
    ld(ps0(rB));
    b.op(sub ? op::f64_sub : op::f64_add);
    b.set(L_r);
    finish_double(L_r, out, slow);
    if (negate) {
      b.get(out);
      b.op(op::f64_neg);
      b.set(out);
    }
  };

  // ---- no slow path: compares, selects, fctiwz ----
  switch (o) {
    case Op::fcmpu: case Op::fcmpo: case Op::ps_cmpu0: case Op::ps_cmpo0: case Op::ps_cmpu1: case Op::ps_cmpo1: {
      // sem::fcmp_bits: LT GT EQ, or UN when either is a NaN. IEEE comparisons
      // order signed zeros, subnormals and infinities exactly as the model does.
      const bool h1 = o == Op::ps_cmpu1 || o == Op::ps_cmpo1;
      ld(h1 ? ps1(rA) : ps0(rA));
      b.set(L_fa);
      ld(h1 ? ps1(rB) : ps0(rB));
      b.set(L_fb);
      set_cr_field(i.crfD(), [&] {
        auto rel = [&](u8 opc, u32 sh) {
          b.get(L_fa);
          b.get(L_fb);
          b.op(opc);
          if (sh) {
            b.i32c(sh);
            b.op(op::i32_shl);
          }
        };
        rel(op::f64_lt, 3);
        rel(op::f64_gt, 2);
        b.op(op::i32_or);
        rel(op::f64_eq, 1);
        b.op(op::i32_or);
        b.get(L_fa);
        b.get(L_fa);
        b.op(op::f64_ne);
        b.get(L_fb);
        b.get(L_fb);
        b.op(op::f64_ne);
        b.op(op::i32_or);
        b.op(op::i32_or);
      });
      return;
    }
    case Op::fsel: case Op::ps_sel: {
      // fp::select: c if a >= 0 (-0 included), else b; a NaN selects b. Bits only.
      auto sel = [&](u32 h, u32 local) {
        ldi(h ? ps1(rC) : ps0(rC));
        ldi(h ? ps1(rB) : ps0(rB));
        ld(h ? ps1(rA) : ps0(rA));
        b.f64c_bits(0);
        b.op(op::f64_ge);
        b.op(op::select);
        b.set(local);
      };
      sel(0, L_w);
      if (o == Op::ps_sel) sel(1, L_w2);
      fp_put_i64(ps0(rD), L_w);
      if (o == Op::ps_sel) fp_put_i64(ps1(rD), L_w2);
      return;
    }
    case Op::fctiwz: {
      // fp::to_int32 toward zero: NaN -> 0x80000000, saturating, and bit 32 set
      // for a zero result from a negative source.
      ld(ps0(rB));
      b.tee(L_fa);
      b.fc(op::fc_i32_trunc_sat_f64_s);
      b.set(L_tmp);
      b.i32c(0x80000000u);
      b.get(L_tmp);
      b.get(L_fa);
      b.get(L_fa);
      b.op(op::f64_ne);
      b.op(op::select);
      b.tee(L_tmp);
      b.op(op::i64_extend_i32_u);
      b.i64c(static_cast<s64>(0xFFF8000000000000ull));
      b.op(op::i64_or);
      b.get(L_tmp);
      b.op(op::i32_eqz);
      b.get(L_fa);
      b.op(op::i64_reinterpret_f64);
      b.i64c(63);
      b.op(op::i64_shr_u);
      b.op(op::i32_wrap_i64);
      b.op(op::i32_and);
      b.op(op::i64_extend_i32_u);
      b.i64c(32);
      b.op(op::i64_shl);
      b.op(op::i64_or);
      b.set(L_w);
      fp_put_i64(ps0(rD), L_w);
      return;
    }
    default:
      break;
  }

  // ---- fast path, else the runtime ----
  const int done = b.block();
  const int slow = b.block();
  // FPSCR: round to nearest, NI set.
  fpscr_ok();
  b.op(op::i32_eqz);
  b.br_if(slow);
  if (!ctx_.opt.fp_exact && o != Op::fctiw) {  // Q4, F1: the fast model, no other way out
    emit_fp_fast(x, dst);
    b.br(done);
    b.end();
    exec_call(x);
    fp_reload(dst);
    b.end();
    return;
  }
  auto both = [&](u32 local) {  // a single result fills both halves
    st(ps0(rD), local);
    st(ps1(rD), local);
  };
  switch (o) {
    case Op::fadd: case Op::fsub: case Op::fmul: case Op::fdiv:
      ld(ps0(rA));
      ld(o == Op::fmul ? ps0(rC) : ps0(rB));
      b.op(o == Op::fadd ? op::f64_add : o == Op::fsub ? op::f64_sub : o == Op::fmul ? op::f64_mul : op::f64_div);
      b.set(L_r);
      finish_double(L_r, L_f0, slow);
      st(ps0(rD), L_f0);
      break;
    case Op::fmadd: case Op::fmsub: case Op::fnmadd: case Op::fnmsub:
      dmadd(o == Op::fmsub || o == Op::fnmsub, o == Op::fnmadd || o == Op::fnmsub, L_f0, slow);
      st(ps0(rD), L_f0);
      break;
    case Op::fadds: single(FA::Add, ps0(rA), ps0(rB), 0, false, L_f0, slow); both(L_f0); break;
    case Op::fsubs: single(FA::Sub, ps0(rA), ps0(rB), 0, false, L_f0, slow); both(L_f0); break;
    case Op::fmuls: single(FA::Mul, ps0(rA), 0, ps0(rC), false, L_f0, slow); both(L_f0); break;
    case Op::fdivs: single(FA::Div, ps0(rA), ps0(rB), 0, false, L_f0, slow); both(L_f0); break;
    case Op::fres: single(FA::Recip, 0, ps0(rB), 0, false, L_f0, slow); both(L_f0); break;
    case Op::fmadds: single(FA::Madd, ps0(rA), ps0(rB), ps0(rC), false, L_f0, slow); both(L_f0); break;
    case Op::fmsubs: single(FA::Msub, ps0(rA), ps0(rB), ps0(rC), false, L_f0, slow); both(L_f0); break;
    case Op::fnmadds: single(FA::Madd, ps0(rA), ps0(rB), ps0(rC), true, L_f0, slow); both(L_f0); break;
    case Op::fnmsubs: single(FA::Msub, ps0(rA), ps0(rB), ps0(rC), true, L_f0, slow); both(L_f0); break;
    case Op::frsp:
      // fp::round_to_single: one rounding of the exact operand; NaN -> runtime.
      ld(ps0(rB));
      b.tee(L_r);
      b.get(L_r);
      b.op(op::f64_ne);
      b.br_if(slow);
      b.f64c_bits(0);
      b.get(L_r);
      b.op(op::f64_copysign);
      b.get(L_r);
      b.op(op::f32_demote_f64);
      b.op(op::f64_promote_f32);
      b.get(L_r);
      b.op(op::f64_abs);
      b.f64c_bits(kF64MinSingleNormal);
      b.op(op::f64_lt);
      b.op(op::select);
      b.set(L_f0);
      both(L_f0);
      break;
    case Op::fctiw:
      // fp::to_int32 in round to nearest (the guard): ties to even, then as fctiwz.
      ld(ps0(rB));
      b.tee(L_fa);
      b.op(op::f64_nearest);
      b.fc(op::fc_i32_trunc_sat_f64_s);
      b.set(L_tmp);
      b.i32c(0x80000000u);
      b.get(L_tmp);
      b.get(L_fa);
      b.get(L_fa);
      b.op(op::f64_ne);
      b.op(op::select);
      b.tee(L_tmp);
      b.op(op::i64_extend_i32_u);
      b.i64c(static_cast<s64>(0xFFF8000000000000ull));
      b.op(op::i64_or);
      b.get(L_tmp);
      b.op(op::i32_eqz);
      b.get(L_fa);
      b.op(op::i64_reinterpret_f64);
      b.i64c(63);
      b.op(op::i64_shr_u);
      b.op(op::i32_wrap_i64);
      b.op(op::i32_and);
      b.op(op::i64_extend_i32_u);
      b.i64c(32);
      b.op(op::i64_shl);
      b.op(op::i64_or);
      b.set(L_w);
      fp_put_i64(ps0(rD), L_w);
      break;
    case Op::frsqrte: case Op::ps_rsqrte: {
      // fp::rsqrt_estimate: 1 / sqrt_rn(b) in double; NaN operand -> runtime; a
      // negative operand -> the default NaN; +-0 -> +-inf; +inf -> +0. ps_rsqrte
      // rounds each half to single (the default NaN stays itself).
      auto half = [&](u32 off, bool to_single, u32 local) {
        ld(off);
        b.tee(L_fa);
        b.get(L_fa);
        b.op(op::f64_ne);
        b.br_if(slow);
        b.f64c_bits(kF64One);
        b.get(L_fa);
        b.op(op::f64_sqrt);
        b.op(op::f64_div);
        b.set(L_r);
        b.i64c(static_cast<s64>(kDefaultNaNBits));
        if (to_single) {  // fp::round_to_single with NI: below 2^-126 a signed zero
          b.f64c_bits(0);
          b.get(L_r);
          b.op(op::f64_copysign);
          b.get(L_r);
          b.op(op::f32_demote_f64);
          b.op(op::f64_promote_f32);
          b.get(L_r);
          b.op(op::f64_abs);
          b.f64c_bits(kF64MinSingleNormal);
          b.op(op::f64_lt);
          b.op(op::select);
        } else {
          b.get(L_r);
        }
        b.op(op::i64_reinterpret_f64);
        b.get(L_r);
        b.get(L_r);
        b.op(op::f64_ne);
        b.op(op::select);
        b.set(local);
      };
      if (o == Op::frsqrte) {
        half(ps0(rB), false, L_w);
        fp_put_i64(ps0(rD), L_w);
      } else {
        half(ps0(rB), true, L_w);
        half(ps1(rB), true, L_w2);
        fp_put_i64(ps0(rD), L_w);
        fp_put_i64(ps1(rD), L_w2);
      }
      break;
    }
    default: {
      // Paired singles: both halves are computed before either is stored (rD may
      // be a source).
      FA k = FA::Add;
      bool neg = false;
      u32 a0 = ps0(rA), a1 = ps1(rA), b0 = ps0(rB), b1 = ps1(rB), c0 = ps0(rC), c1 = ps1(rC);
      switch (o) {
        case Op::ps_add: k = FA::Add; break;
        case Op::ps_sub: k = FA::Sub; break;
        case Op::ps_mul: k = FA::Mul; break;
        case Op::ps_div: k = FA::Div; break;
        case Op::ps_res: k = FA::Recip; break;
        case Op::ps_muls0: k = FA::Mul; c1 = c0; break;
        case Op::ps_muls1: k = FA::Mul; c0 = c1; break;
        case Op::ps_madd: k = FA::Madd; break;
        case Op::ps_msub: k = FA::Msub; break;
        case Op::ps_nmadd: k = FA::Madd; neg = true; break;
        case Op::ps_nmsub: k = FA::Msub; neg = true; break;
        case Op::ps_madds0: k = FA::Madd; c1 = c0; break;
        case Op::ps_madds1: k = FA::Madd; c0 = c1; break;
        case Op::ps_sum0: case Op::ps_sum1: {
          // s = A.ps0 + B.ps1 (single); ps_sum0: (s, C.ps1), ps_sum1: (C.ps0, s).
          single(FA::Add, ps0(rA), ps1(rB), 0, false, L_f0, slow);
          const bool s0 = o == Op::ps_sum0;
          ldi(s0 ? ps1(rC) : ps0(rC));
          b.set(L_w);
          st(s0 ? ps0(rD) : ps1(rD), L_f0);
          fp_put_i64(s0 ? ps1(rD) : ps0(rD), L_w);
          b.br(done);
          b.end();
          exec_call(x);
          fp_reload(dst);
          b.end();
          return;
        }
        default:
          break;  // is_fp_inline() and these cases list the same opcodes
      }
      single(k, a0, b0, c0, neg, L_f0, slow);
      single(k, a1, b1, c1, neg, L_f1, slow);
      st(ps0(rD), L_f0);
      st(ps1(rD), L_f1);
      break;
    }
  }
  b.br(done);
  b.end();
  exec_call(x);
  fp_reload(dst);
  b.end();
}

// Q4, F1: the fast model of ppc/fp.h (its comment has the rules), for every
// instruction emit_fp computes but fctiw (exact in both models). Every step is one
// IEEE double operation in the order fp.cpp writes it, a single result is the
// double rounded to f32 and back (Q6: subnormal results stay), and a NaN result
// becomes the default NaN,
// whatever bits the host's arithmetic gave it. Called inside emit_fp's fast-path
// block, after the FPSCR check; the sources are in their locals.
void FuncGen::emit_fp_fast(const InsnInfo& x, std::uint64_t) {
  Fn& b = *body_;
  const Insn i{x.word};
  const Op o = x.op;
  const u32 rD = i.rD(), rA = i.rA(), rB = i.rB(), rC = i.rC();
  constexpr u64 kOne = 0x3FF0000000000000ull;
  auto ld = [&](u32 off) { fp_get(off); };
  // The result for the double in L_r, as i64 bits into `out` (fp::fast_finish).
  auto fin = [&](bool single, u32 out) {
    b.i64c(static_cast<s64>(kDefaultNaNBits));
    b.get(L_r);
    if (single) {
      b.op(op::f32_demote_f64);
      b.op(op::f64_promote_f32);
    }
    b.op(op::i64_reinterpret_f64);  // Q6: no flush below the normal range
    b.get(L_r);
    b.get(L_r);
    b.op(op::f64_ne);
    b.op(op::select);  // NaN: the default NaN
    b.set(out);
  };
  auto binary = [&](u8 opc, u32 x_off, u32 y_off) {
    ld(x_off);
    ld(y_off);
    b.op(opc);
    b.set(L_r);
  };
  auto madd = [&](u32 a_off, u32 c_off, u32 b_off, bool neg_b, bool neg_r) {
    ld(a_off);
    ld(c_off);
    b.op(op::f64_mul);
    ld(b_off);
    b.op(neg_b ? op::f64_sub : op::f64_add);
    if (neg_r) b.op(op::f64_neg);
    b.set(L_r);
  };
  auto recip = [&](u32 b_off) {
    b.f64c_bits(kOne);
    ld(b_off);
    b.op(op::f64_div);
    b.set(L_r);
  };
  auto rsqrt = [&](u32 b_off) {
    ld(b_off);
    b.op(op::f64_sqrt);
    b.set(L_fb);
    b.f64c_bits(kOne);
    b.get(L_fb);
    b.op(op::f64_div);
    b.set(L_r);
  };
  const bool nm = o == Op::fnmadd || o == Op::fnmsub || o == Op::fnmadds || o == Op::fnmsubs ||
                  o == Op::ps_nmadd || o == Op::ps_nmsub;
  const bool ms = o == Op::fmsub || o == Op::fnmsub || o == Op::fmsubs || o == Op::fnmsubs || o == Op::ps_msub ||
                  o == Op::ps_nmsub;
  switch (o) {
    case Op::fadd: case Op::fsub: case Op::fmul: case Op::fdiv:
      binary(o == Op::fadd ? op::f64_add : o == Op::fsub ? op::f64_sub : o == Op::fmul ? op::f64_mul : op::f64_div,
             ps0(rA), o == Op::fmul ? ps0(rC) : ps0(rB));
      fin(false, L_w);
      fp_put_i64(ps0(rD), L_w);
      return;
    case Op::fmadd: case Op::fmsub: case Op::fnmadd: case Op::fnmsub:
      madd(ps0(rA), ps0(rC), ps0(rB), ms, nm);
      fin(false, L_w);
      fp_put_i64(ps0(rD), L_w);
      return;
    case Op::frsqrte:
      rsqrt(ps0(rB));
      fin(false, L_w);
      fp_put_i64(ps0(rD), L_w);
      return;
    case Op::fadds: case Op::fsubs: case Op::fmuls: case Op::fdivs: case Op::fres: case Op::frsp:
    case Op::fmadds: case Op::fmsubs: case Op::fnmadds: case Op::fnmsubs:
      switch (o) {
        case Op::fadds: binary(op::f64_add, ps0(rA), ps0(rB)); break;
        case Op::fsubs: binary(op::f64_sub, ps0(rA), ps0(rB)); break;
        case Op::fmuls: binary(op::f64_mul, ps0(rA), ps0(rC)); break;
        case Op::fdivs: binary(op::f64_div, ps0(rA), ps0(rB)); break;
        case Op::fres: recip(ps0(rB)); break;
        case Op::frsp:
          ld(ps0(rB));
          b.set(L_r);
          break;
        default: madd(ps0(rA), ps0(rC), ps0(rB), ms, nm); break;
      }
      fin(true, L_w);
      fp_put_i64(ps0(rD), L_w);  // a single result fills both halves
      fp_put_i64(ps1(rD), L_w);
      return;
    case Op::ps_rsqrte:  // round_to_single(rsqrt_estimate(b)) per half
      for (u32 h : {0u, 1u}) {
        const u32 out = h ? L_w2 : L_w;
        rsqrt(h ? ps1(rB) : ps0(rB));
        fin(false, out);
        b.get(out);
        b.op(op::f64_reinterpret_i64);
        b.set(L_r);
        fin(true, out);
      }
      break;
    case Op::ps_sum0: case Op::ps_sum1: {  // s = A.ps0 + B.ps1; ps_sum0: (s, C.ps1), ps_sum1: (C.ps0, s)
      binary(op::f64_add, ps0(rA), ps1(rB));
      const bool s0 = o == Op::ps_sum0;
      fin(true, s0 ? L_w : L_w2);
      fp_get_i64(s0 ? ps1(rC) : ps0(rC));
      b.set(s0 ? L_w2 : L_w);
      break;
    }
    default: {
      // Paired singles, both halves computed before either is stored (rD may be a source).
      for (u32 h : {0u, 1u}) {
        const u32 a = h ? ps1(rA) : ps0(rA), bb = h ? ps1(rB) : ps0(rB), c = h ? ps1(rC) : ps0(rC);
        const u32 c0 = ps0(rC), c1 = ps1(rC);
        switch (o) {
          case Op::ps_add: binary(op::f64_add, a, bb); break;
          case Op::ps_sub: binary(op::f64_sub, a, bb); break;
          case Op::ps_div: binary(op::f64_div, a, bb); break;
          case Op::ps_mul: binary(op::f64_mul, a, c); break;
          case Op::ps_muls0: binary(op::f64_mul, a, c0); break;
          case Op::ps_muls1: binary(op::f64_mul, a, c1); break;
          case Op::ps_res: recip(bb); break;
          case Op::ps_madd: case Op::ps_msub: case Op::ps_nmadd: case Op::ps_nmsub: madd(a, c, bb, ms, nm); break;
          case Op::ps_madds0: madd(a, c0, bb, false, false); break;
          case Op::ps_madds1: madd(a, c1, bb, false, false); break;
          default: break;  // is_fp_inline() lists the same opcodes
        }
        fin(true, h ? L_w2 : L_w);
      }
      break;
    }
  }
  fp_put_i64(ps0(rD), L_w);
  fp_put_i64(ps1(rD), L_w2);
}

// lmw / stmw: the in-module helper moves the words between guest memory and the
// Cpu struct's GPRs (the node has the helper's dataflow: dirty sources flushed
// before, written registers reloaded after when live).
void FuncGen::emit_memh(const InsnInfo& x, RegSet flush_set) {
  Fn& b = *body_;
  const Insn i{x.word};
  flush(flush_set);
  b.get(kCpu);
  // (rA|0) + d from the Cpu struct: the helper's dataflow keeps no register live
  // in its local, and the flush above made memory current.
  if (i.rA()) {
    b.get(kCpu);
    b.mem(op::i32_load, 2, kGpr + 4 * i.rA());
    if (i.simm()) {
      b.i32c(i.simm());
      b.op(op::i32_add);
    }
  } else {
    b.i32c(i.simm());
  }
  b.i32c(i.rD());
  b.i32c(x.addr);
  ticks_expr();
  b.call(x.op == Op::lmw ? hLmw : hStmw);
}

void FuncGen::emit_resume_block() {
  // rt.resume: resume_target(), then the dispatcher index of the label; 0 = keep unwinding.
  body_->get(kCpu);
  body_->get(L_sp);
  body_->global(gTableBase);
  body_->i32c(self_slot_);
  body_->op(op::i32_add);
  body_->call(iResume);
  body_->tee(L_label);
  body_->op(op::i32_eqz);
  body_->if_();
  body_->op(op::return_);
  body_->end();
  RegSet ru = 0;
  for (u32 a : resume_labels_) ru |= nodes_[(a - f_.start) / 4].Lin;
  reload(ru);
  load_ticks();
  body_->br(loop_level_);
}

void FuncGen::emit_fall() {
  const Node& F = nodes_[f_.insns.size()];
  materialize();
  flush(F.Din);
  store_ticks();
  body_->get(kCpu);
  if (f_.falls_into) {
    body_->call(ctx_.callable.at(f_.falls_into));
    body_->op(op::return_);
  } else {
    body_->i32c(f_.end);
    body_->i32c(static_cast<u32>(WasmTrap::RanOffEnd));
    body_->call(iTrap);
    body_->op(op::unreachable);
  }
}

void FuncGen::emit_node(std::size_t k) {
  Fn& b = *body_;
  const InsnInfo& x = f_.insns[k];
  const Node& N = nodes_[k];
  const u32 a = x.addr, ret = a + 4;
  auto safepoint = [&] {
    flush(N.Din);
    store_pc(a);
    store_ticks();
    load_sp();
    b.get(kCpu);
    b.call(iSafepoint);
    check(a);
    reload(N.Lm);
    load_ticks();
  };
  if (N.pre != pNone || N.fpchk) fp_reset();  // Q4: a safe point or the FP check may switch FPR contents
  if (N.pre == pIdle) {
    materialize();
    safepoint();
  } else if (N.pre == pBack) {
    materialize();
    b.get(L_t);
    b.get(kCpu);
    b.mem(op::i64_load, 3, kNextEvent);
    b.op(op::i64_ge_u);
    b.if_();
    safepoint();
    b.end();
  }
  if (N.fpchk) {
    // Floating-point unavailable (E3a): with MSR[FP] clear this FPU instruction
    // does not start; the OS handler switches the FPU context and returns here.
    materialize();
    b.get(kCpu);
    b.mem(op::i32_load, 2, kMsr);
    b.i32c(ppc::msr::FP);
    b.op(op::i32_and);
    b.op(op::i32_eqz);
    b.if_();
    flush(N.Din);
    store_ticks();
    load_sp();
    b.get(kCpu);
    b.i32c(a);
    b.call(iFpUnavailable);
    check(a);
    reload(N.Lm);
    load_ticks();
    b.end();
  }
  ++p_;  // ++c.ticks: this instruction has started
  ++ctx_.out->stats.instructions;
  const RegSet D = dm(N);
  switch (N.kind) {
    case kNop:
      ++ctx_.out->stats.inline_insns;
      return;
    case kInline:
      ++ctx_.out->stats.inline_insns;
      if (ctx_.opt.precise_io && N.mem) flush(D);
      if (fuse_[k]) {  // Q4: the next node's branch tests it
        fused_ = static_cast<int>(k);
        return;
      }
      emit_inline(x);
      return;
    case kFp:
      ++ctx_.out->stats.inline_insns;
      emit_fp(x);
      return;
    case kMemH:
      ++ctx_.out->stats.inline_insns;
      emit_memh(x, ctx_.opt.precise_io ? D : (N.R & D));
      reload(N.W & N.Lout);
      return;
    case kHelper:
      fp_reset();  // Q4: rt.exec (mffs, mtfsf, ...)
      ++ctx_.out->stats.helper_insns;
      ++ctx_.out->stats.helper_by_op[static_cast<std::size_t>(x.op)];
      flush(N.R & D);
      b.get(kCpu);
      b.i32c(x.word);
      b.i32c(a);
      ticks_expr();
      b.call(iExec);
      reload(N.W & N.Lout);
      return;
    case kTw:
      ++ctx_.out->stats.helper_insns;
      flush(N.R & D);
      b.get(kCpu);
      b.i32c(x.word);
      b.i32c(a);
      b.call(iTw);
      return;
    case kTrapOp: case kTrap:
      ++ctx_.out->stats.branch_insns;
      materialize();
      flush(D);
      store_ticks();
      b.get(kCpu);
      b.i32c(a);
      b.i32c(static_cast<u32>(N.kind == kTrapOp     ? WasmTrap::ExternalControl
                              : x.op == Op::invalid ? WasmTrap::InvalidInstruction
                                                    : WasmTrap::UnconditionalTrap));
      b.call(iTrap);
      b.op(op::unreachable);
      return;
    case kSc:
      fp_reset();
      ++ctx_.out->stats.helper_insns;
      materialize();
      flush(D);
      store_pc(ret);
      store_ticks();
      b.get(kCpu);
      b.call(iSystemCall);
      check(ret);
      reload(N.Lout);
      load_ticks();
      return;
    case kRfi:
      ++ctx_.out->stats.branch_insns;
      materialize();
      flush(D);
      store_ticks();
      b.get(kCpu);
      b.call(iRfi);
      leave();
      return;
    default:
      break;
  }
  ++ctx_.out->stats.branch_insns;
  materialize();
  switch (N.kind) {
    case kJump:
      jump(seg_at_[static_cast<std::size_t>(N.s0)]);
      return;
    case kCJump: {
      int t = seg_at_[static_cast<std::size_t>(N.s0)];
      cond_expr(x);
      if (t > cur_seg_) {
        b.br_if(level_[static_cast<std::size_t>(t)]);
      } else if (t == cur_seg_ && self_loop_ >= 0) {
        b.br_if(self_loop_);
      } else if (structured_) {
        b.br_if(loop_at_[static_cast<std::size_t>(t)]);  // V4
      } else {
        b.if_();
        jump(t);
        b.end();
      }
      return;
    }
    case kJTab: {
      const auto& targets = jt_succ_[static_cast<std::size_t>(N.jt)];
      u32 lo = 0xFFFFFFFFu, hi = 0;
      std::vector<int> distinct;
      for (int s : targets) {
        u32 t = f_.start + 4 * static_cast<u32>(s);
        lo = std::min(lo, t);
        hi = std::max(hi, t);
        if (std::find(distinct.begin(), distinct.end(), s) == distinct.end()) distinct.push_back(s);
      }
      int dflt = b.block();
      std::vector<int> lv(distinct.size());
      for (std::size_t d = distinct.size(); d-- > 0;) lv[d] = b.block();
      std::vector<int> table((hi - lo) / 4 + 1, dflt);
      for (std::size_t d = 0; d < distinct.size(); ++d)
        table[(f_.start + 4 * static_cast<u32>(distinct[d]) - lo) / 4] = lv[d];
      reg_get(reg::CTR);
      b.i32c(~3u);
      b.op(op::i32_and);
      b.i32c(lo);
      b.op(op::i32_sub);
      b.i32c(2);
      b.op(op::i32_shr_u);
      b.br_table(table, dflt);
      for (std::size_t d = 0; d < distinct.size(); ++d) {
        b.end();
        jump(seg_at_[static_cast<std::size_t>(distinct[d])]);
      }
      b.end();
      b.get(kCpu);
      b.i32c(a);
      b.i32c(static_cast<u32>(WasmTrap::JumpOutsideTable));
      b.call(iTrap);
      b.op(op::unreachable);
      return;
    }
    case kCall: {
      fp_reset();  // Q4: the callee may change any FPR (its return is a label anyway)
      const bool indirect = x.flow == Flow::IndirectCall;
      if (N.cond) {
        cond_expr(x);
        b.set(L_tc);
      }
      if (indirect) {
        reg_get(x.op == Op::bclr ? reg::LR : reg::CTR);
        b.i32c(~3u);
        b.op(op::i32_and);
        b.set(L_tgt);
      }
      if (N.W & ppc::bit(reg::LR)) {
        b.i32c(ret);
        reg_set(reg::LR);
      }
      if (N.cond) {
        b.get(L_tc);
        b.if_();
      }
      flush(D | N.W);
      store_ticks();
      load_sp();
      b.get(kCpu);
      if (x.flow == Flow::Call) {
        b.call(ctx_.callable.at(x.target));
      } else if (indirect) {
        b.get(L_tgt);
        b.call(iCallIndirect);
      } else {
        b.i32c(x.target);
        b.call(iAbsoluteCall);
      }
      check(ret);
      reload(N.Lout);
      load_ticks();
      if (N.cond) b.end();
      return;
    }
    case kTail: case kRet: case kIJump: {
      if (N.cond) {
        cond_expr(x);
        b.if_();
      }
      flush(D | N.W);
      if (N.kind == kTail) {
        store_ticks();
        b.get(kCpu);
        b.return_call(ctx_.callable.at(x.target));
      } else if (N.kind == kRet) {
        b.get(kCpu);
        reg_get(reg::LR);
        b.i32c(~3u);
        b.op(op::i32_and);
        b.mem(op::i32_store, 2, kPc);
        store_ticks();
        b.op(op::return_);
      } else {
        store_ticks();
        b.get(kCpu);
        reg_get(reg::CTR);
        b.i32c(~3u);
        b.op(op::i32_and);
        b.call(iCallIndirect);
        b.op(op::return_);
      }
      if (N.cond) b.end();
      return;
    }
    default:
      return;
  }
}

// V4: loops for a function without the dispatcher. Every branch from segment u
// back to segment h <= u (u > h: h == u is a self loop, emit_segment's) makes h a
// loop header whose loop runs to the last such u. The loops must nest (two may
// share segments only if one contains the other), and no branch may enter a loop
// but at its header (forward branches into a loop's later segments come from
// inside it). Then every branch is a br: forward to a block that ends right
// before its target, backward to the loop that starts at it.
bool FuncGen::plan_loops() {
  const std::size_t n = f_.insns.size();
  const int nseg = static_cast<int>(seg_start_.size());
  loop_end_.assign(static_cast<std::size_t>(nseg), -1);
  loop_at_.assign(static_cast<std::size_t>(nseg), -1);
  std::vector<std::pair<int, int>> edges;
  for (std::size_t k = 0; k < n; ++k) {
    if (!f_.reachable[k]) continue;
    const Node& N = nodes_[k];
    const int u = seg_at_[k];
    if (N.kind == kJump || N.kind == kCJump) edges.push_back({u, seg_at_[static_cast<std::size_t>(N.s0)]});
    if (N.jt >= 0)
      for (int s : jt_succ_[static_cast<std::size_t>(N.jt)]) edges.push_back({u, seg_at_[static_cast<std::size_t>(s)]});
  }
  for (const auto& [u, v] : edges)
    if (v < u) loop_end_[static_cast<std::size_t>(v)] = std::max(loop_end_[static_cast<std::size_t>(v)], u);
  // Nesting: scan the segments with a stack of the loops open there.
  std::vector<int> open;
  for (int s = 0; s < nseg; ++s) {
    while (!open.empty() && loop_end_[static_cast<std::size_t>(open.back())] < s) open.pop_back();
    const int e = loop_end_[static_cast<std::size_t>(s)];
    if (e < 0) continue;
    if (!open.empty() && e > loop_end_[static_cast<std::size_t>(open.back())]) return false;  // overlapping loops
    open.push_back(s);
  }
  // No entry into a loop but at its header.
  for (const auto& [u, v] : edges) {
    if (v <= u) continue;
    for (int h = 0; h < v; ++h) {
      const int e = loop_end_[static_cast<std::size_t>(h)];
      if (e >= v && (u < h || u > e)) return false;
    }
  }
  return true;
}

// V4: segments a..b, a region at one nesting level: the whole function, or a
// loop (its wasm loop opened by the caller, a its header). Blocks for the
// segments of this level (not those inside a nested loop) open here, the last
// outermost; a nested loop is a region of its own.
void FuncGen::emit_region(int a, int b, bool is_loop, const std::function<void(int)>& emit_segment) {
  Fn& body = *body_;
  std::vector<int> mine;  // the segments after a at this level
  const int a_end = loop_end_[static_cast<std::size_t>(a)];  // a's own loop, when a starts a nested one here
  for (int s = !is_loop && a_end >= 0 ? a_end + 1 : a + 1; s <= b;) {
    mine.push_back(s);
    const int e = loop_end_[static_cast<std::size_t>(s)];
    s = e >= 0 ? e + 1 : s + 1;
  }
  for (auto it = mine.rbegin(); it != mine.rend(); ++it) level_[static_cast<std::size_t>(*it)] = body.block();
  for (int s = a; s <= b;) {
    if (s != a) body.end();  // the block that ends right before s
    const int e = loop_end_[static_cast<std::size_t>(s)];
    if (e >= 0 && !(is_loop && s == a)) {
      loop_at_[static_cast<std::size_t>(s)] = body.loop();
      emit_region(s, e, true, emit_segment);
      body.end();
      s = e + 1;
    } else {
      emit_segment(s);
      ++s;
    }
  }
}

bool FuncGen::run(Fn& body, std::string* error) {
  body_ = &body;
  if (!classify(error)) return false;
  const std::size_t n = f_.insns.size();
  // Resume labels (runtime/native.h): every call's return address and every
  // back edge, except entries; none in a replaced function's original.
  std::vector<u32> entries = mode_ == Mode::Body ? f_.entries : std::vector<u32>{f_.start};
  if (mode_ == Mode::Body || mode_ == Mode::Plain) {  // V4: not in Fast
    auto add = [&](u32 a) {
      if (f_.contains(a) && f_.reachable[(a - f_.start) / 4] &&
          std::find(f_.entries.begin(), f_.entries.end(), a) == f_.entries.end())
        resume_labels_.push_back(a);
    };
    for (u32 a : f_.resume_points) add(a);
    for (u32 a : f_.back_edges) add(a);
    std::sort(resume_labels_.begin(), resume_labels_.end());
    resume_labels_.erase(std::unique(resume_labels_.begin(), resume_labels_.end()), resume_labels_.end());
  }
  has_resume_ = !resume_labels_.empty();

  // Code segments: one per label.
  std::vector<u32> labels(entries.begin(), entries.end());
  for (std::size_t k = 0; k < n; ++k) {
    if (!f_.reachable[k]) continue;
    const Node& N = nodes_[k];
    if (N.kind == kJump || N.kind == kCJump) labels.push_back(f_.start + 4 * static_cast<u32>(N.s0));
    if (N.jt >= 0)
      for (int s : jt_succ_[static_cast<std::size_t>(N.jt)]) labels.push_back(f_.start + 4 * static_cast<u32>(s));
  }
  labels.insert(labels.end(), resume_labels_.begin(), resume_labels_.end());
  std::sort(labels.begin(), labels.end());
  labels.erase(std::unique(labels.begin(), labels.end()), labels.end());
  if (labels.empty() || labels.front() != f_.start) {
    if (error) *error = f_.sym->name + ": the entry is not the first label";
    return false;
  }
  const int first = has_resume_ ? 1 : 0;
  seg_start_.clear();
  if (has_resume_) seg_start_.push_back(0);
  seg_start_.insert(seg_start_.end(), labels.begin(), labels.end());
  seg_at_.assign(n, -1);
  for (std::size_t k = 0, l = 0; k < n; ++k) {
    u32 a = f_.start + 4 * static_cast<u32>(k);
    while (l + 1 < labels.size() && labels[l + 1] <= a) ++l;
    seg_at_[k] = first + static_cast<int>(l);
  }
  // Segments with a branch back to their own start (self loops), and whether
  // any other backward branch needs the dispatcher.
  self_loops_.assign(seg_start_.size(), false);
  for (std::size_t k = 0; k < n; ++k) {
    if (!f_.reachable[k]) continue;
    const Node& N = nodes_[k];
    if ((N.kind == kJump || N.kind == kCJump) && seg_at_[static_cast<std::size_t>(N.s0)] == seg_at_[k])
      self_loops_[static_cast<std::size_t>(seg_at_[k])] = true;
  }
  // MSR[FP] checks (E3a): an FPU instruction is checked unless MSR[FP] is known
  // to be set on every path to it (E2d: a forward "must" dataflow). It is known
  // set after a checked or executed FPU instruction, until a safe point (an
  // interrupt may switch threads), a call (the callee may), mtmsr or sc; at an
  // entry or a resume label it is unknown. So the exception is taken at the first
  // FPU instruction that runs with MSR[FP] clear, as natively; only the number
  // of checks differs from the C++ emitter's per-block rule.
  if (ctx_.opt.fpu_checks) {
    std::vector<std::vector<int>> preds(n);
    std::vector<bool> unknown(n, false);
    for (u32 e : entries) unknown[(e - f_.start) / 4] = true;
    for (u32 r : resume_labels_) unknown[(r - f_.start) / 4] = true;
    for (std::size_t k = 0; k < n; ++k) {
      if (!f_.reachable[k]) continue;
      const Node& N = nodes_[k];
      auto add = [&](int s) {
        if (s >= 0 && static_cast<std::size_t>(s) < n) preds[static_cast<std::size_t>(s)].push_back(static_cast<int>(k));
      };
      add(N.s0);
      add(N.s1);
      if (N.jt >= 0)
        for (int s : jt_succ_[static_cast<std::size_t>(N.jt)]) add(s);
    }
    auto is_fpu = [&](std::size_t k) { return f_.insns[k].flow == Flow::Plain && ppc::uses_fpu(f_.insns[k].op); };
    // known_in: set when the node starts (before its own safe point). Starts
    // optimistic everywhere but at entries and resume labels, then only drops.
    std::vector<u8> known_in(n, 1), known_out(n, 1);
    auto out_of = [&](std::size_t k, bool in) {
      const Node& N = nodes_[k];
      const InsnInfo& x = f_.insns[k];
      bool v = in && N.pre == pNone;
      if (is_fpu(k)) v = true;  // checked, or known set
      if (N.kind == kCall || N.kind == kSc || x.op == Op::mtmsr || x.op == Op::sc) v = false;
      return v;
    };
    for (bool changed = true; changed;) {
      changed = false;
      for (std::size_t k = 0; k < n; ++k) {
        if (!f_.reachable[k]) continue;
        bool in = !unknown[k] && !preds[k].empty();
        for (int p : preds[k]) in = in && known_out[static_cast<std::size_t>(p)];
        bool out = out_of(k, in);
        if (in != static_cast<bool>(known_in[k]) || out != static_cast<bool>(known_out[k])) {
          known_in[k] = in;
          known_out[k] = out;
          changed = true;
        }
      }
    }
    for (std::size_t k = 0; k < n; ++k)
      if (f_.reachable[k] && is_fpu(k) && !(known_in[k] && nodes_[k].pre == pNone)) nodes_[k].fpchk = true;
  }
  dataflow();
  cr_liveness();
  dispatch_ = has_resume_ || mode_ == Mode::Body || !f_.tables.empty();
  for (std::size_t k = 0; k < n && !dispatch_; ++k) {
    if (!f_.reachable[k]) continue;
    const Node& N = nodes_[k];
    if ((N.kind == kJump || N.kind == kCJump) && seg_at_[static_cast<std::size_t>(N.s0)] < seg_at_[k]) dispatch_ = true;
  }
  // V4: without resume labels or secondary entries, loops (and jump tables) as wasm
  // loops and blocks where they nest (plan_loops).
  if (dispatch_ && ctx_.opt.structured_loops && !has_resume_ && mode_ != Mode::Body && plan_loops()) {
    dispatch_ = false;
    structured_ = true;
    ++ctx_.out->stats.structured_functions;
  }
  if (dispatch_) ++ctx_.out->stats.dispatch_functions;
  for (u32 a : resume_labels_) resume_out_.push_back({a, static_cast<u32>(seg_at_[(a - f_.start) / 4])});
  ctx_.out->stats.resume_labels += resume_labels_.size();

  // Locals: cpu [, entry], i32 temporaries and the register locals, then the i64s.
  u32 next = mode_ == Mode::Body ? 2 : 1;
  const u32 params = next;
  L_label = next++;
  L_sp = next++;
  L_tc = next++;
  L_tgt = next++;
  L_tmp = next++;
  L_fpok = next++;
  for (int r = 0; r < reg::kCount; ++r) regloc_[r] = (used_ & ppc::bit(r)) ? static_cast<int>(next++) : -1;
  const u32 n_i32 = next - params;
  L_t = next++;
  L_w = next++;
  L_w2 = next++;
  if (uses_fp_) {
    L_fa = next++;
    L_fb = next++;
    L_fc = next++;
    L_r = next++;
    L_f0 = next++;
    L_f1 = next++;
  }
  // Q4: the halves of every FPR the function's floating-point code names.
  u32 n_fpr_locals = 0;
  {
    std::uint32_t fprs = 0;
    for (std::size_t k = 0; k < n; ++k) {
      if (!f_.reachable[k] || !ppc::uses_fpu(f_.insns[k].op) || f_.insns[k].flow != Flow::Plain) continue;
      const Insn w{f_.insns[k].word};
      fprs |= 1u << w.rD() | 1u << w.rA() | 1u << w.rB() | 1u << w.rC();
    }
    for (int h = 0; h < 64; ++h) {
      fpr_local_[h] = (fprs >> (h / 2) & 1) ? static_cast<int>(next++) : -1;
      if (fpr_local_[h] >= 0) ++n_fpr_locals;
    }
  }
  const u32 n_f64 = (uses_fp_ ? 6 : 0) + n_fpr_locals;
  body.uleb(n_f64 ? 3 : 2);
  body.uleb(n_i32);
  body.byte(kI32);
  body.uleb(3);
  body.byte(kI64);
  if (n_f64) {
    body.uleb(n_f64);
    body.byte(kF64);
  }

  // Preamble.
  load_sp();
  if (mode_ == Mode::Body) {
    body.get(1);
    body.set(L_label);
    RegSet live = 0;
    for (u32 e : f_.entries) live |= nodes_[(e - f_.start) / 4].Lin;
    reload(live);
    load_ticks();
  } else {
    if (has_resume_) {
      body.i32c(0);
      body.set(L_label);
      pre_level_ = body.block();
      glb(body, gResume);
      body.mem(op::i32_load8_u, 0, 0);
      body.br_if(pre_level_);
    }
    if (mode_ == Mode::Plain || mode_ == Mode::Fast) {
      emit_prologue(body, f_.start, [&] {
        if (has_resume_) body.br(pre_level_);
        else leave();
      });
      if (hook_ >= 0) {
        body.get(kCpu);
        body.i32c(hook_);
        body.call(iHleBefore);
        // The hook may stop the run before the first instruction (runtime/hle.h).
        glb(body, gUnwinding);
        body.mem(op::i32_load8_u, 0, 0);
        body.if_();
        if (has_resume_) body.br(pre_level_);
        else leave();
        body.end();
      }
    }
    reload(nodes_[0].Lin);
    load_ticks();
    if (dispatch_) {
      body.i32c(seg_at_[0]);
      body.set(L_label);
    }
    if (has_resume_) body.end();
  }

  // Segments.
  const int nseg = static_cast<int>(seg_start_.size());
  level_.assign(static_cast<std::size_t>(nseg), -1);
  auto emit_segment = [&](int s) {
    cur_seg_ = s;
    if (has_resume_ && s == 0) {
      emit_resume_block();
      return;
    }
    p_ = 0;
    fp_reset();  // Q4: a label: other paths arrive here
    std::size_t k = (seg_start_[static_cast<std::size_t>(s)] - f_.start) / 4;
    std::size_t end = s + 1 < nseg ? (seg_start_[static_cast<std::size_t>(s) + 1] - f_.start) / 4 : n;
    // A segment that branches back to its own start (a loop without inner
    // labels) is wrapped in a wasm loop: those branches are direct, not through
    // the dispatcher (E2d). Its start stays a dispatcher target (resume entries).
    self_loop_ = self_loops_[static_cast<std::size_t>(s)] ? body.loop() : -1;
    bool falls = false;
    std::size_t last = k;
    for (; k < end; ++k) {
      if (!f_.reachable[k]) continue;
      emit_node(k);
      falls = falls_through(f_.insns[k]);
      last = k;
    }
    if (falls) {
      if (last + 1 >= n) emit_fall();
      else materialize();
    }
    if (self_loop_ >= 0) body.end();
    self_loop_ = -1;
  };
  if (dispatch_) {
    loop_level_ = body.loop();
    for (int s = nseg - 1; s >= 0; --s) level_[static_cast<std::size_t>(s)] = body.block();
    body.get(L_label);
    body.br_table(level_, level_[static_cast<std::size_t>(nseg - 1)]);
    for (int s = 0; s < nseg; ++s) {
      body.end();
      emit_segment(s);
    }
    body.end();
  } else if (structured_) {
    emit_region(0, nseg - 1, false, emit_segment);
  } else {
    for (int s = nseg - 1; s >= 1; --s) level_[static_cast<std::size_t>(s)] = body.block();
    emit_segment(0);
    for (int s = 1; s < nseg; ++s) {
      body.end();
      emit_segment(s);
    }
  }
  body.byte(op::end);
  if (body.depth != 0) {
    if (error) *error = f_.sym->name + ": unbalanced blocks";
    return false;
  }
  return true;
}

// ---- in-module helpers ----

void swap32_from_local(Fn& h, u32 local) {  // stack: x (also in `local`) -> bswap(x)
  h.i32c(8);
  h.op(op::i32_rotl);
  h.i32c(0x00FF00FFu);
  h.op(op::i32_and);
  h.get(local);
  h.i32c(8);
  h.op(op::i32_rotr);
  h.i32c(0xFF00FF00u);
  h.op(op::i32_and);
  h.op(op::i32_or);
}

// P2: before a store to guest memory, whether the render thread has still to
// read any of it (env.gp_pages, ppc::g_gp_pages: per 2^WasmGpLayout::kShift
// bytes of physical memory, nonzero while an issued GX command reads them); if
// so rt.gp_wait(cpu, first, last) waits for it. `first` and `last` push the
// first and the last byte's address.
// Emitted only into a module of the threaded engine (WasmEmitOptions::shared_memory):
// elsewhere nothing is ever pending.
thread_local bool t_gp_checks = false;

void gp_check(Fn& h, const std::function<void()>& first, const std::function<void()>& last) {
  if (!t_gp_checks) return;
  auto entry = [&](const std::function<void()>& addr) {
    if (!t_fixed.gp_pages) h.global(gGpPages);
    addr();
    h.i32c(WasmGpLayout::kShift);
    h.op(op::i32_shr_u);
    h.i32c(WasmGpLayout::kMask);
    h.op(op::i32_and);
    h.i32c(2);
    h.op(op::i32_shl);
    if (!t_fixed.gp_pages) h.op(op::i32_add);
    h.mem(op::i32_load, 2, t_fixed.gp_pages);  // V4: the table's address as the offset
  };
  entry(first);
  entry(last);
  h.op(op::i32_or);
  h.if_();
  h.get(0);
  first();
  last();
  h.call(iGpWait);
  h.end();
}

// The host pointer of the 64 KiB page holding `ea` (param 1), teed into `p_local`.
void page_ptr(Fn& h, u32 p_local) {
  h.get(0);
  h.mem(op::i32_load, 2, kMem);
  h.get(1);
  h.i32c(16);
  h.op(op::i32_shr_u);
  h.i32c(2);
  h.op(op::i32_shl);
  h.op(op::i32_add);
  h.mem(op::i32_load, 2, 0);
  h.tee(p_local);
}

void host_addr(Fn& h, u32 p_local) {
  h.get(p_local);
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.op(op::i32_add);
}

// Q6: MEM1's own path (env.mem1, WasmMem1Layout): an access of up to 8 bytes at
// 0x80000000 + t with t < limit is at base + t, the host address `access` finds
// in the i32 local `a`; `access` ends with a return. The page table maps the
// same bytes there (Machine), so the result is the same.
// V4: with MEM1 fixed (WasmFixed::mem1_limit) the limit is a constant and `a`
// keeps t: `access` adds mem1_offset() to its offsets (the load's or store's
// offset immediate holds the base, so the host address is never computed).
u32 mem1_offset() { return t_fixed.mem1_limit ? t_fixed.mem1_base : 0; }
void mem1_path(Fn& h, u32 a, const std::function<void()>& access) {
  using M = WasmMem1Layout;
  h.get(1);
  h.i32c(0x80000000u);
  h.op(op::i32_sub);
  h.tee(a);
  if (t_fixed.mem1_limit) {
    h.i32c(t_fixed.mem1_limit);
    h.op(op::i32_lt_u);
    h.if_();
    access();
    h.end();
    return;
  }
  h.global(gMem1);
  h.mem(op::i32_load, 2, M::kLimit);
  h.op(op::i32_lt_u);
  h.if_();
  h.global(gMem1);
  h.mem(op::i32_load, 2, M::kBase);
  h.get(a);
  h.op(op::i32_add);
  h.set(a);
  access();
  h.end();
}

// Q6: a memory helper's front: MEM1's own path, else a tail call of its page-
// table path (`page`, the same parameters). Small, so V8 inlines it into hot
// callers. The forms: read (cpu, ea, pc, ticks) -> i32 of 1, 2 or 4 bytes;
// write (cpu, ea, value, pc, ticks); read64 -> i64; write64 (cpu, ea, i64, pc,
// ticks); lfs (read32 as a double's bits); stfs (an i64 double stored as single).
enum class Front { Read, Write, Read64, Write64, Lfs, Stfs };
void double_to_single_word(Fn& h, u32 bits);  // below
void single_to_double_inline(Fn& h, u32 word);
Fn helper_front(Front f, int bytes, u32 page) {
  Fn h;
  const bool wr = f == Front::Write || f == Front::Write64 || f == Front::Stfs;
  const u32 params = wr ? 5 : 4;
  const u32 a = params, w = params + 1;  // the host address; a word
  const u32 mo = mem1_offset();  // V4: added to the accesses' offsets (0 unless MEM1 is fixed)
  h.uleb(1);
  h.uleb(2);
  h.byte(kI32);
  auto gp = [&](u32 n) {
    gp_check(h, [&] { h.get(1); }, [&] {
      h.get(1);
      if (n > 1) {
        h.i32c(n - 1);
        h.op(op::i32_add);
      }
    });
  };
  if (f == Front::Stfs) {  // the word first (the page path converts again)
    double_to_single_word(h, 2);
    h.set(w);
  }
  mem1_path(h, a, [&] {
    switch (f) {
      case Front::Read:
        h.get(a);
        if (bytes == 1) {
          h.mem(op::i32_load8_u, 0, mo + 0);
        } else if (bytes == 2) {
          h.mem(op::i32_load16_u, 0, mo + 0);
          h.tee(w);
          h.i32c(8);
          h.op(op::i32_shl);
          h.get(w);
          h.i32c(8);
          h.op(op::i32_shr_u);
          h.op(op::i32_or);
          h.i32c(0xFFFF);
          h.op(op::i32_and);
        } else {
          h.mem(op::i32_load, 0, mo + 0);
          h.tee(w);
          swap32_from_local(h, w);
        }
        break;
      case Front::Write:
        gp(static_cast<u32>(bytes));
        h.get(a);
        if (bytes == 1) {
          h.get(2);
          h.mem(op::i32_store8, 0, mo + 0);
        } else if (bytes == 2) {
          h.get(2);
          h.i32c(8);
          h.op(op::i32_shl);
          h.get(2);
          h.i32c(8);
          h.op(op::i32_shr_u);
          h.i32c(0xFF);
          h.op(op::i32_and);
          h.op(op::i32_or);
          h.mem(op::i32_store16, 0, mo + 0);
        } else {
          h.get(2);
          swap32_from_local(h, 2);
          h.mem(op::i32_store, 0, mo + 0);
        }
        break;
      case Front::Read64:
        for (u32 off : {0u, 4u}) {
          h.get(a);
          h.mem(op::i32_load, 0, mo + off);
          h.tee(w);
          swap32_from_local(h, w);
          h.op(op::i64_extend_i32_u);
          if (!off) {
            h.i64c(32);
            h.op(op::i64_shl);
          }
        }
        h.op(op::i64_or);
        break;
      case Front::Write64:
        gp(8);
        for (u32 off : {0u, 4u}) {
          h.get(a);
          h.get(2);
          if (!off) {
            h.i64c(32);
            h.op(op::i64_shr_u);
          }
          h.op(op::i32_wrap_i64);
          h.tee(w);
          swap32_from_local(h, w);
          h.mem(op::i32_store, 0, mo + off);
        }
        break;
      case Front::Lfs:
        h.get(a);
        h.mem(op::i32_load, 0, mo + 0);
        h.tee(w);
        swap32_from_local(h, w);
        h.set(w);
        single_to_double_inline(h, w);
        break;
      case Front::Stfs:
        gp(4);
        h.get(a);
        h.get(w);
        swap32_from_local(h, w);
        h.mem(op::i32_store, 0, mo + 0);
        break;
    }
    h.op(op::return_);
  });
  for (u32 k = 0; k < params; ++k) h.get(k);
  h.return_call(page);
  h.byte(op::end);
  return h;
}

// (cpu, ea, pc, ticks) -> value: a mapped page and no page crossing read the
// big-endian bytes directly; anything else is ppc::readN in the runtime.
Fn helper_read(int bytes, u32 slow) {
  Fn h;
  h.uleb(1);
  h.uleb(1);
  h.byte(kI32);  // local 4: the page pointer, then the loaded value
  auto slow_call = [&] {
    h.get(0);
    h.get(1);
    h.get(2);
    h.get(3);
    h.call(slow);
  };
  page_ptr(h, 4);
  h.if_i32();
  if (bytes == 1) {
    host_addr(h, 4);
    h.mem(op::i32_load8_u, 0, 0);
  } else {
    h.get(1);
    h.i32c(0xFFFF);
    h.op(op::i32_and);
    h.i32c(0x10000 - bytes);
    h.op(op::i32_le_u);
    h.if_i32();
    host_addr(h, 4);
    if (bytes == 2) {
      h.mem(op::i32_load16_u, 0, 0);
      h.tee(4);
      h.i32c(8);
      h.op(op::i32_shl);
      h.get(4);
      h.i32c(8);
      h.op(op::i32_shr_u);
      h.op(op::i32_or);
      h.i32c(0xFFFF);
      h.op(op::i32_and);
    } else {
      h.mem(op::i32_load, 0, 0);
      h.tee(4);
      swap32_from_local(h, 4);
    }
    h.else_();
    slow_call();
    h.end();
  }
  h.else_();
  slow_call();
  h.end();
  h.byte(op::end);
  return h;
}

// Q4: fp::single_to_double of the word in `word` (an i32 local) as an i64 on
// the stack: f64.promote_f32 is exact for every non-NaN single; a NaN takes
// h.s2d (its bit formula).
void single_to_double_inline(Fn& h, u32 word) {
  h.get(word);
  h.i32c(0x7FFFFFFF);
  h.op(op::i32_and);
  h.i32c(0x7F800000);
  h.op(op::i32_gt_u);
  h.if_i64();
  h.get(word);
  h.call(hS2D);
  h.else_();
  h.get(word);
  h.op(op::f32_reinterpret_i32);
  h.op(op::f64_promote_f32);
  h.op(op::i64_reinterpret_f64);
  h.end();
}

// Q4: fp::double_to_single_ftz (psq_st, float types) of the bits in the i64 local
// `bits`, as an i32 on the stack: the bit formula, or the sign alone below the
// single normal range.
void double_to_single_ftz_inline(Fn& h, u32 bits) {
  h.get(bits);
  h.i64c(32);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0xC0000000u);
  h.op(op::i32_and);
  h.get(bits);
  h.i64c(29);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x3FFFFFFF);
  h.op(op::i32_and);
  h.op(op::i32_or);
  h.get(bits);
  h.i64c(32);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x80000000u);
  h.op(op::i32_and);
  h.get(bits);
  h.i64c(52);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x7FF);
  h.op(op::i32_and);
  h.i32c(896);
  h.op(op::i32_gt_u);
  h.get(bits);
  h.i64c(0x7FFFFFFFFFFFFFFFll);
  h.op(op::i64_and);
  h.op(op::i64_eqz);
  h.op(op::i32_or);
  h.op(op::select);
}

// Q4: lfs: (cpu, ea, pc, ticks) -> the double's bits: read32 (a mapped page
// without crossing: a plain load; else h.rd32) and fp::single_to_double, in one call.
Fn helper_load_single() {
  Fn h;
  h.uleb(1);
  h.uleb(2);
  h.byte(kI32);  // local 4: the page pointer; 5: the word
  page_ptr(h, 4);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.i32c(0x10000 - 4);
  h.op(op::i32_le_u);
  h.if_();
  host_addr(h, 4);
  h.mem(op::i32_load, 0, 0);
  h.tee(5);
  swap32_from_local(h, 5);
  h.set(5);
  single_to_double_inline(h, 5);
  h.op(op::return_);
  h.end();
  h.end();
  h.get(0);
  h.get(1);
  h.get(2);
  h.get(3);
  h.call(hRd32);
  h.set(5);
  single_to_double_inline(h, 5);
  h.byte(op::end);
  return h;
}

// fp::double_to_single of the i64 local `bits`, pushed as an i32: the bit
// formula, or h.d2s where the result is a single denormal (exponents 874-896).
void double_to_single_word(Fn& h, u32 bits) {
  h.get(bits);
  h.i64c(52);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x7FF);
  h.op(op::i32_and);
  h.i32c(874);
  h.op(op::i32_sub);
  h.i32c(896 - 874);
  h.op(op::i32_le_u);
  h.if_i32();
  h.get(bits);
  h.call(hD2S);
  h.else_();
  h.get(bits);
  h.i64c(32);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0xC0000000u);
  h.op(op::i32_and);
  h.get(bits);
  h.i64c(29);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x3FFFFFFF);
  h.op(op::i32_and);
  h.op(op::i32_or);
  h.end();
}

// Q4: stfs: (cpu, ea, double bits, pc, ticks): fp::double_to_single (the bit
// formula, or h.d2s where the result is a single denormal, exponents 874-896)
// and write32 (a mapped page without crossing: a plain store after the threaded
// engine's check; else h.wr32), in one call.
Fn helper_store_single() {
  Fn h;
  h.uleb(1);
  h.uleb(2);
  h.byte(kI32);  // local 5: the page pointer; 6: the word
  double_to_single_word(h, 2);
  h.set(6);
  page_ptr(h, 5);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.i32c(0x10000 - 4);
  h.op(op::i32_le_u);
  h.if_();
  gp_check(h, [&] { h.get(1); }, [&] {
    h.get(1);
    h.i32c(3);
    h.op(op::i32_add);
  });
  host_addr(h, 5);
  h.get(6);
  swap32_from_local(h, 6);
  h.mem(op::i32_store, 0, 0);
  h.op(op::return_);
  h.end();
  h.end();
  h.get(0);
  h.get(1);
  h.get(6);
  h.get(3);
  h.get(4);
  h.call(hWr32);
  h.byte(op::end);
  return h;
}

// Q4: (cpu, ea, pc, ticks) -> the big-endian doubleword (lfd, psq_l of float
// pairs): read64, the high word then the low word. Both in one mapped page:
// plain loads; otherwise two h.rd32 (which take the runtime's path where needed).
Fn helper_read64() {
  Fn h;
  h.uleb(1);
  h.uleb(2);
  h.byte(kI32);  // local 4: the page pointer; 5: a word
  page_ptr(h, 4);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.i32c(0x10000 - 8);
  h.op(op::i32_le_u);
  h.if_();
  for (u32 off : {0u, 4u}) {
    host_addr(h, 4);
    h.mem(op::i32_load, 0, off);
    h.tee(5);
    swap32_from_local(h, 5);
    h.op(op::i64_extend_i32_u);
    if (!off) {
      h.i64c(32);
      h.op(op::i64_shl);
    }
  }
  h.op(op::i64_or);
  h.op(op::return_);
  h.end();
  h.end();
  for (u32 off : {0u, 4u}) {
    h.get(0);
    h.get(1);
    if (off) {
      h.i32c(off);
      h.op(op::i32_add);
    }
    h.get(2);
    h.get(3);
    h.call(hRd32);
    h.op(op::i64_extend_i32_u);
    if (!off) {
      h.i64c(32);
      h.op(op::i64_shl);
    }
  }
  h.op(op::i64_or);
  h.byte(op::end);
  return h;
}

// Q4: (cpu, ea, value, pc, ticks), write64 (stfd, psq_st of float pairs): the
// high word, then the low word. Both in one mapped page: plain stores (after the
// threaded engine's check of the 8 bytes); otherwise two h.wr32.
Fn helper_write64() {
  Fn h;
  h.uleb(1);
  h.uleb(2);
  h.byte(kI32);  // local 5: the page pointer; 6: a word
  page_ptr(h, 5);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.i32c(0x10000 - 8);
  h.op(op::i32_le_u);
  h.if_();
  gp_check(h, [&] { h.get(1); }, [&] {
    h.get(1);
    h.i32c(7);
    h.op(op::i32_add);
  });
  for (u32 off : {0u, 4u}) {
    host_addr(h, 5);
    h.get(2);
    if (!off) {
      h.i64c(32);
      h.op(op::i64_shr_u);
    }
    h.op(op::i32_wrap_i64);
    h.tee(6);
    swap32_from_local(h, 6);
    h.mem(op::i32_store, 0, off);
  }
  h.op(op::return_);
  h.end();
  h.end();
  for (u32 off : {0u, 4u}) {
    h.get(0);
    h.get(1);
    if (off) {
      h.i32c(off);
      h.op(op::i32_add);
    }
    h.get(2);
    if (!off) {
      h.i64c(32);
      h.op(op::i64_shr_u);
    }
    h.op(op::i32_wrap_i64);
    h.get(3);
    h.get(4);
    h.call(hWr32);
  }
  h.byte(op::end);
  return h;
}

// (cpu, ea, value, pc, ticks), as helper_read. P2: a store to the write-gather
// pipe that leaves fewer than 32 bytes gathered is appended to the runtime's
// buffer directly (WasmWgpLayout), as Hardware::write would append it; Q4: one
// that completes the burst (or reaches into the next) goes to rt.wgp_store, which
// gathers it as Hardware::write would, burst included, without the rest of the
// MMIO path (the module counts it and ends the poll detector's run here too).
Fn helper_write(int bytes, u32 slow) {
  Fn h;
  h.uleb(1);
  h.uleb(3);
  h.byte(kI32);  // local 5: the page pointer; 6, 7: the write-gather state and its count
  // The write-gather pipe (an unmapped page, so the runtime's path otherwise):
  // any address in its 4 KiB block, as Hardware::write takes them (Q4: a psq_st
  // writes its second element to 0xCC008004). Q6: first, before the page table
  // (the pipe's stores are this helper's common case: MEM1's go through the
  // front); `fast` is only ever set by a Hardware, whose pipe page is unmapped.
  using L = WasmWgpLayout;
  h.get(1);
  h.i32c(0xCC008000u);
  h.op(op::i32_sub);
  h.i32c(0x1000);
  h.op(op::i32_lt_u);
  h.if_();
  glb(h, gWgp);
  h.tee(6);
  h.mem(op::i32_load, 2, L::kFast);
  h.if_();
  h.get(6);
  h.i32c(0);
  h.mem(op::i32_store, 2, L::kLastRead);
  h.get(6);
  h.get(6);
  h.mem(op::i64_load, 3, L::kFastStores);
  h.i64c(1);
  h.op(op::i64_add);
  h.mem(op::i64_store, 3, L::kFastStores);
  h.get(6);
  h.mem(op::i32_load, 2, L::kN);
  h.tee(7);
  h.i32c(static_cast<u32>(32 - bytes));
  h.op(op::i32_lt_u);
  h.if_();
  h.get(6);
  h.get(7);
  h.op(op::i32_add);
  if (bytes == 1) {
    h.get(2);
    h.mem(op::i32_store8, 0, L::kBuf);
  } else if (bytes == 2) {
    h.get(2);
    h.i32c(8);
    h.op(op::i32_shl);
    h.get(2);
    h.i32c(8);
    h.op(op::i32_shr_u);
    h.i32c(0xFF);
    h.op(op::i32_and);
    h.op(op::i32_or);
    h.mem(op::i32_store16, 0, L::kBuf);
  } else {
    h.get(2);
    swap32_from_local(h, 2);
    h.mem(op::i32_store, 0, L::kBuf);
  }
  h.get(6);
  h.get(7);
  h.i32c(static_cast<u32>(bytes));
  h.op(op::i32_add);
  h.mem(op::i32_store, 2, L::kN);
  h.op(op::return_);
  h.end();
  h.get(0);  // Q4: it completes the burst (or reaches into the next)
  h.get(2);
  h.i32c(static_cast<u32>(bytes));
  h.get(3);
  h.get(4);
  h.call(iWgpStore);
  h.op(op::return_);
  h.end();
  h.end();
  page_ptr(h, 5);
  h.if_();
  if (bytes != 1) {
    h.get(1);
    h.i32c(0xFFFF);
    h.op(op::i32_and);
    h.i32c(0x10000 - bytes);
    h.op(op::i32_le_u);
    h.if_();
  }
  gp_check(h, [&] { h.get(1); }, [&] {
    h.get(1);
    if (bytes > 1) {
      h.i32c(static_cast<u32>(bytes - 1));
      h.op(op::i32_add);
    }
  });
  host_addr(h, 5);
  if (bytes == 1) {
    h.get(2);
    h.mem(op::i32_store8, 0, 0);
  } else if (bytes == 2) {
    h.get(2);
    h.i32c(8);
    h.op(op::i32_shl);
    h.get(2);
    h.i32c(8);
    h.op(op::i32_shr_u);
    h.i32c(0xFF);
    h.op(op::i32_and);
    h.op(op::i32_or);
    h.mem(op::i32_store16, 0, 0);
  } else {
    h.get(2);
    swap32_from_local(h, 2);
    h.mem(op::i32_store, 0, 0);
  }
  h.op(op::return_);
  if (bytes != 1) h.end();
  h.end();
  h.get(0);
  h.get(1);
  h.get(2);
  h.get(3);
  h.get(4);
  h.call(slow);
  h.byte(op::end);
  return h;
}

Fn helper_cmp(bool is_signed) {  // (a, b, so) -> LT GT EQ SO bits
  Fn h;
  h.uleb(0);
  auto rel = [&](u8 opc, int sh) {
    h.get(0);
    h.get(1);
    h.op(opc);
    if (sh) {
      h.i32c(sh);
      h.op(op::i32_shl);
    }
  };
  rel(is_signed ? op::i32_lt_s : op::i32_lt_u, 3);
  rel(is_signed ? op::i32_gt_s : op::i32_gt_u, 2);
  h.op(op::i32_or);
  rel(op::i32_eq, 1);
  h.op(op::i32_or);
  h.get(2);
  h.op(op::i32_or);
  h.byte(op::end);
  return h;
}

Fn helper_div(bool is_signed) {  // sem::divw / divwu, the hardware's overflow results included
  Fn h;
  h.uleb(0);
  if (is_signed) {
    h.get(1);
    h.op(op::i32_eqz);
    h.get(0);
    h.i32c(static_cast<s32>(0x80000000u));
    h.op(op::i32_eq);
    h.get(1);
    h.i32c(-1);
    h.op(op::i32_eq);
    h.op(op::i32_and);
    h.op(op::i32_or);
    h.if_i32();
    h.get(0);
    h.i32c(31);
    h.op(op::i32_shr_s);
    h.else_();
    h.get(0);
    h.get(1);
    h.op(op::i32_div_s);
    h.end();
  } else {
    h.get(1);
    h.op(op::i32_eqz);
    h.if_i32();
    h.i32c(0);
    h.else_();
    h.get(0);
    h.get(1);
    h.op(op::i32_div_u);
    h.end();
  }
  h.byte(op::end);
  return h;
}

// fp::single_to_double (lfs): (s) -> double bits. f64.promote_f32 is exact for
// every non-NaN single (denormals included); a NaN keeps its payload by the bit
// formula (WebAssembly's NaN conversions are not deterministic).
Fn helper_single_to_double() {
  Fn h;
  h.uleb(0);
  h.get(0);
  h.i32c(0x7FFFFFFF);
  h.op(op::i32_and);
  h.i32c(0x7F800000);
  h.op(op::i32_gt_u);
  h.if_i64();
  h.get(0);
  h.i32c(0x80000000u);
  h.op(op::i32_and);
  h.op(op::i64_extend_i32_u);
  h.i64c(32);
  h.op(op::i64_shl);
  h.i64c(0x7FF0000000000000ll);
  h.op(op::i64_or);
  h.get(0);
  h.i32c(0x7FFFFF);
  h.op(op::i32_and);
  h.op(op::i64_extend_i32_u);
  h.i64c(29);
  h.op(op::i64_shl);
  h.op(op::i64_or);
  h.else_();
  h.get(0);
  h.op(op::f32_reinterpret_i32);
  h.op(op::f64_promote_f32);
  h.op(op::i64_reinterpret_f64);
  h.end();
  h.byte(op::end);
  return h;
}

// fp::double_to_single (stfs: no rounding, denormalizing), in integer arithmetic.
Fn helper_double_to_single() {
  Fn h;
  h.uleb(1);
  h.uleb(1);
  h.byte(kI32);  // local 1: the exponent
  h.get(0);
  h.i64c(52);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x7FF);
  h.op(op::i32_and);
  h.tee(1);
  h.i32c(874);
  h.op(op::i32_lt_u);
  h.get(1);
  h.i32c(896);
  h.op(op::i32_gt_u);
  h.op(op::i32_or);
  h.get(0);
  h.i64c(0x7FFFFFFFFFFFFFFFll);
  h.op(op::i64_and);
  h.op(op::i64_eqz);
  h.op(op::i32_or);
  h.if_i32();
  // The top two bits of the high word and the next 30 bits of the double.
  h.get(0);
  h.i64c(32);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0xC0000000u);
  h.op(op::i32_and);
  h.get(0);
  h.i64c(29);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x3FFFFFFF);
  h.op(op::i32_and);
  h.op(op::i32_or);
  h.else_();
  // Denormalize: the significand with its hidden bit, shifted right, truncated.
  h.get(0);
  h.i64c(0x000FFFFFFFFFFFFFll);
  h.op(op::i64_and);
  h.i64c(21);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x80000000u);
  h.op(op::i32_or);
  h.i32c(905);
  h.get(1);
  h.op(op::i32_sub);
  h.op(op::i32_shr_u);
  h.get(0);
  h.i64c(32);
  h.op(op::i64_shr_u);
  h.op(op::i32_wrap_i64);
  h.i32c(0x80000000u);
  h.op(op::i32_and);
  h.op(op::i32_or);
  h.end();
  h.byte(op::end);
  return h;
}

// lmw / stmw (E2d): (cpu, ea, first, pc, ticks). The words move between guest
// memory and cpu.gpr[first..31] in register order. When the whole run lies in one
// mapped page every word is a plain host access (as sem's read32/write32 would
// make it); otherwise each word goes through h.rd32 / h.wr32, which take the
// runtime's path for MMIO, unmapped pages and page crossings.
Fn helper_multiple(bool store) {
  Fn h;
  h.uleb(1);
  h.uleb(4);
  h.byte(kI32);  // 5 page, 6 host pointer, 7 register, 8 word
  page_ptr(h, 5);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.i32c(128);
  h.get(2);
  h.i32c(2);
  h.op(op::i32_shl);
  h.op(op::i32_sub);
  h.op(op::i32_add);
  h.i32c(0x10000);
  h.op(op::i32_le_u);
  h.if_();
  if (store)  // the words [ea, ea + 4 * (32 - first))
    gp_check(h, [&] { h.get(1); }, [&] {
      h.get(1);
      h.i32c(127);
      h.op(op::i32_add);
      h.get(2);
      h.i32c(2);
      h.op(op::i32_shl);
      h.op(op::i32_sub);
    });
  host_addr(h, 5);
  h.set(6);
  h.get(2);
  h.set(7);
  int fast = h.loop();
  if (store) {
    h.get(6);
    h.get(0);
    h.get(7);
    h.i32c(2);
    h.op(op::i32_shl);
    h.op(op::i32_add);
    h.mem(op::i32_load, 2, kGpr);
    h.tee(8);
    swap32_from_local(h, 8);
    h.mem(op::i32_store, 0, 0);
  } else {
    h.get(0);
    h.get(7);
    h.i32c(2);
    h.op(op::i32_shl);
    h.op(op::i32_add);
    h.get(6);
    h.mem(op::i32_load, 0, 0);
    h.tee(8);
    swap32_from_local(h, 8);
    h.mem(op::i32_store, 2, kGpr);
  }
  h.get(6);
  h.i32c(4);
  h.op(op::i32_add);
  h.set(6);
  h.get(7);
  h.i32c(1);
  h.op(op::i32_add);
  h.tee(7);
  h.i32c(32);
  h.op(op::i32_lt_u);
  h.br_if(fast);
  h.end();
  h.op(op::return_);
  h.end();
  h.end();
  // Word by word.
  h.get(2);
  h.set(7);
  int slow = h.loop();
  if (store) {
    h.get(0);
    h.get(1);
    h.get(0);
    h.get(7);
    h.i32c(2);
    h.op(op::i32_shl);
    h.op(op::i32_add);
    h.mem(op::i32_load, 2, kGpr);
    h.get(3);
    h.get(4);
    h.call(hWr32);
  } else {
    h.get(0);
    h.get(7);
    h.i32c(2);
    h.op(op::i32_shl);
    h.op(op::i32_add);
    h.get(0);
    h.get(1);
    h.get(3);
    h.get(4);
    h.call(hRd32);
    h.mem(op::i32_store, 2, kGpr);
  }
  h.get(1);
  h.i32c(4);
  h.op(op::i32_add);
  h.set(1);
  h.get(7);
  h.i32c(1);
  h.op(op::i32_add);
  h.tee(7);
  h.i32c(32);
  h.op(op::i32_lt_u);
  h.br_if(slow);
  h.end();
  h.byte(op::end);
  return h;
}

// 2^k as f64 bits, k from an i32 local holding scale_exp's 6-bit field (fp.cpp:
// scale - 64 when bit 5 is set), negated for loads. Leaves the f64 on the stack.
void pow2_of_scale(Fn& h, u32 scale_local, bool negate) {
  h.i32c(1023);
  h.get(scale_local);
  h.get(scale_local);
  h.i32c(32);
  h.op(op::i32_and);
  h.i32c(1);
  h.op(op::i32_shl);
  h.op(op::i32_sub);  // scale_exp
  h.op(negate ? op::i32_sub : op::i32_add);
  h.op(op::i64_extend_i32_u);
  h.i64c(52);
  h.op(op::i64_shl);
  h.op(op::f64_reinterpret_i64);
}

// psq_l (E2d): (cpu, ea, fpr offset, gqr index | w << 3, pc, ticks), sem::psq_load:
// both raw values are read (ps0 first) before the FPR is written. Float types
// (0-3) convert like lfs; integer types dequantize exactly: v * 2^-scale is a
// small integer times a power of two, exact in double and single (fp::dequantize;
// zero is +0). When the whole access lies in one mapped page the bytes are read
// directly (as sem's read8/16/32 would), else element by element through h.rd*.
Fn helper_psq_load() {
  Fn h;
  h.uleb(3);
  h.uleb(4);
  h.byte(kI32);  // 6 GQR, 7 scale / raw value, 8 element size / page, 9 host pointer (0: slow)
  h.uleb(2);
  h.byte(kI64);  // 10 ps0, 11 ps1
  h.uleb(1);
  h.byte(kF64);  // 12 the scale factor
  // GQR[i]
  h.get(0);
  h.get(3);
  h.i32c(7);
  h.op(op::i32_and);
  h.i32c(2);
  h.op(op::i32_shl);
  h.op(op::i32_add);
  h.mem(op::i32_load, 2, kGqr);
  h.set(6);
  // Q4: the common case first: a float pair (load type 0-3, W clear) in one
  // mapped page; both raw words are read, then both halves written.
  h.get(6);
  h.i32c(0x40000);
  h.op(op::i32_and);
  h.get(3);
  h.i32c(8);
  h.op(op::i32_and);
  h.op(op::i32_or);
  h.op(op::i32_eqz);
  h.if_();
  page_ptr(h, 7);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.i32c(0x10000 - 8);
  h.op(op::i32_le_u);
  h.if_();
  host_addr(h, 7);
  h.set(9);
  for (u32 half : {0u, 1u}) {
    h.get(9);
    h.mem(op::i32_load, 0, 4 * half);
    h.tee(8);
    swap32_from_local(h, 8);
    h.set(8);
    single_to_double_inline(h, 8);
    h.set(half ? 11 : 10);
  }
  h.get(0);
  h.get(2);
  h.op(op::i32_add);
  h.get(10);
  h.mem(op::i64_store, 3, 0);
  h.get(0);
  h.get(2);
  h.op(op::i32_add);
  h.get(11);
  h.mem(op::i64_store, 3, 8);
  h.op(op::return_);
  h.end();
  h.end();
  h.end();
  // Element size: 4 (float types), 2 (types 5, 7) or 1 (types 4, 6); the access
  // covers one element with W set, else two.
  h.i32c(4);
  h.i32c(2);
  h.i32c(1);
  h.get(6);
  h.i32c(0x10000);
  h.op(op::i32_and);
  h.op(op::select);
  h.get(6);
  h.i32c(0x40000);
  h.op(op::i32_and);
  h.op(op::i32_eqz);
  h.op(op::select);
  h.set(8);
  h.i32c(0);
  h.set(9);
  page_ptr(h, 7);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.get(8);
  h.get(3);
  h.i32c(3);
  h.op(op::i32_shr_u);
  h.i32c(1);
  h.op(op::i32_and);
  h.i32c(1);
  h.op(op::i32_xor);
  h.op(op::i32_shl);  // size << !w
  h.op(op::i32_add);
  h.i32c(0x10000);
  h.op(op::i32_le_u);
  h.if_();
  host_addr(h, 7);
  h.set(9);
  h.end();
  h.end();
  auto body = [&](bool fast) {
    // The raw element at byte offset `off` (zero-extended).
    auto rd = [&](int bytes, u32 off) {
      if (fast) {
        h.get(9);
        if (bytes == 1) {
          h.mem(op::i32_load8_u, 0, off);
        } else if (bytes == 2) {
          h.mem(op::i32_load16_u, 0, off);
          h.tee(7);
          h.i32c(8);
          h.op(op::i32_shl);
          h.get(7);
          h.i32c(8);
          h.op(op::i32_shr_u);
          h.op(op::i32_or);
          h.i32c(0xFFFF);
          h.op(op::i32_and);
        } else {
          h.mem(op::i32_load, 0, off);
          h.tee(7);
          swap32_from_local(h, 7);
        }
        return;
      }
      h.get(0);
      h.get(1);
      if (off) {
        h.i32c(off);
        h.op(op::i32_add);
      }
      h.get(4);
      h.get(5);
      h.call(bytes == 1 ? hRd8 : bytes == 2 ? hRd16 : hRd32);
    };
    auto ps1_or_one = [&](const std::function<void()>& value) {
      h.get(3);
      h.i32c(8);
      h.op(op::i32_and);
      h.if_i64();
      h.i64c(static_cast<s64>(0x3FF0000000000000ull));
      h.else_();
      value();
      h.end();
      h.set(11);
    };
    h.get(6);
    h.i32c(0x40000);  // load type bit 2: integer types 4-7
    h.op(op::i32_and);
    h.op(op::i32_eqz);
    h.if_();
    rd(4, 0);
    h.call(hS2D);
    h.set(10);
    ps1_or_one([&] {
      rd(4, 4);
      h.call(hS2D);
    });
    h.else_();
    h.get(6);
    h.i32c(24);
    h.op(op::i32_shr_u);
    h.i32c(63);
    h.op(op::i32_and);
    h.set(7);
    pow2_of_scale(h, 7, true);
    h.set(12);
    // A raw value as a dequantized f64 (as i64 bits): 8 or 16 bits, signed for types 6 and 7.
    auto deq = [&](bool wide, u32 off) {
      rd(wide ? 2 : 1, off);
      h.tee(7);
      h.op(wide ? op::i32_extend16_s : op::i32_extend8_s);
      h.get(7);
      h.get(6);
      h.i32c(0x20000);
      h.op(op::i32_and);
      h.op(op::select);
      h.op(op::f64_convert_i32_s);
      h.get(12);
      h.op(op::f64_mul);
      h.op(op::i64_reinterpret_f64);
    };
    h.get(6);
    h.i32c(0x10000);  // types 5 and 7: 16 bits
    h.op(op::i32_and);
    h.if_();
    deq(true, 0);
    h.set(10);
    ps1_or_one([&] { deq(true, 2); });
    h.else_();
    deq(false, 0);
    h.set(10);
    ps1_or_one([&] { deq(false, 1); });
    h.end();
    h.end();
  };
  h.get(9);
  h.if_();
  body(true);
  h.else_();
  body(false);
  h.end();
  h.get(0);
  h.get(2);
  h.op(op::i32_add);
  h.get(10);
  h.mem(op::i64_store, 3, 0);
  h.get(0);
  h.get(2);
  h.op(op::i32_add);
  h.get(11);
  h.mem(op::i64_store, 3, 8);
  h.byte(op::end);
  return h;
}

// psq_st (E2d): (cpu, ea, fpr offset, gqr index | w << 3, pc, ticks), sem::psq_store
// (ps0 written first). Float types: fp::double_to_single_ftz on the bits. Integer
// types, fp::quantize: a NaN saturates high; otherwise round to single (nearest,
// subnormals kept: f32.demote), scale by 2^scale (exact in double), truncate
// toward zero and saturate (i32.trunc_sat, then the type's range). One mapped
// page: direct stores; else element by element through h.wr*.
Fn helper_psq_store() {
  Fn h;
  h.uleb(3);
  h.uleb(6);
  h.byte(kI32);  // 6 GQR, 7 low bound / page, 8 high bound, 9 value, 10 element size, 11 host pointer
  h.uleb(1);
  h.byte(kI64);  // 12 the FPR half's bits
  h.uleb(2);
  h.byte(kF64);  // 13 the scale factor, 14 the FPR half
  h.get(0);
  h.get(3);
  h.i32c(7);
  h.op(op::i32_and);
  h.i32c(2);
  h.op(op::i32_shl);
  h.op(op::i32_add);
  h.mem(op::i32_load, 2, kGqr);
  h.set(6);
  // Q4: the common case first: a float pair (store type 0-3, W clear). Both
  // words (ps0's first) to one mapped page, or appended to the write-gather pipe
  // while they leave its burst unfinished (as two h.wr32 would append them);
  // otherwise through h.wr32.
  h.get(6);
  h.i32c(4);
  h.op(op::i32_and);
  h.get(3);
  h.i32c(8);
  h.op(op::i32_and);
  h.op(op::i32_or);
  h.op(op::i32_eqz);
  h.if_();
  for (u32 half : {0u, 1u}) {
    h.get(0);
    h.get(2);
    h.op(op::i32_add);
    h.mem(op::i64_load, 3, 8 * half);
    h.set(12);
    double_to_single_ftz_inline(h, 12);
    h.set(half ? 8 : 9);
  }
  page_ptr(h, 7);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.i32c(0x10000 - 8);
  h.op(op::i32_le_u);
  h.if_();
  gp_check(h, [&] { h.get(1); }, [&] {
    h.get(1);
    h.i32c(7);
    h.op(op::i32_add);
  });
  for (u32 half : {0u, 1u}) {
    host_addr(h, 7);
    h.get(half ? 8 : 9);
    swap32_from_local(h, half ? 8 : 9);
    h.mem(op::i32_store, 0, 4 * half);
  }
  h.op(op::return_);
  h.end();
  h.else_();
  {
    using L = WasmWgpLayout;
    h.get(1);
    h.i32c(0xCC008000u);
    h.op(op::i32_sub);
    h.i32c(0x1000);
    h.op(op::i32_lt_u);
    h.if_();
    glb(h, gWgp);
    h.tee(10);
    h.mem(op::i32_load, 2, L::kFast);
    h.if_();
    h.get(10);
    h.mem(op::i32_load, 2, L::kN);
    h.tee(11);
    h.i32c(32 - 8);
    h.op(op::i32_lt_u);
    h.if_();
    for (u32 half : {0u, 1u}) {
      h.get(10);
      h.get(11);
      h.op(op::i32_add);
      h.get(half ? 8 : 9);
      swap32_from_local(h, half ? 8 : 9);
      h.mem(op::i32_store, 0, L::kBuf + 4 * half);
    }
    h.get(10);
    h.get(11);
    h.i32c(8);
    h.op(op::i32_add);
    h.mem(op::i32_store, 2, L::kN);
    h.get(10);
    h.i32c(0);
    h.mem(op::i32_store, 2, L::kLastRead);
    h.get(10);
    h.get(10);
    h.mem(op::i64_load, 3, L::kFastStores);
    h.i64c(2);
    h.op(op::i64_add);
    h.mem(op::i64_store, 3, L::kFastStores);
    h.op(op::return_);
    h.end();
    h.end();
    h.end();
  }
  h.end();
  for (u32 half : {0u, 1u}) {
    h.get(0);
    h.get(1);
    if (half) {
      h.i32c(4);
      h.op(op::i32_add);
    }
    h.get(half ? 8 : 9);
    h.get(4);
    h.get(5);
    h.call(hWr32);
  }
  h.op(op::return_);
  h.end();
  h.i32c(4);
  h.i32c(2);
  h.i32c(1);
  h.get(6);
  h.i32c(1);
  h.op(op::i32_and);
  h.op(op::select);
  h.get(6);
  h.i32c(4);
  h.op(op::i32_and);
  h.op(op::i32_eqz);
  h.op(op::select);
  h.set(10);
  h.i32c(0);
  h.set(11);
  page_ptr(h, 7);
  h.if_();
  h.get(1);
  h.i32c(0xFFFF);
  h.op(op::i32_and);
  h.get(10);
  h.get(3);
  h.i32c(3);
  h.op(op::i32_shr_u);
  h.i32c(1);
  h.op(op::i32_and);
  h.i32c(1);
  h.op(op::i32_xor);
  h.op(op::i32_shl);
  h.op(op::i32_add);
  h.i32c(0x10000);
  h.op(op::i32_le_u);
  h.if_();
  gp_check(h, [&] { h.get(1); }, [&] {  // the 1 to 8 bytes written (the last of the 8 a pair can cover)
    h.get(1);
    h.i32c(7);
    h.op(op::i32_add);
  });
  host_addr(h, 7);
  h.set(11);
  h.end();
  h.end();
  auto body = [&](bool fast) {
    // Writes the value `value` pushes as `bytes` big-endian bytes at byte offset `off`.
    auto wr = [&](int bytes, u32 off, const std::function<void()>& value) {
      if (fast) {
        h.get(11);
        value();
        if (bytes == 1) {
          h.mem(op::i32_store8, 0, off);
        } else if (bytes == 2) {
          h.tee(9);
          h.i32c(8);
          h.op(op::i32_shl);
          h.get(9);
          h.i32c(8);
          h.op(op::i32_shr_u);
          h.i32c(0xFF);
          h.op(op::i32_and);
          h.op(op::i32_or);
          h.mem(op::i32_store16, 0, off);
        } else {
          h.tee(9);
          swap32_from_local(h, 9);
          h.mem(op::i32_store, 0, off);
        }
        return;
      }
      h.get(0);
      h.get(1);
      if (off) {
        h.i32c(off);
        h.op(op::i32_add);
      }
      value();
      h.get(4);
      h.get(5);
      h.call(bytes == 1 ? hWr8 : bytes == 2 ? hWr16 : hWr32);
    };
    auto unless_w = [&](const std::function<void()>& then) {
      h.get(3);
      h.i32c(8);
      h.op(op::i32_and);
      h.op(op::i32_eqz);
      h.if_();
      then();
      h.end();
    };
    h.get(6);
    h.i32c(4);  // store type bit 2
    h.op(op::i32_and);
    h.op(op::i32_eqz);
    h.if_();
    {
      auto ftz = [&](u32 half) {  // fp::double_to_single_ftz of the half at fpr offset + half
        h.get(0);
        h.get(2);
        h.op(op::i32_add);
        h.mem(op::i64_load, 3, half);
        h.tee(12);
        h.i64c(52);
        h.op(op::i64_shr_u);
        h.op(op::i32_wrap_i64);
        h.i32c(0x7FF);
        h.op(op::i32_and);
        h.i32c(896);
        h.op(op::i32_gt_u);
        h.get(12);
        h.i64c(0x7FFFFFFFFFFFFFFFll);
        h.op(op::i64_and);
        h.op(op::i64_eqz);
        h.op(op::i32_or);
        h.if_i32();
        h.get(12);
        h.i64c(32);
        h.op(op::i64_shr_u);
        h.op(op::i32_wrap_i64);
        h.i32c(0xC0000000u);
        h.op(op::i32_and);
        h.get(12);
        h.i64c(29);
        h.op(op::i64_shr_u);
        h.op(op::i32_wrap_i64);
        h.i32c(0x3FFFFFFF);
        h.op(op::i32_and);
        h.op(op::i32_or);
        h.else_();
        h.get(12);
        h.i64c(32);
        h.op(op::i64_shr_u);
        h.op(op::i32_wrap_i64);
        h.i32c(0x80000000u);
        h.op(op::i32_and);
        h.end();
      };
      wr(4, 0, [&] { ftz(0); });
      unless_w([&] { wr(4, 4, [&] { ftz(8); }); });
    }
    h.else_();
    {
      // Bounds: types 4 (u8) 0..255, 5 (u16) 0..65535, 6 (s8) -128..127, 7 (s16) -32768..32767.
      h.i32c(65535);
      h.i32c(255);
      h.get(6);
      h.i32c(1);
      h.op(op::i32_and);
      h.op(op::select);
      h.get(6);
      h.i32c(2);
      h.op(op::i32_and);
      h.i32c(1);
      h.op(op::i32_shr_u);
      h.op(op::i32_shr_u);  // signed types: half the range
      h.set(8);
      h.get(8);
      h.i32c(-1);
      h.op(op::i32_xor);  // signed: -hi - 1
      h.i32c(0);
      h.get(6);
      h.i32c(2);
      h.op(op::i32_and);
      h.op(op::select);
      h.set(7);
      h.get(6);
      h.i32c(8);
      h.op(op::i32_shr_u);
      h.i32c(63);
      h.op(op::i32_and);
      h.set(9);
      pow2_of_scale(h, 9, false);
      h.set(13);
      auto quant = [&](u32 half) {
        h.get(0);
        h.get(2);
        h.op(op::i32_add);
        h.mem(op::f64_load, 3, half);
        h.tee(14);
        h.op(op::f32_demote_f64);
        h.op(op::f64_promote_f32);
        h.get(13);
        h.op(op::f64_mul);
        h.fc(op::fc_i32_trunc_sat_f64_s);
        h.set(9);
        h.get(7);
        h.get(9);
        h.get(9);
        h.get(7);
        h.op(op::i32_lt_s);
        h.op(op::select);  // t < lo ? lo : t
        h.set(9);
        h.get(8);
        h.get(9);
        h.get(9);
        h.get(8);
        h.op(op::i32_gt_s);
        h.op(op::select);  // t > hi ? hi : t
        h.get(8);
        h.get(14);
        h.get(14);
        h.op(op::f64_eq);
        h.op(op::select);  // a NaN saturates high
      };
      h.get(6);
      h.i32c(1);
      h.op(op::i32_and);
      h.if_();
      wr(2, 0, [&] { quant(0); });
      unless_w([&] { wr(2, 2, [&] { quant(8); }); });
      h.else_();
      wr(1, 0, [&] { quant(0); });
      unless_w([&] { wr(1, 1, [&] { quant(8); }); });
      h.end();
    }
    h.end();
  };
  h.get(11);
  h.if_();
  body(true);
  h.else_();
  body(false);
  h.end();
  h.byte(op::end);
  return h;
}

void functype(Buf& b, const std::vector<u8>& params, const std::vector<u8>& results) {
  b.byte(0x60);
  b.uleb(params.size());
  for (u8 t : params) b.byte(t);
  b.uleb(results.size());
  for (u8 t : results) b.byte(t);
}

}  // namespace

const std::vector<std::string>& wasm_import_names() {
  static const std::vector<std::string> names = {
      "on_entry", "safepoint", "read8",    "read16",     "read32", "write8", "write16",     "write32", "exec",
      "call_indirect", "absolute_call", "resume", "hle_call", "hle_before", "trap",   "rfi",    "system_call", "tw",
      "fp_unavailable", "gp_wait", "wgp_store"};
  return names;
}

const char* wasm_trap_text(std::uint32_t code) {
  switch (static_cast<WasmTrap>(code)) {
    case WasmTrap::InvalidInstruction: return "invalid instruction";
    case WasmTrap::UnconditionalTrap: return "unconditional trap";
    case WasmTrap::RanOffEnd: return "ran off the end of a function";
    case WasmTrap::JumpOutsideTable: return "jump outside its table";
    case WasmTrap::ExternalControl: return "external control instruction";
    case WasmTrap::BadEntry: return "bad entry";
  }
  return "trap";
}

bool emit_wasm(const ProgramCfg& p, const WasmEmitOptions& opt, WasmModule* out, std::string* error) {
  t_gp_checks = opt.shared_memory;
  t_fixed = opt.fixed;
  *out = WasmModule{};
  out->fixed = opt.fixed;
  if (p.errors) {
    if (error) *error = "the CFG has errors";
    return false;
  }
  if (wasm_import_names().size() != kImports) {
    if (error) *error = "import table mismatch";
    return false;
  }
  Ctx ctx{p, opt, {}, {}, out};
  ctx.start_slot.assign(p.functions.size(), -1);
  out->stats.helper_by_op.assign(static_cast<std::size_t>(Op::count), 0);
  std::vector<WFunc> funcs;
  u32 slot = 0;
  auto index_of = [&](std::size_t k) { return kFirstGuest + static_cast<u32>(k); };
  // Plan every wasm function first: direct calls need their indices.
  for (std::size_t fi = 0; fi < p.functions.size(); ++fi) {
    const FunctionCfg& f = p.functions[fi];
    bool multi = f.entries.size() > 1;
    const HleSpec* spec = find_spec(opt, f);
    if (spec && multi) spec = nullptr;  // as emit_cpp: not with secondary entries
    if (spec) out->hle_matched.push_back(spec->index);
    if (spec && !spec->before) {
      u32 orig = index_of(funcs.size());
      funcs.push_back({WKind::Orig, fi, f.start, static_cast<int>(slot++)});
      ctx.callable[f.start] = index_of(funcs.size());
      ctx.start_slot[fi] = static_cast<int>(slot);
      funcs.push_back({WKind::HleStub, fi, f.start, static_cast<int>(slot++), spec->index, false, orig});
      out->hle_sites.push_back({f.start, spec->index});
    } else if (multi) {
      u32 body = index_of(funcs.size());
      funcs.push_back({WKind::Body, fi});
      for (u32 e : f.entries) {
        ctx.callable[e] = index_of(funcs.size());
        if (e == f.start) ctx.start_slot[fi] = static_cast<int>(slot);
        funcs.push_back({WKind::Stub, fi, e, static_cast<int>(slot++), -1, false, body});
      }
    } else if (opt.fast_entries && has_resume_labels(f)) {
      // V4: the Fast function for calls and the entry, the Slow copy for resume entries.
      ctx.callable[f.start] = index_of(funcs.size());
      funcs.push_back(
          {WKind::Fast, fi, f.start, static_cast<int>(slot++), spec ? spec->index : -1, spec && spec->before});
      ctx.start_slot[fi] = static_cast<int>(slot);
      funcs.push_back(
          {WKind::Slow, fi, f.start, static_cast<int>(slot++), spec ? spec->index : -1, spec && spec->before});
    } else {
      ctx.callable[f.start] = index_of(funcs.size());
      ctx.start_slot[fi] = static_cast<int>(slot);
      funcs.push_back(
          {WKind::Plain, fi, f.start, static_cast<int>(slot++), spec ? spec->index : -1, spec && spec->before});
    }
  }
  out->slots = slot;

  // The module (V4: before the bodies, which go straight into its code section,
  // so the code is never copied: the heap's high-water mark stays one module).
  Buf m;
  m.b.reserve(static_cast<std::size_t>(p.reachable_words) * (opt.fast_entries ? 80 : 42));
  const u8 header[] = {0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00};  //  asm, version 1
  for (u8 c : header) m.byte(c);
  {
    Buf t;
    t.uleb(kTypes);
    functype(t, {kI32}, {});                          // tV_I
    functype(t, {kI32, kI32}, {});                    // tV_II
    functype(t, {kI32, kI32, kI32, kI64}, {kI32});    // tI_IIIL
    functype(t, {kI32, kI32, kI32, kI32, kI64}, {});  // tV_IIIIL
    functype(t, {kI32, kI32, kI32, kI64}, {});        // tV_IIIL
    functype(t, {kI32, kI32, kI32}, {kI32});          // tI_III
    functype(t, {kI32, kI32, kI32}, {});              // tV_III
    functype(t, {kI32, kI32}, {kI32});                // tI_II
    functype(t, {kI32}, {kI64});                      // tL_I
    functype(t, {kI64}, {kI32});                      // tI_L
    functype(t, {kI32, kI32, kI32, kI32, kI32, kI64}, {});  // tV_IIIIIL
    functype(t, {kI32, kI32, kI32, kI64}, {kI64});          // tL_IIIL
    functype(t, {kI32, kI32, kI64, kI32, kI64}, {});        // tV_IILIL
    m.section(1, t);
  }
  {
    Buf im;
    const auto& names = wasm_import_names();
    im.uleb(kImports + 2 + kGlobals);
    for (u32 k = 0; k < kImports; ++k) {
      im.name("rt");
      im.name(names[k]);
      im.byte(0x00);
      im.uleb(kImportType[k]);
    }
    im.name("env");
    im.name("memory");
    im.byte(0x02);
    if (opt.shared_memory) {  // P2: the threaded engine's memory (shared, so it declares a maximum)
      im.byte(0x03);
      im.uleb(1);
      im.uleb(65536);
    } else {
      im.byte(0x00);
      im.uleb(1);
    }
    im.name("env");
    im.name("table");
    im.byte(0x01);
    im.byte(kFuncRef);
    im.byte(0x00);
    im.uleb(1);
    for (const char* g : {"table_base", "unwinding", "resume", "entry_hook", "wgp", "gp_pages", "mem1"}) {
      im.name("env");
      im.name(g);
      im.byte(0x03);
      im.byte(kI32);
      im.byte(0x00);
    }
    m.section(2, im);
  }
  {
    Buf fs;
    fs.uleb(kHelperCount + funcs.size());
    for (Ty t : kHelperType) fs.uleb(t);
    for (const WFunc& w : funcs) fs.uleb(w.kind == WKind::Body ? tV_II : tV_I);
    m.section(3, fs);
  }
  {
    Buf el;
    el.uleb(1);
    el.uleb(0);  // active, table 0, funcref
    el.byte(op::global_get);
    el.uleb(gTableBase);
    el.byte(op::end);
    std::vector<u32> by_slot(slot);
    for (std::size_t k = 0; k < funcs.size(); ++k)
      if (funcs[k].slot >= 0) by_slot[static_cast<std::size_t>(funcs[k].slot)] = index_of(k);
    el.uleb(by_slot.size());
    for (u32 fidx : by_slot) el.uleb(fidx);
    m.section(9, el);
  }
  // Bodies: the code section, its size patched at the end (a 5-byte LEB128).
  m.byte(10);
  const std::size_t code_size_at = m.b.size();
  for (int k = 0; k < 5; ++k) m.byte(k < 4 ? 0x80 : 0x00);
  const std::size_t code_start = m.b.size();
  m.uleb(kHelperCount + funcs.size());
  auto put = [&](const Fn& fn) {
    m.uleb(fn.b.size());
    m.append(fn.b);
  };
  put(helper_front(Front::Read, 1, hRd8P));
  put(helper_front(Front::Read, 2, hRd16P));
  put(helper_front(Front::Read, 4, hRd32P));
  put(helper_front(Front::Write, 1, hWr8P));
  put(helper_front(Front::Write, 2, hWr16P));
  put(helper_front(Front::Write, 4, hWr32P));
  put(helper_cmp(true));
  put(helper_cmp(false));
  put(helper_div(true));
  put(helper_div(false));
  put(helper_single_to_double());
  put(helper_double_to_single());
  put(helper_multiple(false));
  put(helper_multiple(true));
  put(helper_psq_load());
  put(helper_psq_store());
  put(helper_front(Front::Read64, 8, hRd64P));
  put(helper_front(Front::Write64, 8, hWr64P));
  put(helper_front(Front::Lfs, 4, hLfsP));
  put(helper_front(Front::Stfs, 4, hStfsP));
  put(helper_read(1, iRead8));  // Q6: the page-table paths (hRd8P ...)
  put(helper_read(2, iRead16));
  put(helper_read(4, iRead32));
  put(helper_write(1, iWrite8));
  put(helper_write(2, iWrite16));
  put(helper_write(4, iWrite32));
  put(helper_read64());
  put(helper_write64());
  put(helper_load_single());
  put(helper_store_single());
  struct BodyInfo {  // per Body: its entries' dispatcher indices, and whether it has resume labels
    std::vector<std::pair<u32, u32>> entry_seg;
    bool resume = false;
  };
  std::unordered_map<u32, BodyInfo> bodies;
  for (std::size_t k = 0; k < funcs.size(); ++k) {
    const WFunc& w = funcs[k];
    const FunctionCfg& f = p.functions[w.cfg];
    Fn fn;
    if (w.kind == WKind::Plain || w.kind == WKind::Body || w.kind == WKind::Orig || w.kind == WKind::Fast ||
        w.kind == WKind::Slow) {
      FuncGen::Mode mode = w.kind == WKind::Plain || w.kind == WKind::Slow ? FuncGen::Mode::Plain
                           : w.kind == WKind::Fast                         ? FuncGen::Mode::Fast
                           : w.kind == WKind::Body                         ? FuncGen::Mode::Body
                                                                           : FuncGen::Mode::Orig;
      FuncGen g(ctx, w.cfg, mode, w.hook ? w.hle : -1);
      if (!g.run(fn, error)) return false;
      if (w.kind != WKind::Orig) {
        for (const auto& [a, seg] : g.resume()) {
          out->resume.push_back({a, static_cast<u32>(ctx.start_slot[w.cfg])});
          out->resume_index.push_back({a, seg});
        }
        if (w.kind != WKind::Slow) ++out->stats.functions;
      }
      if (w.kind == WKind::Body) {
        BodyInfo info;
        info.resume = !g.resume().empty();
        for (u32 e : f.entries) info.entry_seg.push_back({e, g.entry_segment(e)});
        bodies[index_of(k)] = info;
      }
    } else if (w.kind == WKind::Stub) {
      const BodyInfo& info = bodies.at(w.target);
      fn.uleb(0);
      if (w.entry == f.start && info.resume) {
        glb(fn, gResume);
        fn.mem(op::i32_load8_u, 0, 0);
        fn.if_();
        fn.get(0);
        fn.i32c(0);
        fn.call(w.target);
        fn.op(op::return_);
        fn.end();
      }
      emit_prologue(fn, w.entry, [&] { emit_leave(fn); });
      u32 seg = 0;
      for (const auto& [e, s] : info.entry_seg)
        if (e == w.entry) seg = s;
      fn.get(0);
      fn.i32c(seg);
      fn.call(w.target);
      fn.byte(op::end);
    } else {  // HleStub
      fn.uleb(0);
      emit_prologue(fn, w.entry, [&] { emit_leave(fn); });
      fn.get(0);
      fn.i32c(w.hle);
      fn.global(gTableBase);
      fn.i32c(funcs[w.target - kFirstGuest].slot);
      fn.op(op::i32_add);
      fn.call(iHleCall);
      fn.byte(op::end);
      ++out->stats.hle_stubs;
    }
    put(fn);
    if (w.slot >= 0 && w.kind != WKind::Orig && w.kind != WKind::Slow) {
      out->entries.push_back({w.entry, static_cast<u32>(w.slot)});
      ++out->stats.entries;
    }
  }
  out->stats.wasm_functions = funcs.size();
  out->stats.code_bytes = m.b.size() - code_start;
  {
    const std::size_t n = m.b.size() - code_start;
    if (static_cast<std::uint64_t>(n) >= (std::uint64_t{1} << 32)) {
      if (error) *error = "the code section is too large";
      return false;
    }
    for (int k = 0; k < 5; ++k)
      m.b[code_size_at + static_cast<std::size_t>(k)] =
          static_cast<u8>(((static_cast<std::uint64_t>(n) >> (7 * k)) & 0x7F) | (k < 4 ? 0x80 : 0));
  }

  // Safe points and idle spins, as the C++ emitter's table lists them.
  for (const auto& f : p.functions) {
    out->safe.insert(out->safe.end(), f.entries.begin(), f.entries.end());
    out->safe.insert(out->safe.end(), f.back_edges.begin(), f.back_edges.end());
    out->idle.insert(out->idle.end(), f.idle_spins.begin(), f.idle_spins.end());
  }
  std::sort(out->safe.begin(), out->safe.end());
  std::sort(out->idle.begin(), out->idle.end());

  if (opt.names) {
    // Custom section "name", subsection 1: function names (profiling only).
    Buf fnames;
    const auto& inames = wasm_import_names();
    fnames.uleb(kImports + kHelperCount + funcs.size());
    for (u32 k = 0; k < kImports; ++k) {
      fnames.uleb(k);
      fnames.name("rt." + inames[k]);
    }
    for (u32 k = 0; k < kHelperCount; ++k) {
      fnames.uleb(kImports + k);
      fnames.name(std::string("h.") + kHelperName[k]);
    }
    for (std::size_t k = 0; k < funcs.size(); ++k) {
      const WFunc& w = funcs[k];
      const char* suffix = w.kind == WKind::Body ? ".body" : w.kind == WKind::Orig ? ".orig" : w.kind == WKind::HleStub ? ".hle"
                           : w.kind == WKind::Slow ? ".resume" : "";
      std::string nm = p.functions[w.cfg].sym->name + suffix;
      if (w.kind == WKind::Stub) {
        char b[16];
        std::snprintf(b, sizeof b, ".e%08X", w.entry);
        nm += b;
      }
      fnames.uleb(index_of(k));
      fnames.name(nm);
    }
    Buf ns;
    ns.name("name");
    ns.byte(1);
    ns.uleb(fnames.b.size());
    ns.append(fnames.b);
    m.section(0, ns);
  }
  out->bytes = std::move(m.b);
  return true;
}

}  // namespace mvp::xlat
