# Generals Parity Wave 3 Audit: GameEngineDevice

Date: 2026-05-11
Scope: `GeneralsMD/Code/GameEngineDevice/**` annotation deltas that do not currently appear in `Generals/Code/GameEngineDevice/**` annotation set.
Status: Completed (Batches 1-3 completed; OpenAL/FFmpeg scope deferred by design)

## Classification Legend

- Applicability:
  - `Direct`: Can be ported with minimal/no semantic adaptation.
  - `Adapted`: Should be ported with Generals base-specific adjustments.
  - `N/A`: Zero Hour-only behavior/content.
- Risk:
  - `Low`, `Medium`, `High`

## ZH -> Generals Missing Candidate Inventory

| File | Subsystem | Applicability | Risk | Decision | Notes |
|---|---|---|---|---|---|
| Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/W3DBufferManager.cpp | Shadow backend | Direct | Low | Ported (Batch 1) | Ported `std::max` to `MAX` macro parity for VB/IB sizing paths. |
| Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/W3DProjectedShadow.cpp | Shadow backend | Adapted | Medium | Ported (Batch 1 + Batch 2) | Batch 1: projection/decal bitmask checks + `MAX/MIN` clipping parity. Batch 2: dynamic projection update flagging and per-frame refresh path for animated casters. |
| Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/W3DShadow.cpp | Shadow backend | Adapted | Low | Ported (Batch 1) | Ported ShadowType routing by bitmask to handle combined flags safely. |
| Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/W3DVolumetricShadow.cpp | Shadow backend | Adapted | Low | Ported (Batch 1) | Ported strict-aliasing-safe edge-pair swaps and `__max` to `MAX` portability parity. |
| Code/GameEngineDevice/Include/PchCompat.h | Build/platform include | Adapted | Medium | Deferred (intentional) | Header exists only in ZH include surface; not required for current Generals build path. |
| Code/GameEngineDevice/Include/OpenALAudioManager.h | Audio backend glue | Adapted | Medium | Deferred (intentional) | Generals base has no local OpenAL manager source file in this path; keep Wave 3 scope focused on shared renderer/shadow backend parity. |
| Code/GameEngineDevice/Source/OpenALAudioManager.cpp | Audio backend | Adapted | High | Deferred (intentional) | ZH-only implementation unit in current tree; porting implies a new backend layer in Generals and dedicated audio runtime plan. |
| Code/GameEngineDevice/Source/VideoDevice/FFmpeg/FFmpegVideoPlayer.cpp | Video/audio backend | Adapted | High | Deferred (intentional) | ZH-only FFmpeg/OpenAL integration path; keep in dedicated audio/video parity track with startup and media-validation harness. |

## Generals -> ZH Reverse Deltas (Validation List)

| File | Direction | Decision | Notes |
|---|---|---|---|
| Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DScene.cpp | Generals-only annotation | Intentional (annotation-only) | Runtime behavior already matches ZH infantry-lighting path; no mirror required for code behavior. |

## Batch 1 Progress (This Session)

- Ported low-risk backend-only shadow parity in 4 files:
  - `W3DBufferManager.cpp`
  - `W3DProjectedShadow.cpp`
  - `W3DShadow.cpp`
  - `W3DVolumetricShadow.cpp`
- Kept scope strictly rendering/backend safety (no gameplay logic deltas).
- Validation completed with Build + Deploy tasks on macOS without automatic game execution.

## Batch 2 Progress (This Session)

- Ported dynamic projected-shadow synchronization behavior in `W3DProjectedShadow.cpp`:
  - dynamic projection flag propagation from `shadowType`/`allowUpdates` during shadow creation,
  - per-frame texture refresh path for `SHADOW_DYNAMIC_PROJECTION` casters,
  - projection-matrix update condition switched to bitmask-safe check (`m_type & SHADOW_PROJECTION`).
- Scope remained renderer/backend-only with no gameplay logic changes.
- Validation completed with Build + Deploy tasks on macOS without automatic game execution.

## Batch 3 Progress (This Session)

- Ported remaining volumetric-shadow safety delta in `W3DVolumetricShadow.cpp`:
  - clamp `vectorScaleMax` to `MAX_EXTRUSION_LENGTH` before shadow volume construction to avoid runaway extrusion spikes on shallow light rays.
- Kept scope strictly shadow backend behavior (no gameplay/system logic changes).
- Validation completed with Build + Deploy tasks on macOS without automatic game execution.

## Remaining Work for Wave 3

1. Wave 3 renderer/shadow parity is complete for current Generals base scope.
2. OpenAL/FFmpeg items are intentionally deferred to a dedicated audio/video parity track.

## Validation Gates

1. Build Generals (active preset).
2. Deploy Generals.
3. User-run manual smoke (menu + skirmish visual checks).
4. Targeted shadow checks for decal/projection/volumetric rendering.

## Notes

- This audit intentionally excludes automatic game execution.
- Decisions are limited to Wave 3 GameEngineDevice scope and current low-risk parity policy.
