// Memory-mapped hardware (E0b items 7, 9 and 10): the Flipper register blocks at
// 0xCC000000. Registers without a model store what the guest writes and read it
// back; the models below answer what the boot polls. Time is the guest time base
// (advance(), and the CPU's clock between safe points):
// - VI: display interrupts DI0-DI3 at their programmed lines, every frame (R2);
//   the beam position follows the clock;
// - PI: the interrupt cause from the device states, and the mask;
// - EXI: immediate and DMA transfers complete at once; channel 0 device 1 is the
//   IPL chip's RTC and SRAM (fixed values, R12); channel 0 device 0 is the memory
//   card in slot A when the host inserted one (insert_card, runtime/memcard.h),
//   else absent like every other device;
// - DSP: reset and halt bits, the ARAM DMA (with 16 MiB of ARAM), the AI DMA
//   (a block every len x 32 bytes of 32 or 48 kHz stereo, AIDINT at each block
//   start), and the DSP side of the mailbox protocol, without running DSP code:
//   the IPL init microcode's answer, the boot ROM's task loader, the task
//   manager's switches to the CARD unlock microcode and back (its two mails,
//   then done), and the AX microcode's frame handshake (each command list is
//   answered with a yield
//   after a fixed time; the voices advance through runtime/ax.h, R8; the mix
//   is computed by an optional read-only observer, engine/audio, E3b);
// - DI: the DVD drive reads the disc image (vfs::Source); a command completes
//   after a fixed latency plus its transfer time (R7), then raises TCINT;
// - AI: the streaming sample counter counts at 32 or 48 kHz of virtual time
//   while playing (the SDK times its edges in __AI_SRC_INIT);
// - GX FIFO: the write-gather pipe (0xCC008000) collects 32 bytes and writes
//   them to the CPU FIFO in memory at the PI write pointer, which wraps at the
//   FIFO end. With the CP linked to it, the graphics processor reads from its
//   read pointer up to that write pointer at once (it is infinitely fast), and
//   stops at an enabled breakpoint until software moves or disables it. The
//   commands it reads are parsed (gx_fifo.h): draw-done and token commands raise
//   the PE interrupts, a reached breakpoint the CP interrupt. The first bytes
//   read are kept for reports.
// Every access is logged (capped) and counted.
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ppc/cpu.h"
#include "runtime/ax.h"
#include "runtime/gx_fifo.h"
#include "runtime/memcard.h"

namespace mvp::vfs {
class Source;
}

namespace mvp::rt {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

struct MmioAccess {
  u32 pc, addr, value;
  int size;
  bool write;
};

struct HardwareConfig {
  u32 rtc_seconds = 0x0D6C5A00;  // RTC counter (seconds since 2000-01-01): a fixed date, R12
  u8 sram_language = 0;          // English
  u8 sram_flags = 0x2C;          // sound mode and boot flags as a fresh console's SRAM; not progressive
  // DVD timing (R7): every drive command completes this many time-base ticks
  // after it starts, plus its DMA length at this rate. Both are part of the
  // netplay handshake. 40,500 ticks = 1 ms; 13,184 ticks per KiB = 3 MiB/s.
  std::uint32_t di_latency_tb = 40500;
  std::uint32_t di_tb_per_kib = 13184;
  // AX voices (R8, runtime/ax.h): each audio frame advances the running voices'
  // parameter blocks (play position, loop, end). Off: they never move (E0b).
  bool ax_voices = true;
};

// The write-gather pipe's 32-byte buffer (P2), where translated code may append
// to it without calling the runtime (wasm_emit's store helpers, for stores to
// 0xCC008000 that do not complete a burst), and what Hardware::write does for
// such a store besides appending: end a run of reads of one register (the poll
// detector) and count the access. Hardware keeps its own; the browser's runtime
// points it at the one its translated module knows (set_wgp_state).
struct WgpState {
  std::uint8_t buf[32] = {};
  std::uint32_t n = 0;            // bytes gathered (0-31)
  std::uint32_t fast = 0;         // 1: translated code may append (no access log is being kept)
  std::uint32_t last_read = 0;    // the last register read (0 after a write): the poll detector
  std::uint32_t pad = 0;
  std::uint64_t fast_stores = 0;  // stores the translated code appended (accesses to 0xCC008000)
  class Hardware* owner = nullptr;  // Q4: whose pipe it is (set_wgp_state; the module never reads it)
};

// What the AX voice model did (runtime/ax.h), summed over all frames.
struct AxStats {
  std::uint64_t voice_frames = 0, samples = 0, ended = 0, looped = 0, updates = 0, max_running = 0;
};

class Hardware : public ppc::Io {
 public:
  Hardware(ppc::Cpu* cpu, u8* mem1, u32 mem1_size, const HardwareConfig& cfg = {});

