// The memory card in slot A (EXI channel 0, device 0): a 64 Mbit flash card,
// 1019 blocks of 8 KiB for files, as Nintendo's Memory Card 1019 (the game asks
// for 524 blocks free for all its kinds of files, more than a 251-block card
// has). It answers the way the SDK's CARD library drives it (src/dolphin/card:
// __CARDReadStatus, __CARDReadSegment, __CARDWritePage, __CARDEraseSector,
// ReadArrayUnlock).
//
// Commands, each from a chip select to the next deselect:
//   00 00            the EXI ID (4 bytes: 0x00000040, 64 Mbit, 8 KiB sectors, latency index 0)
//   83 00            status (1 byte): 0x40 unlocked, 0x18 the error bits, 0x01 ready
//   85 00            the vendor ID (2 bytes)
//   89               clear the error bits
//   81 xx            interrupts on (xx = 1) or off: an EXI interrupt when a program or an erase ends
//   52 a a a a       read from the address (then the latency's dummy bytes, then the data)
//   F2 a a a a       program a 128-byte page (the data follows)
//   F1 a a           erase an 8 KiB sector (to 0xFF)
//   F4 00 00         erase the card
// A program or an erase happens at the deselect and ends after a fixed time
// (deterministic), with the EXI interrupt when it is on.
//
// The card is locked from power-on until the SDK's unlock (__CARDUnlock): four
// reads with special addresses. The first sends a random value and reads some
// dummy bytes; the card derives the scramble the SDK derives from them, and the
// second read returns the card's flash ID scrambled with it, so the SDK recovers
// the same ID at every boot (DoMount stores it in SRAM, and the card's header
// is checked against it: VerifyID). After the fourth read the card is unlocked.
// The flash ID is twelve zero bytes. The DSP's part of the unlock is the
// Hardware's (runtime/hw.h, the card task).
//
// The card's contents are the player's storage, not machine state: save-state
// files leave them out (runtime/state_file.h); its transfer state is in them.
#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "common/state_io.h"

namespace mvp::rt {

class MemoryCard {
 public:
  using u8 = std::uint8_t;
  using u32 = std::uint32_t;

  static constexpr u32 kSize = 8u << 20;   // 64 Mbit
  static constexpr u32 kSector = 8192;     // a block
  static constexpr u32 kPage = 128;
  static constexpr u32 kExiId = 0x00000040u;
  static constexpr std::uint16_t kVendorId = 0xC221;
  static constexpr u8 kUnlocked = 0x40, kReady = 0x01;
  // Time a page program and a sector (or card) erase take, in time-base ticks.
  static constexpr std::uint64_t kProgramTb = 40500;     // 1 ms
  static constexpr std::uint64_t kEraseTb = 4 * 40500;   // 4 ms

  // A card with these contents (kSize bytes; anything else is cut or padded with 0xFF).
  explicit MemoryCard(std::vector<u8> image);

  // A new card, formatted as the game's CARDFormat formats one on this console
  // (src/dolphin/card/tu_8041C2D8.c): the SRAM's counter bias and language and
  // the flash ID (zeros) go into the header, with `format_time` (an OSTime) as
  // its time and seed; the directories and block maps empty; the files' blocks erased.
  static std::vector<u8> formatted(u32 counter_bias, u8 language, std::uint64_t format_time);

  // The EXI transfer, byte by byte between a select() and a deselect().
  void select();
  void write_byte(u8 b);
  u8 read_byte();
  // Ends the command; `now` is the time base. True if a program or an erase
  // started (the Hardware then raises the interrupt at busy_until()).
  bool deselect(std::uint64_t now);
  std::uint64_t busy_until() const { return busy_until_; }
  // The program or erase ended: true if the EXI interrupt should be raised.
  bool finish_busy();

  // As at power-on (locked, idle, interrupts off), with the same contents: a
  // save state from a machine that had no card is loaded on one that has.
  void power_on();

  bool interrupts_on() const { return irq_on_; }
  bool unlocked() const { return unlocked_; }
  const std::vector<u8>& image() const { return image_; }
  // Counts every program and erase: the host keeps the image when it changes.
  std::uint64_t writes() const { return writes_; }

  // V4: for Hardware's snapshots, which copy the image by sectors (only those
  // written since a snapshot was taken): every program or erase stamps its
  // sectors with the epoch Hardware gives (set_epoch). Host bookkeeping, not the
  // card's state: Hardware keeps it out of its member-wise copies.
  std::vector<u8>& image_store() { return image_; }
  std::vector<std::uint32_t>& stamps() { return stamp_; }
  const std::vector<std::uint32_t>& stamps() const { return stamp_; }
  void set_epoch(std::uint32_t e) { epoch_ = e; }
  void stamp_all() { std::fill(stamp_.begin(), stamp_.end(), epoch_); }

  // The transfer state (not the contents) for save-state files.
  template <class Ar>
  void state_fields(Ar& a) {
    a(status_);
    a(irq_on_);
    a(unlocked_);
    a(unlock_step_);
    a(scramble_);
    a(selected_);
    a(cmd_);
    a(cmd_len_);
    a(addr_);
    a(read_n_);
    a(out_);
    a(out_at_);
    a(busy_until_);
    a(page_);
  }

 private:
  void start_read();
  void prepare_unlock_answer();

  std::vector<u8> image_;
  u8 status_ = kReady;
  bool irq_on_ = false, unlocked_ = false;
  u32 unlock_step_ = 0;   // unlock reads done (four unlock the card)
  u32 scramble_ = 0;      // the unlock scramble after the first read
  bool selected_ = false;
  std::vector<u8> cmd_;   // the command bytes so far (at most 5, then the page's data for F2)
  u32 cmd_len_ = 0;
  u32 addr_ = 0;          // a read's next address
  u32 read_n_ = 0;        // bytes read during this command
  std::vector<u8> out_;   // an answer's bytes (ID, status, vendor ID, the unlock's)
  u32 out_at_ = 0;
  std::uint64_t busy_until_ = ~0ull;
  std::vector<u8> page_;  // a page program's data
  std::uint64_t writes_ = 0;
  std::vector<std::uint32_t> stamp_ = std::vector<std::uint32_t>(kSize / kSector, 0);  // V4: per sector
  std::uint32_t epoch_ = 0;
};

}  // namespace mvp::rt
