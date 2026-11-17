# Phase 01: Build System, Dependencies & NGMP CMake Alignment

## Objective
Configure `vcpkg.json`, CMake modules (`cmake/`), and `CMakePresets.json` so `GeneralsX` can target `wasm32-emscripten` with WebGPU (`SAGE_USE_WEBGPU=ON`), disable legacy GameSpy (`SAGE_USE_GAMESPY=OFF`), and keep `cmake/ngmp.cmake` cleanly structured for future Emscripten NGMP networking (`SAGE_USE_NGMP`).

---

## 1. Update `vcpkg.json`

Edit `/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis/vcpkg.json` so `wasm32-emscripten` does not try to build native-only packages (`fontconfig`, `curl`, `gamenetworkingsockets`, `ffmpeg`). Keep `"features": ["ssl", "websockets"]` on `curl` for desktop builds!

- `fontconfig`: change `"platform": "!windows"` to `"platform": "!(windows | emscripten)"`
- `curl`: add `"platform": "!emscripten"`
  ```json
  {
    "name": "curl",
    "platform": "!emscripten",
    "features": ["ssl", "websockets"]
  }
  ```
- `gamenetworkingsockets`: change `"platform": "!osx"` to `"platform": "!(osx | emscripten)"`

---

## 2. Add `cmake/webgpu.cmake`

Copy `cmake/webgpu.cmake` directly from `wasm-generals` (`Core` contract for Emscripten + `emdawnwebgpu`):

```bash
cp /Users/felipebraz/.gemini/antigravity/worktrees/wasm-generals/wasm_port_architecture_analysis/cmake/webgpu.cmake \
   /Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis/cmake/webgpu.cmake
```

Verify its contents match:
- Interface target `webgpu_runtime` with `SAGE_USE_WEBGPU`
- `--use-port=emdawnwebgpu` and `-fexceptions` on compile & link
- Link options: `-sASYNCIFY=1`, `-sSTACK_SIZE=1048576`, `-sINITIAL_MEMORY=2147483648`, `-lidbfs.js`

> **Why fixed 2 GiB heap (`-sINITIAL_MEMORY=2147483648`)?**  
> Memory growth detaches `emdawnwebgpu` JS heap views (causing black canvas), resizable ArrayBuffers break `TextDecoder`, and >2 GiB hits signed pointer issues in legacy 2003 code.

---

## 3. Update Root `CMakeLists.txt`

In `CMakeLists.txt`, immediately after `include(FetchContent)` (around line 57), add:

```cmake
# GeneralsX @feature Configure WebAssembly and WebGPU before backend selection.
if(SAGE_USE_WEBGPU)
    include(cmake/webgpu.cmake)
endif()
```

Keep the existing `generalsx_big` custom target (lines 158–188) active; `Python3` builds `generalsx.big` in `${CMAKE_BINARY_DIR}` for all targets, including WASM.

---

## 4. Update `cmake/config-build.cmake`

1. Near lines 15–18 (next to `SAGE_USE_SDL3`, `SAGE_USE_OPENAL`, `SAGE_USE_MINIAUDIO`), add:
   ```cmake
   option(SAGE_USE_GAMESPY "Enable legacy GameSpy networking" ON)
   option(SAGE_USE_WEBGPU "Use WebGPU through Emscripten" OFF)

   if(SAGE_USE_WEBGPU AND NOT EMSCRIPTEN)
       message(FATAL_ERROR "SAGE_USE_WEBGPU requires the Emscripten toolchain")
   endif()
   ```
2. Add `add_feature_info(...)` entries for `GameSpySupport` (`SAGE_USE_GAMESPY`) and `WebGPURenderer` (`SAGE_USE_WEBGPU`).
3. After the `if(SAGE_USE_NGMP)` block (around line 159), propagate `SAGE_USE_GAMESPY` and `SAGE_USE_WEBGPU` to `core_config`:
   ```cmake
   if(SAGE_USE_GAMESPY)
       target_compile_definitions(core_config INTERFACE SAGE_USE_GAMESPY)
   endif()

   if(SAGE_USE_WEBGPU)
       target_compile_definitions(core_config INTERFACE SAGE_USE_WEBGPU)
       target_link_libraries(core_config INTERFACE webgpu_runtime)
   endif()
   ```

---

## 5. Align `cmake/ngmp.cmake` for Future Browser NGMP

