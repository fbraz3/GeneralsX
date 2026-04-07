# TheSuperHackers Upstream Sync Plan (2026-04-07)

## Scope

- Branch: `thesuperhackers-sync-04-07-2026`
- Merge target: `thesuperhackers/main`
- Objective: absorb upstream bugfix/stability changes while preserving GeneralsX cross-platform architecture (SDL3 + DXVK + OpenAL + FFmpeg) and platform isolation.

## Merge Snapshot

`git merge --no-ff thesuperhackers/main` completed with auto-merged changes plus explicit content conflicts in:

1. `Core/GameEngine/Source/Common/INI/INI.cpp`
2. `Core/GameEngine/Source/Common/System/Debug.cpp`
3. `Core/Libraries/Source/WWVegas/WWDownload/registry.cpp`
4. `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
5. `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`

Large non-conflicting touched surface is also high-risk (build scripts/CMake, game engine startup, W3D filesystem/device paths, network/game info).

## Conflict Resolution Strategy

### A) Build System and Runtime Integration

- Priority: preserve GeneralsX cross-platform configure/build workflows and not regress SDL3/DXVK/OpenAL/FFmpeg integration.
- Approach:
  - Keep upstream cleanups and correctness changes in shared engine/build files where they are backend-agnostic.
  - Reject upstream changes only when they remove or weaken cross-platform runtime support.
  - Re-check touched CMake and launch-path files for silent regressions even when no textual conflict happened.

### B) Platform Abstraction and Crash/Debug Paths

- Conflict focus: `Debug.cpp`, `registry.cpp`, `GameEngine.cpp`.
- Approach:
  - Keep Windows-native behavior where intended (`MessageBox`, Win32 registry APIs).
  - Preserve non-Windows fallback behavior required by Linux/macOS (stderr crash output, compat headers, platform guards).
  - Remove temporary debugging artifacts and duplicated/inconsistent blocks introduced during prior investigations.

### C) Core Engine Parsing / Determinism-Sensitive Behavior

- Conflict focus: `INI.cpp`.
- Approach:
  - Preserve deterministic file load ordering and upstream parser stability fixes.
  - Remove accidental duplicated loop structure and noisy temporary diagnostics that can hide logic flow.
  - Keep only diagnostics that are intentional and minimally invasive to runtime behavior.

### D) DX8 Wrapper and Device Layer Stability

- Conflict focus: `Generals/.../dx8wrapper.cpp`.
- Approach:
  - Keep matrix/path portability work needed by GeneralsX modern stack.
  - Keep upstream removals/cleanups for dead debug counters if not functionally required.
  - Ensure compatibility with both legacy and modern build variants.

## File-by-File Plan

### 1) `Core/GameEngine/Source/Common/INI/INI.cpp`

- Expected issue: conflict between temporary deep debug instrumentation and upstream clean deterministic two-pass load logic.
- Planned resolution:
  - Keep upstream logic structure for two-pass root/subdir loading.
  - Retain only essential GeneralsX behavior that impacts correctness.
  - Remove duplicated nested list creation and exception text that replaced valid logic.
- Validation:
  - Verify no conflict markers and parser compiles.
  - Ensure function still returns read count and throws only expected exceptions.

### 2) `Core/GameEngine/Source/Common/System/Debug.cpp`

- Expected issue: upstream simplified Unicode message box path vs local cross-platform guards.
- Planned resolution:
  - Keep `_WIN32` guarded MessageBox behavior from local branch for portability.
  - Keep upstream simplifications where safe, but do not remove Linux/macOS crash reporting fallback.
- Validation:
  - Confirm non-Windows builds do not call Win32 APIs.

### 3) `Core/Libraries/Source/WWVegas/WWDownload/registry.cpp`

- Expected issue: include order/header conflict between upstream and local compat support.
- Planned resolution:
  - Preserve Windows registry implementation under `_WIN32`.
  - Preserve non-Windows compatibility headers/stubs needed for successful compilation.
  - Normalize includes to avoid duplicate/conflicting declarations.
- Validation:
  - Confirm HKEY type resolves correctly on all target platforms.

### 4) `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

