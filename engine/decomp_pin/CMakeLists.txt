# The engine's pinned copy of the decomp's metadata (README.md): two data-free checks.
engine_test(test_decomp_pin SOURCES tests/test_decomp_pin.cpp LIBS engine_runtime engine_translator)
target_compile_definitions(test_decomp_pin PRIVATE MVP_DECOMP_PIN_DIR="${MVP_DECOMP_PIN}")
add_test(NAME decomp_pin_hashes
         COMMAND ${CMAKE_COMMAND} -DPIN=${MVP_DECOMP_PIN} -P ${CMAKE_CURRENT_SOURCE_DIR}/check_pin.cmake)
