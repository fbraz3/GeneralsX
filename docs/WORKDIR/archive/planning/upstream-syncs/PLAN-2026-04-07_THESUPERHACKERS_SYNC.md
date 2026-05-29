# TheSuperHackers Upstream Sync Plan - 2026-04-07

## Scope

Merge `thesuperhackers/main` into `thesuperhackers-sync-04-07-2026` while preserving the established GeneralsX cross-platform stack:

- SDL3 for windowing/input
- DXVK for graphics
- OpenAL for audio
- FFmpeg/Bink replacement path where applicable
- Existing macOS/Linux build, deploy, and runtime flows

This plan is based on the live merge result produced on 2026-04-07 and targets the currently unresolved conflicts.

## Current Conflict Set

1. `.github/workflows/ci.yml`
2. `Core/GameEngine/Source/Common/INI/INI.cpp`
3. `Core/GameEngine/Source/Common/System/Debug.cpp`
4. `Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DView.cpp`
5. `Core/Libraries/Source/WWVegas/WWDownload/registry.cpp`
6. `Generals/Code/GameEngine/Include/Common/GlobalData.h`
7. `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
8. `GeneralsMD/Code/GameEngine/Include/Common/GlobalData.h`
9. `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`

## Resolution Principles

1. Preserve GeneralsX cross-platform behavior first when upstream changes would downgrade SDL3, DXVK, OpenAL, FFmpeg, or non-Windows runtime support.
2. Import upstream bug fixes, safety fixes, and behavioral corrections when they do not conflict with the platform strategy.
3. Treat INI parsing, registry/runtime path handling, and launch/game initialization as high-risk areas because small regressions there can break build output, startup, save/load, or deterministic data loading.
4. Avoid blanket `ours` or `theirs` decisions even within the same subsystem.
5. Prefer upstream structure where files were unified into `Core/`, but reconcile that structure with existing GeneralsX platform layers instead of rolling back cross-platform abstractions.

## Subsystem Plan

### 1. Build and CI

Files:

- `.github/workflows/ci.yml`

Conflict shape:

- GeneralsX side adds Linux/macOS cross-platform workflows and change-detection gating for `Generals` and `GeneralsMD`.
- Upstream side routes jobs through Windows-oriented toolchain matrix workflows.

Resolution:

- Keep GeneralsX Linux/macOS workflow wiring intact.
- Inspect whether any upstream CI improvements are portable and can be adopted without dropping the current macOS/Linux validation coverage.
- Reject any merge result that silently replaces cross-platform jobs with Windows-only toolchain jobs.

Risk mitigation:

- After conflict resolution, validate the YAML for syntax and ensure existing reusable workflow paths still exist.

### 2. INI Parsing and Load Order

Files:

- `Core/GameEngine/Source/Common/INI/INI.cpp`

Conflict shape:

- Upstream simplified directory loading logic and recently landed INI performance / cleanup changes.
- GeneralsX side carries temporary diagnostics and local changes from prior debugging sessions.

Resolution:

- Keep the effective upstream load-order logic unless it would break cross-platform path handling.
- Remove or reduce transient debug-only instrumentation if it duplicates logic or changes control flow.
- Re-check surrounding non-conflicted upstream INI commits after the merge to ensure no performance or parsing behavior regressions were introduced indirectly.
- Be extra careful with directory traversal, sort order, path separators, and exception behavior.

Specific analysis steps:

1. Compare the conflicted blocks against the upstream intent from recent INI commits.
2. Confirm that root-directory files still load before subdirectory files.
3. Confirm that both `\\` and `/` separators remain accepted where needed for cross-platform behavior.
4. Confirm no debug instrumentation accidentally duplicated the directory enumeration loop.

Risk mitigation:

- Run configure/build validation after resolution.
- Review adjacent INI-related hunks in the merge result, not only the conflicted sections.

### 3. Runtime Error Reporting and Platform Isolation

Files:

- `Core/GameEngine/Source/Common/System/Debug.cpp`
- `Core/Libraries/Source/WWVegas/WWDownload/registry.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`

Conflict shape:

- Upstream assumes Windows-native dialogs/registry behavior in some paths.
- GeneralsX side adds cross-platform alternatives for registry and fatal-error reporting.

Resolution:

- Preserve Windows behavior on Windows.
- Preserve non-Windows behavior for Linux/macOS without reintroducing hard Win32 dependencies into shared runtime paths.
- Keep the registry.ini backend and compatibility abstractions required for mod/runtime support.
- Merge upstream logic only if it is platform-neutral or can be wrapped safely in platform guards.

Specific analysis steps:

1. Verify includes and typedef availability remain valid on all supported platforms.
2. Check that non-Windows fatal-error handling still emits a usable diagnostic path.
3. Confirm `GameEngine.cpp` conflict resolution does not regress GeneralsX user-data/runtime path behavior.

Risk mitigation:

- Validate macOS configure/build first because it is more sensitive to leaked Win32 assumptions.

### 4. Camera, Gameplay-Facing View Behavior, and Data Definitions

Files:

- `Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DView.cpp`
- `Generals/Code/GameEngine/Include/Common/GlobalData.h`
- `GeneralsMD/Code/GameEngine/Include/Common/GlobalData.h`

Conflict shape:

- Upstream landed camera fixes and retail-scripted-camera preservation changes.
- GeneralsX carries camera height notes and aspect-ratio-sensitive behavior.

Resolution:

- Preserve the actual upstream camera fix behavior.
- Retain GeneralsX explanatory comments only if they do not interfere with preprocessor structure.
- Ensure both Generals and Zero Hour headers remain consistent.

Specific analysis steps:

1. Confirm preprocessor guards stay balanced.
2. Confirm `setZoomToMax()` reflects the intended upstream behavior.
3. Review related non-conflicted camera code for follow-up adjustments after the merge.

Risk mitigation:

- Treat this area as gameplay-visible and include it in the manual test checklist.

### 5. Renderer / DX8 Wrapper

Files:

- `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

Conflict shape:

- Upstream removed legacy frame-stat globals in favor of cleaner statistics handling.
- GeneralsX carries DXVK-related modernization and Windows-guarded behavior.

Resolution:

- Prefer the upstream cleanup when it is dead or redundant legacy state.
- Preserve any GeneralsX DXVK compatibility work and any non-Windows-safe definitions.

Specific analysis steps:

1. Check whether the removed variables are still referenced elsewhere.
2. If unused, accept the upstream cleanup.
3. Verify no local DXVK path depends on those globals.

### 6. Post-Conflict Safety Sweep

After all textual conflicts are resolved:

1. Inspect `git diff --check` for whitespace or marker mistakes.
2. Inspect staged merge result for non-conflict upstream hunks touching INI, runtime paths, registry, SDL3, DXVK, OpenAL, FFmpeg, and scripts.
3. Run validation sequentially:
   - macOS configure/build flow first
   - Linux configure/build flow second
4. If validation fails, fix only merge-induced regressions.

## Expected Risk Areas After Merge

- INI parsing correctness and load order
- Runtime path and registry abstraction behavior
- Camera zoom behavior and scripted camera compatibility
- Shared build/CI wiring that may hide cross-platform regressions
- Any non-conflicted upstream changes adjacent to platform abstraction code

## Decision Log Format

For each major conflict resolution, capture:

- File/subsystem
- What each side changed
- Final resolution
- Why the rejected side was not kept as-is
- Validation impact or follow-up checks required