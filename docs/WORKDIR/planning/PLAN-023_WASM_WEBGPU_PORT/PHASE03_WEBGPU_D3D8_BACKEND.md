# Phase 03: WebGPU Direct3D 8 Facade & NullAudio Backend

## Objective
Add the `WebGPUDevice` Direct3D 8 translation layer (`WebGPUContext` + `WebGPUD3D8` via `emdawnwebgpu`), the explicit `NullAudioManager` fallback, and hook `DX8Wrapper::Init()` to instantiate `CreateWebGPUDirect3D8()` when `SAGE_USE_WEBGPU` is active.

---

## 1. Copy `WebGPUDevice` and `NullAudioManager` Files

Copy these self-contained files from `/Users/felipebraz/.gemini/antigravity/worktrees/wasm-generals/wasm_port_architecture_analysis` to `/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis`:

```bash
SRC=/Users/felipebraz/.gemini/antigravity/worktrees/wasm-generals/wasm_port_architecture_analysis
DST=/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis

mkdir -p "$DST/Core/GameEngineDevice/Include/WebGPUDevice" \
         "$DST/Core/GameEngineDevice/Source/WebGPUDevice" \
         "$DST/Core/GameEngineDevice/Include/NullAudioDevice"

cp "$SRC/Core/GameEngineDevice/Include/WebGPUDevice/WebGPUContext.h"    "$DST/Core/GameEngineDevice/Include/WebGPUDevice/"
cp "$SRC/Core/GameEngineDevice/Include/WebGPUDevice/WebGPUD3D8.h"      "$DST/Core/GameEngineDevice/Include/WebGPUDevice/"
cp "$SRC/Core/GameEngineDevice/Source/WebGPUDevice/WebGPUContext.cpp"   "$DST/Core/GameEngineDevice/Source/WebGPUDevice/"
cp "$SRC/Core/GameEngineDevice/Source/WebGPUDevice/WebGPUContextProbe.cpp" "$DST/Core/GameEngineDevice/Source/WebGPUDevice/"
cp "$SRC/Core/GameEngineDevice/Source/WebGPUDevice/WebGPUD3D8.cpp"     "$DST/Core/GameEngineDevice/Source/WebGPUDevice/"
cp "$SRC/Core/GameEngineDevice/Include/NullAudioDevice/NullAudioManager.h" "$DST/Core/GameEngineDevice/Include/NullAudioDevice/"
```

> **Note**: Do **NOT** copy `Core/GameEngineDevice/Source/WebGPUDevice/GeneralsXWebShell.html` — it was an obsolete prototype shell superseded by `web/` (see `GeneralsMD/Code/Main/CMakeLists.txt` which compiles with `-sMODULARIZE=1 -sEXPORT_ES6=1` emitting `.js` + `.wasm`).

---

## 2. Wire Targets in `Core/GameEngineDevice/CMakeLists.txt`

Edit `Core/GameEngineDevice/CMakeLists.txt`:

1. After `target_link_libraries(corei_gameenginedevice_public INTERFACE corei_gameengine_public)` (around line 240), add:
   ```cmake
   if(NOT SAGE_USE_OPENAL)
       target_sources(corei_gameenginedevice_private INTERFACE
           Include/NullAudioDevice/NullAudioManager.h
       )
   endif()
   ```
2. At the end of `Core/GameEngineDevice/CMakeLists.txt` (after line 318), add the `SAGE_USE_WEBGPU` library and probe executable:
   ```cmake
   # GeneralsX @feature Codex 04/08/2026 Build reusable WebGPU context and its browser acceptance probe.
   if(SAGE_USE_WEBGPU)
       add_library(webgpu_context STATIC
           Include/WebGPUDevice/WebGPUD3D8.h
           Include/WebGPUDevice/WebGPUContext.h
           Source/WebGPUDevice/WebGPUD3D8.cpp
           Source/WebGPUDevice/WebGPUContext.cpp
       )
       target_include_directories(webgpu_context PUBLIC
           Include
           ${dxvk_SOURCE_DIR}/include/dxvk
           ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWLib
           ${CMAKE_SOURCE_DIR}/Dependencies/Utility
       )
       target_compile_features(webgpu_context PRIVATE cxx_std_20)
       target_link_libraries(webgpu_context PUBLIC webgpu_runtime)

       add_executable(webgpu_context_probe
           Source/WebGPUDevice/WebGPUContextProbe.cpp
       )
       target_compile_features(webgpu_context_probe PRIVATE cxx_std_20)
       target_link_libraries(webgpu_context_probe PRIVATE webgpu_context)
       set_target_properties(webgpu_context_probe PROPERTIES
           OUTPUT_NAME WebGPUContextProbe
           SUFFIX ".html"
       )
   endif()
   ```

---

## 3. Hook `DX8Wrapper::Init()` in `dx8wrapper.cpp`

Edit `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`:

1. Near the `#include "dx8wrapper.h"` header block (around line 53), add:
   ```cpp
   #if defined(SAGE_USE_WEBGPU)
   #include "WebGPUDevice/WebGPUD3D8.h"
   #endif
   ```
2. In `DX8Wrapper::Init(void * hwnd, bool lite)` (around line 661), inside `if (!lite) {`:
   ```cpp
   #if defined(SAGE_USE_WEBGPU)
       D3DInterface = CreateWebGPUDirect3D8();
       if (!D3DInterface) {
           fprintf(stderr, "ERROR: DX8Wrapper::Init() - WebGPU D3D8 initialization failed\n");
           return false;
       }
   #else
       // ... existing DXVK_CONFIG / LoadLibrary(D3D8.DLL / libdxvk_d3d8) / Direct3DCreate8 block ...
   #endif
   ```
   Verify that `IsInitted = true;` still executes immediately after the `#endif`.

---

## Checklist

- [x] Copy `WebGPUDevice` headers and sources into `Core/GameEngineDevice/`.
- [x] Copy `NullAudioManager.h` into `Core/GameEngineDevice/Include/NullAudioDevice/`.
- [x] Add `NullAudioManager.h` and `webgpu_context` CMake targets to `Core/GameEngineDevice/CMakeLists.txt`.
- [x] Hook `CreateWebGPUDirect3D8()` in `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`.
