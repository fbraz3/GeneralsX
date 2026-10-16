# Upstream Sync Plan: TheSuperHackers (10-16-2026)

## 1. Overview and Objective
- **Branch**: `thesuperhackers-sync-10-16-2026`
- **Upstream Remote**: `thesuperhackers` (`git@github.com:TheSuperHackers/GeneralsGameCode.git`)
- **Incoming Commits**: 49 commits from `thesuperhackers/main` up to `0af9db488`
- **Objective**: Reconcile upstream stability, determinism, and bug fixes while strictly preserving GeneralsX's cross-platform architecture (SDL3, DXVK, MiniAudio/OpenAL, FFmpeg, NGMP multiplayer, and bit-identical math determinism).

---

## 2. Incoming Upstream Changes Analysis
Key upstream improvements and structural changes being merged:
1. **ArchiveFileSystem Unification** (`f29956197`, `e4b4d1e94`, `52a777aa0`):
   - Merged `Win32BIGFileSystem` and `StdBIGFileSystem` into a single `BIGFileSystem` in `Core/GameEngine/Source/Common/System/BIGFileSystem.cpp` and `Core/GameEngine/Include/Common/BIGFileSystem.h`.
   - Deleted `StdBIGFileSystem.h` and `StdBIGFileSystem.cpp`.
   - Unified `Win32BIGFile` and `StdBIGFile` into `ArchiveFile`.
   - Added mod loading support (`loadMods`).
2. **Debug & MiniDumper Subsystem Decoupling** (`79e87d082`, `502ef2c6c`, `3be561fca`, `0daf80285`, `9a9a9729c`, `3300a5b89`):
   - Extracted `MiniDumper` to a dedicated static library.
   - Decoupled `Debug.cpp` with host callbacks (`DebugSetCrashHandler`) and moved engine-independent debug core to `Core/Libraries/Source/debug/` (`core_debug`).
3. **Trig Math & Lib Unification** (`16bfbd654`):
   - Moved `Trig.cpp` to `Core/Libraries/Source/Lib/Trig.cpp`.
   - Deleted redundant `Generals/Code/GameEngine/Source/Common/System/Trig.cpp`.
4. **Pathfinder Modularization** (`53fb940c9`, `0b53cb51b`, `9eb2ec859`, `21b926283`, `2dbd42926`, `a4d96e994`, `d28c90992`, `5c18f11af`, `c427b2cdb`, `3ecdcd3ad`):
   - Decomposed monolithic `AIPathfind.cpp` into discrete headers and implementations (`PathfindLayer`, `PathfindZoneManager`, `ZoneBlock`, `PathfindCell`, `PathfindCellList`, `PathfindCellInfo`, `Path`, `PathNode`).
5. **Renderer & WW3D Refactoring** (`ab568af44`, `fd62135da`):
   - Replaced dynamic `RenderBackend` pointer with static `Renderer` calls (`Renderer::Begin_Scene()`, etc.).
6. **Locomotor & Physics Diagonal Speed Discrepancy** (`d1bd190af`):
   - Corrected 2D/3D forward speed calculation to use dot projection while preserving retail compatibility paths (`useLegacyForwardSpeed`).
7. **Font Cache Optimization & Sentence Texture Growth** (`98b1d71bf`, `255f18a94`, `4b865489e`, `f924fd333`, `2b34c7e8c`):
   - Replaced full texel glyph cache with 1 byte per glyph pixel coverage, dynamically sized font scratch bitmaps, and discarded glyph caches on map loads and resolution changes.
8. **INI Numeric Parser Legacy Compatibility** (`aee68d6e5`):
   - Preserves legacy parsing for out-of-range numbers via `from_chars` and fallback conversions.
9. **Unit Testing & Benchmarks Framework** (`adac468d7`, `1b6af7470`, `64e8392d8`, `26b719959`):
   - Integrated Google Test and Google Benchmark subdirectories under `Tests/` and `cmake/`.

---

## 3. Conflict Analysis & Resolution Strategy

### Subsystem A: CI/CD & Repository Infrastructure
- Files: `.github/copilot-instructions.md`, `.github/workflows/ci.yml`, `.github/workflows/reusable-build-toolchain.yml`
- **Rule 62**: Never replace our CI/CD infrastructure with upstream versions. Keep our multi-platform matrix (Linux Flatpak, macOS ARM64, Windows, Replay Tests) intact.
- **Resolution**: Restore HEAD version for all `.github/` files (`git checkout HEAD -- .github/`).

### Subsystem B: Root Build Configuration & Dependencies
1. **`CMakeLists.txt`**:
   - Upstream added `googletest.cmake`, `googlebenchmark.cmake`, and `add_subdirectory(Dependencies/Tests/Benchmarks)`.
   - GeneralsX added `sage_patch` dependency, DXVK options, and multi-platform flags.
   - **Resolution**: Keep GeneralsX's target rules and hook in upstream's `googletest`/`googlebenchmark` blocks when enabled.
