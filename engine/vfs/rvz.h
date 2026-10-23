// RVZ: Dolphin's compressed disc image container. A player's own disc image is
// smaller on disk this way, and the engine reads it directly through Source, so
// everything above the disc layer is unchanged.
//
// The format is WIA v1 with the RVZ magic ("RVZ\x1"), a 12-byte rvz_group_t table
// and Zstandard as the compression method. It is documented upstream in Dolphin's
// docs/WiaAndRvz.md; this is an implementation from that document (Dolphin is a
// reference, never a source: docs/REUSE.md, CLAUDE.md).
//
// GameCube discs only for now: disc_type 1 and no partitions. Wii partition data
// (hash exceptions and AES keys) and the bzip2/LZMA/LZMA2 methods are refused with
// a reason rather than guessed at. RVZ packing (the pseudorandom-padding encoding)
// is decoded, since a Dolphin-written file uses it, but the packer never emits it.
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "vfs/source.h"

namespace mvp::vfs {

class RvzSource final : public Source {
 public:
  // Returns null (with the reason in *error) if `inner` is not a GameCube RVZ image.
  static std::unique_ptr<RvzSource> open(std::unique_ptr<Source> inner, std::string* error = nullptr);

  std::uint64_t size() const override { return iso_size_; }
  bool read(std::uint64_t off, void* dst, std::size_t n) override;

  // The four bytes at offset 0: 'R','V','Z',0x01 (the magic "RVZ\x1" of
  // WiaAndRvz.md; Dolphin's RVZ_MAGIC 0x015A5652 is that byte string read little
  // endian, so the file's bytes are compared directly).
  static constexpr std::uint8_t kMagic[4] = {'R', 'V', 'Z', 0x01};

  // Compression methods of wia_disc_t.compression. None and Zstd are implemented.
  enum class Method : std::uint32_t { None = 0, Bzip2 = 2, Lzma = 3, Lzma2 = 4, Zstd = 5 };

  // The chunk size a packed image uses by default (the format's maximum, and what
  // Dolphin writes: larger chunks compress better and cost decompression latency).
  static constexpr std::uint32_t kDefaultChunkSize = 0x200000u;

 private:
  RvzSource() = default;

  // One wia_raw_data_t after the format's offset rounding: the region of the disc
  // it covers, and the group entries that hold its chunks.
  struct RawRange {
    std::uint64_t off = 0;
    std::uint64_t size = 0;
    std::uint32_t group0 = 0;
    std::uint32_t ngroups = 0;
  };

  // One rvz_group_t: where its stored bytes are, and whether they are compressed.
  struct Group {
    std::uint64_t file_off = 0;
    std::uint32_t stored = 0;  // lower 31 bits of data_size: bytes in the file (0 = all zero)
    bool compressed = false;   // most significant bit of data_size
    std::uint32_t packed = 0;  // rvz_packed_size: 0 = RVZ packing not used
  };

  // Decompresses group `index`, whose decompressed size is `group_bytes`.
  bool load_group(std::uint32_t index, std::uint64_t group_bytes, std::vector<std::uint8_t>* out,
                  std::string* error) const;

  std::unique_ptr<Source> inner_;
  std::uint64_t iso_size_ = 0;
  std::uint32_t chunk_size_ = 0;
  Method method_ = Method::None;
  std::vector<std::uint8_t> dhead_;  // wia_disc_t.dhead: the disc's first 0x80 bytes
  std::vector<RawRange> raw_;
  std::vector<Group> groups_;

  // One-group cache: reads walk the disc forwards (the DOL, then the FST, then the
  // data a scene loads), so the previous chunk is almost always the next one asked
  // for. Mutable because read() is const-eligible on a Source.
  mutable std::uint32_t cached_group_ = 0xFFFFFFFFu;
  mutable std::vector<std::uint8_t> cached_bytes_;
};

namespace rvz {

// Packs a plain disc image into an RVZ stream: GameCube layout, one raw-data range
// covering the whole disc, chunks of `chunk_size` bytes. `sink` receives the file's
// bytes in order. `level` is a Zstandard level (negative is legal); `method` None
// stores every chunk raw.
//
// The packer exists so an image can be produced without Dolphin, and so the
// round-trip is testable without game data. It never emits RVZ packing.
bool pack(Source& iso, const std::function<bool(const std::uint8_t*, std::size_t)>& sink,
          std::uint32_t chunk_size, int level, std::string* error = nullptr);

// The same, into memory (tests and small images).
std::vector<std::uint8_t> pack_to_vector(Source& iso, std::uint32_t chunk_size, int level,
                                         std::string* error = nullptr);

}  // namespace rvz

}  // namespace mvp::vfs
