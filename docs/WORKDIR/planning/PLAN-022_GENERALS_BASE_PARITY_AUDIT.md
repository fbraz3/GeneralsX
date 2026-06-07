# PLAN-022 Generals Base Parity Audit Against Zero Hour Customizations

Date: 2026-05-11
Owner: GeneralsX
Status: In Progress

## Goal

Establish systematic parity between the Generals base branch (`Generals/`) and the existing cross-platform customizations already present in Zero Hour (`GeneralsMD/`) where changes are shared, low-risk, and backend/platform related.

This plan intentionally shifts from bug-by-bug fixes to an audit-first parity workflow.

## Why This Plan Exists

Recent debugging (FPS cap and infantry lighting) exposed a recurring pattern:
- Isolated fixes are slower than auditing known customization deltas.
- Zero Hour has a larger customization footprint than Generals base.
- We need a deterministic, reproducible parity process with clear acceptance criteria.

## Current Inventory Snapshot

Quick inventory from `GeneralsX @...` annotations:
- Annotated files in Zero Hour tree: 114
- Annotated files in Generals base tree: 52

Relative-path files present in Zero Hour annotations but not in Generals annotations are concentrated in:
- `Code/GameEngine/Source` (22)
- `Code/CompatLib/Include` (19)
- `Code/GameEngineDevice/Source` (6)
- `Code/CompatLib/Source` (6)
- `Code/Libraries/Source` (4)
- `Code/GameEngine/Include` (4)
- `Code/GameEngineDevice/Include` (2)
- `Code/CompatLib/CMakeLists.txt` (1)

Relative-path files present in Generals annotations but not in Zero Hour annotations:
- `Code/GameEngine/Source` (2)
- `Code/GameEngineDevice/Source` (1)
- `Code/Libraries/Source` (1)

## Scope Rules

Include:
- Cross-platform backend changes (SDL3, DXVK integration points, OpenAL, FFmpeg glue, platform abstraction, file/path portability, deterministic timing guards).
- Shared stability fixes that do not alter faction/gameplay design.

Exclude:
- Zero Hour-only gameplay features/content behavior.
- Mod/expansion-specific mechanics without a Generals equivalent.
- Large refactors not required for parity.

## Parity Matrix Dimensions

Each candidate change is classified with:
- Subsystem: `CompatLib`, `GameEngine`, `GameEngineDevice`, `Libraries`, build glue.
- Change type: `build`, `platform`, `determinism`, `stability`, `UI/runtime`, `gameplay-risk`.
- Applicability: `Direct`, `Adapted`, `Not Applicable`.
- Risk: `Low`, `Medium`, `High`.
- Validation profile: `Compile`, `Boot`, `Menu`, `Skirmish`, `Replay`.

## Execution Waves

### Wave 0 - Baseline and Freeze

1. Freeze current ad-hoc parity edits into tracked parity work items.
2. Keep existing FPS/lighting edits under validation but do not continue random bug hunts.
3. Define a parity branch checklist per subsystem.

Deliverable:
- Baseline checklist committed in `docs/WORKDIR/audit/`.

### Wave 1 - CompatLib and Platform Surface

Target:
- `GeneralsMD/Code/CompatLib/**` deltas missing in `Generals/`.

Method:
1. Build file-by-file applicability table.
2. Port only platform abstraction and compile/runtime safety deltas.
3. Skip ZH-exclusive glue and confirm no gameplay coupling.

Acceptance:
- Generals compiles cleanly on active presets.
- No new platform include/type regressions.

### Wave 2 - GameEngine Core Runtime Parity

Target:
- Missing `GeneralsMD/Code/GameEngine/Source/**` portability/determinism/stability deltas.

Focus areas:
- Frame pacing bootstrapping and option fallback behavior.
- Path handling and case-sensitivity behaviors.
- Replay-safe timing and non-Windows runtime guards.

Acceptance:
- Menu and skirmish boot on macOS/Linux presets.
- Replay smoke path remains stable.

### Wave 3 - GameEngineDevice Rendering/Input/Audio Glue

Target:
- Missing `GeneralsMD/Code/GameEngineDevice/**` low-level deltas.

Focus areas:
- W3D rendering parity checks (including scene-light setup behavior).
- SDL3 input path parity where applicable.
- OpenAL/FFmpeg glue parity that is backend-only.

Acceptance:
- No regressions in window/input/audio startup.
- Render path parity checks pass for known visual baselines.

### Wave 4 - Libraries and Build Integration

Target:
- `GeneralsMD/Code/Libraries/Source/**` and build glue deltas.

Acceptance:
- Clean configure/build consistency for active platform presets.
- No dependency-order regressions.

### Wave 5 - Reverse-Diff Validation

Target:
- Files customized only in Generals base and absent in ZH annotation set.

Method:
1. Verify whether these are valid base-only adaptations.
2. Decide if any should be mirrored to ZH or explicitly documented as intentional divergence.

Acceptance:
- Every base-only customization has explicit classification: `Intentional` or `Needs Mirror`.

## Work Product Templates

For each subsystem, produce:
1. Delta inventory table: `File`, `Category`, `Applicability`, `Decision`, `Reason`.
2. Implementation batch list (small, reviewable commits).
3. Validation evidence (build + smoke + replay notes).
4. Follow-up issue list for non-parity bugs.

## Stop Conditions

Stop and escalate when:
- A candidate port touches Zero Hour-specific gameplay logic.
- The port requires deleting existing systems/components.
- Determinism risk is unclear.

In these cases, document in `docs/WORKDIR/audit/` and request explicit decision.

## Immediate Next Steps

1. Create Wave 1 audit sheet under `docs/WORKDIR/audit/` with file-by-file classification for `CompatLib` deltas.
2. Split candidates into `Direct` and `Adapted` buckets.
3. Implement the first low-risk parity batch from `CompatLib` only.
4. Run platform build validations after each batch.

## Success Criteria

Parity workflow is considered successful when:
- Every missing ZH annotation candidate has a documented decision.
- Low/medium-risk backend parity deltas are ported to Generals.
- Remaining mismatches are intentional, documented, and linked to tracked issues.
- Runtime behavior differences are reduced to a small, auditable set of non-parity bugs.