In `cmake/ngmp.cmake`, prepare the build module so that when `SAGE_USE_NGMP=ON` is eventually enabled for WebAssembly, CMake uses browser-native HTTP/WebSocket/WebRTC instead of desktop `CURL` / `GameNetworkingSockets`:

1. Keep `nlohmann_json` (`lines 5-13`, `lines 150-154`) enabled for all platforms, including `EMSCRIPTEN`.
2. Keep the NGMP endpoint configuration block (`NGMP_SERVER_HOST`, `NGMP_SERVER_PORT`, `NGMP_USE_SSL`, `NGMP_WEB_PORTAL_URL`, `NGMP_MOTD_URL`, `lines 216-288`) active for all platforms, including `EMSCRIPTEN`.
3. Wrap the native dependency discovery and linking (`find_package(CURL REQUIRED)`, `Protobuf`, `Abseil`, `OpenSSL`, `GameNetworkingSockets`, `CURL_LIBRARIES` / `CURL_INCLUDE_DIRS` — `lines 15-148` and `156-214`) inside:
   ```cmake
   if(NOT EMSCRIPTEN)
       # ... existing native CURL, Protobuf, OpenSSL, and GameNetworkingSockets setup ...
   endif()
   ```
   Wait — make sure `nlohmann_json` target linking (`lines 150-154`) stays **outside** `if(NOT EMSCRIPTEN)` so `nlohmann_json` still links under Emscripten.

---

## 6. Update Supporting `cmake/*.cmake` Modules

1. **`cmake/compilers.cmake`** (line 35):
   - Change `else()` to `elseif(NOT EMSCRIPTEN)` in the block that adds `-g` debug info to Release builds (prevents incompatible DWARF post-processing in Emscripten Release builds).
2. **`cmake/dx8.cmake`**:
   - Add an `elseif(SAGE_USE_WEBGPU)` branch before `elseif(APPLE AND SAGE_USE_MOLTENVK)` (matching `wasm-generals/cmake/dx8.cmake` lines 31–38):
     ```cmake
     elseif(SAGE_USE_WEBGPU)
       FetchContent_Declare(
         dxvk
         URL https://github.com/doitsujin/dxvk/releases/download/v2.6/dxvk-native-2.6-steamrt-sniper.tar.gz
       )
       FetchContent_MakeAvailable(dxvk)
       message(STATUS "Using DXVK Direct3D 8 headers with the WebGPU backend")
     ```
     This exposes only the D3D8 API headers (`${dxvk_SOURCE_DIR}/include/dxvk`) needed by `WW3D2` and `WebGPUD3D8.cpp` without building or loading DXVK `.so`/`.dylib`.
3. **`cmake/gamemath.cmake`**:
   - After `FetchContent_MakeAvailable(gamemath)`, add:
     ```cmake
     if(EMSCRIPTEN)
         target_compile_definitions(gamemath PRIVATE FE_INVALID=0 FE_INEXACT=0 FE_UNDERFLOW=0)
     endif()
     ```
4. **`cmake/gamespy.cmake`**:
   - Replace with the conditional version from `wasm-generals/cmake/gamespy.cmake`:
     ```cmake
     if(SAGE_USE_GAMESPY)
         FetchContent_MakeAvailable(gamespy)
     else()
         FetchContent_GetProperties(gamespy)
         if(NOT gamespy_POPULATED)
             FetchContent_Populate(gamespy)
         endif()
         add_library(gamespy_disabled INTERFACE)
         target_include_directories(gamespy_disabled INTERFACE
             ${gamespy_SOURCE_DIR}/include
             ${gamespy_SOURCE_DIR}/include/gamespy
         )
         add_library(gamespy::gamespy ALIAS gamespy_disabled)
     endif()
     ```
5. **`cmake/miniaudio.cmake`**:
   - Note that `GeneralsX` HEAD links `Threads::Threads` and `${CMAKE_DL_LIBS} m` inside both `elseif(UNIX)` blocks (lines 30–31 and 61–62). Guard those thread/dl links with `if(NOT EMSCRIPTEN)`:
     ```cmake
     elseif(UNIX)
         target_compile_definitions(miniaudio_lib INTERFACE
             MA_NO_WASAPI MA_NO_DSOUND MA_NO_WINMM MA_NO_COREAUDIO)
         if(NOT EMSCRIPTEN)
             find_package(Threads REQUIRED)
             target_link_libraries(miniaudio_lib INTERFACE Threads::Threads ${CMAKE_DL_LIBS} m)
         endif()
     ```
