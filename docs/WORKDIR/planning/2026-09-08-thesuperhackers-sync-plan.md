# Upstream Sync Plan: TheSuperHackers (2026-09-08)

## 1. Overview and Objectives
This document establishes the conflict resolution plan for merging `thesuperhackers/main` into `thesuperhackers-sync-09-08-2026`.
The upstream baseline contains commits up to `689a09ac4`, featuring:
- GUI centralization to `Core/GameEngine/` (moving `GameWindowManager`, `GameWindowTransitionsStyles`, `AnimateWindowManager`).
- WW3D2 `IRenderBackend` interface introduction.
- Particle system batching optimizations and lifetime bug fixes.
- Scorch buffer limit extensions and safety checks.
- AI update fixes (contact weapons not blocked by obstacles, assisted targeting initial frame).
- Production cancel/refund bug fixes.
- Audio volume improvements and removal of legacy `has3DSensitiveStreamsPlaying` hack.
- Bink volume calculation improvements and tool stubbing (`NullVideoPlayer`).
- Save game absolute path support and loading failure dialogs.

The goal is to incorporate these enhancements while preserving GeneralsX's cross-platform architecture (SDL3, DXVK, MiniAudio, OpenAL, FFmpeg), determinism requirements, and CI/CD infrastructure.

---

## 2. Conflicts and Resolution Strategies

### 2.1 CI/CD Infrastructure
**Files:**
- `.github/workflows/build-toolchain.yml` (deleted in HEAD, modified upstream)
- `.github/workflows/weekly-release.yml` (deleted in HEAD, modified upstream)
- `.github/workflows/ci.yml` (content conflict)

**Resolution:**
- Per prompt instructions: **Never replace CI/CD infrastructure with upstream versions.**
- Remove `.github/workflows/build-toolchain.yml` and `.github/workflows/weekly-release.yml` (`git rm`).
- Restore `.github/workflows/ci.yml` from HEAD (`git checkout --ours .github/workflows/ci.yml`).

---

### 2.2 Graphics Backend (`Core/Libraries/Source/WWVegas/WW3D2/ww3d.cpp`)
**Conflict:**
- In `WW3D::End_Render(bool flip_frame)`:
  - HEAD: calls `DX8Wrapper::Set_Transform_Dirty()` to avoid expensive per-frame cache invalidation (~33KB memset + texture unbinds).
  - Upstream: calls `Get_Render_Backend()->Invalidate_Cached_Render_States()`.

**Analysis:**
- Upstream introduced `IRenderBackend` which abstracts backend calls. In the upstream PR, they systematically changed `DX8Wrapper::X` to `Get_Render_Backend()->X`.
- Reverting to `Invalidate_Cached_Render_States()` re-introduces the performance regression that GeneralsX explicitly optimized in commit `801a61035`.
- `ww3d.cpp` still includes `dx8wrapper.h`.
- Resolving with `DX8Wrapper::Set_Transform_Dirty()` preserves GeneralsX's frame performance optimization while remaining fully compatible with the DX8 backend.

---

### 2.3 Windows Library CMake (`Core/Libraries/Source/WWVegas/WWLib/CMakeLists.txt`)
**Conflict:**
- Upstream added `registry.cpp/.h` and `Usp10Loader.cpp/.h` under `if(WIN32)`.

**Analysis:**
- These files are scoped strictly to `if(WIN32)` and provide necessary support for upstream Windows/VC6 builds without affecting Linux or macOS.
- Accept upstream changes inside `if(WIN32)`.

---

### 2.4 Save Game Diagnostics (`Generals` & `GeneralsMD`: `GameState.cpp`)
**Files:**
- `Generals/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp`

**Conflict:**
- HEAD outputs error diagnostics to `stderr` with `fprintf(stderr, ...)` and called `TheGameEngine->setQuitting(TRUE)`.
- Upstream replaced sudden exit with a user-facing dialog `showQueuedSaveGameLoadFailure()` and `DEBUG_LOG`.

**Analysis:**
- In release builds, `DEBUG_LOG` is compiled out, but console diagnostics are vital for CLI/testing runs as per `AGENTS.md`.
- Showing `showQueuedSaveGameLoadFailure()` is a better UX than terminating the engine abruptly.
- Reconcile both: output `std::fprintf(stderr, ...)` and call `showQueuedSaveGameLoadFailure()`.

