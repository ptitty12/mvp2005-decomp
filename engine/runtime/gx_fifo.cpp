#include "runtime/gx_fifo.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "common/state_io.h"

namespace mvp::rt {

namespace {

std::uint32_t be32_at(const std::uint8_t* p) {
  return std::uint32_t(p[0]) << 24 | std::uint32_t(p[1]) << 16 | std::uint32_t(p[2]) << 8 | p[3];
}

// Bytes per component for the VAT component formats u8, s8, u16, s16, f32.
std::uint32_t comp_bytes(std::uint32_t fmt) {
  switch (fmt) {
    case 0: case 1: return 1;
    case 2: case 3: return 2;
    default: return 4;
  }
}

// Bytes per direct color for the formats RGB565, RGB8, RGBX8, RGBA4, RGBA6, RGBA8.
std::uint32_t color_bytes(std::uint32_t fmt) {
  switch (fmt) {
    case 0: case 3: return 2;
    case 1: case 4: return 3;
    default: return 4;
  }
}

std::uint32_t bits(std::uint32_t v, int lo, int n) { return (v >> lo) & ((1u << n) - 1); }

}  // namespace

std::uint32_t GxFifo::vertex_size(int fmt) const {
  std::uint32_t a = vat_a_[fmt], b = vat_b_[fmt], c = vat_c_[fmt];
  auto index_bytes = [](std::uint32_t type) { return type == 2 ? 1u : type == 3 ? 2u : 0u; };
  std::uint32_t size = 0;
  // Matrix indices (bit 0: position/normal, bits 1-8: texture coordinates 0-7).
  for (int k = 0; k < 9; ++k) size += bits(vcd_lo_, k, 1);
  // Position.
  std::uint32_t t = bits(vcd_lo_, 9, 2);
  if (t == 1) size += (bits(a, 0, 1) ? 3 : 2) * comp_bytes(bits(a, 1, 3));
  else size += index_bytes(t);
  // Normal (or normal, binormal, tangent).
  t = bits(vcd_lo_, 11, 2);
  bool nbt = bits(a, 9, 1) != 0;
  if (t == 1) size += (nbt ? 9 : 3) * comp_bytes(bits(a, 10, 3));
  else size += index_bytes(t) * (nbt && bits(a, 31, 1) ? 3 : 1);
  // Colors 0 and 1.
  for (int k = 0; k < 2; ++k) {
    t = bits(vcd_lo_, 13 + 2 * k, 2);
    if (t == 1) size += color_bytes(bits(a, 14 + 4 * k, 3));
    else size += index_bytes(t);
  }
  // Texture coordinates 0-7: count bit and format, spread over VAT A, B and C.
  static constexpr int kCnt[8][2] = {{0, 21}, {1, 0}, {1, 9}, {1, 18}, {1, 27}, {2, 5}, {2, 14}, {2, 23}};
  for (int k = 0; k < 8; ++k) {
    t = bits(vcd_hi_, 2 * k, 2);
    if (t == 1) {
      std::uint32_t reg = kCnt[k][0] == 0 ? a : kCnt[k][0] == 1 ? b : c;
      int at = kCnt[k][1];
      size += (bits(reg, at, 1) ? 2 : 1) * comp_bytes(bits(reg, at + 1, 3));
    } else {
      size += index_bytes(t);
    }
  }
  return size;
}

std::size_t GxFifo::command_length(const std::uint8_t* p, std::size_t n) const {
  const std::uint8_t op = p[0];
  switch (op) {
    case 0x00: case 0x48: return 1;
    case 0x08: return 6;
    case 0x10:
      if (n < 5) return 0;
      return 5 + 4 * (std::size_t(be32_at(p + 1) >> 16 & 0xFFFF) + 1);
    case 0x20: case 0x28: case 0x30: case 0x38: case 0x61: return 5;
    case 0x40: return 9;
    default:
      break;
  }
  if (op >= 0x80 && op < 0xC0) {  // quads, triangles, strips, fans, lines, line strips, points
    if (n < 3) return 0;
    std::size_t count = std::size_t(p[1]) << 8 | p[2];
    const int fmt = op & 7;
    if (!(vsize_valid_ & (1u << fmt))) {
      vsize_[fmt] = vertex_size(fmt);
      vsize_valid_ |= static_cast<std::uint8_t>(1u << fmt);
    }
    return 3 + count * vsize_[fmt];
  }
  return ~std::size_t(0);  // not a command
}

void GxFifo::call_list(std::uint32_t addr, std::uint32_t size, Effects& fx) {
  ++display_lists_;
  addr &= 0x03FFFFFFu;
  if (!mem_ || addr > mem_size_ || size > mem_size_ - addr) {
    ++display_list_errors_;
    return;
  }
  const std::uint8_t* p = mem_ + addr;
  std::size_t at = 0;
  while (at < size) {
    std::size_t len = command_length(p + at, size - at);
    if (len == 0 || len == ~std::size_t(0) || len > size - at) {
      // Lists are padded with NOPs to 32 bytes; anything else is an error.
      ++display_list_errors_;
      return;
    }
    execute(p + at, len, fx, true);
    at += len;
  }
}

void GxFifo::execute(const std::uint8_t* cmd, std::size_t len, Effects& fx, bool in_list) {
  ++commands_;
  if (observer_ && cmd[0] != 0x40) {  // Q4: collected, handed over together (flush_commands)
    if (batch_n_ == batch_.size()) flush_commands();
    batch_[batch_n_++] = {cmd, static_cast<std::uint32_t>(len)};
  }
  switch (cmd[0]) {
    case 0x08: {
      std::uint8_t reg = cmd[1];
      std::uint32_t v = be32_at(cmd + 2);
      if (reg == 0x50) vcd_lo_ = v;
      else if (reg == 0x60) vcd_hi_ = v;
      else if ((reg & 0xF8) == 0x70) vat_a_[reg & 7] = v;
      else if ((reg & 0xF8) == 0x80) vat_b_[reg & 7] = v;
      else if ((reg & 0xF8) == 0x90) vat_c_[reg & 7] = v;
      else return;
      vsize_valid_ = 0;  // the vertex sizes
      return;
    }
    case 0x40:
      if (in_list) {
        ++display_list_errors_;  // lists do not nest
        return;
      }
      call_list(be32_at(cmd + 1), be32_at(cmd + 5), fx);
      return;
    case 0x61: {
      ++bp_loads_;
      std::uint32_t v = be32_at(cmd + 1);
      std::uint32_t reg = v >> 24;
      if (reg == 0x45 && (v & 2)) {
        fx.draw_done = true;
        ++fx.draw_dones;
      }
      if (reg == 0x47 || reg == 0x48) {
        fx.token_set = true;
        fx.token = static_cast<std::uint16_t>(v);
        if (reg == 0x48) fx.token_int = true;
      }
      return;
    }
    default:
      if (cmd[0] >= 0x80) {
        ++primitives_;
        vertices_ += std::uint32_t(cmd[1]) << 8 | cmd[2];
      }
      return;
  }
}

std::string GxFifo::broken_context() const {
  std::string out;
  char b[64];
  const std::uint64_t n = recent_n_ < recent_.size() ? recent_n_ : recent_.size();
  constexpr std::uint32_t kHead = 24;  // bytes shown per command
  for (std::uint64_t k = recent_n_ - n; k < recent_n_; ++k) {
    const Recent& r = recent_[k & (recent_.size() - 1)];
    std::snprintf(b, sizeof b, "  @%llu len %u:", static_cast<unsigned long long>(r.offset), r.len);
    out += b;
    const std::uint32_t h = r.len < kHead ? r.len : kHead;
    if (r.offset + kHistory < pushed_ || r.offset + h > pushed_) {
      out += " (bytes no longer kept)\n";
      continue;
    }
    for (std::uint32_t i = 0; i < h; ++i) {
      std::snprintf(b, sizeof b, " %02X", history_[(r.offset + i) & (kHistory - 1)]);
      out += b;
    }
    out += r.len > kHead ? " ...\n" : "\n";
  }
  out += "  lost at:";
  for (std::uint8_t v : broken_tail_) {
    std::snprintf(b, sizeof b, " %02X", v);
    out += b;
  }
  return out + "\n";
}

GxFifo::Effects GxFifo::push(const std::uint8_t* bytes, std::size_t n) {
  Effects fx;
  if (broken_) return fx;
  // Q4: the stream's last bytes, for the report (broken_context).
  if (n == 32 && !(pushed_ & 31)) {  // a burst: a copy of fixed size
    std::memcpy(history_.data() + (pushed_ & (kHistory - 1)), bytes, 32);
  } else {
    for (std::size_t k = n > kHistory ? n - kHistory : 0; k < n;) {
      const std::size_t at = static_cast<std::size_t>((pushed_ + k) & (kHistory - 1));
      const std::size_t run = std::min(n - k, kHistory - at);
      std::memcpy(history_.data() + at, bytes + k, run);
      k += run;
    }
  }
  pushed_ += n;
  // Q2: with nothing pending (the usual case: a burst holds whole commands and
  // perhaps the start of one more), the commands are parsed where the bytes are
  // and only an incomplete tail is kept; otherwise the bytes join what is pending.
  const std::uint8_t* p = bytes;
  std::size_t avail = n;
  if (pend_ != pbeg_) {
    pending_append(bytes, n);
    if (need_ > pend_ - pbeg_) return fx;  // the command at the head is still incomplete
    p = pending_.data() + pbeg_;
    avail = pend_ - pbeg_;
  }
  need_ = 0;
  std::size_t at = 0;
  while (at < avail) {
    std::size_t len = command_length(p + at, avail - at);
    if (len == ~std::size_t(0)) {
      if (observer_) flush_commands();  // the ones before it, while their bytes are still there
      broken_ = true;
      broken_opcode_ = p[at];
      broken_offset_ = consumed_;
      broken_tail_.assign(p + at, p + std::min(avail, at + 64));
      pbeg_ = pend_ = 0;
      return fx;
    }
    if (len == 0 || len > avail - at) {  // the rest of the command is still to come
      if (len) need_ = len;  // its length, from the head of the pending bytes kept below
      break;
    }
    Recent& r = recent_[recent_n_++ & (recent_.size() - 1)];
    r.offset = consumed_;
    r.len = static_cast<std::uint32_t>(len);
    execute(p + at, len, fx, false);
    consumed_ += len;
    at += len;
  }
  if (observer_) flush_commands();  // before the pending bytes change: some of them point into them
  if (p == bytes) {
    pbeg_ = pend_ = 0;
    if (at < n) pending_append(bytes + at, n - at);
  } else {
    pbeg_ += at;
    if (pbeg_ == pend_) pbeg_ = pend_ = 0;
  }
  return fx;
}

void GxFifo::pending_append(const std::uint8_t* bytes, std::size_t n) {
  if (pend_ + n > pending_.size()) {
    if (pbeg_) {  // move what is pending to the front first
      std::memmove(pending_.data(), pending_.data() + pbeg_, pend_ - pbeg_);
      pend_ -= pbeg_;
      pbeg_ = 0;
    }
    if (pend_ + n > pending_.size()) pending_.resize(std::max(2 * pending_.size(), pend_ + n));
  }
  std::memcpy(pending_.data() + pend_, bytes, n);
  pend_ += n;
}

template <class Ar>
void GxFifo::state_fields(Ar& a) {
  a(need_);
  a(consumed_);
  a(vcd_lo_);
  a(vcd_hi_);
  a(vat_a_);
  a(vat_b_);
  a(vat_c_);
  a(commands_);
  a(bp_loads_);
  a(primitives_);
  a(vertices_);
  a(display_lists_);
  a(display_list_errors_);
  a(broken_);
  a(broken_opcode_);
  a(broken_offset_);
}

void GxFifo::write_state(StateWriter& w) const {
  w(std::vector<std::uint8_t>(pending_.begin() + static_cast<std::ptrdiff_t>(pbeg_),
                              pending_.begin() + static_cast<std::ptrdiff_t>(pend_)));
  const_cast<GxFifo*>(this)->state_fields(w);  // the writer only reads the fields
}

void GxFifo::read_state(StateReader& r) {
  std::vector<std::uint8_t> pending;
  r(pending);
  pending_.assign(std::max<std::size_t>(4096, pending.size()), 0);
  std::copy(pending.begin(), pending.end(), pending_.begin());
  pbeg_ = 0;
  pend_ = pending.size();
  state_fields(r);
  vsize_valid_ = 0;
  batch_n_ = 0;
}

}  // namespace mvp::rt
