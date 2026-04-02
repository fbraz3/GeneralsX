# PHASE06: Windows x64 Unified Modern Build (Issue #29)

**Status**: In progress (core build + CI wiring landed)  
**Created**: 2026-04-01  
**Owner**: Bender (Copilot)  
**Issue**: https://github.com/fbraz3/GeneralsX/issues/29

## 1. Executive Summary

This phase defines the migration of Windows builds to the same modern stack already used by Linux/macOS:

- SDL3 for windowing/input
- DXVK for D3D8 translation to Vulkan
- OpenAL for audio
- FFMPEG for video
- x86_64-only targets
- ** IMPORTANT**: You are allowed to break win32 / vc6 builds, but it's imperative to keep Linux and Mac builds working.

User-level policy for this phase explicitly overrides legacy constraints:

1. VC6 compatibility is no longer required.
2. After functional Windows x64 build exists, documentation must be updated for the post-legacy model.
3. `.github/workflows/build-windows.yml` must be implemented to produce real Windows artifacts.
4. 32-bit targets should be removed if impact is low.

This document provides a deep technical audit, implementation plan, risk analysis, and acceptance checklist.

## 2. Inputs and Policy Overrides

### 2.1 Source Inputs

- GitHub issue #29 target: modern Windows x64 build with SDL3 + DXVK + OpenAL.
- Current repository state (audit performed on 2026-04-01).
- Reference repositories and architecture docs:
  - `references/references/fighter19-dxvk-port`
  - `references/references/jmarshall-win64-modern`
  - DeepWiki summaries for `TheSuperHackers/GeneralsGameCode`
  - DeepWiki summaries for `electronicarts/CnC_Generals_Zero_Hour`

### 2.2 Effective Rules for Phase 6

- Legacy VC6 continuity is not a blocker.
- Target architecture is x64 only.
- Windows must converge on the same stack as Linux/macOS.
- Determinism remains mandatory (render/audio/backend changes must not change gameplay simulation behavior).

## 3. Current-State Audit (2026-04-01)

## 3.1 Build Presets and Architecture

Observed in `CMakePresets.json`:

- Present:
  - `vc6*` presets (legacy 32-bit)
  - `win32*` presets (32-bit)
  - `win32-vcpkg*` presets (32-bit)
  - `mingw-w64-i686*` presets (32-bit)
  - `linux64-deploy`
  - `macos-vulkan`
- Missing:
  - `windows64-deploy`
  - `windows64-testing`
  - Any x64 Windows build preset

Conclusion: issue #29 cannot be completed without introducing Windows x64 presets.

## 3.2 CI/Workflow Status

Observed in workflows:

- `.github/workflows/build-windows.yml` is a placeholder that exits successfully without compiling.
- `.github/workflows/ci.yml` triggers reusable `build-windows.yml`, but passes preset `win64-modern`.
- `build-windows.yml` workflow input options expose `windows64-deploy` only.

Critical mismatch:

- CI calls `win64-modern`
- Windows workflow advertises `windows64-deploy`
- Neither preset currently exists in `CMakePresets.json`

Conclusion: Windows CI currently reports success without validating build correctness.

## 3.3 Dependency and Backend State

Observed in CMake modules:

- `cmake/openal.cmake` already supports FetchContent on all platforms, including Windows.
- `cmake/sdl3.cmake` is FetchContent-based and controlled via `SAGE_USE_SDL3`.
- `cmake/dx8.cmake` currently has three branches:
  - `SAGE_USE_DX8` => native DX8 SDK path (legacy)
  - `APPLE && SAGE_USE_MOLTENVK` => DXVK source build via Meson
  - else => Linux DXVK native tarball path

Gap:

- No dedicated Windows modern branch in `cmake/dx8.cmake` for DXVK on Windows x64.

## 3.4 Scripts and Tasking

Observed repository state:

- `scripts/build/windows/` does not exist.
- `.vscode/tasks.json` includes Linux/macOS tasks only.
- No Windows modern build/deploy/run scripts are available.

Conclusion: tooling parity for Windows is currently absent.

## 3.5 Documentation State

Observed docs:

- Build docs exist for Linux and macOS (`docs/BUILD/LINUX.md`, `docs/BUILD/MACOS.md`).
- No dedicated `docs/BUILD/WINDOWS.md` for modern SDL3+DXVK+OpenAL x64 flow.
- Various instruction files still describe legacy VC6 as baseline.

Conclusion: documentation does not reflect the requested post-legacy phase.

## 4. Root-Cause Analysis

Issue #29 remains unresolved due to structural blockers rather than a single missing patch:

1. No Windows x64 CMake preset to drive configure/build.
2. Windows workflow is intentionally stubbed.
3. DXVK CMake integration has Linux/macOS modern paths but not Windows modern path.
4. No Windows build scripts/tasks for consistent local/CI execution.
5. Documentation has not transitioned to x64-only guidance.

## 5. Impact Assessment for 32-bit Removal

### 5.1 Scope of 32-bit Surfaces

Primary 32-bit surfaces found:

- Presets in `CMakePresets.json`:
  - `vc6*`
  - `win32*`
  - `win32-vcpkg*`
  - `mingw-w64-i686*`
- Toolchain file: `cmake/toolchains/mingw-w64-i686.cmake`
- Triplet: `triplets/x86-windows.cmake`
- Docs and instructions referencing VC6/win32
- Historical workflow files under `.github/workflows/old/`

### 5.2 Risk Level

Low-to-medium (manageable) if done in stages:

- **Low risk** for removing CI/task/docs references to 32-bit.
- **Medium risk** for deleting all 32-bit presets immediately (may break local dev expectations for contributors still using legacy scripts).

### 5.3 Recommended Strategy

Stage removal to reduce disruption:

1. Disable 32-bit in active CI and docs first.
2. Land stable `windows64-deploy` and verify artifact output.
3. Remove 32-bit presets/toolchains in a dedicated cleanup change after green builds.

This satisfies the "remove 32-bit if impact is low" requirement while preserving a controlled transition.

## 6. Reference Findings Applied to Phase 6

## 6.1 fighter19 Reference

From `references/references/fighter19-dxvk-port`:

- Demonstrates practical modern presets and multi-platform split.
- Shows that modern branch prioritizes functionality over strict legacy compatibility.
- Confirms viability of x64 toolchain path and DXVK-based rendering model.

Applied principle for this phase:

- Prioritize a working modern Windows target with clear presets and reproducible CI, not legacy coexistence.

## 6.2 jmarshall Reference

From `references/references/jmarshall-win64-modern`:

- Confirms practicality of modern Windows x64 migration with OpenAL-backed audio and modern compiler toolchain.

Applied principle for this phase:

- Keep audio backend migration on OpenAL path and avoid introducing new legacy audio coupling.

## 6.3 Upstream/EA DeepWiki Context

DeepWiki content for upstream/original code confirms:

- Architecture is deeply rooted in Win32 + D3D8-era assumptions.
- Most fragile coupling is around rendering device, platform abstractions, and legacy toolchain assumptions.

Applied principle for this phase:

- Isolate modernization changes in build/device/platform layers and avoid gameplay logic modifications.

## 7. Implementation Plan

## 7.1 Phase 6 Deliverables

1. Add Windows x64 CMake presets.
2. Implement real Windows build workflow.
3. Wire DXVK + SDL3 + OpenAL for Windows modern path.
4. Add Windows build scripts and VS Code tasks.
5. Publish Windows modern build documentation.
6. Execute staged 32-bit retirement plan.

## 7.2 Workstreams

### Workstream A: Build Foundation

- Add `windows64-deploy` configure/build/workflow presets.
- Optional companion debug preset: `windows64-testing`.
- Ensure key cache variables align with Linux/macOS modern stack:
  - `SAGE_USE_DX8=OFF`
  - `SAGE_USE_SDL3=ON`
  - `SAGE_USE_OPENAL=ON`
  - `SAGE_USE_GLM=ON`

### Workstream B: DXVK Windows Path

- Add explicit Windows modern DXVK integration path in `cmake/dx8.cmake`.
- Keep include/link behavior deterministic and local to backend layer.
- Verify produced runtime DLL/SO equivalents are bundled for Windows execution.

### Workstream C: CI Implementation

- Replace placeholder `.github/workflows/build-windows.yml` with real configure/build/artifact flow.
- Resolve preset naming mismatch (`win64-modern` vs `windows64-deploy`) in `ci.yml` and reusable workflow.
- Upload logs + binaries as artifacts, and fail pipeline on real build failure.

### Workstream D: Local Tooling

- Create `scripts/build/windows/` scripts for configure/build/deploy/run.
- Add corresponding entries to `.vscode/tasks.json` for reproducible local execution.

### Workstream E: Documentation Transition