  u32 read(u32 addr, int size) override;
  void write(u32 addr, u32 value, int size) override;
  void* hardware() override { return this; }

  const std::vector<MmioAccess>& log() const { return log_; }
  // Accesses per register address (byte address; only the ones accessed).
  std::map<u32, std::uint64_t> read_counts() const { return counts(reads_); }
  std::map<u32, std::uint64_t> write_counts() const {
    auto m = counts(writes_);
    if (wgp_->fast_stores) m[0xCC008000u] += wgp_->fast_stores;  // P2: stores appended by translated code
    return m;
  }
  // P2: use `s` for the write-gather pipe's buffer from now on (its state moves there).
  void set_wgp_state(WgpState* s) {
    if (s == wgp_) return;
    *s = *wgp_;
    s->owner = this;
    wgp_ = s;
  }
  // Q4: a store to the write-gather pipe that translated code counted and ended
  // the poll detector's run for (as write() does: WgpState), while `fast` is set
  // (nothing is logged): what write() does for it from there on.
  void wgp_store_counted(u32 value, int size) { wgp_write(value, size); }
  // Q6: host code that runs a game function natively (runtime/hle_game.h) may
  // let the GP read its bursts at once: between gp_defer(true) and
  // gp_defer(false) a completed burst still goes to memory and moves the CPU
  // FIFO's write pointer, and the GP reads everything written at the end. No
  // guest code runs in between and the effects (the PE token, the finish flag,
  // the CP's pointers) land in the state before the next safe point either way,
  // so the guest sees the same; the commands are parsed in one piece.
  void gp_defer(bool on) {
    gp_defer_ = on;
    if (!on && gp_deferred_) {
      gp_deferred_ = false;
      gp_run();
    }
  }
  // Q4: a store of `size` bytes (`value` right-aligned, masked to its size) to the
  // write-gather pipe, at an address write() takes for it, by host code that runs
  // a game function natively (runtime/hle_game.h) for the instruction at `pc`,
  // with c.ticks == `ticks` there: what write(addr, value, size) does, inline
  // while nothing is logged (`fast`). V4: a store that completes a burst (or
  // reaches into the next) takes write()'s pipe path from there too (count, end
  // the poll detector's run, gather: no log entry once `fast` is set, and `fast`
  // stays set), with c.pc and c.ticks set first, as write() would have them.
  void wgp_store(u32 addr, u32 value, int size, u32 pc, std::uint64_t ticks) {
    WgpState& w = *wgp_;
    if (w.fast) {
      ++writes_[addr - kRegBase];
      w.last_read = 0;
      if (w.n + static_cast<u32>(size) < 32) {
        u8* g = w.buf + w.n;
        for (int k = 0; k < size; ++k) g[k] = static_cast<u8>(value >> (8 * (size - 1 - k)));
        w.n += static_cast<u32>(size);
        return;
      }
      cpu_->pc = pc;
      cpu_->ticks = ticks;
      wgp_write(value, size);
      return;
    }
    cpu_->pc = pc;
    cpu_->ticks = ticks;
    write(addr, value, size);
  }
  const std::vector<u8>& fifo() const { return fifo_; }  // the first bytes written to the write-gather pipe
  std::uint64_t fifo_bytes() const { return fifo_bytes_; }
  const GxFifo& gx() const { return gx_; }
  // The renderer (E1): sees every executed GX command and every VI field.
  void set_gfx_observer(GfxObserver* o) {
    gfx_ = o;
    gx_.set_observer(o);
  }
  std::uint64_t draw_done_count() const { return draw_dones_; }
  std::uint64_t gp_bursts() const { return gp_bursts_; }
  // mtspr WPAR: the write-gather pipe restarts empty; bytes of a partial burst are
  // dropped (GXRedirectWriteGatherPipe / GXRestoreWriteGatherPipe, E3a).
  void wgp_reset() {
    if (wgp_->n) ++wgp_dropped_;
    wgp_->n = 0;
  }
  std::uint64_t wgp_resets_dropping() const { return wgp_dropped_; }
  const std::array<u8, 64>& sram() const { return sram_; }

