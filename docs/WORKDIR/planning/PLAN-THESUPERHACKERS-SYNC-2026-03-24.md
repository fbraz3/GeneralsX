# TheSuperHackers Upstream Sync Plan (2026-03-24)

## Scope
- Source branch: `thesuperhackers/main`
- Target branch: `thesuperhackers-sync-03-24-2026`
- Goal: Integrate upstream fixes while preserving GeneralsX cross-platform architecture (SDL3 + DXVK + OpenAL + FFmpeg).

## Merge State Summary
The merge completed with 5 unresolved conflicts:
1. `Generals/Code/GameEngine/Source/GameClient/InGameUI.cpp`
2. `Generals/Code/GameEngine/Source/Common/RTS/Player.cpp`
3. `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
4. `GeneralsMD/Code/GameEngine/Source/GameClient/InGameUI.cpp`
5. `GeneralsMD/Code/GameEngine/Source/Common/RTS/Player.cpp`

## Critical Conflict Resolution Strategy

### A) Gameplay/UI camera initialization conflicts (`InGameUI.cpp` in Generals and ZH)
- Conflict shape:
  - GeneralsX side sets `setCameraHeightAboveGroundLimitsToDefault()`.
  - Upstream side sets `setDefaultView(0.0f, 0.0f, 1.0f)`.
- Risk:
  - Camera setup affects gameplay visibility and could cause aspect/FOV regressions.
  - Wrong choice can break existing cross-platform camera behavior already stabilized in GeneralsX.
- Resolution principle:
  - Preserve GeneralsX camera default-height behavior, then evaluate whether upstream default-view change is additive and safe.
- Decision rule:
  - Keep GeneralsX line as primary because it reflects established cross-platform tuning.
  - Do not introduce default-view override unless proven non-regressive for both game variants.

### B) Gameplay battle plan scalar inversion conflicts (`Player.cpp` in Generals and ZH)
- Conflict shape:
  - GeneralsX side switched `__max` to `std::max` and directly modifies incoming `bonus` scalars.
  - Upstream side inverts a local copy (`invertedBonus`) and avoids mutating original `bonus` values.
- Risk:
  - Mutating original bonus object can cause stacked battle-plan logic bugs over time.
  - Keeping `__max` harms portability under modern compilers.
- Resolution principle:
  - Merge semantics: use upstream safer copy-based inversion + keep modern portability (`std::max`).
- Decision rule:
  - Use `invertedBonus` local copy.
  - Use `std::max` (not `__max`) for cross-platform compatibility.

### C) Rendering present-params conflict (`dx8wrapper.cpp` in Generals)
- Conflict shape:
  - GeneralsX side forces `MultiSampleType = D3DMULTISAMPLE_NONE` and always `D3DSWAPEFFECT_DISCARD`.
  - Upstream side sets `SwapEffect` conditionally (`DISCARD` windowed, `FLIP` fullscreen).
- Risk:
  - Present parameters impact DX8/DXVK behavior and runtime stability.
  - Fullscreen `FLIP` may conflict with prior cross-platform tuning.
- Resolution principle:
  - Preserve known-stable GeneralsX behavior unless upstream change is required for correctness.
- Decision rule:
  - Keep explicit `MultiSampleType = D3DMULTISAMPLE_NONE`.
  - Keep always-discard swap effect as existing project policy.

## Special Constraints
- No blanket “ours/theirs” strategy.
- No platform-specific leakage into gameplay logic.
- No removal of SDL3/DXVK/OpenAL/FFmpeg paths.
- Keep deterministic gameplay-sensitive behavior unchanged unless fix is clearly safe.

## Risky Areas Requiring Extra Verification
1. Tactical view camera initialization path in both game variants.
2. Battle plan bonus add/remove cycles (stacking and reversion correctness).
3. DX8 present parameters in fullscreen/windowed paths, especially under DXVK.
4. Any upstream changes auto-merged under `Core/` and engine glue code.

## Step-by-Step Execution Plan
1. Resolve `InGameUI.cpp` conflicts in Generals and ZH with consistent camera policy.
2. Resolve `Player.cpp` conflicts in Generals and ZH with copy-based inversion + portable max.
3. Resolve `dx8wrapper.cpp` conflict preserving existing cross-platform rendering policy.
4. Validate no conflict markers remain and merge can be committed.
5. Validate macOS configure/build flow.
6. Validate Linux configure/build flow.
7. Commit merge.
8. Push branch and provide post-merge report.

## Execution Log (Major Decisions)
- [Done] `Generals/Code/GameEngine/Source/GameClient/InGameUI.cpp`
  - Kept `setCameraHeightAboveGroundLimitsToDefault()`.
  - Rejected upstream `setDefaultView(0.0f, 0.0f, 1.0f)` to avoid changing stabilized camera behavior without targeted runtime validation.

- [Done] `GeneralsMD/Code/GameEngine/Source/GameClient/InGameUI.cpp`
  - Kept `setCameraHeightAboveGroundLimitsToDefault()`.
  - Added merge note comment for cross-platform camera-limit preservation.

- [Done] `Generals/Code/GameEngine/Source/Common/RTS/Player.cpp`
  - Adopted upstream safer local-copy inversion model (`invertedBonus`) to avoid mutating source bonus data.
  - Replaced non-portable `__max` with `std::max` to preserve modern cross-platform portability.

- [Done] `GeneralsMD/Code/GameEngine/Source/Common/RTS/Player.cpp`
  - Applied same hybrid policy as Generals variant for behavior parity.

- [Done] `Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
  - Kept explicit `MultiSampleType = D3DMULTISAMPLE_NONE` and always-discard swap effect, matching established GeneralsX rendering policy.

- [In Progress] Post-resolution validation and merge finalization.
