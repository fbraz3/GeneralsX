# PLAN-2026-05-15 THESUPERHACKERS UPSTREAM SYNC

## Objective

Merge `thesuperhackers/main` into `thesuperhackers-sync-15-05-2026` while preserving the GeneralsX cross-platform stack:
- SDL3 for platform, windowing, and input
- DXVK for graphics
- OpenAL for audio
- FFmpeg where applicable
- strict platform isolation from gameplay logic

This sync must absorb useful upstream bug fixes and unification work without undoing the repository's cross-platform architecture.

## Merge Snapshot

Current unresolved conflicts after `git merge --no-ff thesuperhackers/main`:
- `Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt` - content conflict
- `Generals/Code/GameEngine/Include/GameLogic/Damage.h` - modify/delete conflict
- `Generals/Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp` - modify/delete conflict

High-risk auto-merged areas that should still be reviewed after conflict resolution:
- `Core/GameEngine/CMakeLists.txt`
- `Core/GameEngine/Source/Common/System/Radar.cpp`
- `Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp` under its new `Core/` location
- `Core/GameEngine/Include/GameLogic/Damage.h` under its new `Core/` location
- `Core/Libraries/Source/WWVegas/WW3D2/*` moved or renamed by upstream unification
- `Generals/Code/GameEngine/CMakeLists.txt` and `GeneralsMD/Code/GameEngine/CMakeLists.txt`
- platform and launcher wiring touched by the merge in `CMakeLists.txt` and `Core/*/CMakeLists.txt`

## Conflict Resolution Strategy

### 1) `Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt`

#### What Is Conflicted
The only textual conflict is the `dx8vertexbuffer.{cpp,h}` entry block.
- Our side kept those sources commented out.
- Upstream re-enabled them as part of the unified `Core/` library set.

#### Resolution Decision
Re-include `dx8vertexbuffer.{cpp,h}` unless the code proves platform-hostile during review.

#### Reasoning
- `DX8VertexBufferClass` is used throughout the renderer/device layer, including vertex-buffer consumers in `GameEngineDevice`.
- The code is part of the DX8 abstraction layer, not a native Win32-only gameplay dependency.
- The repository already treats DX8 via DXVK as a cross-platform rendering path, so excluding the vertex-buffer implementation would be a regression unless a concrete platform break is proven.

#### Risk
- Re-enabling the files could surface platform-specific include or ABI issues during build.
- Keeping them disabled would risk breaking renderer/device code that depends on them.

#### Mitigation
- Confirm the surrounding `Core/Libraries/Source/WWVegas/WW3D2` sources still build in the unified layout.
- Preserve the current Linux/macOS-safe browser stub comments already in the file for `dx8webbrowser`-related code.
- After resolution, run a repository-wide conflict-marker sweep and then a macOS configure/build validation.

### 2) `Generals/Code/GameEngine/Include/GameLogic/Damage.h`

#### What Is Conflicted
Upstream deleted the legacy `Generals/` copy because the content now lives under `Core/GameEngine/Include/GameLogic/Damage.h`.

#### Resolution Decision
Accept the upstream move to `Core/` and remove the stale `Generals/` duplicate.

#### Reasoning
- The merged `Core/` header is the authoritative location for unified game-engine headers.
- Keeping the legacy file would reintroduce duplicate sources of truth and increase drift between branches.
- The `Core/` version already carries the GeneralsX-specific ARM64-safe `DeathTypeFlags` fix, so the cross-platform behavior should remain intact through the unified path.

#### Risk
- Accidentally keeping or editing the stale `Generals/` duplicate would split behavior across products.
- Deleting the wrong file would break include resolution if a caller still points at the legacy path.

#### Mitigation
- Verify all include sites resolve to the `Core/` header after the move.
- Confirm no build files still depend on the deleted legacy path.
- Keep the ARM64-specific bit-shift fix and other cross-platform safety changes from the `Core/` copy.

### 3) `Generals/Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp`

#### What Is Conflicted
Upstream deleted the legacy `Generals/` implementation because the source now lives under `Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp`.

#### Resolution Decision
Accept the upstream move to `Core/` and remove the stale `Generals/` duplicate.

#### Reasoning
- This is part of the upstream `unify` move set and should follow the unified `Core/` layout.
- The `Core/` implementation is the one that should be kept current for shared gameplay logic.
- Retaining the old tree copy would create a second implementation path and make future merges harder.

#### Risk
- Gameplay dispatch is sensitive: a bad resolution could break command handling, rally points, UI transitions, or network/gameplay event flow.
- If the old path is still referenced in a build file, removal could break the build.

#### Mitigation
- Verify the `Core/` source contains the expected GeneralsX-safe changes and still compiles in its new location.
- Audit any include or build references that may still mention the old `Generals/` path.
- Keep the unified file as the single source of truth for gameplay dispatch logic.

## Special Constraints

- Do not use blanket ours/theirs resolution for the unification moves.
- Do not sacrifice the cross-platform architecture to simplify the merge.
- Do not reintroduce native platform APIs into gameplay logic.
- Treat any removal of an existing cross-platform capability as a high-risk decision and justify it explicitly.
- Preserve the repository's current platform strategy even when upstream structure is cleaner.

## Execution Order

1. Resolve the CMake conflict in `Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt`.
2. Remove the legacy `Generals/` duplicates that upstream moved into `Core/`.
3. Audit the new `Core/` copies for include and build integrity.
4. Verify no merge markers remain anywhere in the tree.
5. Validate the repository with the scope allowed by the user:
   - macOS configure/build only
   - no Linux build
   - no game execution
6. Clean generated build/runtime artifacts if any are created during validation.
7. Commit the merge result and push the sync branch.

## Validation Scope For This Session

The user explicitly requested:
- no Linux build
- no game execution

Therefore validation is intentionally limited to macOS configure/build checks only. Runtime smoke and Linux validation are deferred by request, not omission.

## Decision Log

### `Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt`
- Decision:
- Reasoning:
- Risk:
- Mitigation:

### `Generals/Code/GameEngine/Include/GameLogic/Damage.h`
- Decision:
- Reasoning:
- Risk:
- Mitigation:

### `Generals/Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp`
- Decision:
- Reasoning:
- Risk:
- Mitigation:
