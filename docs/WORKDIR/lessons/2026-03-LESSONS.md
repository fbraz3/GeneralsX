# 2026-03 Lessons Learned

## Session 2026-03-09 - macOS deploy path regression

- Problem: `scripts/build/macos/deploy-macos-zh.sh` resolved `PROJECT_ROOT` with `$(dirname "$0")/..`, which points to `.../scripts/build` when called from `scripts/build/macos`.
- Symptom: Deploy looked for binary at `scripts/build/build/macos-vulkan/GeneralsMD/GeneralsXZH` and failed.
- Root cause: Script location changed to `scripts/build/macos`, but root-resolution logic still assumed a different layout.
- Fix: Use `SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"` and `PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"`.
- Scope: Applied to `scripts/build/macos/deploy-macos-zh.sh`, `scripts/build/macos/run-macos-zh.sh`, and `scripts/build/macos/bundle-macos-zh.sh`.
- Prevention: In scripts under nested folders, derive repo root from `BASH_SOURCE[0]` and keep helper command examples aligned with the actual script paths.

## Session 2026-03-09 - Linux deploy/bundle mirrored the same root-path bug

- Problem: `scripts/build/linux/deploy-linux-zh.sh` and `scripts/build/linux/bundle-linux-zh.sh` used the same `$(dirname "$0")/..` root resolution pattern.
- Symptom: They would resolve `PROJECT_ROOT` as `.../scripts/build`, causing wrong binary/library paths (`.../scripts/build/build/linux64-deploy/...`).
- Fix: Standardized both scripts to `SCRIPT_DIR + ../../..` based on `${BASH_SOURCE[0]}`.
- Scope: Applied to `scripts/build/linux/deploy-linux-zh.sh` and `scripts/build/linux/bundle-linux-zh.sh`; updated Linux helper command paths to `./scripts/build/linux/...` for consistency.
- Prevention: When scripts are moved into deeper subfolders, audit all sibling platform scripts for the same root-resolution assumption before closing the change.

## Session 2026-03-09 - DXVK patch drift replaced by fork-pinned source

- Problem: macOS crashed with `EXC_BREAKPOINT` in `dxvk::env::getExeName()` because DXVK source lacked a valid `__APPLE__` branch in `getExePath()`.
- Root cause: The dynamic patch pipeline reported success for Patch 6 but did not actually modify `util_env.cpp` due brittle text replacement assumptions.
- Fix: Applied the full macOS patchset directly in `fbraz3/dxvk` branch `generalsx-macos-v2.6`, manually added `_NSGetExecutablePath` branch in `src/util/util_env.cpp`, and pinned CMake to commit `ffcdbcaf1b5a321406ffed43c4e815fd7c7e1797`.
- Build integration: `cmake/dx8.cmake` now fetches `https://github.com/fbraz3/dxvk.git`, uses `DXVK_SOURCE_DIR=_deps/dxvk-src-fbraz3`, and disables `PATCH_COMMAND` for macOS ExternalProject.
- Validation: Game reached runtime loop without reproducing the immediate `Direct3DCreate8` trap.

## Session 2026-03-10 - Build success is not startup safety in CI

- Problem: Build-only CI can pass while runtime startup still crashes (segfault/abort) in SDL3/DXVK/OpenAL initialization paths.
- Insight: Headless replay is excellent for determinism and logic regressions, but it can bypass graphics/audio/UI startup paths and miss bootstrap crashes.
- Decision: Use layered CI gates: build integrity, runtime bootstrap smoke (required), replay determinism (required, scoped), and sanitizers (nightly).
- Reference: `docs/WORKDIR/planning/PLAN-021_CI_RUNTIME_CONFIDENCE.md`.

## Session 2026-03-12 - Backslash path literals break save/replay on Linux

- Problem: Save and replay directory helpers used Windows suffixes like `"Save\\"` and `"Replays\\"` unconditionally.
- Symptom: Linux created literal entries such as `Save\\` and `Save\\00000002.sav` under `~/.local/share/generals_zh/`, so save/load and replay discovery behaved incorrectly.
- Root cause: Backslash is a normal filename character on POSIX, not a path separator.
- Fix: Added platform-specific separators in `GameState::getSaveDirectory()` and `RecorderClass::getReplayDir()/getReplayArchiveDir()` for both ZH and Generals (`\\` on Windows, `/` on non-Windows).
- Prevention: Never hardcode Windows path separators in cross-platform code; centralize path construction by platform or use filesystem paths when possible.
