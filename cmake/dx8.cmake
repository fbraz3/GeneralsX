# DirectX 8 headers and rendering backend selection
# GeneralsX @build BenderAI 10/02/2026 - Session 18
# Fighter19's approach: Fetch ONE OR THE OTHER, never both
# 
# On Windows: Use min-dx8-sdk (minimal Windows DirectX headers + libs)
# On Linux: Use DXVK native (Wine-compatible DirectX headers + Vulkan libs)
#
# CRITICAL: Mixing headers causes conflicts - dx8-src has incomplete types,
# DXVK has full DirectX8+Wine headers. Compiler picks first path = wrong headers.

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
  # macOS: Fetch DXVK source code and compile with MoltenVK (Vulkan→Metal bridge)
  # GeneralsX @build BenderAI 24/02/2026 - Phase 5 macOS port
  # MoltenVK provides Vulkan API on macOS via Metal translation layer
  # DXVK-native supports SDL3 WSI (PR #4404) and builds on macOS with Meson
  message(STATUS "Configuring DXVK (v2.6) from source for macOS with MoltenVK...")
  
  FetchContent_Declare(
    dxvk
    GIT_REPOSITORY https://github.com/doitsujin/dxvk.git
    GIT_TAG        v2.6
  )
  FetchContent_MakeAvailable(dxvk)
  message(STATUS "DXVK source directory: ${dxvk_SOURCE_DIR}")
  message(STATUS "Note: DXVK compilation requires Meson installed: brew install meson")
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
