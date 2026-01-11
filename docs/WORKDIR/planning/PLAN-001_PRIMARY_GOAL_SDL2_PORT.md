# PLAN-001: Primary Goal — Win32 → SDL2 Port (No Graphics Backend Changes)

## Goal
Make the game run natively on Windows, macOS, and Linux by replacing Win32-only OS/window/input/etc. calls with SDL2 equivalents, without changing the rendering backend implementation.

## Non-Goals (for this plan)
- No DirectX → Vulkan translation work (that is a later goal).
- No large gameplay refactors.
- No broad rewrites of tools (only the game runtime path that affects shipping targets).

## Success Criteria (Acceptance)
The following must work on macOS/Linux/Windows builds:
1. Load game assets (.big, textures, models, sounds).
2. Show initial screen and main menu.
3. Navigate menus.
4. Start and play a skirmish (single-player vs AI) successfully.
5. Play Campaign missions successfully.
6. Play Generals Challenge successfully.

## Current Reality (Baseline)
The main Zero Hour entry point is Win32-specific:
- It includes `<windows.h>` and uses `WinMain`, `WNDCLASS`, `CreateWindow`, `WndProc`, `PeekMessage/GetMessage/DispatchMessage`, etc.
- The engine device layer uses Win32-centric types/events (e.g., feeding WM_* mouse messages into `Win32Mouse`).

Key files to anchor the port:
- Game entry point: GeneralsMD/Code/Main/WinMain.cpp
- Win32 game engine message pump: GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp
- Win32 input: GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/* (mouse/keyboard/IME)
- Win32 display + OS services: GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/*

Reference implementation ideas exist in the submodule:
- references/fighter19-dxvk-port/GeneralsMD/Code/Main/SDL3Main.cpp
- references/fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/*

## Strategy (Keep Changes Centralized)
1. **Introduce an SDL2-based platform device layer** (parallel to Win32Device), used on macOS/Linux first.
2. **Minimize invasiveness** by translating SDL events into existing engine-level input flows at a single boundary layer.
3. **Prefer shims and compat wrappers** in the appropriate compatibility locations for shared code, rather than sprinkling platform `#ifdef`s across gameplay.
4. Keep Windows build working during transition (Win32Device remains supported until SDL2 path is fully stable).

## Subsystems to Port (Win32 → SDL2)
### A) Application entry + window lifecycle
- Replace `WinMain` + `CreateWindow/WndProc` with an SDL2-driven `main` (or SDL2 main wrapper) on non-Windows targets.
- Map window events:
  - focus gained/lost
  - minimize/restore
  - resize/move
  - close requests

### B) Event pump
- Replace `PeekMessage/GetMessage/TranslateMessage/DispatchMessage` with `SDL_PollEvent` loop.
- Preserve game loop cadence: poll events once per frame (or as currently done by `GameEngine::update()` paths).

### C) Mouse input
- Replace WM_* mouse event injection (`Win32Mouse::addWin32Event`) with SDL mouse events.
- Cursor capture/relative mode:
  - windowed: obey `Mouse::refreshCursorCapture` semantics
  - fullscreen: prevent cursor escape and maintain correct warping behavior

### D) Keyboard input
- Replace WM_KEY* / `GetAsyncKeyState` usage (game runtime) with SDL keyboard events/state.
- Preserve edge semantics used in the game ("pressed since last call" patterns).

### E) IME / text input
- Replace Win32 IME message handling with SDL2 text input APIs.
- Preserve existing `IMEManager` surface and composition flow.

### F) Timers / sleep
- Replace `Sleep`, `timeGetTime`, etc. with SDL timing equivalents.
- Ensure minimized/alt-tab behavior does not busy-loop and does not starve networking updates.

### G) Display enumeration & fullscreen
- Replace Win32 display queries with SDL display APIs.
- Preserve windowed/fullscreen toggles and resolution behavior.

### H) OS integration
- URL launching, clipboard, message boxes (where applicable) via SDL2 or existing cross-platform helpers.

### I) Configuration (registry removal)
- Ensure runtime does not depend on Windows registry.
- Use INI/config-based storage as described by assets/ini/README.md.

### J) Crash handling + diagnostics
- Keep crash dumping on supported platforms (or degrade gracefully where unavailable).
- Ensure logs are written to the cross-platform log path.

## Phased Execution
This plan is executed via phase checklists in docs/WORKDIR/phases:
- PHASE01_AUDIT_AND_BASELINE.md
- PHASE02_SDL2_APP_AND_EVENT_PUMP.md
- PHASE03_SDL2_INPUT_AND_IME.md
- PHASE04_CONFIG_FILESYSTEM_OS_SERVICES.md
- PHASE05_STABILITY_PERF_AND_GAMEPLAY_VALIDATION.md

## Engineering Guardrails
- Avoid refactoring gameplay logic; constrain platform work to the device/platform layers.
- Avoid adding platform code directly into rendering wrappers (DX8/Vulkan work is secondary goal).
- Keep paths and file access going through `TheFileSystem` abstractions.
- Track regressions by validating the acceptance criteria after each phase.

## Risks / Gotchas
- The current input pipeline is Win32 message-shaped (WM_*). Avoid spreading translation logic; keep it at one boundary.
- IME behavior varies a lot across platforms; implement feature-parity targets, then harden.
- Fullscreen + focus handling can cause deadlocks/pauses if not mapped cleanly to SDL events.
- Tooling code under Core/Tools is heavily Win32-specific; it should not block the shipping runtime path.
