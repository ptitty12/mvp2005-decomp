# Renderer (E1): the GX command stream rendered with WebGPU.
#
# engine_gfx is the data-free core (GX state machine, vertex decoding and
# transform, texture decoders, TEV -> WGSL); it needs nothing outside the repo
# and its tests run everywhere. engine_gfx_wgpu is the WebGPU backend: Dawn
# natively (SDL3 for the window), browser WebGPU through Emscripten's
# emdawnwebgpu port from the same Dawn release (E2; a canvas instead of the
# window). Dawn, SDL3 and emdawnwebgpu are SDKs outside the repository, pinned
# in docs/engine/TOOLCHAIN.md; without them the backend is skipped and the rest
# of the engine builds as before.

add_library(engine_gfx STATIC png.cpp gx_processor.cpp texture_decode.cpp tev_wgsl.cpp render_ring.cpp gp_reads.cpp gp_memory.cpp)
target_link_libraries(engine_gfx PUBLIC engine_common)
target_compile_options(engine_gfx PRIVATE ${ENGINE_WARN})
# P2: the render thread's command ring (render_ring.h), with a producer and a
# consumer thread (native; the browser build without threads has none).
if(NOT EMSCRIPTEN)
  find_package(Threads REQUIRED)
  engine_test(test_render_ring SOURCES tests/test_render_ring.cpp LIBS engine_gfx Threads::Threads)
endif()

