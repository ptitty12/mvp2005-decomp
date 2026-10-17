# Roster and team packs (E6): format v1 of docs/MODDING.md, the engine's
# implementation. Data-free; the reference is tools/rosters/pack.py.
add_library(engine_mods STATIC json.cpp tables.cpp zip.cpp pack.cpp apply.cpp mount.cpp)
target_link_libraries(engine_mods PUBLIC engine_vfs engine_common)
target_compile_options(engine_mods PRIVATE ${ENGINE_WARN})
# Packs and extracted discs in host directories (native tools, tests): kept out of
# engine_mods so the browser modules carry no std::filesystem or iostreams.
add_library(engine_mods_host STATIC host.cpp)
target_link_libraries(engine_mods_host PUBLIC engine_mods)
target_compile_options(engine_mods_host PRIVATE ${ENGINE_WARN})

# Gameplay mods (hook API v1, docs/mods/ABS_CHALLENGE.md §5): mods that run host
# code at retail-address sites of the running game (runtime/mod_hooks.h).
add_library(engine_gameplay_mods STATIC gameplay/host.cpp gameplay/abs.cpp gameplay/builtin.cpp gameplay/timeline.cpp)
target_link_libraries(engine_gameplay_mods PUBLIC engine_runtime engine_common)
target_compile_options(engine_gameplay_mods PRIVATE ${ENGINE_WARN})
engine_test(test_gameplay SOURCES tests/test_gameplay.cpp LIBS engine_gameplay_mods)

# mvp-pack: pack.py's commands on the engine's implementation (and mount-check).
add_executable(mvp-pack tools/mvp_pack.cpp)
target_link_libraries(mvp-pack PRIVATE engine_mods_host)
target_compile_options(mvp-pack PRIVATE ${ENGINE_WARN})

engine_test(test_mods SOURCES tests/test_mods.cpp LIBS engine_mods_host
            ENV "MVP_REPO_ROOT=${MVP_REPO_ROOT}")

# The cross-check against the reference tool: pack.py's own synthetic disc and
# test packs (tools/rosters/tests/test_pack.py), applied by both; issues, pack,
# set and result hashes and the output files must be identical. Python only
# (standard library); skipped (77) without it. Native builds only: under
# Emscripten mvp-pack runs through Node, which the harness does not drive.
if(NOT EMSCRIPTEN)
  # Local only (the player's disc; skipped, 77, without it): the example pack mounted on
  # the virtual disc, every file read back, and MODDING.md §12's result hash.
  add_test(NAME local_mods_example
           COMMAND mvp-pack mount-check --disc "${MVP_DISC_PATH}"
                   --expect-result 181b6977c6fa8893c3a06968e8d719d260e769e417eb7403cfbaf6ba9cea973f
                   ${MVP_REPO_ROOT}/docs/examples/packs/fictional-league)
  set_tests_properties(local_mods_example PROPERTIES SKIP_RETURN_CODE 77 LABELS local)
  find_package(Python3 COMPONENTS Interpreter QUIET)
  if(Python3_Interpreter_FOUND)
    add_test(NAME mods_crosscheck
             COMMAND ${Python3_EXECUTABLE} -I ${CMAKE_CURRENT_SOURCE_DIR}/tests/crosscheck.py
                     --mvp-pack $<TARGET_FILE:mvp-pack> --repo ${MVP_REPO_ROOT})
    set_tests_properties(mods_crosscheck PROPERTIES SKIP_RETURN_CODE 77)
  endif()
endif()
