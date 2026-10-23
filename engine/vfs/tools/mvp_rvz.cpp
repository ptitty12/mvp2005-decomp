// mvp-rvz: pack a plain disc image into an RVZ compressed image. This is how a
// smaller image gets made without Dolphin (docs/engine/WEB.md on carrying a disc
// around). It reads and writes local files only: the image stays the player's own
// copy and nothing is uploaded (CLAUDE.md hard rules).
//
//   mvp-rvz pack <image> <out.rvz> [--level N] [--chunk N] [--none]
//
// The input goes through the engine's own Source, so a CISO works too.
//
// Chunk size is a real tradeoff in both directions, and it depends on how the image
// reaches the page. Measured on this game (level 3, 300 VI fields of a browser run;
// the reader keeps 48 MiB of decompressed chunks). "file" is the page's own picked
// file; "server" is the deployment serving it over HTTP, which is how a player-link
// holder gets it:
//
//   chunk    image           file: I/O / worst frame    server: I/O / worst frame
//   2 MiB    1049 MB (76%)   420 ms / 174 ms            932 ms / 340 ms
//   512 KiB  1078 MB (78%)   -                           396 ms / 138 ms
//   128 KiB  1119 MB (81%)   260 ms / 123 ms            248 ms / 142 ms
//   plain ISO for scale      601 ms / 107 ms            191 ms / 144 ms
//
// A large chunk is much worse over the server: the page reads it in blocks and
// prefetches ahead, and a miss that inflates 2 MiB to answer a few KB defeats both,
// which is where the 340 ms frame comes from. The decompressed-chunk cache matters
// as much: before it existed the 2 MiB image cost 1269 ms over the file path.
//
// The default is 128 KiB and level 9: over the server it is within 30% of the
// uncompressed image at ~81% of its size, and the frame worse-case matches it. A
// local-only image can use 2 MiB. For the smallest image, ignore latency and pack
// with --chunk 2097152 --level 19: 912 MB (66%), but a 420 ms file path and a much
// worse worst-case frame. Level is nearly free at run time (zstd decode speed barely
// moves with it), so it is only ever a packing-time cost:
//
//   level   size @128 KiB   pack time
//   3       1119 MB (81.1%) 26 s
//   9       1113 MB (80.7%) 32 s
//   15      1100 MB (79.8%) 2m33s
//   19      1095 MB (79.4%) 5m42s
//
// Stripping is not available: the disc is 100% covered by its files (461 bytes
// unused), because NKit had already removed the junk, so there is nothing to drop.
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
               "  --level N  Zstandard level (default 9; negative is legal)\n"
               "  --chunk N  chunk size in bytes (default 128 KiB; see the note above)\n"
               "  --none     store chunks uncompressed\n");
  return 2;
}

std::uint32_t parse_u32(const char* s) { return static_cast<std::uint32_t>(std::strtoul(s, nullptr, 0)); }

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4 || std::strcmp(argv[1], "pack") != 0) return usage();

  const std::string in = argv[2];
  const std::string out = argv[3];
  int level = 9;
  // 128 KiB: the server path's measurement above. Not RvzSource::kDefaultChunkSize,
  // which is the format's reference size (what Dolphin writes), not our choice.
  std::uint32_t chunk = 0x20000;
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
