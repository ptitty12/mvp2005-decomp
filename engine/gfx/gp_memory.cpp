#include "gfx/gp_memory.h"

#include <algorithm>

namespace mvp::gfx {

void GpMemory::attach(const std::uint8_t* live, std::uint32_t size) {
  ++version_;
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
  ++version_;
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
  ++version_;
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
  ++version_;
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
  ++version_;
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

bool GpMemory::window(std::uint32_t addr, std::uint32_t len, Window& w) const {
  addr &= 0x01FFFFFFu;
  if (!live_ || !len || addr > size_ || len > size_ - addr) return false;
  const std::uint32_t page = addr >> kPageShift;
  if (((addr + len - 1) >> kPageShift) != page) return false;  // view() looks at each page: one page per window
  const std::uint32_t page_lo = page << kPageShift;
  const std::uint32_t page_hi = std::min(page_lo + (1u << kPageShift), size_);
  const bool copies = shadows_ && shadow_page_[page];         // view() takes shadowed()
  const bool snapshots = any_active_ && page_gen_[page] == gen_;  // plain_view() takes overlay()
  if (!copies && !snapshots) {
    w = {page_lo, page_hi, live_, version_};
    return true;
  }
  // 64-byte entries (a copy's unit; two 32-byte lines each) of the page and of the request.
  const std::uint32_t pe0 = page_lo >> kShadowShift, pe1 = (page_hi - 1) >> kShadowShift;
  std::uint32_t e0 = addr >> kShadowShift, e1 = (addr + len - 1) >> kShadowShift;
  auto copied = [&](std::uint32_t e) { return copies && shadow_valid_[e] != 0; };
  auto plain = [&](std::uint32_t e) {  // no copy, and no snapshot over either line: memory as it is
    if (copied(e)) return false;
    const std::uint32_t l = e << (kShadowShift - kLineShift);
    return !snapshots || (!active_line(l) && !active_line(l + 1));
  };
  bool all_copied = true;
  for (std::uint32_t e = e0; e <= e1; ++e) all_copied = all_copied && copied(e);
  if (all_copied) {  // shadowed(): the copy itself
    while (e0 > pe0 && copied(e0 - 1)) --e0;
    while (e1 < pe1 && copied(e1 + 1)) ++e1;
    w = {e0 << kShadowShift, std::min((e1 + 1) << kShadowShift, page_hi), shadow_.data(), version_};
    return true;
  }
  for (std::uint32_t e = e0; e <= e1; ++e)
    if (!plain(e)) return false;  // a copy or a snapshot under part of it: view() composes one
  while (e0 > pe0 && plain(e0 - 1)) --e0;
  while (e1 < pe1 && plain(e1 + 1)) ++e1;
  w = {e0 << kShadowShift, std::min((e1 + 1) << kShadowShift, page_hi), live_, version_};
  return true;
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