  // The memory card in slot A (runtime/memcard.h): inserted by the host before
  // the machine starts (offline only: an online game has none on either side).
  // Its contents are the player's storage; the host keeps them when writes()
  // moves (card()->image()).
  void insert_card(std::vector<u8> image) { card_.emplace(std::move(image)); }
  const MemoryCard* card() const { return card_ ? &*card_ : nullptr; }

  // ---- time and interrupts (E0b items 9 and 10) ----
  // Time is the guest time base (40.5 MHz), driven by the runtime's virtual clock.
  // Moves the devices to time `tb` (VI display interrupts so far).
  void advance(std::uint64_t tb);
  // The next time base at which advance() has something to do (~0 if none).
  std::uint64_t next_event_tb() const;
  // PI interrupt cause bits (0xCC003000) from the device states, and the mask.
  u32 pi_cause() const;
  u32 pi_mask() const { return reg(0xCC003004u); }
  bool interrupt_pending() const { return (pi_cause() & pi_mask() & 0xFFFFu) != 0; }
  std::uint64_t vi_fields() const { return vi_fields_; }

  // The disc the DVD drive reads (no disc: every DI command stops the run).
  void set_disc(vfs::Source* disc) { disc_ = disc; }
  std::uint64_t di_commands() const { return di_commands_; }
  std::uint64_t dsp_frames() const { return dsp_frames_; }
  const AxStats& ax_stats() const { return ax_; }
  // The mixer (E3b, engine/audio): sees every AX frame the DSP processes, read-only.
  void set_ax_observer(ax::FrameObserver* o) { ax_obs_ = o; }
  // The rate the AI plays the DSP's output at (AICR bit 6: 32 kHz, else 48 kHz).
  u32 ai_dsp_rate() const { return (ai_cr_ & 0x40) ? 32000u : 48000u; }
  int dsp_state() const { return static_cast<int>(dsp_state_); }
  std::size_t dsp_mails_waiting() const { return dsp_out_.size(); }
  u32 dsp_control() const { return dsp_cr_; }
  u32 dsp_mail_to_cpu() const { return dsp_mail_to_cpu_; }
  std::uint64_t ai_dma_blocks() const { return aid_interrupts_; }
  std::uint64_t di_bytes() const { return di_bytes_; }

  // A read of one register this many times in a row stops the run (a poll that
  // the device model does not answer).
  std::uint64_t poll_limit = 1000000;

  // Save states (runtime/save_state.h): a copy of every device's state, ARAM and
  // the GX FIFO parser included (and, alongside, statistics). The copy belongs to
  // this machine: restore_from() keeps this object's observers, disc and
  // write-gather pipe location and only takes the state.
  std::unique_ptr<Hardware> snapshot() const;
  void save_into(Hardware& snap) const;
  void restore_from(const Hardware& snap);
  // Save-state files (runtime/state_file.h): the same device state, field by
  // field (common/state_io.h), into or from a snapshot object made by
  // snapshot(). Not the observers, the disc, the access log or the access
  // counts, which stay the snapshot's (its machine's). A member added to this
  // class that the guest can see belongs in state_fields() too.
  void write_state(StateWriter& w) const;
  void read_state(StateReader& r);
  // Save-state files, version 2: what a card adds (its transfer state and the
  // DSP's card task), after the rest. A version 1 file had no card: read_card_state
  // is not called, and the card is as at power-on (locked, idle).
  void write_card_state(StateWriter& w) const;
  void read_card_state(StateReader& r);
  void reset_card_state();

