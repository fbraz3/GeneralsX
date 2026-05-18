# Windows64 MinGW Functional Build Execution Strategy

**Status**: Phase 0 COMPLETED | Phase 1-9 IN PROGRESS
**Date**: 2026-05-18
**Primary Target**: GeneralsXZH first, Generals backport second
**Build Target**: `windows64-deploy`
**Toolchain**: MinGW-w64 x86_64 on Windows MSYS2

## Goal

Deliver a functional Windows x86_64 MinGW build that converges on the same open-source runtime stack used by Linux and macOS: SDL3 for windowing/input, DXVK for Direct3D 8 translation, OpenAL for audio, and FFmpeg for video.

## Execution Model

- This document is an implementation tracker, not a research backlog.
- A phase is only marked as completed when code, scripts, and validation evidence exist.
- Design-only notes are insufficient without executable changes and verification steps.

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
- Task reference: [PLAN-WINDOWS64_TASK_01_TOOLCHAIN_AND_PRESETS.md](PLAN-WINDOWS64_TASK_01_TOOLCHAIN_AND_PRESETS.md)

### ✅ Phase 2 - CMake Feature Gates [COMPLETED]
- Remove hard dependencies on 32-bit checks from the modern path (✅ DONE)
- Gate Miles and Bink behind legacy-only conditions (✅ DONE)
- Ensure the modern path is driven by `SAGE_USE_SDL3`, `SAGE_USE_OPENAL`, `RTS_BUILD_OPTION_FFMPEG`, and DXVK-specific policy
- Modified `cmake/miles.cmake` to gate Miles for legacy-only
- Modified `cmake/bink.cmake` to gate Bink for legacy-only
- Modern Windows64 path uses SDL3, DXVK, OpenAL, FFmpeg; Miles/Bink disabled
- Task reference: [PLAN-WINDOWS64_TASK_02_CMAKE_FEATURE_GATES.md](PLAN-WINDOWS64_TASK_02_CMAKE_FEATURE_GATES.md)

### ✅ Phase 3 - Entry Point and Engine Selection [COMPLETED]
- Keep platform entry files separated and controlled by build/platform rules (✅ DONE)
- Allow Windows64 modern builds to instantiate SDL3-based engine path from WinMain factory (✅ DONE)
- Keep any remaining legacy Win32 entrypoints isolated and optional
- Modified `WinMain.cpp` factory logic with Phase 3 documentation
- Feature flag `SAGE_USE_SDL3` controls which engine path is instantiated on Windows
- Task reference: [PLAN-WINDOWS64_TASK_03_ENTRYPOINT_AND_ENGINE_FACTORY.md](PLAN-WINDOWS64_TASK_03_ENTRYPOINT_AND_ENGINE_FACTORY.md)

### ✅ Phase 4 - DXVK Runtime on Windows [COMPLETED]
- Extend DXVK integration for Windows64 (✅ DONE)
- Define `d3d8.dll` bundling and loading strategy (✅ DONE)
- Validate graphics device path without changing Linux/macOS behavior (tracked under Phase 8 regression gates)
- DXVK runtime loading path uses `LoadLibrary("D3D8.DLL")` and resolves app-local DXVK runtime first
- CMake now stages DXVK runtime DLLs for Windows64 modern path (`d3d8.dll`, `dxgi.dll`, `d3d11.dll`)
- Task reference: [PLAN-WINDOWS64_TASK_04_DXVK_WINDOWS_RUNTIME.md](PLAN-WINDOWS64_TASK_04_DXVK_WINDOWS_RUNTIME.md)

### Phase 5 - OpenAL and FFmpeg on Windows [PENDING]
- Implement OpenAL runtime backend selection and startup path on Windows64
- Implement FFmpeg runtime path for Windows64 video playback
- Remove remaining modern-path runtime assumptions that require Miles or Bink
- Done criteria: `windows64-deploy` builds and reaches menu with OpenAL and FFmpeg paths active
- Task reference: [PLAN-WINDOWS64_TASK_05_OPENAL_FFMPEG_WINDOWS.md](PLAN-WINDOWS64_TASK_05_OPENAL_FFMPEG_WINDOWS.md)

### Phase 6 - Legacy Windows Cull [PENDING]
- Execute keep/isolate/remove actions for Win32 legacy pieces after audit
- Move unavoidable legacy code behind explicit modern-path guards
- Remove dead legacy-only code from modern path build graphs
- Done criteria: modern Windows64 build no longer links/depends on legacy-only paths unless explicitly flagged
- Task reference: [PLAN-WINDOWS64_TASK_06_LEGACY_WINDOWS_CULL.md](PLAN-WINDOWS64_TASK_06_LEGACY_WINDOWS_CULL.md)

### Phase 7 - Bundle and Runtime Validation [PENDING]
- Implement Windows build/deploy/run/bundle scripts under `scripts/build/windows/`
- Implement runtime artifact layout and copy rules for SDL3, DXVK, OpenAL, FFmpeg
- Run smoke launch validation with `-win -noshellmap`
- Done criteria: single-command bundle flow produces runnable Windows64 package
- Task reference: [PLAN-WINDOWS64_TASK_07_BUNDLE_AND_RUNTIME_VALIDATION.md](PLAN-WINDOWS64_TASK_07_BUNDLE_AND_RUNTIME_VALIDATION.md)

### Phase 8 - Cross-Platform Regression Gates [PENDING]
- Execute Linux and macOS smoke gates after each structural Windows change
- Execute Windows64 smoke validation and capture logs as evidence
- Add minimal deterministic replay check where feasible
- Done criteria: Linux/macOS/Windows64 smoke gates pass for the same change set
- Task reference: [PLAN-WINDOWS64_TASK_08_CI_AND_DOCS.md](PLAN-WINDOWS64_TASK_08_CI_AND_DOCS.md)

### Phase 9 - CI and Docs Completion [PENDING]
- Implement Windows64 MinGW CI workflow with configure/build/smoke stages
- Publish and verify canonical contributor docs for setup/build/troubleshooting
- Done criteria: CI green for Windows64 lane and docs usable by a clean environment

## Execution Summary

1. Create and validate the Windows64 MinGW toolchain and presets.
2. Replace legacy 32-bit and VC6 gates with feature-based modern gates.
3. Move Windows64 to the SDL3-based entry and engine path.
4. Define and implement DXVK runtime usage on Windows64.
5. Enable OpenAL and FFmpeg as the default modern Windows64 media stack.
6. Isolate and remove legacy Windows-only runtime dependencies from the modern path.
7. Add Windows build/deploy/run/bundle scripts.
8. Add cross-platform regression gates and CI.

## Acceptance Direction

- `cmake --preset windows64-deploy` configures on Windows MSYS2.
- `cmake --build build/windows64-deploy --target z_generals` produces a Windows x86_64 executable.
- The modern Windows64 build path does not require Visual Studio.
- Linux and macOS continue to configure and build after each structural change.