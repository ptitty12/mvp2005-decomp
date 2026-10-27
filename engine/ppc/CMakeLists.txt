add_library(engine_ppc STATIC decode.cpp disasm.cpp effects.cpp fp.cpp interp.cpp)
target_link_libraries(engine_ppc PUBLIC engine_common)
target_compile_options(engine_ppc PRIVATE ${ENGINE_WARN})

engine_test(test_ppc_decode SOURCES tests/test_decode.cpp LIBS engine_ppc)
engine_test(test_ppc_fp SOURCES tests/test_fp.cpp LIBS engine_ppc)
engine_test(test_ppc_sem SOURCES tests/test_sem.cpp LIBS engine_ppc)
# Cross-build exactness (E2): native and WebAssembly must print the same digests.
engine_test(test_ppc_digest SOURCES tests/test_digest.cpp LIBS engine_ppc)
# Register effects (E2's wasm emitter) against the semantics.
engine_test(test_ppc_effects SOURCES tests/test_effects.cpp LIBS engine_ppc)
# Differential fuzzing against pureikyubu's interpreter (data-free).
engine_test(test_ppc_fuzz_pk SOURCES tests/test_fuzz_pk.cpp tests/pk_oracle.cpp LIBS engine_ppc pureikyubu_core)
if(MSVC)
  set_source_files_properties(tests/pk_oracle.cpp PROPERTIES COMPILE_OPTIONS "/w")
else()
  set_source_files_properties(tests/pk_oracle.cpp PROPERTIES COMPILE_OPTIONS "-w")
endif()

# Local only: skipped (77) without the disc and dtk's listing.
add_executable(local_disasm tests/local_disasm.cpp)
target_link_libraries(local_disasm PRIVATE engine_ppc engine_vfs)
target_compile_options(local_disasm PRIVATE ${ENGINE_WARN})
target_compile_definitions(local_disasm PRIVATE MVP_DISC_DEFAULT="${MVP_DISC_PATH}" MVP_DTK_ASM_DEFAULT="${MVP_DTK_ASM_PATH}"
                                               MVP_DECOMP_PIN_DIR="${MVP_DECOMP_PIN}")
add_test(NAME local_disasm COMMAND local_disasm)
set_tests_properties(local_disasm PROPERTIES SKIP_RETURN_CODE 77 LABELS local)
