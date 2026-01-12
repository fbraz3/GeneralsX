# SDL2Device Architecture Design
**Phase 02: SDL2 App Entry + Event Pump**  
**Status**: Design Document  
**Date**: 2026-01-11

---

## Overview

This document describes the architecture of the SDL2Device layer introduced in Phase 02. It explains how SDL2-based application entry and event pumping replaces Win32 equivalents while maintaining compatibility with existing game engine code.

---

## 1. Architecture Layers

### 1.1 Application Entry Point

**File**: `GeneralsMD/Code/Main/SDL2Main.cpp`

The SDL2Main function mirrors WinMain but uses SDL2 instead of Win32:

```
main(argc, argv)
  ├─ Initialize memory manager
  ├─ Parse command-line arguments
  ├─ Create SDL2 window (1024x768 default)
  ├─ Initialize SingleInstance mechanism (rts::ClientInstance)
  ├─ Call GameMain() [main game loop]
  └─ Cleanup SDL2 and memory
```

**Key differences from WinMain**:
- Uses `SDL_Init()` instead of `SetUnhandledExceptionFilter()`
- Uses `SDL_CreateWindow()` instead of `CreateWindow()`
- Uses `SDL_Quit()` instead of `OleUninitialize()`
- No resource loading (splash bitmap handled later)
- Signal handlers for SIGTERM/SIGINT (Unix-style shutdown)

### 1.2 Platform Game Engine

**Files**:
- `GeneralsMD/Code/GameEngineDevice/Include/SDL2Device/Common/SDL2GameEngine.h`
- `GeneralsMD/Code/GameEngineDevice/Source/SDL2Device/Common/SDL2GameEngine.cpp`

The SDL2GameEngine class extends GameEngine:

```cpp
class SDL2GameEngine : public GameEngine
{
  void init();
  void reset();
  void update();                  // Called every frame from GameMain
  void serviceSDL2OS();          // Event pump (replaces serviceWindowsOS)
  void handleWindowEvent(...);   // Maps SDL window events
  void handleQuitEvent();        // Maps SDL_QUIT
};
```

### 1.3 SDL2 Event Translation Boundary

The critical point where SDL2 events translate to engine-level events:

```
SDL2 Events                  Engine Events
├─ SDL_QUIT                 ├─ MSG_META_DEMO_INSTANT_QUIT
├─ SDL_WINDOWEVENT_CLOSE    └─ (same)
├─ SDL_WINDOWEVENT_FOCUS_GAINED  ├─ setIsActive(true)
├─ SDL_WINDOWEVENT_FOCUS_LOST    ├─ setIsActive(false)
├─ SDL_WINDOWEVENT_MINIMIZED     ├─ Low-power update (SDL_Delay)
├─ SDL_WINDOWEVENT_RESTORED      └─ Normal update resume
├─ SDL_MOUSEBUTTONDOWN     ├─ Phase 03: SDL2Mouse::addEvent()
├─ SDL_MOUSEMOTION         └─ (deferred to Phase 03)
└─ SDL_TEXTINPUT           ├─ Phase 03: IMEManager::addText()
                           └─ (deferred to Phase 03)
```

---

## 2. Global State Management

### 2.1 SDL2Main.cpp Globals

Replace Win32 globals with SDL2 equivalents:

| Win32 Global | SDL2 Global | Purpose |
|--------------|-------------|---------|
| `HWND ApplicationHWnd` | `SDL_Window *g_applicationWindow` | Main application window |
| `HINSTANCE ApplicationHInstance` | N/A | Not needed with SDL2 |
| `Win32Mouse *TheWin32Mouse` | `SDL2Mouse *TheSDL2Mouse` | Input device (Phase 03) |
| `DWORD TheMessageTime` | `Uint32 g_eventTimestamp` | Event timestamp |

### 2.2 Persistence Across Module Boundaries

Globals defined in SDL2Main.cpp are declared as `extern` in:
- SDL2GameEngine.cpp (for window access, event pumping)
- SDL2Mouse.h/cpp (Phase 03, for input injection)
- IMEManager.cpp (Phase 03, for text input)

---

## 3. Event Loop Structure

### 3.1 Game Main Loop (GameMain() in Main.cpp)

```
GameMain()
  ├─ Initialize subsystems
  ├─ while (!quitting)
  │   ├─ GameEngine::update()
  │   │   ├─ GameEngine::update() [game logic]
  │   │   └─ SDL2GameEngine::serviceSDL2OS()  ← EVENT PUMP HERE
  │   │       ├─ SDL_PollEvent loop
  │   │       ├─ handleWindowEvent()
  │   │       ├─ handleQuitEvent()
  │   │       └─ (Phase 03: handleMouseEvent, handleKeyboardEvent)
  │   ├─ Render frame
  │   └─ Frame timing
  └─ Shutdown subsystems
```

### 3.2 SDL_PollEvent Loop (serviceSDL2OS)

```cpp
void SDL2GameEngine::serviceSDL2OS()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    g_eventTimestamp = event.common.timestamp;
    
    switch (event.type)
    {
      case SDL_QUIT: handleQuitEvent(); break;
      case SDL_WINDOWEVENT: handleWindowEvent(event.window); break;
      case SDL_KEYDOWN: /* Phase 03 */ break;
      case SDL_MOUSEBUTTONDOWN: /* Phase 03 */ break;
      case SDL_TEXTINPUT: /* Phase 03 */ break;
      ...
    }
  }
}
```

**Key Design Point**: Non-blocking event polling (`SDL_PollEvent`) called once per frame, exactly like Win32 `PeekMessage/GetMessage` pattern.

---

## 4. Window Lifecycle Management

### 4.1 Window Creation (SDL2Main.cpp)

```cpp
Uint32 windowFlags = SDL_WINDOW_SHOWN;
if (!runWindowed)
  windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

g_applicationWindow = SDL_CreateWindow(
  "Command and Conquer Generals",
  SDL_WINDOWPOS_CENTERED,
  SDL_WINDOWPOS_CENTERED,
  1024,  // width
  768,   // height
  windowFlags
);
```

Maps to:
- Windowed: `SDL_WINDOW_SHOWN` (no fullscreen flag)
- Fullscreen: `SDL_WINDOW_FULLSCREEN_DESKTOP` (borderless fullscreen)

### 4.2 Window Event Handling (SDL2GameEngine.cpp)

| SDL Event | Handler | Engine Action |
|-----------|---------|---------------|
| `FOCUS_GAINED` | `handleWindowEvent()` | `setIsActive(true)`, reset keyboard, regain audio/mouse |
| `FOCUS_LOST` | `handleWindowEvent()` | `setIsActive(false)`, reset keyboard, lose audio focus |
| `MINIMIZED` | `handleWindowEvent()` | `setIsActive(false)` |
| `RESTORED` | `handleWindowEvent()` | `setIsActive(true)` |
| `MOVED` | `handleWindowEvent()` | Refresh cursor capture |
| `RESIZED` | `handleWindowEvent()` | Refresh cursor capture |
| `ENTER` / `LEAVE` | `handleWindowEvent()` | Track cursor entering/leaving window |
| `CLOSE` | `handleWindowEvent()` | Post `MSG_META_DEMO_INSTANT_QUIT` |

### 4.3 Minimized Window Handling (Alt-Tab Equivalent)

```cpp
void SDL2GameEngine::update()
{
  GameEngine::update();
  
  // Check if minimized
  if (SDL_GetWindowFlags(g_applicationWindow) & SDL_WINDOW_MINIMIZED)
  {
    while (minimized)
    {
      SDL_Delay(5);  // Replaces Sleep(5) from Win32
      serviceSDL2OS();
      TheLAN->update();  // Keep network alive
    }
  }
  
  serviceSDL2OS();  // Normal event pump
}
```

This mirrors Win32GameEngine's alt-tab handling exactly.

---

## 5. Minimized Window Implications (Critical Design Notes)

### 5.1 Alt-Tab on Windows (Native Build)

