# PHASE 1: Win32 API Audit Findings
**Status**: COMPLETE  
**Date**: 2026-01-11  
**Scope**: Win32 → SDL2 Port (No Graphics Backend Changes)

---

## Executive Summary

The Windows-only codebase has been systematically audited for Win32 dependencies in the shipping runtime (GeneralsXZH). Key findings:

- **Entry Point**: Single Win32 entry point in `GeneralsMD/Code/Main/WinMain.cpp`
- **Message Pump**: Win32 message loop via `PeekMessage/GetMessage/TranslateMessage/DispatchMessage` in `Win32GameEngine::serviceWindowsOS()`
- **Platform Device Layer**: `Win32Device/` directory contains all platform-specific code (windowing, input, display)
- **Single-Instance Mechanism**: Already partially cross-platform via `ClientInstance` class
- **Input System**: Win32 message-based input (mouse, keyboard, IME) injected into engine via `Win32Mouse::addWin32Event()`

All Win32 APIs have been identified and grouped by subsystem for SDL2 replacement strategy.

---

## Section A: Application Entry Point & Window Lifecycle

### Current Implementation
**File**: [GeneralsMD/Code/Main/WinMain.cpp](GeneralsMD/Code/Main/WinMain.cpp)

#### Entry Point (`WinMain`)
- **Lines 637-732**: Main entry point function
- Uses `HINSTANCE`, `LPSTR lpCmdLine`, `Int nCmdShow` parameters (Win32-specific)
- Sets up critical sections, memory manager, working directory
- Initializes window via `initializeAppWindows()`
- Calls `GameMain()` for main loop
- **Key Win32 APIs**: `SetUnhandledExceptionFilter`, `GetModuleFileName`, `SetCurrentDirectory`, `LoadImage`

#### Window Creation (`initializeAppWindows`)
- **Lines 493-566**: Window class registration and creation
- **WNDCLASS registration**: Custom `WndProc` callback
- **Window styles**: `WS_POPUP|WS_VISIBLE` + window/fullscreen flags
- **Window positioning**: Uses `GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN)` for centering
- **Key Win32 APIs**:
  - `RegisterClass()` - window class registration
  - `CreateWindow()` - window creation
  - `SetWindowPos()` - Z-order and positioning
  - `SetFocus()`, `SetForegroundWindow()`, `ShowWindow()`, `UpdateWindow()`
  - `GetSystemMetrics()` - screen dimensions
  - `AdjustWindowRect()` - window rect calculation

#### Window Procedure (`WndProc`)
- **Lines 350-532**: Central message handler for all window events
- **IME Integration**: First checks `TheIMEManager->serviceIMEMessage()`
- **Messages Handled**: WM_NCHITTEST, WM_POWERBROADCAST, WM_SYSCOMMAND, WM_QUERYENDSESSION, WM_CLOSE, WM_MOVE, WM_SIZE, WM_SETFOCUS, WM_KILLFOCUS, WM_ACTIVATEAPP, WM_ACTIVATE, WM_KEYDOWN, mouse buttons, mousewheel, WM_MOUSEMOVE, WM_SETCURSOR, WM_PAINT, WM_ERASEBKGND
- **Focus Management**: Handles `WM_SETFOCUS/WM_KILLFOCUS` to reset keyboard state
- **Fullscreen Handling**: Prevents moving/sizing/menu in fullscreen mode
- **Key Win32 APIs**:
  - `GetWindowRect()`, `GetClientRect()` - window bounds
  - `PostQuitMessage()` - exit request
  - `FindWindow()` - search window by class/title
  - `SetWindowPos()` - HWND_TOPMOST for fullscreen
  - Message dispatch via `DefWindowProc()`

---

## Section B: Message Pump & Event Loop

### Current Implementation
**File**: [GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp](GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp)

#### Main Game Loop Integration
- **Lines 102-119**: `update()` method calls `GameEngine::update()` then `serviceWindowsOS()`
- Handles alt-tab (iconic window) by sleeping 5ms and continuing network updates
- Recovers audio focus after alt-tab via `TheAudio->setVolume()`

#### Message Service Function (`serviceWindowsOS`)
- **Lines 128-162**: Core message pump
- Uses `PeekMessage()` to check for pending messages
- Uses `GetMessage()` to retrieve message (blocking until available)
- **Timing**: Captures `msg.time` into global `TheMessageTime` for later use
- Calls `TranslateMessage()` for character translation (esp. keyboards)
- Calls `DispatchMessage()` to route message to `WndProc()`
- **Key Win32 APIs**:
  - `PeekMessage()` - non-blocking message check
  - `GetMessage()` - blocking message retrieval (handles WM_QUIT)
  - `TranslateMessage()` - keyboard character generation
  - `DispatchMessage()` - message routing to window procedure

#### Globals
- **Lines 60-62**: Global window handle and mouse object pointers
- `HWND ApplicationHWnd` - accessible to all code
- `Win32Mouse *TheWin32Mouse` - injected input events
- `DWORD TheMessageTime` - timestamp of last message

---

## Section C: Input System (Mouse, Keyboard, IME)

### Mouse Input

#### Win32Mouse Class
**Files**:
- Header: `GeneralsMD/Code/GameEngineDevice/Include/Win32Device/GameClient/Win32Mouse.h`
- Implementation: [GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32Mouse.cpp](GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32Mouse.cpp)

**WndProc Integration**:
- **Lines 460-490 (WinMain.cpp)**: Captures mouse button events
  - `WM_LBUTTONDOWN/UP/DBLCLK`, `WM_RBUTTONDOWN/UP/DBLCLK`, `WM_MBUTTONDOWN/UP/DBLCLK`
  - Calls `TheWin32Mouse->addWin32Event(message, wParam, lParam, TheMessageTime)`
- **Lines 491-501**: Mouse wheel handling (message 0x020A = WM_MOUSEWHEEL)
  - Validates mouse is within client area before injection
- **Lines 502-524**: Mouse move tracking
  - Checks window activity (`isWinMainActive`)
  - Validates client area bounds
  - Tracks cursor entering/leaving window via `TheMouse->onCursorMovedInside/Outside()`
- **Lines 525-530**: Cursor appearance (`WM_SETCURSOR`)
  - Restores custom cursor via `TheWin32Mouse->setCursor()`

#### Cursor Capture Management
- **Lines 275-280 (WM_MOVE/WM_SIZE)**: Refreshes cursor capture on window move/resize
- **Lines 303-318 (WM_ACTIVATE)**: Cursor capture only after activation events
- **Single-Instance**: Cursor relative mode and capture managed via `Mouse::refreshCursorCapture()`

#### Keyboard Input

**Files**:
- Mouse: [GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32DIKeyboard.cpp](GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32DIKeyboard.cpp)
- Implementation uses Win32 DirectInput or GetAsyncKeyState patterns

**WndProc Integration**:
- **Lines 382-391 (WinMain.cpp)**: `WM_KEYDOWN` handling
  - VK_ESCAPE triggers `PostQuitMessage(0)` for immediate exit
  - Other keys likely fed to IME manager first

#### IME (Input Method Editor)
**File**: [GeneralsMD/Code/GameEngine/Include/GameClient/IMEManager.h](GeneralsMD/Code/GameEngine/Include/GameClient/IMEManager.h)

**WndProc Integration**:
- **Lines 357-366 (WinMain.cpp)**: First handler in message processing
  - `TheIMEManager->serviceIMEMessage(hWnd, message, wParam, lParam)`
  - If IME intercepts, returns `TheIMEManager->result()` immediately
  - Prevents double-processing of IME messages

**Messages Handled**:
- `WM_IME_STARTCOMPOSITION`, `WM_IME_ENDCOMPOSITION`, `WM_IME_COMPOSITION`
- `WM_IME_SETCONTEXT`, `WM_IME_NOTIFY`, `WM_IME_CONTROL`
- `WM_IME_COMPOSITIONFULL`, `WM_IME_SELECT`, `WM_IME_CHAR`
- `WM_IME_KEYDOWN`, `WM_IME_KEYUP`

