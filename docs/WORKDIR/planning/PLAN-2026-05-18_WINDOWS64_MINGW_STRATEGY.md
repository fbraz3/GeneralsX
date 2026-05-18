# Windows64 MinGW Functional Build Strategy

**Status**: Phase 0 COMPLETED | Phase 1-9 IN PROGRESS
**Date**: 2026-05-18
**Primary Target**: GeneralsXZH first, Generals backport second
**Build Target**: `windows64-deploy`
**Toolchain**: MinGW-w64 x86_64 on Windows MSYS2

## Goal

Deliver a functional Windows x86_64 MinGW build that converges on the same open-source runtime stack used by Linux and macOS: SDL3 for windowing/input, DXVK for Direct3D 8 translation, OpenAL for audio, and FFmpeg for video.

## Constraints

- VC6 compatibility is not a goal for this workstream.
- Linux and macOS must remain buildable and must be treated as regression gates.
- Platform code must stay isolated in backend/device layers and CMake feature gates.
- Zero Hour is the primary target; Generals follows only when the change is backend-shared.

## Phase Status

### ✅ Phase 0 - Policy and Baseline [COMPLETED]
- Reclassified `mingw-w64-i686` as legacy/exploratory
- Established `windows64-deploy` as modern Windows path
- Defined SDL3, DXVK, OpenAL, FFmpeg as required components
- Toolchain x86_64 exists at `cmake/toolchains/mingw-w64-x86_64.cmake`
- Presets configured in `CMakePresets.json`

### ✅ Phase 1 - Toolchain and Presets [COMPLETED]
- Add MinGW-w64 x86_64 toolchain file (✅ DONE)
- Add `windows64-deploy` and `windows64-debug` presets (✅ DONE)
- Create setup script `scripts/env/setup-mingw-windows.ps1` (✅ DONE)
- Document Phase 1 deliverables (✅ DONE)

### ✅ Phase 2 - CMake Feature Gates [COMPLETED]
- Remove hard dependencies on 32-bit checks from the modern path (✅ DONE)
- Gate Miles and Bink behind legacy-only conditions (✅ DONE)
- Ensure the modern path is driven by `SAGE_USE_SDL3`, `SAGE_USE_OPENAL`, `RTS_BUILD_OPTION_FFMPEG`, and DXVK-specific policy
- Modified `cmake/miles.cmake` to gate Miles for legacy-only
- Modified `cmake/bink.cmake` to gate Bink for legacy-only
- Modern Windows64 path uses SDL3, DXVK, OpenAL, FFmpeg; Miles/Bink disabled

### ✅ Phase 3 - Entry Point and Engine Selection [COMPLETED]
- Keep platform entry files separated and controlled by build/platform rules (✅ DONE)
- Allow Windows64 modern builds to instantiate SDL3-based engine path from WinMain factory (✅ DONE)
- Keep any remaining legacy Win32 entrypoints isolated and optional
- Modified `WinMain.cpp` factory logic with Phase 3 documentation
- Feature flag `SAGE_USE_SDL3` controls which engine path is instantiated on Windows

### ✅ Phase 4 - DXVK Runtime on Windows [COMPLETED]
- Extend DXVK integration for Windows64 (✅ DONE)
- Define `d3d8.dll` bundling and loading strategy (✅ DONE)
- Validate graphics device path without changing Linux/macOS behavior (tracked under Phase 8 regression gates)
- DXVK runtime loading path uses `LoadLibrary("D3D8.DLL")` and resolves app-local DXVK runtime first
- CMake now stages DXVK runtime DLLs for Windows64 modern path (`d3d8.dll`, `dxgi.dll`, `d3d11.dll`)

### Phase 5 - OpenAL and FFmpeg on Windows [PENDING]
- Promote OpenAL as functional Windows64 audio backend
- Promote FFmpeg as functional Windows64 video backend

### Phase 6 - Legacy Windows Cull [PENDING]
- Audit Win32 display/input code
- Classify legacy pieces as keep, isolate, or remove

### Phase 7 - Bundle and Runtime Validation [PENDING]
- Create Windows build/deploy/run/bundle scripts
- Define runtime artifact layout

### Phase 8 - Cross-Platform Regression Gates [PENDING]
- Re-run Linux/macOS smoke tests
- Add Windows64 smoke validation

### Phase 9 - CI and Docs Completion [PENDING]
- Add Windows64 MinGW CI pipeline
- Publish canonical setup documentation

## Task Summary

1. Create and validate the Windows64 MinGW toolchain and presets.
2. Replace legacy 32-bit and VC6 gates with feature-based modern gates.
3. Move Windows64 to the SDL3-based entry and engine path.
4. Define and implement DXVK runtime usage on Windows64.
5. Enable OpenAL and FFmpeg as the default modern Windows64 media stack.
6. Isolate or remove legacy Windows-only runtime dependencies.
7. Add Windows build/deploy/run/bundle scripts.
8. Add cross-platform regression gates and CI.

## Child Task Documents

Notes:
- Phase 0 is a completed baseline phase and does not have a dedicated child task file.
- Cross-platform regression gate coverage is split across Task 07 (smoke validation) and Task 08 (CI/docs gates).

- [PLAN-WINDOWS64_TASK_01_TOOLCHAIN_AND_PRESETS.md](PLAN-WINDOWS64_TASK_01_TOOLCHAIN_AND_PRESETS.md)
- [PLAN-WINDOWS64_TASK_02_CMAKE_FEATURE_GATES.md](PLAN-WINDOWS64_TASK_02_CMAKE_FEATURE_GATES.md)
- [PLAN-WINDOWS64_TASK_03_ENTRYPOINT_AND_ENGINE_FACTORY.md](PLAN-WINDOWS64_TASK_03_ENTRYPOINT_AND_ENGINE_FACTORY.md)
- [PLAN-WINDOWS64_TASK_04_DXVK_WINDOWS_RUNTIME.md](PLAN-WINDOWS64_TASK_04_DXVK_WINDOWS_RUNTIME.md)
- [PLAN-WINDOWS64_TASK_05_OPENAL_FFMPEG_WINDOWS.md](PLAN-WINDOWS64_TASK_05_OPENAL_FFMPEG_WINDOWS.md)
- [PLAN-WINDOWS64_TASK_06_LEGACY_WINDOWS_CULL.md](PLAN-WINDOWS64_TASK_06_LEGACY_WINDOWS_CULL.md)
- [PLAN-WINDOWS64_TASK_07_BUNDLE_AND_RUNTIME_VALIDATION.md](PLAN-WINDOWS64_TASK_07_BUNDLE_AND_RUNTIME_VALIDATION.md)
- [PLAN-WINDOWS64_TASK_08_CI_AND_DOCS.md](PLAN-WINDOWS64_TASK_08_CI_AND_DOCS.md)

## Acceptance Direction

- `cmake --preset windows64-deploy` configures on Windows MSYS2.
- `cmake --build build/windows64-deploy --target z_generals` produces a Windows x86_64 executable.
- The modern Windows64 build path does not require Visual Studio.
- Linux and macOS continue to configure and build after each structural change.