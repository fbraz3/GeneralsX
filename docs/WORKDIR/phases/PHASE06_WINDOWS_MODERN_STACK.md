# PHASE06: Windows Modern Stack (DXVK + SDL3 + OpenAL)

**Status**: 📋 Planned
**Type**: Platform preset — Windows native build with cross-platform backends
**Created**: 2026-02-26
**Prerequisites**: Phase 1 Linux build stable and functional

---

## Goal

Create a new CMake preset **`win64-modern`** that builds the game on Windows using the same
backend stack as the Linux build:

| Subsystem  | Legacy (vc6 / win32)       | win64-modern                         |
|------------|----------------------------|--------------------------------------|
| Graphics   | DirectX 8 native           | DXVK → Vulkan (d3d8.dll at runtime)  |
| Window     | Win32 native (WM_* msgs)   | SDL3                                 |
| Input      | Win32 native (RAWINPUT)    | SDL3                                 |
| Audio      | Miles Sound System         | OpenAL Soft (WASAPI backend)         |
| Video      | Bink                       | FFmpeg                               |
| Arch       | 32-bit (x86)               | 64-bit (x86_64)                      |
| Compiler   | VC6 / MSVC 2022 x86        | MSVC 2022 x64                        |

**Why this matters:**
- Testing and debugging Windows-direct: same backend divergences caught before Linux runs
- Enables the SDL3+OpenAL code paths to be exercised without a VM or Docker
- Bridges Windows and Linux: a single codebase serving both via the same feature flags
- Simplifies future Phase 4 hardening (one backend, two platforms)

---

## Scope

### In Scope ✅
- New CMake preset `win64-modern` (configure + build entries)
- DXVK Windows headers integration (compile-time only; runtime via deployed `d3d8.dll`)
- SDL3 windowing on Windows (FetchContent, already in codebase)
- OpenAL Soft on Windows (FetchContent: `kcat/openal-soft`)
- FFmpeg integration on Windows (already in build system, needs Windows guard review)
- Remove `#ifndef _WIN32` guards from `SDL3GameEngine.cpp` and `OpenALAudioManager.cpp`
- Pre-guards in `windows_compat.h` to prevent DXVK headers conflicting with MSVC `<windows.h>`
- `cmake/dx8.cmake` triple path (DX8 native / DXVK Windows headers / DXVK Linux native)
- `cmake/sdl3.cmake` platform guards (remove Linux-only paths on Windows)
- `cmake/openal.cmake` new file with FetchContent for kcat/openal-soft
- Deploy script: `scripts/deploy_zh_modern.ps1` (copies runtime DLLs + `d3d8.dll`)
- VS Code tasks for configure / build / deploy / run with the new preset
- Documentation updates (README, install notes)

### Out of Scope ❌
- VC6 or `win32` preset changes (must remain untouched and working)
- Audio quality tuning / mixing improvements
- macOS support (Phase 5)
- Video playback beyond FFmpeg stub already in Linux
- Multiplayer / network testing
- 32-bit `win64-modern` variant (architecture is locked to x64)

---

## Architecture

### Runtime Deployment Diagram

```
build/win64-modern/GeneralsMD/
├── GeneralsXZH.exe          ← MSVC x64, SDL entry point
├── SDL3.dll                 ← SDL3 shared library (FetchContent build output)
├── openal32.dll             ← OpenAL Soft (FetchContent build output)
├── d3d8.dll                 ← DXVK (replaces Windows' own d3d8.dll, routes to Vulkan)
├── dxvk.conf                ← Optional DXVK tuning file
└── [game data files...]     ← Copied by deploy script from Run/
```

### Stack Relationship

```
 GeneralsXZH.exe  (MSVC x64)
       │
       ├── Window/Input ─────► SDL3.dll  ──────────► Windows (WS_* handles hidden)
       │
       ├── Audio ────────────► openal32.dll ─────────► WASAPI / WinMM
       │
       └── Graphics ─────────► d3d8.dll  (DXVK)
                                    │
                                    └──► vulkan-1.dll ──► GPU Vulkan driver
```

### Compile-time vs Runtime DXVK