---

### 2.5 Unified GUI (`Generals/Code/GameEngine/Source/GameClient/GUI/GameWindowTransitionsStyles.cpp`)
**Conflict:**
- Deleted upstream, modified in HEAD.

**Analysis:**
- Upstream unified `GameWindowTransitionsStyles.cpp` into `Core/GameEngine/Source/GameClient/GUI/GameWindowTransitionsStyles.cpp`.
- The unified version in `Core` already received all changes and was auto-merged.
- Delete the redundant file in `Generals/` via `git rm`.

---

### 2.6 WorldBuilder Tool Video Player (`Generals` & `GeneralsMD`: `WorldBuilder.cpp`)
**Files:**
- `Generals/Code/Tools/WorldBuilder/src/WorldBuilder.cpp`
- `GeneralsMD/Code/Tools/WorldBuilder/src/WorldBuilder.cpp`

**Conflict:**
- HEAD had `if (!TheAudio->isMusicAlreadyLoaded()) return FALSE;` and `VideoPlayer()`.
- Upstream replaced `VideoPlayer()` with `NullVideoPlayer()` since WorldBuilder does not require full video playback engines (preventing unnecessary Bink/FFmpeg dependencies).

**Analysis:**
- Combine both: keep the music check from HEAD, and use `NullVideoPlayer()` as introduced by upstream.

---

### 2.7 Documentation (`README.md`)
**Conflict:**
- Upstream inserted vcpkg CI caching documentation in place of GeneralsX project description and links.

**Analysis:**
- Keep GeneralsX documentation (HEAD).

---

### 2.8 CMake Build Configuration (`cmake/config-build.cmake`)
**Conflict:**
- Upstream removed or lacked GeneralsX options (`RTS_BUILD_OPTION_FFMPEG`, `RTS_BUILD_OPTION_DEEP_CRC`, `SAGE_USE_SDL3`, `SAGE_USE_OPENAL`, `SAGE_USE_MINIAUDIO`, `SAGE_UPDATE_CHECK`, `SAGE_USE_MOLTENVK`, `RTS_BUILD_OPTION_SAGE_PATCH`).

**Analysis:**
- Keep GeneralsX options from HEAD, which are critical for Linux and macOS builds.

---

### 2.9 Package Manifests (`vcpkg.json` and `vcpkg-lock.json`)
**Conflict:**
- Upstream added feature `"ffmpeg"` and updated baseline in `vcpkg.json`; deleted `vcpkg-lock.json`.

**Analysis:**
- In `vcpkg.json`, keep GeneralsX dependency declarations (`freetype`, `fontconfig`, `openal-soft`, `curl`, `stb`, `glm`, `gli`) and incorporate upstream's `"features": { "ffmpeg": ... }`.
- Retain `vcpkg-lock.json` from HEAD to preserve reproducible dependency versions for macOS ARM64 builds.

---

### 2.10 Audio Parity (Post-Merge Audit)
**Subsystems:**
- `MiniAudioManager` and `OpenALAudioManager`.

**Action Items:**
- Upstream eliminated `has3DSensitiveStreamsPlaying()` from `AudioManager` and `MilesAudioManager`.
- Remove `has3DSensitiveStreamsPlaying()` from `MiniAudioManager` and `OpenALAudioManager` to maintain strict parity.
- Add `TheVideoPlayer->setVolume(getVolume(AudioAffect_Speech));` inside volume change handlers in `MiniAudioManager` and `OpenALAudioManager` to replicate the Bink/video speech volume dispatch added to `MilesAudioManager`.

---

## 3. Verification & Execution Steps
1. Execute conflict resolutions across the 14 unmerged files.
2. Apply Audio Parity adjustments to MiniAudio and OpenAL devices.
3. Check working tree for any residual conflict markers (`rg "<<<<<<"`).
4. Run CMake configure on `macos-vulkan`: `cmake --preset macos-vulkan`.
5. Build target `z_generals` (or run `./scripts/build/macos/build-macos-zh.sh`).
6. Run smoke validation if feasible.
7. Record worklog entry in `docs/WORKLOG/2026-09-DIARY.md`.
8. Commit the merge and push branch `thesuperhackers-sync-09-08-2026` to origin.
