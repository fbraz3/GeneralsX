# PLAN: TheSuperHackers Upstream Sync (2026-05-07)

## Scope

Sync `thesuperhackers/main` into branch `thesuperhackers-sync-05-07-2026` while preserving GeneralsX cross-platform architecture (SDL3 + DXVK + OpenAL + FFmpeg) and accepting upstream bugfix/stability improvements where compatible.

## Current Merge State

`git merge --no-ff thesuperhackers/main` is in progress.

Unresolved conflicts detected:
1. `Generals/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetListBox.cpp` (modify/delete)
2. `Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/GUI/Gadget/W3DProgressBar.cpp` (modify/delete)

Large upstream movement from `Generals*` and `GeneralsMD*` into shared `Core/` has already been applied by merge machinery (many rename/delete transitions).

## Critical Conflict Inventory and Resolution Strategy

### A) Unify/File-Move Conflicts (Highest Priority)

#### Key conflicts
- Legacy `Generals` gadget implementation files were deleted upstream and replaced by unified `Core` equivalents.
- Local branch had modifications in deleted `Generals` files, triggering modify/delete conflicts.

#### Resolution reasoning
- Upstream unification into `Core/` is strategic and should be preserved.
- For each conflict, verify whether local modifications contain cross-platform critical logic absent from the new `Core` location.
- If local change is non-functional or already represented in `Core`, accept deletion of old path and keep unified `Core` file.
- If local change is functional and missing in `Core`, port exact functional delta into `Core` and then delete old `Generals` copy.

#### Concrete steps
1. Diff each unresolved file vs corresponding unified `Core` file.
2. Identify semantic deltas (not comments/header text only).
3. Port missing semantic deltas to `Core` only when required.
4. Resolve conflict by removing deprecated `Generals` path file.
5. Re-check CMake references for stale source paths.

### B) Build System and Presets

#### Risk
- Upstream unification can accidentally rewire include paths or source lists in ways that break SDL3/DXVK/OpenAL integration.

#### Strategy
- Inspect merged CMake changes in:
  - `Core/GameEngine/CMakeLists.txt`
  - `Core/GameEngineDevice/CMakeLists.txt`
  - `Generals/Code/GameEngine/CMakeLists.txt`
  - `GeneralsMD/Code/GameEngine/CMakeLists.txt`
  - `Generals/Code/GameEngineDevice/CMakeLists.txt`
  - `GeneralsMD/Code/GameEngineDevice/CMakeLists.txt`
- Ensure moved files are consistently referenced from `Core` and removed from game-specific source lists.
- Preserve existing GeneralsX platform settings (especially macOS Vulkan path and DXVK/OpenAL/FFmpeg wiring).

### C) Platform Abstraction and Rendering/Audio Wiring

#### Risk
- Upstream compatibility-oriented changes may regress cross-platform stack if they reintroduce assumptions tied to legacy Windows behavior.

#### Strategy
- Review merged deltas around:
  - Core platform/file-system headers and implementations
  - networking/web browser wrappers touched by merge
  - W3D rendering files touched by merge
- Keep cross-platform architecture precedence; no platform leakage into gameplay logic.

### D) INI/Load-Order and Determinism Risk

#### Risk
- Upstream parser/performance changes can alter behavior on macOS targets or replay-sensitive paths.

#### Strategy
- Since this merge surfaced mostly structural unification and not parser conflicts, verify compile-time integrity now.
- Flag parser/load-order as post-merge risk area for focused manual gameplay validation.

## Risk Mitigation Rules During Resolution

1. No blanket "ours" or "theirs" for broad trees.
2. No temporary stubs, disabled code paths, or bypasses.
3. Preserve `Core/` unification when technically valid.
4. Preserve GeneralsX cross-platform stack and isolation boundaries.
5. Validate macOS configure/build only (per user request).
6. Do not run Linux game runtime checks (explicitly deferred by user).

## Validation Plan (User-Requested Scope)

1. Ensure no unresolved merge files remain.
2. Verify no merge markers exist anywhere.
3. Validate macOS configure/build flow via existing tasks/scripts.
4. Do not execute Linux runtime or Linux interactive run.
5. Remove generated caches/artifacts before commit.

## Decision Log Template (to be filled while resolving)

- Decision ID:
- File(s):
- Conflict type:
- Options considered:
- Selected resolution:
- Why this preserves upstream value:
- Why this preserves GeneralsX cross-platform behavior:
- Follow-up risk:
