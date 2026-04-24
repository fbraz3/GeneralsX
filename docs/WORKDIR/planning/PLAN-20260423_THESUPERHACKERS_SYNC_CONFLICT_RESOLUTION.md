# PLAN-20260423: TheSuperHackers Upstream Sync Conflict Resolution

## Scope

Merge `thesuperhackers/main` into branch `thesuperhackers-sync-04-23-2026` while preserving GeneralsX cross-platform architecture:
- SDL3 platform layer
- DXVK graphics path
- OpenAL audio path
- FFmpeg integration trajectory
- Platform isolation from gameplay logic

## Why This Merge Is Risky

The upstream includes `unify` commits that move shared files from game-specific trees (`Generals/`, `GeneralsMD/`) into `Core/`. GeneralsX also diverged in cross-platform architecture. This means conflicts can hide semantic regressions even when files compile.

## Current Unmerged Conflicts (Must Resolve Manually)

1. `Core/Libraries/Source/WWVegas/WW3D2/render2dsentence.cpp`
- Type: content conflict
- Risk: GUI/text rendering and recent 2D render performance updates.
- Resolution intent: preserve upstream bug/perf improvements unless they conflict with DXVK path; keep GeneralsX platform-safe adjustments.

2. `Generals/Code/GameEngine/Include/GameClient/Display.h`
- Type: modify/delete conflict (deleted upstream due unification; modified in GeneralsX)
- Risk: stale duplicate headers can diverge from unified `Core` behavior.
- Resolution intent: follow upstream structure by accepting move to `Core` and deleting game-local duplicate unless GeneralsX-only symbols are missing in `Core`.

3. `Generals/Code/GameEngine/Source/GameClient/System/Image.cpp`
- Type: modify/delete conflict (deleted upstream due unification; modified in GeneralsX)
- Risk: duplicate runtime logic and diverged rendering/image handling between game tree and `Core`.
- Resolution intent: follow unified `Core` path and delete old game-local source after verifying needed behavior exists in `Core` version.

4. `Generals/Code/GameEngine/Source/Common/GlobalData.cpp`
- Type: content conflict
- Risk: gameplay/runtime options, determinism-sensitive settings, renderer and input options binding.
- Resolution intent: merge semantically; keep upstream fixes while preserving GeneralsX cross-platform options and guardrails.

5. `scripts/tooling/cpp/maintenance/remove_return.py`
- Type: file location conflict (upstream added in old path while GeneralsX renamed tooling tree)
- Risk: broken script path in existing workflows.
- Resolution intent: keep GeneralsX script organization (`scripts/tooling/cpp/maintenance/`) and preserve upstream script content.

## High-Risk Non-Conflict Areas To Audit Before Validation

