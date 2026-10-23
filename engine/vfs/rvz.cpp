#include "vfs/rvz.h"

#include <zstd.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <utility>

#include "common/bytes.h"

namespace mvp::vfs {
namespace {

constexpr std::size_t kHeaderSize = 0x124;   // wia_file_head_t + wia_disc_t
constexpr std::size_t kRawEntrySize = 24;    // wia_raw_data_t
constexpr std::size_t kGroupEntrySize = 12;  // rvz_group_t
constexpr std::uint64_t kSector = 0x8000;    // VolumeWii::BLOCK_TOTAL_SIZE

std::string hex32(std::uint32_t v) {
  char b[16];
  std::snprintf(b, sizeof b, "0x%08x", v);
  return b;
}

// The GameCube/Wii padding PRNG: a lagged Fibonacci generator (xor, 32, 521).
// WiaAndRvz.md "PRNG algorithm"; the byte order of an output word is the
// document's (the second byte shifts by 18, not 16).
class PaddingPrng {
 public:
  explicit PaddingPrng(const std::uint8_t* seed) {
    for (std::size_t i = 0; i < 17; ++i) buffer_[i] = be32(seed + 4 * i);
    for (std::size_t i = 17; i < kWords; ++i)
      buffer_[i] = (buffer_[i - 17] << 23) ^ (buffer_[i - 16] >> 9) ^ buffer_[i - 1];
    for (int i = 0; i < 4; ++i) advance();
  }

  void skip(std::uint64_t bytes) {
    while (bytes >= 4) {
      next();
      bytes -= 4;
    }
  }

  void emit(std::uint8_t* out, std::size_t n) {
    std::size_t i = 0;
    while (i < n) {
      if (left_ == 0) {
        advance();
        left_ = kWords;
      }
      const std::uint32_t w = buffer_[pos_++];
      --left_;
      // The word's four bytes in the documented order: the second byte shifts by
      // 18, not 16 (WiaAndRvz.md reproduces the console's padding exactly).
      if (i < n) out[i++] = static_cast<std::uint8_t>(w >> 24);
      if (i < n) out[i++] = static_cast<std::uint8_t>(w >> 18);
      if (i < n) out[i++] = static_cast<std::uint8_t>(w >> 8);
      if (i < n) out[i++] = static_cast<std::uint8_t>(w);
    }
  }

 private:
  static constexpr std::size_t kWords = 521;

  void next() {
    if (left_ == 0) {
      advance();
      left_ = kWords;
    }
    ++pos_;
    --left_;
  }

  void advance() {
    for (std::size_t i = 0; i < 32; ++i) buffer_[i] ^= buffer_[i + kWords - 32];
    for (std::size_t i = 32; i < kWords; ++i) buffer_[i] ^= buffer_[i - 32];
    pos_ = 0;
    left_ = kWords;
  }

