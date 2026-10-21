// Data-free tests for the browser client's caches (E2c): the block cache that
// reads the server's disc image (block_cache.h) and the translated module's
// serialization (module_cache.h). Run under Node (preset "emscripten").
#include <doctest/doctest.h>

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "web/wasm/block_cache.h"
#include "web/wasm/module_cache.h"

using namespace mvp;
using namespace mvp::web;

namespace {

// A "server" over bytes in memory that counts its requests.
struct FakeFetcher final : RangeFetcher {
  const std::vector<std::uint8_t>* bytes = nullptr;
  int* requests = nullptr;
  std::vector<std::pair<std::uint64_t, std::size_t>>* log = nullptr;
  bool fail = false;
  bool fetch(std::uint64_t off, std::uint8_t* dst, std::size_t n) override {
    ++*requests;
    if (log) log->push_back({off, n});
    if (fail || off + n > bytes->size()) return false;
    std::copy(bytes->begin() + static_cast<std::ptrdiff_t>(off),
              bytes->begin() + static_cast<std::ptrdiff_t>(off + n), dst);
    return true;
  }
  std::string error() const override { return fail ? "offline" : ""; }
};

std::vector<std::uint8_t> pattern(std::size_t n) {
  std::vector<std::uint8_t> v(n);
  std::uint32_t x = 12345;
  for (auto& b : v) {
    x = x * 1103515245u + 12345u;
    b = static_cast<std::uint8_t>(x >> 16);
  }
  return v;
}

struct Fixture {
  std::vector<std::uint8_t> image;
  int requests = 0;
  std::vector<std::pair<std::uint64_t, std::size_t>> log;
  FakeFetcher* fetcher = nullptr;
  std::unique_ptr<BlockCacheSource> src;
  Fixture(std::size_t size, BlockCacheSource::Options opt) : image(pattern(size)) {
    auto f = std::make_unique<FakeFetcher>();
    f->bytes = &image;
    f->requests = &requests;
    f->log = &log;
    fetcher = f.get();
    src = std::make_unique<BlockCacheSource>(std::move(f), image.size(), opt);
  }
  bool same(std::uint64_t off, std::size_t n) {
    std::vector<std::uint8_t> got(n);
    if (!src->read(off, got.data(), n)) return false;
    return std::equal(got.begin(), got.end(), image.begin() + static_cast<std::ptrdiff_t>(off));
  }
};

BlockCacheSource::Options small_blocks() {
  BlockCacheSource::Options o;
  o.block = 4096;
  o.max_blocks = 16;
  o.max_run = 4;
  o.ahead = 2;
  return o;
}

}  // namespace

TEST_CASE("block cache: random reads return the image's bytes, across blocks and the short last block") {
  Fixture fx(4096 * 40 + 1234, small_blocks());
  std::mt19937 rng(7);
  for (int k = 0; k < 2000; ++k) {
    const std::uint64_t off = rng() % fx.image.size();
    const std::size_t n = std::min<std::size_t>(rng() % 20000, fx.image.size() - off);
    REQUIRE(fx.same(off, n));
  }
  CHECK(fx.same(fx.image.size() - 1234, 1234));
  CHECK(fx.same(0, fx.image.size()));  // larger than the cache
  std::uint8_t b;
  CHECK_FALSE(fx.src->read(fx.image.size(), &b, 1));
  CHECK(fx.src->read(fx.image.size(), &b, 0));
  CHECK(fx.src->stats().evicted > 0);
}

TEST_CASE("block cache: hits cost no request; sequential misses fetch growing runs") {
  Fixture fx(4096 * 64, small_blocks());
  CHECK(fx.same(100, 10));
  CHECK(fx.requests == 1);
  CHECK(fx.same(200, 3000));  // same block
  CHECK(fx.requests == 1);
  // A front-to-back stream: runs of 1, 2, 4, 4, ... blocks.
  for (std::uint64_t off = 4096; off < 4096 * 20; off += 2048) REQUIRE(fx.same(off, 2048));
  REQUIRE(fx.log.size() >= 4);
  CHECK(fx.log[0].second == 4096);      // block 0
  CHECK(fx.log[1].second == 2 * 4096);  // blocks 1-2: right after block 0's run
  CHECK(fx.log[2].second == 4 * 4096);  // blocks 3-6
  CHECK(fx.log.back().second == 4 * 4096);
  CHECK(fx.requests < 10);  // 20 blocks in fewer requests
}

TEST_CASE("block cache: read-ahead wants the blocks after a stream; inserted blocks are hits") {
  Fixture fx(4096 * 64, small_blocks());
  CHECK(fx.src->want().count == 0);  // no stream yet
  for (std::uint64_t b = 0; b < 3; ++b) REQUIRE(fx.same(b * 4096, 4096));
  const int before = fx.requests;
  auto w = fx.src->want();
  // Blocks fetched by the last run may already be there; what is wanted is
  // the first missing ones within `ahead` of the last read block.
  REQUIRE(w.count > 0);
  CHECK(w.first >= 3);
  CHECK(fx.src->want().count == 0);  // in flight: not wanted twice
  std::vector<std::uint8_t> data(fx.image.begin() + static_cast<std::ptrdiff_t>(w.first * 4096),
                                 fx.image.begin() + static_cast<std::ptrdiff_t>((w.first + w.count) * 4096));
  fx.src->insert(w.first, w.count, data.data(), data.size(), true);
  CHECK(fx.src->stats().prefetched == w.count);
  REQUIRE(fx.same(w.first * 4096, w.count * 4096));
  CHECK(fx.requests == before);  // served from the inserted blocks
  CHECK(fx.src->stats().prefetch_used == w.count);
  // A failed prefetch releases its blocks; a wrong length is ignored.
  auto w2 = fx.src->want();
  REQUIRE(w2.count > 0);
  fx.src->insert(w2.first, w2.count, nullptr, 0, false);
  auto w3 = fx.src->want();
  CHECK(w3.first == w2.first);
  fx.src->insert(w3.first, w3.count, data.data(), 7, true);
  CHECK(fx.src->stats().prefetched == w.count);
}

TEST_CASE("block cache: a failed request fails the read and says why") {
  Fixture fx(4096 * 8, small_blocks());
  fx.fetcher->fail = true;
  std::uint8_t b[16];
  CHECK_FALSE(fx.src->read(0, b, 16));
  CHECK(fx.src->error() == "offline");
  fx.fetcher->fail = false;
  CHECK(fx.same(0, 16));
}

TEST_CASE("module cache: a module survives the round trip; damage is refused") {
  xlat::WasmModule m;
  m.bytes = {0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00, 0x42};
  m.slots = 7;
  m.entries = {{0x80003100, 0}, {0x80003200, 1}};
  m.resume = {{0x80003110, 0}};
  m.resume_index = {{0x80003110, 3}, {0x80003210, 9}};
  m.safe = {0x80003104};
  m.idle = {0x80003300, 0x80003304};
  m.hle_sites = {{0x80004000, 2}, {0x80004100, 5}};
  m.hle_matched = {2, 5};
  m.fixed = {0x1000, 0x1004, 0x1008, 0x2000, 0x3000, 0x400000, 0x17FFFF9};  // V4
  m.stats.functions = 2;
  m.stats.instructions = 123;
  m.stats.code_bytes = 9;
  m.stats.helper_by_op = {0, 1, 2, 3};
  const auto blob = serialize_module(m);
  auto joined = serialize_module_header(m);
  joined.insert(joined.end(), m.bytes.begin(), m.bytes.end());
  CHECK(joined == blob);
  xlat::WasmModule r;
  std::string err;
  REQUIRE(deserialize_module(blob.data(), blob.size(), &r, &err));
  CHECK(r.bytes == m.bytes);
  CHECK(r.slots == 7);
  REQUIRE(r.entries.size() == 2);
  CHECK(r.entries[1].address == 0x80003200);
  CHECK(r.entries[1].slot == 1);
  CHECK(r.resume.size() == 1);
  CHECK(r.resume_index == m.resume_index);
  CHECK(r.safe == m.safe);
  CHECK(r.idle == m.idle);
  CHECK(r.hle_sites == m.hle_sites);
  CHECK(r.hle_matched == m.hle_matched);
  CHECK(r.fixed == m.fixed);
  CHECK(r.stats.functions == 2);
  CHECK(r.stats.instructions == 123);
  CHECK(r.stats.code_bytes == 9);
  CHECK(r.stats.helper_by_op == m.stats.helper_by_op);

  auto cut = blob;
  cut.pop_back();
  CHECK_FALSE(deserialize_module(cut.data(), cut.size(), &r, &err));
  auto bad = blob;
  bad[0] = 'X';
  CHECK_FALSE(deserialize_module(bad.data(), bad.size(), &r, &err));
  auto huge = blob;
  huge[16] = 0xFF;  // the entry count: more records than bytes
  huge[17] = 0xFF;
  CHECK_FALSE(deserialize_module(huge.data(), huge.size(), &r, &err));
}