Even if auto-merged, these touched areas require targeted checks:
- `Core/GameEngine/Source/Common/INI/INI.cpp` and related INI API headers
- `Core/GameEngine/Source/GameClient/*` display/system classes moved from game trees
- `Core/GameEngineDevice/*` including W3D view and Miles/OpenAL interaction points
- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` and rendering support files
- `CMakeLists.txt`, `Core/*/CMakeLists.txt`, and platform scripts under `scripts/`

## Conflict Resolution Procedure

### A. Per-File 3-Way Analysis

For each unmerged file:
1. Inspect ours/theirs/base versions (`git show :1:`, `:2:`, `:3:`).
2. Identify whether conflict is architectural (move/unify), behavioral bugfix, or style/refactor.
3. Choose resolution at function granularity (no blanket ours/theirs decisions).
4. Ensure no platform-specific code leaks into gameplay logic.

### B. Unify Move Conflicts (Display/Image)

1. Confirm unified replacements in `Core/GameEngine/...` include required symbols and behavior.
2. Preserve upstream move by removing stale files in `Generals/...` when safe.
3. Verify include paths/build references are consistent post-resolution.

### C. GlobalData Merge

1. Preserve upstream stability/bugfix logic.
2. Preserve GeneralsX cross-platform runtime configuration behavior.
3. Re-check determinism-related flags and defaults.
4. Verify no regressions in options parsing/loading flow.

### D. render2dsentence Merge

1. Prioritize upstream 2D rendering fixes/perf changes.
2. Keep compatibility with DX8 wrapper abstractions used by DXVK path.
3. Reject any change that assumes platform-native APIs in gameplay/client logic.

### E. Script Relocation Conflict

1. Keep script in organized path under `scripts/tooling/cpp/maintenance/`.
2. Ensure executable mode and references remain valid.

## Validation Plan (Sequential, Not Parallel)

1. Conflict marker sweep:
- Verify zero `<<<<<<<`, `=======`, `>>>>>>>` markers in repo.

2. macOS flow first:
- Configure (`[macOS] Configure`)
- Build ZH (`[macOS] Build GeneralsXZH`)
- Build Generals (`[macOS] Build GeneralsX`)

3. Linux flow second:
- Configure/build using docker script (`[Linux] Pipeline: Docker Build ZH (Full)`)
- Build Generals path if touched by merge scripts/config.

4. Runtime smoke:
- macOS run ZH (`[macOS] Run GeneralsXZH`) and Generals equivalent if available
- Linux smoke (`[Linux] Smoke Test GeneralsXZH`) and Generals run/smoke where applicable
- Success criterion: enter and exit main loop cleanly.

5. Artifact cleanup:
- Remove local runtime/build artifacts and caches accidentally introduced by runs.

## Decision Log Format (to be filled during execution)

For each major conflict decision:
- File:
- Decision:
- Reasoning:
- Risk:
- Mitigation/verification:

## Decision Log (Execution)

### 1) Core/Libraries/Source/WWVegas/WW3D2/render2dsentence.cpp
- Decision: Removed the temporary conflict block and kept the existing single implementation of `FontCharsClass::Update_Current_Buffer` that is already used by both Windows and FreeType paths.
- Reasoning: Upstream introduced the same function body at an earlier location due code movement; in GeneralsX this method already exists in a cross-platform section used by Windows and Linux text rendering.
- Risk: Keeping both versions would cause duplicate definition/build break and uncertain behavior.
- Mitigation/verification: Verified conflict markers are gone and retained the platform-independent implementation tagged for cross-platform font pipeline.

### 2) Generals/Code/GameEngine/Source/Common/GlobalData.cpp
- Decision: Kept both include sets by combining them: retained `#ifndef _WIN32` `<filesystem>` and added upstream `ww3d.h` and `texturefilter.h` includes.
- Reasoning: The conflict was include-only. GeneralsX needs filesystem on non-Windows user data paths; upstream added renderer-related headers required by new options/features.
- Risk: Missing include side could break platform path code or option/render integrations.
- Mitigation/verification: Resolved with additive include strategy, preserving both behavioral intents.

### 3) Generals/Code/GameEngine/Include/GameClient/Display.h (modify/delete)
- Decision: Accepted upstream unification structure and removed the game-local duplicate file from `Generals/`.
- Reasoning: Upstream moved display/client abstractions into `Core`, and the `Core` header contains newer batching-related interface updates not present in the stale duplicate.
- Risk: Keeping duplicate headers could reintroduce divergence between products.
- Mitigation/verification: Confirmed unified `Core` header contains relevant behavior and updated index resolution by deleting obsolete game-local copy.

### 4) Generals/Code/GameEngine/Source/GameClient/System/Image.cpp (modify/delete)
- Decision: Accepted upstream unification structure and removed the game-local duplicate source from `Generals/`.
- Reasoning: The file is now unified in `Core/GameEngine/Source/GameClient/System/Image.cpp`; retaining old copy would duplicate runtime image parsing logic.
- Risk: Divergent mapped-image loading behavior between products.
- Mitigation/verification: Confirmed Core version includes GeneralsX path separator fixes and staged deletion of obsolete game-local source.

### 5) scripts/tooling/cpp/maintenance/remove_return.py (file location)
- Decision: Kept file in GeneralsX organized tooling path and adjusted script root traversal after relocation.
- Reasoning: Upstream added script in old scripts layout, while GeneralsX reorganized script folders. Original relative root calculation no longer pointed to repository root after move.
- Risk: Script would silently scan wrong directories or fail to find project trees.
- Mitigation/verification: Updated `root_dir` traversal to reach repository root from `scripts/tooling/cpp/maintenance`.

## Special Constraint For This Sync

Because upstream has several `unify` commits, every move/delete conflict from `Generals/` or `GeneralsMD/` to `Core/` must be resolved with structure convergence first, then behavior validation. No blind keep-ours/keep-theirs allowed.