  std::uint32_t buffer_[kWords] = {};
  std::size_t pos_ = 0;
  std::size_t left_ = 0;
};

// Decodes the RVZ packing scheme (runs of literal bytes and PRNG-generated runs)
// into exactly `out_size` bytes. `disc_off` is the absolute disc offset of the
// data, which the PRNG needs when it does not start on a 32 KiB boundary.
bool decode_packing(const std::uint8_t* in, std::size_t in_size, std::uint64_t disc_off,
                    std::uint8_t* out, std::size_t out_size, std::string* error) {
  std::size_t i = 0, o = 0;
  while (o < out_size) {
    if (i + 4 > in_size) {
      if (error) *error = "RVZ: packing header past the group";
      return false;
    }
    std::uint32_t size = be32(in + i);
    i += 4;
    if ((size & 0x80000000u) == 0) {
      if (i + size > in_size || o + size > out_size) {
        if (error) *error = "RVZ: packing literal past the group";
        return false;
      }
      std::memcpy(out + o, in + i, size);
      i += size;
      o += size;
    } else {
      size &= 0x7FFFFFFFu;
      if (i + 68 > in_size || o + size > out_size) {
        if (error) *error = "RVZ: packing PRNG run past the group";
        return false;
      }
      PaddingPrng prng(in + i);
      i += 68;
      prng.skip(disc_off % kSector);
      prng.emit(out + o, size);
      o += size;
    }
  }
  return true;
}

bool zstd_decompress(const std::uint8_t* in, std::size_t in_size, std::uint8_t* out,
                     std::size_t out_size, std::string* error) {
  const std::size_t got = ZSTD_decompress(out, out_size, in, in_size);
  if (ZSTD_isError(got)) {
    if (error) *error = std::string("RVZ: zstd: ") + ZSTD_getErrorName(got);
    return false;
  }
  if (got != out_size) {
    if (error) *error = "RVZ: zstd gave " + std::to_string(got) + " of " + std::to_string(out_size);
    return false;
  }
  return true;
}

}  // namespace

std::unique_ptr<RvzSource> RvzSource::open(std::unique_ptr<Source> inner, std::string* error) {
  auto fail = [&](const std::string& why) -> std::unique_ptr<RvzSource> {
    if (error) *error = why;
    return nullptr;
  };
  if (!inner) return fail("RVZ: no source");

  std::vector<std::uint8_t> h(kHeaderSize);
  if (!inner->read(0, h.data(), h.size())) return fail("RVZ: header read failed");
  if (std::memcmp(h.data(), kMagic, sizeof kMagic) != 0) return fail("RVZ: bad magic");
  const std::uint32_t version = be32(h.data() + 0x04);
  const std::uint32_t version_compatible = be32(h.data() + 0x08);
  // We implement up to 1.0.0 and read files written for 0.3.0 (WiaAndRvz.md version
  // rules, Dolphin's Initialize: our version must reach the file's compatible
  // version, and the file's version must reach what we read).
  constexpr std::uint32_t kVersion = 0x01000000u;
  constexpr std::uint32_t kVersionReadCompatible = 0x00030000u;
  if (version_compatible > kVersion)
    return fail("RVZ: needs format version " + hex32(version_compatible));
  if (version < kVersionReadCompatible) return fail("RVZ: the format version is too old to read");

  const std::uint32_t disc_type = be32(h.data() + 0x48);
  if (disc_type != 1)
    return fail("RVZ: not a GameCube image (disc type " + std::to_string(disc_type) +
                (disc_type == 2 ? ", a Wii image: partition data is not implemented" : "") + ")");

  const std::uint32_t compression = be32(h.data() + 0x4C);
  const Method method = static_cast<Method>(compression);
  if (method != Method::None && method != Method::Zstd)
    return fail("RVZ: compression " + std::to_string(compression) +
                " is not implemented (only none and zstd)");

  const std::uint32_t chunk = be32(h.data() + 0x54);
  if (chunk < kSector) return fail("RVZ: chunk size is below 32 KiB");
  if (chunk < kDefaultChunkSize) {
    if ((chunk & (chunk - 1)) != 0) return fail("RVZ: chunk size is not a power of two");
  } else if (chunk % kDefaultChunkSize != 0) {
    return fail("RVZ: chunk size is not a multiple of 2 MiB");
  }

  const std::uint32_t n_part = be32(h.data() + 0xD8);
  if (n_part != 0)
    return fail("RVZ: " + std::to_string(n_part) + " partitions (Wii partition data is not implemented)");

  const std::uint64_t iso_size = be64(h.data() + 0x24);
  if (iso_size == 0) return fail("RVZ: zero disc size");

  const std::uint32_t n_raw = be32(h.data() + 0xFC);
  const std::uint64_t raw_off = be64(h.data() + 0x100);
  const std::uint32_t raw_size = be32(h.data() + 0x108);
  const std::uint32_t n_groups = be32(h.data() + 0x10C);
  const std::uint64_t group_off = be64(h.data() + 0x110);
  const std::uint32_t group_size = be32(h.data() + 0x118);
  if (n_raw == 0 || n_groups == 0) return fail("RVZ: no data entries");

  std::unique_ptr<RvzSource> s(new RvzSource());
  s->inner_ = std::move(inner);
  s->iso_size_ = iso_size;
  s->chunk_size_ = chunk;
  s->method_ = method;
  s->dhead_.assign(h.begin() + 0x58, h.begin() + 0xD8);

  // Both tables are stored compressed with the file's own method (WiaAndRvz.md;
  // Dolphin reads them through ReadCompressedData).
  auto read_table = [&](std::uint64_t off, std::uint32_t stored, std::size_t want,
                        std::vector<std::uint8_t>* out, const char* what) -> bool {
    if (stored == 0 || off + stored > s->inner_->size()) {
      if (error) *error = std::string("RVZ: the ") + what + " is outside the file";
      return false;
    }
    std::vector<std::uint8_t> raw(stored);
    if (!s->inner_->read(off, raw.data(), raw.size())) {
      if (error) *error = std::string("RVZ: the ") + what + " read failed";
      return false;
    }
    if (method == Method::None && raw.size() != want) {
      if (error) *error = std::string("RVZ: the ") + what + " has the wrong size";
      return false;
    }
    out->assign(want, 0);
    if (method == Method::None) {
      *out = std::move(raw);
      return true;
    }
    return zstd_decompress(raw.data(), raw.size(), out->data(), want, error);
  };

  std::vector<std::uint8_t> raw_table, group_table;
  if (!read_table(raw_off, raw_size, std::size_t(n_raw) * kRawEntrySize, &raw_table, "raw-data table"))
    return nullptr;
  if (!read_table(group_off, group_size, std::size_t(n_groups) * kGroupEntrySize, &group_table,
                  "group table"))
    return nullptr;

  s->raw_.reserve(n_raw);
  for (std::uint32_t i = 0; i < n_raw; ++i) {
    const std::uint8_t* e = raw_table.data() + std::size_t(i) * kRawEntrySize;
    RawRange r;
    r.off = be64(e);
    r.size = be64(e + 8);
    r.group0 = be32(e + 16);
    r.ngroups = be32(e + 20);
    if (r.size == 0) continue;
    // The format lets a range start unaligned; readers round it down to 32 KiB and
    // extend the size so the end stays put (WiaAndRvz.md on wia_raw_data_t, and
    // Dolphin's ReadFromGroups does exactly this).
    const std::uint64_t skipped = r.off % kSector;
    r.off -= skipped;
    r.size += skipped;
    if (std::uint64_t(r.group0) + r.ngroups > n_groups) return fail("RVZ: a raw-data range overruns the group table");
    if (r.off + r.size > iso_size) return fail("RVZ: a raw-data range is outside the disc");
    s->raw_.push_back(r);
  }
  if (s->raw_.empty()) return fail("RVZ: no raw-data ranges");

  s->groups_.reserve(n_groups);
  for (std::uint32_t i = 0; i < n_groups; ++i) {
    const std::uint8_t* e = group_table.data() + std::size_t(i) * kGroupEntrySize;
    Group g;
    g.file_off = std::uint64_t(be32(e)) << 2;
    const std::uint32_t data_size = be32(e + 4);
    g.compressed = (data_size & 0x80000000u) != 0;
    g.stored = data_size & 0x7FFFFFFFu;
    g.packed = be32(e + 8);
    if (g.stored != 0 && g.file_off + g.stored > s->inner_->size())
      return fail("RVZ: a group is outside the file");
    s->groups_.push_back(g);
  }
  return s;
}

bool RvzSource::load_group(std::uint32_t index, std::uint64_t group_bytes,
                           std::vector<std::uint8_t>* out, std::string* error) const {
  if (index >= groups_.size()) {
    if (error) *error = "RVZ: group index out of range";
    return false;
  }
  const Group& g = groups_[index];
  if (g.stored == 0) {
    out->assign(std::size_t(group_bytes), 0);
    return true;
  }
  std::vector<std::uint8_t> stored(g.stored);
  if (!inner_->read(g.file_off, stored.data(), stored.size())) {
    if (error) *error = "RVZ: group read failed";
    return false;
  }
  if (!g.compressed || method_ == Method::None) {
    if (stored.size() < group_bytes) {
      if (error) *error = "RVZ: a stored group is shorter than the data it covers";
      return false;
    }
    stored.resize(std::size_t(group_bytes));
    *out = std::move(stored);
    return true;
  }
  std::vector<std::uint8_t> plain(std::size_t(g.packed ? g.packed : group_bytes));
  if (!zstd_decompress(stored.data(), stored.size(), plain.data(), plain.size(), error)) return false;
  if (g.packed == 0) {
    *out = std::move(plain);
    return true;
  }
  out->assign(std::size_t(group_bytes), 0);
  // The packing PRNG wants the disc offset of the data it generates.
  std::uint64_t disc_off = 0;
  for (const RawRange& r : raw_) {
    const std::uint64_t first = r.group0, last = r.group0 + r.ngroups;
    if (index >= first && index < last) {
      disc_off = r.off + (std::uint64_t(index) - first) * chunk_size_;
      break;
    }
  }
  return decode_packing(plain.data(), plain.size(), disc_off, out->data(), out->size(), error);
}

bool RvzSource::read(std::uint64_t off, void* dst, std::size_t n) {
  if (off > iso_size_ || n > iso_size_ - off) return false;
  std::uint8_t* out = static_cast<std::uint8_t*>(dst);
  std::string error;
  while (n) {
    // wia_disc_t.dhead carries the disc's first 0x80 bytes.
    if (off < dhead_.size()) {
      const std::size_t take = std::min<std::size_t>(n, dhead_.size() - std::size_t(off));
      std::memcpy(out, dhead_.data() + std::size_t(off), take);
      out += take;
      off += take;
      n -= take;
      continue;
    }
    const RawRange* range = nullptr;
    for (const RawRange& r : raw_) {
      if (off >= r.off && off < r.off + r.size) {
        range = &r;
        break;
      }
    }
    if (range == nullptr) return false;
    const std::uint64_t local = off - range->off;
    const std::uint64_t chunk = local / chunk_size_;
    const std::uint32_t index = range->group0 + std::uint32_t(chunk);
    const std::uint64_t group_start = range->off + chunk * chunk_size_;
    const std::uint64_t group_bytes =
        std::min<std::uint64_t>(chunk_size_, range->off + range->size - group_start);
    const std::size_t in_group = std::size_t(off - group_start);
    const std::size_t take = std::size_t(std::min<std::uint64_t>(group_bytes - in_group, n));

    if (cached_group_ != index) {
      if (!load_group(index, group_bytes, &cached_bytes_, &error)) return false;
      cached_group_ = index;
    }
    std::memcpy(out, cached_bytes_.data() + in_group, take);
    out += take;
    off += take;
    n -= take;
  }
  return true;
}

}  // namespace mvp::vfs
