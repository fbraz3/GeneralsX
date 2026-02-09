# DirectX 8 headers and rendering backend selection
# 
# On Windows: Use min-dx8-sdk (minimal Windows DirectX headers)
# On Linux: Use DXVK (DirectX→Vulkan translation) or min-dx8-sdk
#
# Set SAGE_USE_DX8=ON to use min-dx8-sdk (Windows native DX8)
# Set SAGE_USE_DX8=OFF to use DXVK (Linux D3D8→Vulkan)

if(SAGE_USE_DX8)
  # Windows path: Use minimal DX8 SDK for native DirectX rendering
  FetchContent_Declare(
    dx8
    GIT_REPOSITORY https://github.com/TheSuperHackers/min-dx8-sdk.git
    GIT_TAG        7bddff8c01f5fb931c3cb73d4aa8e66d303d97bc
  )
  FetchContent_MakeAvailable(dx8)
  message(STATUS "Using Windows DX8 SDK (min-dx8-sdk)")
else()
  # Linux path: Use DXVK for DirectX→Vulkan translation
  FetchContent_Declare(
    dxvk
    URL        https://github.com/doitsujin/dxvk/releases/download/v2.6/dxvk-native-2.6-steamrt-sniper.tar.gz
  )
  FetchContent_MakeAvailable(dxvk)
  message(STATUS "Using Linux DXVK (DirectX→Vulkan)")
  message(STATUS "DXVK source directory: ${dxvk_SOURCE_DIR}")
endif()
