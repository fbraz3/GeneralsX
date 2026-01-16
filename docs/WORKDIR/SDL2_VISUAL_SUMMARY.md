# SDL2 Implementation Reference: Visual Summary

**Last Updated**: January 15, 2026

---

## Complete File Inventory

### Source: jmarshall-win64-modern (Architecture Blueprint)

```
CRITICAL FOR UNDERSTANDING ABSTRACTION PATTERN:

GameEngine/Include/GameClient/
  ├─ Mouse.h                        [Abstract interface - base for all mice]
  │  public methods: init, reset, update, setCursor, capture, releaseCapture
  │  abstract: getMouseEvent() → MouseIO
  │
  ├─ Keyboard.h                     [Abstract interface - base for all keyboards]
  │  public methods: init, reset, update, getCapsState
  │  abstract: getKey() → KeyboardIO
  │
  └─ GameWindow.h                   [Abstract window - base for all windows]
     public methods: draw, update, event handling

GameEngineDevice/Include/Win32Device/GameClient/
  ├─ Win32Mouse.h                   [Concrete: extends Mouse]
  │  adds: addWin32Event(UINT msg, WPARAM wParam, LPARAM lParam, DWORD time)
  │  implements: getMouseEvent() using Win32 messages
  │  ├─ m_eventBuffer[Mouse::NUM_MOUSE_EVENTS]
  │  └─ Win32MouseEvent structure with msg/wparam/lparam/time
  │
  └─ Win32DIKeyboard.h              [Concrete: extends Keyboard]
     adds: DirectInput specific initialization
     implements: getKey() using DirectInput
     ├─ LPDIRECTINPUT8 m_pDirectInput
     └─ LPDIRECTINPUTDEVICE8 m_pKeyboardDevice

GameEngineDevice/Include/W3DDevice/GameClient/
  ├─ W3DMouse.h                     [Extension: extends Win32Mouse]
  │  purpose: Adds W3D-specific cursor rendering on top of Win32 mouse
  │  adds: Draw support for 2D surfaces, 3D models, polygons
  │  methods: setRedrawMode(), draw()
  │
  └─ W3DGameWindow.h                [Concrete: extends GameWindow]
     purpose: Window with W3D text rendering
     adds: Render2DSentenceClass m_textRenderer
```

**Abstraction Hierarchy:**
```
┌─────────────────────────────────────────┐
│  Business Logic (GameClient)            │
│  Uses: TheMouse, TheKeyboard            │
└─────────────────┬───────────────────────┘
                  │
        ┌─────────┴──────────┐
        │                    │
   ┌────▼──────┐      ┌──────▼────┐
   │   Mouse   │      │ Keyboard  │   ← ABSTRACT (GameEngine)
   └────▲──────┘      └──────▲────┘
        │                    │
   ┌────┴──────┐      ┌──────┴────┐
   │ Win32Mouse│      │Win32DIKey │   ← CONCRETE (GameEngineDevice)
   │   [W32]   │      │  [Win32]   │
   └────▲──────┘      └──────────┬─┘
        │                        │
   ┌────┴──────┐                │
   │  W3DMouse │                │      ← EXTENDED (W3DDevice)
   │  [W3D]    │                │
   └───────────┘                └─────▶ Direct Input API
```

---

### Source: fighter19-dxvk-port (SDL3 Implementation)

**READY-TO-ADAPT SOURCE CODE:**