| Phase        | What happens                                               |
|--------------|------------------------------------------------------------|
| **Compile**  | Include DXVK headers from git (same `d3d8.h` as Linux)    |
| **Link**     | No `d3d8.lib`; `d3d8.dll` is loaded at runtime by the EXE |
| **Runtime**  | DXVK's `d3d8.dll` intercepts and translates to Vulkan      |

This is identical in concept to how DXVK works via Proton on Linux. On Windows, the game calls
`Direct3DCreate8()` normally — DXVK's DLL in the same folder intercepts before the system one.

---

## Implementation Plan

### Step 1 — CMakePresets.json: New `win64-modern` preset

**File**: `CMakePresets.json`

Add under `configurePresets`:
```jsonc
{
    "name": "win64-modern",
    "displayName": "Windows 64-bit Modern (DXVK + SDL3 + OpenAL)",
    "inherits": "default-vcpkg",
    "generator": "Ninja",
    "hidden": false,
    "binaryDir": "${sourceDir}/build/${presetName}",
    "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
        "CMAKE_BUILD_TYPE": "RelWithDebInfo",
        "SAGE_USE_DX8": "OFF",
        "SAGE_USE_SDL3": "ON",
        "SAGE_USE_OPENAL": "ON",
        "SAGE_USE_GLM": "ON",
        "RTS_CRASHDUMP_ENABLE": "OFF",
        "RTS_BUILD_OPTION_FFMPEG": "ON"
    }
}
```

Add under `buildPresets`:
```jsonc
{
    "name": "win64-modern",
    "configurePreset": "win64-modern",
    "displayName": "Build Windows 64-bit Modern (DXVK + SDL3 + OpenAL)",
    "description": "Windows MSVC x64 build using DXVK/SDL3/OpenAL backends",
    "configuration": "RelWithDebInfo"
},
{
    "name": "win64-modern-debug",
    "configurePreset": "win64-modern-debug",
    "displayName": "Build Windows 64-bit Modern (Debug)",
    "configuration": "Debug"
}
```

**Note**: No `"architecture": {"value": "Win32"}` — MSVC defaults to x64 in the Ninja generator.
**Contrast with win32**: `win32` specifies `Win32` architecture and `SAGE_USE_DX8` is implicitly
`ON` (default). `win64-modern` sets `SAGE_USE_DX8: OFF` to trigger the DXVK headers path.

---

### Step 2 — cmake/config-build.cmake: Platform-neutral option labels

The options currently carry `"(Linux)"` in their descriptions — misleading for Windows use:

```cmake
# Before:
option(SAGE_USE_SDL3   "Use SDL3 for windowing/input (Linux)" OFF)
option(SAGE_USE_OPENAL "Use OpenAL for audio backend (Linux)" OFF)

# After:
option(SAGE_USE_SDL3   "Use SDL3 for windowing/input (cross-platform)" OFF)
option(SAGE_USE_OPENAL "Use OpenAL for audio backend (cross-platform)" OFF)
```

Also update the corresponding `add_feature_info` descriptions to match.

---

### Step 3 — cmake/openal.cmake: New file, FetchContent kcat/openal-soft

**New file**: `cmake/openal.cmake`

```cmake
# OpenAL Soft audio library
# GeneralsX @build BenderAI 26/02/2026 - Phase 06
# Cross-platform OpenAL implementation via kcat/openal-soft.
# Used on Linux and Windows Modern builds (SAGE_USE_OPENAL=ON).
# On Linux: WASAPI/ALSA/PipeWire backends.
# On Windows: WASAPI backend (primary), WinMM fallback.

if(SAGE_USE_OPENAL)
    message(STATUS "Configuring OpenAL Soft (v1.24.2) with FetchContent...")

    include(FetchContent)

    FetchContent_Declare(
        openal_soft
        GIT_REPOSITORY https://github.com/kcat/openal-soft.git
        GIT_TAG        1.24.2
    )

    # Disable unnecessary build components
    set(ALSOFT_INSTALL_RUNTIME_LIBS  ON  CACHE BOOL "Install runtime libs" FORCE)
    set(ALSOFT_EXAMPLES              OFF CACHE BOOL "Build examples"       FORCE)
    set(ALSOFT_TESTS                 OFF CACHE BOOL "Build tests"          FORCE)
    set(ALSOFT_UTILS                 OFF CACHE BOOL "Build utils"          FORCE)
    set(ALSOFT_NO_CONFIG_UTIL        ON  CACHE BOOL "Disable config util"  FORCE)

    # Windows-specific: prefer WASAPI (modern, low-latency)
    if(WIN32)
        set(ALSOFT_REQUIRE_WASAPI ON CACHE BOOL "Require WASAPI backend on Windows" FORCE)
    endif()

    FetchContent_MakeAvailable(openal_soft)

    # openal-soft creates the OpenAL::OpenAL target
    message(STATUS "OpenAL Soft configured: target OpenAL::OpenAL available")
endif()
```

