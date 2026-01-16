# SDL2 Implementation: Complete File Tree Reference

**Reference Search Complete**: January 15, 2026

---

## ALL RELEVANT FILES FOUND IN REFERENCES

### jmarshall-win64-modern: Base Classes & Architecture

```
references/jmarshall-win64-modern/Code/

GameEngine/Include/GameClient/
├── Mouse.h                           [379 lines]
│   ├─ class Mouse : public SubsystemInterface
│   ├─ struct MouseIO { position, buttons, wheel, etc }
│   ├─ struct CursorInfo
│   ├─ enum MouseCursor (NUM_MOUSE_CURSORS, various cursor types)
│   └─ Provides: Abstract mouse interface for all implementations
│
├── Keyboard.h                        [173 lines]
│   ├─ class Keyboard : public SubsystemInterface
│   ├─ struct KeyboardIO { key, status, state, sequence }
│   ├─ Methods: getFirstKey(), isShift(), isCtrl(), isAlt()
│   └─ Provides: Abstract keyboard interface for all implementations
│
└── GameWindow.h                      [424+ lines]
    ├─ class GameWindow : public MemoryPoolObject
    └─ Provides: Abstract window interface for all implementations

GameEngineDevice/Include/Win32Device/GameClient/
├── Win32Mouse.h                      [Concrete: extends Mouse]
│   ├─ class Win32Mouse : public Mouse
│   ├─ struct Win32MouseEvent { msg, wparam, lparam, time }
│   ├─ Method: void addWin32Event(UINT msg, WPARAM wParam, LPARAM lParam, DWORD time)
│   ├─ Data: m_eventBuffer[NUM_MOUSE_EVENTS]
│   └─ Purpose: Translate Win32 window messages to MouseIO events
│
└── Win32DIKeyboard.h                 [Concrete: extends Keyboard]
    ├─ class DirectInputKeyboard : public Keyboard
    ├─ Data: LPDIRECTINPUT8, LPDIRECTINPUTDEVICE8
    └─ Purpose: Translate DirectInput events to KeyboardIO events

GameEngineDevice/Include/W3DDevice/GameClient/
├── W3DGameWindow.h                   [Extends GameWindow]
│   ├─ class W3DGameWindow : public GameWindow
│   ├─ Data: Render2DSentenceClass m_textRenderer
│   └─ Purpose: Window rendering with W3D text support
│
├── W3DMouse.h                        [Extends Win32Mouse]
│   ├─ class W3DMouse : public Win32Mouse
│   ├─ Data: SurfaceClass*, CameraClass*
│   └─ Purpose: Win32 mouse extended with W3D cursor rendering
│
└── GameClient/
    └─ Module/                       [Subdirectory for gadgets/UI]

```

### fighter19-dxvk-port: Production SDL3 Code (Adapt to SDL2)

```
references/fighter19-dxvk-port/GeneralsMD/Code/

GameEngineDevice/Include/SDL3Device/GameClient/
├── SDL3Keyboard.h                    [~150 lines] ← ADAPT TO SDL2
│   ├─ #pragma once
│   ├─ #include <SDL3/SDL.h>          ← CHANGE TO <SDL2/SDL.h>
│   ├─ union SDL_Event;               ← Forward declaration
│   ├─ class SDL3Keyboard : public Keyboard
│   │  ├─ SDL3Keyboard();
│   │  ├─ virtual ~SDL3Keyboard();
│   │  ├─ virtual void init();
│   │  ├─ virtual void reset();
│   │  ├─ virtual void update();
│   │  ├─ virtual Bool getCapsState();
│   │  ├─ virtual void getKey(KeyboardIO *key);
│   │  ├─ void addSDLEvent(SDL_Event *ev);  ← EVENT SUBMISSION API
│   │  ├─ std::vector<SDL_Event> m_events;  ← EVENT BUFFER
│   │  ├─ static KeyDefType ConvertSDLKey(SDL_Keycode keycode);
│   │  ├─ void openKeyboard();
│   │  └─ void closeKeyboard();
│   └─ Data members: m_events vector, index pointers
│
└── SDL3Mouse.h                       [~200 lines] ← ADAPT TO SDL2
    ├─ #pragma once
    ├─ #include <SDL3/SDL.h>          ← CHANGE TO <SDL2/SDL.h>
    ├─ #include <SDL3_image/SDL_image.h>  ← OPTIONAL for SDL2
    ├─ class AnimatedCursor { ... };  ← 100% REUSABLE
    │  ├─ std::array<SDL_Cursor*, MAX_2D_CURSOR_ANIM_FRAMES>
    │  ├─ std::array<SDL_Surface*, MAX_2D_CURSOR_ANIM_FRAMES>
    │  ├─ int m_currentFrame, m_frameCount, m_frameRate
    │  └─ Destructor cleanup
    │
    ├─ class SDL3Mouse : public Mouse
    │  ├─ SDL3Mouse();
    │  ├─ virtual ~SDL3Mouse();
    │  ├─ virtual void init();
    │  ├─ virtual void reset();
    │  ├─ virtual void update();
    │  ├─ virtual void setCursor(MouseCursor cursor);
    │  ├─ virtual void capture();
    │  ├─ virtual void releaseCapture();
    │  ├─ virtual void setVisibility(Bool visible);
    │  ├─ virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush);
    │  ├─ void addSDLEvent(SDL_Event *ev);  ← EVENT SUBMISSION API
    │  ├─ AnimatedCursor* loadCursorFromFile(const char* file);  ← ANI LOADER
    │  ├─ static void scaleMouseCoordinates(...);
    │  └─ void lostFocus(Bool state);
    │
    └─ Data: m_eventBuffer, indices, cursor data, ani structures

GameEngineDevice/Source/SDL3Device/GameClient/

SDL3Keyboard.cpp                       [310 lines] ← ADAPT TO SDL2
├─ #include "SDL3Device/GameClient/SDL3Keyboard.h"
├─ #include <SDL3/SDL.h>               ← CHANGE TO <SDL2/SDL.h>
│
├─ ConvertSDLKey(SDL_Keycode keycode) function:
│  ├─ Maps SDLK_A...SDLK_Z → KEY_A...KEY_Z
│  ├─ Maps SDLK_0...SDLK_9 → KEY_0...KEY_9  [with special SDLK_0 case]
│  ├─ Maps special keys: SDLK_RETURN, SDLK_SPACE, SDLK_ESCAPE, etc.
│  ├─ Maps modifier keys: SDLK_LSHIFT, SDLK_RSHIFT, SDLK_LCTRL, etc.
│  ├─ Maps arrow keys: SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT
│  └─ Returns KEY_NONE for unmapped keys
│
├─ SDL3Keyboard::openKeyboard():
│  ├─ SDL_InitSubSystem(SDL_INIT_EVENTS)
│  └─ DEBUG_LOG on success/failure
│
├─ SDL3Keyboard::closeKeyboard():
│  ├─ SDL_QuitSubSystem(SDL_INIT_EVENTS)
│  └─ DEBUG_LOG cleanup message
│
├─ SDL3Keyboard::init():
│  └─ Calls openKeyboard()
│
├─ SDL3Keyboard::reset():
│  └─ Clears event buffer
│
├─ SDL3Keyboard::update():
│  └─ Processes events and updates key states
│
├─ SDL3Keyboard::getKey(KeyboardIO *key):
│  └─ Retrieves next key from event buffer
│
└─ SDL3Keyboard::addSDLEvent(SDL_Event *ev):
   └─ Called from main event loop, adds event to buffer

SDL3Mouse.cpp                          [637 lines] ← ADAPT TO SDL2 (large but 95% reusable)
├─ #include "SDL3Device/GameClient/SDL3Mouse.h"
├─ #include <SDL3/SDL_events.h>        ← CHANGE TO <SDL2/SDL.h>
├─ #include <SDL3_image/SDL_image.h>   ← OPTIONAL, ANI parser doesn't need
│
├─ RIFF/ACON ANI File Format Structures (100% REUSABLE):
│  ├─ typedef std::array<char, 4> FourCC;
│  ├─ struct ANIHeader { size, frames, steps, width, height, ... }
│  ├─ struct RIFFChunk { id, size, type }
│  ├─ Helper functions: getNextChunk(), getChunkData()
│  └─ FourCC constants: riff_id, acon_id, anih_id, fram_id, icon_id, etc.
│
├─ AnimatedCursor* loadCursorFromFile(const char* filepath) (400+ lines, 100% REUSABLE):
│  ├─ File reading via TheFileSystem::openFile()
│  ├─ Buffer management: file->readEntireAndClose()
│  ├─ RIFF header validation
│  ├─ ACON chunk iteration and parsing
│  ├─ ANI header extraction (frame count, frame rate, etc)
│  ├─ Frame icon parsing and SDL surface creation per frame
│  ├─ SDL_CreateCursor() for each frame with correct hotspot
│  ├─ AnimatedCursor struct population
│  └─ Cleanup: delete[] file_buffer
│
├─ SDL3Mouse::getMouseEvent(MouseIO *result, Bool flush):
│  ├─ Checks event buffer for pending events
│  ├─ Translates SDL event to MouseIO structure
│  ├─ Handles mouse position, button states, wheel
│  └─ Returns MOUSE_OK or MOUSE_EVENT_NONE
│
├─ void SDL3Mouse::addSDLEvent(SDL_Event *ev):
│  └─ Called from main event loop, adds to m_eventBuffer
│
├─ void scaleMouseCoordinates(int rawX, int rawY, Uint32 windowID, ...):
│  └─ Applies DPI/resolution scaling to mouse coordinates
│
├─ SDL3Mouse::init():
│  ├─ Calls initCursorResources()
│  └─ Initializes animated cursor system
│
├─ SDL3Mouse::reset():
│  └─ Resets mouse state
│
├─ SDL3Mouse::setCursor(MouseCursor cursor):
│  ├─ Loads cursor from file (ANI if available)
│  ├─ Sets current cursor
│  └─ Handles animation setup
│
└─ Other methods: capture(), releaseCapture(), setVisibility(), update()
```