2. **`cmake/config-build.cmake`**:
   - Upstream added `RTS_BUILD_OPTION_TESTS` and `RTS_BUILD_OPTION_BENCHMARKS`.
   - GeneralsX added `RTS_BUILD_OPTION_FFMPEG`, `RTS_BUILD_OPTION_DEEP_CRC`, `SAGE_USE_SDL3`, `SAGE_USE_OPENAL`, `SAGE_USE_MINIAUDIO`, `SAGE_UPDATE_CHECK`, `SAGE_USE_MOLTENVK`, `RTS_BUILD_OPTION_SAGE_PATCH`, `SAGE_USE_NGMP`, and UNIX libc definitions (`HAVE_STRLCPY`, etc.).
   - **Resolution**: Combine both sets of options. Retain all GeneralsX feature flags and add the unit test/benchmark flags.

### Subsystem C: Filesystem & Asset Loading (Core/GameEngine)
1. **`Core/GameEngine/Include/Common/BIGFileSystem.h` & `Core/GameEngine/Source/Common/System/BIGFileSystem.cpp`**:
   - Upstream merged `Win32BIGFileSystem` and `StdBIGFileSystem` into `BIGFileSystem`, adding `loadMods`.
   - GeneralsX has vital asset resolution logic: `loadPrimaryGameAssets` (environment variables `GENERALSX_ZH_ASSET_PATH`, flatpak paths, steam directory detection), `loadBaseGeneralsAssetsForZH`, `loadOneArchiveFile` (duplicate archive prevention), and `patchFilenameList` / `isPatchArchiveFilename` for INI patch precedence.
   - **Resolution**: Port all GeneralsX asset detection, environment variables, duplicate protection, and patch archive sorting into the unified `BIGFileSystem`. Adopt upstream's `loadMods()`.
2. **`Core/GameEngineDevice/Include/StdDevice/Common/StdBIGFileSystem.h` & `StdBIGFileSystem.cpp`**:
   - Deleted by upstream. Now redundant since all functionality is in `BIGFileSystem`.
   - **Resolution**: Delete via `git rm`.
3. **`Core/GameEngine/Include/Common/GameDefines.h`**:
   - GeneralsX uses 1024x768 default resolution (`DEFAULT_DISPLAY_WIDTH 1024`, `DEFAULT_DISPLAY_HEIGHT 768`).
   - Upstream added non-tweakable retail physics discrepancy macros (`USE_RETAIL_PHYSICS_FORWARD_SPEED_DISCREPANCY`, etc.).
   - **Resolution**: Keep GeneralsX's 1024x768 resolution and integrate upstream's new non-tweakable defines.

### Subsystem D: Engine Core & Platform Abstraction
1. **`Core/GameEngine/Source/Common/INI/INI.cpp`**:
   - High-risk area on macOS.
   - Upstream added fallback for `std::from_chars` out-of-range via `std::strtoll`/`std::strtoull`/`std::strtof`.
   - GeneralsX has `#if defined(__APPLE__)` fallback using `std::strtod` due to Apple Clang `std::from_chars` float quirks, integer saturation to field limits (`#297`), and stderr logging.
   - **Resolution**: Reconcile both: keep Apple Clang floating-point safety and integer saturation while adopting upstream's `+` prefix handling and out-of-range fallback logic.
2. **`Core/GameEngine/Source/Common/System/Debug.cpp`**:
   - Upstream decoupled MiniDumper and added `DebugSetCrashHandler`.
   - GeneralsX has POSIX `THREAD_ID` support, stderr fatal logging for headless/Linux/macOS, and Unicode MessageBox guards.
   - **Resolution**: Preserve GeneralsX's POSIX thread handling and stderr crash outputs while adopting upstream's `MiniDumper/MiniDumper.h` path and `crashHandler` callback.
3. **`Core/GameEngine/Source/Common/System/GameMemoryNull.cpp`**:
   - Merge upstream's `#include "Common/GameMemory.h"` and GeneralsX's `<stdlib.h>` vs `<malloc.h>` Apple guard.
4. **`Core/GameEngine/Source/GameLogic/AI/AIPathfind.cpp`**:
   - Upstream cleanly split out `PathfindLayer`, `ZoneBlock`, `PathfindCell`, etc.
   - **Resolution**: Accept upstream's modularized pathfind files. Ensure headers compile and no non-deterministic math leaks exist.
5. **`Core/GameEngineDevice/CMakeLists.txt`**:
   - Upstream unconditionally linked `deps_bink` and `deps_miles`.
   - GeneralsX isolates Windows-only dependencies under `if(WIN32)` and supports `SAGE_USE_OPENAL`, `SAGE_USE_MINIAUDIO`, and `RTS_BUILD_OPTION_FFMPEG`.
   - Upstream removed `Win32BIGFile` and `StdBIGFileSystem` from this target.
   - **Resolution**: Remove deleted BIG file sources, preserve `if(WIN32)` isolation, and keep OpenAL, MiniAudio, and POSIX network interface.
6. **`Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`**:
   - Upstream added `Free_All_FontChars_Glyph_Caches()` in `setDisplayMode` and `reset()`.
   - GeneralsX added `TheW3DPostProcess->reset()` and SDL3 window mode updates.
   - **Resolution**: Keep both: reset post-processing and flush glyph caches.
