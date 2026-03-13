# PLAN-021: CI Runtime Confidence Strategy

**Date**: 2026-03-10
**Status**: Proposed
**Scope**: GeneralsMD (primary), extend to Generals when low-risk

## Problem Statement

Current CI confidence is dominated by compile and link success. This catches build breakages, but it does not reliably catch startup crashes (for example, segfaults during SDL3/DXVK/OpenAL initialization).

Upstream replay-based validation is highly valuable, but replay checks alone do not guarantee runtime bootstrap safety in modern cross-platform paths.

## Goal

Increase confidence that a generated build can start and run through early initialization without crashing, while preserving deterministic replay validation.

## Evidence and Context

### Existing repository behavior

- Current CI builds Linux and macOS, then validates artifacts.
- Existing Linux smoke script currently tolerates crash in some paths and is diagnostic-oriented.
- Historical upstream replay workflow executes `-headless -replay` checks with timeout and log upload.

### Upstream/DeepWiki behavior summary

Headless replay testing is best at:
- Logic determinism regression detection
- Desync detection via CRC checks
- Replay compatibility checks

Headless replay testing is weak at:
- Graphics initialization crashes
- Audio device initialization crashes
- UI/window/input startup regressions
- Early process bootstrap failures before replay execution path

## Recommended Confidence Model

Use layered validation instead of a single gate.

### Layer 1: Build Integrity (already present)

Purpose:
- Verify configure, compile, and link
- Verify expected artifacts are produced

### Layer 2: Runtime Bootstrap Smoke (new mandatory gate)

Purpose:
- Catch startup segfaults and fatal initialization errors quickly

Definition:
- Run built executable for a short bounded window (20-60 seconds)
- Fail on non-zero exit code, timeout without expected health marker, or crash signature
- Always collect logs and crash diagnostics as artifacts

Minimum checks by platform:
- Linux: run deployment bundle with required shared libraries in place
- macOS: run built app/binary with Vulkan/OpenAL environment configured in CI runner

### Layer 3: Headless Replay Determinism (mandatory, scoped)

Purpose:
- Catch game logic regressions and determinism breaks

Definition:
- PR: run a short replay subset for fast feedback
- Nightly: run full replay suite for deep confidence

### Layer 4: Sanitizer Runtime (nightly or optional PR)

Purpose:
- Detect memory errors and undefined behavior not visible in normal builds

Definition:
- Linux/macOS ASan + UBSan job
- Run smoke startup and at least one replay sample

## Why Replay-Only Is Not Enough

Replay-only can still pass when startup is broken in modern runtime paths because headless mode bypasses substantial rendering/audio/UI initialization work.

Replay should remain a key gate, but as a complement to runtime bootstrap smoke, not a replacement.

## Proposed CI Rollout

## Phase A (Immediate)

- Add runtime bootstrap smoke jobs to Linux and macOS reusable workflows.
- Make smoke jobs required for PR merge.
- Ensure crash diagnostics are uploaded on failure.

Acceptance Criteria:
- CI fails if process crashes during startup
- CI fails if executable cannot reach defined health signal within timeout
- Artifacts include startup logs and backtrace/crash reports

## Phase B (Short Term)

- Add replay subset gate for PRs (fast deterministic coverage).
- Add full replay suite on nightly schedule.

Acceptance Criteria:
- PR replay gate runtime remains practical for contributor feedback
- Nightly replay suite produces stable pass/fail signal

## Phase C (Medium Term)

- Add sanitizer matrix (ASan/UBSan) to nightly.
- Promote sanitizer smoke to optional PR label-triggered checks.

Acceptance Criteria:
- Memory/UB regressions are surfaced within 24h through nightly pipeline

## Practical Job Design

## Runtime Smoke Job Contract

Inputs:
- `game`
- `preset`
- `timeout_seconds`

Behavior:
- Prepare runtime bundle/environment
- Launch executable with CI-safe arguments
- Enforce timeout
- Parse logs for health marker and fatal signatures
- Return non-zero on crash/failure conditions

Artifacts:
- Startup stdout/stderr
- Engine debug logs
- Platform crash diagnostics (where available)

## Replay Job Contract

Inputs:
- `game`
- `preset`
- replay dataset path or artifact
- timeout

Behavior:
- Run `-headless -replay` with bounded timeout
- Fail on non-zero exit or determinism mismatch signal
- Upload replay logs and CRC output

## Risk and Trade-Offs

- CI duration will increase; mitigate with PR subset + nightly full suite.
- Runtime smoke can be flaky if environment is unstable; mitigate by bundling libs and strict deterministic startup arguments.
- Replay dataset management requires legal-safe trimmed assets and caching strategy.

## Success Metrics

- Reduction in post-merge startup crash incidents
- Reduced mean time to identify runtime regressions
- Stable PR feedback loop (low flaky-failure rate)
- Replay determinism signal remains green over time

## Decision

Adopt a layered model:
1. Build integrity
2. Runtime bootstrap smoke (required)
3. Replay determinism (required, scoped by PR vs nightly)
4. Sanitizers (nightly)

This model gives better confidence than build-only and better real-world startup protection than replay-only.
