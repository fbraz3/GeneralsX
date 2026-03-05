---
applyTo: '**'
---

# GeneralsX Agent Instructions (Lean)

## Mission and Scope

GeneralsX ports Command & Conquer: Generals Zero Hour to a single cross-platform codebase:
- SDL3 (windowing/input)
- DXVK (DirectX 8 -> Vulkan)
- OpenAL (audio)
- 64-bit targets

Primary target is `GeneralsMD/` (Zero Hour). `Generals/` is secondary and should receive only low-risk shared backend/platform backports.

This is legacy code (20+ years old). Prioritize minimal diffs, determinism, and retail compatibility.

## Non-Negotiables

1. Single codebase across Linux, macOS, and Windows modern stack.
2. No platform hacks inside gameplay logic.
3. Preserve retail behavior and replay compatibility.
4. Keep legacy VC6/win32 paths working unless the task explicitly says otherwise.
5. Do not remove systems (audio/video/features) without explicit user decision.

## Architecture Boundaries

- Game targets: `GeneralsMD/`, `Generals/`
- Shared runtime: `Core/`
- Platform/backend boundary (allowed area for platform work):
  - `Core/GameEngineDevice/`
  - `Core/Libraries/Source/Platform/`
  - `GeneralsMD/Code/CompatLib/`

Entry points:
- `GeneralsMD/Code/Main/WinMain.cpp`
- `Generals/Code/Main/WinMain.cpp`

Build/preset authority:
- `CMakePresets.json`
- `cmake/dx8.cmake`, `cmake/sdl3.cmake`, `cmake/openal.cmake`

## Canonical Workflows

Prefer VS Code tasks in `.vscode/tasks.json` for daily build/deploy/run/debug.

### Linux (Docker)
```bash
./scripts/docker-configure-linux.sh linux64-deploy
./scripts/docker-build-linux-zh.sh linux64-deploy
./scripts/deploy-linux-zh.sh
./scripts/run-linux-zh.sh -win
```

### macOS (Native)
```bash
./scripts/build-macos-zh.sh
./scripts/deploy-macos-zh.sh
./scripts/run-macos-zh.sh -win
```

### Windows Modern
```powershell
.\scripts\configure_cmake_modern.ps1
.\scripts\build_zh_modern.ps1
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern
.\scripts\run_zh_modern.ps1
```

### Replay Compatibility (Retail Behavior)
Use optimized VC6 with `RTS_BUILD_OPTION_DEBUG=OFF`:
```bat
generalszh.exe -jobs 4 -headless -replay subfolder/*.rep
```

## Presets That Matter

Legacy baseline:
- `vc6` (retail compatibility reference)
- `win32` (upstream modernization path)

Cross-platform modern stack:
- `linux64-deploy`
- `macos-vulkan`
- `win64-modern`
- `win64-modern-debug`

Cross-compile utility:
- `mingw-w64-i686`

## Project Conventions

- Every user-facing code change must include:
  - `// GeneralsX @keyword author DD/MM/YYYY Description`
- Keywords: `@bugfix`, `@feature`, `@performance`, `@refactor`, `@tweak`, `@build`
- Use English for code/comments/docs.
- Keep changes small and localized; avoid opportunistic refactors.

## Known Pitfalls (High Value)

1. SDL3 is cross-platform. Do not wrap SDL3 implementation files with `#ifndef _WIN32`.
2. Use precise platform guards:
   - macOS-only: `__APPLE__`
   - Linux-only: `__linux__`
   - POSIX-wide: `#ifndef _WIN32`
3. CMake options are not C++ defines unless exported via `target_compile_definitions`.
4. Linux includes are case-sensitive; use `scripts/cpp/fixIncludesCase.sh` when needed.

## DXVK Header Conflict Rule (Critical)

`windows_compat.h` handles DXVK redefinition conflicts via pre-guards before `windows_base.h`.

Location:
- `GeneralsMD/Code/CompatLib/Include/windows_compat.h`

Guards to preserve:
- `_MEMORYSTATUS_DEFINED`
- `_IUNKNOWN_DEFINED`

If those are removed or reordered, clean reconfigure can reintroduce `MEMORYSTATUS`/`IUnknown` conflicts.

## Documentation Hygiene

- Dev diary only in `docs/DEV_BLOG/YYYY-MM-DIARY.md` (newest entries first).
- Active technical work goes in `docs/WORKDIR/` (`phases/`, `planning/`, `reports/`, `support/`, `audit/`, `lessons/`).
- Do not place active docs directly in `docs/` root.
- Consult `docs/WORKDIR/lessons/` before complex fixes.

## Backport Policy

Backport Zero Hour -> Generals only when all are true:
1. Change is backend/platform/shared (`Core` or platform layer).
2. Change is low-risk for gameplay behavior.
3. No expansion-specific logic is involved.

## Reference Repositories (Use for Patterns, Not Copy-Paste)

- `references/fighter19-dxvk-port/` -> DXVK + SDL3 integration patterns.
- `references/jmarshall-win64-modern/` -> OpenAL and 64-bit modernization patterns.
- `references/thesuperhackers-main/` -> upstream behavior baseline.

Before implementing non-trivial backend work:
1. Check relevant `docs/WORKDIR/phases/` file.
2. Check `docs/WORKDIR/lessons/` for prior failures/patterns.
3. Validate on affected presets/tasks.