---

## Implementation Locations (To Be Created)

```
Core/GameEngineDevice/

NEW DIRECTORIES TO CREATE:
├── Include/
│   └── SDL2Device/               ← NEW
│       ├── Common/               ← NEW
│       └── GameClient/           ← NEW
│           ├── SDL2Keyboard.h    ← NEW (adapt from fighter19)
│           └── SDL2Mouse.h       ← NEW (adapt from fighter19)
│
└── Source/
    └── SDL2Device/               ← NEW
        ├── Common/               ← NEW
        └── GameClient/           ← NEW
            ├── SDL2Keyboard.cpp  ← NEW (adapt from fighter19)
            └── SDL2Mouse.cpp     ← NEW (adapt from fighter19)

EXISTING STRUCTURE (for reference):
├── Include/
│   ├── OpenALAudioDevice/
│   ├── MilesAudioDevice/
│   ├── VideoDevice/
│   ├── W3DDevice/               ← Study pattern from this
│   │   ├── Common/
│   │   └── GameClient/
│   │       ├── W3DGameWindow.h   ← Reference for window handling
│   │       └── W3DMouse.h        ← Reference for cursor extension
│   └── Win32Device/             ← Study pattern from this
│       ├── Common/
│       └── GameClient/
│           ├── Win32DIKeyboard.h ← Reference for keyboard pattern
│           └── Win32Mouse.h      ← Reference for mouse pattern
│
└── Source/
    ├── OpenALAudioDevice/
    ├── MilesAudioDevice/
    ├── VideoDevice/
    ├── W3DDevice/
    │   ├── Common/
    │   └── GameClient/
    │       ├── W3DGameWindow.cpp
    │       └── W3DMouse.cpp
    └── Win32Device/
        ├── Common/
        └── GameClient/
            ├── Win32DIKeyboard.cpp
            └── Win32Mouse.cpp
```

---

## File Change Summary

### Files NOT Requiring Changes (Reference Only)

```
✓ Code/GameEngine/Include/GameClient/Mouse.h
✓ Code/GameEngine/Include/GameClient/Keyboard.h  
✓ Code/GameEngine/Include/GameClient/GameWindow.h
  → These define the abstract interfaces that SDL2 implementations inherit from
```

### Files to CREATE (SDL2 Adaptations)