- Expected issue: local legacy debug counters + Unicode helper conflict with upstream cleanup.
- Planned resolution:
  - Keep only required cross-platform behavior (`TheSystemIsUnicode` guard and Linux-safe definition if still used).
  - Drop obsolete counters/statics if upstream removed and they are not used.
- Validation:
  - Ensure no unresolved references and no Win32-only symbol leaks on non-Windows.

### 5) `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`

- Expected issue: Windows-only system() / MessageBox Unicode detection conflict.
- Planned resolution:
  - Keep `_WIN32` guarding around Windows-only calls.
  - Preserve non-Windows no-op/fallback behavior for texture conversion and Unicode detection.
  - Keep upstream intended behavior for Windows unchanged.
- Validation:
  - Confirm macOS/Linux compilation path remains clean.

## Research and References to Consult During Resolution

- Upstream target behavior from conflict stages (`:2` and `:3` merge blobs).
- Local references:
  - `references/thesuperhackers-main/` for expected upstream semantics.
  - `references/fighter19-dxvk-port/` for Linux/platform guards and DX8 portability patterns.
  - `references/jmarshall-win64-modern/` for modernized compatibility patterns where relevant.

## High-Risk Subsystems and Mitigation

1. Runtime startup and crash handling:
- Risk: platform-specific calls leaking into non-Windows code path.
- Mitigation: strict `_WIN32` guards and compile validation on macOS then Linux.

2. INI and file loading:
- Risk: deterministic order regressions or malformed exception paths.
- Mitigation: preserve two-pass load semantics and avoid ad-hoc instrumentation logic.

3. Graphics wrapper:
- Risk: subtle DX8 wrapper regressions in mixed legacy/modern builds.
- Mitigation: keep functional portability changes, drop dead debug-only statics.

4. Auto-merged build/runtime files:
- Risk: silent break in CMake presets/scripts despite no textual conflict.
- Mitigation: run configure/build validations in required sequence (macOS full, then Linux full).

## Execution Sequence

1. Resolve each explicit conflict with hybrid/manual merge.
2. Verify no markers remain repository-wide.
3. Review key auto-merged files in build/runtime paths.
4. Validate macOS configure/build flow completely.
5. Validate Linux configure/build flow completely.
6. Commit merge result.
7. Push `thesuperhackers-sync-04-07-2026` to origin.
8. Produce final conflict-resolution and risk report.

## Execution Log

### Decision 1: `Core/GameEngine/Source/Common/INI/INI.cpp`

- Resolution applied: kept upstream deterministic two-pass directory load logic and removed local temporary debug-only nested loops/conflict leftovers.
- Reasoning: upstream version is structurally correct and avoids duplicated list creation/iteration that could mask parser issues.
- Cross-platform impact: no platform-specific behavior removed; this is parser/control-flow stabilization.

### Decision 2: `Core/GameEngine/Source/Common/System/Debug.cpp`

- Resolution applied: hybrid result preserving GeneralsX `_WIN32` crash dialog guard and non-Windows `stderr` fallback, while keeping upstream surrounding code.
- Reasoning: upstream-only `MessageBoxW` path is unsafe for Linux/macOS portability in this codebase.
- Cross-platform impact: Windows keeps native dialog behavior; Linux/macOS retain deterministic crash logging path.

### Decision 3: `Core/Libraries/Source/WWVegas/WWDownload/registry.cpp`

- Resolution applied: removed ad-hoc conflict-side include hacks and normalized to `Registry.h`, `<string>`, and `win.h` include pattern.
- Reasoning: `win.h` is the project compatibility layer and avoids brittle cross-directory include references.
- Cross-platform impact: preserves Windows registry API behavior and keeps compatibility typedefs on non-Windows builds.

### Decision 4: `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

- Resolution applied: retained local debug counter/statics block currently referenced by existing code, instead of upstream-dropping it blindly.
- Reasoning: symbol usage still exists (`number_of_DX8_calls++`), so dropping declarations would introduce compile regressions.
- Cross-platform impact: no backend downgrade; preserved current runtime behavior pending future deliberate cleanup.

