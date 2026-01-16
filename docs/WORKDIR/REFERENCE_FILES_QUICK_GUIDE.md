]\# Quick Reference: SDL2 & Cross-Platform Implementation Files

## File Locations in Reference Repositories

### jmarshall-win64-modern: Base Classes & Architecture Pattern

**Base Input Interfaces (GameEngine Layer):**
```
references/jmarshall-win64-modern/Code/GameEngine/Include/GameClient/
├── Mouse.h                    [379 lines] Abstract mouse interface
├── Keyboard.h                 [173 lines] Abstract keyboard interface
└── GameWindow.h               [424+ lines] Abstract window interface
```

**Platform Device Layer (GameEngineDevice):**
```
references/jmarshall-win64-modern/Code/GameEngineDevice/

WIN32 IMPLEMENTATIONS:
├── Include/Win32Device/GameClient/
│   ├── Win32Mouse.h           Abstract Win32 mouse (message-based)
│   └── Win32DIKeyboard.h      DirectInput keyboard implementation
└── Source/Win32Device/GameClient/
    ├── Win32Mouse.cpp         Message event handling
    └── Win32DIKeyboard.cpp    DirectInput event translation

W3D RENDERING EXTENSIONS:
├── Include/W3DDevice/GameClient/
│   ├── W3DGameWindow.h        Window with W3D rendering
│   └── W3DMouse.h             Mouse with W3D cursor support
└── Source/W3DDevice/GameClient/
    ├── W3DGameWindow.cpp
    └── W3DMouse.cpp
```

---

### fighter19-dxvk-port: SDL3 Implementation (Adapt to SDL2)

**SDL3 DEVICE LAYER (PRODUCTION REFERENCE):**
```
references/fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/

Include/SDL3Device/GameClient/
├── SDL3Keyboard.h             ← ADAPT TO SDL2
│   Key Classes: SDL3Keyboard extends Keyboard
│   Lines: ~150
│   Key Methods:
│   - void addSDLEvent(SDL_Event *ev)
│   - virtual void getKey(KeyboardIO *key)
│   - static KeyDefType ConvertSDLKey(SDL_Keycode)
│
└── SDL3Mouse.h                ← ADAPT TO SDL2
    Key Classes: SDL3Mouse extends Mouse
    Lines: ~200
    Key Methods:
    - void addSDLEvent(SDL_Event *ev)
    - virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush)
    - AnimatedCursor* loadCursorFromFile(const char* file)
    - static void scaleMouseCoordinates(int rawX, int rawY, ...)

Source/SDL3Device/GameClient/
├── SDL3Keyboard.cpp           [310 lines total]
│   Key Implementation:
│   - ConvertSDLKey() function - Maps SDL keycodes to KeyDefType enum
│   - Event buffer management - std::vector<SDL_Event>
│   - SDL_InitSubSystem(SDL_INIT_EVENTS) / SDL_QuitSubSystem()
│
└── SDL3Mouse.cpp              [637 lines total]
    Key Implementation:
    - ANI file parser - Full RIFF/ACON format parsing
    - AnimatedCursor struct - Manages multi-frame cursors
    - Coordinate scaling for DPI awareness
    - Event translation SDL → MouseIO

REFERENCE DIRECTORIES (for comparison):
├── Include/Win32Device/GameClient/
│   ├── Win32DIKeyboard.h
│   └── Win32Mouse.h
├── Include/StdDevice/
└── Include/W3DDevice/
```

---

## SDL2 Implementation Plan - File Structure to Create

```
Core/GameEngineDevice/

NEW SDL2 DEVICE LAYER:
Include/SDL2Device/GameClient/
├── SDL2Keyboard.h             [~150 lines, adapt from SDL3Keyboard.h]
│   Contents:
│   - class SDL2Keyboard : public Keyboard
│   - void addSDLEvent(SDL_Event *ev)
│   - virtual void getKey(KeyboardIO *key)
│   - static KeyDefType ConvertSDLScancode(SDL_Scancode)
│
└── SDL2Mouse.h                [~200 lines, adapt from SDL3Mouse.h]
    Contents:
    - class SDL2Mouse : public Mouse
    - void addSDLEvent(SDL_Event *ev)
    - virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush)
    - AnimatedCursor* loadCursorFromFile(const char* file)
    - static void scaleMouseCoordinates(...)

Source/SDL2Device/GameClient/
├── SDL2Keyboard.cpp           [~300 lines, adapt from SDL3Keyboard.cpp]
└── SDL2Mouse.cpp              [~600 lines, adapt from SDL3Mouse.cpp]
    - ANI parser is 100% reusable
    - AnimatedCursor handling is 100% reusable
    - Only SDL3 → SDL2 API changes needed

WINDOW MANAGEMENT (NEW):
Include/SDL2Device/GameClient/
└── SDL2GameWindow.h           [NEW, ~150 lines]
    - class SDL2GameWindow : public GameWindow
    - SDL_Window management
    - Window event handling

Source/SDL2Device/GameClient/
└── SDL2GameWindow.cpp         [NEW, ~150 lines]
```

---

## Key Code Patterns from fighter19-dxvk-port

### Pattern 1: Keyboard Key Translation (SDL3 → SDL2)

**SDL3 Version (fighter19):**
```cpp
static KeyDefType ConvertSDLKey(SDL_Keycode keycode) {
    if (keycode >= SDLK_A && keycode <= SDLK_Z)
        return (KeyDefType)(KEY_A + (keycode - SDLK_A));
    else if (keycode >= SDLK_0 && keycode <= SDLK_9) {
        if (keycode == SDLK_0) return KEY_0;
        else return (KeyDefType)(KEY_1 + (keycode - SDLK_1));
    }
    // ... more key mappings
}
```

