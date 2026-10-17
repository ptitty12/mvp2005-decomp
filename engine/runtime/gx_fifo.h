// The GX command stream (E0b item 10): the bytes the game writes to the
// write-gather pipe, split into commands as the command processor would. E0b
// draws nothing; it only needs the commands that report back to the CPU (the
// pixel engine's draw-done and token writes, which raise PE interrupts) and the
// vertex formats, to know where each primitive's vertex data ends.
//
// Command lengths follow the public GX documentation (YAGCD; the Dolphin SDK's
// GX macros): NOP 1 byte, CP register load 6, XF load 5 + 4n, indexed XF load
// 5, display list call 9, vertex cache invalidate 1, BP load 5, primitives 3 +
// count x vertex size from the current VCD and VAT registers. A display list
// call runs the list's commands from memory (they can change the vertex formats).
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace mvp::rt {

// One command (Q4: GfxObserver::gx_commands).
struct GxCommand {
  const std::uint8_t* p;
  std::uint32_t len;
};

// What the renderer (engine/gfx, E1) sees of the machine: every command the
// graphics processor executes, in order and complete with its payload (display
// lists arrive as the commands inside them, not as the call), and every VI
// field with the frame buffer address the video interface scans out.
// Observers only read; nothing they do reaches guest state (determinism).
class GfxObserver {
 public:
  virtual ~GfxObserver() = default;
  virtual void gx_command(const std::uint8_t* cmd, std::size_t len) = 0;
  // Q4: the commands one push() executed, in order (GxFifo hands them over
  // together, before it returns; the bytes stay valid until then). By default
  // each goes to gx_command.
  virtual void gx_commands(const GxCommand* cmds, std::size_t n) {
    for (std::size_t k = 0; k < n; ++k) gx_command(cmds[k].p, cmds[k].len);
  }
  // `xfb` is the physical address of the top field's frame buffer (VI TFBL).
  virtual void vi_field(std::uint64_t field, std::uint32_t xfb) = 0;
  // P2: the guest is about to be able to learn that the graphics processor has
  // consumed every command so far (a draw-done or token command completed; it
  // reads the CP status, read pointer or read-write distance; the GP stopped at
  // a breakpoint). From then on it may rewrite memory those commands point at
  // (vertex arrays, matrices, palettes, textures). A renderer that runs behind
  // the emulation (on its own thread) must have read that memory by now; an
  // inline renderer has nothing to do. Nothing here changes guest state.
  virtual void gp_sync() {}
  // Q1: memory over [addr, addr + len) (physical) was written without the data
  // cache (DMA from the drive, ARAM or EXI; the write-gather pipe), so the GP
  // sees it at once (gfx/gp_memory.h; the cache's write-backs come through
  // rt::g_dc_flush_hook). Nothing here changes guest state.
  virtual void gp_write_through(std::uint32_t /*addr*/, std::uint32_t /*len*/) {}
};

class GxFifo {
 public:
  // What a BP register load reports to the pixel engine.
  struct Effects {
    bool draw_done = false;  // BP 0x45 with bit 1: the finish interrupt
    std::uint32_t draw_dones = 0;  // Q6: how many (a push may hold several)
    bool token_int = false;  // BP 0x48: token and the token interrupt
    bool token_set = false;  // BP 0x47 or 0x48: the token register
    std::uint16_t token = 0;
  };

  // Appends bytes and parses every complete command; returns the PE effects of
  // the commands completed by these bytes.
  Effects push(const std::uint8_t* bytes, std::size_t n);

  // Memory that display list addresses (physical) point into.
  void set_memory(const std::uint8_t* mem, std::uint32_t size) {
    mem_ = mem;
    mem_size_ = size;
  }

  // The renderer's view of the stream (null: none).
  void set_observer(GfxObserver* o) { observer_ = o; }

  // Vertex size in bytes for vertex format `fmt` (0-7) from the current state.
  std::uint32_t vertex_size(int fmt) const;

  std::uint64_t commands() const { return commands_; }
  std::uint64_t bp_loads() const { return bp_loads_; }
  std::uint64_t primitives() const { return primitives_; }
  std::uint64_t vertices() const { return vertices_; }
  std::uint64_t display_lists() const { return display_lists_; }
  // Display lists that ended inside a command, held one that starts no command,
  // or lay outside memory (their remaining bytes are skipped).
  std::uint64_t display_list_errors() const { return display_list_errors_; }
  // A byte that starts no known command stops the parse for good (the stream
  // position is lost); its value and offset are kept for the report.
  bool broken() const { return broken_; }
  std::uint8_t broken_opcode() const { return broken_opcode_; }
  std::uint64_t broken_offset() const { return broken_offset_; }
  // For the report when the parse is lost: the last commands before it (stream
  // offset, length, first bytes) and the bytes from the lost position on.
  std::string broken_context() const;

 private:
  // Length of the command at p (n bytes available), 0 if more bytes are needed
  // to know it, or ~0 if p[0] starts no command.
  std::size_t command_length(const std::uint8_t* p, std::size_t n) const;
  void execute(const std::uint8_t* cmd, std::size_t len, Effects& fx, bool in_list);
  void call_list(std::uint32_t addr, std::uint32_t size, Effects& fx);
  // Q4: hands the commands collected since the last call to the observer.
  void flush_commands() {
    if (batch_n_) observer_->gx_commands(batch_.data(), batch_n_);
    batch_n_ = 0;
  }

  // Bytes of an incomplete command (and what follows it), waiting for the rest:
  // pending_[pbeg_, pend_) (Q4: appended and consumed in place).
  std::vector<std::uint8_t> pending_ = std::vector<std::uint8_t>(4096);
  std::size_t pbeg_ = 0, pend_ = 0;
  void pending_append(const std::uint8_t* bytes, std::size_t n);
  // P1: the length of the incomplete command at the head of the pending bytes, once known
  // (a long primitive arrives over many bursts: it is measured once, not per burst).
  std::size_t need_ = 0;
  // P1: vertex sizes per format, valid until a VCD or VAT write.
  mutable std::array<std::uint32_t, 8> vsize_{};
  mutable std::uint8_t vsize_valid_ = 0;
  std::uint64_t consumed_ = 0;
  std::uint32_t vcd_lo_ = 0, vcd_hi_ = 0;
  std::array<std::uint32_t, 8> vat_a_{}, vat_b_{}, vat_c_{};
  std::uint64_t commands_ = 0, bp_loads_ = 0, primitives_ = 0, vertices_ = 0, display_lists_ = 0;
  std::uint64_t display_list_errors_ = 0;
  GfxObserver* observer_ = nullptr;
  const std::uint8_t* mem_ = nullptr;
  std::uint32_t mem_size_ = 0;
  bool broken_ = false;
  std::uint8_t broken_opcode_ = 0;
  std::uint64_t broken_offset_ = 0;
  // Q4: the commands executed by the current push(), for the observer.
  std::array<GxCommand, 256> batch_{};
  std::size_t batch_n_ = 0;
  // The last commands' offsets and lengths, and (Q4) the stream's last bytes, from
  // which the report takes their first bytes (instead of a copy per command).
  struct Recent {
    std::uint64_t offset = 0;
    std::uint32_t len = 0;
  };
  std::array<Recent, 32> recent_{};  // a power of two
  std::uint64_t recent_n_ = 0;
  static constexpr std::size_t kHistory = 4096;  // a power of two
  std::array<std::uint8_t, kHistory> history_{};
  std::uint64_t pushed_ = 0;  // stream bytes pushed (history_ holds the last kHistory of them)
  std::vector<std::uint8_t> broken_tail_;
};

}  // namespace mvp::rt
