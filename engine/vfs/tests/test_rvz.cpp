// Data-free tests for the RVZ reader and packer: a synthetic disc, packed in
// memory and read back, plus the errors a malformed image must produce.
#include <doctest/doctest.h>

#include <cstring>
#include <string>
#include <vector>

#include "common/bytes.h"
#include "vfs/rvz.h"
#include "vfs/source.h"

using namespace mvp;
using namespace mvp::vfs;

namespace {

constexpr std::uint32_t kChunk = 0x8000;  // 32 KiB, the format's minimum

// A deterministic disc: the first half compresses, the second half does not (a
// xorshift stream), so the packer exercises both the compressed and the raw path.
std::vector<u8> make_disc(std::size_t size) {
  std::vector<u8> d(size);
  for (std::size_t i = 0; i < size; ++i) d[i] = u8((i * 7) ^ (i >> 5));
  std::uint32_t s = 0x12345678;
  for (std::size_t i = size / 2; i < size; ++i) {
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    d[i] = u8(s);
  }
  return d;
}

std::vector<u8> pack_disc(const std::vector<u8>& disc, int level) {
  MemorySource src(disc);
  std::string error;
  std::vector<u8> rvz = rvz::pack_to_vector(src, kChunk, level, &error);
  REQUIRE_MESSAGE(!rvz.empty(), error);
  return rvz;
}

std::unique_ptr<RvzSource> open_rvz(std::vector<u8> bytes, std::string* error = nullptr) {
  return RvzSource::open(std::make_unique<MemorySource>(std::move(bytes)), error);
}

std::vector<u8> read_all(Source& s) {
  std::vector<u8> out(s.size());
  REQUIRE(s.read(0, out.data(), out.size()));
  return out;
}

}  // namespace

TEST_CASE("rvz: the container has the RVZ magic and is smaller than the disc") {
  const auto disc = make_disc(kChunk * 8 + 123);
  const auto rvz = pack_disc(disc, 3);
  // The literal file bytes, not a word: getting this wrong still round-trips
  // through our own reader, so assert the four bytes a real RVZ file must have.
  CHECK(std::memcmp(rvz.data(), "RVZ", 3) == 0);
  CHECK(rvz[3] == 0x01);
  // Half the disc compresses, so the container must come out smaller.
  CHECK(rvz.size() < disc.size());
}

TEST_CASE("rvz: a packed image reads back byte for byte") {
  const auto disc = make_disc(kChunk * 8 + 123);
  const auto rvz = pack_disc(disc, 3);
  auto s = open_rvz(rvz);
  REQUIRE(s);
  CHECK(s->size() == disc.size());
  CHECK(read_all(*s) == disc);
}

TEST_CASE("rvz: unaligned and crossing reads agree with the image") {
  const auto disc = make_disc(kChunk * 3 + 777);
  const auto rvz = pack_disc(disc, 1);
  auto s = open_rvz(rvz);
  REQUIRE(s);
  for (std::size_t off = 0; off + 37 <= disc.size(); off += 4093) {
    std::vector<u8> got(37);
    REQUIRE(s->read(off, got.data(), got.size()));
    CHECK(std::memcmp(got.data(), disc.data() + off, got.size()) == 0);
  }
  // A read that crosses the 0x80 dhead boundary and a chunk boundary.
  for (std::size_t off : {std::size_t(0x41), std::size_t(kChunk - 9), std::size_t(2 * kChunk - 1)}) {
    std::vector<u8> got(64);
    REQUIRE(s->read(off, got.data(), got.size()));
    CHECK(std::memcmp(got.data(), disc.data() + off, got.size()) == 0);
  }
}

TEST_CASE("rvz: the uncompressed method round-trips") {
  const auto disc = make_disc(kChunk * 2 + 5);
  const auto rvz = pack_disc(disc, INT32_MIN);
  auto s = open_rvz(rvz);
  REQUIRE(s);
  CHECK(read_all(*s) == disc);
  // With no compression the container is the disc plus its overhead.
  CHECK(rvz.size() >= disc.size());
}

TEST_CASE("rvz: an all-zero disc packs to almost nothing and still reads") {
  const std::vector<u8> disc(kChunk * 4, 0);
  const auto rvz = pack_disc(disc, 3);
  CHECK(rvz.size() < 0x400);  // every chunk is the "all zero" special case
  auto s = open_rvz(rvz);
  REQUIRE(s);
  CHECK(read_all(*s) == disc);
}

TEST_CASE("rvz: reads outside the disc are refused") {
  const auto disc = make_disc(kChunk + 16);
  auto s = open_rvz(pack_disc(disc, 3));
  REQUIRE(s);
  u8 b = 0;
  CHECK_FALSE(s->read(disc.size(), &b, 1));
  CHECK_FALSE(s->read(disc.size() - 8, &b, 16));
  CHECK(s->read(disc.size() - 1, &b, 1));
}

TEST_CASE("rvz: a malformed image is refused with a reason") {
  const auto disc = make_disc(kChunk * 2);
  const auto good = pack_disc(disc, 3);
  std::string error;

  auto bad_magic = good;
  bad_magic[0] = 'X';
  CHECK_FALSE(open_rvz(bad_magic, &error));
  CHECK(error.find("magic") != std::string::npos);

  // A Wii image (disc_type 2) needs partition and hash-exception support.
  auto wii = good;
  put_be32(&wii[0x48], 2);
  CHECK_FALSE(open_rvz(wii, &error));
  CHECK(error.find("Wii") != std::string::npos);

  // lzma2 (4) is a WIA method this reader does not implement.
  auto lzma = good;
  put_be32(&lzma[0x4C], 4);
  CHECK_FALSE(open_rvz(lzma, &error));
  CHECK(error.find("compression") != std::string::npos);

  // A chunk size below the format's 32 KiB floor.
  auto small = good;
  put_be32(&small[0x54], 0x1000);
  CHECK_FALSE(open_rvz(small, &error));
  CHECK(error.find("chunk") != std::string::npos);

  // A truncated file cannot hold the table it points at.
  auto cut = good;
  cut.resize(0x124 + 8);
  CHECK_FALSE(open_rvz(cut, &error));
}