```
1. Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Keyboard.h
   Source: fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Include/SDL3Device/GameClient/SDL3Keyboard.h
   Changes:
   • Rename: SDL3Keyboard → SDL2Keyboard
   • Change: #include <SDL3/SDL.h> → #include <SDL2/SDL.h>
   • Modify: ConvertSDLKey() → ConvertSDLScancode()
   • Modify: SDL_Keycode → SDL_Scancode
   • Keep: Class structure, methods, event buffer pattern
   Lines: ~150
   
2. Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Keyboard.cpp
   Source: fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/GameClient/SDL3Keyboard.cpp
   Changes:
   • Rename all references: SDL3Keyboard → SDL2Keyboard
   • Update ConvertSDLKey implementation:
     - Change SDLK_* constants → SDL_SCANCODE_* constants
     - Map different numeric values for scancodes
   • Update SDL3/SDL.h includes → SDL2/SDL.h
   • Keep: Logic, buffer management, initialization/cleanup
   Lines: ~310
   
3. Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Mouse.h
   Source: fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Include/SDL3Device/GameClient/SDL3Mouse.h
   Changes:
   • Rename: SDL3Mouse → SDL2Mouse
   • Change: #include <SDL3/SDL.h> → #include <SDL2/SDL.h>
   • Keep: AnimatedCursor struct (100% reusable)
   • Keep: All method signatures
   • Keep: Event buffer structure
   Lines: ~200
   
4. Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Mouse.cpp
   Source: fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/GameClient/SDL3Mouse.cpp
   Changes:
   • Rename all references: SDL3Mouse → SDL2Mouse
   • Update includes: SDL3 → SDL2
   • Keep: ANI file parser (100% identical)
   • Keep: AnimatedCursor logic (100% identical)
   • Keep: Event translation logic (100% identical)
   • Keep: DPI scaling function
   Lines: ~637
```

### Files Requiring Updates (Integration)

```
Core/GameEngineDevice/CMakeLists.txt
• Add SDL2Device directory to build
• Add SDL2Mouse.cpp and SDL2Keyboard.cpp sources
• Link SDL2 library dependency
• Define SDL2Device target

Generals/Code/Main/WinMain.cpp (or equivalent game main)
• Integration point for event loop
• Call TheSDL2Mouse->addSDLEvent() in SDL_PollEvent loop
• Call TheSDL2Keyboard->addSDLEvent() in SDL_PollEvent loop

GeneralsMD/Code/Main/WinMain.cpp (same for Zero Hour)
• Same integration as Generals

Core/GameEngine/Include/GameClient/GameClient.h (or SDL2Device.h)
• Global pointers if needed: SDL2Mouse *TheSDL2Mouse, etc.
• Or use existing mouse/keyboard abstract pointers
```

---

## Total Code to Adapt

```
Source Files:
  ✓ SDL3Keyboard.h      (150 lines)  → SDL2Keyboard.h      (150 lines)
  ✓ SDL3Keyboard.cpp    (310 lines)  → SDL2Keyboard.cpp    (310 lines)
  ✓ SDL3Mouse.h         (200 lines)  → SDL2Mouse.h         (200 lines)
  ✓ SDL3Mouse.cpp       (637 lines)  → SDL2Mouse.cpp       (637 lines)
  ──────────────────────────────────────────────────────────────────────
  TOTAL:              ~1,297 lines to adapt (95%+ reusable)

Build Configuration:
  + CMakeLists.txt updates (minor: ~10 lines)
  + SDL2Device target creation (~20 lines)

Integration:
  + Main event loop wiring (~10 lines per target)
  + Global object initialization (~5 lines per target)

ESTIMATED CHANGES: ~100 lines SDL2-specific, rest copy/paste
```

---

## Copy-Paste Ready Code Blocks

### Block 1: Key Translation Mapping (99% identical SDL2 → SDL3)

