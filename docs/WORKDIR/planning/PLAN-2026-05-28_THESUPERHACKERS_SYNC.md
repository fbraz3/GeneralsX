# Plan: TheSuperHackers Upstream Sync (2026-05-28)

## Context

Branch: thesuperhackers-sync-05-28-2026

Merge command executed:
- git merge thesuperhackers/main

Current merge state:
- One unresolved conflict remains in Core/GameEngine/Include/GameClient/ParticleSys.h.
- Additional non-conflict merge changes touched multiple files in Core, Generals, and GeneralsMD and still require semantic review.

Why this merge is high risk:
- Upstream keeps refining replay determinism and subsystem behavior.
- GeneralsX must preserve SDL3 + DXVK + OpenAL + FFmpeg cross-platform architecture and avoid regressions in launch/render/audio startup paths.

## Most Important Conflicts And How They Will Be Resolved

### 1) Particle system dummy behavior

File:
- Core/GameEngine/Include/GameClient/ParticleSys.h

Observed conflict:
- Local side adds a headless safety update override to prevent full particle update path execution.
- Upstream side adds CRC-aware behavior split:
  - In RETAIL_COMPATIBLE_CRC mode, keep default init behavior to preserve particle-template-driven logic CRC.
  - In non-retail mode, skip init/reset and keep dummy no-op behavior.

Resolution strategy:
- Keep both intents by composing behavior:
  - Preserve upstream RETAIL_COMPATIBLE_CRC guard on init/reset.
  - Preserve GeneralsX headless-safe no-op update behavior.
  - Keep dummy identity and render no-op methods.

Rationale:
- Avoid replay determinism regressions from missing particle templates in retail CRC mode.
- Keep headless runtime stable by ensuring the hot update path remains inert.

Risk level:
- High for replay compatibility and headless runtime stability.

Mitigation:
- Resolve with minimal semantic delta and no API shape changes.
- Validate configure/build on macOS, then Linux sequentially.
- Validate runtime smoke paths for GeneralsXZH and GeneralsX minimum main-loop entry/exit.

## High-Risk Areas For Post-Merge Semantic Review

These may not have textual conflicts but are operationally risky and will be reviewed after conflict resolution:

1. Build system and presets:
- CMakeLists.txt, CMakePresets.json, cmake/*, scripts/build/*, scripts/env/*

2. Platform abstraction and render/audio stack:
- Core/GameEngineDevice/*
- SDL3/DXVK/OpenAL/FFmpeg integration paths

3. Replay and logic determinism:
- Core/GameEngine/Source/Common/*
- INI parsing and file-load-order-sensitive code

4. Unified Core migration impacts:
- Any upstream move/unify changes between Generals*/ and Core/

5. Runtime launch and integration wiring:
- entry points, renderer startup, audio wiring, and asset/runtime path handling

## Detailed Resolution Procedure

### Phase A - Conflict Resolution

1. Resolve ParticleSys conflict by combining upstream CRC guard and local headless safety update no-op.
2. Stage the file and verify no unresolved U status remains.
3. Verify no merge markers remain in repository.

### Phase B - Semantic Validation Of Merge Result

1. Inspect merged diff in risk areas for silent behavior regressions.
2. Confirm no cross-platform stack downgrades were introduced.
3. Confirm no platform-specific leakage into gameplay logic.

### Phase C - Build And Runtime Validation (Sequential)

1. macOS configure/build flow for both products:
- configure/build GeneralsXZH first
- configure/build GeneralsX second

2. Linux configure/build flow for both products (after macOS completes).

3. Runtime smoke:
- GeneralsXZH main loop enters/exits cleanly
- GeneralsX main loop enters/exits cleanly

### Phase D - Finalization

1. Remove generated artifacts (runtime caches and local build/run byproducts).
2. Ensure clean merge state and no unresolved markers.
3. Update dev blog with this sync session summary.
4. Commit merge result.
5. Push branch to origin.

## Constraints And Special Considerations

- No blanket keep-ours/keep-theirs strategy.
- No sacrifice of SDL3 + DXVK + OpenAL + FFmpeg architecture for merge convenience.
- INI and numeric parsing changes are treated as macOS high risk and require deliberate review.
- If conflict resolution implies capability removal or downgrade, document explicit justification before acceptance.

## Decision Log (to be updated during execution)

- Decision A - ParticleSys conflict resolved with composed behavior.
  - File: Core/GameEngine/Include/GameClient/ParticleSys.h
  - Outcome:
    - Kept upstream RETAIL_COMPATIBLE_CRC split (do not suppress template loading in retail-compatible mode).
    - Kept local GeneralsX headless protection (`update()` remains no-op in dummy manager).
    - Kept new polymorphic identity hook (`isDummy()`), with base false and dummy true.
  - Why:
    - Preserves replay/CRC-sensitive behavior while maintaining headless stability in cross-platform runtime paths.

- Decision B - Replay headless update path removal accepted with risk note.
  - Files:
    - Core/GameEngine/Source/Common/ReplaySimulation.cpp
    - Generals/Code/GameEngine/Source/GameClient/GameClient.cpp
    - GeneralsMD/Code/GameEngine/Source/GameClient/GameClient.cpp
  - Outcome:
    - Accepted upstream removal of `GameClient::updateHeadless()` call and method bodies.
  - Why:
    - Upstream change is intentionally paired with particle-system dummy semantics to avoid lifecycle side effects.
    - This avoids dual update semantics in headless replay loops.
  - Follow-up risk mitigation:
    - Runtime smoke for both products remains mandatory before finalizing.

- Decision C - Audio request ownership hardening accepted.
  - Files:
    - Core/GameEngineDevice/Include/MilesAudioDevice/MilesAudioManager.h
    - Core/GameEngineDevice/Source/MilesAudioDevice/MilesAudioManager.cpp
  - Outcome:
    - Accepted upstream API shift from `playAudioEvent(AudioEventRTS*)` to `playAudioEvent(AudioRequest*)`.
    - Accepted `releasePendingEvent()` transfer points to centralize event ownership.
  - Why:
    - Reduces dangling/duplicate ownership risk in play request handling.

- Decision D - Water rendering fixes accepted.
  - File: Core/GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWater.cpp
  - Outcome:
    - Accepted river alpha + shroud scaling adjustment.
    - Accepted shader combine fix for river visuals under shroud.
  - Why:
    - Aligns with recent upstream water bugfix intent without touching SDL3/DXVK integration boundaries.

- Validation status:
  - macOS configure: PASS.
  - macOS build `z_generals`: PASS (exit code 0, warnings only).
  - macOS build `g_generals`: PASS (exit code 0, warnings only).
  - Linux configure/build: IN PROGRESS (Docker configure running).
  - Runtime smoke: PENDING.
  - Artifact cleanup, commit, push: PENDING.