# PHASE01: Audit and Baseline (Win32 → SDL2)

## Objective
Build a precise inventory of Win32-only dependencies in the shipping runtime path (GeneralsXZH) and define the exact replacement targets using SDL2.

## Scope
- Shipping runtime only (GeneralsXZH / core engine runtime code paths).
- Exclude tooling where it does not block the game running.
- Exclude reference submodules from the audit results.

## Deliverables
- A curated list of Win32 APIs used by runtime, grouped by subsystem.
- A list of existing abstraction points to hook SDL2 into (entry point, device layer, filesystem, logging, config).
- A minimal “SDL2 device layer” directory skeleton plan (names, files, ownership boundaries).

## Checklist
- [x] Identify all runtime entry points and platform selection logic (Windows vs non-Windows).
  - Entry: `GeneralsMD/Code/Main/WinMain.cpp` (APIENTRY WinMain)
  - Engine: `Win32GameEngine` class (platform-specific subclass)
  - Device: `Win32Device/` directory structure
- [x] Enumerate Win32 window/message pump usage and where it feeds into engine.
  - Window creation: `initializeAppWindows()` with `CreateWindow`, `RegisterClass`
  - Message pump: `Win32GameEngine::serviceWindowsOS()` with `PeekMessage/GetMessage/TranslateMessage/DispatchMessage`
  - Message handler: `WndProc()` callback in WinMain.cpp
- [x] Enumerate input dependencies (mouse, keyboard, IME) in runtime.
  - Mouse: `Win32Mouse::addWin32Event()` receives WM_LBUTTONDOWN/UP, WM_MOUSEMOVE, WM_MOUSEWHEEL
  - Keyboard: `WM_KEYDOWN` handling (VK_ESCAPE for quit)
  - IME: `TheIMEManager->serviceIMEMessage()` for WM_IME_* messages
- [x] Enumerate timing/sleep usage in runtime.
  - `Sleep(5)` in alt-tab handling (Win32GameEngine::update)
  - `Sleep(100)`, `Sleep(1)` in game client initialization and loading screens
  - `GetTickCount()` for double-click detection
- [x] Enumerate display/fullscreen management usage in runtime.
  - Window styles: `WS_POPUP|WS_VISIBLE` + window/fullscreen flags
  - Positioning: `GetSystemMetrics()`, `SetWindowPos()`, `HWND_TOPMOST`
  - Fullscreen restrictions: `WM_SYSCOMMAND` filtering, menu prevention
- [x] Enumerate registry usage in runtime (and decide replacements).
  - Limited direct registry usage; `GetRegistryLanguage()` for localization
  - Already using INI files via `TheFileSystem` abstraction
  - Configuration via `assets/ini/GeneralsXZH.ini`
- [x] Enumerate OS-integration usage (URL launch, clipboard, dialogs) in runtime.
  - Power management: `WM_POWERBROADCAST` (suspend/resume)
  - Window activation: `SetForegroundWindow()`, `ShowWindow(SW_RESTORE)`
  - Load screen: `LoadImage()` with `LR_LOADFROMFILE`
  - Error handling: `SetErrorMode()`, `SetUnhandledExceptionFilter()`
- [x] Document the "single-instance" mechanism and whether it needs SDL2-compatible replacement.
  - Already cross-platform! Uses `rts::ClientInstance` with:
    - Windows: `CreateMutex()` / `CloseHandle()`
    - Non-Windows: File-based locking via `open()` + `flock()`
- [x] Produce an initial integration diagram: Entry → Engine → Device → OS bridge.
  - Included in audit findings document with detailed flow diagrams

## Exit Criteria
- The audit list is complete enough that PHASE02 can be executed without additional "surprise" Win32 APIs.
  - **STATUS: ✅ ACHIEVED**
  - All Win32 APIs cataloged in summary table
  - SDL2 equivalents identified
  - Integration architecture documented
  - No critical dependencies missed
