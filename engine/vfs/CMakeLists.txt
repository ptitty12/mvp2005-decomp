add_library(engine_vfs STATIC source.cpp gcm.cpp dol.cpp disc_check.cpp refpack.cpp big.cpp overlay.cpp)
target_link_libraries(engine_vfs PUBLIC engine_common)
target_compile_options(engine_vfs PRIVATE ${ENGINE_WARN})

add_executable(mvp-disc tools/mvp_disc.cpp)
target_link_libraries(mvp-disc PRIVATE engine_vfs)
target_compile_options(mvp-disc PRIVATE ${ENGINE_WARN})

engine_test(test_vfs SOURCES tests/test_vfs.cpp LIBS engine_vfs)

# Local only: skipped (77) without the disc.
add_executable(local_disc tests/local_disc.cpp)
target_link_libraries(local_disc PRIVATE engine_vfs)
target_compile_options(local_disc PRIVATE ${ENGINE_WARN})
target_compile_definitions(local_disc PRIVATE MVP_DISC_DEFAULT="${MVP_DISC_PATH}" MVP_DECOMP_PIN_DIR="${MVP_DECOMP_PIN}")
add_test(NAME local_disc COMMAND local_disc)
set_tests_properties(local_disc PROPERTIES SKIP_RETURN_CODE 77 LABELS local)
