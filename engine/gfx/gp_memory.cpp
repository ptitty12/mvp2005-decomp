#include "gfx/gp_memory.h"

#include <algorithm>

namespace mvp::gfx {

void GpMemory::attach(const std::uint8_t* live, std::uint32_t size) {
  live_ = live;
  size_ = size & ~((1u << kLineShift) - 1);
  lines_ = size_ >> kLineShift;
  snap_.assign(size_, 0);
  gen_of_.assign(lines_, 0);
  page_gen_.assign((size_ >> kPageShift) + 1, 0);
  any_active_ = false;
  shadow_.clear();
  shadow_valid_.clear();
  shadow_page_.clear();
  shadows_ = 0;
}

void GpMemory::shadow(std::uint32_t addr, std::uint32_t len, const std::uint8_t* data) {
  addr &= 0x01FFFFFFu;
  if (!live_ || !len || addr >= size_ || len > size_ - addr) return;
  if (shadow_.empty()) {
    shadow_.assign(size_, 0);
    shadow_valid_.assign(size_ >> kShadowShift, 0);
    shadow_page_.assign((size_ >> kPageShift) + 1, 0);
  }
  std::memcpy(shadow_.data() + addr, data, len);
  for (std::uint32_t k = addr >> kShadowShift, e = (addr + len - 1) >> kShadowShift; k <= e; ++k)
    if (!shadow_valid_[k]) {
      shadow_valid_[k] = 1;
      ++shadow_page_[k >> (kPageShift - kShadowShift)];
      ++shadows_;
    }
  ++stats_.shadows;
  stats_.shadow_bytes += len;
}

void GpMemory::end_shadow(std::uint32_t addr, std::uint32_t len) {
  addr &= 0x01FFFFFFu;
  if (!shadows_ || !len || addr >= size_) return;
  const std::uint32_t last = std::min(addr + len - 1, size_ - 1);
  for (std::uint32_t k = addr >> kShadowShift, e = last >> kShadowShift; k <= e; ++k)
    if (shadow_valid_[k]) {
      shadow_valid_[k] = 0;
      --shadow_page_[k >> (kPageShift - kShadowShift)];
      --shadows_;
    }
}

// Q6: a view over memory where some entries hold copies: the copy itself when
// they cover all of it, else the view without them with the copies laid over.
const std::uint8_t* GpMemory::shadowed(std::uint32_t addr, std::uint32_t len, int slot) {
  const std::uint32_t first = addr >> kShadowShift, last = (addr + len - 1) >> kShadowShift;
  std::uint32_t k = first;
  while (k <= last && shadow_valid_[k]) ++k;
  if (k > last) {
    ++stats_.shadow_views;
    return shadow_.data() + addr;
  }
  k = first;
  while (k <= last && !shadow_valid_[k]) ++k;
  const std::uint8_t* base = plain_view(addr, len, slot);
  if (k > last) return base;
  std::vector<std::uint8_t>& out = scratch_[slot & 1];
  if (base != out.data()) {
    if (out.size() < len) out.resize(len);
    std::memcpy(out.data(), base, len);
  }
  ++stats_.shadow_views;
  for (; k <= last; ++k) {
    if (!shadow_valid_[k]) continue;
    const std::uint32_t a = std::max(addr, k << kShadowShift);
    const std::uint32_t e = std::min(addr + len, (k + 1) << kShadowShift);
    std::memcpy(out.data() + (a - addr), shadow_.data() + a, e - a);
  }
  return out.data();
}

void GpMemory::mark(std::uint32_t first_line, std::uint32_t last_line) {
  for (std::uint32_t l = first_line; l <= last_line; ++l) gen_of_[l] = gen_;
  for (std::uint32_t p = first_line >> (kPageShift - kLineShift), e = last_line >> (kPageShift - kLineShift); p <= e; ++p)
    page_gen_[p] = gen_;
  any_active_ = true;
}

void GpMemory::flush(std::uint32_t addr, std::uint32_t len) {
  addr &= 0x01FFFFFFu;
  if (!live_ || !len || addr >= size_) return;
  const std::uint32_t first = addr >> kLineShift;
  const std::uint32_t last = std::min((addr + len - 1) >> kLineShift, lines_ - 1);
  const std::uint32_t a = first << kLineShift, n = (last - first + 1) << kLineShift;
  std::memcpy(snap_.data() + a, live_ + a, n);
  mark(first, last);
  ++stats_.flushes;
  stats_.flushed_bytes += n;
}

void GpMemory::activate(std::uint32_t addr, std::uint32_t len) {
  addr &= 0x01FFFFFFu;
  if (!live_ || !len || addr >= size_) return;
  const std::uint32_t first = addr >> kLineShift;
  const std::uint32_t last = std::min((addr + len - 1) >> kLineShift, lines_ - 1);
  mark(first, last);
  ++stats_.flushes;
  stats_.flushed_bytes += (last - first + 1) << kLineShift;
}

void GpMemory::set_lines(std::uint32_t first_line, std::uint32_t lines, const std::uint8_t* data) {
  if (first_line >= lines_) return;
  lines = std::min(lines, lines_ - first_line);
  std::memcpy(snap_.data() + (static_cast<std::size_t>(first_line) << kLineShift), data,
              static_cast<std::size_t>(lines) << kLineShift);
}

void GpMemory::write_through(std::uint32_t addr, std::uint32_t len) {
  addr &= 0x01FFFFFFu;
  if (!any_active_ || !len || addr >= size_) return;
  const std::uint32_t first = addr >> kLineShift;
  const std::uint32_t last = std::min((addr + len - 1) >> kLineShift, lines_ - 1);
  for (std::uint32_t p = first >> (kPageShift - kLineShift), e = last >> (kPageShift - kLineShift); p <= e; ++p) {
    if (page_gen_[p] != gen_) continue;  // nothing flushed in this page this field
    const std::uint32_t l0 = std::max(first, p << (kPageShift - kLineShift));
    const std::uint32_t l1 = std::min(last, ((p + 1) << (kPageShift - kLineShift)) - 1);
    for (std::uint32_t l = l0; l <= l1; ++l)
      if (gen_of_[l] == gen_) {
        gen_of_[l] = 0;
        ++stats_.drops;
      }
  }
}

const std::uint8_t* GpMemory::overlay(std::uint32_t addr, std::uint32_t len, int slot) {
  const std::uint32_t first = addr >> kLineShift, last = (addr + len - 1) >> kLineShift;
  std::uint32_t l = first;
  while (l <= last && !active_line(l)) ++l;
  if (l > last) return live_ + addr;  // the page had a flush, these lines not
  std::vector<std::uint8_t>& out = scratch_[slot & 1];
  if (out.size() < len) out.resize(len);
  std::memcpy(out.data(), live_ + addr, len);
  ++stats_.overlays;
  for (; l <= last; ++l) {
    if (!active_line(l)) continue;
    const std::uint32_t a = std::max(addr, l << kLineShift);
    const std::uint32_t e = std::min(addr + len, (l + 1) << kLineShift);
    std::memcpy(out.data() + (a - addr), snap_.data() + a, e - a);
    ++stats_.overlaid_lines;
  }
  return out.data();
}

}  // namespace mvp::gfx
