# DirectX 8 headers and rendering backend selection
# GeneralsX @build BenderAI 10/02/2026 - Session 18 (updated 26/02/2026 for Phase 6)
# Fighter19's approach: Fetch ONE OR THE OTHER, never both
# 
# On Windows (vc6/win32): Use min-dx8-sdk (native DirectX 8)
# On Windows (win64-modern): Use DXVK headers (DirectX→Vulkan, runtime d3d8.dll)
# On Linux: Use DXVK native (Wine-compatible DirectX headers + Vulkan libs)
#
# CRITICAL: Mixing headers causes conflicts - min-dx8-sdk has incomplete types vs full DXVK.
# Compiler picks first path = wrong headers. Use platform guards to ensure single path.

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
  # Compile against DXVK d3d8.h headers (same headers as Linux DXVK).
  # At runtime, DXVK's d3d8.dll (deployed alongside EXE) intercepts D3D8 calls
  # and translates them to Vulkan. This is identical to how Proton handles D3D8→Vulkan.
  # We do NOT link against DXVK .lib files — the DLL is loaded dynamically.
  # Note: Phase 6 places DXVK Windows DLLs alongside the EXE at runtime.
  
  FetchContent_Declare(
    dxvk_headers
    GIT_REPOSITORY https://github.com/doitsujin/dxvk.git
    GIT_TAG        v2.6
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

  # Extract paths for use in target_include_directories and deployment
  # GeneralsX @build BenderAI 28/02/2026 Strategy shift:
  # Instead of using DXVK headers (which have MinGW types like __MSABI_LONG incompatible with MSVC x64),
  # use jmarshall's battle-tested DX90SDK headers which are proven to compile on Windows x64.
  # DXVK is ONLY for Vulkan (not DirectX headers).
  # D3D8 headers come from Code/Libraries/DX90SDK (copied from jmarshall-win64-modern).
  
  # Check if DX90SDK exists (was copied from jmarshall reference)
  # Try shared location first (Core/Libraries), then game-specific locations
  if(EXISTS "${CMAKE_SOURCE_DIR}/Core/Libraries/DX90SDK/Include")
    set(DXVK_D3D8_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Core/Libraries/DX90SDK/Include" CACHE PATH "D3D8/D3D9 headers (jmarshall DX90SDK)" FORCE)
    message(STATUS "✅ Using D3D8 headers from jmarshall DX90SDK (Core/Libraries): ${DXVK_D3D8_INCLUDE_DIR}")
  elseif(EXISTS "${CMAKE_SOURCE_DIR}/GeneralsMD/Code/Libraries/DX90SDK/Include")
    set(DXVK_D3D8_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/GeneralsMD/Code/Libraries/DX90SDK/Include" CACHE PATH "D3D8/D3D9 headers (jmarshall DX90SDK)" FORCE)
    message(STATUS "✅ Using D3D8 headers from jmarshall DX90SDK (GeneralsMD): ${DXVK_D3D8_INCLUDE_DIR}")
  elseif(EXISTS "${CMAKE_SOURCE_DIR}/Generals/Code/Libraries/DX90SDK/Include")
    set(DXVK_D3D8_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Generals/Code/Libraries/DX90SDK/Include" CACHE PATH "D3D8/D3D9 headers (jmarshall DX90SDK)" FORCE)
    message(STATUS "✅ Using D3D8 headers from jmarshall DX90SDK (Generals): ${DXVK_D3D8_INCLUDE_DIR}")
  else()
    # Fallback: use DXVK native headers (NOT RECOMMENDED - has MinGW type conflicts on MSVC x64)
    set(DXVK_D3D8_INCLUDE_DIR "${dxvk_headers_SOURCE_DIR}/include/native/directx" CACHE PATH "D3D8 headers (DXVK fallback, may have x64 issues on Windows)" FORCE)
    message(STATUS "⚠️  WARNING: DX90SDK not found. Falling back to DXVK native headers (may cause x64 compilation issues on Windows/MSVC).")
  endif()
  
  # DXVK include path is FOR VULKAN ONLY, not DirectX
  set(DXVK_INCLUDE_DIR "${dxvk_headers_SOURCE_DIR}/include" CACHE PATH "DXVK Vulkan headers" FORCE)
  set(DXVK_WIN_DLL_DIR "${dxvk_win_release_SOURCE_DIR}/x64" CACHE PATH "DXVK Windows DLLs (x64)")
  
  message(STATUS "Using DXVK headers (Windows Modern, D3D8→Vulkan at runtime)")
  message(STATUS "  D3D8 headers: ${DXVK_D3D8_INCLUDE_DIR}")
  message(STATUS "  DXVK DLL dir: ${DXVK_WIN_DLL_DIR}")
  
  message(STATUS "Using DXVK headers (Windows Modern, D3D8→Vulkan at runtime)")
  message(STATUS "  DXVK headers: ${DXVK_INCLUDE_DIR}")
  message(STATUS "  DXVK DLL dir: ${DXVK_WIN_DLL_DIR}")

else()
  # ── Linux: DXVK native (linux64-deploy) ─────────────────────────────────────
  # Fetch DXVK native (Steam Runtime) for DirectX→Vulkan translation on Linux.
  # DXVK includes complete Wine-compatible DirectX headers.
  FetchContent_Declare(
    dxvk
    URL https://github.com/doitsujin/dxvk/releases/download/v2.6/dxvk-native-2.6-steamrt-sniper.tar.gz
  )
  FetchContent_MakeAvailable(dxvk)
  message(STATUS "Using DXVK native (Linux DirectX→Vulkan)")
  message(STATUS "  DXVK source directory: ${dxvk_SOURCE_DIR}")
endif()

# ── Vulkan SDK detection for DXVK builds ──────────────────────────────────────
# DXVK requires Vulkan headers + libraries at build time (for Windows Modern and Linux).
# GeneralsX @build BenderAI 26/02/2026 - Step 10 Phase 6
if(NOT SAGE_USE_DX8 OR WIN32)
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