The game can still be built with Win32GameEngine on Windows:
- Uses `PeekMessage/GetMessage/DispatchMessage` (existing code)
- Handles `WM_ACTIVATEAPP` to detect alt-tab
- Calls `Sleep(5)` in the iconic window loop

### 5.2 Window Minimization on macOS/Linux

SDL2 reports minimization via `SDL_WINDOWEVENT_MINIMIZED`:
- `SDL_GetWindowFlags() & SDL_WINDOW_MINIMIZED` returns true
- Loop in `update()` detects this and calls `SDL_Delay(5)`
- Same network update behavior as Windows

### 5.3 CPU Efficiency

Both implementations:
- ✅ Avoid busy-loop when minimized
- ✅ Sleep in 5ms intervals (prevents CPU spinning)
- ✅ Process network updates to prevent lag on reconnection
- ✅ Exit loop when game quits or multiplayer resumes

---

## 6. Compiler & CMake Integration

### 6.1 Conditional Compilation

**On Windows**: Compile Win32Main.cpp + Win32Device/
```cmake
if(WIN32)
  target_sources(GeneralsXZH PRIVATE
    Main/WinMain.cpp
    GameEngineDevice/Source/Win32Device/...
  )
  target_sources(z_gameenginedevice PRIVATE
    Include/Win32Device/...
  )
endif()
```

**On macOS/Linux**: Compile SDL2Main.cpp + SDL2Device/
```cmake
if(NOT WIN32)
  target_sources(GeneralsXZH PRIVATE
    Main/SDL2Main.cpp
    GameEngineDevice/Source/SDL2Device/...
  )
  target_sources(z_gameenginedevice PRIVATE
    Include/SDL2Device/...
  )
  target_link_libraries(GeneralsXZH PRIVATE SDL2::SDL2)
endif()
```

### 6.2 CMake Preset Strategy

- **vc6** preset: Windows 32-bit (uses WinMain + Win32Device)
- **macos** preset: macOS ARM64 (uses main + SDL2Device + SDL2)
- **linux** preset: Linux x86_64 (uses main + SDL2Device + SDL2)
- **windows** preset (future): Windows 64-bit (uses main + SDL2Device + SDL2)

---

## 7. Phased Implementation (Why This Design Works)

### Phase 02 (Current)
✅ SDL2Main.cpp entry point  
✅ SDL2GameEngine with SDL_PollEvent  
✅ Window lifecycle (focus, minimize, close)  
✅ Minimized window handling (alt-tab equivalent)  
✅ Event timestamp tracking  

### Phase 03
🔲 SDL2Mouse (mouse input injection)  
🔲 SDL2Keyboard (keyboard input)  
🔲 IMEManager SDL2 integration  
🔲 Text input handling  

### Phase 04
🔲 SDL2 Timing APIs (SDL_Delay, SDL_GetTicks)  
🔲 Power events (SDL_POWERSTATE)  
🔲 Cross-platform paths & config  

### Phase 05
🔲 Stability testing on all platforms  
🔲 Performance optimization  
🔲 Gameplay validation  

---

## 8. Key Design Decisions & Rationale

### 8.1 Separate Entry Point vs. Single main()

**Decision**: Use separate SDL2Main.cpp (non-Windows) and WinMain.cpp (Windows)

**Rationale**:
- Clear separation of platform-specific code
- Easier to maintain (each platform has its own entry point)
- Mirrors existing codebase structure
- No `#ifdef WIN32` pollution in entry point
- CMake can select which to compile

**Alternative Rejected**: Single main() with conditional compilation
- Would require `#ifdef WIN32` throughout entry point
- Harder to understand control flow
- Duplicates code paths unnecessarily

### 8.2 Global SDL Window Pointer vs. Engine Property

**Decision**: Use global `g_applicationWindow` pointer (mirrors `HWND ApplicationHWnd`)

**Rationale**:
- Matches existing Win32 pattern
- Accessible to all modules without changing signatures
- Only 2 instances (window + renderer)
- Will be eliminated in Phase 03 when we improve abstraction

