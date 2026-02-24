# DirectX 8 headers and rendering backend selection
# GeneralsX @build BenderAI 10/02/2026 - Session 18
# Fighter19's approach: Fetch ONE OR THE OTHER, never both
#
# On Windows: Use min-dx8-sdk (minimal Windows DirectX headers + libs)
# On Linux:   Use DXVK native pre-built tarball (DirectX→Vulkan translation)
# On macOS:   Build DXVK from source using Meson + MoltenVK (DirectX→Metal bridge)
#
# CRITICAL: Mixing headers causes conflicts - dx8-src has incomplete types,
# DXVK has full DirectX8+Wine headers. Compiler picks first path = wrong headers.
#
# macOS DXVK build (Session 61, 24/02/2026):
#   DXVK 2.6 builds natively on macOS arm64 via its "native" build mode.
#   Five source-level patches are required; applied automatically by
#   cmake/dxvk-macos-patches.py.
#
# Patches applied to DXVK source for macOS:
#   1. include/native/windows/windows_base.h and src/util/util_win32_compat.h:
#      __unix__ is NOT defined on macOS; add __APPLE__ to the #if guard so
#      LoadLibraryA/GetProcAddress/FreeLibrary shims are compiled.
#   2. src/util/util_env.cpp:
#      pthread_setname_np on macOS takes only 1 arg (name), not 2 (thread, name).
#   3. src/util/util_small_vector.h:
#      lzcnt(n-1) where n is size_t (unsigned long on macOS arm64) is ambiguous
#      between uint32_t and uint64_t overloads; cast to uint64_t explicitly.
#   4. src/util/util_bit.h:
#      Add uintptr_t overloads for tzcnt/lzcnt (macOS arm64: uintptr_t =
#      unsigned long, distinct from uint32_t and uint64_t, causing ambiguity).
#   5. src/{dxgi,d3d8,d3d9,d3d10,d3d11}/meson.build:
#      -Wl,--version-script is GNU ld only. macOS ld rejects it; guard with
#      platform != 'darwin'.
#
# Reference: docs/WORKDIR/lessons/2026-02-LESSONS.md (Session 61)

set(DXVK_VERSION "v2.6")

if(SAGE_USE_DX8)
  # Windows: Fetch min-dx8-sdk for native DirectX 8
  FetchContent_Declare(
    dx8
    GIT_REPOSITORY https://github.com/TheSuperHackers/min-dx8-sdk.git
    GIT_TAG        7bddff8c01f5fb931c3cb73d4aa8e66d303d97bc
  )
  FetchContent_MakeAvailable(dx8)
  message(STATUS "Using DirectX 8 SDK (Windows native)")

elseif(APPLE AND SAGE_USE_MOLTENVK)
  # macOS: Build DXVK 2.6 from source using Meson + MoltenVK
  # GeneralsX @build BenderAI 24/02/2026 - Phase 5 macOS port (Session 61)
  find_program(MESON_EXECUTABLE meson HINTS /usr/local/bin /opt/homebrew/bin)
  find_program(NINJA_EXECUTABLE ninja HINTS /usr/local/bin /opt/homebrew/bin)

  if(NOT MESON_EXECUTABLE)
    message(FATAL_ERROR "DXVK macOS build requires meson: brew install meson")
  endif()
  if(NOT NINJA_EXECUTABLE)
    message(FATAL_ERROR "DXVK macOS build requires ninja: brew install ninja")
  endif()

  # Detect host architecture so Clang targets the correct slice
  # (critical on Apple Silicon: /usr/local/bin/meson is x86_64 via Rosetta,
  #  but we must produce arm64 dylibs for the native game process)
  execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE DXVK_HOST_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  message(STATUS "Building DXVK ${DXVK_VERSION} for macOS/${DXVK_HOST_ARCH} with Meson (${MESON_EXECUTABLE})")

  include(ExternalProject)
  set(DXVK_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/dxvk-src")
  set(DXVK_BUILD_DIR  "${CMAKE_BINARY_DIR}/_deps/dxvk-build-macos")
  set(DXVK_D3D8_LIB  "${DXVK_BUILD_DIR}/src/d3d8/libdxvk_d3d8.0.dylib")
  set(DXVK_D3D9_LIB  "${DXVK_BUILD_DIR}/src/d3d9/libdxvk_d3d9.0.dylib")

  ExternalProject_Add(dxvk_macos_build
    GIT_REPOSITORY    https://github.com/doitsujin/dxvk.git
    GIT_TAG           ${DXVK_VERSION}
    SOURCE_DIR        ${DXVK_SOURCE_DIR}
    BINARY_DIR        ${DXVK_BUILD_DIR}
    # Apply macOS patches before configuring
    PATCH_COMMAND
      ${CMAKE_COMMAND} -E echo "Applying macOS patches to DXVK..." &&
      ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/dxvk-macos-patches.py ${DXVK_SOURCE_DIR}
    # Configure with Meson using SDL3 windowing system.
    # Force -arch flag so that even an x86_64 Meson (Rosetta) produces the
    # correct slice for the host CPU (arm64 on Apple Silicon).
    CONFIGURE_COMMAND
      ${CMAKE_COMMAND} -E env
        CC=clang CXX=clang++
        "CFLAGS=-arch ${DXVK_HOST_ARCH}"
        "CXXFLAGS=-arch ${DXVK_HOST_ARCH}"
        "LDFLAGS=-arch ${DXVK_HOST_ARCH}"
      ${MESON_EXECUTABLE} setup ${DXVK_BUILD_DIR} ${DXVK_SOURCE_DIR}
        -Ddxvk_native_wsi=sdl3
        --buildtype=release
        --reconfigure
    # Build d3d9 first (d3d8 links against it at runtime via @rpath),
    # then d3d8. Both dylibs must be present in the runtime directory.
    BUILD_COMMAND
      ${NINJA_EXECUTABLE} -C ${DXVK_BUILD_DIR}
        src/d3d9/libdxvk_d3d9.0.dylib
        src/d3d8/libdxvk_d3d8.0.dylib
    # DXVK install is not needed; we deploy the dylibs manually
    INSTALL_COMMAND   ""
    # Only re-patch/reconfigure when the git tag changes
    UPDATE_DISCONNECTED TRUE
  )

  # Copy libdxvk_d3d9 + libdxvk_d3d8 to build dir and create unversioned symlinks.
  # d3d8 links against d3d9 via @rpath, so both must be present at runtime.
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

  # Export path so other cmake files know where the headers are
  set(DXVK_INCLUDE_DIR "${DXVK_SOURCE_DIR}/include/native" CACHE PATH "DXVK native headers")
  message(STATUS "DXVK source directory: ${DXVK_SOURCE_DIR}")
  message(STATUS "DXVK d3d8 library:     ${DXVK_D3D8_LIB}")

else()
  # Linux: Fetch pre-built DXVK native binary for DirectX→Vulkan translation
  # Native 32-bit and 64-bit Linux binaries (.so)
  FetchContent_Declare(
    dxvk
    URL        https://github.com/doitsujin/dxvk/releases/download/v2.6/dxvk-native-2.6-steamrt-sniper.tar.gz
  )
  FetchContent_MakeAvailable(dxvk)
  message(STATUS "Using DXVK native (Linux DirectX→Vulkan)")
  message(STATUS "DXVK source directory: ${dxvk_SOURCE_DIR}")
endif()
