// Local-only test: needs the player's disc (MVP_DISC). Exits 77 (skipped) without it.
//
// Checks: the image opens, main.dol hashes to GV4E69 rev 0, and the DOL section
// table, named from the disc's /mvp.elf, equals the decomp's dol_layout.txt (the
// engine's pinned copy, decomp_pin/).
// Prints only section names and counts, nothing from the game's bytes.
#include <cstdio>
#include <string>

#include "common/file.h"
#include "vfs/disc_check.h"

#ifndef MVP_DISC_DEFAULT
#define MVP_DISC_DEFAULT ""
#endif
#ifndef MVP_DECOMP_PIN_DIR
#define MVP_DECOMP_PIN_DIR ""
#endif

using namespace mvp;
using namespace mvp::vfs;

int main() {
  std::string path = env_or("MVP_DISC", MVP_DISC_DEFAULT);
  const std::string layout = std::string(MVP_DECOMP_PIN_DIR) + "/dol_layout.txt";
  if (path.empty() || !read_file(layout)) {
    std::printf("SKIP: MVP_DISC not set or %s missing\n", layout.c_str());
    return 77;
  }
  auto probe = FileSource::open(path);
  if (!probe) {
    std::printf("SKIP: cannot open %s\n", path.c_str());
    return 77;
  }
  probe.reset();
  std::string err;
  auto disc = open_disc(path, true, &err);
  if (!disc) {
    std::printf("FAIL: %s\n", err.c_str());
    return 1;
  }
  std::printf("disc %s rev %u, main.dol sha1 %s (retail)\n", disc->gcm.game_id().c_str(), disc->gcm.revision(),
              disc->dol_sha1.c_str());
  auto elf = disc->gcm.read_file(*disc->image, "/mvp.elf");
  if (!elf) {
    std::printf("FAIL: /mvp.elf missing\n");
    return 1;
  }
  auto secs = parse_elf_sections(*elf, &err);
  if (!secs) {
    std::printf("FAIL: %s\n", err.c_str());
    return 1;
  }
  auto got = dol_layout_lines(disc->dol, *secs);
  auto want = normalized_layout(*read_text(layout));
  bool same = got == want;
  for (std::size_t i = 0; i < std::max(got.size(), want.size()); ++i) {
    const char* g = i < got.size() ? got[i].c_str() : "(none)";
    const char* w = i < want.size() ? want[i].c_str() : "(none)";
    std::printf("%s %-60s | %s\n", (i < got.size() && i < want.size() && got[i] == want[i]) ? "  " : "!!", g, w);
  }
  std::printf("%s: %zu DOL sections (+bss) vs dol_layout.txt\n", same ? "PASS" : "FAIL", disc->dol.sections().size());
  return same ? 0 : 1;
}