**Future**: Encapsulate in `ApplicationWindow` class or engine property

### 8.3 Event Translation at serviceSDL2OS() Boundary

**Decision**: Map ALL SDL2 events to engine-level concepts in serviceSDL2OS()

**Rationale**:
- Single boundary point (not scattered `#ifdef`s)
- Easy to understand control flow
- Phase 03 can extend with input-specific handling
- No changes needed to core engine code

**Example**:
```cpp
// SDL2GameEngine.cpp serviceSDL2OS():
case SDL_WINDOWEVENT_FOCUS_GAINED:
  setIsActive(true);  // Engine-level call (not SDL_specific)
  if (TheMouse) TheMouse->regainFocus();  // Existing interface
```

### 8.4 Minimized Window Loop in update() vs. separate thread

**Decision**: Sleep loop in `update()` method (single-threaded)

**Rationale**:
- Matches Win32 implementation exactly
- Keeps logic in same thread (no race conditions)
- Simplest to understand and maintain
- Game engine is not multithreaded anyway
- Network updates still happen (`TheLAN->update()`)

---

## 9. Testing Strategy

### 9.1 Phase 02 Smoke Tests

After building, verify:

1. **Application starts**: No segfaults or assertion failures
2. **Window appears**: SDL2 window creates successfully
3. **Initial screen**: Game shows initial loading/splash screen
4. **Message pump works**: ESC key quits cleanly (SDL_QUIT event)
5. **Minimization**: Alt-Tab minimizes and restores without crashing
6. **Focus events**: Regain focus doesn't cause audio/input glitches

### 9.2 Debugging Checklist

- [ ] Verify `g_applicationWindow` is not NULL when expected
- [ ] Check SDL_GetWindowFlags() returns correct minimized state
- [ ] Confirm SDL_Delay(5) actually pauses without busy-loop (check CPU %)
- [ ] Verify game doesn't hang when minimized with network traffic

---

## 10. Known Limitations & Future Work

### 10.1 Phase 02 Limitations

- **No input handling**: ESC key handled, but no mouse/keyboard proper input
- **No IME**: Text input APIs not integrated (Phase 03)
- **No timing**: Using default SDL timing (Phase 04)
- **No network events**: Power state events not handled (Phase 04)

### 10.2 Gaps to Address in Future Phases

| Gap | Phase | Status |
|-----|-------|--------|
| Mouse input | Phase 03 | 🔲 Not started |
| Keyboard input | Phase 03 | 🔲 Not started |
| IME/Text input | Phase 03 | 🔲 Not started |
| Cursor management | Phase 03 | 🔲 Not started |
| Timing functions | Phase 04 | 🔲 Not started |
| Power events | Phase 04 | 🔲 Not started |
| Configuration loading | Phase 04 | 🔲 Not started |

---

## 11. References

### Audit Phase 1 Findings
- [PHASE01_WIN32_AUDIT_FINDINGS.md](../support/PHASE01_WIN32_AUDIT_FINDINGS.md) - Complete Win32 API mapping

### Plan & Baseline
- [PLAN-001_PRIMARY_GOAL_SDL2_PORT.md](../planning/PLAN-001_PRIMARY_GOAL_SDL2_PORT.md) - Overall strategy
- [PHASE01_AUDIT_AND_BASELINE.md](./PHASE01_AUDIT_AND_BASELINE.md) - Phase 1 details

### SDL2 Documentation
- SDL2 API Reference: https://wiki.libsdl.org/SDL2/FrontPage
- SDL_PollEvent: https://wiki.libsdl.org/SDL2/SDL_PollEvent
- SDL_WindowEvent: https://wiki.libsdl.org/SDL2/SDL_WindowEvent

### Reference Implementation
- `references/fighter19-dxvk-port/GeneralsMD/Code/Main/SDL3Main.cpp` - Linux SDL3 entry point
- `references/fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/` - SDL3 device layer

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-11 | Phase 02 | Initial design document for SDL2Device architecture |

