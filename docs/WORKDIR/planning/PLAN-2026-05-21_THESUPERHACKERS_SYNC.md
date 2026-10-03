# Plan: TheSuperHackers Upstream Sync (2026-05-21)

## Context

This sync merges `thesuperhackers/main` into `thesuperhackers-sync-05-21-2026` while preserving the cross-platform GeneralsX architecture (SDL3 + DXVK + OpenAL + FFmpeg).

Current merge state:
- Merge command has been executed.
- Conflict set identified and resolved:
  - `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/SlavedUpdate.cpp`
- 74 files are staged by the merge result, concentrated in:
  - `GeneralsMD` (28)
  - `Generals` (25)
  - `Core` (21)

## Goals

1. Keep useful upstream fixes and stability updates.
2. Preserve existing GeneralsX cross-platform behavior and platform isolation.
3. Resolve conflicts with root-cause reasoning, not blanket keep-ours/keep-theirs.
4. Validate requested scope only:
   - Build on macOS for `GeneralsXZH` and `GeneralsX`.
   - Do not build Linux in this session.
   - Do not execute game binaries in this session.

## Important Conflict and Resolution Strategy

### 1) SlavedUpdate welding particle logic (`GeneralsMD/.../SlavedUpdate.cpp`)

Observed conflict pattern:
- Upstream side simplifies duplicated position block and uses cleaner lifetime expression.
- Local side includes defensive template null-checking and had duplicated position handling in the conflict region.

Resolution policy:
- Keep the functional cleanup that removes duplicated/ambiguous bone position code.
- Keep defensive template validity check before particle creation.
- Keep equivalent lifetime behavior (no gameplay timing change).
- Preserve audio event placement behavior.

Risk level: Medium (visual/audio repair effect path), low gameplay determinism risk.

Mitigation:
- Resolve with minimal semantic delta.
- Re-scan merge markers globally.
- Compile both macOS products to ensure compile-time integrity.

## High-Risk Areas to Review After Conflict Resolution

Even without textual conflicts, upstream integration touched many gameplay/engine files. These areas are risk hotspots and will be reviewed through targeted diff checks:

1. Build and integration paths under `Core` and product game trees (`Generals`, `GeneralsMD`).
2. Shared engine behavior potentially unified/refactored upstream.
3. Launch/runtime wiring changes that may affect renderer/audio/input initialization indirectly.
4. INI/file-load sensitive code if touched by staged merge results (manual grep/diff review).

## Step-by-Step Execution Plan

1. Resolve the single unresolved conflict in `SlavedUpdate.cpp` with a reconciled version.
2. Stage the resolved file and confirm no `U` files remain.
3. Verify no merge markers remain anywhere in tree (`<<<<<<<`, `=======`, `>>>>>>>`).
4. Perform targeted post-merge diff review for touched subsystem hotspots.
5. Build only on macOS, sequentially:
   - `GeneralsXZH`
   - `GeneralsX`
6. Skip Linux builds by explicit user request.
7. Skip runtime execution/smoke by explicit user request.
8. Remove generated runtime/build artifacts that appear in working tree.
9. Update dev diary with sync summary before commit.
10. Commit merge result.
11. Push branch `thesuperhackers-sync-05-21-2026` to origin.

## Decision Log (executed)

- Decision A:
  - File(s): `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/SlavedUpdate.cpp`
  - Problem: merge conflict in welding particle path mixed two divergent edits; one side carried duplicated bone-position handling while the other side simplified the block.
  - Chosen resolution: keep the simplified single position flow, preserve defensive `tmp` null-check before particle creation, and keep lifetime expression behavior aligned with upstream (`m_framesToWait * LOGICFRAMES_PER_SECOND`).
  - Why this preserves GeneralsX strategy: avoids regressions in cross-platform visual/audio effect path while retaining defensive behavior and removing ambiguous duplicated logic.

- Decision B:
  - File(s): whole merge tree (post-resolution check)
  - Problem: ensure merge completion quality before validation.
  - Chosen resolution: verified zero unresolved `U` files and ran explicit marker checks for `<<<<<<<`, `=======`, `>>>>>>>` patterns after conflict resolution.
  - Why this preserves GeneralsX strategy: prevents latent merge artifacts from breaking cross-platform build/config pipelines.

## Validation Scope Statement

This session intentionally uses reduced validation scope requested by user:
- Included: macOS build for `GeneralsXZH` and `GeneralsX`.
- Excluded: Linux build.
- Excluded: runtime launch/main-loop smoke tests.
