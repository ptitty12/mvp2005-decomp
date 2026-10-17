# The browser build (E2): the engine as a Worker module (mvp-web.js + .wasm)
# and the web shell (TypeScript), assembled into one static site directory:
#
#   build/engine/emscripten/web/site/
#     index.html, style.css      the page (static/)
#     main.js, worker.js, ...    the shell (src/*.ts, compiled by tsc)
#     mvp-web.js, mvp-web.wasm   the engine: translator, runtime, renderer
#     mvp-web-mt.js, .wasm       the same engine with a render thread (P2), for
#                                pages with cross-origin isolation; built in a
#                                second tree (build/engine/emscripten/mt)
#     symbols.txt                config/GV4E69/symbols.txt (repository metadata,
#                                not game data: ARCHITECTURE.md §4)
#
# Nothing in it is derived from the disc: the player's browser translates the
# DOL from the image the player picks. tools/serve.mjs serves this directory
# (and only it) on localhost. docs/engine/WEB.md has the details.

set(MVP_WEB_SITE ${CMAKE_CURRENT_BINARY_DIR}/site)

# ---- The engine module ----
# The browser client's caches (E2c), data-free and tested under Node: the
# server image's block cache and the translated module's serialization.
add_library(engine_web_core STATIC wasm/block_cache.cpp wasm/module_cache.cpp)
target_link_libraries(engine_web_core PUBLIC engine_translator engine_runtime)
target_compile_options(engine_web_core PRIVATE ${ENGINE_WARN})
engine_test(test_web_cache SOURCES wasm/tests/test_web_cache.cpp LIBS engine_web_core)
# Development: replays a disc read trace through that cache over a modelled
# network (wasm/tools/cache_sim.cpp), to choose its settings.
add_executable(mvp-web-cache-sim wasm/tools/cache_sim.cpp)
target_link_libraries(mvp-web-cache-sim PRIVATE engine_web_core)
target_compile_options(mvp-web-cache-sim PRIVATE ${ENGINE_WARN})

set(_mvp_web_sources wasm/mvp_web.cpp wasm/web_disc.cpp wasm/net_web.cpp wasm/packs_web.cpp wasm/render_side.cpp)
if(MVP_WEB_THREADS)
  list(APPEND _mvp_web_sources wasm/render_thread.cpp)
endif()
add_executable(mvp-web ${_mvp_web_sources})
target_link_libraries(mvp-web PRIVATE engine_wasm_host engine_audio engine_web_core engine_net engine_mods
                      engine_gameplay_mods)  # gameplay mods (hook API v1)
target_compile_options(mvp-web PRIVATE ${ENGINE_WARN})
if(TARGET engine_gfx_wgpu)
  target_link_libraries(mvp-web PRIVATE engine_gfx_wgpu)
else()
  message(WARNING "web: no browser renderer (emdawnwebgpu missing); mvp-web is not built")
  set_target_properties(mvp-web PROPERTIES EXCLUDE_FROM_ALL ON)
endif()
set_target_properties(mvp-web PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MVP_WEB_SITE})
# The engine-wide Emscripten options target Node (tests and tools); these come
# after them and win: a Worker ES module, no host file system, the runtime
# kept alive between calls, the exports the shell uses.
target_link_options(mvp-web PRIVATE
  -sENVIRONMENT=worker
  -sNODERAWFS=0
  -sEXIT_RUNTIME=0
  -sMODULARIZE=1
  -sEXPORT_ES6=1
  -sEXPORT_NAME=createMvpEngine
  -sALLOW_TABLE_GROWTH=1
  "-sEXPORTED_RUNTIME_METHODS=['UTF8ToString','HEAPU8','specialHTMLTargets']"
  "-sEXPORTED_FUNCTIONS=['_malloc','_free']")

# ---- The threaded engine (P2): this tree is the second one (MVP_WEB_THREADS) ----
# mvp-web-mt.js / .wasm: shared memory; the render thread (render_thread.cpp) is
# a pthread whose Worker is created with the module, before the engine starts.
# wasmMemory: the Worker makes fresh views when another thread grew the memory.
if(MVP_WEB_THREADS)
  set_target_properties(mvp-web PROPERTIES OUTPUT_NAME mvp-web-mt)
  target_compile_definitions(mvp-web PRIVATE MVP_WEB_THREADS=1)
  # (-pthread with memory growth makes JavaScript's heap accesses check for growth; the
  # engine's few are not hot.)
  target_link_options(mvp-web PRIVATE
    -sPTHREAD_POOL_SIZE=1
    -Wno-pthreads-mem-growth
    "-sEXPORTED_RUNTIME_METHODS=['UTF8ToString','HEAPU8','specialHTMLTargets','wasmMemory']")
  return()  # the page, the shell and the rest come from the main tree
endif()

# The page's roster-pack checker (E6): engine/mods alone (no disc, no game), so the
# Roster packs panel shows each pack's problems and hashes before a game starts,
# with the same code the engine applies them with. Also loaded by the unit tests.
add_executable(mvp-packs wasm/packs_web.cpp)
target_link_libraries(mvp-packs PRIVATE engine_mods)
target_compile_options(mvp-packs PRIVATE ${ENGINE_WARN})
set_target_properties(mvp-packs PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MVP_WEB_SITE})
target_link_options(mvp-packs PRIVATE
  -sENVIRONMENT=web,worker,node
  -sNODERAWFS=0
  -sEXIT_RUNTIME=0
  -sMODULARIZE=1
  -sEXPORT_ES6=1
  -sEXPORT_NAME=createMvpPacks
  -sSTACK_SIZE=1MB
  -sINITIAL_MEMORY=16MB
  "-sEXPORTED_RUNTIME_METHODS=['UTF8ToString','HEAPU8']"
  "-sEXPORTED_FUNCTIONS=['_malloc','_free']")
add_dependencies(mvp-web mvp-packs)

# ---- The shell ----
# tsc from engine/web/node_modules (pinned in package.json / package-lock.json;
# installed with `npm ci` in engine/web, docs/engine/TOOLCHAIN.md).
set(MVP_WEB_TSC ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/typescript/bin/tsc)
# The SDK's Node (Emscripten's toolchain file finds it for running the tests).
set(MVP_NODE "${CMAKE_CROSSCOMPILING_EMULATOR}")
file(GLOB MVP_WEB_TS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ts)
file(GLOB MVP_WEB_STATIC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/static/*)
if(EXISTS ${MVP_WEB_TSC} AND MVP_NODE)
  add_custom_command(
    OUTPUT ${MVP_WEB_SITE}/main.js ${MVP_WEB_SITE}/worker.js
    COMMAND ${MVP_NODE} ${MVP_WEB_TSC} -p ${CMAKE_CURRENT_SOURCE_DIR}/tsconfig.json --outDir ${MVP_WEB_SITE}
    DEPENDS ${MVP_WEB_TS} ${CMAKE_CURRENT_SOURCE_DIR}/tsconfig.json
    COMMENT "web: compiling the shell (tsc)"
    VERBATIM)
  set(_mvp_web_js ${MVP_WEB_SITE}/main.js ${MVP_WEB_SITE}/worker.js)
  # The shell's data-free unit tests (E2c): the gamepad mapping, the audio
  # resampler, the disc's server contract; node --test on the compiled modules.
  file(GLOB MVP_WEB_TESTS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.test.mjs)
  add_test(NAME test_web_shell COMMAND ${MVP_NODE} --test ${MVP_WEB_TESTS})
  set_tests_properties(test_web_shell PROPERTIES ENVIRONMENT "MVP_WEB_SITE=${MVP_WEB_SITE};MVP_REPO_ROOT=${MVP_REPO_ROOT}")
else()
  message(STATUS "web: TypeScript not installed in engine/web/node_modules (npm ci); the shell is not compiled")
  set(_mvp_web_js)
endif()
set(_mvp_web_copies)
foreach(f ${MVP_WEB_STATIC})
  get_filename_component(n ${f} NAME)
  add_custom_command(OUTPUT ${MVP_WEB_SITE}/${n}
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${f} ${MVP_WEB_SITE}/${n}
                     DEPENDS ${f} VERBATIM)
  list(APPEND _mvp_web_copies ${MVP_WEB_SITE}/${n})
endforeach()
add_custom_command(OUTPUT ${MVP_WEB_SITE}/symbols.txt
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MVP_REPO_ROOT}/config/GV4E69/symbols.txt
                           ${MVP_WEB_SITE}/symbols.txt
                   DEPENDS ${MVP_REPO_ROOT}/config/GV4E69/symbols.txt VERBATIM)
# Notices for the third-party code compiled into mvp-web.js/.wasm (docs/REUSE.md
# §6): Emscripten's runtime and libraries, and emdawnwebgpu (Dawn's BSD-3-Clause
# C++ side; its JS side is under Emscripten's license).
set(_mvp_web_notices)
if(EMSCRIPTEN_ROOT_PATH AND EXISTS "${EMSCRIPTEN_ROOT_PATH}/LICENSE")
  list(APPEND _mvp_web_notices "${EMSCRIPTEN_ROOT_PATH}/LICENSE=emscripten.txt")
endif()
if(MVP_EMDAWNWEBGPU_DIR AND EXISTS "${MVP_EMDAWNWEBGPU_DIR}/webgpu_cpp/LICENSE")
  list(APPEND _mvp_web_notices "${MVP_EMDAWNWEBGPU_DIR}/webgpu_cpp/LICENSE=dawn-emdawnwebgpu.txt")
endif()
foreach(pair ${_mvp_web_notices})
  string(REPLACE "=" ";" pair "${pair}")
  list(GET pair 0 src)
  list(GET pair 1 name)
  add_custom_command(OUTPUT ${MVP_WEB_SITE}/licenses/${name}
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${MVP_WEB_SITE}/licenses/${name}
                     DEPENDS ${src} VERBATIM)
  list(APPEND _mvp_web_copies ${MVP_WEB_SITE}/licenses/${name})
endforeach()

# build-id.json: the engine modules' and symbols.txt's hashes, the key of the
# browser's cache of the translated module (E2c; P2: the threaded engine's too).
set(_mvp_build_id ${CMAKE_COMMAND} -DWASM=${MVP_WEB_SITE}/mvp-web.wasm -DWASM_MT=${MVP_WEB_SITE}/mvp-web-mt.wasm
                  -DSYMBOLS=${MVP_REPO_ROOT}/config/GV4E69/symbols.txt
                  -DOUT=${MVP_WEB_SITE}/build-id.json -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/build_id.cmake)
add_custom_command(TARGET mvp-web POST_BUILD COMMAND ${_mvp_build_id} VERBATIM)

# The threaded engine (P2), built by the same build in a second tree with every
# library compiled for wasm threads (MVP_WEB_THREADS), after this tree's
# libraries (so the two compiles do not run at once), and copied into the site.
option(MVP_WEB_MT "Build the threaded engine mvp-web-mt in a second tree (P2)" ON)
if(MVP_WEB_MT AND TARGET engine_gfx_wgpu)
  include(ExternalProject)
  cmake_host_system_information(RESULT _mvp_jobs QUERY NUMBER_OF_LOGICAL_CORES)
  if(_mvp_jobs GREATER 8)
    set(_mvp_jobs 8)
  endif()
  set(_mvp_mt ${CMAKE_BINARY_DIR}/mt)
  ExternalProject_Add(mvp-web-mt
    SOURCE_DIR ${PROJECT_SOURCE_DIR}
    BINARY_DIR ${_mvp_mt}
    PREFIX ${CMAKE_BINARY_DIR}/mt-ep
    CMAKE_GENERATOR ${CMAKE_GENERATOR}
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_MAKE_PROGRAM}
      -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
      -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
      -DMVP_WEB_THREADS:BOOL=ON
      -DMVP_GFX_WEBGPU:BOOL=ON
      -DMVP_EMDAWNWEBGPU_DIR:PATH=${MVP_EMDAWNWEBGPU_DIR}
      -DMVP_DISC:FILEPATH=
    BUILD_COMMAND ${CMAKE_COMMAND} --build ${_mvp_mt} --target mvp-web --parallel ${_mvp_jobs}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_mvp_mt}/web/site/mvp-web-mt.js
                    ${_mvp_mt}/web/site/mvp-web-mt.wasm ${MVP_WEB_SITE}
            COMMAND ${_mvp_build_id}
    BUILD_ALWAYS ON
    USES_TERMINAL_BUILD ON
    DEPENDS engine_wasm_host engine_gfx_wgpu engine_audio engine_web_core engine_net engine_mods)
  add_dependencies(mvp-web mvp-web-mt)
endif()

add_custom_target(mvp-web-shell ALL DEPENDS ${_mvp_web_js} ${_mvp_web_copies} ${MVP_WEB_SITE}/symbols.txt)
add_dependencies(mvp-web mvp-web-shell)
