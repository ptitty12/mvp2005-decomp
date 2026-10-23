# RVZ compressed images (vfs/rvz.cpp, vfs/rvz_pack.cpp) use the vendored zstd in
# third_party/zstd, so both the host and the WebAssembly build read them.
set(MVP_VFS_SOURCES source.cpp gcm.cpp dol.cpp disc_check.cpp refpack.cpp big.cpp overlay.cpp
    rvz.cpp rvz_pack.cpp)

add_library(engine_vfs STATIC ${MVP_VFS_SOURCES})
target_link_libraries(engine_vfs PUBLIC engine_common PRIVATE engine_zstd)
target_compile_definitions(engine_vfs PRIVATE MVP_HAVE_ZSTD=1)
target_compile_options(engine_vfs PRIVATE ${ENGINE_WARN})

# Authoring tool: pack a plain disc image into an RVZ (no Dolphin needed).
add_executable(mvp-rvz tools/mvp_rvz.cpp)
target_link_libraries(mvp-rvz PRIVATE engine_vfs)
target_compile_options(mvp-rvz PRIVATE ${ENGINE_WARN})

engine_test(test_rvz SOURCES tests/test_rvz.cpp LIBS engine_vfs)

add_executable(mvp-disc tools/mvp_disc.cpp)
target_link_libraries(mvp-disc PRIVATE engine_vfs)
target_compile_options(mvp-disc PRIVATE ${ENGINE_WARN})

engine_test(test_vfs SOURCES tests/test_vfs.cpp LIBS engine_vfs)

# Local only: skipped (77) without the disc.
add_executable(local_disc tests/local_disc.cpp)
target_link_libraries(local_disc PRIVATE engine_vfs)
target_compile_options(local_disc PRIVATE ${ENGINE_WARN})
target_compile_definitions(local_disc PRIVATE MVP_DISC_DEFAULT="${MVP_DISC_PATH}" MVP_REPO_ROOT="${MVP_REPO_ROOT}")
add_test(NAME local_disc COMMAND local_disc)
set_tests_properties(local_disc PROPERTIES SKIP_RETURN_CODE 77 LABELS local)