```
GeneralsMD/Code/GameEngineDevice/Include/SDL3Device/GameClient/

SDL3Keyboard.h (150 lines)
├─ class SDL3Keyboard : public Keyboard
├─ Key methods:
│  ├─ virtual void init()
│  ├─ virtual void reset()
│  ├─ virtual void update()
│  ├─ virtual void getCapsState()
│  ├─ virtual void getKey(KeyboardIO *key)
│  └─ void addSDLEvent(SDL_Event *ev)           ← EVENT SUBMISSION
├─ Data:
│  ├─ std::vector<SDL_Event> m_events          ← EVENT BUFFER
│  ├─ UnsignedInt m_nextFreeIndex
│  └─ UnsignedInt m_nextGetIndex
└─ Key Helper:
   └─ static KeyDefType ConvertSDLKey(SDL_Keycode keycode)

SDL3Mouse.h (200 lines)
├─ class SDL3Mouse : public Mouse
├─ Key methods:
│  ├─ virtual void init()
│  ├─ virtual void reset()
│  ├─ virtual void update()
│  ├─ virtual void setCursor(MouseCursor cursor)
│  ├─ virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush)
│  ├─ void addSDLEvent(SDL_Event *ev)           ← EVENT SUBMISSION
│  ├─ AnimatedCursor* loadCursorFromFile(const char* file)
│  └─ static void scaleMouseCoordinates(int rawX, int rawY, ...)
├─ Data:
│  ├─ SDL_Event m_eventBuffer[Mouse::NUM_MOUSE_EVENTS]
│  ├─ UnsignedInt m_nextFreeIndex
│  └─ UnsignedInt m_nextGetIndex
└─ Key Structure:
   └─ struct AnimatedCursor { SDL_Cursor*, SDL_Surface*, frame data }

GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/GameClient/

SDL3Keyboard.cpp (310 lines)
├─ Implementation of virtual methods
├─ openKeyboard() → SDL_InitSubSystem(SDL_INIT_EVENTS)
├─ closeKeyboard() → SDL_QuitSubSystem(SDL_INIT_EVENTS)
├─ ConvertSDLKey() → Maps SDL_Keycode to KeyDefType enum (KEY_A, KEY_0, etc)
└─ getKey() → Translates SDL_Event to KeyboardIO structure

SDL3Mouse.cpp (637 lines)
├─ Implementation of virtual methods
├─ getMouseEvent() → Translates SDL_Event to MouseIO structure
├─ loadCursorFromFile() → RIFF/ACON ANI parser
│  ├─ File reading via TheFileSystem
│  ├─ RIFF header parsing
│  ├─ ACON chunk enumeration
│  ├─ Icon frame extraction
│  ├─ SDL_Surface creation per frame
│  └─ SDL_Cursor creation per frame
├─ scaleMouseCoordinates() → DPI-aware position mapping
└─ AnimatedCursor update logic → Frame advancement based on frame rate
```

---

## Adaptation Roadmap: SDL3 → SDL2

### Change Matrix

```
╔═══════════════════════╦═════════════╦═════════════╦═══════════════════════╗
║ Component             ║ SDL3        ║ SDL2        ║ Change Required?      ║
╠═══════════════════════╬═════════════╬═════════════╬═══════════════════════╣
║ Keycode Type          ║ SDL_Keycode ║ SDL_Scancode║ YES - Use scancodes   ║
║ Key Values            ║ SDLK_A      ║ SDL_SCANCODE║ YES - Different enum  ║
║ Keyboard Init         ║ SDL_INIT... ║ SDL_INIT... ║ NO - Identical        ║
║ Event struct          ║ SDL_Event   ║ SDL_Event   ║ NO - Compatible       ║
║ Mouse Position        ║ SDL_GetMS() ║ SDL_GetMS() ║ NO - Identical        ║
║ Mouse Button events   ║ WM_MOUSE... ║ WM_MOUSE... ║ NO - Identical        ║
║ Cursor API            ║ SDL_Cursor  ║ SDL_Cursor  ║ NO - Identical        ║
║ Surface creation      ║ SDL_Surface ║ SDL_Surface ║ NO - Identical        ║
║ File I/O              ║ TheFileSystem║TheFileSystem║ NO - Identical        ║
║ ANI parser            ║ Custom code ║ Custom code ║ NO - Identical        ║
║ Event buffer          ║ vector<...> ║ vector<...> ║ NO - Identical        ║
║ Window creation       ║ SDL_CreateW ║ SDL_CreateW ║ NO - Identical        ║
║ DPI scaling           ║ Custom      ║ Custom      ║ NO - Identical        ║
╚═══════════════════════╩═════════════╩═════════════╩═══════════════════════╝
```

