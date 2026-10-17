#include "runtime/hw.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "common/bytes.h"
#include "ppc/mem_access.h"
#include "runtime/ax.h"
#include "vfs/source.h"

namespace mvp::rt {

namespace {

constexpr u32 kExiBase = 0xCC006800u, kExiStride = 0x14u;
constexpr u32 kDspBase = 0xCC005000u;
constexpr u32 kDiBase = 0xCC006000u;
constexpr u32 kAiBase = 0xCC006C00u;
constexpr std::uint64_t kTbHz = 40500000;  // time-base ticks per second

// DI status bits: masks (stored) and interrupt flags (write 1 to clear).
constexpr u32 kDiBrk = 0x1, kDiMasks = 0x2 | 0x8 | 0x20, kDiFlags = 0x4 | 0x10 | 0x40, kDiTcint = 0x10,
              kDiBrkint = 0x40;

// The drive's answer to INQUIRY (0x12): revision 2, device code 6, firmware
// date 2002-04-02. Fixed (R12); nothing in the game reads more than that it
// arrived.
constexpr u8 kDriveInfo[32] = {0x00, 0x02, 0x00, 0x06, 0x20, 0x02, 0x04, 0x02};

// EXI CSR bits.
constexpr u32 kExiIntW1C = 0x2 | 0x8 | 0x800;  // EXIINT, TCINT, EXTINT
constexpr u32 kExiTcint = 0x8;
// DSP control register bits.
constexpr u16 kDspRes = 0x0001, kDspHalt = 0x0004, kDspAidint = 0x0008, kDspArint = 0x0020, kDspDspint = 0x0080;
constexpr u16 kDspW1C = kDspAidint | kDspArint | kDspDspint;

}  // namespace

Hardware::Hardware(ppc::Cpu* cpu, u8* mem1, u32 mem1_size, const HardwareConfig& cfg)
    : cpu_(cpu), mem1_(mem1), mem1_size_(mem1_size), cfg_(cfg), aram_(0x01000000, 0) {
  rtc_ = cfg.rtc_seconds;
  gx_.set_memory(mem1, mem1_size);
  // A valid SRAM image: OSSram (0x14 bytes) then OSSramEx. Checksums cover the
  // 16-bit words from counterBias (offset 0x0C) to the end of OSSram.
  sram_.fill(0);
  sram_[0x11] = 0;  // ntd: NTSC
  sram_[0x12] = cfg.sram_language;
  sram_[0x13] = cfg.sram_flags;
  u16 sum = 0, inv = 0;
  for (u32 k = 0x0C; k < 0x14; k += 2) {
    u16 w = static_cast<u16>(be16(&sram_[k]));
    sum = static_cast<u16>(sum + w);
    inv = static_cast<u16>(inv + static_cast<u16>(~w));
  }
  put_be16(&sram_[0], sum);
  put_be16(&sram_[2], inv);
}

const char* Hardware::block_name(u32 a) {
  if (a >= 0xCC000000u && a < 0xCC001000u) return "CP";
  if (a >= 0xCC001000u && a < 0xCC002000u) return "PE";
  if (a >= 0xCC002000u && a < 0xCC003000u) return "VI";
  if (a >= 0xCC003000u && a < 0xCC004000u) return "PI";
  if (a >= 0xCC004000u && a < 0xCC005000u) return "MI";
  if (a >= 0xCC005000u && a < 0xCC006000u) return "DSP";
  if (a >= 0xCC006000u && a < 0xCC006400u) return "DI";
  if (a >= 0xCC006400u && a < 0xCC006800u) return "SI";
  if (a >= 0xCC006800u && a < 0xCC006C00u) return "EXI";
  if (a >= 0xCC006C00u && a < 0xCC007000u) return "AI";
  if (a >= 0xCC008000u && a < 0xCC009000u) return "WGP";
  return nullptr;
}

u32 Hardware::reg(u32 addr) const {
  const u32 off = (addr & ~3u) - kRegBase;
  return off < kRegBytes ? regs_[off >> 2] : 0;
}

void Hardware::set_reg(u32 addr, u32 value, int size) {
  u32 a = addr & ~3u;
  u32 old = reg(a);
  u32 sh = (4 - (addr & 3) - static_cast<u32>(size)) * 8;
  u32 mask = size == 4 ? 0xFFFFFFFFu : ((1u << (8 * size)) - 1) << sh;
  if (a - kRegBase < kRegBytes) regs_[(a - kRegBase) >> 2] = (old & ~mask) | ((value << sh) & mask);
}

// ---- EXI ----

int Hardware::exi_device(int ch) const {
  u32 cs = (exi_[ch].csr >> 7) & 7;
  if (cs & 1) return 0;
  if (cs & 2) return 1;
  if (cs & 4) return 2;
  return -1;
}

u8 Hardware::ipl_read_byte(ExiChannel& e) {
  u32 a = e.cmd & 0x7FFFFFFFu;
  u8 b = 0;
  if (a >= 0x20000100u && a < 0x20001100u) {  // SRAM: offset = (a - 0x100) >> 6, then sequential
    u32 off = ((a - 0x20000100u) >> 6) + e.offset;
    b = off < sram_.size() ? sram_[off] : 0;
  } else if (a >= 0x20000000u && a < 0x20000100u) {  // RTC counter, big-endian
    u32 off = e.offset;
    b = off < 4 ? static_cast<u8>(rtc_ >> (24 - 8 * off)) : 0;
  }  // IPL ROM (fonts) and the UART are not provided: zeros.
  ++e.offset;
  return b;
}

void Hardware::ipl_write_byte(ExiChannel& e, u8 b) {
  if (!e.have_cmd) {
    e.cmd = (e.cmd << 8) | b;
    if (++e.cmd_bytes == 4) {
      e.have_cmd = true;
      e.offset = 0;
    }
    return;
  }
  u32 a = e.cmd & 0x7FFFFFFFu;
  if (!(e.cmd & 0x80000000u)) return;  // data written during a read command
  if (a >= 0x20000100u && a < 0x20001100u) {
    u32 off = ((a - 0x20000100u) >> 6) + e.offset;
    if (off < sram_.size()) sram_[off] = b;
  } else if (a >= 0x20000000u && a < 0x20000100u) {
    if (e.offset < 4) rtc_ = (rtc_ & ~(0xFFu << (24 - 8 * e.offset))) | (u32(b) << (24 - 8 * e.offset));
  }
  ++e.offset;
}

void Hardware::exi_transfer(int ch) {
  ExiChannel& e = exi_[ch];
  int dev = exi_device(ch);
  bool ipl = ch == 0 && dev == 1;
  u32 rw = (e.cr >> 2) & 3;  // 0 read, 1 write, 2 read-write
  if (e.cr & 2) {  // DMA
    u32 pa = e.mar & 0x01FFFFE0u;
    if (rw == 0 && pa < mem1_size_) {
      ppc::gp_check(pa, std::min(e.len, mem1_size_ - pa));  // P2: memory the render thread still reads
      if (gfx_) gfx_->gp_write_through(pa, std::min(e.len, mem1_size_ - pa));  // Q1: memory written without the cache
    }
    for (u32 k = 0; k < e.len && pa + k < mem1_size_; ++k) {
      if (rw == 0) mem1_[pa + k] = ipl ? ipl_read_byte(e) : 0;
      else if (ipl) ipl_write_byte(e, mem1_[pa + k]);
    }
  } else {  // immediate: TLEN+1 bytes, most significant first
    u32 n = ((e.cr >> 4) & 3) + 1;
    if (rw == 1 || rw == 2) {
      for (u32 k = 0; k < n; ++k)
        if (ipl) ipl_write_byte(e, static_cast<u8>(e.data >> (24 - 8 * k)));
    }
    if (rw == 0 || rw == 2) {
      u32 d = 0;
      for (u32 k = 0; k < n; ++k) d |= u32(ipl ? ipl_read_byte(e) : 0) << (24 - 8 * k);
      e.data = d;
    }
  }
  e.cr &= ~1u;      // TSTART clears: the transfer is complete
  e.csr |= kExiTcint;
}

void Hardware::exi_write(int ch, u32 r, u32 v) {
  ExiChannel& e = exi_[ch];
  switch (r) {
    case 0x00: {
      u32 old_dev = (e.csr >> 7) & 7;
      e.csr = (e.csr & kExiIntW1C & ~v) | (v & ~kExiIntW1C & ~0x1000u);  // EXT is read-only (no card)
      if (((e.csr >> 7) & 7) != old_dev) {  // chip select changed: a new command starts
        e.have_cmd = false;
        e.cmd = 0;
        e.cmd_bytes = 0;
        e.offset = 0;
      }
      break;
    }
    case 0x04: e.mar = v; break;
    case 0x08: e.len = v; break;
    case 0x0C:
      e.cr = v;
      if (v & 1) exi_transfer(ch);
      break;
    case 0x10: e.data = v; break;
  }
}

// ---- GX FIFO ----

u16 Hardware::cp16(u32 off) const {
  u32 w = reg(0xCC000000u + (off & ~3u));
  return static_cast<u16>((off & 2) ? w : w >> 16);
}

void Hardware::set_cp32(u32 lo_off, u32 v) {
  set_reg(0xCC000000u + lo_off, v & 0xFFFFu, 2);
  set_reg(0xCC000000u + lo_off + 2, v >> 16, 2);
}

// CP FIFO registers: base 0x20, end 0x24, high and low watermarks 0x28 and 0x2C,
// read-write distance 0x30, write pointer 0x34, read pointer 0x38, breakpoint
// 0x3C. Control (0x02): read enable 0x1, breakpoint enable 0x2, overflow and
// underflow interrupt enables 0x4 and 0x8, link to the CPU FIFO 0x10,
// breakpoint interrupt enable 0x20.
u32 Hardware::gp_distance() const {
  u32 base = cp32(0x20) & 0x03FFFFE0u, end = cp32(0x24) & 0x03FFFFFFu;
  u32 wp = cp32(0x34) & 0x03FFFFE0u, rp = cp32(0x38) & 0x03FFFFE0u;
  u32 size = (end + 32 - base) & ~31u;
  return wp >= rp ? wp - rp : wp + size - rp;
}

bool Hardware::gp_at_breakpoint() const {
  return (cp16(0x02) & 0x2) && (cp32(0x38) & 0x03FFFFE0u) == (cp32(0x3C) & 0x03FFFFE0u);
}

u32 Hardware::cp_status() const {
  u32 d = gp_distance();
  u32 sr = 0;
  if (d > (cp32(0x28) & 0x03FFFFE0u)) sr |= 0x1;  // above the high watermark
  if (d < (cp32(0x2C) & 0x03FFFFE0u)) sr |= 0x2;  // below the low watermark
  if (d == 0) sr |= 0x4 | 0x8;                   // read and command idle
  if (gp_at_breakpoint()) sr |= 0x10;
  return sr;
}

void Hardware::wgp_write(u32 value, int size) {
  u8* const gather = wgp_->buf;
  u32& gathered = wgp_->n;
  if (gathered + static_cast<u32>(size) <= 32) {  // P1: the whole value fits (always, for aligned stores)
    u8* g = gather + gathered;
    for (int k = 0; k < size; ++k) g[k] = static_cast<u8>(value >> (8 * (size - 1 - k)));
    gathered += static_cast<u32>(size);
    if (gathered == 32) wgp_burst();
    return;
  }
  for (int k = 0; k < size; ++k) {
    gather[gathered++] = static_cast<u8>(value >> (8 * (size - 1 - k)));
    if (gathered == 32) wgp_burst();
  }
}

void Hardware::wgp_burst() {
  wgp_->n = 0;
  const u8* const gather = wgp_->buf;
  // The FIFO end may be 0x04000000, the top of the 64 MiB physical space: that is
  // how GXRedirectWriteGatherPipe turns the pipe into a plain memory writer (base
  // 0, end 0x04000000, write pointer at the destination, the GP unlinked). Bit 26
  // counts there; masking it off once sent that data to the GP as commands.
  u32 base = reg(0xCC00300Cu) & 0x03FFFFE0u, end = reg(0xCC003010u) & 0x07FFFFFFu;
  u32 wp = reg(0xCC003014u);
  if (end == 0) {  // no CPU FIFO set up yet: straight to the GP
    gp_consume(gather, 32);
    return;
  }
  u32 pa = wp & 0x03FFFFE0u, wrap = wp & 0x04000000u;
  if (pa + 32 > mem1_size_) throw StopRun{"GX FIFO burst outside MEM1"};
  ppc::gp_check(pa, 32);  // P2: memory the render thread still reads (a redirected pipe writes anywhere)
  std::memcpy(mem1_ + pa, gather, 32);
  if (gfx_) gfx_->gp_write_through(pa, 32);  // Q1: the pipe writes memory without the cache
  ++gp_bursts_;
  pa += 32;
  if (pa > end) {
    pa = base;
    wrap ^= 0x04000000u;
  }
  set_reg(0xCC003014u, pa | wrap, 4);
  if (cp16(0x02) & 0x10) set_cp32(0x34, pa);  // linked: the GP's write pointer follows
  gp_run();
}

void Hardware::gp_run() {
  if (!(cp16(0x02) & 0x1)) return;  // reads disabled
  u32 base = cp32(0x20) & 0x03FFFFE0u, end = cp32(0x24) & 0x03FFFFFFu;
  u32 wp = cp32(0x34) & 0x03FFFFE0u, rp = cp32(0x38) & 0x03FFFFE0u;
  bool moved = false;
  while (rp != wp && !gp_at_breakpoint()) {
    if (rp + 32 > mem1_size_) throw StopRun{"GX FIFO read outside MEM1"};
    gp_consume(mem1_ + rp, 32);
    rp += 32;
    if (rp > end) rp = base;
    set_cp32(0x38, rp);
    moved = true;
  }
  // Q6: the GP's progress raises nothing but the CP interrupt (a watermark or
  // the breakpoint, each with its enable): a safe point is due only when that
  // cause is up. (Any other cause kicks where it is raised; a safe point with
  // nothing to deliver changes nothing, so the guest sees the same.)
  const bool at_bp = gp_at_breakpoint();
  if ((moved || at_bp) && cp_cause()) kick();
  if (gfx_ && at_bp) gfx_->gp_sync();  // P2: the guest can see where the GP stopped
}

void Hardware::gp_consume(const u8* bytes, std::size_t n) {
  if (fifo_.size() < kFifoKeep) fifo_.insert(fifo_.end(), bytes, bytes + n);
  fifo_bytes_ += n;
  GxFifo::Effects fx = gx_.push(bytes, n);
  if (fx.token_set) pe_token_ = fx.token;
  if (fx.token_int) pe_ctrl_ |= 0x4;
  if (fx.draw_done) {
    pe_ctrl_ |= 0x8;
    ++draw_dones_;
  }
  if (fx.token_int || fx.draw_done) kick();
  // P2: from here the guest can know that the GP consumed every command so far.
  if (gfx_ && (fx.draw_done || fx.token_set)) gfx_->gp_sync();
}

// ---- AI ----

u32 Hardware::ai_count(std::uint64_t tb) const {
  if (!(ai_cr_ & 0x1) || tb < ai_t0_) return ai_count_base_;
  std::uint64_t rate = (ai_cr_ & 0x2) ? 48000 : 32000;
  return ai_count_base_ + static_cast<u32>((tb - ai_t0_) * rate / kTbHz);
}

std::uint64_t Hardware::ai_next_change_tb() const {
  if (!(ai_cr_ & 0x1)) return ~0ull;
  std::uint64_t rate = (ai_cr_ & 0x2) ? 48000 : 32000;
  std::uint64_t t = now_tb_ > ai_t0_ ? now_tb_ : ai_t0_;
  std::uint64_t n = (t - ai_t0_) * rate / kTbHz + 1;  // the next count
  return ai_t0_ + (n * kTbHz + rate - 1) / rate;
}

void Hardware::ai_write(u32 r, u32 v) {
  if (r == 0x8) {  // the counter itself
    ai_count_base_ = v;
    ai_t0_ = now();
    return;
  }
  // Control: the counter keeps its value across changes; its phase restarts.
  std::uint64_t t = now();
  ai_count_base_ = (v & 0x20) ? 0 : ai_count(t);
  ai_t0_ = t;
  ai_cr_ = v & ~0x20u & ~0x8u;  // the reset bit reads back clear; no AI interrupt is modeled
}

// ---- DI (the DVD drive) ----

u32 Hardware::di_read(u32 r) const {
  switch (r) {
    case 0x00: return di_sr_;
    case 0x04: return di_cvr_;  // bit 0 clear: the cover is closed
    case 0x08: case 0x0C: case 0x10: return di_cmd_[(r - 0x08) / 4];
    case 0x14: return di_mar_;
    case 0x18: return di_len_;
    case 0x1C: return di_cr_;
    case 0x20: return di_imm_;
    default: return di_cfg_;
  }
}

void Hardware::di_write(u32 r, u32 v) {
  switch (r) {
    case 0x00:
      di_sr_ = (di_sr_ & kDiFlags & ~v) | (v & kDiMasks);
      if (v & kDiBrk) {  // break: abandon the command in progress
        di_done_tb_ = ~0ull;
        di_cr_ &= ~1u;
        di_sr_ |= kDiBrkint;
      }
      break;
    case 0x04: di_cvr_ = (di_cvr_ & 0x4 & ~v) | (v & 0x2); break;
    case 0x08: case 0x0C: case 0x10: di_cmd_[(r - 0x08) / 4] = v; break;
    case 0x14: di_mar_ = v & 0x03FFFFE0u; break;
    case 0x18: di_len_ = v & 0x03FFFFE0u; break;
    case 0x1C: {
      di_cr_ = v & 7;
      if (!(v & 1)) break;
      u32 cmd = di_cmd_[0] >> 24;
      switch (cmd) {
        case 0x12: case 0xA8: case 0xAB: case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: break;
        default: {
          char b[96];
          std::snprintf(b, sizeof b, "DI command 0x%08X is not modeled (pc 0x%08X)", di_cmd_[0], cpu_->pc);
          throw StopRun{b};
        }
      }
      if ((v & 4) && (v & 2)) throw StopRun{"DI DMA write to the disc"};
      ++di_commands_;
      std::uint64_t bytes = (v & 2) ? di_len_ : 0;
      di_done_tb_ = now() + cfg_.di_latency_tb + bytes * cfg_.di_tb_per_kib / 1024;
      break;
    }
    case 0x20: di_imm_ = v; break;
    default: di_cfg_ = v; break;
  }
}

void Hardware::di_complete() {
  di_done_tb_ = ~0ull;
  u32 cmd = di_cmd_[0] >> 24;
  if (di_cr_ & 2) {  // DMA from the drive
    u32 pa = di_mar_ & 0x01FFFFE0u, n = di_len_;
    if (pa + n > mem1_size_) throw StopRun{"DI DMA outside MEM1"};
    ppc::gp_check(pa, n);  // P2: memory the render thread still reads
    if (gfx_) gfx_->gp_write_through(pa, n);  // Q1: DMA writes memory without the cache
    if (cmd == 0xA8) {
      std::uint64_t off = std::uint64_t(di_cmd_[1]) << 2;
      if (!disc_ || !disc_->read(off, mem1_ + pa, n)) {
        char b[96];
        std::snprintf(b, sizeof b, "DI read of 0x%X bytes at disc offset 0x%llX failed", n,
                      static_cast<unsigned long long>(off));
        throw StopRun{b};
      }
    } else if (cmd == 0x12) {
      for (u32 k = 0; k < n; ++k) mem1_[pa + k] = k < sizeof kDriveInfo ? kDriveInfo[k] : 0;
    } else {
      std::memset(mem1_ + pa, 0, n);
    }
    di_bytes_ += n;
    di_mar_ += n;
    di_len_ = 0;
  } else if (cmd == 0xE0 || cmd == 0xE2) {
    di_imm_ = 0;  // no error; no audio stream playing
  }
  di_cr_ &= ~1u;
  di_sr_ |= kDiTcint;
}

// ---- DSP and ARAM ----

void Hardware::dsp_aram_dma() {
  u32 mm = ar_mm_ & 0x01FFFFE0u, ar = ar_ar_ & 0x00FFFFE0u, n = ar_cnt_ & 0x7FFFFFE0u;
  bool to_mem = (ar_cnt_ & 0x80000000u) != 0;
  if (to_mem) ppc::gp_check(mm, n);  // P2: memory the render thread still reads
  if (to_mem && gfx_) gfx_->gp_write_through(mm, n);  // Q1: DMA writes memory without the cache
  for (u32 k = 0; k < n; ++k) {
    if (mm + k >= mem1_size_ || ar + k >= aram_.size()) break;
    if (to_mem) mem1_[mm + k] = aram_[ar + k];
    else aram_[ar + k] = mem1_[mm + k];
  }
  ar_cnt_ &= 0x80000000u;
  dsp_cr_ |= kDspArint;  // DMA complete
}

u16 Hardware::dsp_read16(u32 off) {
  switch (off) {
    case 0x00: return static_cast<u16>(dsp_mail_to_dsp_ >> 16);
    case 0x02: return static_cast<u16>(dsp_mail_to_dsp_);
    case 0x04: return static_cast<u16>(dsp_mail_to_cpu_ >> 16);
    case 0x06: {  // reading the low half takes the mail; the next one may follow
      u16 lo = static_cast<u16>(dsp_mail_to_cpu_);
      dsp_mail_to_cpu_ &= 0x7FFFFFFFu;
      dsp_deliver();
      return lo;
    }
    case 0x0A: return dsp_cr_;
    case 0x3A: {  // AI DMA: 32-byte blocks left in the current block
      if (!aid_on_ || aid_end_tb_ == ~0ull) return 0;
      std::uint64_t t = now(), span = aid_end_tb_ - aid_start_tb_;
      if (t >= aid_end_tb_ || span == 0) return 0;
      return static_cast<u16>(aid_blocks_ * (aid_end_tb_ - t) / span);
    }
    case 0x16: {  // ARAM mode: bit 0 reports the ARAM controller ready
      u32 w = reg(kDspBase + 0x14u);
      return static_cast<u16>(w | 1u);
    }
    case 0x20: return static_cast<u16>(ar_mm_ >> 16);
    case 0x22: return static_cast<u16>(ar_mm_);
    case 0x24: return static_cast<u16>(ar_ar_ >> 16);
    case 0x26: return static_cast<u16>(ar_ar_);
    case 0x28: return static_cast<u16>(ar_cnt_ >> 16);
    case 0x2A: return static_cast<u16>(ar_cnt_);
    default: {
      u32 w = reg(kDspBase + (off & ~3u));
      return static_cast<u16>((off & 2) ? w : w >> 16);
    }
  }
}

void Hardware::dsp_write16(u32 off, u16 v) {
  switch (off) {
    case 0x00: dsp_mail_to_dsp_ = (dsp_mail_to_dsp_ & 0xFFFFu) | (u32(v) << 16); return;
    case 0x02: {  // the low half sends the mail (the valid bit, 31, is set by
      // the hardware); the DSP takes it at once, so the valid bit reads clear
      u32 mail = (dsp_mail_to_dsp_ & 0xFFFF0000u) | v;
      dsp_mail_to_dsp_ = mail & 0x7FFFFFFFu;
      dsp_receive(mail);
      return;
    }
    case 0x0A: {
      u16 old = dsp_cr_;
      dsp_cr_ = static_cast<u16>((dsp_cr_ & kDspW1C & ~v) | (v & ~kDspW1C & ~kDspRes));
      if (v & kDspRes) {  // reset completes at once; the DSP is halted and rebooting
        dsp_booting_ = true;
        dsp_mail_to_cpu_ = 0;
        dsp_out_.clear();
        dsp_state_ = DspState::Off;
      }
      if ((old & kDspHalt) && !(dsp_cr_ & kDspHalt) && dsp_booting_) {
        dsp_booting_ = false;
        if (dsp_boots_++ == 0) {
          // The IPL's DSP init microcode (uploaded by __OSInitAudioSystem)
          // answers with this mail when it starts.
          dsp_mail_to_cpu_ = 0x80544348u;
          dsp_state_ = DspState::IplInit;
        } else {
          // The boot ROM: ready to load a task.
          dsp_mail_to_cpu_ = 0x8071FEEDu;
          dsp_state_ = DspState::RomReady;
          dsp_load_n_ = 0;
        }
      }
      return;
    }
    case 0x36: {  // AI DMA control: enable (bit 15) and length in 32-byte blocks
      set_reg(kDspBase + off, v, 2);
      bool on = (v & 0x8000u) != 0;
      if (on && !aid_on_) {
        aid_on_ = true;
        aid_start_block(now());
      } else if (!on) {
        aid_on_ = false;
        aid_end_tb_ = ~0ull;
      }
      return;
    }
    case 0x20: ar_mm_ = (ar_mm_ & 0xFFFFu) | (u32(v) << 16); return;
    case 0x22: ar_mm_ = (ar_mm_ & 0xFFFF0000u) | v; return;
    case 0x24: ar_ar_ = (ar_ar_ & 0xFFFFu) | (u32(v) << 16); return;
    case 0x26: ar_ar_ = (ar_ar_ & 0xFFFF0000u) | v; return;
    case 0x28: ar_cnt_ = (ar_cnt_ & 0xFFFFu) | (u32(v) << 16); return;
    case 0x2A:
      ar_cnt_ = (ar_cnt_ & 0xFFFF0000u) | v;
      dsp_aram_dma();
      return;
    default: set_reg(kDspBase + off, v, 2); return;
  }
}

// ---- DSP mailbox HLE and the AI DMA ----

namespace {
constexpr std::uint64_t kDspTaskStartTb = 4050;  // 0.1 ms: the ucode loads and initializes
constexpr std::uint64_t kAxFrameTb = 20250;      // 0.5 ms: the AX ucode mixes one 5 ms frame
}  // namespace

void Hardware::dsp_post(std::uint64_t delay_tb, u32 mail, bool irq) {
  DspMail m{now() + delay_tb, mail, irq};
  auto at = std::upper_bound(dsp_out_.begin(), dsp_out_.end(), m.tb,
                             [](std::uint64_t t, const DspMail& x) { return t < x.tb; });
  dsp_out_.insert(at, m);
  kick();
}

void Hardware::dsp_deliver() {
  if (dsp_out_.empty() || (dsp_mail_to_cpu_ & 0x80000000u) || dsp_out_.front().tb > now()) return;
  DspMail m = dsp_out_.front();
  dsp_out_.erase(dsp_out_.begin());
  dsp_mail_to_cpu_ = m.mail | 0x80000000u;
  if (m.irq) {
    dsp_cr_ |= kDspDspint;
    kick();
  }
}

void Hardware::dsp_receive(u32 mail) {
  char b[112];
  switch (dsp_state_) {
    case DspState::RomReady:
    case DspState::Loading:
      // __DSP_boot_task: ten mails (0x80F3A001, IRAM source, 0x80F3C002, IRAM
      // address, 0x80F3A002, length, 0x80F3B002, 0, 0x80F3D001, entry point).
      if (dsp_load_n_ == 0 && mail != 0x80F3A001u) break;
      dsp_state_ = DspState::Loading;
      if (++dsp_load_n_ == 10) {
        dsp_state_ = DspState::Running;
        dsp_post(kDspTaskStartTb, 0xDCD10000u, true);  // the task is initialized
      }
      return;
    case DspState::Running:
      if (dsp_cmdlist_next_) {  // the command list address: run a frame, then yield
        dsp_cmdlist_next_ = false;
        ++dsp_frames_;
        if (cfg_.ax_voices) {  // the voices advance (R8, runtime/ax.h)
          ax::FrameStats st =
              ax::process_frame(mem1_, mem1_size_, {aram_.data(), static_cast<u32>(aram_.size())}, mail, ax_obs_);
          if (!st.error.empty()) {
            char at[32];
            std::snprintf(at, sizeof at, " (pc 0x%08X)", cpu_ ? cpu_->pc : 0u);
            throw StopRun{st.error + at};
          }
          ax_.voice_frames += st.running;
          ax_.samples += st.samples;
          ax_.ended += st.ended;
          ax_.looped += st.looped;
          ax_.updates += st.updates;
          ax_.max_running = std::max<std::uint64_t>(ax_.max_running, st.running);
        }
        dsp_post(kAxFrameTb, 0xDCD10002u, true);
        return;
      }
      if ((mail & 0xFFFF0000u) == 0xBABE0000u) {  // command list length follows... its address
        dsp_cmdlist_next_ = true;
        return;
      }
      if (mail == 0xCDD10003u) return;  // resume the same task: wait for the next list
      break;
    default:
      break;
  }
  std::snprintf(b, sizeof b, "DSP mail 0x%08X is not modeled (state %d, pc 0x%08X)", mail,
                static_cast<int>(dsp_state_), cpu_->pc);
  throw StopRun{b};
}

std::uint64_t Hardware::aid_block_tb(u32 blocks) const {
  // 32-byte blocks of 16-bit stereo samples (4 bytes): 8 samples per block.
  std::uint64_t rate = (ai_cr_ & 0x40) ? 32000 : 48000;
  return std::uint64_t(blocks) * 8 * kTbHz / rate;
}

void Hardware::aid_start_block(std::uint64_t tb) {
  // The DMA latches the programmed address and length and raises AIDINT, so
  // the CPU can program the next block while this one plays.
  aid_blocks_ = (dsp_read16(0x36) & 0x7FFFu);
  aid_start_tb_ = tb;
  std::uint64_t len = aid_block_tb(aid_blocks_);
  aid_end_tb_ = aid_on_ && len ? tb + len : ~0ull;
  dsp_cr_ |= kDspAidint;
  ++aid_interrupts_;
  kick();
}

// ---- VI and the interrupt cause ----

void Hardware::vi_schedule_frame() {
  vi_events_.clear();
  for (int n = 0; n < 4; ++n) {
    u32 di = reg(0xCC002030u + 4u * static_cast<u32>(n));
    if (!(di & 0x10000000u)) continue;     // enable
    u32 vct = (di >> 16) & 0x3FF;          // line, 1-based
    if (vct == 0 || vct > kLines) continue;
    vi_events_.emplace_back(frame_start_tb_ + (vct - 1) * line_tb(), n);
  }
  std::sort(vi_events_.begin(), vi_events_.end());
}

std::uint64_t Hardware::now() const {
  if (cpu_ && cpu_->hooks) return std::max(now_tb_, cpu_->hooks->time_base(*cpu_));
  return now_tb_;
}

void Hardware::advance(std::uint64_t tb) {
  if (tb < now_tb_) return;
  now_tb_ = tb;
  if (di_done_tb_ <= tb) di_complete();
  while (aid_end_tb_ <= tb) aid_start_block(aid_end_tb_);
  dsp_deliver();
  for (;;) {
    while (!vi_events_.empty() && vi_events_.front().first <= tb) {
      int n = vi_events_.front().second;
      vi_events_.erase(vi_events_.begin());
      u32 a = 0xCC002030u + 4u * static_cast<u32>(n);
      u32 di = reg(a);
      if (di & 0x10000000u) {  // still enabled: raise its interrupt status
        regs_[(a - kRegBase) >> 2] = di | 0x80000000u;
        if (n < 2) {
          ++vi_fields_;
          if (gfx_) {  // the top field's frame buffer (TFBL: address, or address >> 5 with POFF)
            u32 tfbl = reg(0xCC00201Cu);
            u32 xfb = (tfbl & 0x10000000u) ? (tfbl & 0xFFFFFFu) << 5 : tfbl & 0xFFFFFFu;
            gfx_->vi_field(vi_fields_, xfb);
          }
        }
      }
    }
    if (frame_start_tb_ + kFrameTb > tb) break;
    frame_start_tb_ += kFrameTb;
    vi_schedule_frame();
  }
}

std::uint64_t Hardware::next_event_tb() const {
  std::uint64_t next = frame_start_tb_ + kFrameTb;
  if (!vi_events_.empty() && vi_events_.front().first < next) next = vi_events_.front().first;
  next = std::min({next, di_done_tb_, ai_next_change_tb(), aid_end_tb_});
  if (!dsp_out_.empty() && !(dsp_mail_to_cpu_ & 0x80000000u)) next = std::min(next, dsp_out_.front().tb);
  return next;
}

void Hardware::kick() {
  if (cpu_ && cpu_->next_event > cpu_->ticks) cpu_->next_event = cpu_->ticks;
}

u32 Hardware::vi_current_line() const {
  return static_cast<u32>(((now() - frame_start_tb_) % kFrameTb) / line_tb()) + 1;
}

bool Hardware::cp_cause() const {
  const u32 cr = cp16(0x02);
  if (!(cr & (0x4 | 0x8 | 0x20))) return false;  // Q4: the CP status only when it can raise one
  const u32 sr = cp_status();
  return ((sr & 0x1) && (cr & 0x4)) || ((sr & 0x2) && (cr & 0x8)) || ((sr & 0x10) && (cr & 0x20));
}

u32 Hardware::pi_cause() const {
  u32 cause = 0x10000;  // RSWST: the reset switch is not pressed
  for (int n = 0; n < 4; ++n) {
    u32 di = reg(0xCC002030u + 4u * static_cast<u32>(n));
    if ((di & 0x80000000u) && (di & 0x10000000u)) cause |= 0x100;  // VI
  }
  for (const auto& e : exi_)
    if (((e.csr & 0x2) && (e.csr & 0x1)) || ((e.csr & 0x8) && (e.csr & 0x4)) || ((e.csr & 0x800) && (e.csr & 0x400)))
      cause |= 0x10;  // EXI
  if (((di_sr_ & 0x4) && (di_sr_ & 0x2)) || ((di_sr_ & 0x10) && (di_sr_ & 0x8)) || ((di_sr_ & 0x40) && (di_sr_ & 0x20)) ||
      ((di_cvr_ & 0x4) && (di_cvr_ & 0x2)))
    cause |= 0x4;  // DI
  if (cp_cause()) cause |= 0x800;  // CP
  if ((pe_ctrl_ & 0x4) && (pe_ctrl_ & 0x1)) cause |= 0x200;  // PE token
  if ((pe_ctrl_ & 0x8) && (pe_ctrl_ & 0x2)) cause |= 0x400;  // PE finish
  if (((dsp_cr_ & 0x08) && (dsp_cr_ & 0x10)) || ((dsp_cr_ & 0x20) && (dsp_cr_ & 0x40)) ||
      ((dsp_cr_ & 0x80) && (dsp_cr_ & 0x100)))
    cause |= 0x40;  // DSP
  return cause;
}

// ---- dispatch ----

u32 Hardware::read(u32 addr, int size) {
  if (!block_name(addr)) {
    char b[96];
    std::snprintf(b, sizeof b, "read of %d bytes at 0x%08X, outside the hardware blocks", size, addr);
    throw StopRun{b};
  }
  ++reads_[addr - kRegBase];
  if (addr == wgp_->last_read) {
    if (++same_reads_ >= poll_limit) {
      char b[112];
      std::snprintf(b, sizeof b, "%s register 0x%08X polled %llu times in a row (from 0x%08X)", block_name(addr), addr,
                    static_cast<unsigned long long>(same_reads_), cpu_->pc);
      throw StopRun{b};
    }
  } else {
    wgp_->last_read = addr;
    same_reads_ = 1;
  }
  u32 v;
  if (addr >= kDspBase && addr < kDspBase + 0x1000u) {
    u32 off = addr - kDspBase;
    if (size == 4) v = (u32(dsp_read16(off)) << 16) | dsp_read16(off + 2);
    else if (size == 2) v = dsp_read16(off);
    else v = static_cast<u8>(dsp_read16(off & ~1u) >> ((off & 1) ? 0 : 8));
  } else {
    u32 word;
    if (addr >= kExiBase && addr < kExiBase + 3 * kExiStride) {
      int ch = static_cast<int>((addr - kExiBase) / kExiStride);
      u32 r = (addr - kExiBase) % kExiStride & ~3u;
      const ExiChannel& e = exi_[ch];
      word = r == 0 ? e.csr : r == 4 ? e.mar : r == 8 ? e.len : r == 12 ? e.cr : e.data;
    } else if (addr >= kDiBase && addr < kDiBase + 0x28u) {
      word = di_read((addr - kDiBase) & ~3u);
    } else if ((addr & ~3u) == kAiBase + 0x0u) {
      word = ai_cr_;
    } else if ((addr & ~3u) == kAiBase + 0x8u) {
      word = ai_count(now());
    } else if ((addr & ~3u) == 0xCC003000u) {
      word = pi_cause();
      if (gfx_ && (word & 0x800)) gfx_->gp_sync();  // P2: a CP interrupt tells where the GP is
    } else if ((addr & ~3u) == 0xCC000000u) {
      word = cp_status() << 16 | cp16(0x02);
      if (gfx_ && (size == 4 || (addr & 3) < 2)) gfx_->gp_sync();  // P2: the CP status (GP idle)
    } else if ((addr & ~3u) == 0xCC000030u) {
      u32 d = gp_distance();
      word = (d & 0xFFFFu) << 16 | d >> 16;
      if (gfx_) gfx_->gp_sync();  // P2: the read-write distance
    } else if ((addr & ~3u) == 0xCC000038u) {
      word = reg(addr);
      if (gfx_) gfx_->gp_sync();  // P2: the GP's read pointer
    } else if ((addr & ~3u) == 0xCC001008u) {
      word = (reg(0xCC001008u) & 0xFFFF0000u) | pe_ctrl_;
    } else if ((addr & ~3u) == 0xCC00100Cu) {
      word = (reg(0xCC00100Cu) & 0xFFFF0000u) | pe_token_;
    } else if ((addr & ~3u) == 0xCC00202Cu) {  // VCT (hi) / HCT (lo): the beam position
      word = (vi_current_line() << 16) | 1u;
    } else {
      word = reg(addr);
    }
    u32 sh = (4 - (addr & 3) - static_cast<u32>(size)) * 8;
    v = size == 4 ? word : (word >> sh) & ((1u << (8 * size)) - 1);
  }
  if (log_.size() < 100000) log_.push_back({cpu_->pc, addr, v, size, false});
  return v;
}

void Hardware::write(u32 addr, u32 value, int size) {
  const bool wgp = addr - 0xCC008000u < 0x1000u;  // the common case, first
  if (!wgp && !block_name(addr)) {
    char b[96];
    std::snprintf(b, sizeof b, "write of %d bytes at 0x%08X, outside the hardware blocks", size, addr);
    throw StopRun{b};
  }
  ++writes_[addr - kRegBase];
  wgp_->last_read = 0;
  if (log_.size() < 100000) log_.push_back({cpu_->pc, addr, value, size, true});
  if (wgp) {  // write-gather pipe: the GX FIFO
    wgp_->fast = log_.size() >= 100000 ? 1u : 0u;  // P2: translated code may append once nothing is logged
    wgp_write(value, size);
    return;
  }
  // Any other register write may raise, clear, mask or unmask an interrupt
  // cause (DMA completions are immediate): check at the next safe point.
  kick();
  if (addr >= 0xCC000000u && addr < 0xCC000040u) {  // CP
    set_reg(addr, value, size);
    gp_run();  // a pointer, the breakpoint or the read enable may have changed
    return;
  }
  if ((addr & ~3u) == 0xCC001008u && (size == 4 || (size == 2 && addr == 0xCC00100Au))) {
    // PE interrupt control (the low half of the word).
    u16 v = static_cast<u16>(value);
    pe_ctrl_ = static_cast<u16>((v & 0x3) | (pe_ctrl_ & 0xC & ~v));
    if (size == 4) set_reg(0xCC001008u, value & 0xFFFF0000u, 4);
    return;
  }
  if (addr >= kDspBase && addr < kDspBase + 0x1000u) {
    u32 off = addr - kDspBase;
    if (size == 4) {
      dsp_write16(off, static_cast<u16>(value >> 16));
      dsp_write16(off + 2, static_cast<u16>(value));
    } else if (size == 2) {
      dsp_write16(off, static_cast<u16>(value));
    } else {
      set_reg(addr, value, size);
    }
    return;
  }
  if (addr >= kExiBase && addr < kExiBase + 3 * kExiStride && size == 4 && !(addr & 3)) {
    int ch = static_cast<int>((addr - kExiBase) / kExiStride);
    exi_write(ch, (addr - kExiBase) % kExiStride, value);
    return;
  }
  if (addr >= kDiBase && addr < kDiBase + 0x28u && size == 4 && !(addr & 3)) {
    di_write(addr - kDiBase, value);
    return;
  }
  if (((addr & ~3u) == kAiBase || (addr & ~3u) == kAiBase + 0x8u) && size == 4) {
    ai_write(addr - kAiBase, value);
    return;
  }
  set_reg(addr, value, size);
  if (addr >= 0xCC002030u && addr < 0xCC002040u) {
    // A display interrupt was reprogrammed: redo this frame's schedule from now.
    vi_schedule_frame();
    while (!vi_events_.empty() && vi_events_.front().first <= now_tb_) vi_events_.erase(vi_events_.begin());
  }
}

}  // namespace mvp::rt
