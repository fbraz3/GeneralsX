# Phase 02: Platform Compatibility, 32-bit Types & Font Resolution

## Objective
Adapt `WWLib`, `Dependencies/Utility`, `CompatLib`, and `WW3D2` for 32-bit `wasm32-emscripten` compilation and ensure Zero Hour text rendering uses FreeType directly with the repository's bundled `assets/fonts/` without requiring Fontconfig.

---

## 1. Fixed-Width Types & Compatibility Headers

1. **`Core/Libraries/Source/WWVegas/WWLib/bittype.h`** (line 71):
   - Note: `GeneralsX` HEAD already unified `uint32`/`sint32` to `uint32_t`/`int32_t` globally (line 50). Only the `DWORD`/`ULONG` block needs Emscripten added:
   - Change line 71 from:
     ```cpp
     #if defined(__linux__) || defined(__APPLE__)
     ```
     to:
     ```cpp
     #if defined(__linux__) || defined(__APPLE__) || defined(__EMSCRIPTEN__)
     ```
     This keeps `DWORD` and `ULONG` as `uint32_t` to match DXVK D3D8 headers (`d3d8types.h`).

2. **`Core/Libraries/Source/WWVegas/WWLib/osdep.h`** (line 30):
   - Emscripten libc already provides `strupr` or defines incompatibilities with static `strupr(char*)`.
   - Change line 30 from:
     ```cpp
     #if !defined(strupr)
     ```
     to:
     ```cpp
     #if !defined(strupr) && !defined(__EMSCRIPTEN__)
     ```

3. **`Dependencies/Utility/Utility/endian_compat.h`** (lines 27 and 118):
   - Emscripten provides `<endian.h>` and is little-endian `wasm32`.
   - On both line 27 and line 118, change:
     ```cpp
     #if defined(__linux__) || defined(__CYGWIN__)
     ```
     to:
     ```cpp
     #if defined(__linux__) || defined(__CYGWIN__) || defined(__EMSCRIPTEN__)
     ```

4. **`Generals/Code/CompatLib/Include/memory_compat.h`** and **`GeneralsMD/Code/CompatLib/Include/memory_compat.h`** (lines 6 and 26):
   - Emscripten provides `<malloc.h>` and `malloc_usable_size()`.
   - Change `#ifdef __linux__` on lines 6 and 26 to:
     ```cpp
     #if defined(__linux__) || defined(__EMSCRIPTEN__)
     ```

5. **`GeneralsMD/Code/CompatLib/Source/d3dx8_compat.cpp`**:
   - Lines 8 and 109: Change `#ifndef __APPLE__` to `#if !defined(__APPLE__) && !defined(__EMSCRIPTEN__)` so Emscripten uses the deterministic manual box filter instead of GLI.
   - Inside `D3DXFilterTexture()` (compare with `wasm-generals/GeneralsMD/Code/CompatLib/Source/d3dx8_compat.cpp` around lines 429–438): fix the surface reference leak / error check where `topsurf->Release()` is called before assigning `topsurf = mipsurf`.

---

## 2. WW3D2 Build, Font Resolution & Audio Stubs

1. **`Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt`** (lines 250–283):
   - Update compile definitions:
     ```cmake
     target_compile_definitions(corei_ww3d2 INTERFACE
         $<$<PLATFORM_ID:Linux,Darwin,Emscripten>:SAGE_USE_FREETYPE>
         $<$<PLATFORM_ID:Linux,Darwin>:SAGE_USE_FONTCONFIG>
     )
     ```
   - When `SAGE_USE_WEBGPU` is ON, expose `Core/GameEngineDevice/Include` so `dx8wrapper.cpp` can include `WebGPUDevice/WebGPUD3D8.h`:
     ```cmake
     if(SAGE_USE_WEBGPU)
         target_include_directories(corei_ww3d2 INTERFACE
             "${CMAKE_SOURCE_DIR}/Core/GameEngineDevice/Include"
         )
     endif()
     ```
   - Guard `Fontconfig` and `Iconv` discovery:
     ```cmake
     if(UNIX)
         find_package(Freetype REQUIRED)
     endif()
     if(UNIX AND NOT EMSCRIPTEN)
         find_package(Fontconfig REQUIRED)
     endif()
     if(APPLE)
         find_package(Iconv REQUIRED)
     endif()
     ```
   - Update `target_link_libraries(corei_ww3d2 INTERFACE ...)`:
     ```cmake
     $<$<PLATFORM_ID:Linux,Darwin,Emscripten>:Freetype::Freetype>
     $<$<PLATFORM_ID:Linux>:Fontconfig::Fontconfig>
     $<$<PLATFORM_ID:Darwin>:Fontconfig::Fontconfig>
     $<$<PLATFORM_ID:Darwin>:Iconv::Iconv>
     ```

