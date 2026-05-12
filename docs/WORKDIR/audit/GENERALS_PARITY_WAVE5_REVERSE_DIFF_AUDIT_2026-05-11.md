# Generals Parity Wave 5 Audit: Reverse-Diff Classification

Date: 2026-05-11
Scope: Reverse scan of Generals-only annotated files not mirrored in Zero Hour (Wave 5).
Status: Completed

## Objective

Classify each Generals-only customization candidate as either:

- `Needs Mirror`: low-risk, relevant parity adaptation that should be mirrored.
- `Intentional`: divergence is expansion-specific, high-risk, or annotation-only with no behavior gap.

## Candidate Inventory and Decisions

| File | Subsystem | Decision | Risk | Rationale |
|---|---|---|---|---|
| `Code/GameEngine/Include/Common/GameLOD.h` | LOD API surface | Intentional | Low | Reverse-diff is annotation/comment-level only; no missing behavior identified in ZH header API. |
| `Code/GameEngine/Source/Common/GameLOD.cpp` | Non-Windows hardware fallback | Needs Mirror (ported) | Low | Generals guarded fallback defaults (`RAM/CPU/Freq`) only when detection fails; ZH previously overwrote values unconditionally on non-Windows. Mirrored conditional fallback to preserve detected values when available. |
| `Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptActions.cpp` | Script action runtime | Intentional | High | Large structural divergence tied to Zero Hour gameplay/script surface. Unsafe for blind mirror without feature-scoped design work. |
| `Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DScene.cpp` | Scene/render integration | Intentional | High | Broad divergence with expansion/platform history; prior parity work already identified this as non-actionable reverse-diff for Wave scope. |
| `Code/Libraries/Source/WWVegas/WW3D2/part_ldr.h` | Emitter serialization header | Intentional | Low | Reverse difference is annotation-level after Wave 4 serializer parity port; no missing ZH functionality in this header from Generals-only custom notes. |
| `Code/Libraries/Source/WWVegas/WW3D2/sortingrenderer.cpp` | Sorting renderer state refs | Intentional | Low | Generals-only annotation references unified `RenderStateStruct` cleanup; ZH already uses `vertex_buffers[]` in `Release_Refs` and related code paths. Behavior is aligned. |

## Implemented Mirror Change (Wave 5)

- Mirrored conditional non-Windows fallback logic into:
  - `GeneralsMD/Code/GameEngine/Source/Common/GameLOD.cpp`
- Applied rule:
  - Set fallback RAM/CPU/Freq defaults only when detected values are unknown/zero.

## Validation

1. Static diagnostics on edited file reported no errors:
   - `GeneralsMD/Code/GameEngine/Source/Common/GameLOD.cpp`
2. Platform build task executed for ZH (`[Platform] Build GeneralsXZH`) and produced compiler warnings consistent with existing baseline warning profile; no new Wave 5 file-level diagnostics were reported.

## Conclusion

Wave 5 reverse-diff classification is complete for the current candidate set.

- `Needs Mirror`: 1 file (ported)
- `Intentional`: 5 files

No additional low-risk reverse-diff mirror actions remain from this Wave 5 batch.