add_library(engine_runtime STATIC native.cpp machine.cpp boot.cpp hw.cpp gx_fifo.cpp sys_hooks.cpp hle.cpp hle_game.cpp
            input.cpp ax.cpp host_yield.cpp mod_hooks.cpp save_state.cpp state_file.cpp memcard.cpp)
target_link_libraries(engine_runtime PUBLIC engine_ppc engine_vfs)
target_compile_options(engine_runtime PRIVATE ${ENGINE_WARN})

engine_test(test_boot_hw SOURCES tests/test_boot_hw.cpp LIBS engine_runtime)
engine_test(test_clock SOURCES tests/test_clock.cpp LIBS engine_runtime)
engine_test(test_devices SOURCES tests/test_devices.cpp LIBS engine_runtime)
engine_test(test_input SOURCES tests/test_input.cpp LIBS engine_runtime)
engine_test(test_ax SOURCES tests/test_ax.cpp LIBS engine_runtime)
engine_test(test_state_file SOURCES tests/test_state_file.cpp LIBS engine_runtime)

# Local only (Q4): the native implementations of game functions (hle_game.h)
# against the interpreter running the originals from the disc's main.dol.
# Skipped (77) without the disc.
add_executable(local_hle_native tests/local_hle_native.cpp)
target_link_libraries(local_hle_native PRIVATE engine_runtime engine_vfs)
target_compile_options(local_hle_native PRIVATE ${ENGINE_WARN})
target_compile_definitions(local_hle_native PRIVATE MVP_DISC_DEFAULT="${MVP_DISC_PATH}")
add_test(NAME local_hle_native COMMAND local_hle_native)
set_tests_properties(local_hle_native PROPERTIES SKIP_RETURN_CODE 77 LABELS local)

# Lockstep checker (development): translated code against the interpreter.
add_library(engine_lockstep STATIC lockstep.cpp)
target_link_libraries(engine_lockstep PUBLIC engine_runtime engine_translator)
target_compile_options(engine_lockstep PRIVATE ${ENGINE_WARN})

# The WebAssembly emitter's host side (E2; Emscripten only): the "rt" imports a
# translated module calls, and loading a module into the runtime's own table.
if(EMSCRIPTEN)
  add_library(engine_wasm_host STATIC wasm_host.cpp)
  target_link_libraries(engine_wasm_host PUBLIC engine_runtime engine_translator)
  target_compile_options(engine_wasm_host PRIVATE ${ENGINE_WARN})
  target_link_options(engine_wasm_host INTERFACE -sALLOW_TABLE_GROWTH=1)
  # Data-free: the synthetic program translated to wasm in-process, in lockstep.
  engine_test(test_wasm_lockstep SOURCES tests/test_wasm_lockstep.cpp ${PROJECT_SOURCE_DIR}/translator/tests/synth.cpp
              LIBS engine_lockstep engine_wasm_host)
  # E2d: the semantics digest and the inline floating point through translated
  # code, against the interpreter (data-free).
  engine_test(test_wasm_digest SOURCES tests/test_wasm_digest.cpp LIBS engine_wasm_host)
endif()

# Data-free emitter test: the synthetic program is translated at build time.
set(MVP_SYNTH_DIR ${CMAKE_BINARY_DIR}/synth_gen)
set(MVP_SYNTH_SOURCES ${MVP_SYNTH_DIR}/game_00.cpp ${MVP_SYNTH_DIR}/game_01.cpp ${MVP_SYNTH_DIR}/game_table.cpp)
add_custom_command(
  OUTPUT ${MVP_SYNTH_SOURCES} ${MVP_SYNTH_DIR}/game_decls.h
  COMMAND mvp-synth-emit ${MVP_SYNTH_DIR}
  DEPENDS mvp-synth-emit
  COMMENT "Translating the synthetic test program to C++"
  VERBATIM)
engine_test(test_emit_lockstep SOURCES tests/test_emit_lockstep.cpp ${PROJECT_SOURCE_DIR}/translator/tests/synth.cpp
            ${MVP_SYNTH_SOURCES} LIBS engine_lockstep)
target_include_directories(test_emit_lockstep PRIVATE ${MVP_SYNTH_DIR})
if(MSVC)
  set_source_files_properties(${MVP_SYNTH_SOURCES} PROPERTIES COMPILE_OPTIONS "/w;/bigobj")
else()
  set_source_files_properties(${MVP_SYNTH_SOURCES} PROPERTIES COMPILE_OPTIONS "-w")
endif()

