// The RVZ packer: turns a plain disc image into an RVZ image, so one can be made
// without Dolphin, and so the reader's round-trip is testable without game data.
//
// Layout written (the format lets the parts sit in any order): the 0x124-byte
// header, then the chunk data, then the compressed raw-data table and group table.
// Sizes are measured in a first pass so the output can be streamed in one go with
// no seeking, at the cost of compressing each chunk twice.
#include "vfs/rvz.h"

#include <zstd.h>

#include <algorithm>
#include <cstring>
#include <vector>

#include "common/bytes.h"
#include "common/sha1.h"

namespace mvp::vfs::rvz {
namespace {

constexpr std::size_t kHeadSize = 0x48;  // wia_file_head_t
constexpr std::size_t kDiscSize = 0xDC;  // wia_disc_t
constexpr std::uint32_t kVersion = 0x01000000u;
constexpr std::uint32_t kVersionCompatible = 0x00030000u;  // RVZ_VERSION_WRITE_COMPATIBLE
constexpr std::uint32_t kCompressionZstd = 5;
constexpr std::uint32_t kCompressionNone = 0;

std::size_t align4(std::size_t x) { return (x + 3) & ~std::size_t(3); }

// What a chunk's group entry will say.
struct Slot {
  bool compressed = false;
  std::uint32_t stored = 0;  // bytes in the file; 0 means "all zero"
};

bool compress_chunk(const std::uint8_t* in, std::size_t in_size, int level,
                    std::vector<std::uint8_t>* out, std::string* error) {
  out->resize(ZSTD_compressBound(in_size));
  const std::size_t n = ZSTD_compress(out->data(), out->size(), in, in_size, level);
  if (ZSTD_isError(n)) {
    if (error) *error = std::string("RVZ: zstd: ") + ZSTD_getErrorName(n);
    return false;
  }
  out->resize(n);
  return true;
}

bool compress_table(const std::vector<std::uint8_t>& in, int level, std::vector<std::uint8_t>* out,
                    std::string* error) {
  return compress_chunk(in.data(), in.size(), level, out, error);
}

// Measures one chunk: all-zero, compressible, or stored raw.
bool measure(Source& iso, std::uint64_t off, std::size_t size, int level, Slot* slot,
             std::string* error) {
  std::vector<std::uint8_t> chunk(size);
  if (!iso.read(off, chunk.data(), chunk.size())) {
    if (error) *error = "RVZ: reading the disc failed";
    return false;
  }
  bool any = false;
  for (std::uint8_t b : chunk) {
    if (b) {
      any = true;
      break;
    }
  }
  if (!any) {
    slot->stored = 0;
    slot->compressed = false;
    return true;
  }
  std::vector<std::uint8_t> packed;
  if (level != INT32_MIN) {
    if (!compress_chunk(chunk.data(), chunk.size(), level, &packed, error)) return false;
    if (packed.size() < chunk.size()) {
      slot->compressed = true;
      slot->stored = static_cast<std::uint32_t>(packed.size());
      return true;
    }
  }
  slot->compressed = false;
  slot->stored = static_cast<std::uint32_t>(chunk.size());
  return true;
}

}  // namespace

bool pack(Source& iso, const std::function<bool(const std::uint8_t*, std::size_t)>& sink,
          std::uint32_t chunk_size, int level, std::string* error) {
  auto fail = [&](const std::string& why) -> bool {
    if (error) *error = why;
    return false;
  };
  const std::uint64_t iso_size = iso.size();
  if (iso_size == 0) return fail("RVZ: nothing to pack");
  if (chunk_size < 0x8000) return fail("RVZ: chunk size is below 32 KiB");
  if (chunk_size < RvzSource::kDefaultChunkSize) {
    if ((chunk_size & (chunk_size - 1)) != 0) return fail("RVZ: chunk size is not a power of two");
  } else if (chunk_size % RvzSource::kDefaultChunkSize != 0) {
    return fail("RVZ: chunk size is not a multiple of 2 MiB");
  }

  const std::uint32_t n_groups =
      static_cast<std::uint32_t>((iso_size + chunk_size - 1) / chunk_size);
  if (n_groups == 0) return fail("RVZ: no chunks");
  const bool use_zstd = level != INT32_MIN;

  // Pass one: measure every chunk and lay the file out.
  std::vector<Slot> slots(n_groups);
  std::vector<std::uint64_t> file_off(n_groups);
  std::uint64_t at = kHeadSize + kDiscSize;
  for (std::uint32_t i = 0; i < n_groups; ++i) {
    const std::uint64_t start = std::uint64_t(i) * chunk_size;
    const std::size_t size =
        static_cast<std::size_t>(std::min<std::uint64_t>(chunk_size, iso_size - start));
    if (!measure(iso, start, size, level, &slots[i], error)) return false;
    at = align4(at);
    file_off[i] = at;
    at += slots[i].stored;
  }

  // The raw-data table: one range covering the whole disc.
  std::vector<std::uint8_t> raw_table(24, 0);
  put_be64(raw_table.data(), 0);
  put_be64(raw_table.data() + 8, iso_size);
  put_be32(raw_table.data() + 16, 0);
  put_be32(raw_table.data() + 20, n_groups);

  // The group table.
  std::vector<std::uint8_t> group_table(std::size_t(n_groups) * 12, 0);
  for (std::uint32_t i = 0; i < n_groups; ++i) {
    std::uint8_t* e = group_table.data() + std::size_t(i) * 12;
    put_be32(e, static_cast<std::uint32_t>(file_off[i] >> 2));
    const std::uint32_t flag = (slots[i].compressed ? 0x80000000u : 0u);
    put_be32(e + 4, flag | slots[i].stored);
    put_be32(e + 8, 0);  // no RVZ packing
  }

  std::vector<std::uint8_t> raw_stored, group_stored;
  if (use_zstd) {
    if (!compress_table(raw_table, level, &raw_stored, error)) return false;
    if (!compress_table(group_table, level, &group_stored, error)) return false;
  } else {
    raw_stored = raw_table;
    group_stored = group_table;
  }

  const std::uint64_t raw_off = align4(at);
  const std::uint64_t group_off = align4(raw_off + raw_stored.size());
  const std::uint64_t total = group_off + group_stored.size();
  if (raw_off > 0xFFFFFFFFull || group_off > 0xFFFFFFFFull)
    return fail("RVZ: the image is too large for the format's 32-bit offsets");

  // The header.
  std::vector<std::uint8_t> head(kHeadSize + kDiscSize, 0);
  std::memcpy(head.data(), RvzSource::kMagic, sizeof RvzSource::kMagic);
  put_be32(head.data() + 0x04, kVersion);
  put_be32(head.data() + 0x08, kVersionCompatible);
  put_be32(head.data() + 0x0C, static_cast<std::uint32_t>(kDiscSize));
  put_be64(head.data() + 0x24, iso_size);
  put_be64(head.data() + 0x2C, total);

  std::uint8_t* d = head.data() + kHeadSize;
  put_be32(d + 0x00, 1);  // GameCube
  put_be32(d + 0x04, use_zstd ? kCompressionZstd : kCompressionNone);
  put_be32(d + 0x08, static_cast<std::uint32_t>(level));
  put_be32(d + 0x0C, chunk_size);
  // dhead: the disc's first 0x80 bytes.
  if (!iso.read(0, d + 0x10, 0x80)) return fail("RVZ: reading the disc header failed");
  put_be32(d + 0x90, 0);  // no partitions
  put_be32(d + 0x94, 0);
  put_be64(d + 0x98, 0);
  put_be32(d + 0xB4, 1);  // one raw-data range
  put_be64(d + 0xB8, raw_off);
  put_be32(d + 0xC0, static_cast<std::uint32_t>(raw_stored.size()));
  put_be32(d + 0xC4, n_groups);
  put_be64(d + 0xC8, group_off);
  put_be32(d + 0xD0, static_cast<std::uint32_t>(group_stored.size()));
  d[0xD4] = 0;  // compr_data_len, no compressor-specific data for zstd

  // Both hashes the format records (WiaAndRvz.md). Dolphin does not check them on
  // read, but a file that disagrees with its own hashes is a bug worth not writing.
  {
    Sha1 disc_hash;
    disc_hash.update(d, kDiscSize);
    const auto digest = disc_hash.finish();
    std::memcpy(head.data() + 0x10, digest.data(), digest.size());
    Sha1 head_hash;
    head_hash.update(head.data(), 0x34);
    const auto h = head_hash.finish();
    std::memcpy(head.data() + 0x34, h.data(), h.size());
  }

  // Write: header, then the chunk data, then the tables.
  if (!sink(head.data(), head.size())) return fail("RVZ: writing the header failed");
  std::vector<std::uint8_t> chunk;
  std::size_t written = head.size();
  for (std::uint32_t i = 0; i < n_groups; ++i) {
    const std::size_t pad = static_cast<std::size_t>(file_off[i] - written);
    if (pad) {
      const std::vector<std::uint8_t> zeros(pad, 0);
      if (!sink(zeros.data(), zeros.size())) return fail("RVZ: writing failed");
      written += pad;
    }
    if (slots[i].stored == 0) continue;
    const std::uint64_t start = std::uint64_t(i) * chunk_size;
    const std::size_t size =
        static_cast<std::size_t>(std::min<std::uint64_t>(chunk_size, iso_size - start));
    if (slots[i].compressed) {
      std::vector<std::uint8_t> src(size);
      if (!iso.read(start, src.data(), src.size())) return fail("RVZ: reading the disc failed");
      if (!compress_chunk(src.data(), src.size(), level, &chunk, error)) return false;
      if (chunk.size() != slots[i].stored)
        return fail("RVZ: compression is not deterministic between passes");
    } else {
      chunk.assign(size, 0);
      if (!iso.read(start, chunk.data(), chunk.size())) return fail("RVZ: reading the disc failed");
    }
    if (!sink(chunk.data(), chunk.size())) return fail("RVZ: writing failed");
    written += chunk.size();
  }
  for (const auto* table : {&raw_stored, &group_stored}) {
    const std::size_t pad = static_cast<std::size_t>(
        (table == &raw_stored ? raw_off : group_off) - written);
    if (pad) {
      const std::vector<std::uint8_t> zeros(pad, 0);
      if (!sink(zeros.data(), zeros.size())) return fail("RVZ: writing failed");
      written += pad;
    }
    if (!sink(table->data(), table->size())) return fail("RVZ: writing failed");
    written += table->size();
  }
  return true;
}

std::vector<std::uint8_t> pack_to_vector(Source& iso, std::uint32_t chunk_size, int level,
                                         std::string* error) {
  std::vector<std::uint8_t> out;
  if (!pack(iso, [&](const std::uint8_t* p, std::size_t n) {
        out.insert(out.end(), p, p + n);
        return true;
      }, chunk_size, level, error)) {
    out.clear();
  }
  return out;
}

}  // namespace mvp::vfs::rvz
