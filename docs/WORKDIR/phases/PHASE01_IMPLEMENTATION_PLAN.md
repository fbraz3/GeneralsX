# PHASE01: Implementation Plan — Linux Graphics (DXVK + SDL3)

**Status**: 🚧 In Progress (70% complete)
**Created**: 2026-02-07
**Last Updated**: 2026-02-08

## Progress Snapshot
✅ DXVK loader integration complete  
✅ SDL3 device layer implemented (~450 lines real code)  
✅ OpenAL audio manager implemented (~550 lines real code)  
✅ CMake presets and flags configured  
✅ Source files added to build system  
⚠️ Pending: Docker smoke tests, filesystem case handling

## Goal
Port the graphics and windowing subsystem to Linux using DXVK for DirectX→Vulkan translation and SDL3 for windowing/input, while preserving Windows builds and minimizing core changes.

## High-level Milestones

1. Prepare build environment and presets (CMake + Docker)
2. Integrate DXVK loader into `dx8wrapper` (runtime loader switch)
3. Add SDL3-based entry point and device implementations
4. Add platform selection macros and compile guards
5. Build + smoke tests in Docker (no automated test here — manual later)
6. Iterate fixes and prepare merge to integration branch

## Tasks (detailed)

### A. Prep & Branching
- Create branch `phase1/dxvk-sdl3` from main
- Update `docs/WORKDIR/phases/PHASE01_IMPLEMENTATION_PLAN.md` with progress
- Ensure dev diary updated before commits

### B. Build Presets & Docker (config only)
- [X] Add `linux64-deploy` to `CMakePresets.json` (if missing)
- [X] Add `SAGE_USE_SDL3=ON`, `SAGE_USE_OPENAL=ON` in linux preset cacheVariables
- [ ] Add `Dockerfile.builder` (see `phase0-build-system.md`) — do not run
- [X] Add VS Code tasks to `.vscode/tasks.json` for configure/build via Docker (no execution)

### C. Minimal DXVK Loader Change (low-risk)
- [X] Modify `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`:
  - Replace `LoadLibrary("D3D8.DLL")` with runtime switch:
    ```cpp
    #ifdef _WIN32
      D3D8Lib = LoadLibrary("D3D8.DLL");
    #else
      D3D8Lib = LoadLibrary("libdxvk_d3d8.so");
    #endif
    ```
- [X] Add `#ifdef` guards where appropriate; keep old code path for Windows unchanged
- [X] Add comment linking to fighter19 reference and rationale

### D. SDL3 Entry Point & Device Layer
- [X] Add `GeneralsMD/Code/Main/SDL3Main.cpp` (based on fighter19)
  - Set `setenv("DXVK_WSI_DRIVER","SDL3",1);`
  - Call `SDL_Vulkan_LoadLibrary(nullptr)` and `SDL_CreateWindow` with `SDL_WINDOW_VULKAN`
- [X] Add `GameEngineDevice/` files (no subdirectory created):
  - `SDL3GameEngine.{h,cpp}` (subclass `GameEngine`) — **~450 lines real implementation**
  - ⚠️ `SDL3Mouse.{h,cpp}`, `SDL3Keyboard.{h,cpp}` — Deferred, events wired directly in SDL3GameEngine
  - ⚠️ `SDL3OSDisplay.{h,cpp}` / `SDL3IMEManager` — Deferred to Phase 2+
- [X] Wire factory in `SDL3GameEngine::createAudioManager()` to choose OpenAL/Miles via CMake flags
- [X] Add `OpenALAudioManager.{h,cpp}` — **~550 lines real implementation with device/context lifecycle**

### E. CMake Integration
- [X] Conditional source lists in `GameEngineDevice/CMakeLists.txt`:
  - If `SAGE_USE_SDL3` add SDL3 device sources — **Session 6: Added SDL3GameEngine.{h,cpp}**
  - If `SAGE_USE_OPENAL` add OpenAL audio sources — **Session 6: Added OpenALAudioManager.{h,cpp}**
  - Ensure dxvk FetchContent in top-level CMake when `SAGE_USE_DX8` is OFF — ⚠️ TBD
- [ ] Add install rules to package DXVK native libraries with Linux distribution target — ⚠️ Phase 1.5
- [X] Added SDL3Main.cpp to `Main/CMakeLists.txt` — **Session 6**

### F. Filesystem & Case Sensitivity
- [ ] Implement VFS helper to perform case-insensitive lookups on Linux when requested
- [ ] Add unit checks when loading assets (log missing due to case)

### G. Smoke Tests & Validation (local, manual)
- [ ] Configure + build via Docker (developer runs later)
- [ ] Launch binary on Linux VM or host (developer) and verify:
  - Main menu renders
  - Load skirmish map
  - 10-minute stability test (no crash)
- [ ] Verify Windows VC6 and Win32 builds still compile

### H. Hardening & Follow-ups
- [ ] Capture DXVK logs and add helpful debug flags
- [ ] Profile performance for common scenes
- [ ] Iterate on render-state differences or shader fallbacks

## Acceptance Criteria (Phase 1)
- `linux64-deploy` preset exists and compiles (developer to run)
- `dx8wrapper` loads `libdxvk_d3d8.so` on non-Windows builds
- SDL3 windowing is used on Linux builds and maps input correctly
- Game renders main menu and can load a skirmish map (silent audio OK)
- Windows builds unmodified and compile

## Risk & Mitigation (short)
- DXVK incompatibilities: use fighter19 DXVK artifacts and test on multiple drivers
- Asset case issues: add case-insensitive lookup
- Audio still stubbed: Phase 2 will integrate OpenAL

## Deliverables for Phase 1
- Patch set for `dx8wrapper` (runtime loader switch)
- New `SDL3Main.cpp` + `SDL3Device/` implementation
- CMake presets and Dockerfile (configuration only)
- Documentation and dev-diary updates

## Timeline (suggested)
- Week 1: Presets + minimal dx8wrapper change + Dockerfile config
- Week 2: SDL3 device implementation + CMake wiring
- Week 3: Integration testing, fixes, documentation
- Week 4: Merge to integration branch, prepare Phase 2

## Notes
- Keep PRs small and focused: one PR for loader + build changes, one PR for SDL3 device, one PR for cleanup.
- Preserve Windows code paths at all times.
