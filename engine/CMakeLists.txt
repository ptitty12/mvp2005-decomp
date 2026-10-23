# MVP Baseball 2005 browser engine: native development build.
#
# Layout and design: ../docs/ARCHITECTURE.md (§11) and ../docs/engine/E0_SURVEY.md (§8).
# Everything derived from the disc (extracted DOL, translated C++, traces) is
# written under the build tree, never into the source tree.
cmake_minimum_required(VERSION 3.25)
project(mvp_engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optional local data for tests that need the disc. Unset means those tests
# report "skipped" (exit code 77). They default to the environment variables of
# the same name at configure time, and those also override them at test time.
set(MVP_DISC "$ENV{MVP_DISC}" CACHE FILEPATH "Player's GV4E69 disc image (ISO or CISO), for local-only tests")
set(MVP_DTK_ASM "$ENV{MVP_DTK_ASM}" CACHE PATH "dtk split assembly directory (build/GV4E69/asm), for the local disassembler check")

# Forward-slash copies for C string literals (compile definitions).
file(TO_CMAKE_PATH "${MVP_DISC}" MVP_DISC_PATH)
file(TO_CMAKE_PATH "${MVP_DTK_ASM}" MVP_DTK_ASM_PATH)

# Repository root: config/GV4E69 (symbols.txt, splits.txt) is metadata, not game data.
get_filename_component(MVP_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# Deterministic floating point on the host: no contraction into FMA, no fast-math.
if(MSVC)
  set(ENGINE_WARN /W4 /permissive- /utf-8 /Zc:__cplusplus /Zc:preprocessor /fp:precise /EHsc /bigobj)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
else()
  set(ENGINE_WARN -Wall -Wextra -ffp-contract=off -fno-fast-math)
endif()

# Browser build (E2, preset "emscripten"): wasm32, run under Node.
# - Native WebAssembly exceptions: the runtime still throws to end a run
#   (GuestFault, StopRun); unwinding for thread switches needs none (E0c).
# - NODERAWFS: tools and local tests read the host file system directly (the
#   disc, symbols.txt), as their native builds do.
# - A large stack: guest call chains nest translated frames deeply.
# - EXIT_RUNTIME: the exit code reaches ctest (77 = skipped).
if(EMSCRIPTEN)
  add_compile_options(-fwasm-exceptions)
  add_link_options(-fwasm-exceptions -sNODERAWFS=1 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB -sMAXIMUM_MEMORY=4GB -sSTACK_SIZE=16MB
                   -sEXIT_RUNTIME=1 -sENVIRONMENT=node)
  # P2: the threaded browser engine (mvp-web-mt) is built in a second tree with
  # everything compiled for wasm threads (shared memory, atomics); the main tree
  # builds that tree as part of mvp-web (web/CMakeLists.txt). Its targets are
  # the same; only mvp-web is used from it.
  option(MVP_WEB_THREADS "Emscripten: compile everything with wasm threads (-pthread), for mvp-web-mt (P2)" OFF)
  if(MVP_WEB_THREADS)
    add_compile_options(-pthread)
    add_link_options(-pthread)
  endif()
endif()

enable_testing()

# Test framework (doctest, MIT, vendored: see docs/REUSE.md).
add_library(engine_doctest_main STATIC third_party/doctest_main.cpp)
target_include_directories(engine_doctest_main PUBLIC third_party)

# Tests that need local data exit 77 when it is missing; ctest reports them as skipped.
function(engine_test name)
  cmake_parse_arguments(ET "" "" "SOURCES;LIBS;ENV" ${ARGN})
  add_executable(${name} ${ET_SOURCES})
  target_link_libraries(${name} PRIVATE engine_doctest_main ${ET_LIBS})
  target_compile_options(${name} PRIVATE ${ENGINE_WARN})
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES SKIP_RETURN_CODE 77)
  if(ET_ENV)
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "${ET_ENV}")
  endif()
endfunction()

add_subdirectory(third_party/pureikyubu)
add_subdirectory(third_party/zstd)
add_subdirectory(common)
add_subdirectory(vfs)
add_subdirectory(mods)
add_subdirectory(ppc)
add_subdirectory(translator)
add_subdirectory(runtime)
add_subdirectory(gfx)
add_subdirectory(audio)
add_subdirectory(net)
if(NOT EMSCRIPTEN)
  add_subdirectory(server)
endif()
# The browser build (E2): the engine as a Worker module and the web shell.
if(EMSCRIPTEN)
  add_subdirectory(web)
endif()
