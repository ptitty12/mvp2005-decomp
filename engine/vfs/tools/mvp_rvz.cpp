// mvp-rvz: pack a plain disc image into an RVZ compressed image. This is how a
// smaller image gets made without Dolphin (docs/engine/WEB.md on carrying a disc
// around). It reads and writes local files only: the image stays the player's own
// copy and nothing is uploaded (CLAUDE.md hard rules).
//
//   mvp-rvz pack <image> <out.rvz> [--level N] [--chunk N] [--none]
//
// The input goes through the engine's own Source, so a CISO works too.
//
// Chunk size is a real tradeoff, measured on this game (level 3, 300 VI fields of
// a browser run; the reader keeps 48 MiB of decompressed chunks):
//
//   chunk    image          I/O time   worst read   stalled frames
//   2 MiB    1049 MB (76%)  420 ms     32 ms        none
//   128 KiB  1119 MB (81%)  260 ms     93 ms        none
//
// A 2 MiB image that the reader cached with only its last chunk (before the chunk
// cache existed) cost 1269 ms and 221 ms worst, so the cache matters more than the
// chunk size. Smaller chunks suit a slower host or a slower machine; the default
// keeps the smaller image.
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include "vfs/rvz.h"
#include "vfs/source.h"

namespace {

int usage() {
  std::fprintf(stderr,
               "usage: mvp-rvz pack <image> <out.rvz> [--level N] [--chunk N] [--none]\n"
               "  --level N  Zstandard level (default 3; negative is legal)\n"
               "  --chunk N  chunk size in bytes (default 2 MiB)\n"
               "  --none     store chunks uncompressed\n");
  return 2;
}

std::uint32_t parse_u32(const char* s) { return static_cast<std::uint32_t>(std::strtoul(s, nullptr, 0)); }

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4 || std::strcmp(argv[1], "pack") != 0) return usage();

  const std::string in = argv[2];
  const std::string out = argv[3];
  int level = 3;
  std::uint32_t chunk = mvp::vfs::RvzSource::kDefaultChunkSize;
  for (int i = 4; i < argc; ++i) {
    if (std::strcmp(argv[i], "--level") == 0 && i + 1 < argc) {
      level = std::atoi(argv[++i]);
    } else if (std::strcmp(argv[i], "--chunk") == 0 && i + 1 < argc) {
      chunk = parse_u32(argv[++i]);
    } else if (std::strcmp(argv[i], "--none") == 0) {
      level = INT32_MIN;
    } else {
      return usage();
    }
  }

  std::string error;
  auto image = mvp::vfs::open_disc_image(in, &error);
  if (!image) {
    std::fprintf(stderr, "mvp-rvz: %s\n", error.c_str());
    return 1;
  }

  std::FILE* f = std::fopen(out.c_str(), "wb");
  if (!f) {
    std::fprintf(stderr, "mvp-rvz: cannot write %s\n", out.c_str());
    return 1;
  }
  const std::uint64_t src_size = image->size();
  const bool ok = mvp::vfs::rvz::pack(
      *image,
      [f](const std::uint8_t* p, std::size_t n) { return std::fwrite(p, 1, n, f) == n; }, chunk,
      level, &error);
  std::fclose(f);
  if (!ok) {
    std::fprintf(stderr, "mvp-rvz: %s\n", error.c_str());
    std::remove(out.c_str());
    return 1;
  }

  std::FILE* o = std::fopen(out.c_str(), "rb");
  if (o) {
    std::fseek(o, 0, SEEK_END);
    const long long made = std::ftell(o);
    std::fclose(o);
    std::printf("packed %s -> %s: %lld of %llu bytes (%.1f%%)\n", in.c_str(), out.c_str(), made,
                static_cast<unsigned long long>(src_size),
                src_size ? 100.0 * double(made) / double(src_size) : 0.0);
  }
  return 0;
}
