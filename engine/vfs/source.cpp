#include "vfs/source.h"

#include <cstdio>
#include <cstring>

#include "common/bytes.h"
#include "vfs/rvz.h"

namespace mvp::vfs {

std::vector<std::uint8_t> Source::read_vec(std::uint64_t off, std::size_t n, bool* ok) {
  std::vector<std::uint8_t> v(n);
  bool r = read(off, v.data(), n);
  if (ok) *ok = r;
  if (!r) v.clear();
  return v;
}

namespace {
int seek64(std::FILE* f, std::uint64_t off) {
#if defined(_MSC_VER)
  return _fseeki64(f, static_cast<__int64>(off), SEEK_SET);
#else
  return fseeko(f, static_cast<off_t>(off), SEEK_SET);
#endif
}
std::uint64_t tell64(std::FILE* f) {
#if defined(_MSC_VER)
  return static_cast<std::uint64_t>(_ftelli64(f));
#else
  return static_cast<std::uint64_t>(ftello(f));
#endif
}
}  // namespace

std::unique_ptr<FileSource> FileSource::open(const std::string& path) {
  std::FILE* f = std::fopen(path.c_str(), "rb");
  if (!f) return nullptr;
  std::unique_ptr<FileSource> s(new FileSource());
  s->f_ = f;
#if defined(_MSC_VER)
  _fseeki64(f, 0, SEEK_END);
#else
  fseeko(f, 0, SEEK_END);
#endif
  s->size_ = tell64(f);
  return s;
}

FileSource::~FileSource() {
  if (f_) std::fclose(static_cast<std::FILE*>(f_));
}

bool FileSource::read(std::uint64_t off, void* dst, std::size_t n) {
  if (off > size_ || n > size_ - off) return false;
  if (n == 0) return true;
  std::FILE* f = static_cast<std::FILE*>(f_);
  if (seek64(f, off) != 0) return false;
  return std::fread(dst, 1, n, f) == n;
}

bool MemorySource::read(std::uint64_t off, void* dst, std::size_t n) {
  if (off > bytes_.size() || n > bytes_.size() - off) return false;
  if (n) std::memcpy(dst, bytes_.data() + off, n);
  return true;
}

std::unique_ptr<CisoSource> CisoSource::open(std::unique_ptr<Source> inner, std::string* error) {
  auto fail = [&](const char* why) -> std::unique_ptr<CisoSource> {
    if (error) *error = why;
    return nullptr;
  };
  constexpr std::size_t kHeader = 0x8000;
  if (inner->size() < kHeader) return fail("CISO: file shorter than its header");
  std::vector<std::uint8_t> h(kHeader);
  if (!inner->read(0, h.data(), kHeader)) return fail("CISO: header read failed");
  if (std::memcmp(h.data(), "CISO", 4) != 0) return fail("CISO: bad magic");
  std::uint32_t bs = le32(h.data() + 4);
  if (bs == 0 || (bs & (bs - 1)) != 0) return fail("CISO: block size is not a power of two");
  std::unique_ptr<CisoSource> s(new CisoSource());
  s->block_size_ = bs;
  std::uint64_t next = kHeader;
  std::size_t last_used = 0;
  std::size_t nblocks = kHeader - 8;
  s->block_off_.assign(nblocks, -1);
  for (std::size_t i = 0; i < nblocks; ++i) {
    std::uint8_t m = h[8 + i];
    if (m > 1) return fail("CISO: bad block map entry");
    if (m == 1) {
      s->block_off_[i] = static_cast<std::int64_t>(next);
      next += bs;
      last_used = i + 1;
    }
  }
  if (next > inner->size()) return fail("CISO: stored blocks exceed the file");
  // The header doesn't record the logical size. Unstored blocks read as zero, so
  // the image covers at least a full GameCube disc (1,459,978,240 bytes) and at
  // least every stored block, within what the block map can address.
  constexpr std::uint64_t kGameCubeDiscSize = 1459978240ull;
  std::uint64_t stored = static_cast<std::uint64_t>(last_used) * bs;
  std::uint64_t mapped = static_cast<std::uint64_t>(nblocks) * bs;
  std::uint64_t logical = stored > kGameCubeDiscSize ? stored : kGameCubeDiscSize;
  s->size_ = logical < mapped ? logical : mapped;
  s->inner_ = std::move(inner);
  return s;
}

bool CisoSource::read(std::uint64_t off, void* dst, std::size_t n) {
  if (off > size_ || n > size_ - off) return false;
  std::uint8_t* out = static_cast<std::uint8_t*>(dst);
  while (n) {
    std::uint64_t blk = off / block_size_;
    std::uint32_t bo = static_cast<std::uint32_t>(off % block_size_);
    std::size_t take = block_size_ - bo;
    if (take > n) take = n;
    std::int64_t at = block_off_[static_cast<std::size_t>(blk)];
    if (at < 0) {
      std::memset(out, 0, take);
    } else if (!inner_->read(static_cast<std::uint64_t>(at) + bo, out, take)) {
      return false;
    }
    out += take;
    off += take;
    n -= take;
  }
  return true;
}

std::unique_ptr<Source> open_disc_image(const std::string& path, std::string* error) {
  auto f = FileSource::open(path);
  if (!f) {
    if (error) *error = "cannot open " + path;
    return nullptr;
  }
  char magic[4] = {};
  if (!(f->size() >= 4 && f->read(0, magic, 4))) return f;
  if (std::memcmp(magic, "CISO", 4) == 0) return CisoSource::open(std::move(f), error);
#ifdef MVP_HAVE_ZSTD
  // RVZ: Dolphin's compressed container ("RVZ\x1"). Without zstd it stays a plain
  // byte source and the identity check reports the bad magic instead.
  if (std::memcmp(magic, "RVZ", 3) == 0 && static_cast<unsigned char>(magic[3]) == 1)
    return RvzSource::open(std::move(f), error);
#endif
  return f;
}

}  // namespace mvp::vfs