# ---- Translated game code: local only, derived from the player's disc ----
# mvp-xlat translates main.dol into build/engine/<preset>/gen at build time. The
# files never leave build/. Without the disc these targets do not exist.
if(MVP_DISC_PATH AND EXISTS "${MVP_DISC_PATH}")
  set(MVP_GEN_DIR ${CMAKE_BINARY_DIR}/gen)
  set(MVP_GEN_SHARDS 48)
  set(MVP_GEN_SOURCES ${MVP_GEN_DIR}/game_table.cpp)
  math(EXPR last "${MVP_GEN_SHARDS} - 1")
  foreach(i RANGE 0 ${last})
    if(i LESS 10)
      list(APPEND MVP_GEN_SOURCES ${MVP_GEN_DIR}/game_0${i}.cpp)
    else()
      list(APPEND MVP_GEN_SOURCES ${MVP_GEN_DIR}/game_${i}.cpp)
    endif()
  endforeach()
  add_custom_command(
    OUTPUT ${MVP_GEN_SOURCES} ${MVP_GEN_DIR}/game_decls.h
    COMMAND mvp-xlat emit-cpp ${MVP_GEN_DIR} --shards ${MVP_GEN_SHARDS}
    DEPENDS mvp-xlat ${MVP_DECOMP_PIN}/symbols.txt
    COMMENT "Translating main.dol to C++ (build/ only)"
    VERBATIM)
  add_library(mvp_game_native STATIC ${MVP_GEN_SOURCES})
  target_include_directories(mvp_game_native PRIVATE ${MVP_GEN_DIR})
  target_link_libraries(mvp_game_native PUBLIC engine_runtime)
  set_target_properties(mvp_game_native PROPERTIES MSVC_DEBUG_INFORMATION_FORMAT "")
  # Optimization of the translated code (E3a; one option for every preset): /O2 on
  # MSVC, -O1 elsewhere (clang, Emscripten), unless MVP_GAME_OPT names another
  # level. For a quick native build or E2's /Od comparison build pass
  # -DMVP_GAME_OPT=-Od (from Git Bash write -O2/-Od, not /O2, which its path
  # conversion mangles); E2's native /O2 reference is the default now. Guest
  # floating point is integer code or provably equal host arithmetic (ppc/fp.h),
  # and host FP keeps /fp:precise without contraction, so the level changes speed
  # only: local_boot_twice and local_play_determinism give the same guest state
  # at /Od and /O2.
  set(MVP_GAME_OPT "" CACHE STRING "Optimization flag for the translated game code (empty: /O2 on MSVC, -O1 elsewhere)")
  if(MSVC)
    set(game_opt /O2)
  else()
    set(game_opt -O1)
  endif()
  if(MVP_GAME_OPT)
    set(game_opt ${MVP_GAME_OPT})
  endif()
  if(MSVC)
    target_compile_options(mvp_game_native PRIVATE ${game_opt} /fp:precise /bigobj /w /EHsc)
  else()
    target_compile_options(mvp_game_native PRIVATE ${game_opt} -ffp-contract=off -fno-fast-math -w)
  endif()

  add_executable(mvp-boot tools/mvp_boot.cpp)
  target_link_libraries(mvp-boot PRIVATE mvp_game_native engine_lockstep engine_audio)  # engine_audio: audio/ (E3b)
  target_compile_options(mvp-boot PRIVATE ${ENGINE_WARN})
  if(MSVC)
    target_link_options(mvp-boot PRIVATE /STACK:67108864)
  endif()
  if(EMSCRIPTEN)
    # E2: the same tool with the DOL translated to WebAssembly at startup
    # (tools/wasm_game.cpp) instead of the C++-emitted translation.
    add_executable(mvp-boot-wasm tools/mvp_boot.cpp tools/wasm_game.cpp)
    target_link_libraries(mvp-boot-wasm PRIVATE engine_lockstep engine_wasm_host engine_audio)  # --audio-wav (E3b)
    target_compile_options(mvp-boot-wasm PRIVATE ${ENGINE_WARN})
    target_compile_definitions(mvp-boot-wasm PRIVATE MVP_BOOT_WASM=1)
  endif()
  add_test(NAME local_lockstep COMMAND mvp-boot lockstep)
  set_tests_properties(local_lockstep PROPERTIES SKIP_RETURN_CODE 77 LABELS local)
  # E0b item 7: boot with the IPL environment until main.
  add_test(NAME local_boot_main COMMAND mvp-boot run --until main --mmio 0)
  set_tests_properties(local_boot_main PROPERTIES SKIP_RETURN_CODE 77 LABELS local)
  # E0b items 9 and 10: threads, interrupts and devices. Boots until the fifth
  # guest thread (the memory-card worker, created by the fourth OSCreateThread
  # site) starts at fn_8035A198; two runs must match.
  add_test(NAME local_boot_twice
           COMMAND ${CMAKE_COMMAND} -DBOOT=$<TARGET_FILE:mvp-boot> "-DEMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
                   -DWORK=${CMAKE_BINARY_DIR} -DUNTIL=0x8035A198
                   -DENTRIES=100000000 -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/twice.cmake)
  set_tests_properties(local_boot_twice PROPERTIES SKIP_RETURN_CODE 77 LABELS local TIMEOUT 600)
  # E0c (R14): a scripted START press at frame 280 changes the run after that
  # frame and not before; two runs with the script are identical.
  add_test(NAME local_input_start
           COMMAND ${CMAKE_COMMAND} -DBOOT=$<TARGET_FILE:mvp-boot> "-DEMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
                   -DWORK=${CMAKE_BINARY_DIR} -DPRESS=280 -DFRAMES=290
                   -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/input.cmake)
  set_tests_properties(local_input_start PROPERTIES SKIP_RETURN_CODE 77 LABELS local TIMEOUT 1800)
endif()