**Effort**: ~100 lines changed out of ~950 total (11% modification rate)

---

## File Creation Checklist

### Phase: Create SDL2Device Layer

```
[  ] Create directory: Core/GameEngineDevice/Include/SDL2Device/GameClient/
[  ] Create directory: Core/GameEngineDevice/Include/SDL2Device/Common/
[  ] Create directory: Core/GameEngineDevice/Source/SDL2Device/GameClient/
[  ] Create directory: Core/GameEngineDevice/Source/SDL2Device/Common/

[  ] Copy and adapt SDL3Keyboard.h → SDL2Keyboard.h
     • Rename class SDL3Keyboard → SDL2Keyboard
     • Change #include "SDL3/SDL.h" → #include <SDL2/SDL.h>
     • Update ConvertSDLKey() → ConvertSDLScancode()
     • Update keycode type SDL_Keycode → SDL_Scancode

[  ] Copy and adapt SDL3Keyboard.cpp → SDL2Keyboard.cpp
     • Update ConvertSDLKey() implementation
     • Update SDL_Keycode → SDL_Scancode throughout
     • Map SDLK_* → SDL_SCANCODE_* (different enum values)
     
[  ] Copy and adapt SDL3Mouse.h → SDL2Mouse.h
     • Rename class SDL3Mouse → SDL2Mouse
     • Change #include "SDL3/SDL.h" → #include <SDL2/SDL.h>
     • Keep AnimatedCursor struct (100% reusable)
     • Keep loadCursorFromFile() (100% reusable)

[  ] Copy and adapt SDL3Mouse.cpp → SDL2Mouse.cpp
     • Keep ANI file parser (100% reusable)
     • Keep AnimatedCursor update logic (100% reusable)
     • Update SDL3-specific API calls (minimal)
     • Keep event translation logic (100% reusable)

[  ] Create SDL2GameWindow.h
     • class SDL2GameWindow : public GameWindow
     • Add SDL_Window* m_window
     • Window creation, destruction, message handling

[  ] Create SDL2GameWindow.cpp
     • Implement SDL_CreateWindow()
     • Implement window event handling
     • Implement window positioning/sizing

[  ] Update Core/GameEngineDevice/CMakeLists.txt
     • Add SDL2Device as target
     • Link SDL2 library
     • Add SDL2Device sources to build

[  ] Create integration point in main game loop
     • Call SDL2Mouse::addSDLEvent(ev) in event loop
     • Call SDL2Keyboard::addSDLEvent(ev) in event loop
```

---

## Code Patterns at a Glance

### Pattern A: Event Buffer (Keyboard & Mouse)

**Used in**: SDL3Keyboard, SDL3Mouse → SDL2Keyboard, SDL2Mouse

```cpp
// HEADER
std::vector<SDL_Event> m_events;
UnsignedInt m_nextFreeIndex = 0;
UnsignedInt m_nextGetIndex = 0;

// CALLED FROM MAIN EVENT LOOP (Add event)
void addSDLEvent(SDL_Event *ev) {
    if (m_nextFreeIndex < MAX_EVENTS)
        m_events[m_nextFreeIndex++] = *ev;
}

// CALLED BY BASE CLASS UPDATE (Retrieve event)
virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush) {
    if (m_nextGetIndex >= m_nextFreeIndex)
        return MOUSE_EVENT_NONE;
    
    translateEvent(m_nextGetIndex, result);
    if (flush) m_nextGetIndex++;
    return MOUSE_OK;
}
```
**Reusability**: 100% - Copy directly, only need SDL2 compatibility

---

### Pattern B: Key Translation (Keyboard)

**Used in**: SDL3Keyboard → SDL2Keyboard

