# DirectX 8 headers and rendering backend selection
# GeneralsX @build BenderAI 10/02/2026 - Session 18 (updated 26/02/2026 Phase 6, 24/02/2026 Phase 5)
# Fighter19's approach: Fetch ONE OR THE OTHER, never both
#
# On Windows (vc6/win32):    Use min-dx8-sdk (native DirectX 8)
# On Windows (win64-modern): Use DXVK headers + runtime d3d8.dll (DirectX→Vulkan)
# On Linux:                  Use DXVK pre-built tarball (DirectX→Vulkan translation)
# On macOS:                  Build DXVK from source using Meson + MoltenVK (DirectX→Metal bridge)
#
# CRITICAL: Mixing headers causes conflicts - dx8-src has incomplete types,
# DXVK has full DirectX8+Wine headers. Compiler picks first path = wrong headers.
# Use platform guards to ensure single header path per platform.
#
# macOS DXVK build (Session 61, 24/02/2026):
#   DXVK 2.6 builds natively on macOS arm64 via its "native" build mode.
#   Patches applied automatically by cmake/dxvk-macos-patches.py:
#   1. include/native/windows/windows_base.h and src/util/util_win32_compat.h:
#      __unix__ is NOT defined on macOS; add __APPLE__ to the #if guard.
#   2. src/util/util_env.cpp:
#      pthread_setname_np on macOS takes only 1 arg (name), not 2.
#   3. src/util/util_small_vector.h:
#      lzcnt(n-1) ambiguity between uint32_t/uint64_t; cast to uint64_t.
#   4. src/util/util_bit.h:
#      Add uintptr_t overloads for tzcnt/lzcnt (macOS arm64 ambiguity).
#   5. src/{dxgi,d3d8,d3d9,d3d10,d3d11}/meson.build:
#      -Wl,--version-script is GNU ld only; guard with platform != 'darwin'.
#
# Reference: docs/WORKDIR/lessons/2026-02-LESSONS.md (Session 61)

# GeneralsX @build BenderAI 28/05/2026 - Align DXVK_VERSION with Windows runtime DLLs (was v2.6)
set(DXVK_VERSION "v2.7.1")
# Strip leading 'v' for use in filenames (e.g. dxvk-native-2.7.1-steamrt-sniper.tar.gz)
string(REGEX REPLACE "^v" "" DXVK_VERSION_BARE "${DXVK_VERSION}")

if(SAGE_USE_DX8)
  # ── Windows Legacy (vc6/win32): DirectX 8 native ──────────────────────────────
  FetchContent_Declare(
    dx8
    GIT_REPOSITORY https://github.com/TheSuperHackers/min-dx8-sdk.git
    GIT_TAG        7bddff8c01f5fb931c3cb73d4aa8e66d303d97bc
  )
  FetchContent_MakeAvailable(dx8)
  message(STATUS "Using DirectX 8 SDK (Windows native, legacy)")

