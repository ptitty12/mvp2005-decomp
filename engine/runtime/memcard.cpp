#include "runtime/memcard.h"

#include <algorithm>
#include <cstring>

namespace mvp::rt {

namespace {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// The unlock's scramble, as the SDK computes it (src/dolphin/card/CARDUnlock.c).
u32 exnor_1st(u32 data, u32 rshift) {
  u32 work = data;
  for (u32 i = 0; i < rshift; i++) {
    const u32 wk = ~(work ^ (work >> 7) ^ (work >> 15) ^ (work >> 23));
    work = (work >> 1) | ((wk << 30) & 0x40000000u);
  }
  return work;
}

u32 exnor(u32 data, u32 lshift) {
  u32 work = data;
  for (u32 i = 0; i < lshift; i++) {
    const u32 wk = ~(work ^ (work << 7) ^ (work << 15) ^ (work << 23));
    work = (work << 1) | ((wk >> 30) & 0x00000002u);
  }
  return work;
}

u32 bitrev(u32 data) {
  u32 wk = 0, k = 0, j = 1;
  for (u32 i = 0; i < 32; i++) {
    if (i > 15) {
      if (i == 31) wk |= ((data & (1u << 31)) >> 31) & 1u;
      else {
        wk |= (data & (1u << i)) >> j;
        j += 2;
      }
    } else {
      wk |= (data & (1u << i)) << (31 - i - k);
      k++;
    }
  }
  return wk;
}

// The scramble moved on by `shift` bits (the SDK's step after each word it reads).
u32 advance(u32 s, u32 shift) {
  const u32 wk = exnor(s, shift);
  const u32 wk1 = ~(wk ^ (wk << 7) ^ (wk << 15) ^ (wk << 23));
  return wk | ((wk1 >> 31) & 1u);
}

void put16(u8* p, u16 v) {
  p[0] = static_cast<u8>(v >> 8);
  p[1] = static_cast<u8>(v);
}
void put32(u8* p, u32 v) {
  for (int k = 0; k < 4; ++k) p[k] = static_cast<u8>(v >> (24 - 8 * k));
}

// __CARDCheckSum: the sum of the big-endian 16-bit words and of their complements (0xFFFF counts as 0).
void checksum(const u8* p, u32 length, u8* sum, u8* inv) {
  u16 s = 0, i = 0;
  for (u32 k = 0; k + 1 < length; k += 2) {
    const u16 w = static_cast<u16>(p[k] << 8 | p[k + 1]);
    s = static_cast<u16>(s + w);
    i = static_cast<u16>(i + static_cast<u16>(~w));
  }
  put16(sum, s == 0xFFFF ? 0 : s);
  put16(inv, i == 0xFFFF ? 0 : i);
}

// The SDK's 64-bit generator for a card's serial (CARDFormat, VerifyID): `rand * 1103515245 + 12345`, shifted
// right arithmetically, in two's complement (the console's wrap-around).
std::int64_t next_rand(std::int64_t r) {
  return static_cast<std::int64_t>(static_cast<u64>(r) * 1103515245u + 12345u) >> 16;
}

}  // namespace

MemoryCard::MemoryCard(std::vector<u8> image) : image_(std::move(image)) { image_.resize(kSize, 0xFF); }

void MemoryCard::power_on() {
  status_ = kReady;
  irq_on_ = unlocked_ = selected_ = false;
  unlock_step_ = scramble_ = cmd_len_ = addr_ = read_n_ = out_at_ = 0;
  cmd_.clear();
  out_.clear();
  page_.clear();
  busy_until_ = ~0ull;
}

std::vector<std::uint8_t> MemoryCard::formatted(u32 counter_bias, u8 language, std::uint64_t format_time) {
  constexpr u32 kBlocks = kSize / kSector, kSystemBlocks = 5;
  std::vector<u8> img(kSize, 0xFF);
  // Block 0: the card's ID (CARDID).
  u8* id = img.data();
  std::int64_t rand = static_cast<std::int64_t>(format_time);
  const u8 flash_id[12] = {};
  for (int i = 0; i < 12; i++) {
    rand = next_rand(rand);
    id[i] = static_cast<u8>(flash_id[i] + static_cast<u8>(rand));
    rand = next_rand(rand) & 0x7FFF;
  }
  for (int k = 0; k < 8; ++k) id[12 + k] = static_cast<u8>(format_time >> (56 - 8 * k));  // the format time
  put32(id + 20, counter_bias);
  put32(id + 24, language);
  put32(id + 28, 0);           // the VI's dimming register at the time: 0 on this console
  put16(id + 0x20, 0);         // device ID
  put16(id + 0x22, kExiId & 0xFC);  // size in Mbit
  put16(id + 0x24, 0);         // encoding: ANSI (the USA game)
  checksum(id, 0x1FC, id + 0x1FC, id + 0x1FE);
  // Blocks 1 and 2: the directory, twice, empty; its check at the end (CARDDirCheck).
  for (u32 i = 0; i < 2; ++i) {
    u8* dir = img.data() + (1 + i) * kSector;
    put16(dir + 0x1FFA, static_cast<u16>(i));  // checkCode
    checksum(dir, kSector - 4, dir + 0x1FFC, dir + 0x1FFE);
  }
  // Blocks 3 and 4: the block map, twice: every file block free.
  for (u32 i = 0; i < 2; ++i) {
    u8* fat = img.data() + (3 + i) * kSector;
    std::memset(fat, 0, kSector);
    put16(fat + 4, static_cast<u16>(i));                       // checkCode
    put16(fat + 6, static_cast<u16>(kBlocks - kSystemBlocks));  // free blocks
    checksum(fat + 4, kSector - 4, fat + 0, fat + 2);
  }
  return img;
}

void MemoryCard::select() {
  selected_ = true;
  cmd_.clear();
  cmd_len_ = 0;
  read_n_ = 0;
  out_.clear();
  out_at_ = 0;
  page_.clear();
}

void MemoryCard::write_byte(u8 b) {
  if (!selected_) return;
  if (cmd_.empty()) {
    cmd_.push_back(b);
    switch (b) {
      case 0x00: case 0x83: case 0x85: case 0x81: cmd_len_ = 2; break;
      case 0x52: case 0xF2: cmd_len_ = 5; break;
      case 0xF1: case 0xF4: cmd_len_ = 3; break;
      case 0x89:  // clear the error bits
        status_ = static_cast<u8>(status_ & ~0x18);
        cmd_len_ = 1;
        break;
      default: cmd_len_ = 1; break;  // wake up, sleep and the rest: nothing to do
    }
    return;
  }
  if (cmd_.size() < cmd_len_) {
    cmd_.push_back(b);
    if (cmd_.size() < cmd_len_) return;
    switch (cmd_[0]) {
      case 0x00: out_ = {static_cast<u8>(kExiId >> 24), static_cast<u8>(kExiId >> 16), static_cast<u8>(kExiId >> 8),
                         static_cast<u8>(kExiId)}; break;
      case 0x83: out_ = {status_}; break;
      case 0x85: out_ = {static_cast<u8>(kVendorId >> 8), static_cast<u8>(kVendorId)}; break;
      case 0x81: irq_on_ = (cmd_[1] & 1) != 0; break;
      case 0x52: start_read(); break;
      case 0xF2: addr_ = (u32(cmd_[1] & 0x7F) << 17) | (u32(cmd_[2]) << 9) | (u32(cmd_[3] & 3) << 7) | (cmd_[4] & 0x7F); break;
      default: break;
    }
    return;
  }
  // After the command: a page's data, or a read's dummy bytes (ignored).
  if (cmd_[0] == 0xF2 && page_.size() < kPage) page_.push_back(b);
}

void MemoryCard::start_read() {
  if (unlocked_) {
    addr_ = (u32(cmd_[1] & 0x7F) << 17) | (u32(cmd_[2]) << 9) | (u32(cmd_[3] & 3) << 7) | (cmd_[4] & 0x7F);
    return;
  }
  // The unlock's reads (ReadArrayUnlock). The first sends the random value in
  // the address bytes (mode 0); the second's answer is the flash ID, scrambled.
  if (unlock_step_ == 0) {
    addr_ = (u32(cmd_[1] & 3) << 29) | (u32(cmd_[2]) << 21) | (u32(cmd_[3] & 3) << 19) | (u32(cmd_[4] & 0x7F) << 12);
  } else if (unlock_step_ == 1) {
    prepare_unlock_answer();
  }
}

void MemoryCard::prepare_unlock_answer() {
  // para1A, para1B, Ans1 (the flash ID: zeros), para2A, para2B (the DSP's input: zeros), each XORed with the
  // scramble as the SDK moves it on after each word.
  out_.assign(20, 0);
  u32 s = scramble_;
  for (int w = 0; w < 5; ++w) {
    put32(out_.data() + 4 * w, 0u ^ s);
    s = advance(s, 32);
  }
}

std::uint8_t MemoryCard::read_byte() {
  if (!selected_) return 0;
  ++read_n_;
  if (out_at_ < out_.size()) return out_[out_at_++];
  if (!cmd_.empty() && cmd_[0] == 0x52 && cmd_.size() == cmd_len_ && unlocked_) {
    const u8 b = image_[addr_ % kSize];
    addr_ = (addr_ + 1) % kSize;
    return b;
  }
  return 0;
}

bool MemoryCard::deselect(std::uint64_t now) {
  if (!selected_) return false;
  selected_ = false;
  if (cmd_.empty() || cmd_.size() < cmd_len_) return false;
  switch (cmd_[0]) {
    case 0x52:
      if (unlocked_) return false;
      if (unlock_step_ == 0) {  // the scramble the SDK derives from its random value and the bytes it read
        const u32 wk = exnor_1st(addr_, read_n_ * 8 + 1);
        const u32 wk1 = ~(wk ^ (wk >> 7) ^ (wk >> 15) ^ (wk >> 23));
        scramble_ = bitrev(wk | ((wk1 << 31) & 0x80000000u));
      }
      if (++unlock_step_ == 4) {
        unlocked_ = true;
        status_ |= kUnlocked;
      }
      return false;
    case 0xF2: {
      if (!unlocked_) return false;
      const u32 base = (addr_ % kSize) & ~(kPage - 1);
      for (u32 i = 0; i < page_.size(); ++i) image_[base + ((addr_ + i) & (kPage - 1))] = page_[i];
      busy_until_ = now + kProgramTb;
      ++writes_;
      return true;
    }
    case 0xF1: {
      if (!unlocked_) return false;
      const u32 sector = ((u32(cmd_[1] & 0x7F) << 17) | (u32(cmd_[2]) << 9)) % kSize & ~(kSector - 1);
      std::fill_n(image_.begin() + sector, kSector, u8{0xFF});
      busy_until_ = now + kEraseTb;
      ++writes_;
      return true;
    }
    case 0xF4:
      if (!unlocked_) return false;
      std::fill(image_.begin(), image_.end(), u8{0xFF});
      busy_until_ = now + kEraseTb;
      ++writes_;
      return true;
    default:
      return false;
  }
}

bool MemoryCard::finish_busy() {
  busy_until_ = ~0ull;
  return irq_on_;
}

}  // namespace mvp::rt
