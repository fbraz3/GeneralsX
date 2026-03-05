# AI Coding Agent Quickstart

## Big Picture
- Two game targets: `GeneralsMD/` (Zero Hour, primary) and `Generals/` (base game); shared runtime lives in `Core/`.
- Top-level build wires dependencies and targets in `CMakeLists.txt` (`Core/`, then `GeneralsMD/` / `Generals/`).
- Architecture direction is one cross-platform codebase: SDL3 + DXVK + OpenAL + 64-bit; legacy VC6/win32 path is kept for retail compatibility.
- Keep platform work isolated to `Core/GameEngineDevice/`, `Core/Libraries/Source/Platform/`, and `GeneralsMD/Code/CompatLib/`.

## Entry Points & Integration Boundaries
- Game launch entry points: `GeneralsMD/Code/Main/WinMain.cpp` and `Generals/Code/Main/WinMain.cpp`.
- Device/backend integration is centered in `Core/GameEngineDevice/Source/` and `cmake/dx8.cmake`, `cmake/sdl3.cmake`, `cmake/openal.cmake`.
- Build presets and feature toggles are authoritative in `CMakePresets.json` (not in ad-hoc scripts).
- Reference implementations: `references/fighter19-dxvk-port/` (DXVK/SDL3 patterns), `references/jmarshall-win64-modern/` (OpenAL/64-bit patterns), `references/thesuperhackers-main/` (upstream behavior).

## Critical Workflows (Use These First)
- VS Code tasks in `.vscode/tasks.json` are the canonical daily workflow (configure/build/deploy/run per platform).
- Linux Docker: `./scripts/docker-configure-linux.sh linux64-deploy` then `./scripts/docker-build-linux-zh.sh linux64-deploy`.
- macOS native: `./scripts/build-macos-zh.sh`, deploy with `./scripts/deploy-macos-zh.sh`, run with `./scripts/run-macos-zh.sh -win`.
- Windows modern: use `win64-modern` (`scripts/configure_cmake_modern.ps1`, `scripts/build_zh_modern.ps1`, `scripts/deploy_zh_modern.ps1`, `scripts/run_zh_modern.ps1`).
- Replay compatibility check (Windows retail behavior): `generalszh.exe -jobs 4 -headless -replay subfolder/*.rep` with optimized VC6 and `RTS_BUILD_OPTION_DEBUG=OFF`.

## Project-Specific Conventions
- Every user-facing code change must include: `// GeneralsX @keyword author DD/MM/YYYY Description`.
- Allowed keywords: `@bugfix`, `@feature`, `@performance`, `@refactor`, `@tweak`, `@build`.
- Preserve determinism: rendering/audio/platform changes must not alter game logic outcomes.
- Prefer minimal, isolated diffs; do not refactor gameplay code while doing platform/backend changes.

## Pitfalls That Recur Here
- SDL3 is cross-platform: do not wrap SDL3 implementation files with `#ifndef _WIN32`.
- Use platform guards precisely: macOS-only fixes should use `__APPLE__` (not broad `_WIN32`/else patterns that can break Linux).
- CMake options are not C++ defines unless exported via `target_compile_definitions`.
- DXVK type conflicts after clean reconfigure are handled via pre-guards in `GeneralsMD/Code/CompatLib/Include/windows_compat.h` (`_MEMORYSTATUS_DEFINED`, `_IUNKNOWN_DEFINED`).
- Linux include paths are case-sensitive; check/fix case mismatches (`scripts/cpp/fixIncludesCase.sh`).

## Documentation & Session Hygiene
- Write docs in English only.
- Dev diary is monthly: `docs/DEV_BLOG/YYYY-MM-DIARY.md` (newest entries first).
- Put active technical notes in `docs/WORKDIR/` (`phases/`, `reports/`, `support/`, `lessons/`), not directly under `docs/`.
- Before implementing complex fixes, consult `docs/WORKDIR/lessons/` and relevant `docs/WORKDIR/phases/PHASE*.md`.

## Backport Policy
- Zero Hour first; backport to `Generals/` only for shared backend/platform changes with low gameplay risk.
- Do not backport expansion-specific gameplay logic.