elseif(WIN32)
  # ── Windows Modern (win64-modern): DXVK headers for D3D8→Vulkan ───────────────
  # Compile against DX90SDK d3d8.h headers (jmarshall's proven MSVC x64 headers).
  # At runtime, DXVK's d3d8.dll (deployed alongside EXE) intercepts D3D8 calls
  # and translates them to Vulkan. This is identical to how Proton handles D3D8→Vulkan.
  # We do NOT link against DXVK .lib files — the DLL is loaded dynamically.
  # Note: Phase 6 places DXVK Windows DLLs alongside the EXE at runtime.
  # GeneralsX @build BenderAI 26/02/2026 - Phase 6 Windows Modern stack

  FetchContent_Declare(
    dxvk_headers
    GIT_REPOSITORY https://github.com/doitsujin/dxvk.git
    GIT_TAG        ${DXVK_VERSION}
    GIT_SHALLOW    TRUE
  )

  # Fetch only headers (no CMakeLists.txt in subdirectory)
  FetchContent_GetProperties(dxvk_headers)
  if(NOT dxvk_headers_POPULATED)
    FetchContent_Populate(dxvk_headers)
  endif()

  # Download Windows release binaries (contains d3d8.dll for deployment)
  FetchContent_Declare(
    dxvk_win_release
    URL https://github.com/doitsujin/dxvk/releases/download/v2.7.1/dxvk-2.7.1.tar.gz
    URL_HASH SHA256=d85ce7c79f57ecd765aaa1b9e7007cb875e6fde9f6d331df799bce73d513ce87
  )
  FetchContent_MakeAvailable(dxvk_win_release)

  # GeneralsX @build BenderAI 28/02/2026 Strategy:
  # Use jmarshall's DX90SDK headers (MSVC x64-proven) instead of DXVK native headers
  # (which have MinGW types like __MSABI_LONG incompatible with MSVC x64).
  # D3D8 headers come from Code/Libraries/DX90SDK (copied from jmarshall-win64-modern).
  if(EXISTS "${CMAKE_SOURCE_DIR}/Core/Libraries/DX90SDK/Include")
    set(DXVK_D3D8_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Core/Libraries/DX90SDK/Include" CACHE PATH "D3D8/D3D9 headers (jmarshall DX90SDK)" FORCE)
    message(STATUS "Using D3D8 headers from DX90SDK (Core/Libraries): ${DXVK_D3D8_INCLUDE_DIR}")
  elseif(EXISTS "${CMAKE_SOURCE_DIR}/GeneralsMD/Code/Libraries/DX90SDK/Include")
    set(DXVK_D3D8_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/GeneralsMD/Code/Libraries/DX90SDK/Include" CACHE PATH "D3D8/D3D9 headers (jmarshall DX90SDK)" FORCE)
    message(STATUS "Using D3D8 headers from DX90SDK (GeneralsMD): ${DXVK_D3D8_INCLUDE_DIR}")
  elseif(EXISTS "${CMAKE_SOURCE_DIR}/Generals/Code/Libraries/DX90SDK/Include")
    set(DXVK_D3D8_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Generals/Code/Libraries/DX90SDK/Include" CACHE PATH "D3D8/D3D9 headers (jmarshall DX90SDK)" FORCE)
    message(STATUS "Using D3D8 headers from DX90SDK (Generals): ${DXVK_D3D8_INCLUDE_DIR}")
  else()
    # Fallback: DXVK native headers (NOT RECOMMENDED - may have MinGW type conflicts on MSVC x64)
    set(DXVK_D3D8_INCLUDE_DIR "${dxvk_headers_SOURCE_DIR}/include/native/directx" CACHE PATH "D3D8 headers (DXVK fallback)" FORCE)
    message(WARNING "DX90SDK not found. Falling back to DXVK native headers (may cause MSVC x64 issues).")
  endif()

  set(DXVK_INCLUDE_DIR "${dxvk_headers_SOURCE_DIR}/include" CACHE PATH "DXVK Vulkan headers" FORCE)
  set(DXVK_WIN_DLL_DIR "${dxvk_win_release_SOURCE_DIR}/x64" CACHE PATH "DXVK Windows DLLs (x64)")

  message(STATUS "Using DXVK for Windows Modern (D3D8→Vulkan at runtime via d3d8.dll)")
  message(STATUS "  D3D8 headers: ${DXVK_D3D8_INCLUDE_DIR}")
  message(STATUS "  DXVK Vulkan headers: ${DXVK_INCLUDE_DIR}")
  message(STATUS "  DXVK DLL dir: ${DXVK_WIN_DLL_DIR}")

elseif(APPLE AND SAGE_USE_MOLTENVK)
  # ── macOS (macos-vulkan): Build DXVK from source via Meson + MoltenVK ─────────
  # GeneralsX @build BenderAI 24/02/2026 - Phase 5 macOS port (Session 61)
  find_program(MESON_EXECUTABLE meson HINTS /usr/local/bin /opt/homebrew/bin)
  find_program(NINJA_EXECUTABLE ninja HINTS /usr/local/bin /opt/homebrew/bin)

  if(NOT MESON_EXECUTABLE)
    message(FATAL_ERROR "DXVK macOS build requires meson: brew install meson")
  endif()
  if(NOT NINJA_EXECUTABLE)
    message(FATAL_ERROR "DXVK macOS build requires ninja: brew install ninja")
  endif()

  # Detect host architecture so Clang targets the correct slice.
  # IMPORTANT: prefer CMAKE_OSX_ARCHITECTURES (set by the preset) over uname -m.
  # On Apple Silicon Macs running CMake / meson via Rosetta, uname -m returns
  # x86_64 even though the native executable arch is arm64.
  if(CMAKE_OSX_ARCHITECTURES)
    list(GET CMAKE_OSX_ARCHITECTURES 0 DXVK_HOST_ARCH)
  else()
    execute_process(
      COMMAND uname -m
      OUTPUT_VARIABLE DXVK_HOST_ARCH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()
  message(STATUS "Building DXVK ${DXVK_VERSION} for macOS/${DXVK_HOST_ARCH} with Meson (${MESON_EXECUTABLE})")

  include(ExternalProject)
  set(DXVK_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/dxvk-src")
  set(DXVK_BUILD_DIR  "${CMAKE_BINARY_DIR}/_deps/dxvk-build-macos")
  set(DXVK_D3D8_LIB  "${DXVK_BUILD_DIR}/src/d3d8/libdxvk_d3d8.0.dylib")
  set(DXVK_D3D9_LIB  "${DXVK_BUILD_DIR}/src/d3d9/libdxvk_d3d9.0.dylib")

  # Detect Vulkan SDK location for Meson configuration.
  # VULKAN_SDK must point to the platform subdir (e.g. ~/VulkanSDK/1.4.x/macOS)
  # where lib/libvulkan.dylib and lib/libMoltenVK.dylib live.
  # GeneralsX @build BenderAI 03/03/2026: Normalize env path to macOS platform subdir
  set(VULKAN_SDK_ENV "$ENV{VULKAN_SDK}")

  # If VULKAN_SDK points to the version root (has macOS/ subdir), normalize it
  if(VULKAN_SDK_ENV AND EXISTS "${VULKAN_SDK_ENV}/macOS/lib/libMoltenVK.dylib")
    set(VULKAN_SDK_ENV "${VULKAN_SDK_ENV}/macOS")
    message(STATUS "DXVK macOS build: Normalized VULKAN_SDK to platform subdir: ${VULKAN_SDK_ENV}")
  endif()

  if(NOT VULKAN_SDK_ENV OR NOT EXISTS "${VULKAN_SDK_ENV}/lib/libMoltenVK.dylib")
    # Try home directory: look for ~/VulkanSDK/*/macOS
    file(GLOB VULKAN_HOME_DIRS "$ENV{HOME}/VulkanSDK/*/macOS")
    if(VULKAN_HOME_DIRS)
      list(SORT VULKAN_HOME_DIRS)
      list(REVERSE VULKAN_HOME_DIRS)
      list(GET VULKAN_HOME_DIRS 0 POTENTIAL_SDK)
      if(EXISTS "${POTENTIAL_SDK}/lib/libMoltenVK.dylib")
        set(VULKAN_SDK_ENV "${POTENTIAL_SDK}")
      endif()
    endif()
  endif()

  if(NOT VULKAN_SDK_ENV OR NOT EXISTS "${VULKAN_SDK_ENV}/lib/libMoltenVK.dylib")
    # Try common Homebrew locations
    foreach(BREW_PATH "/usr/local/Caskroom/vulkan-sdk/latest/VulkanSDK/macOS" "/opt/homebrew/Caskroom/vulkan-sdk/latest/VulkanSDK/macOS")
      if(EXISTS "${BREW_PATH}/lib/libMoltenVK.dylib")
        set(VULKAN_SDK_ENV "${BREW_PATH}")
        break()
      endif()
    endforeach()
  endif()

  if(VULKAN_SDK_ENV AND EXISTS "${VULKAN_SDK_ENV}/lib/libMoltenVK.dylib")
    message(STATUS "DXVK macOS build: Using Vulkan SDK at ${VULKAN_SDK_ENV}")
    set(VULKAN_SDK_ENV_VAR "VULKAN_SDK=${VULKAN_SDK_ENV}")
  else()
    message(WARNING "DXVK macOS build: Vulkan SDK / MoltenVK not found; Meson will search system paths")
    if(VULKAN_SDK_ENV)
      message(STATUS "  VULKAN_SDK checked: ${VULKAN_SDK_ENV}")
    endif()
    set(VULKAN_SDK_ENV_VAR "")
  endif()

  ExternalProject_Add(dxvk_macos_build
    GIT_REPOSITORY    https://github.com/doitsujin/dxvk.git
    GIT_TAG           ad253b8a7e20b7cf16fce7d1c505928a434eac29
    SOURCE_DIR        ${DXVK_SOURCE_DIR}
    BINARY_DIR        ${DXVK_BUILD_DIR}
    PATCH_COMMAND
      ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/dxvk-macos-patches.py ${DXVK_SOURCE_DIR}
    CONFIGURE_COMMAND
      ${CMAKE_COMMAND} -E env
        CC=clang CXX=clang++
        "CFLAGS=-arch ${DXVK_HOST_ARCH} -mcpu=apple-m1"
        "CXXFLAGS=-arch ${DXVK_HOST_ARCH} -mcpu=apple-m1"
        "LDFLAGS=-arch ${DXVK_HOST_ARCH}"
        ${VULKAN_SDK_ENV_VAR}
      ${MESON_EXECUTABLE} setup ${DXVK_BUILD_DIR} ${DXVK_SOURCE_DIR}
        --native-file ${CMAKE_SOURCE_DIR}/cmake/meson-arm64-native.ini
        -Ddxvk_native_wsi=sdl3
        --buildtype=release
        --reconfigure
    BUILD_COMMAND
      ${NINJA_EXECUTABLE} -C ${DXVK_BUILD_DIR}
        src/d3d9/libdxvk_d3d9.0.dylib
        src/d3d8/libdxvk_d3d8.0.dylib
    INSTALL_COMMAND   ""
    UPDATE_DISCONNECTED TRUE
  )

  add_custom_command(
    OUTPUT  "${CMAKE_BINARY_DIR}/libdxvk_d3d9.0.dylib"
            "${CMAKE_BINARY_DIR}/libdxvk_d3d8.0.dylib"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${DXVK_D3D9_LIB} "${CMAKE_BINARY_DIR}/libdxvk_d3d9.0.dylib"
    COMMAND ${CMAKE_COMMAND} -E create_symlink
              libdxvk_d3d9.0.dylib "${CMAKE_BINARY_DIR}/libdxvk_d3d9.dylib"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${DXVK_D3D8_LIB} "${CMAKE_BINARY_DIR}/libdxvk_d3d8.0.dylib"
    COMMAND ${CMAKE_COMMAND} -E create_symlink
              libdxvk_d3d8.0.dylib "${CMAKE_BINARY_DIR}/libdxvk_d3d8.dylib"
    DEPENDS dxvk_macos_build
    COMMENT "Installing libdxvk_d3d8 + libdxvk_d3d9 to build directory"
  )
  add_custom_target(dxvk_d3d8_install ALL
    DEPENDS "${CMAKE_BINARY_DIR}/libdxvk_d3d8.0.dylib"
            "${CMAKE_BINARY_DIR}/libdxvk_d3d9.0.dylib"
  )

  set(DXVK_INCLUDE_DIR "${DXVK_SOURCE_DIR}/include/native" CACHE PATH "DXVK native headers")
  # Mirror lowercase dxvk_SOURCE_DIR that FetchContent sets on Linux
  # so CompatLib/CMakeLists.txt check works on macOS as well
  set(dxvk_SOURCE_DIR "${DXVK_SOURCE_DIR}" CACHE PATH "DXVK source directory (macOS)")
  message(STATUS "DXVK source directory: ${DXVK_SOURCE_DIR}")
  message(STATUS "DXVK d3d8 library:     ${DXVK_D3D8_LIB}")

else()
  # ── Linux (linux64-deploy): DXVK pre-built native ────────────────────────────
  # Fetch DXVK native (Steam Runtime) for DirectX→Vulkan translation on Linux.
  # GeneralsX @build BenderAI 28/05/2026 - Restore FetchContent_Declare lost during merge conflict resolution
  FetchContent_Declare(
    dxvk
    URL https://github.com/doitsujin/dxvk/releases/download/${DXVK_VERSION}/dxvk-native-${DXVK_VERSION_BARE}-steamrt-sniper.tar.gz
  )
  FetchContent_MakeAvailable(dxvk)
  message(STATUS "Using DXVK native (Linux DirectX→Vulkan)")
  message(STATUS "  DXVK source directory: ${dxvk_SOURCE_DIR}")
endif()

# ── Vulkan SDK detection for DXVK builds ──────────────────────────────────────
# DXVK requires Vulkan headers + libraries at build time on all modern platforms:
# Windows Modern (win64-modern), macOS (macos-vulkan), Linux (linux64-deploy).
# vc6/win32 presets have SAGE_USE_DX8=ON so they skip this block.
# GeneralsX @build BenderAI 26/02/2026 - Phase 6 Windows Modern + Phase 5 macOS
if(NOT SAGE_USE_DX8)
  # Either Windows Modern (DXVK) or Linux (DXVK native)
  find_package(Vulkan REQUIRED)
  
  if(NOT Vulkan_FOUND)
    message(FATAL_ERROR
      "Vulkan SDK not found. Required for DXVK builds.\n"
      "Install from: https://vulkan.lunarg.com/sdk/home\n"
      "Then ensure VULKAN_SDK environment variable is set.")
  endif()
  
  message(STATUS "Vulkan SDK found: ${Vulkan_INCLUDE_DIR}")
endif()