6. **`cmake/sdl3.cmake`**:
   - Apply all `if(EMSCRIPTEN)` guards from `wasm-generals/cmake/sdl3.cmake`:
     - Skip system `find_package(SDL3)` when `EMSCRIPTEN` (`if(NOT EMSCRIPTEN AND CMAKE_SYSTEM_NAME STREQUAL "Linux")`).
     - Set `SDL_SHARED OFF` and `SDL_STATIC ON` when `EMSCRIPTEN`.
     - Set `SDL_WAYLAND OFF` and `SDL_X11 OFF` when `EMSCRIPTEN`.
     - Skip system `PNG` search when `if(EMSCRIPTEN)`.
     - Set `SDL3IMAGE_DEPS_SHARED OFF`, `SDL3IMAGE_JPG OFF`, `SDL3IMAGE_PNG OFF`, `SDL3IMAGE_TIF OFF`, `SDL3IMAGE_WEBP OFF`, `SDL3IMAGE_AVIF OFF`, `SDL3IMAGE_XCUR OFF` when `EMSCRIPTEN` (browser ANI/ICO cursor loading uses SDL_image's built-in BMP/CUR/ANI decoders).

---

## 7. Add `webgpu` Preset to `CMakePresets.json`

1. Under `configurePresets`, add:
   ```json
   {
       "name": "webgpu",
       "displayName": "Chrome WebAssembly + WebGPU",
       "inherits": "default-vcpkg",
       "generator": "Ninja",
       "hidden": false,
       "binaryDir": "${sourceDir}/build/${presetName}",
       "cacheVariables": {
           "CMAKE_BUILD_TYPE": "Release",
           "CMAKE_EXECUTABLE_SUFFIX": ".html",
           "VCPKG_TARGET_TRIPLET": "wasm32-emscripten",
           "VCPKG_CHAINLOAD_TOOLCHAIN_FILE": "$env{EMSCRIPTEN_ROOT}/cmake/Modules/Platform/Emscripten.cmake",
           "RTS_BUILD_CORE_TOOLS": "OFF",
           "RTS_BUILD_GENERALS": "OFF",
           "RTS_BUILD_ZEROHOUR": "ON",
           "RTS_BUILD_ZEROHOUR_TOOLS": "OFF",
           "RTS_BUILD_OPTION_FFMPEG": "OFF",
           "RTS_CRASHDUMP_ENABLE": "OFF",
           "SAGE_USE_DX8": "OFF",
           "SAGE_USE_GLM": "ON",
           "SAGE_USE_GAMESPY": "OFF",
           "SAGE_USE_NGMP": "OFF",
           "SAGE_USE_MINIAUDIO": "ON",
           "SAGE_USE_OPENAL": "OFF",
           "SAGE_USE_SDL3": "ON",
           "SAGE_USE_WEBGPU": "ON",
           "SAGE_UPDATE_CHECK": "OFF",
           "RTS_BUILD_OPTION_SAGE_PATCH": "OFF"
       },
       "description": "Build Zero Hour for Chrome with WebAssembly and WebGPU."
   }
   ```
2. Under `buildPresets`, add:
   ```json
   {
       "name": "webgpu",
       "configurePreset": "webgpu",
       "displayName": "Build Chrome WebAssembly + WebGPU",
       "description": "Build GeneralsXZH for Chrome with WebAssembly and WebGPU"
   }
   ```

---

## Checklist

- [x] Update `vcpkg.json` platform constraints for `emscripten`.
- [x] Copy `cmake/webgpu.cmake` from `wasm-generals`.
- [x] Include `cmake/webgpu.cmake` in root `CMakeLists.txt` when `SAGE_USE_WEBGPU` is enabled.
- [x] Add `SAGE_USE_GAMESPY` and `SAGE_USE_WEBGPU` options and `core_config` propagation in `cmake/config-build.cmake`.
- [x] Guard native CURL/Protobuf/OpenSSL/GNS in `cmake/ngmp.cmake` with `if(NOT EMSCRIPTEN)` while keeping `nlohmann_json` and NGMP definitions shared.
- [x] Update `cmake/compilers.cmake`, `cmake/dx8.cmake`, `cmake/gamemath.cmake`, `cmake/gamespy.cmake`, `cmake/miniaudio.cmake`, and `cmake/sdl3.cmake`.
- [x] Add `webgpu` configurePreset and buildPreset to `CMakePresets.json`.
