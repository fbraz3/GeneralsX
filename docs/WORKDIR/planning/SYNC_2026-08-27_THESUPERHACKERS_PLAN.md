# TheSuperHackers Sync Plan - 2026-08-27

## Scope

Merge `thesuperhackers/main` into `thesuperhackers-sync-08-27-2026` from
GeneralsX commit `fb5f74361`. The upstream range starts after common ancestor
`ec0658a35` and contains 26 non-merge commits across 83 files.

The sync imports save/load improvements, deterministic network command sorting,
shared tunnel-exit fixes, particle and scorch maintenance, selection fixes,
UTF-8 utilities, missile behavior corrections, and related Generals/Zero Hour
parity work.

The `thesuperhackers` SSH remote could not authenticate on this machine, so the
same public repository was fetched over HTTPS. No merge content changed because
of the transport fallback.

## Conflict Resolution

### Particle system dummy

**File:** `Core/GameEngine/Include/GameClient/ParticleSys.h`

Accept upstream's base `ParticleSystemManager::isDummy()` capability and the
dummy manager's `isDummy()` and `isXferEnabled()` overrides. The base declaration
was adjacent to independently merged code and must be restored explicitly so
the dummy override is valid. These methods expose existing behavior without
entering the full particle update or transfer paths. Preserve GeneralsX's
retail-compatible CRC branch, template loading, no-op update/render methods,
and headless replay safeguards.

### Save/load result handling

**Files:**

- `Generals/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/PopupSaveLoad.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/PopupSaveLoad.cpp`

Reconcile both implementations rather than selecting one side:

1. Keep the GeneralsX null check immediately after retrieving the selected save.
2. Keep `isPopup` as the authoritative indicator for an in-game load menu,
   because shell state may already have changed.
3. Accept upstream's `SaveCode` and filename capture, centralized
   `presentLoadResult()`, and recovery to the shell after a failed load.
4. Avoid retrieving the selected item twice and preserve Generals/Zero Hour
   parity.

This area is risky because loading resets engine and shell state while list item
data is being destroyed. Validation must cover successful and failed loads from
both the shell and in-game popup.

### Shared network exits

**Files:**

- `Generals/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate.cpp`

Accept upstream's polymorphic containment validation. Upstream adds
`ContainModuleInterface::isContained()` and overrides it in tunnel and cave
containers to search their shared contained-items list, so a passenger may
still exit through any endpoint in the same network. Other container types use
the direct `OpenContain` membership check.

This abstraction supersedes the GeneralsX-specific
`isSharedNetworkExitContainer()` and `isCaveContainer()` helpers. Remove those
helpers to avoid duplicate policy and accept upstream's
`objectToExit->getContain()->isContained(us)` checks in Generals and in both
Zero Hour exit paths. Preserve Zero Hour's subdued-state checks.

## Automatically Merged Areas Requiring Review

### Cross-platform and build structure

- Follow upstream's move of `W3DParticleSys.h` into `Core/`.
- Verify all three CMake surfaces reference the unified header exactly once.
- Preserve SDL3, DXVK, MiniAudio, FFmpeg, macOS, Linux, and experimental Windows
  source selection.
- Reject any CI or issue-template changes from upstream. The current upstream
  range does not touch `.github`, but the final diff will verify this.

### Determinism and gameplay

- Review `NetCommandList` sorting for stable full-key ordering and ID wraparound.
- Review diving missile and neutron missile changes for native `libm`, NaN,
  float-to-int, FMA, and FPU-state hazards.
- Review tunnel/cave occupant transfer and shared exits as one behavior set.
- Verify module-factory additions are identical between Generals and Zero Hour.
- Search all imported gameplay code for native transcendental math and replace
  unsafe calls with repository-standard `WWMath` wrappers where required.

### Save/load and text

- Validate `-loadsave`, deferred load requests, result presentation, and shell
  recovery ordering.
- Review UTF-8 conversion and fallback behavior for malformed byte sequences,
  save names, usernames, and macOS paths.
- Treat INI changes as high risk even though this range adds only the particle
  depth field; verify parsing and defaults on macOS.

### Renderer-only changes

- Confirm scorch, smudge, terrain, and particle changes remain client-only and
  do not alter simulation CRC state.
- Preserve GeneralsX renderer quality, texture filtering, shoreline alpha, and
  platform backend changes.

## Execution Order

1. Resolve particle dummy capability flags.
2. Reconcile save/load handling in Generals, then mirror the resolution in Zero
   Hour while preserving variant-specific behavior.
3. Resolve shared exits in Generals and both Zero Hour exit paths.
4. Audit moved files, CMake references, deterministic math, and `.github`.
5. Remove all merge markers and run `git diff --check`.
6. Configure macOS and build `g_generals` and `z_generals`.
7. Run macOS launch/main-loop smoke checks for both products.
8. Configure and build Linux only after macOS validation succeeds.
9. Run Linux smoke checks where the local Docker/runtime environment permits.
10. Remove generated caches and runtime artifacts from the worktree.
11. Update the August worklog and this document with major outcomes.
12. Commit the merge, push the dated branch to the fork remote, and open a PR
    against `fbraz3/GeneralsX:main`.

## Risk Mitigation

- Keep conflict edits surgical and compare each result with both merge stages.
- Preserve paired Generals/Zero Hour behavior unless a feature is variant-only.
- Use targeted builds first, then runtime smoke checks.
- Do not modify or replace CI/CD configuration.
- Do not accept native platform APIs in shared game logic.
- Stop before commit if either supported-platform build fails for a change
  introduced by this merge.

## Outcome

- Resolved all five content conflicts using the decisions above.
- Restored the base particle-manager capability required by the upstream dummy
  override.
- Adopted upstream's shared-list-aware containment API and removed the
  superseded GeneralsX-specific tunnel/cave helpers.
- Preserved save-selection null safety and `isPopup` handling while accepting
  centralized save/load result feedback.
- Confirmed no upstream changes entered `.github` or replaced GeneralsX CI.
- Confirmed imported shared game logic adds no native platform APIs.
- Replaced imported debug-only `ceilf`, `cosf`, and `sinf` calls with `WWMath`
  equivalents in both games.
- Configured `macos-vulkan` and built `g_generals` and `z_generals`.
- Launched and exited the main loop cleanly for both macOS products using the
  deployed Zero Hour Vulkan runtime.
- The `f7400-cleared.rep` replay mismatched at frame 6705 on both the pre-sync
  and merged binaries with identical CRCs, confirming a pre-existing fixture
  incompatibility rather than a sync regression.
- Linux Docker configure/build could not run because Docker is not installed
  on this machine. Pull-request CI remains required for Linux validation.