engine_test(test_gfx_png SOURCES tests/test_png.cpp LIBS engine_gfx)
engine_test(test_tev_wgsl SOURCES tests/test_tev_wgsl.cpp LIBS engine_gfx)
engine_test(test_texture_decode SOURCES tests/test_texture_decode.cpp LIBS engine_gfx)
engine_test(test_gx_processor SOURCES tests/test_gx_processor.cpp LIBS engine_gfx)
target_include_directories(test_gx_processor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# V5: the fast vertex path against the reference path, bit for bit, on random streams.
engine_test(test_vertex_exact SOURCES tests/test_vertex_exact.cpp LIBS engine_gfx)
target_include_directories(test_vertex_exact PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# P2: the memory each command makes the renderer read (gp_reads.h).
engine_test(test_gp_reads SOURCES tests/test_gp_reads.cpp LIBS engine_gfx)
# Q1: what the GP reads of memory, after the data cache (gp_memory.h).
engine_test(test_gp_memory SOURCES tests/test_gp_memory.cpp LIBS engine_gfx)
target_include_directories(test_gp_memory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_gp_reads PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# The runtime's hook (runtime/gx_fifo.h) feeding the processor.
engine_test(test_gfx_hook SOURCES tests/test_gfx_hook.cpp LIBS engine_gfx engine_runtime)
target_include_directories(test_gfx_hook PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Development (P1): the GX processor's cost per primitive and vertex on a
# synthetic in-play stream (tools/gx_bench.cpp); under Node with the Emscripten preset.
add_executable(mvp-gx-bench tools/gx_bench.cpp)
target_link_libraries(mvp-gx-bench PRIVATE engine_gfx)
target_compile_options(mvp-gx-bench PRIVATE ${ENGINE_WARN})
if(EMSCRIPTEN)
  target_link_options(mvp-gx-bench PRIVATE --profiling-funcs)  # names in V8's profiler (node --cpu-prof)
endif()

# ---- WebGPU backend (Dawn + SDL3) ----
# Pinned versions (docs/engine/TOOLCHAIN.md). The defaults point at the pinned
# packages under %USERPROFILE%\tools-ext; MVP_DAWN_DIR / MVP_SDL3_DIR (cache or
# environment) override them.
set(MVP_DAWN_RELEASE "v20261013.193955")
set(MVP_DAWN_COMMIT "f63db59722e8c1c0da9a3bcea0e40fb5c817a059")
set(MVP_SDL3_VERSION "3.4.18")
file(TO_CMAKE_PATH "$ENV{USERPROFILE}" _mvp_home)
set(_mvp_dawn_default "${_mvp_home}/tools-ext/dawn/${MVP_DAWN_RELEASE}/Dawn-${MVP_DAWN_COMMIT}-windows-latest-Release")
set(_mvp_sdl3_default "${_mvp_home}/tools-ext/sdl3/${MVP_SDL3_VERSION}/SDL3-${MVP_SDL3_VERSION}")
if(NOT "$ENV{MVP_DAWN_DIR}" STREQUAL "")
  set(_mvp_dawn_default "$ENV{MVP_DAWN_DIR}")
endif()
if(NOT "$ENV{MVP_SDL3_DIR}" STREQUAL "")
  set(_mvp_sdl3_default "$ENV{MVP_SDL3_DIR}")
endif()
set(MVP_DAWN_DIR "${_mvp_dawn_default}" CACHE PATH "Dawn release package (the directory holding lib/cmake/Dawn)")
set(MVP_SDL3_DIR "${_mvp_sdl3_default}" CACHE PATH "SDL3 development package (the directory holding cmake/SDL3Config.cmake)")
option(MVP_GFX_WEBGPU "Build the WebGPU renderer when Dawn and SDL3 (native) or emdawnwebgpu (Emscripten) are found" ON)

set(MVP_HAVE_WEBGPU OFF)
if(MVP_GFX_WEBGPU AND EMSCRIPTEN)
  # Browser (E2): emdawnwebgpu, the webgpu.h of the same Dawn release, as an
  # Emscripten "local port" (text only: C++ and JS sources, built by emcc).
  set(_mvp_emdawn_default "${_mvp_home}/tools-ext/emdawnwebgpu/${MVP_DAWN_RELEASE}/emdawnwebgpu_pkg")
  if(NOT "$ENV{MVP_EMDAWNWEBGPU_DIR}" STREQUAL "")
    set(_mvp_emdawn_default "$ENV{MVP_EMDAWNWEBGPU_DIR}")
  endif()
  set(MVP_EMDAWNWEBGPU_DIR "${_mvp_emdawn_default}" CACHE PATH "emdawnwebgpu_pkg of the pinned Dawn release (the directory holding emdawnwebgpu.port.py)")
  if(EXISTS "${MVP_EMDAWNWEBGPU_DIR}/emdawnwebgpu.port.py")
    message(STATUS "gfx: WebGPU renderer with emdawnwebgpu ${MVP_DAWN_RELEASE} (browser)")
    add_library(engine_gfx_wgpu STATIC wgpu/gpu.cpp wgpu/gpu_timer.cpp wgpu/window.cpp wgpu/renderer.cpp)
    set(_mvp_emdawn_port "--use-port=${MVP_EMDAWNWEBGPU_DIR}/emdawnwebgpu.port.py")
    target_compile_options(engine_gfx_wgpu PUBLIC ${_mvp_emdawn_port})
    target_link_options(engine_gfx_wgpu INTERFACE ${_mvp_emdawn_port})
    target_link_libraries(engine_gfx_wgpu PUBLIC engine_gfx)
    target_compile_options(engine_gfx_wgpu PRIVATE ${ENGINE_WARN})
  else()
    message(STATUS "gfx: emdawnwebgpu not found (MVP_EMDAWNWEBGPU_DIR=${MVP_EMDAWNWEBGPU_DIR}); "
                   "the browser renderer is not built")
  endif()
elseif(MVP_GFX_WEBGPU)
  if(EXISTS "${MVP_DAWN_DIR}/lib/cmake/Dawn/DawnConfig.cmake" AND EXISTS "${MVP_SDL3_DIR}/cmake/SDL3Config.cmake")
    set(MVP_HAVE_WEBGPU ON)
  else()
    message(STATUS "gfx: Dawn or SDL3 not found (MVP_DAWN_DIR=${MVP_DAWN_DIR}, MVP_SDL3_DIR=${MVP_SDL3_DIR}); "
                   "the WebGPU renderer is not built")
  endif()
endif()

if(MVP_HAVE_WEBGPU)
  find_package(Threads REQUIRED)
  find_package(Dawn CONFIG REQUIRED PATHS "${MVP_DAWN_DIR}/lib/cmake/Dawn" NO_DEFAULT_PATH)
  find_package(SDL3 CONFIG REQUIRED PATHS "${MVP_SDL3_DIR}/cmake" NO_DEFAULT_PATH)
  message(STATUS "gfx: WebGPU renderer with Dawn ${MVP_DAWN_RELEASE} and SDL3 ${MVP_SDL3_VERSION}")

  set(_mvp_wgpu_sources wgpu/gpu.cpp wgpu/gpu_timer.cpp wgpu/window.cpp wgpu/renderer.cpp)
  # The prebuilt Dawn may come from a newer MSVC STL than the installed one
  # (wgpu/msvc_stl_compat.cpp explains); add the forwarding shim only when
  # the toolset lacks the split min/max_element entry points.
  if(MSVC)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles([[
      extern "C" const void* __stdcall __std_min_element_4u(const void*, const void*) noexcept;
      int main() { int a[2] = {1, 0}; return __std_min_element_4u(a, a + 2) == a + 1 ? 0 : 1; }
    ]] MVP_MSVC_HAS_SPLIT_MINMAX)
    if(NOT MVP_MSVC_HAS_SPLIT_MINMAX)
      list(APPEND _mvp_wgpu_sources wgpu/msvc_stl_compat.cpp)
    endif()
  endif()
  add_library(engine_gfx_wgpu STATIC ${_mvp_wgpu_sources})
  target_link_libraries(engine_gfx_wgpu PUBLIC engine_gfx dawn::webgpu_dawn SDL3::SDL3)
  target_compile_options(engine_gfx_wgpu PRIVATE ${ENGINE_WARN})

  # Dawn's D3D12 backend loads the DirectX shader compiler (dxcompiler.dll and
  # dxil.dll) from the executable's directory. The Windows SDK that the MSVC
  # toolchain already uses ships both; MVP_DXC_DIR overrides it.
  set(_mvp_dxc_default "")
  if(WIN32)
    if(NOT "$ENV{WindowsSdkVerBinPath}" STREQUAL "" AND EXISTS "$ENV{WindowsSdkVerBinPath}/x64/dxcompiler.dll")
      file(TO_CMAKE_PATH "$ENV{WindowsSdkVerBinPath}/x64" _mvp_dxc_default)
    else()
      file(TO_CMAKE_PATH "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin" _mvp_kits_bin)
      file(GLOB _mvp_dxc_found "${_mvp_kits_bin}/10.*/x64/dxcompiler.dll")
      if(_mvp_dxc_found)
        list(SORT _mvp_dxc_found COMPARE NATURAL)
        list(GET _mvp_dxc_found -1 _mvp_dxc_last)
        get_filename_component(_mvp_dxc_default "${_mvp_dxc_last}" DIRECTORY)
      endif()
    endif()
  endif()
  set(MVP_DXC_DIR "${_mvp_dxc_default}" CACHE PATH "Directory holding dxcompiler.dll and dxil.dll (default: the Windows SDK's)")
  set(_mvp_runtime_dlls $<TARGET_FILE:SDL3::SDL3>)
  if(WIN32)
    if(EXISTS "${MVP_DXC_DIR}/dxcompiler.dll" AND EXISTS "${MVP_DXC_DIR}/dxil.dll")
      list(APPEND _mvp_runtime_dlls "${MVP_DXC_DIR}/dxcompiler.dll" "${MVP_DXC_DIR}/dxil.dll")
      message(STATUS "gfx: DirectX shader compiler from ${MVP_DXC_DIR}")
    else()
      message(STATUS "gfx: no dxcompiler.dll/dxil.dll found (MVP_DXC_DIR); D3D12 device creation will fail")
    endif()
  endif()

  # Copy the runtime DLLs next to every executable that uses the backend.
  function(mvp_copy_sdl3 target)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_mvp_runtime_dlls} $<TARGET_FILE_DIR:${target}>
      VERBATIM)
  endfunction()

  # Needs a GPU adapter; exits 77 (skipped) where there is none.
  engine_test(test_gfx_gpu SOURCES tests/test_gpu.cpp LIBS engine_gfx_wgpu)
  mvp_copy_sdl3(test_gfx_gpu)
  engine_test(test_gfx_render SOURCES tests/test_gfx_render.cpp LIBS engine_gfx_wgpu)
  target_include_directories(test_gfx_render PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  mvp_copy_sdl3(test_gfx_render)

  # mvp-play: the game with the renderer (needs the translated code, so the disc).
  if(TARGET mvp_game_native)
    add_executable(mvp-play tools/mvp_play.cpp)
    target_link_libraries(mvp-play PRIVATE mvp_game_native engine_gfx_wgpu engine_translator engine_audio_sdl  # audio/ (E3b)
                          engine_mods_host  # --pack (E6)
                          engine_gameplay_mods)  # --mod (hook API v1)
    target_compile_options(mvp-play PRIVATE ${ENGINE_WARN})
    if(MSVC)
      target_link_options(mvp-play PRIVATE /STACK:67108864)
    endif()
    mvp_copy_sdl3(mvp-play)
    # The renderer must not change the guest: same guest state with and without it.
    add_test(NAME local_play_determinism
             COMMAND ${CMAKE_COMMAND} -DPLAY=$<TARGET_FILE:mvp-play> -DWORK=${CMAKE_BINARY_DIR} -DFIELDS=700
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/play_twice.cmake)
    set_tests_properties(local_play_determinism PROPERTIES SKIP_RETURN_CODE 77 LABELS local TIMEOUT 900)
  endif()
endif()
