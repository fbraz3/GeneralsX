# Upstream Sync Plan: TheSuperHackers (10-07-2026)

## 1. Overview and Objective
- **Branch**: `thesuperhackers-sync-10-07-2026`
- **Upstream Remote**: `thesuperhackers` (`git@github.com:TheSuperHackers/GeneralsGameCode.git`)
- **Incoming Commits**: 23 commits from `thesuperhackers/main` up to `2bfaea60b`
- **Objective**: Reconcile upstream stability, determinism, and bug fixes while strictly preserving GeneralsX's cross-platform architecture (SDL3, DXVK, MiniAudio/OpenAL, FFmpeg, NGMP multiplayer, and bit-identical math determinism).

---

## 2. Incoming Upstream Changes Analysis
Key upstream improvements being merged:
1. **Scripted Particle Cannon Attacks** (`18068d0e1`, `832360698`): Fix crash and reset manual control when scripted particle cannon attacks target objects.
2. **Terrain Logic Bounds** (`28170001f`): Correct row scan bounds in `TerrainLogic::flattenTerrain()` and `TerrainLogic::createCraterInTerrain()` to avoid redundant/out-of-bounds terrain sampling, with `RETAIL_COMPATIBLE_CRC` guarding row 0.
3. **AI States & Player Fixes** (`3f0c898f5`, `b0c29eba8`, `5430d9def`): Retail compatibility in `AIAttackState::onEnter`, initialization of uninitialized variable in `AIPlayer::onUnitProduced`, and supply attack timing fixes.
4. **Target Naming Unification** (`ff53184e2`): Dependencies standardized with `deps_` prefix (`deps_dbghelp`, `deps_usp10`, `deps_miles`, `deps_bink`).
5. **String and Filesystem Robustness** (`0ba1a4fe4`, `3f3ed9fc8`, `f8ba7eb44`): Token clearing in `AsciiString`/`UnicodeString::nextToken()` and path tokenization in archive/local filesystems.
6. **FieldParse Terminators** (`9298f33b5`, `10f5504b2`): Missing null terminators in INI `FieldParse` tables.
7. **Physics Interpolation** (`6e21c946d`): Interpolate physics transforms to decouple rendering from logic step in `Drawable.cpp`.
8. **WorldBuilder Tool Unification** (`5ae042caf`): Aligned `WHeightMapEdit` across Generals and Zero Hour.

---

## 3. Conflict Analysis & Resolution Strategy

### Subsystem A: Build System & Dependencies
1. **`CMakeLists.txt`**
   - **Conflict**: Target naming for `dbghelploader` / `usp10loader` vs `deps_dbghelp` / `deps_usp10`.
   - **Resolution**: Adopt `deps_dbghelp` and `deps_usp10` interface libraries on non-Windows (`NOT WIN32`), retaining `target_include_directories` and `target_sources(deps_dbghelp INTERFACE .../DbgHelpGuard.cpp)` so Linux and macOS builds receive required stubs.
2. **`Core/GameEngineDevice/CMakeLists.txt`**
   - **Conflict**: Upstream links `deps_bink`, `deps_miles`, and `d3d8lib` unconditionally in `corei_gameenginedevice_public`.
   - **Resolution**: Keep platform isolation. Only link `deps_bink` and `deps_miles` on Windows (`if(WIN32)`). Preserve Linux/macOS paths: OpenAL (`SAGE_USE_OPENAL`), MiniAudio (`SAGE_USE_MINIAUDIO`), and FFmpeg (`RTS_BUILD_OPTION_FFMPEG`).
3. **`Core/Libraries/Source/WWVegas/CMakeLists.txt`**
   - **Conflict**: Renamed dependency targets (`deps_dbghelp`, `deps_miles`, `deps_usp10`).
   - **Resolution**: Use `deps_dbghelp`, `deps_miles`, and `deps_usp10` while maintaining `d3d8lib` and `stlport` interfaces.
4. **`Generals/Code/Libraries/Source/WWVegas/CMakeLists.txt` & `GeneralsMD/Code/Libraries/Source/WWVegas/CMakeLists.txt`**
   - **Conflict**: `milesloader` vs `deps_miles`.
   - **Resolution**: Update to `deps_miles`.
5. **`Generals/Code/Main/CMakeLists.txt` & `GeneralsMD/Code/Main/CMakeLists.txt`**
   - **Conflict**: Upstream unconditionally links Windows libraries (`comctl32`, `d3d8`, `dinput8`, `dxguid`, `deps_bink`, `deps_miles`, `imm32`, `vfw32`, `winmm`).
   - **Resolution**: Keep Windows libraries isolated under `if(WIN32)` using the new `deps_` target names. Retain `GeneralsX` and `GeneralsXZH` target output names and `sdl3lib` links for SDL3.

### Subsystem B: Platform Abstraction & Header Guards
1. **`Core/GameEngine/Include/Precompiled/PreRTS.h`**
   - **Conflict**: Upstream MinGW ATL compatibility vs GeneralsX non-Windows `windows_compat.h`.
   - **Resolution**: Retain `#ifdef _WIN32` guard for Windows/MinGW ATL inclusions, and `#else` with `#include "windows_compat.h"` for Linux and macOS.
2. **`Core/GameEngineDevice/Source/MilesAudioDevice/MilesAudioManager.cpp`**
   - **Conflict**: Direct inclusion of `<dsound.h>` vs `#ifdef _WIN32` guard.
   - **Resolution**: Preserve `#ifdef _WIN32` around `#include <dsound.h>`.
3. **`Core/GameEngineDevice/Source/StdDevice/Common/StdLocalFileSystem.cpp`**
   - **Conflict**: Path normalization typo fix comment.
   - **Resolution**: Accept upstream fix with unified comment.
4. **`Core/GameEngineDevice/Source/W3DDevice/GameClient/GUI/GUICallbacks/W3DMainMenu.cpp`**
   - **Conflict**: Direct `<windows.h>` and `<mmsystem.h>` inclusion vs `#ifdef _WIN32` guard.
   - **Resolution**: Preserve `#ifdef _WIN32` guard to prevent compilation errors on non-Windows.

### Subsystem C: Simulation & Determinism
1. **`Core/GameEngine/Source/GameLogic/Map/TerrainLogic.cpp`**
   - **Conflict**: `TerrainLogic::createCraterInTerrain()` loop bounds and retail CRC compatibility vs GeneralsX NaN bounds checking and `WWMath::SqrtOrigin`.
   - **Resolution**: Keep GeneralsX NaN/inf guards (`std::isfinite`), `mapExtent` bounding clamp, and deterministic `WWMath::SqrtOrigin()`. Integrate upstream's loop bound optimization `for (Int j = iMin.y; j <= iMax.y; ++j)` and `#if RETAIL_COMPATIBLE_CRC iMin.y = std::max(0, iMin.y); #endif`.

---

## 4. Execution Steps
1. Resolve each of the 12 files following the strategy above.
2. Verify zero merge markers remain (`git diff --check` / grep for `<<<<<<<`).
3. Audit for duplicated/orphaned files between `Core/` and `Generals/`/`GeneralsMD/`.
4. Run CMake configure (`cmake --preset macos-vulkan` or native build check).
5. Compile and validate targets.
6. Clean build artifacts and commit.
7. Push `thesuperhackers-sync-10-07-2026` branch.