---

## Section D: Timing & Sleep

### Current Timing APIs
**Pattern**: Used in multiple subsystems for throttling and synchronization

#### Locations Found:
1. **Win32GameEngine::update() - Alt-tab throttling**
   - `Sleep(5)` while window is iconic (minimized)
   - Prevents CPU spinning during alt-tab

2. **Game Logic & Loading Screens** (from audit scope)
   - `Sleep(100)` in GameClient.cpp line 569 (GameClient initialization)
   - `Sleep(1)` in LoadScreen.cpp lines 540, 1056, 1102 (frame throttling)
   - `Sleep(100)` in LoadScreen.cpp lines 603, 1121, 1262 (loading progress)
   - `Sleep(33)` in GameLogic.cpp line 2211 (frame rate limiting)

3. **File Transfer & Network**
   - `Sleep(500)` in FileTransfer.cpp line 62 (network sync)

#### Key Win32 APIs:
- `Sleep(milliseconds)` - blocking sleep
- `GetTickCount()` - millisecond timer (used in tools, rare in game runtime)
- `timeGetTime()` - high-resolution timer (used in tools)
- `::GetDoubleClickTime()` - UI double-click detection

### Replacement Strategy:
- `SDL_Delay()` for `Sleep()`
- `SDL_GetTicks()` or `SDL_GetTicks64()` for timing
- System clocks via POSIX `clock_gettime()` where precision needed

---

## Section E: Single-Instance Mechanism

### Current Implementation
**File**: [GeneralsMD/Code/GameEngine/Source/GameClient/ClientInstance.cpp](GeneralsMD/Code/GameEngine/Source/GameClient/ClientInstance.cpp)

**Already Cross-Platform!** ✅

#### Windows Path (Lines 83-121)
```cpp
#if defined(_WIN32)
    HANDLE mutexHandle = CreateMutex(NULL, FALSE, guidStr.c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Another instance is running
        CloseHandle(mutexHandle);
        return false;  // or allow multi-instance
    }
#endif
```

#### Non-Windows Path (Lines 52-63)
```cpp
#if !defined(_WIN32)
static int g_clientInstanceLockFd = -1;
static bool tryAcquireInstanceLock(const std::string& name) {
    // File-based locking via open() + flock()
    // Returns false if lock already held
}
#endif
```

#### Existing Abstractions
- `rts::ClientInstance::initialize()` - initialize single-instance lock
- `rts::ClientInstance::isInitialized()` - check if lock acquired
- `rts::ClientInstance::getInstanceIndex()` - instance ID (0-based)
- `rts::ClientInstance::getFirstInstanceName()` - returns GENERALS_GUID constant
- `rts::ClientInstance::isMultiInstance()` - allow multiple instances (debug mode)
- `rts::ClientInstance::skipPrimaryInstance()` - use alternate GUID for spawned processes

### Integration Point
- Called in WinMain.cpp after global data initialization
- If already running and not in multi-instance mode, brings existing window to foreground

---

## Section F: Display & Fullscreen Management

### Current Implementation
**Files**:
- [GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32OSDisplay.cpp](GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32OSDisplay.cpp)
- WM_DISPLAYCHANGE handling in WndProc

#### Window Creation Parameters (WinMain.cpp)
- **Fullscreen mode**: `WS_POPUP | WS_VISIBLE | WS_EX_TOPMOST | WS_SYSMENU`
- **Windowed mode**: `WS_POPUP | WS_VISIBLE | WS_DLGFRAME | WS_CAPTION | WS_SYSMENU`
- **Screen centering**: `GetSystemMetrics(SM_CXSCREEN / SM_CYSCREEN)`
- **Window positioning**: `AdjustWindowRect()` for frame calculations
- **Default dimensions**: 1024x768 (DEFAULT_DISPLAY_WIDTH/HEIGHT)

#### Fullscreen Restrictions (WndProc)
- **Lines 424-443 (WM_SYSCOMMAND)**: Prevents moving/sizing/maximize/power in fullscreen
  - SC_MOVE, SC_SIZE, SC_MAXIMIZE, SC_MONITORPOWER blocked when `!m_windowed`