```cpp
// SDL3 VERSION
static KeyDefType ConvertSDLKey(SDL_Keycode keycode) {
    if (keycode >= SDLK_A && keycode <= SDLK_Z)
        return (KeyDefType)(KEY_A + (keycode - SDLK_A));
    else if (keycode >= SDLK_0 && keycode <= SDLK_9) {
        if (keycode == SDLK_0) return KEY_0;
        else return (KeyDefType)(KEY_1 + (keycode - SDLK_1));
    }
    // ... more mappings ...
    return KEY_NONE;
}

// SDL2 ADAPTATION (Change keycode → scancode)
static KeyDefType ConvertSDLScancode(SDL_Scancode scancode) {
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
        return (KeyDefType)(KEY_A + (scancode - SDL_SCANCODE_A));
    else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
        return (KeyDefType)(KEY_1 + (scancode - SDL_SCANCODE_1));
    else if (scancode == SDL_SCANCODE_0)
        return KEY_0;
    // ... more mappings ...
    return KEY_NONE;
}
```
**Reusability**: 95% - Only enum names change, logic identical

---

### Pattern C: Animated Cursor Loader (Mouse)

**Used in**: SDL3Mouse → SDL2Mouse

```cpp
struct AnimatedCursor {
    std::array<SDL_Cursor*, MAX_2D_CURSOR_ANIM_FRAMES> m_frameCursors;
    std::array<SDL_Surface*, MAX_2D_CURSOR_ANIM_FRAMES> m_frameSurfaces;
    int m_currentFrame = 0;
    int m_frameCount = 0;
    int m_frameRate = 0;  // In 1/60th second units
};

AnimatedCursor* loadCursorFromFile(const char* filepath) {
    // 1. Open file via TheFileSystem
    File* file = TheFileSystem->openFile(filepath);
    
    // 2. Read entire file buffer
    Int size = file->size();
    char* buffer = file->readEntireAndClose();
    
    // 3. Parse RIFF header
    RIFFChunk *header = (RIFFChunk*)buffer;
    if (header->id != {'R','I','F','F'}) return NULL;
    
    // 4. Iterate through chunks (ANIM, frame icons, etc)
    RIFFChunk *chunk = getNextChunk(header);
    while (chunk < endOfFile) {
        if (chunk->id == {'a','n','i','h'}) {
            // Parse animation header
            ANIHeader *ani = (ANIHeader*)getChunkData(chunk);
            frameCount = ani->frames;
            frameRate = ani->displayRate;
        } else if (chunk->id == {'i','c','o','n'}) {
            // Parse icon frame - create SDL surface and cursor
            SDL_Surface* surface = parseDIBImage(getChunkData(chunk));
            frameSurfaces[frameIndex] = surface;
            frameCursors[frameIndex] = SDL_CreateCursor(
                surface->pixels, 
                surface->w, 
                surface->h, 
                hotspotX, hotspotY
            );
        }
        chunk = getNextChunk(chunk);
    }
    
    delete[] buffer;
    return cursor;
}
```
**Reusability**: 100% - Identical in SDL2 (SDL_Cursor and SDL_Surface APIs unchanged)

---

## Quick Lookup Table: SDL3 → SDL2 Keycode Changes

