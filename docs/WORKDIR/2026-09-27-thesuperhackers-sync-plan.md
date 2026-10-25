# Conflict Resolution Plan: TheSuperHackers Upstream Sync (2026-09-27)

## 1. Overview & Objectives
- **Target Branch**: `thesuperhackers-sync-09-27-2026`
- **Upstream Target**: `thesuperhackers/main` (weekly-2026-09-25 + subsequent commits up to b687c073b)
- **Merge Base**: `e4017100cdb1f1cf7b14af5d874d63f458da44cf`
- **Objective**: Import upstream bug fixes, refactorings, optimizations, and Core unifications while strictly preserving the GeneralsX cross-platform architecture (SDL3, DXVK, MiniAudio, FFmpeg, NGMP multiplayer, 64-bit/ARM compatibility, and cross-platform determinism).
- **Core Duplication Watch**: Actively audit and eliminate duplicate files between `Generals/`, `GeneralsMD/`, and `Core/`, ensuring unified implementations in `Core/` contain all GeneralsX platform additions.

---

## 2. Inventory of Conflicts & Resolution Strategy

### Section A: Files Moved to Core / Unified Structure (High Priority: Prevent Duplicates)
1. **`ThingTemplate` (`Core/GameEngine/.../ThingTemplate.*` vs `Generals/Code/.../ThingTemplate.*`)**
   - **Status**: UD on `Generals/Code/GameEngine/Source/Common/Thing/ThingTemplate.cpp`. Upstream moved `ThingTemplate` to `Core/`.
   - **Resolution**: `Core/GameEngine/Source/Common/Thing/ThingTemplate.cpp` already contains the 64-bit integer cast fixes (`uintptr_t`). Remove `Generals/Code/GameEngine/Source/Common/Thing/ThingTemplate.cpp` with `git rm` to prevent duplicate implementations.
2. **`W3DDisplay` Subsystem (`Core/GameEngineDevice/.../W3DDisplay.*` vs `Generals/Code/.../W3DDisplay.*`)**
   - **Status**: Upstream unified `W3DDisplay`, `W3DDebugDisplay`, `W3DDebugIcons`, `W3DDisplayString`, and `W3DDisplayStringManager` into `Core/GameEngineDevice/`. Git left `UD` on `Generals/.../W3DDisplay.h` and `Generals/.../W3DDisplay.cpp`, and `UU` on `Core/.../W3DDisplay.cpp`.
   - **Resolution**:
     - Port all GeneralsX platform and display features into `Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`:
       - SDL3 window mode application and centering (`SDL3_ApplyWindowModeForRenderConfig`, `SDL3_CenterWindowOnCurrentDisplay`, `SDL3_EnsureNativeFullscreen`)
       - Filtered resolution cache (`buildFilteredResolutions`, `getDisplayModeCount`, `getDisplayModeDescription`)
       - Pillarbox helpers (`Pillarbox_Get_Rect`, `Pillarbox_Process_Resize`, `Pillarbox_Begin`)
       - Window showing after DXVK init (`SDL_ShowWindow(TheSDL3Window)`)
       - Texture depth normalization for DXVK (`Set_Texture_Bitdepth(getBitDepth() == 16 ? 16 : 32)`)
       - FPS string with buffer count (`TheNetwork->getBufferedFramesAvailable()`)
       - Cross-platform MAX/MIN macros for clipping rects
     - Remove the orphaned duplicate files in `Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DDisplay.h` and `Source/.../W3DDisplay.cpp` with `git rm`.
3. **Map Trigger & Logic Duplicates (`Generals/Code/.../PolygonTrigger.cpp`, `TerrainLogic.cpp`)**
   - **Status**: DU (deleted in GeneralsX, modified upstream with cosmetic comment reformatting).
   - **Resolution**: These implementations are already unified under `Core/GameEngine/Source/GameLogic/Map/`. Delete the legacy `Generals/` copies with `git rm` to avoid duplication.
4. **DbgHelp / Crash Handling (`Core/Libraries/Source/WWVegas/WWLib/DbgHelpLoader.*`)**
   - **Status**: UD (upstream moved them to `Dependencies/DbgHelp/`).
   - **Resolution**: Accept upstream removal (`git rm`) as these are now housed in `Dependencies/DbgHelp`.

---

### Section B: Base Types, Precompilation & Dependencies
1. **`Core/Libraries/Include/Lib/BaseTypeCore.h` (UD)**
   - Upstream split `BaseType.h` and renamed `BaseTypeCore.h` into `Precompiled/Precompiled/BasePragmas.h`.
   - GeneralsX had no divergent edits against the merge base.
   - Accept upstream removal (`git rm`).
2. **`Core/Libraries/Include/Lib/BaseType.h` (UU)**
   - Upstream factored out math types into `RGBColor.h`, `Coord2D.h`, `Coord3D.h`, `Region3D.h`, etc.
   - GeneralsX had no custom edits in `BaseType.h`. Accept upstream refactor.
3. **`Core/Libraries/Source/WWVegas/WWLib/bittype.h` (UU)**
   - Upstream moved `wchar_t` typedef for VC6 to `CppTypes.h`.
   - Accept upstream version.

---

### Section C: Build System & CMake
1. **`Core/Libraries/Source/WWVegas/CMakeLists.txt`**
   - Reconcile `target_link_libraries(core_wwcommon INTERFACE ...)`: keep `d3d8lib` comment, add upstream's `dbghelploader` and `usp10loader`.