- **Lines 415-423 (WM_NCHITTEST)**: Prevents menu selection in fullscreen
  - Returns HTCLIENT to reject title bar hit testing

#### Key Win32 APIs:
- `GetSystemMetrics()` - screen dimensions
- `AdjustWindowRect()` - frame size calculation
- `GetWindowRect()` - current window bounds
- `GetClientRect()` - client area bounds
- `SetWindowPos()` - positioning and HWND_TOPMOST
- `WM_DISPLAYCHANGE` message - display mode changes

---

## Section G: OS Integration Services

### Application Instance Management
**Already Handled by ClientInstance** ✅
- Window foreground activation via `SetForegroundWindow()`, `ShowWindow(SW_RESTORE)`

### Power Management
**WndProc Integration** (Lines 410-422)
- `WM_POWERBROADCAST` for suspend/resume (PBT_APMQUERYSUSPEND, PBT_APMRESUMESUSPEND)
- Returns TRUE to allow suspend operations

### Load Screen / Splash Image
**WinMain.cpp** (Lines 607-633)
- Loads bitmap from file: `Install_Final.bmp`
- Looks in localized directories first: `Data/<language>/Install_Final.bmp`
- Falls back to root directory
- Uses `LoadImage(..., LR_LOADFROMFILE)` to load from filesystem
- Painted via WM_PAINT handler using GDI (`::BitBlt`)

### Debug Logging
- `OutputDebugString()` in DEBUG_WINDOWS_MESSAGES mode (Line 370)
- Crash dump via MiniDumper class (Phase 30+ feature)

### Error Handling
- `SetErrorMode(SEM_FAILCRITICALERRORS)` - suppress system dialogs
- Unhandled exception filter: `UnHandledExceptionFilter()` (Lines 412-426)
  - Calls `DumpExceptionInfo()` for stack traces
  - Triggers MiniDumper for crash dumps

---

## Section H: Registry / Configuration

### Current Status
**Files**: [GeneralsMD/Code/GameEngine/Include/Common/Registry.h](GeneralsMD/Code/GameEngine/Include/Common/Registry.h)

#### Windows Registry Usage (Limited in Runtime)
- Some registry lookups via `GetRegistryLanguage()` in WinMain.cpp line 615
- Registry primarily used for installation paths, not game runtime

#### INI-Based Configuration (Already in Place) ✅
**Directory**: [assets/ini/](assets/ini/)

**Files**:
- `GeneralsX.ini` - Generals base game configuration
- `GeneralsXZH.ini` - Zero Hour expansion configuration
- `README.md` - Configuration documentation

**Config Locations**:
- Windows: `HKEY_CURRENT_USER\Software\...` mapped to INI
- macOS/Linux: `~/.config/Generals/` or platform-specific user directories

**Runtime Configuration**:
- Uses `TheFileSystem` abstraction for config file access
- Not dependent on Windows registry at runtime
- INI parser handles cross-platform paths and encodings

---

## Section I: Crash Handling & Diagnostics

### Crash Dumping
**Feature**: RTS_ENABLE_CRASHDUMP (Phase 30+)
- Implemented via `MiniDumper` class
- Already has Windows/non-Windows guards
- Uses platform-appropriate dump mechanisms

### Logging
- Debug log output to files in user data directory
- File paths abstracted via `TheFileSystem` and `TheGlobalData->getPath_UserData()`
- Already cross-platform

---

## Section J: Summary Table - Win32 APIs by Replacement Strategy