This file must be `include()`-d in the top-level `CMakeLists.txt` alongside `sdl3.cmake` and `dx8.cmake`.

---

### Step 4 — cmake/sdl3.cmake: Remove Linux-only hardcoded paths

Several options and paths in `sdl3.cmake` are Linux-specific and will either break or produce
wrong results when CMake runs on Windows:

```cmake
# PROBLEM (current):
set(SDL_WAYLAND ON CACHE BOOL "Enable Wayland support (Linux)" FORCE)
set(SDL_X11     ON CACHE BOOL "Enable X11 support (Linux)"     FORCE)
set(PNG_INCLUDE_DIR "/usr/include" ...)
set(PNG_LIBRARY     "/usr/lib/x86_64-linux-gnu/libpng16.so.16" ...)

# FIX: wrap in platform guard:
if(NOT WIN32)
    set(SDL_WAYLAND      ON  CACHE BOOL "Enable Wayland support" FORCE)
    set(SDL_X11          ON  CACHE BOOL "Enable X11 support"     FORCE)
    set(PNG_INCLUDE_DIR  "/usr/include" ...)
    set(PNG_LIBRARY      "/usr/lib/x86_64-linux-gnu/libpng16.so.16" ...)
else()
    # Windows: enable DirectX and WASAPI-backed audio in SDL3
    set(SDL_DIRECTX ON  CACHE BOOL "Enable DirectX support"   FORCE)
    set(SDL_WASAPI  ON  CACHE BOOL "Enable WASAPI audio"      FORCE)
    set(SDL_WAYLAND OFF CACHE BOOL "Disable Wayland (Windows)" FORCE)
    set(SDL_X11     OFF CACHE BOOL "Disable X11 (Windows)"     FORCE)
endif()
```

---

### Step 5 — cmake/dx8.cmake: Triple path for DXVK Windows

Currently the file has two branches: `SAGE_USE_DX8=ON` (Windows, min-dx8-sdk) and `OFF` (Linux,
DXVK-native tarball). We need a third branch: Windows + DXVK headers.

```cmake
if(SAGE_USE_DX8)
    # ── Legacy Windows: DirectX 8 native (used by vc6 and win32 presets) ──────
    FetchContent_Declare(
        dx8
        GIT_REPOSITORY https://github.com/TheSuperHackers/min-dx8-sdk.git
        GIT_TAG        7bddff8c01f5fb931c3cb73d4aa8e66d303d97bc
    )
    FetchContent_MakeAvailable(dx8)
    message(STATUS "Using DirectX 8 SDK (Windows native, legacy)")

elseif(WIN32)
    # ── Windows Modern: DXVK headers (win64-modern preset) ───────────────────
    # Compile against DXVK d3d8.h headers; runtime DLL provided by DXVK release.
    # The DXVK git repo provides headers under include/dxvk/native/d3d8/.
    # Do NOT link against a .lib — d3d8.dll is loaded dynamically at runtime.
    FetchContent_Declare(
        dxvk_headers
        GIT_REPOSITORY https://github.com/doitsujin/dxvk.git
        GIT_TAG        v2.6
        GIT_SHALLOW    TRUE
    )
    # Fetch only (no subdirectory CMakeLists, just headers)
    FetchContent_GetProperties(dxvk_headers)
    if(NOT dxvk_headers_POPULATED)
        FetchContent_Populate(dxvk_headers)
    endif()

    # Download the Windows release zip (contains d3d8.dll for deployment)
    FetchContent_Declare(
        dxvk_win_release
        URL      https://github.com/doitsujin/dxvk/releases/download/v2.6/dxvk-2.6.tar.gz
        URL_HASH SHA256=<hash-to-verify>
    )
    FetchContent_MakeAvailable(dxvk_win_release)

    set(DXVK_INCLUDE_DIR "${dxvk_headers_SOURCE_DIR}/include/dxvk" CACHE PATH "DXVK headers")
    set(DXVK_WIN_DLL_DIR "${dxvk_win_release_SOURCE_DIR}/x64"      CACHE PATH "DXVK Windows DLLs")

    message(STATUS "Using DXVK headers (Windows Modern, DXVK→Vulkan at runtime)")
    message(STATUS "DXVK headers: ${DXVK_INCLUDE_DIR}")
    message(STATUS "DXVK DLL dir: ${DXVK_WIN_DLL_DIR}")

else()
    # ── Linux: DXVK native (linux64-deploy and linux64-testing presets) ───────
    FetchContent_Declare(
        dxvk
        URL https://github.com/doitsujin/dxvk/releases/download/v2.6/dxvk-native-2.6-steamrt-sniper.tar.gz
    )
    FetchContent_MakeAvailable(dxvk)
    message(STATUS "Using DXVK native (Linux, DirectX→Vulkan)")
endif()
```