**SDL2 Adaptation:**
```cpp
static KeyDefType ConvertSDLScancode(SDL_Scancode scancode) {
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
        return (KeyDefType)(KEY_A + (scancode - SDL_SCANCODE_A));
    else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
        return (KeyDefType)(KEY_1 + (scancode - SDL_SCANCODE_1));
    else if (scancode == SDL_SCANCODE_0)
        return KEY_0;
    // ... more key mappings
}
```

### Pattern 2: Event Buffer & Translation (100% Reusable)

```cpp
// Header
std::vector<SDL_Event> m_events;
UnsignedInt m_nextFreeIndex;
UnsignedInt m_nextGetIndex;

// Method for event submission (called from main loop)
void addSDLEvent(SDL_Event *ev) {
    if (m_nextFreeIndex < NUM_EVENTS)
        m_events[m_nextFreeIndex++] = *ev;
}

// Method for event retrieval (base class calls this)
virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush) {
    if (m_nextGetIndex >= m_nextFreeIndex)
        return MOUSE_EVENT_NONE;
    
    translateEvent(m_nextGetIndex, result);
    if (flush) m_nextGetIndex++;
    return MOUSE_OK;
}
```

### Pattern 3: Animated Cursor Loading (100% Reusable)

```cpp
struct AnimatedCursor {
    std::array<SDL_Cursor*, MAX_2D_CURSOR_ANIM_FRAMES> m_frameCursors;
    std::array<SDL_Surface*, MAX_2D_CURSOR_ANIM_FRAMES> m_frameSurfaces;
    int m_currentFrame = 0;
    int m_frameCount = 0;
    int m_frameRate = 0;  // 1/60th second units
    
    ~AnimatedCursor() {
        for (int i = 0; i < MAX_2D_CURSOR_ANIM_FRAMES; i++) {
            if (m_frameCursors[i]) SDL_DestroyCursor(m_frameCursors[i]);
            if (m_frameSurfaces[i]) SDL_DestroySurface(m_frameSurfaces[i]);
        }
    }
};

AnimatedCursor* loadCursorFromFile(const char* filepath) {
    File* file = TheFileSystem->openFile(filepath);
    Int size = file->size();
    char* file_buffer = file->readEntireAndClose();
    
    // Parse RIFF header
    RIFFChunk *riff_header = (RIFFChunk*)file_buffer;
    if (riff_header->id != {'R','I','F','F'}) return NULL;
    
    // Parse ACON chunks
    AnimatedCursor* cursor = new AnimatedCursor();
    // ... parse frames, create surfaces, create cursors ...
    
    delete[] file_buffer;
    return cursor;
}
```

---

## SDL2 vs SDL3 API Compatibility

| Feature | SDL2 | SDL3 | Adaptation | Reusability |
|---------|------|------|-----------|-------------|
| Event System | `SDL_Event` union | `SDL_Event` struct | Minor changes | 95% |
| Keyboard Input | `SDL_GetKeyboardState()` + scancodes | Same + keycodes | Use scancodes instead | 90% |
| Mouse Position | `SDL_GetMouseState()` | Same | Identical | 100% |
| Mouse Buttons | `SDL_MOUSEBUTTONDOWN/UP` | Same | Identical | 100% |
| Cursor API | `SDL_CreateCursor()` | Same | Identical | 100% |
| Surface | `SDL_Surface` | Same | Identical | 100% |
| Init/Quit | `SDL_Init()` / `SDL_Quit()` | Same | Identical | 100% |
| Window | `SDL_CreateWindow()` | Same | Identical | 100% |
| File I/O | Use TheFileSystem, not SDL | Same | Identical | 100% |

**Bottom line**: 95%+ of fighter19's SDL3 code is directly reusable with only SDL3 API calls changed to SDL2 equivalents.

---

## What Needs Custom Implementation (NEW)

1. **SDL2GameWindow.h/cpp** - Window management using SDL_Window
   - Event loop integration
   - Window message handling
   - DPI awareness (optional for Phase 1)

2. **CMakeLists.txt** - Build configuration for SDL2Device
   - Link SDL2 library
   - Define SDL2Device as target

3. **Main Event Loop** - Integration point
   - SDL_PollEvent() loop
   - Call SDL2Mouse::addSDLEvent() and SDL2Keyboard::addSDLEvent()
   - Dispatch to game subsystems

---

## Reference Implementations by Complexity

### Easiest to Implement (Copy & Adapt)
1. **SDL2Keyboard.h** - ~150 lines, straightforward adaptation
2. **Keyboard key translation** - Simple table-based mapping

### Medium Complexity (Copy with Minor Changes)
3. **SDL2Mouse.h** - ~200 lines, mostly identical to SDL3
4. **Event buffer management** - Copy/paste with no functional changes

### Most Complex (Understand Pattern, Then Implement)
5. **AnimatedCursor parser** - 300+ lines, fully independent of SDL version
6. **SDL2GameWindow** - New window management (can reference W3DGameWindow pattern)

---

## Next Steps

1. Read full analysis: [REFERENCE_REPOSITORIES_SDL2_INPUT_ANALYSIS.md](REFERENCE_REPOSITORIES_SDL2_INPUT_ANALYSIS.md)
2. Compare file hierarchies and abstraction patterns
3. Create SDL2Device directory structure
4. Implement SDL2Keyboard.h/cpp (start easiest)
5. Implement SDL2Mouse.h/cpp (animate cursors second)
6. Create SDL2GameWindow.h/cpp (new but follows W3D pattern)
7. Integrate into main event loop
8. Test with Phase 1 acceptance criteria