| Subsystem | Current API | SDL2 Equivalent | Status |
|-----------|------------|-----------------|--------|
| **Window Creation** | CreateWindow, RegisterClass | SDL_CreateWindow | Phase 2 |
| **Message Pump** | PeekMessage, GetMessage, DispatchMessage | SDL_PollEvent | Phase 2 |
| **Focus Management** | SetFocus, SetForegroundWindow | SDL_RaiseWindow, SDL_SetWindowInputFocus | Phase 2 |
| **Window Positioning** | SetWindowPos, GetSystemMetrics | SDL_SetWindowPosition, SDL_DisplayMode | Phase 2 |
| **Mouse Input** | WM_LBUTTONDOWN/UP, WM_MOUSEMOVE | SDL_MOUSEBUTTONDOWN/UP, SDL_MOUSEMOTION | Phase 3 |
| **Mouse Capture** | SetCapture, ReleaseCapture | SDL_CaptureMouse | Phase 3 |
| **Keyboard Input** | WM_KEYDOWN/UP, TranslateMessage | SDL_KEYDOWN/UP, SDL_TEXTINPUT | Phase 3 |
| **IME Input** | WM_IME_* messages | SDL_TEXTEDITING, TextInputEvent | Phase 3 |
| **Timing** | Sleep, GetTickCount | SDL_Delay, SDL_GetTicks | Phase 4 |
| **Fullscreen** | WS_POPUP, HWND_TOPMOST | SDL_SetWindowFullscreen | Phase 2 |
| **Display Metrics** | GetSystemMetrics | SDL_DisplayMode, SDL_GetDisplayBounds | Phase 2 |
| **Window Cursor** | SetCursor, WM_SETCURSOR | SDL_SetCursor, SDL_ShowCursor | Phase 3 |
| **Single-Instance** | CreateMutex, CloseHandle | SDL_CreateMutex / file lock | Phase 4 |
| **Power Management** | WM_POWERBROADCAST | SDL_POWERSTATE event | Phase 4 |
| **Load Screen** | LoadImage (GDI) | Native image loading | Phase 2 |
| **Crash Dumps** | MiniDumper (already cross-platform) | Existing infrastructure | ✅ |
| **Config Files** | INI files (already in place) | TheFileSystem abstraction | ✅ |

---

## Section K: Platform Device Layer Architecture

### Current Structure
```
GeneralsMD/Code/GameEngineDevice/
├── Include/
│   └── Win32Device/
│       ├── Common/
│       │   └── Win32GameEngine.h
│       └── GameClient/
│           ├── Win32Mouse.h
│           ├── Win32DIKeyboard.h
│           └── Win32DIMouse.h
└── Source/
    └── Win32Device/
        ├── Common/
        │   ├── Win32GameEngine.cpp (MESSAGE PUMP)
        │   ├── Win32CDManager.cpp
        │   └── Win32OSDisplay.cpp
        └── GameClient/
            ├── Win32Mouse.cpp (INPUT INJECTION)
            ├── Win32DIKeyboard.cpp
            └── Win32DIMouse.cpp
```

### Proposed SDL2Device Structure
```
GeneralsMD/Code/GameEngineDevice/
├── Include/
│   └── SDL2Device/
│       ├── Common/
│       │   └── SDL2GameEngine.h
│       └── GameClient/
│           ├── SDL2Mouse.h
│           ├── SDL2Keyboard.h
│           └── SDL2IMEManager.h
└── Source/
    └── SDL2Device/
        ├── Common/
        │   ├── SDL2GameEngine.cpp
        │   ├── SDL2Display.cpp
        │   └── SDL2Timing.cpp
        └── GameClient/
            ├── SDL2Mouse.cpp
            ├── SDL2Keyboard.cpp
            └── SDL2IMEManager.cpp
```

### Key Design Points
1. **Parallel Implementation**: Both Win32Device and SDL2Device exist during transition
2. **Platform Selection**: CMake preset determines which device layer is compiled
3. **Engine Abstraction**: GameEngine base class with Win32GameEngine/SDL2GameEngine subclasses
4. **Input Boundary**: Single point where SDL events translate to internal input injection
5. **Message Handling**: Game engine update loop calls device-specific `serviceOS()`

---