  static const char* block_name(u32 addr);

 private:
  template <class Ar>
  void state_fields(Ar& a);
  u32 reg(u32 addr) const;
  void set_reg(u32 addr, u32 value, int size);
  u32 read_word(u32 a);               // 4-byte-aligned register read, with device behaviour
  void write_reg(u32 addr, u32 value, int size);

  // EXI
  struct ExiChannel {
    u32 csr = 0, mar = 0, len = 0, cr = 0, data = 0;
    // The selected device's transfer state (IPL chip on channel 0 device 1).
    bool have_cmd = false;
    u32 cmd = 0, cmd_bytes = 0, offset = 0;
  };
  void exi_write(int ch, u32 reg, u32 v);
  void exi_transfer(int ch);
  int exi_device(int ch) const;  // selected device index, or -1
  u8 ipl_read_byte(ExiChannel& e);
  void ipl_write_byte(ExiChannel& e, u8 b);

  // VI: display interrupts DI0-DI3 at their programmed lines, every frame.
  static constexpr std::uint64_t kFrameTb = 1351350;  // 2 fields of 675,675 (NTSC 59.94 Hz, R2)
  static constexpr std::uint64_t kLines = 525;
  std::uint64_t line_tb() const { return kFrameTb / kLines; }
  void vi_schedule_frame();
  u32 vi_current_line() const;
  std::uint64_t now_tb_ = 0, frame_start_tb_ = 0, vi_fields_ = 0;
  std::vector<std::pair<std::uint64_t, int>> vi_events_;  // (tb, DI index) this frame, sorted

  // The current time base: from the CPU's clock when there is one (MMIO happens
  // between safe points), else the last advance().
  std::uint64_t now() const;

  // AI: control (play 0x1, 48 kHz 0x2, counter reset 0x20) and the sample
  // counter, which runs from `ai_count_base_` at time `ai_t0_` while playing.
  void ai_write(u32 reg, u32 v);
  u32 ai_count(std::uint64_t tb) const;
  std::uint64_t ai_next_change_tb() const;
  u32 ai_cr_ = 0, ai_count_base_ = 0;
  std::uint64_t ai_t0_ = 0;

  // DSP mailbox HLE. Mails to the CPU wait in time order; one is in the mailbox
  // at a time.
  enum class DspState { Off, IplInit, RomReady, Loading, Running, Switching, CardTask };
  struct DspMail {
    std::uint64_t tb;
    u32 mail;
    bool irq;
  };
  void dsp_receive(u32 mail);
  void dsp_post(std::uint64_t delay_tb, u32 mail, bool irq);
  void dsp_deliver();
  DspState dsp_state_ = DspState::Off;
  u32 dsp_boots_ = 0, dsp_load_n_ = 0;
  // The task manager's switch (0xCDD10001, then __DSP_exec_task's ten mails), and
  // the CARD unlock task (runtime/memcard.h): 0 waiting for 0xFF000000, 1 for the
  // parameters' address, 2 done.
  std::array<u32, 10> dsp_switch_{};
  bool dsp_card_task_ = false;
  u32 dsp_card_step_ = 0;
  bool dsp_cmdlist_next_ = false;
  std::vector<DspMail> dsp_out_;
  std::uint64_t dsp_frames_ = 0;
  AxStats ax_;
  ax::FrameObserver* ax_obs_ = nullptr;

  // AI DMA (DSP registers 0x30-0x3A): the block being played and its end.
  void aid_start_block(std::uint64_t tb);
  std::uint64_t aid_block_tb(u32 blocks) const;
  bool aid_on_ = false;
  u32 aid_blocks_ = 0;
  std::uint64_t aid_start_tb_ = 0, aid_end_tb_ = ~0ull;
  std::uint64_t aid_interrupts_ = 0;