---

### Step 6 — CompatLib: Pre-guards for DXVK headers on Windows

**Problem**: DXVK's headers define `MEMORYSTATUS`, `IUnknown`, and `LARGE_INTEGER` — types already
fully declared in MSVC's `<windows.h>`. This causes `redefinition` errors exactly as it did on
Linux (solved via pre-guards in Session 28).

**Solution**: Apply the same pre-guard pattern but conditioned for the Windows+DXVK case:

**File**: `GeneralsMD/Code/CompatLib/Include/windows_compat.h`

```cpp
// ── DXVK pre-guards ──────────────────────────────────────────────────────────
// Prevent DXVK headers from redefining types already declared by the platform.
//
// On Linux (no real windows.h): pre-define guards so DXVK skips incomplete stubs.
// On Windows with DXVK (win64-modern): MSVC's <windows.h> has full definitions;
// pre-define guards so DXVK includes do not redefine them.
// On Windows with DX8 (vc6/win32): guards not needed; min-dx8-sdk has no conflicts.

#if !defined(_WIN32) || (defined(_WIN32) && !defined(SAGE_USE_DX8_NATIVE))
    #ifndef _MEMORYSTATUS_DEFINED
        #define _MEMORYSTATUS_DEFINED 1
    #endif
    #ifndef _IUNKNOWN_DEFINED
        #define _IUNKNOWN_DEFINED 1
    #endif
#endif
// ─────────────────────────────────────────────────────────────────────────────
```

**Note**: This must be included (directly or indirectly) before any DXVK header is pulled in.

> **Lesson from Session 28**: Manual patches to DXVK headers in `build/_deps/` are ephemeral —
> they are lost on clean rebuild. Pre-guards in our own source are permanent.

---

### Step 7 — Remove `#ifndef _WIN32` guards from SDL3/OpenAL sources

The guards were added during Linux porting to prevent SDL3GameEngine and OpenALAudioManager from
being compiled on Windows (where they were not needed). Now that we want them on Windows too, the
guards must be replaced with the semantic feature flags.

**Files to change**:

| File | Current guard | Replace with |
|------|---------------|--------------|
| `GeneralsMD/Code/GameEngineDevice/Source/SDL3GameEngine.cpp` | `#ifndef _WIN32` (line 29) | `#ifdef SAGE_USE_SDL3` |
| `GeneralsMD/Code/GameEngineDevice/Source/OpenALAudioManager.cpp` | `#ifndef _WIN32` (line 29) | `#ifdef SAGE_USE_OPENAL` |
| `GeneralsMD/Code/GameEngineDevice/Include/SDL3GameEngine.h` | `#ifndef _WIN32` (line 31) | `#ifdef SAGE_USE_SDL3` |
| `GeneralsMD/Code/GameEngineDevice/Include/SDL3Device/GameClient/SDL3Mouse.h` | `#ifndef _WIN32` (line 30) | `#ifdef SAGE_USE_SDL3` |
| `GeneralsMD/Code/GameEngineDevice/Include/SDL3Device/GameClient/SDL3Keyboard.h` | `#ifndef _WIN32` (line 30) | `#ifdef SAGE_USE_SDL3` |