7. **`Core/Libraries/Source/Lib/Include/D3D8Stub.h` & `Trig.cpp`**:
   - `Core/Libraries/Source/Lib/Trig.cpp` already includes `WWMath::SinTrig` and `WWMath::ACosTrig`.
   - Upstream moved `Trig.cpp` to `Lib/` and deleted `Generals/.../Trig.cpp`.
   - **Resolution**: Delete duplicate `Generals/Code/GameEngine/Source/Common/System/Trig.cpp` via `git rm`. Retain `D3D8Stub.h` for stub types if needed.
8. **`Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`**:
   - Upstream added `Shutdown()` on failed Direct3D init.
   - GeneralsX added DXVK environment fallback and dlerror logging for Linux/macOS.
   - **Resolution**: Call `Shutdown()` on failure while preserving GeneralsX's logging and DXVK configuration fallback.
9. **`Core/Libraries/Source/WWVegas/WW3D2/render2dsentence.cpp`**:
   - Upstream optimized glyph caching, added `Update_Current_Buffer`, and bounded character extents.
   - GeneralsX has FreeType support and Font Awesome 6 Brands fallback for Windows.
   - **Resolution**: Reconcile both: preserve FreeType hooks and FA fallback while integrating upstream's glyph buffer optimizations and extent clamping.
10. **`Core/Libraries/Source/WWVegas/WW3D2/ww3d.cpp`**:
    - Upstream converted renderer to static `Renderer::Begin_Scene()` and kept full state cache invalidation.
    - GeneralsX added `DX8Wrapper::Set_Transform_Dirty()` and depth-test state initialization.
    - **Resolution**: Adopt `Renderer::` static calls while preserving GeneralsX's dirty transform optimization and Z-buffer state resets.
11. **`Core/Libraries/Source/debug/CMakeLists.txt` & `profile/`**:
    - `core_debug` has Windows dependencies (`windows.h`, `__DebugIncludeInLink1`).
    - **Resolution**: On non-Windows, provide interface library `add_library(core_debug INTERFACE)` so linker references resolve cleanly without trying to compile Windows-only debug code. Adopt qualified include paths (`Include/profile/profile.h`).

### Subsystem E: Product Game Code (Generals & GeneralsMD)
1. **`AIStates.cpp` (Generals & GeneralsMD)**:
   - Both upstream and GeneralsX addressed temporary state reset during update.
   - **Resolution**: Preserve GeneralsX's guard which cleanly prevents exiting an already-reset or changed temporary state.
2. **`PhysicsUpdate.cpp` (Generals & GeneralsMD)**:
   - Upstream optimized diagonal locomotor forward speed with dot product, adding `useLegacyForwardSpeed()`.
   - Upstream used raw `sqrtf` in `useLegacyForwardSpeed()`.
   - **Resolution**: Integrate upstream's dot-product forward speed optimization, but strictly replace `sqrtf` with deterministic `WWMath::SqrtfOrigin`!
3. **`W3DShroud.cpp` (Generals & GeneralsMD)**:
   - Upstream replaced `m_pSrcTexture` with `m_shroudData`.
   - GeneralsX added null safety checks for campaign water rendering.
   - **Resolution**: Check `!m_shroudData` instead of `!m_pSrcTexture`.
4. **Game CMakeLists (`GeneralsMD/Code/GameEngine/CMakeLists.txt`, `Generals/Code/Main/`, `GeneralsMD/Code/Main/`, etc.)**:
   - GeneralsX preserves NGMP (`SAGE_USE_NGMP`), executable output names (`GeneralsX`, `GeneralsXZH`), SDL3, and `if(WIN32)` isolation.
   - **Resolution**: Keep all GeneralsX target names, NGMP sources, SDL3 links, and Windows dependency isolation.

---

## 4. Execution Steps
1. Revert/preserve CI/CD infrastructure (`.github/`).
2. Resolve root build files (`CMakeLists.txt`, `cmake/config-build.cmake`).
3. Reconcile `BIGFileSystem.h` and `BIGFileSystem.cpp`; remove `StdBIGFileSystem.*` via `git rm`.
4. Remove duplicate `Generals/Code/GameEngine/Source/Common/System/Trig.cpp` via `git rm`.
5. Resolve Core library conflicts (`Debug.cpp`, `GameMemoryNull.cpp`, `AIPathfind.cpp`, `W3DDisplay.cpp`, `dx8wrapper.cpp`, `render2dsentence.cpp`, `ww3d.cpp`, `debug/`, `profile/`).
6. Resolve game-level conflicts in `Generals/` and `GeneralsMD/` (`PhysicsUpdate.cpp` with `WWMath::SqrtfOrigin`, `AIStates.cpp`, `W3DShroud.cpp`, and CMakeLists).
7. Audit for remaining duplicate files between `Core/` and `Generals/`/`GeneralsMD/`.
8. Verify zero merge markers remain (`git diff --check`).
9. Validate CMake configure on macOS (`cmake --preset macos-vulkan`).
10. Build target `z_generals` and `g_generals`.
11. Run cleanups and commit final merge.
