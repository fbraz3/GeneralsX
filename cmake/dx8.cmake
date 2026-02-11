# DirectX 8 headers and rendering backend selection
# TheSuperHackers @build fighter19 10/02/2026 Bender - Session 18
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
else()
  # Linux: Fetch DXVK native for DirectX→Vulkan translation
  # DXVK includes complete Wine-compatible DirectX headers (no min-dx8-sdk needed)
  FetchContent_Declare(
    dxvk
    URL        https://github.com/doitsujin/dxvk/releases/download/v2.6/dxvk-native-2.6-steamrt-sniper.tar.gz
  )
  FetchContent_MakeAvailable(dxvk)
  message(STATUS "Using DXVK native (Linux DirectX→Vulkan)")
  message(STATUS "DXVK source directory: ${dxvk_SOURCE_DIR}")
endif()