```
MOST COMMON KEYS (Complete mapping):

SDL3 (fighter19 reference)      →    SDL2 (GeneralsX Phase 1)
─────────────────────────────────────────────────────────────
SDLK_A                           →    SDL_SCANCODE_A
SDLK_0                           →    SDL_SCANCODE_0
SDLK_RETURN                      →    SDL_SCANCODE_RETURN
SDLK_SPACE                       →    SDL_SCANCODE_SPACE
SDLK_ESCAPE                      →    SDL_SCANCODE_ESCAPE
SDLK_LSHIFT                      →    SDL_SCANCODE_LSHIFT
SDLK_RSHIFT                      →    SDL_SCANCODE_RSHIFT
SDLK_LCTRL                       →    SDL_SCANCODE_LCTRL
SDLK_RCTRL                       →    SDL_SCANCODE_RCTRL
SDLK_LALT                        →    SDL_SCANCODE_LALT
SDLK_RALT                        →    SDL_SCANCODE_RALT
SDLK_UP                          →    SDL_SCANCODE_UP
SDLK_DOWN                        →    SDL_SCANCODE_DOWN
SDLK_LEFT                        →    SDL_SCANCODE_LEFT
SDLK_RIGHT                       →    SDL_SCANCODE_RIGHT
SDLK_TAB                         →    SDL_SCANCODE_TAB
SDLK_DELETE                      →    SDL_SCANCODE_DELETE

Note: SDL_SCANCODE_* values are platform-independent
      They represent physical keys, not symbols
      More suitable for games than SDL_Keycode
```

---

## Dependencies Check

### Required Libraries for SDL2 Implementation

```
Primary Dependencies:
  ✓ SDL2 (from vcpkg)
    - SDL2/SDL.h headers
    - SDL_Init(), SDL_Event, SDL_Window, SDL_Cursor, SDL_Surface
  
  ✓ SDL2_image (from vcpkg) [OPTIONAL for Phase 1]
    - Only needed if using IMG_* functions
    - ANI parser doesn't require it (custom binary parser)

Game-Internal Dependencies:
  ✓ GameEngine/Include/GameClient/Mouse.h (base class)
  ✓ GameEngine/Include/GameClient/Keyboard.h (base class)
  ✓ Common/File.h (file I/O)
  ✓ Common/FileSystem.h (TheFileSystem global)
  ✓ Common/Debug.h (DEBUG_LOG macro)
  ✓ Lib/BaseType.h (UnsignedByte, Int, Bool, etc)

Optional (W3D cursor rendering):
  ? W3DDevice/W3DMouse.h (if extending with 3D cursors)
  ? WWD3D8Wrapper.h (graphics API)
```

---

## Testing Checklist

Once SDL2 implementation is complete:

```
Keyboard Tests:
[ ] All letter keys (A-Z) map correctly
[ ] All number keys (0-9) map correctly
[ ] Modifier keys (Shift, Ctrl, Alt) work
[ ] Special keys (Return, Escape, Tab, etc) work
[ ] Rapid key presses don't lose events
[ ] Key repeat triggers correctly
[ ] Caps lock state detected

Mouse Tests:
[ ] Mouse position updates correctly
[ ] Button down/up events recorded
[ ] Double-click detection works
[ ] Mouse wheel scrolls (if supported)
[ ] Cursor animation plays smoothly
[ ] ANI file loading doesn't crash
[ ] Cursor hotspot is correct
[ ] DPI scaling works (if implemented)

Integration Tests:
[ ] Main menu responds to keyboard/mouse
[ ] Game accepts player input in skirmish
[ ] Command/Conquer camera panning works
[ ] Unit selection works
[ ] Building placement works
[ ] Game doesn't crash with rapid input
```

---

## Critical Files to Read (In Order)

```
1. THIS FILE (You are here)
   → Overview and visual reference

2. REFERENCE_REPOSITORIES_SDL2_INPUT_ANALYSIS.md
   → Deep analysis with code patterns and architecture

3. REFERENCE_FILES_QUICK_GUIDE.md
   → Quick lookup: exact file paths and patterns

4. fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/
   Include/SDL3Device/GameClient/SDL3Keyboard.h
   → Study the pattern (150 lines)

5. fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/
   Source/SDL3Device/GameClient/SDL3Keyboard.cpp
   → Study the implementation (310 lines)

6. fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/
   Include/SDL3Device/GameClient/SDL3Mouse.h
   → Study the pattern (200 lines)

7. fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/
   Source/SDL3Device/GameClient/SDL3Mouse.cpp
   → Study the implementation (637 lines)
```

---

**Created**: January 15, 2026  
**Status**: Reference search complete, implementation guide ready
