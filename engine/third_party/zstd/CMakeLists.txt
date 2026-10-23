# Zstandard 1.5.5 (BSD-3-Clause), vendored: the RVZ disc container stores its
# chunks and its tables as zstd streams (vfs/rvz.cpp). Fetched from the v1.5.5
# release tarball; docs/REUSE.md §6 records it. Built from the same sources for the
# host and for WebAssembly.
#
# The x86-64 assembly decompressor (lib/decompress/huf_decompress_amd64.S) is not
# included, so every target takes the portable path; ZSTD_MULTITHREAD is off, since
# the engine decompresses one chunk at a time and must not need threads.
add_library(engine_zstd STATIC
  lib/common/debug.c
  lib/common/entropy_common.c
  lib/common/error_private.c
  lib/common/fse_decompress.c
  lib/common/pool.c
  lib/common/threading.c
  lib/common/xxhash.c
  lib/common/zstd_common.c
  lib/compress/fse_compress.c
  lib/compress/hist.c
  lib/compress/huf_compress.c
  lib/compress/zstd_compress.c
  lib/compress/zstd_compress_literals.c
  lib/compress/zstd_compress_sequences.c
  lib/compress/zstd_compress_superblock.c
  lib/compress/zstd_double_fast.c
  lib/compress/zstd_fast.c
  lib/compress/zstd_lazy.c
  lib/compress/zstd_ldm.c
  lib/compress/zstdmt_compress.c
  lib/compress/zstd_opt.c
  lib/decompress/huf_decompress.c
  lib/decompress/zstd_ddict.c
  lib/decompress/zstd_decompress_block.c
  lib/decompress/zstd_decompress.c
)
target_include_directories(engine_zstd PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib")
target_compile_definitions(engine_zstd PRIVATE
  ZSTD_DISABLE_ASM=1
  ZSTD_LEGACY_SUPPORT=0
  ZSTD_MULTITHREAD=0
)
# Third-party code: the engine's warning set is not ours to fix here.
if(MSVC)
  target_compile_options(engine_zstd PRIVATE /w)
else()
  target_compile_options(engine_zstd PRIVATE -w)
endif()