```cpp
// SDL2 SCANCODE TRANSLATION
// Copy from fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/GameClient/SDL3Keyboard.cpp
// And adapt: SDLK_* → SDL_SCANCODE_*

static KeyDefType ConvertSDLScancode(SDL_Scancode scancode) {
    // Letter keys A-Z
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
        return (KeyDefType)(KEY_A + (scancode - SDL_SCANCODE_A));
    }
    // Number keys 1-9 (special case for 0)
    else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9) {
        return (KeyDefType)(KEY_1 + (scancode - SDL_SCANCODE_1));
    }
    else if (scancode == SDL_SCANCODE_0) {
        return KEY_0;
    }
    // Function keys F1-F12
    else if (scancode >= SDL_SCANCODE_F1 && scancode <= SDL_SCANCODE_F12) {
        return (KeyDefType)(KEY_F1 + (scancode - SDL_SCANCODE_F1));
    }
    // Special keys
    else if (scancode == SDL_SCANCODE_RETURN) return KEY_RETURN;
    else if (scancode == SDL_SCANCODE_ESCAPE) return KEY_ESCAPE;
    else if (scancode == SDL_SCANCODE_SPACE) return KEY_SPACE;
    else if (scancode == SDL_SCANCODE_TAB) return KEY_TAB;
    else if (scancode == SDL_SCANCODE_BACKSPACE) return KEY_BACKSPACE;
    else if (scancode == SDL_SCANCODE_DELETE) return KEY_DELETE;
    else if (scancode == SDL_SCANCODE_LSHIFT) return KEY_LSHIFT;
    else if (scancode == SDL_SCANCODE_RSHIFT) return KEY_RSHIFT;
    else if (scancode == SDL_SCANCODE_LCTRL) return KEY_LCTRL;
    else if (scancode == SDL_SCANCODE_RCTRL) return KEY_RCTRL;
    else if (scancode == SDL_SCANCODE_LALT) return KEY_LALT;
    else if (scancode == SDL_SCANCODE_RALT) return KEY_RALT;
    // Arrow keys
    else if (scancode == SDL_SCANCODE_UP) return KEY_UP;
    else if (scancode == SDL_SCANCODE_DOWN) return KEY_DOWN;
    else if (scancode == SDL_SCANCODE_LEFT) return KEY_LEFT;
    else if (scancode == SDL_SCANCODE_RIGHT) return KEY_RIGHT;
    // Home/End/PgUp/PgDn
    else if (scancode == SDL_SCANCODE_HOME) return KEY_HOME;
    else if (scancode == SDL_SCANCODE_END) return KEY_END;
    else if (scancode == SDL_SCANCODE_PAGEUP) return KEY_PAGEUP;
    else if (scancode == SDL_SCANCODE_PAGEDOWN) return KEY_PAGEDOWN;
    // Insert key
    else if (scancode == SDL_SCANCODE_INSERT) return KEY_INSERT;
    // Numeric keypad
    else if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_9) {
        return (KeyDefType)(KEY_KP_1 + (scancode - SDL_SCANCODE_KP_1));
    }
    else if (scancode == SDL_SCANCODE_KP_0) return KEY_KP_0;
    else if (scancode == SDL_SCANCODE_KP_PLUS) return KEY_KP_PLUS;
    else if (scancode == SDL_SCANCODE_KP_MINUS) return KEY_KP_MINUS;
    else if (scancode == SDL_SCANCODE_KP_PERIOD) return KEY_KP_PERIOD;
    else if (scancode == SDL_SCANCODE_KP_DIVIDE) return KEY_KP_DIVIDE;
    else if (scancode == SDL_SCANCODE_KP_MULTIPLY) return KEY_KP_MULTIPLY;
    else if (scancode == SDL_SCANCODE_KP_ENTER) return KEY_KP_ENTER;
    
    return KEY_NONE;
}
```

---

## Verification Checklist

After creating SDL2Device files:

```
Files Created:
[ ] Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Keyboard.h exists
[ ] Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Keyboard.cpp exists
[ ] Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Mouse.h exists
[ ] Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Mouse.cpp exists

Code Validation:
[ ] SDL2Keyboard.h compiles without errors
[ ] SDL2Keyboard.cpp compiles without errors
[ ] SDL2Mouse.h compiles without errors
[ ] SDL2Mouse.cpp compiles without errors
[ ] No remaining SDL3 references in SDL2 files
[ ] No remaining SDL2 references in comments claiming SDL3
[ ] ConvertSDLScancode() covers all essential game keys
[ ] ANI file parser struct/constant definitions are correct

Build Integration:
[ ] CMakeLists.txt includes SDL2Device
[ ] CMakeLists.txt links SDL2 library
[ ] SDL2Device target builds successfully
[ ] No undefined reference errors
[ ] No missing include path errors
```

---

**Document Complete**: All SDL2 files identified and ready for adaptation

**Next Phase**: Use these file paths and patterns to implement SDL2Device layer