- Create/expand `docs/BUILD/WINDOWS.md` for modern x64 flow.
- Update Linux/macOS docs where they still imply legacy-first framing.
- Update top-level project documentation to reflect post-VC6 policy.

### Workstream F: 32-bit Retirement

Stage 1 (immediate after x64 baseline works):

- Remove 32-bit from active CI matrix/tasks/docs.

Stage 2 (after one stable cycle):

- Remove `vc6*`, `win32*`, `win32-vcpkg*`, `mingw-w64-i686*` presets.
- Remove 32-bit toolchain/triplet files if no remaining references.

## 8. Detailed Checklist

## 8.1 Planning and Analysis

- [x] Deep audit of issue #29 requirements and current repository state.
- [x] Identify CI/preset mismatch blockers.
- [x] Analyze 32-bit removal impact and rollout strategy.

## 8.2 Build and Presets

- [x] Add `windows64-deploy` configure preset.
- [x] Add `windows64-deploy` build preset.
- [x] Add `windows64-deploy` workflow preset.
- [x] (Optional) Add `windows64-testing` debug preset set.

## 8.3 Backend Wiring

- [x] Implement Windows modern branch in `cmake/dx8.cmake`.
- [ ] Validate SDL3 link path on Windows modern target.
- [ ] Validate OpenAL target linkage on Windows modern target.

## 8.4 CI

- [x] Replace stub `build-windows.yml` job with real configure/build steps.
- [x] Align CI preset name to actual preset (`windows64-deploy`).
- [x] Upload Windows build logs/artifacts.
- [x] Ensure CI fails on compile/link errors.

## 8.5 Scripts and Tasks

- [x] Create `scripts/build/windows/build-windows-zh.sh` (or `.ps1` equivalent).
- [x] Create `scripts/build/windows/build-windows-generals.sh`.
- [x] Create deploy/run helpers for Windows modern binaries.
- [x] Add Windows tasks to `.vscode/tasks.json`.

## 8.6 Documentation

- [x] Create/update `docs/BUILD/WINDOWS.md` for x64 modern flow.
- [ ] Update docs that still present VC6/win32 as required baseline.
- [ ] Add migration notes for contributors (x64-only policy).

## 8.7 32-bit Retirement

- [ ] Remove 32-bit references from active CI workflow and docs.
- [ ] Remove 32-bit presets/toolchains after Windows x64 baseline is green.

## 9. Validation Strategy

## 9.1 Build Validation

- Configure: `cmake --preset windows64-deploy`
- Build ZH: `cmake --build build/windows64-deploy --target z_generals`
- Build Generals: `cmake --build build/windows64-deploy --target g_generals`

## 9.2 Runtime Smoke Validation

For both `GeneralsXZH` and `GeneralsX`:

1. Launch to main menu in windowed mode.
2. Enter skirmish map.
3. Verify no immediate crash on render/audio init.

## 9.3 Determinism Safety Checks

- No gameplay logic code changes in GameLogic modules as part of this phase.
- Restrict changes to build system, platform/device layers, and documentation/tooling.

## 10. Risks and Mitigations

| Risk | Severity | Mitigation |
|------|----------|------------|
| DXVK Windows integration gaps | High | Add explicit Windows branch in `cmake/dx8.cmake`; test minimal render path first |
| CI false green due to stub behavior | High | Replace stub with real build and strict failure conditions |
| Preset naming drift (`win64-modern` vs `windows64-deploy`) | Medium | Standardize on `windows64-deploy` everywhere |
| Contributor confusion during 32-bit retirement | Medium | Stage removal and document migration clearly |
| Hidden 32-bit references break tooling | Medium | Grep-driven audit before deleting presets/files |

## 11. Definition of Done (Phase 6)

Phase 6 is complete when all conditions are true:

1. Windows x64 modern preset exists and builds both game targets.
2. Windows CI performs real compile/link and uploads artifacts.
3. Windows binaries launch to main menu and load a skirmish map.
4. Build documentation is updated for the new x64-only modern direction.
5. Active CI and docs no longer depend on 32-bit/VC6 paths.
6. 32-bit artifacts are either removed or explicitly deprecated with a tracked removal commit plan.

## 12. Immediate Next Execution Batch

Implementation should start with the highest-leverage blockers in this exact order:

1. Presets (`windows64-deploy`) + CI naming alignment.
2. Real Windows workflow implementation.
3. DXVK Windows modern CMake branch.
4. Scripts/tasks for local reproducibility.
5. Documentation update and staged 32-bit retirement.