### Decision 5: `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`

- Resolution applied: kept `_WIN32` guard around texture conversion `system(CONVERT_EXEC1)` and retained non-Windows `TheSystemIsUnicode` fallback declaration.
- Reasoning: upstream-only call path risks wrong runtime behavior on non-Windows where conversion tool path is not guaranteed.
- Cross-platform impact: Windows behavior unchanged; Linux/macOS path stays safe and buildable.

## Merge Refresh (2026-04-07, upstream head update)

After fetching newer commits from `thesuperhackers/main` (17 commits ahead of previous integration point), a second merge pass introduced additional conflicts and one delete/modify case.

### New conflict set in this pass

1. `.github/workflows/ci.yml`
2. `Core/GameEngine/Source/Common/INI/INI.cpp`
3. `Core/GameEngine/Source/Common/System/Debug.cpp`
4. `Core/Libraries/Source/WWVegas/WWDownload/registry.cpp`
5. `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
6. `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`
7. `GeneralsMD/Code/GameEngineDevice/Include/OpenALAudioManager.h` (modify/delete)

### Pass-2 resolution constraints

- Keep cross-platform build and runtime strategy as first-class priority.
- Accept upstream CI improvements where they do not remove macOS/Linux validation paths needed by GeneralsX.
- Do not accept upstream deletions that remove active OpenAL integration artifacts used by current GeneralsX audio backend.
- Re-validate conflict files after merge by running macOS build flows for both `GeneralsX` and `GeneralsXZH`.

### Pass-2 risk mitigation

- CI workflow conflict: inspect target matrix and script paths to avoid dropping platform jobs or introducing stale script paths.
- OpenAL header delete/modify conflict: keep file if still referenced by current source/CMake paths; otherwise refactor references in the same pass (no stubs).
- Engine/parser/debug conflicts: preserve deterministic logic and platform guards; remove temporary diagnostics.

### Pass-2 execution outcomes

1. `.github/workflows/ci.yml`
- Decision: kept GeneralsX workflow structure that calls `build-linux.yml` and `build-macos.yml` with `zh-should-build` / `base-should-build` outputs.
- Why: incoming upstream hunk referenced different outputs (`generals`, `shared`, `generalsmd`) and switched jobs to `build-toolchain.yml`, which does not match the local workflow contract in this branch and would risk CI false negatives.

2. `Core/GameEngine/Source/Common/INI/INI.cpp`
- Decision: kept Apple-safe floating-point parsing fallback (`strtof`) and preserved deterministic two-pass directory loading (root files first, subdirectories after).
- Why: this keeps previously validated macOS compatibility while retaining upstream deterministic ordering expectations.

3. `Core/GameEngine/Source/Common/System/Debug.cpp`
- Decision: kept `_WIN32`-guarded MessageBox behavior plus non-Windows `stderr` fatal fallback.
- Why: upstream-only unconditional `MessageBoxW` path is not valid for Linux/macOS runtime.

4. `Core/Libraries/Source/WWVegas/WWDownload/registry.cpp`
- Decision: kept `registryini.h` + `win.h` include set.
- Why: non-Windows path uses `RegistryIni::*` persistence and requires that compatibility include; dropping it would break Linux/macOS registry emulation.

5. `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
- Decision: kept the debug counter statics block currently referenced in file.
- Why: `number_of_DX8_calls` is incremented later in the translation unit; removing declarations would introduce compile errors.

6. `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`
- Decision: kept cross-platform `TheSystemIsUnicode` declaration with non-Windows fallback.
- Why: Windows behavior remains intact while non-Windows code path avoids Win32 dependency leakage.

7. `GeneralsMD/Code/GameEngineDevice/Include/OpenALAudioManager.h` (modify/delete)
- Decision: kept the header (ours).
- Why: it is still referenced by current GeneralsMD source/CMake and acts as an active compatibility forwarder to `OpenALAudioDevice/OpenALAudioManager.h`; accepting upstream deletion would break include paths.