**Audit command** (run before starting):
```bash
grep -rn "#ifndef _WIN32\|#ifdef _WIN32" \
    GeneralsMD/Code/GameEngineDevice/ \
    --include="*.cpp" --include="*.h" | grep -i "SDL3\|OpenAL\|sdl\|al_"
```

**Important**: Only change guards that wrap SDL3/OpenAL code paths. `#ifdef _WIN32` guards in
`W3DDevice/` (W3DMouse, W3DDisplay, W3DFileSystem) must be left untouched — they separate
legitimate Win32 API calls from POSIX alternatives.

---

### Step 8 — WinMain.cpp: SDL3 entrypoint on Windows

`GeneralsMD/Code/Main/WinMain.cpp` currently runs `RegisterClass` / `CreateWindow` / `WinMain`
unconditionally. When `SAGE_USE_SDL3=ON` on Windows, the `SDL3Main.cpp` entrypoint must be used
instead (SDL hijacks `main`/`WinMain` via `SDL_main.h`).

**Strategy**: Gate on `SAGE_USE_SDL3` at compile time:

```cpp
// WinMain.cpp (top section)
#ifdef SAGE_USE_SDL3
// SDL3 handles the entry point via SDL3Main.cpp.
// This translation unit is excluded from the win64-modern build.
// See GeneralsMD/Code/Main/CMakeLists.txt for source selection.
#else  // !SAGE_USE_SDL3 — legacy Win32 path
// ... existing WinMain code unchanged
#endif
```

In `GeneralsMD/Code/Main/CMakeLists.txt`, the source selection already handles Linux:

```cmake
if(SAGE_USE_SDL3)
    list(APPEND MAIN_SOURCES SDL3Main.cpp)
else()
    list(APPEND MAIN_SOURCES WinMain.cpp)
endif()
```

This same logic will now cover Windows Modern. Verify `SDL3Main.cpp` does not contain any
`#ifndef _WIN32` guards that would prevent it from compiling on Windows.

---

### Step 9 — SDL3Main.cpp: Windows compatibility audit

`GeneralsMD/Code/Main/SDL3Main.cpp` was written for Linux. Before it compiles on Windows, audit for:

1. **POSIX-only headers**: `<unistd.h>`, `<signal.h>`, `<sys/types.h>` — wrap in `#ifndef _WIN32`
2. **`setenv` calls**: Windows lacks `setenv`. For DXVK WSI driver selection, use `SDL_setenv_unsafe`
   or `_putenv_s` behind `#ifdef _WIN32`. The `DXVK_WSI_DRIVER` env var may not be needed on Windows
   since DXVK-Windows uses its own WSI.
3. **`SDL_Vulkan_LoadLibrary`**: Should work on Windows if Vulkan SDK is installed; add error message
   if `vulkan-1.dll` is not found.

```cpp
// SDL3Main.cpp — DXVK WSI env var (Linux only)
#ifndef _WIN32
    // Linux: tell DXVK to use SDL3 as the WSI (window system interface)
    setenv("DXVK_WSI_DRIVER", "SDL3", 1);
#endif
// Windows: DXVK-Windows handles WSI natively (Win32 HWND from SDL3)
```

---

### Step 10 — vcpkg.json: Add openal-soft for Windows

To allow vcpkg to supply OpenAL headers if needed (fallback to FetchContent if not):

```jsonc
{
    "dependencies": [
        "zlib",
        "glm",
        "gli",
        { "name": "freetype",   "platform": "!windows" },
        { "name": "fontconfig", "platform": "!windows" },
        { "name": "openal-soft", "platform": "!vc6" }
    ]
}
```

The `"platform": "!vc6"` syntax is not valid vcpkg syntax; use `"platform": "windows"` with a
separate exclusion or rely solely on FetchContent (preferred since it's already consistent with
SDL3 and DXVK).

---

### Step 11 — Deploy script: `scripts/deploy_zh_modern.ps1`

New PowerShell script that deploys the `win64-modern` build with all required runtime DLLs:

```powershell
# scripts/deploy_zh_modern.ps1
# GeneralsX @build BenderAI 26/02/2026 - Phase 06
# Deploys win64-modern build + DXVK/SDL3/OpenAL runtime DLLs.

param(
    [string]$Preset = "win64-modern",
    [string]$GameDir = "$PSScriptRoot\..\GeneralsMD\Run"
)

$BuildDir = "$PSScriptRoot\..\build\$Preset\GeneralsMD"
$OutDir   = $GameDir

# Copy EXE
Copy-Item "$BuildDir\GeneralsXZH.exe" $OutDir -Force

# Copy DXVK Windows DLLs (x64)
$DxvkDir = "$PSScriptRoot\..\build\_deps\dxvk_win_release-src\x64"
Copy-Item "$DxvkDir\d3d8.dll"   $OutDir -Force
Copy-Item "$DxvkDir\d3d9.dll"   $OutDir -Force  # DXVK may bundle d3d9

# Copy SDL3
Copy-Item "$BuildDir\_deps\sdl3-build\SDL3.dll" $OutDir -Force

# Copy OpenAL Soft
Copy-Item "$BuildDir\_deps\openal_soft-build\OpenAL32.dll" $OutDir -Force

# Copy FFmpeg DLLs (if built)
Get-ChildItem "$BuildDir" -Filter "av*.dll" | Copy-Item -Destination $OutDir -Force
Get-ChildItem "$BuildDir" -Filter "sw*.dll" | Copy-Item -Destination $OutDir -Force

Write-Host "Deployed win64-modern to $OutDir"
```

---

### Step 12 — Vulkan SDK prerequisite on build machine

DXVK on Windows requires `vulkan-1.lib` at **link time** and `vulkan-1.dll` at **runtime** (the
latter is provided by GPU drivers, not by us). The build machine needs:

- [LunarG Vulkan SDK](https://vulkan.lunarg.com/sdk/home) installed
- Environment variable `VULKAN_SDK` must point to install directory
- CMake will find it via `find_package(Vulkan REQUIRED)` — already present in `dx8.cmake`

**Detection in CMake** (add to `dx8.cmake` Windows DXVK branch):
```cmake
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR
        "Vulkan SDK not found. Required for win64-modern.\n"
        "Install from: https://vulkan.lunarg.com/sdk/home\n"
        "Then set VULKAN_SDK environment variable.")
endif()
target_link_libraries(RTS PRIVATE Vulkan::Vulkan)
```

---

### Step 13 — VS Code tasks for win64-modern

Add to `.vscode/tasks.json`:

```jsonc
{
    "label": "Configure (Windows win64-modern)",
    "type": "process",
    "command": "powershell",
    "args": ["-NoProfile", "-ExecutionPolicy", "Bypass", "-Command",
        "& { . .\\scripts\\setup_msvc_buildtools_env.ps1; cmake --preset win64-modern }"],
    "group": "build"
},
{
    "label": "Build GeneralsXZH (Windows win64-modern)",
    "type": "process",
    "command": "powershell",
    "args": ["-NoProfile", "-ExecutionPolicy", "Bypass", "-Command",
        "& { . .\\scripts\\setup_msvc_buildtools_env.ps1; cmake --build --preset win64-modern --target z_generals }"],
    "group": { "kind": "build" }
},
{
    "label": "Deploy GeneralsXZH (Windows win64-modern)",
    "type": "shell",
    "command": ".\\scripts\\deploy_zh_modern.ps1",
    "group": "build"
},
{
    "label": "Pipeline: Configure + Build + Deploy + Run (win64-modern)",
    "type": "process",
    "command": "powershell",
    "args": ["-NoProfile", "-ExecutionPolicy", "Bypass", "-Command",
        "& { . .\\scripts\\setup_msvc_buildtools_env.ps1; cmake --preset win64-modern; cmake --build --preset win64-modern --target z_generals; .\\scripts\\deploy_zh_modern.ps1; .\\scripts\\run_zh.ps1 -Preset win64-modern }"],
    "group": "test"
}
```

---

## Risks & Mitigations

| Risk | Severity | Mitigation |
|------|----------|------------|
| DXVK headers conflict with MSVC `<windows.h>` | HIGH | Pre-guard pattern in `windows_compat.h` (proven on Linux, Step 6) |
| SDL3 FetchContent on Windows needs additional build libs | MEDIUM | SDL3 MSVC CI is official and tested; Windows CI libs (DXGI, D3D11) are system-provided |
| POSIX calls in SDL3Main.cpp / LinuxStubs.cpp | MEDIUM | Audit each `#ifndef _WIN32` usage; move to `SAGE_USE_SDL3` (Step 9) |
| 64-bit pointer/int casts in legacy game code | MEDIUM | Treat as MSVC warnings `/W4`; fix with `static_cast` on CI output |
| OpenAL Soft MSVC build quirks | LOW | kcat/openal-soft has official MSVC CI; well-tested |
| `d3d8.dll` (DXVK) shadow system DLL | LOW | Folder-local DLL takes priority — this is by design |
| Vulkan SDK not installed on build machine | LOW | Fail fast with clear CMake error (Step 12) |
| Miles Sound System stubs on x64 | LOW | Stubs already compile on Linux (same flags); verify x64 ABI |
| DXVK-Windows requires GPU with Vulkan support | INFO | Any GPU with Vulkan 1.1+ works (AMD/NVIDIA/Intel since ~2016) |

---

## Acceptance Criteria

Phase 6 is **COMPLETE** when all of the following are satisfied:

- [ ] `cmake --preset win64-modern` configures without errors on Windows 10/11
- [ ] `cmake --build --preset win64-modern --target z_generals` compiles cleanly (0 errors, <50 warnings)
- [ ] `deploy_zh_modern.ps1` copies EXE + DLLs to game directory
- [ ] `GeneralsXZH.exe` launches and displays the main menu with working graphics via Vulkan
- [ ] Audio is audible during gameplay (OpenAL Soft WASAPI backend active)
- [ ] Basic skirmish map loads and plays without crash
- [ ] Window management works (fullscreen toggle, resize, minimize/restore) via SDL3
- [ ] Input (keyboard + mouse) works correctly in-game via SDL3
- [ ] **Zero regressions** on `vc6` and `win32` presets (must still build and run)
- [ ] DXVK diagnostic log confirms D3D8 → Vulkan translation active (`DXVK_LOG_LEVEL=info`)

---

## Prerequisites for Starting

- [ ] Phase 1 Linux build is stable (can run a skirmish on Linux)
- [ ] Vulkan SDK installed on Windows dev machine
- [ ] `win32` preset builds and runs (regression baseline established)
- [ ] `VULKAN_SDK` environment variable is set and `vulkaninfo` runs successfully

---

## Key Files Reference

| File | Purpose in Phase 6 |
|------|-------------------|
| `CMakePresets.json` | Add `win64-modern` configure + build presets |
| `cmake/config-build.cmake` | Update option descriptions; neutral platform labels |
| `cmake/dx8.cmake` | Add Windows DXVK headers branch |
| `cmake/sdl3.cmake` | Add `if(WIN32)` guards for Linux-specific paths |
| `cmake/openal.cmake` | New file: FetchContent kcat/openal-soft |
| `GeneralsMD/Code/CompatLib/Include/windows_compat.h` | Pre-guards for DXVK on Windows |
| `GeneralsMD/Code/GameEngineDevice/Source/SDL3GameEngine.cpp` | Remove `#ifndef _WIN32` guard |
| `GeneralsMD/Code/GameEngineDevice/Source/OpenALAudioManager.cpp` | Remove `#ifndef _WIN32` guard |
| `GeneralsMD/Code/GameEngineDevice/Include/SDL3GameEngine.h` | Remove `#ifndef _WIN32` guard |
| `GeneralsMD/Code/Main/SDL3Main.cpp` | Audit POSIX-only code; add `#ifndef _WIN32` where needed |
| `scripts/deploy_zh_modern.ps1` | New deploy script for DLL distribution |
| `.vscode/tasks.json` | Add win64-modern configure/build/deploy/run tasks |
| `vcpkg.json` | Optional: add openal-soft if FetchContent approach is not used |

---

## Session Progress Log

Sessions working on this phase should update this section:

| Session | Date | Completed |
|---------|------|-----------|
| —       | —    | Phase planned |
