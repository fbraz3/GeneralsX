# TheSuperHackers Upstream Sync Plan (2026-03-27)

## Scope

- Branch: `thesuperhackers-sync-03-27-2026`
- Merge target: `thesuperhackers/main`
- Objective: absorb upstream stability/bugfix improvements while preserving GeneralsX cross-platform stack (SDL3 + DXVK + OpenAL + FFmpeg) and platform isolation.

## Merge Outcome Snapshot

- Upstream merge produced many auto-merged files and 2 explicit textual conflicts:
  - `Core/GameEngineDevice/Include/W3DDevice/GameClient/W3DView.h`
  - `Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h`
- Large touched surface area remains high risk even where no textual conflict occurred.

## Conflict Resolution Principles

1. Do not apply blanket keep-ours or keep-theirs decisions.
2. Keep upstream correctness improvements (for example, `override` correctness and API consistency) when they do not remove GeneralsX platform capabilities.
3. Preserve GeneralsX cross-platform behavior:
   - SDL3 input paths
  - FFmpeg video path where enabled
   - Existing DXVK/OpenAL/FFmpeg integration points
4. Keep platform-specific code isolated in device/platform layers; avoid gameplay logic leakage.
5. Prefer root-cause reconciliation over temporary stubs or feature disablement.

## Detailed Conflict Plan

### 1) W3D view interface conflict

- File: `Core/GameEngineDevice/Include/W3DDevice/GameClient/W3DView.h`
- Conflict nature:
  - Upstream adds `override` annotations and interface consistency updates.
  - Local branch includes extra camera API methods currently used by GeneralsX (`setCameraHeightAboveGroundLimitsToDefault`, `setZoomToMax`) plus custom comment on scripted camera reset behavior.
- Resolution decision:
  - Keep upstream `override` annotations.
  - Keep GeneralsX camera methods that are not present upstream but are part of local behavior and call paths.
  - Ensure no method removal from either side unless proven unused and intentionally deprecated.
- Validation checks:
  - Header compiles in both modern/macOS and Linux presets.
  - No missing implementation link errors for preserved methods.

### 2) W3D game client factory conflict

- File: `Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h`
- Conflict nature:
  - Upstream uses Bink-only `createVideoPlayer()` and Win32-only input factory in inline methods.
  - Local branch adds SDL3 keyboard/mouse creation for non-Windows and FFmpeg option for video player creation.
- Resolution decision:
  - Preserve upstream API hygiene (`override` and signature consistency).
  - Preserve GeneralsX conditional FFmpeg backend under `RTS_HAS_FFMPEG`.
  - Preserve non-Windows SDL3 input factory path and keep Win32 path unchanged for Windows.
- Validation checks:
  - macOS configure/build still sees SDL3 input path and compiles.
  - Linux configure/build path remains valid.

## High-Risk Areas Beyond Explicit Conflicts

1. Build system and presets (`CMakeLists.txt`, `CMakePresets.json`, `cmake/*`).
2. Platform abstraction layers touched by upstream (`Core/GameEngineDevice/*`, Win32 wrappers, file system abstractions).
3. Shared WWVegas and rendering/audio headers (large header churn may introduce subtle ABI/API mismatch).
4. Launch/runtime integration paths (`CommandLine.cpp`, game client/device init).
5. Video/audio backend selections (Bink vs FFmpeg and Miles/OpenAL intersections).

## Risk Mitigation

1. Resolve textual conflicts first with explicit hybrid merges.
2. Verify there are no remaining conflict markers repository-wide.
3. Validate configure/build sequentially:
   - First macOS configure + build flow.
   - Then Linux configure + build flow.
4. If any script regressions appear, patch only the minimal affected scripts while honoring script organization rules.
5. Capture unresolved risk list in final report for targeted post-merge testing.

## Execution Steps

1. Resolve `W3DView.h` with combined upstream overrides + GeneralsX camera methods.
2. Resolve `W3DGameClient.h` with combined upstream overrides + GeneralsX SDL3/FFmpeg behavior.
3. Stage resolutions and complete merge commit.
4. Run validation tasks in required sequence (macOS first, Linux second).
5. Commit any required post-merge fixes separately (if needed).
6. Push branch to origin.
7. Produce final sync report with decisions, risks, status, and test checklist.

## Execution Log

### Decision A: `W3DView.h`

- Applied hybrid merge.
- Kept upstream `override` additions for interface safety and compile-time validation.
- Kept GeneralsX-specific camera APIs not present upstream:
  - `setCameraHeightAboveGroundLimitsToDefault(Real heightScale = 1.0f)`
  - `setZoomToMax()`
- Result: no conflict markers; file staged as resolved.

### Decision B: `W3DGameClient.h`

- Applied hybrid merge.
- Kept upstream `override` signature for `createVideoPlayer()`.
- Preserved GeneralsX FFmpeg/Bink conditional backend selection under `RTS_HAS_FFMPEG`.
- Preserved existing SDL3 input factory behavior for non-Windows paths (outside the textual conflict hunk).
- Result: no conflict markers; file staged as resolved.

### Decision C: Unmerged State Check

- Verified no remaining unmerged entries (`UU/AA/DD/...`) after staging conflict resolutions.
- Next step is full configure/build validation in required sequence: macOS first, Linux second.
