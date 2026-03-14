# 2026-03 Lessons Learned

## Session 2026-03-14 - Base Generals language detection parity with Zero Hour

- Problem: Base Generals on Linux always attempted `Data\\english\\Language.ini` even on localized installs.
- Symptom: Startup logs showed `loadFileDirectory('Data\\english\\Language')` with zero files read on Brazilian Portuguese data sets.
- Root cause: `Generals/Code/GameEngine/Source/Common/System/registry.cpp` kept `_UNIX` stubs that always returned `FALSE`, so `GetRegistryLanguage()` stayed on cached default `"english"`. Zero Hour already had Linux fallback logic (`tryAutoDetectLanguage`) but base Generals did not.
- Fix: Added Linux env-var registry mapping (`CNC_GENERALS_<KEY>`) and BIG-based language auto-detection in base Generals registry code, with candidates for both base (`English.big`, `Brazilian.big`, etc.) and ZH-style (`EnglishZH.big`, `BrazilianZH.big`, etc.) localized packs.
- Prevention: For shared platform behavior (registry emulation, language fallback), always audit both `Generals/` and `GeneralsMD/` implementations before closing Linux parity tasks.

## Session 2026-03-14 - Input factories must preserve non-null initialization contracts

- Problem: `W3DGameClient::createMouse()` on non-Windows could return `nullptr` if `SDL3GameEngine` or its window handle was unavailable.
- Symptom: `GameClient::init()` immediately calls `TheMouse->parseIni()` after `createMouse()`, so a null return turns a setup invariant violation into a later null dereference.
- Root cause: The SDL3 migration encoded a fatal precondition failure as a nullable factory result even though callers treat mouse creation as mandatory.
- Fix: On non-Windows, keep creating `SDL3Mouse` when the SDL window exists, but abort immediately with a fatal diagnostic if the engine/window invariant is broken; do not return `nullptr` from the factory.
- Prevention: Factories used during core subsystem bootstrap must either always return a valid object or terminate at the point of invariant failure. Never defer that failure to downstream member calls.

## Session 2026-03-14 - fenv alone is not enough for Linux replay determinism on x86

- Problem: `setFPMode()` on non-Windows had been reduced to `fesetenv(FE_DFL_ENV)` plus `fesetround(FE_TONEAREST)`, which does not reproduce the Windows baseline's `_PC_24` x87 precision control.
- Symptom: Linux/x86 could run gameplay math with wider x87 precision than the original Windows simulation path, creating replay/desync risk even though the rounding mode looked correct.
- Root cause: `fenv` restores default environment and rounding, but it does not by itself force the x87 control word to 24-bit precision or explicitly realign SSE `MXCSR` rounding.
- Fix: In both Generals and Zero Hour `GameLogic.cpp`, keep the portable `fenv` reset, then on x86/x86_64 explicitly program the x87 control word to single-precision/round-to-nearest and align `MXCSR` rounding with `_MM_ROUND_NEAREST`.
- Prevention: When porting deterministic legacy game code, treat Windows `_controlfp(... _PC_24 | _RC_NEAR)` as behavioral contract, not as an implementation detail that can be replaced with generic `fenv` calls.

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

## Session 2026-03-14 - Base Generals Linux scripts drifted from Zero Hour fixes

- Problem: `scripts/build/linux/deploy-linux.sh` still used the pre-reorg root calculation and also pointed to the Zero Hour runtime/binary paths.
- Symptom: Deploy searched under `scripts/build/build/linux64-deploy/GeneralsMD/GeneralsX` and copied base-game output into `~/GeneralsX/GeneralsMD` instead of `~/GeneralsX/Generals`.
- Root cause: The Zero Hour Linux scripts were fixed after the folder move, but the sibling base-game scripts were not audited in the same pass.
- Fix: Standardized the base deploy/bundle scripts to `${BASH_SOURCE[0]}` + `../../..`, corrected base binary/runtime locations, and updated stale helper references to `scripts/build/linux/...` and `scripts/env/docker/...`.
- Prevention: When a script family is duplicated per game target, always apply and verify structural fixes across both variants before closing the change.

## Session 2026-03-14 - Follow-up audit caught stale usage strings and one broken variable block

- Problem: A follow-up grep found leftover usage strings pointing to pre-reorg script paths, and one patch had accidentally overwritten `DOCKER_IMAGE` in `docker-build-linux-generals.sh`.
- Symptom: The build script had invalid shell structure near the variable declarations, and smoke/help output still suggested nonexistent commands under `scripts/` root.
- Fix: Restored the missing `DOCKER_IMAGE` assignment, updated smoke/build image usage text to the current `scripts/build/linux/...` and `scripts/env/docker/...` paths, and corrected the base bundle extraction hint to reference `Generals/` data.
- Prevention: After path migrations, always run grep for old command strings and re-open the edited file header to catch malformed variable blocks early.

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
