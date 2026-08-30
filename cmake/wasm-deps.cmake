# GeneralsX @build meerzulee 05/07/2026 Emscripten dependency strategy without vcpkg.
# Upstream reference: meerzulee/GeneralsXWeb, commit e44c34794
# https://github.com/meerzulee/GeneralsXWeb/commit/e44c34794
# glm/gli: header-only, fetched directly (their own CMake is skipped — Populate only).
# freetype: Emscripten built-in port (-sUSE_FREETYPE=1 at compile and link).
# fontconfig: local stub resolving every font query to the bundled /fonts/default.ttf.
# openal/curl: not used on wasm (miniaudio audio, update check off).

if(NOT EMSCRIPTEN)
    return()
endif()

include(FetchContent)

# ---- glm (header-only) ----
FetchContent_Declare(
    glm_src
    URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.tar.gz
    URL_HASH SHA256=9f3174561fd26904b23f0db5e560971cbf9b3cbda0b280f04d5c379d03bf234c
)
FetchContent_GetProperties(glm_src)
if(NOT glm_src_POPULATED)
    FetchContent_Populate(glm_src)
endif()
if(NOT TARGET glm::glm)
    add_library(glm_headers INTERFACE)
    target_include_directories(glm_headers INTERFACE ${glm_src_SOURCE_DIR})
    add_library(glm::glm ALIAS glm_headers)
endif()

# ---- gli (header-only) ----
FetchContent_Declare(
    gli_src
    URL https://github.com/g-truc/gli/archive/refs/tags/0.8.2.0.tar.gz
    URL_HASH SHA256=9e7024c2df77c011eff4f66667c1834620c70b7902cd50f32ab48edd49fe0139
)
FetchContent_GetProperties(gli_src)
if(NOT gli_src_POPULATED)
    FetchContent_Populate(gli_src)
endif()
if(NOT TARGET gli)
    add_library(gli INTERFACE)
    target_include_directories(gli INTERFACE ${gli_src_SOURCE_DIR})
endif()

# ---- freetype via Emscripten port ----
# The port supplies headers at compile time and the library at link time.
add_compile_options("-sUSE_FREETYPE=1")
add_link_options("-sUSE_FREETYPE=1")

# ---- fontconfig stub ----
add_library(fontconfig_stub STATIC ${CMAKE_SOURCE_DIR}/wasm/fontconfig_stub/fontconfig_stub.c)
target_include_directories(fontconfig_stub PUBLIC ${CMAKE_SOURCE_DIR}/wasm/fontconfig_stub)
# Header visible everywhere; the stub library is linked explicitly by WW3D2
# (a global link_libraries() here would leak into SDL3's exported targets).
include_directories(${CMAKE_SOURCE_DIR}/wasm/fontconfig_stub)

message(STATUS "wasm-deps: glm/gli fetched, freetype via emscripten port, fontconfig stubbed")

# ---- GameSpy SDK platform identity ----
# The vendored GameSpy SDK detects platforms via __linux__ and defines _LINUX
# internally; Emscripten defines neither. Its Linux/POSIX paths compile fine
# against Emscripten's musl headers (sockets exist at compile time; the service
# is dead anyway and the transport gets replaced with WebRTC later).
# GameSpy platform defines live in cmake/gamespy.cmake (PUBLIC on the gamespy target).

# ---- libc gap shim ----
# Force-included into every TU: wcslcpy/wcslcat (BSD functions macOS has, musl lacks).
add_compile_options("SHELL:-include ${CMAKE_SOURCE_DIR}/wasm/wasm_compat.h")

# ---- C++ exceptions ----
# SAGE throws C++ exceptions as control flow (INI parse errors, MetaMap label
# translation). Without wasm exception support every throw aborts the runtime.
add_compile_options("-fwasm-exceptions")
add_link_options("-fwasm-exceptions")

# ---- Emscripten link configuration ----
# GROWABLE_ARRAYBUFFERS=0: Chrome rejects resizable ArrayBuffer views in WebGL
# upload calls (learned the hard way in d8web).
# INITIAL_MEMORY 512MB + growth: SAGE's memory pools want a large heap up front.
# STACK_SIZE 8MB: the engine assumes Windows-sized thread stacks.
# Link-time: binaryen's -O3 post-link passes SIGABRT on this binary when wasm
# exceptions are enabled; -O1 + stripped DWARF avoids the crashing pass (compile
# optimization stays -O3).
# --profiling-funcs keeps the wasm name section (~1MB): browser stack traces
# name the C++ frame instead of "wasm-function[12345]" — the Safari
# quit-crash hunt needed exactly that.
add_link_options("-O1" "-g0" "--profiling-funcs")
add_link_options(
    "-sALLOW_MEMORY_GROWTH=1"
    "-sGROWABLE_ARRAYBUFFERS=0"
    "-sINITIAL_MEMORY=536870912"
    # 4GB (the wasm32 ceiling): the staged game files live in the wasm heap
    # via MEMFS — ~1.9GB with audio — plus engine pools plus a loaded
    # mission overran the old 2GB cap. Failed growth -> null malloc ->
    # out-of-bounds writes (Safari trapped on quit-to-menu; Chrome likely
    # corrupted silently).
    "-sMAXIMUM_MEMORY=4294967296"
    "-sSTACK_SIZE=8388608"
    "-sEXIT_RUNTIME=0"
)

# ---- d8web: D3D8 to WebGL2 translation layer + engine bridge ----
# GeneralsX @build Copilot 30/08/2026 Pin the renderer instead of requiring a
# sibling checkout so local and CI builds consume the same implementation.
FetchContent_Declare(
    dvijoke
    GIT_REPOSITORY https://github.com/meerzulee/dvijoke.git
    GIT_TAG        dd699bf133ca820833150c373be892e3efb78e69
    GIT_SHALLOW    FALSE
)
FetchContent_GetProperties(dvijoke)
if(NOT dvijoke_POPULATED)
    FetchContent_Populate(dvijoke)
endif()
add_subdirectory("${dvijoke_SOURCE_DIR}/d8web" d8web EXCLUDE_FROM_ALL)
# (the d8web_bridge target is created next to z_generals, where the engine's
# d3d8lib interface target with the DXVK/CompatLib include set already exists)