2. **`Core/Libraries/Source/WWVegas/WW3D2/render2dsentence.h`** (lines 54, 149) and **`render2dsentence.cpp`** (line 2084):
   - `GeneralsX` HEAD already includes multi-tiered font resolution (`Platform::FindLocalFontFile` in `Core/Libraries/Source/Platform/PlatformPaths.h`) which probes `"fonts"`, `"assets/fonts"`, and `$CNC_GENERALS_ZH_PATH/fonts` **before** calling Fontconfig (`Tier 2`), plus bundled fallback to `arial.ttf` / `arialbold.ttf` / `fa-brands-400.ttf` (`Tier 3`).
   - Therefore, unlike `wasm-generals` (which hardcoded `/fonts/GeneralsX.ttf`), `GeneralsX` only needs to guard the `#include <fontconfig/fontconfig.h>` and the `Tier 2` `FcConfig` block with `!defined(__EMSCRIPTEN__)`!
   - In `render2dsentence.h` line 54:
     ```cpp
     #if !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) && !defined(__EMSCRIPTEN__)
         #include <fontconfig/fontconfig.h>
     #endif
     ```
   - In `render2dsentence.cpp` line 2084:
     ```cpp
     #if !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) && !defined(__EMSCRIPTEN__)
     ```
     (around the `FcConfig *config = FcInitLoadConfigAndFonts();` block through `FcConfigDestroy( config );`).
   - Note: Phase 04 preloads `${CMAKE_SOURCE_DIR}/assets/fonts@/fonts` into Emscripten MEMFS so `arial.ttf`, `arialbold.ttf`, `couriernew.ttf`, `timesnewroman.ttf`, and `fa-brands-400.ttf` resolve automatically via `Platform::FindLocalFontFile`.

3. **`Core/Libraries/Source/WWVegas/WW3D2/animatedsoundmgr.cpp`** (lines 48–49) and **`soundrobj.h`** (line 40):
   - In `animatedsoundmgr.cpp`, wrap `#include "WWAudio/WWAudio.h"` and `#include "WWAudio/AudibleSound.h"` in `#if !defined(SAGE_USE_WEBGPU) ... #endif`.
   - In `soundrobj.h`, set `#define noWWAUDIO 0` when `#if defined(SAGE_USE_WEBGPU)` and `#define noWWAUDIO 1` otherwise.

---

## Checklist

- [x] Update `bittype.h` (`DWORD`/`ULONG` for `__EMSCRIPTEN__`).
- [x] Update `osdep.h` (`strupr` guard for `__EMSCRIPTEN__`).
- [x] Update `Dependencies/Utility/Utility/endian_compat.h` (`__EMSCRIPTEN__` support).
- [x] Update `Generals` and `GeneralsMD` `memory_compat.h` (`__EMSCRIPTEN__` support).
- [x] Update `GeneralsMD/Code/CompatLib/Source/d3dx8_compat.cpp` (deterministic box filter for `__EMSCRIPTEN__`).
- [x] Update `Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt` (FreeType for Emscripten, exclude Fontconfig).
- [x] Guard Fontconfig in `render2dsentence.h` and `render2dsentence.cpp` for `__EMSCRIPTEN__`.
- [x] Guard `WWAudio` in `animatedsoundmgr.cpp` and `soundrobj.h` for `SAGE_USE_WEBGPU`.
