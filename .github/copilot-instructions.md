# GeneralsX — Copilot Instructions

## What this repo is
GeneralsX is a cross-platform port of Command & Conquer: Generals / Zero Hour: legacy Win32 + DirectX 8-era code modernized to C++20 with SDL2 and a Vulkan-first renderer (see `CMakePresets.json`).

## Where to make changes (important)
- Prefer adding shims instead of rewriting game logic to “remove Windows”: start in `Dependencies/Utility/Compat/` (e.g., `d3d8.h`) and `Core/Libraries/Source/WWVegas/WW3D2/win32_*_compat.h`.
- Most rendering state flows through `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h` / `DX8Wrapper::*` (e.g., render-state/texture-stage calls); keep changes centralized there.
- Prefer engine-wide safety fixes in `Core/GameEngine/Source/Common/System/GameMemory.cpp` over scattered call-site checks.
- Prefer `TheFileSystem` abstractions (not raw `fopen`) for cross-platform file access.

## Build + run (macOS primary)
- Configure: `cmake --preset macos` (Vulkan is enabled and exported via preset env vars).
- Build (primary target): `cmake --build build/macos --target GeneralsXZH -j 4 2>&1 | tee logs/build.log`
- Deploy helper: `scripts/build_zh.sh` (builds + copies to `$HOME/GeneralsX/GeneralsMD`).
- Run/debug via external Terminal.app wrappers:
  - Run: `scripts/runTerminal.sh "$HOME/GeneralsX/GeneralsMD/GeneralsXZH -win 2>&1 | tee logs/runTerminal.log"`
  - Debug: `scripts/runTerminal.sh "scripts/lldb_debug.sh $HOME/GeneralsX/GeneralsMD/GeneralsXZH 2>&1 | tee logs/debugTerminal.log"`
  - Note: `scripts/runTerminal.sh` enforces a 60s `timeout -s 9` to avoid fullscreen lockups.

## Assets + configuration
- The runtime expects original `.big` assets adjacent to the deployed executable (e.g., `$HOME/GeneralsX/GeneralsMD/Data/`), so deployment location matters.
- User config is INI-based (registry replaced): examples in `assets/ini/` and stored under `~/.config/` on macOS/Linux (see `assets/ini/README.md`).

## Conventions
- Prefer `GeneralsXZH` (Zero Hour) as the main stable target; only backport to `GeneralsX` when the change is verified.
- Don’t hardcode absolute user paths; use `$HOME`.
- When adding docs, follow the repo’s doc layout rules in `.github/instructions/docs.instructions.md`.