2. **`Core/Libraries/Source/debug/CMakeLists.txt`**
   - Keep GeneralsX `if(WIN32)` guard for `core_debug` and PCH to prevent non-Windows build failure. Inside `if(WIN32)`, adopt upstream's `Precompiled/BasePrecomp.h`.
3. **`Generals/Code/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt` & `GeneralsMD/...`**
   - Keep GeneralsX `if(WIN32)` guard on `target_precompile_headers` (critical for macOS/Linux build stability).
   - Update headers inside `if(WIN32)` to upstream's new paths (`Precompiled/BasePrecomp.h`, `Utility/STLUtils.h`).

---

### Section D: Filesystem & Platform Abstractions
1. **`Core/GameEngine/Include/Common/file.h`**
   - Keep GeneralsX `#include <stdio.h>` for `BUFSIZ` on macOS/Clang.
   - Accept removal of obsolete `#include "Lib/BaseType.h"`.
2. **`Core/GameEngine/Source/Common/System/LocalFile.cpp`**
   - Accept removal of obsolete `#include "Lib/BaseType.h"`.
3. **`Core/GameEngineDevice/Source/StdDevice/Common/StdLocalFileSystem.cpp`**
   - Upstream modernized the iteration loop using `for (; iter != std::filesystem::directory_iterator(); ++iter)`.
   - Reconcile with GeneralsX directory separator logic and case-insensitive `.big` extension matching for Linux/macOS.
4. **`Core/GameEngine/Source/Common/INI/INI.cpp`**
   - Reconcile upstream's new `LoadFlags` and `expectFileFound` checks with GeneralsX diagnostic stderr logging.
5. **`Core/GameEngine/Source/GameClient/GameText.cpp`**
   - Retain GeneralsX `FileInstance` parameter and array-supplied `parseStringFile` overlay, adopting upstream's `Bool ok = ...` typing.

---

### Section E: Networking & Online Multiplayer (NGMP Preservation)
1. **`Core/GameEngine/Include/GameNetwork/WOLBrowser/WebBrowser.h`**
   - Accept removal of obsolete `#include <Lib/BaseType.h>`.
2. **`Core/GameEngine/Source/GameNetwork/Network.cpp`**
   - Accept upstream `isMessageTypeWithinNetworkRange(msg->getType())`.
3. **`Core/Libraries/Source/WWVegas/WWDownload/Download.cpp`**
   - Adopt `Utility/stringex.h`, retain GeneralsX `#ifdef _WIN32` platform guards.
4. **`GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/ScoreScreen.cpp`**
   - Preserve `#if defined(SAGE_USE_NGMP)` block checking `TheNGMPGame`. Do NOT revert to GameSpy.
5. **`GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/WOLMapSelectMenu.cpp`**
   - Preserve NGMP map selection logic under `!defined(GENERALS_ONLINE_ALLOW_ALL_SETTINGS_FOR_STATS_MATCHES)`.

---

### Section F: Graphics, Shaders & Shadow Calculations (ARM64 / Determinism)
1. **`Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DShaderManager.cpp`**
   - Retain `Matrix4x4 curView` and `(D3DXMATRIX*)&curView` across shader stages to avoid MSVC/DirectX-only type coupling on macOS/Linux.
2. **`Core/GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWater.cpp`**
   - Retain null-pointer check `&& m_skyBox` to prevent crash when base game skybox assets are absent.
3. **`W3DProjectedShadow.cpp` (`Generals/` and `GeneralsMD/`)**
   - Retain GeneralsX bitmask comparisons `shadowInfo->m_type & SHADOW_DECAL` and dynamic projection flag support.
4. **`W3DVolumetricShadow.cpp` (`Generals/` and `GeneralsMD/`)**
   - Strictly retain GeneralsX `Short` edge-swapping logic; reject upstream `*(Int *)&silhouetteIndices[k]` which triggers undefined behavior and misaligned access faults on ARM64.
5. **`ddsfile.cpp` (`Generals/` and `GeneralsMD/`)**
   - Retain GeneralsX logical dimension calculations for compressed mipmaps and volume textures.

---

### Section G: Replays & Recorder Subsystem
1. **`GeneralsMD/Code/GameEngine/Include/Common/Recorder.h`**
   - Keep GeneralsX `#include "time_compat.h"` for non-Windows builds.
   - Adopt upstream forward declaration `enum GameMode CPP_11(: Int);`.
2. **`Generals/Code/GameEngine/Source/Common/Recorder.cpp` & `GeneralsMD/.../Recorder.cpp`**
   - Reconcile `playbackFile`: adopt upstream `rts::isMultiplayerGame(m_originalGameMode)`, but maintain GeneralsX `CRCInfo*` dynamic heap allocation (`NEW CRCInfo(...)`).

---

## 3. Step-by-Step Execution Sequence
1. Resolve Group A & B: Remove obsolete / moved files (`git rm` duplicates in `Generals/` and old locations).
2. Resolve Group C: CMake configuration files.
3. Resolve Group D: Platform, filesystem, and INI loader files.
4. Resolve Group E: Networking and UI menu files.
5. Resolve Group F: Graphics, shadows, and ddsfile across `Core/`, `Generals/`, and `GeneralsMD/`.
6. Resolve Group G: Replays and `Recorder` files.
7. Verify zero merge markers remain: `git grep -E '<<<<<<<|=======|>>>>>>>'`.
8. Configure and build on macOS (`cmake --preset macos-vulkan` or native build).
9. Smoke test binaries (`GeneralsXZH` and `GeneralsX`).
10. Finalize merge commit, push branch, and prepare report.