  // DI
  void di_write(u32 reg, u32 v);
  u32 di_read(u32 reg) const;
  void di_complete();
  vfs::Source* disc_ = nullptr;
  u32 di_sr_ = 0, di_cvr_ = 0, di_mar_ = 0, di_len_ = 0, di_cr_ = 0, di_imm_ = 0, di_cfg_ = 0;
  u32 di_cmd_[3] = {};
  std::uint64_t di_done_tb_ = ~0ull;
  std::uint64_t di_commands_ = 0, di_bytes_ = 0;

  // DSP
  void dsp_write16(u32 off, u16 v);
  u16 dsp_read16(u32 off);
  void dsp_aram_dma();

  // An interrupt cause may have appeared outside advance(): have the CPU check
  // at its next safe point (deterministic: the point is fixed by the code).
  void kick();

  // PE: interrupt control (enables in bits 0-1, token and finish status in
  // bits 2-3, write 1 to clear) and the last token.
  u16 pe_ctrl_ = 0, pe_token_ = 0;
  GxFifo gx_;
  GfxObserver* gfx_ = nullptr;

  // GX FIFO: the write-gather buffer, and the CP registers (16-bit, kept in
  // regs_; 32-bit values are lo/hi pairs).
  void wgp_write(u32 value, int size);
  void wgp_burst();
  void gp_run();
  void gp_consume(const u8* bytes, std::size_t n);
  u16 cp16(u32 off) const;
  u32 cp32(u32 lo_off) const { return cp16(lo_off) | u32(cp16(lo_off + 2)) << 16; }
  void set_cp32(u32 lo_off, u32 v);
  u32 cp_status() const;
  bool cp_cause() const;  // Q6: the CP interrupt's cause (a watermark or the breakpoint, enabled)
  u32 gp_distance() const;
  bool gp_at_breakpoint() const;
  WgpState own_wgp_;            // the write-gather buffer (P2: or the runtime's, set_wgp_state)
  WgpState* wgp_ = &own_wgp_;
  std::uint64_t wgp_dropped_ = 0;
  bool gp_defer_ = false, gp_deferred_ = false;  // Q6: gp_defer
  std::uint64_t gp_bursts_ = 0;
  std::uint64_t fifo_bytes_ = 0, draw_dones_ = 0;
  static constexpr std::size_t kFifoKeep = 1u << 20;

  ppc::Cpu* cpu_;
  u8* mem1_;
  u32 mem1_size_;
  HardwareConfig cfg_;
  // The hardware blocks' registers, 0xCC000000-0xCC008FFF, as 32-bit words
  // (flat arrays: every GX FIFO burst reads and writes several of them).
  static constexpr u32 kRegBase = 0xCC000000u, kRegBytes = 0x9000u;
  std::vector<u32> regs_ = std::vector<u32>(kRegBytes / 4, 0);
  std::vector<MmioAccess> log_;
  std::vector<std::uint64_t> reads_ = std::vector<std::uint64_t>(kRegBytes, 0),
                             writes_ = std::vector<std::uint64_t>(kRegBytes, 0);
  static std::map<u32, std::uint64_t> counts(const std::vector<std::uint64_t>& v) {
    std::map<u32, std::uint64_t> m;
    for (u32 k = 0; k < kRegBytes; ++k)
      if (v[k]) m[kRegBase + k] = v[k];
    return m;
  }
  std::vector<u8> fifo_;
  std::uint64_t same_reads_ = 0;  // with wgp_->last_read: the poll detector

  ExiChannel exi_[3];
  std::array<u8, 64> sram_{};
  u32 rtc_ = 0;

  u16 dsp_cr_ = 0;
  u32 dsp_mail_to_cpu_ = 0;  // bit 31 = valid
  u32 dsp_mail_to_dsp_ = 0;
  u32 ar_mm_ = 0, ar_ar_ = 0, ar_cnt_ = 0;
  bool dsp_booting_ = false;  // reset and halted since the last reset: the IPL init ucode runs on unhalt
  std::vector<u8> aram_;
  std::optional<MemoryCard> card_;
};

// Thrown when the run must stop: an unanswered poll, an access outside the
// register blocks, or a stop requested by a tool.
struct StopRun {
  std::string why;
};

}  // namespace mvp::rt