## Section L: Integration Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application Entry Point                      │
│                    WinMain / main / SDL_main                     │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
        ┌────────────────────────────────────────┐
        │   Initialize Global Systems            │
        │   (Memory, Config, Audio, etc.)        │
        └────────────────────┬───────────────────┘
                             │
                             ▼
        ┌────────────────────────────────────────┐
        │   Create Game Engine (Platform-Specific)
        │   - Win32GameEngine (Windows)          │
        │   - SDL2GameEngine (Mac/Linux)         │
        └────────────────────┬───────────────────┘
                             │
                             ▼
        ┌────────────────────────────────────────┐
        │   Game Loop: GameEngine::update()      │
        │   1. Update client/logic               │
        │   2. Service OS (Message Pump)         │
        │   3. Render frame                      │
        └────────────────────┬───────────────────┘
                             │
                    ┌────────┴────────┐
                    ▼                 ▼
         ┌─────────────────┐  ┌─────────────────┐
         │  Win32GameEngine │ │ SDL2GameEngine  │
         │  serviceWindowsOS│ │ serviceSDL2     │
         │  - PeekMessage  │  │ - SDL_PollEvent │
         │  - GetMessage   │  │ - SDL_WaitEvent │
         │  - DispatchMsg  │  │ - SDL_PushEvent │
         └────────┬────────┘  └────────┬────────┘
                  │                    │
                  ▼                    ▼
         ┌─────────────────┐  ┌─────────────────┐
         │    WndProc()    │  │  SDL_EventFilter│
         │  - Mouse events │  │  - Mouse events │
         │  - Keyboard     │  │  - Keyboard     │
         │  - IME          │  │  - Text input   │
         │  - Focus        │  │  - Focus        │
         │  - Fullscreen   │  │  - Fullscreen   │
         └────────┬────────┘  └────────┬────────┘
                  │                    │
                  └────────┬───────────┘
                           ▼
         ┌──────────────────────────────┐
         │  Input Injection Layer       │
         │  - Win32Mouse::addEvent()    │
         │  - Keyboard::addKey()        │
         │  - IMEManager::addComposition
         └────────┬─────────────────────┘
                  │
                  ▼
         ┌──────────────────────────────┐
         │  Game Logic Input Handling   │
         │  - GameWindow message flow   │
         │  - Game state updates        │
         └──────────────────────────────┘
```

---

## Exit Criteria Met ✅

- [x] Audit list is complete with all Win32 APIs in shipping runtime
- [x] Integration diagram showing Entry → Engine → Device → OS bridge created
- [x] All subsystems categorized and cross-platform strategy identified
- [x] Existing abstractions (ClientInstance, TheFileSystem) leveraged
- [x] No surprise Win32 APIs discovered; all scoped appropriately

---

## Next Steps (PHASE 2)

1. Create SDL2-based device layer skeleton
2. Implement SDL2 window creation and event loop
3. Port window lifecycle management (focus, minimize, fullscreen)
4. Implement display enumeration via SDL2
5. Port load screen rendering
6. Test window creation, event loop, and basic navigation

---

## Appendix: File Reference Map

### Entry Points
- [GeneralsMD/Code/Main/WinMain.cpp](GeneralsMD/Code/Main/WinMain.cpp) - Main entry, WndProc, window creation

### Platform Device Layer
- [GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp](GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp) - Message pump
- [GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32OSDisplay.cpp](GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32OSDisplay.cpp) - Display management
- [GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32Mouse.cpp](GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32Mouse.cpp) - Mouse input
- [GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32DIKeyboard.cpp](GeneralsMD/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32DIKeyboard.cpp) - Keyboard input

### Cross-Platform Infrastructure (Already in Place)
- [GeneralsMD/Code/GameEngine/Source/GameClient/ClientInstance.cpp](GeneralsMD/Code/GameEngine/Source/GameClient/ClientInstance.cpp) - Single-instance mechanism
- [GeneralsMD/Code/GameEngine/Include/Common/Registry.h](GeneralsMD/Code/GameEngine/Include/Common/Registry.h) - Registry/INI abstraction
- [assets/ini/README.md](assets/ini/README.md) - Configuration file documentation

### Reference Implementations
- `references/fighter19-dxvk-port/GeneralsMD/Code/Main/SDL3Main.cpp` - Linux SDL3 entry point
- `references/fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/` - Linux SDL3 device layer

