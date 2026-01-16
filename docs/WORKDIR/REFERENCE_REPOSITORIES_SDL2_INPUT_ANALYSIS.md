# Reference Repositories: SDL2/Cross-Platform Input & Windowing Analysis

**Document Date**: January 15, 2026  
**Purpose**: Identify reusable SDL2 implementations and cross-platform windowing/input abstractions from reference repositories

---

## Executive Summary

### Key Finding
The **fighter19-dxvk-port** reference contains a **complete SDL3Device layer** with working SDL3 keyboard and mouse implementations that can be adapted for SDL2. The **jmarshall-win64-modern** reference contains the canonical abstraction hierarchy that separates platform-specific code from generic interfaces.

### Recommendation
1. Use the abstraction pattern from `jmarshall-win64-modern` as the reference architecture
2. Adapt the SDL3 implementations from `fighter19-dxvk-port` to SDL2
3. Follow the pattern: `Base Class (GameEngine) → Platform Device → Specific Implementation (SDL2)`

---

## Reference Repository Contents

### 1. jmarshall-win64-modern

**Location**: `references/jmarshall-win64-modern/`  
**Type**: Windows 64-bit modernization with comprehensive fixes  
**Status**: Active reference for correct architecture patterns

#### Device Layer Structure

```
Code/GameEngineDevice/
├── Include/
│   ├── Win32Device/
│   │   ├── Common/
│   │   └── GameClient/
│   │       ├── Win32DIKeyboard.h      [DirectInput keyboard]
│   │       └── Win32Mouse.h            [Win32 message-based mouse]
│   ├── W3DDevice/
│   │   ├── Common/
│   │   ├── GameClient/
│   │   │   ├── W3DGameWindow.h        [W3D window rendering]
│   │   │   └── W3DMouse.h             [W3D cursor rendering]
│   │   └── GameLogic/
│   ├── OpenALAudioDevice/
│   ├── MilesAudioDevice/
│   └── VideoDevice/
└── Source/
    └── [Parallel structure with .cpp implementations]
```

#### Key Input Files

**Base Classes** (GameEngine layer):

1. **Mouse.h** - Base abstract class
   - File: `Code/GameEngine/Include/GameClient/Mouse.h`
   - Lines: 150+ abstract interface
   - Key Methods:
     - `init()` - Initialize mouse system
     - `getMouseEvent(MouseIO *result, Bool flush)` - Retrieve buffered mouse events
     - `setCursor(MouseCursor cursor)` - Set cursor type
     - `capture()` / `releaseCapture()` - Mouse capture control
   - Event Structure: `MouseIO` containing position, button states, wheel position, delta movement
   - Cursor Animation: `MAX_2D_CURSOR_ANIM_FRAMES = 21`, `MAX_2D_CURSOR_DIRECTIONS = 8`

2. **Keyboard.h** - Base abstract class
   - File: `Code/GameEngine/Include/GameClient/Keyboard.h`
   - Lines: 173 total
   - Key Methods:
     - `init()` - Initialize keyboard
     - `getKey(KeyboardIO *key)` - Pure virtual, must implement in derived class
     - `getCapsState()` - Pure virtual, caps lock state
     - `createStreamMessages()` - Convert key states to messages
   - Event Structure: `KeyboardIO` with key code, status, state flags, sequence

3. **GameWindow.h** - Base window abstraction
   - File: `Code/GameEngine/Include/GameClient/GameWindow.h`
   - Lines: 424+ total
   - Pure abstraction for game windows

**Platform-Specific Implementations** (GameEngineDevice layer):

4. **Win32DIKeyboard.h** - DirectInput Keyboard Implementation
   - File: `Code/GameEngineDevice/Include/Win32Device/GameClient/Win32DIKeyboard.h`
   - Derives from: `Keyboard` (base class)
   - Extends: `init()`, `reset()`, `update()`, `getCapsState()`
   - DirectInput specific:
     ```cpp
     LPDIRECTINPUT8 m_pDirectInput;
     LPDIRECTINPUTDEVICE8 m_pKeyboardDevice;
     ```
   - Key method: `virtual void getKey(KeyboardIO *key)` - Implements DirectInput event translation

5. **Win32Mouse.h** - Win32 Message-Based Mouse
   - File: `Code/GameEngineDevice/Include/Win32Device/GameClient/Win32Mouse.h`
   - Derives from: `Mouse` (base class)
   - Event Buffer: `Win32MouseEvent m_eventBuffer[NUM_MOUSE_EVENTS]`
   - Key method: `addWin32Event(UINT msg, WPARAM wParam, LPARAM lParam, DWORD time)` - Called from WndProc
   - Translates WM_* messages to `MouseIO` events

6. **W3DGameWindow.h** - W3D-Specific Window Rendering
   - File: `Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameWindow.h`
   - Derives from: `GameWindow` (base class)
   - Adds W3D text rendering support
   - Extends: Window borders, text, font management

7. **W3DMouse.h** - W3D Cursor Rendering
   - File: `Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DMouse.h`
   - Derives from: `Win32Mouse` (extends, not replaces)
   - Adds W3D-specific cursor features:
     - 2D animated cursor surfaces
     - 3D animated model cursors
     - Polygon-based cursors
   - Multiple cursor rendering modes

#### Abstraction Pattern

```
GameClient (Business Logic)
         ↓
    Mouse / Keyboard (Base Interface)
         ↓
    Win32Mouse / Win32DIKeyboard (Platform Device Layer)
         ↓
    W3DMouse / Existing Keyboard (Rendering-specific extensions)
```

---

### 2. fighter19-dxvk-port

**Location**: `references/fighter19-dxvk-port/`  
**Type**: Linux port with DXVK graphics integration and SDL3 support  
**Status**: Contains working SDL3 implementation for keyboard and mouse

#### Structure Overview

```
GeneralsMD/Code/GameEngineDevice/
├── Include/
│   ├── SDL3Device/             ← NEW SDL3 LAYER
│   │   ├── Common/
│   │   └── GameClient/
│   │       ├── SDL3Keyboard.h
│   │       └── SDL3Mouse.h
│   ├── StdDevice/              [Standard device implementations]
│   ├── Win32Device/            [Original Win32 - for reference]
│   ├── W3DDevice/              [W3D rendering device]
│   └── VideoDevice/            [Video layer]
└── Source/
    └── [Parallel .cpp implementations]
```

#### SDL3 Device Implementation Files

**1. SDL3Keyboard.h** - SDL3-based Keyboard Input
- File: `GeneralsMD/Code/GameEngineDevice/Include/SDL3Device/GameClient/SDL3Keyboard.h`
- **Derives from**: `Keyboard` (base class from GameEngine)
- **Key Additions**:
  ```cpp
  void addSDLEvent(SDL_Event *ev);           // Called from main event loop
  std::vector<SDL_Event> m_events;           // Event buffer
  ```
- **Implementation**: `SDL3Keyboard.cpp` - 310 lines
- **Features**:
  - SDL3 event initialization: `SDL_InitSubSystem(SDL_INIT_EVENTS)`
  - Keycode translation: `ConvertSDLKey(SDL_Keycode keycode)`
  - Maps SDL keycodes to internal `KeyDefType` enum (KEY_A, KEY_0, etc.)
  - Capslock state detection
- **Key Method Pattern**:
  ```cpp
  virtual void getKey(KeyboardIO *key);      // Implements base interface
  void openKeyboard();                       // SDL initialization
  void closeKeyboard();                      // SDL cleanup
  ```

**2. SDL3Mouse.h** - SDL3-based Mouse Input
- File: `GeneralsMD/Code/GameEngineDevice/Include/SDL3Device/GameClient/SDL3Mouse.h`
- **Derives from**: `Mouse` (base class)
- **Key Additions**:
  ```cpp
  void addSDLEvent(SDL_Event *ev);                         // Called from event loop
  SDL_Event m_eventBuffer[Mouse::NUM_MOUSE_EVENTS];       // Event buffer
  AnimatedCursor* loadCursorFromFile(const char* file);   // Load ANI cursors
  static void scaleMouseCoordinates(...);                 // DPI scaling support
  ```
- **Implementation**: `SDL3Mouse.cpp` - 637 lines
- **Features**:
  - **ANI File Support**: Complete RIFF/ACON format parser for animated cursors
    - Reads `.ani` files (animated cursor format)
    - Parses chunks: RIFF header, ACON type, ANIM header, frame data
    - Multi-frame animation with configurable frame rate
  - **Cursor Management**:
    ```cpp
    struct AnimatedCursor {
        std::array<SDL_Cursor*, MAX_2D_CURSOR_ANIM_FRAMES> m_frameCursors;
        std::array<SDL_Surface*, MAX_2D_CURSOR_ANIM_FRAMES> m_frameSurfaces;
        int m_currentFrame;
        int m_frameCount;
        int m_frameRate;  // 1/60th second units
    };
    ```
  - **Coordinate Scaling**: Handles DPI-aware mouse position scaling
  - **Event Translation**: Converts SDL mouse events to `MouseIO` structure

#### SDL3 Implementation Patterns

**Keyboard Event Translation Pattern** (from SDL3Keyboard.cpp):
```cpp
static KeyDefType ConvertSDLKey(SDL_Keycode keycode) {
    if (keycode >= SDLK_A && keycode <= SDLK_Z)
        return (KeyDefType)(KEY_A + (keycode - SDLK_A));
    else if (keycode >= SDLK_0 && keycode <= SDLK_9)
        // Handle 0-9 with special case for SDLK_0
        return (KeyDefType)(KEY_1 + (keycode - SDLK_1));
    // ... more key mappings
}
```

**Mouse Event Pattern** (from SDL3Mouse.cpp):
```cpp
void SDL3Mouse::addSDLEvent(SDL_Event *ev) {
    // Add SDL event to m_eventBuffer
    // Called from main game loop when SDL event received
}

virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush) {
    // Called by base class update()
    // Translate SDL_Event → MouseIO
    // Handles button states, position, wheel, deltas
}
```

#### Cursor Animation Implementation
The SDL3Mouse provides complete animated cursor support:
- Reads `.ani` (Windows animated cursor) files
- Parses RIFF/ACON binary format
- Creates SDL surfaces and cursors per frame
- Auto-animates based on frame rate
- Supports multiple directional cursors

---

## Recommended Adaptation: SDL2 vs SDL3

### Key Differences (SDL3 → SDL2)

The fighter19 code uses **SDL3** (latest), but GeneralsX Phase 1 requires **SDL2**:

| Feature | SDL2 | SDL3 | Adaptation |
|---------|------|------|-----------|
| Event System | `SDL_Event` union | `SDL_Event` struct | Compatible - only minor type changes |
| Keyboard | `SDL_SCANCODE_*` | `SDL_Keycode` | Use SDL2's `SDL_GetKeyboardState()` and scan codes |
| Mouse | `SDL_GetMouseState()` | Same | Compatible |
| Cursor | `SDL_CreateCursor()` | `SDL_CreateCursor()` | Compatible - same API |
| Surface | `SDL_Surface` | Same | Compatible |
| Init | `SDL_Init()` | `SDL_Init()` | Identical |

**Adaptation Strategy**:
- Keep the abstraction layer structure (100% reusable)
- Convert keycode translation to use SDL2's scan codes
- SDL cursor creation is identical between versions
- ANI file parsing is platform-independent (fully reusable)

---

## File Mapping for Implementation

### Must Implement (based on references)

#### 1. Base Input Classes (Copy from jmarshall reference)
- [GameEngine/Include/GameClient/Mouse.h](../../../Code/GameEngine/Include/GameClient/Mouse.h)
- [GameEngine/Include/GameClient/Keyboard.h](../../../Code/GameEngine/Include/GameClient/Keyboard.h)
- [GameEngine/Include/GameClient/GameWindow.h](../../../Code/GameEngine/Include/GameClient/GameWindow.h)

#### 2. Device Layer (Create new SDL2Device)
**Locations to create:**
- `Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Keyboard.h`
- `Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Mouse.h`
- `Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Keyboard.cpp`
- `Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Mouse.cpp`

**Adapt from**:
- `fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Include/SDL3Device/` → SDL2
- Keep 95% of logic, only change SDL3-specific API calls

#### 3. Window Management (Use W3D as base)
- Adapt `jmarshall-win64-modern/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameWindow.h`
- Requires SDL2 window creation (not DirectX 8)

---

## Code Patterns - Ready to Reuse

### Pattern 1: Event Buffer Management (Mouse/Keyboard)

From fighter19's SDL3Mouse:
```cpp
// Event buffer structure
std::vector<SDL_Event> m_events;           // Dynamic event queue
UnsignedInt m_nextFreeIndex;               // Insert position
UnsignedInt m_nextGetIndex;                // Retrieve position

// Event submission (called from main loop)
void addSDLEvent(SDL_Event *ev) {
    if (m_nextFreeIndex < NUM_EVENTS)
        m_events[m_nextFreeIndex++] = *ev;
}

// Event retrieval (called by getMouseEvent/getKey)
virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush) {
    if (m_nextGetIndex >= m_nextFreeIndex)
        return MOUSE_EVENT_NONE;
    
    translateEvent(m_nextGetIndex, result);
    if (flush) m_nextGetIndex++;
    return MOUSE_OK;
}
```

### Pattern 2: Key Translation (SDL → Internal)

From fighter19's SDL3Keyboard:
```cpp
static KeyDefType ConvertSDLKey(SDL_Keycode keycode) {
    // Map ranges efficiently
    if (keycode >= SDLK_A && keycode <= SDLK_Z)
        return (KeyDefType)(KEY_A + (keycode - SDLK_A));
    
    // Handle special cases
    if (keycode == SDLK_RETURN)
        return KEY_RETURN;
    
    // Default unmapped keys
    return KEY_NONE;
}
```

For SDL2 adaptation, use `SDL_SCANCODE_*` instead:
```cpp
static KeyDefType ConvertSDLScancode(SDL_Scancode scancode) {
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
        return (KeyDefType)(KEY_A + (scancode - SDL_SCANCODE_A));
    // ... etc
}
```

### Pattern 3: Animated Cursor Support

From fighter19's SDL3Mouse (fully reusable):
```cpp
struct AnimatedCursor {
    std::array<SDL_Cursor*, MAX_2D_CURSOR_ANIM_FRAMES> m_frameCursors;
    std::array<SDL_Surface*, MAX_2D_CURSOR_ANIM_FRAMES> m_frameSurfaces;
    int m_frameCount;
    int m_frameRate;  // In 1/60th second units
};

AnimatedCursor* loadCursorFromFile(const char* filepath) {
    // Parse RIFF/ACON format
    // Create SDL surfaces from icon chunks
    // Create SDL cursors per frame
    return animCursor;
}

void updateCursor() {
    // Auto-advance frame based on frame rate and elapsed time
    if (currentFrame < frameCount)
        SDL_SetCursor(m_frameCursors[currentFrame]);
}
```

This code works identically in SDL2 and SDL3 (SDL_Cursor API is unchanged).

### Pattern 4: Class Hierarchy (Extensible Design)

```cpp
// Base class (GameEngine) - ABSTRACT
class Mouse : public SubsystemInterface {
    virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush) = 0;
    virtual void setCursor(MouseCursor cursor) = 0;
};

// Device implementation (GameEngineDevice) - CONCRETE
class SDL2Mouse : public Mouse {
    virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush);  // SDL2-specific
    virtual void setCursor(MouseCursor cursor);                        // SDL2-specific
    
    void addSDLEvent(SDL_Event *ev);  // Called from event loop
    SDL_Event m_eventBuffer[NUM_MOUSE_EVENTS];
};

// Rendering extension (optional) - FURTHER EXTENDS
class W3DMouse : public SDL2Mouse {
    // Could add W3D rendering on top of SDL2 base
    virtual void draw();  // W3D-specific cursor drawing
};
```

---

## Integration Checklist

- [ ] **Review Phase**: Understand jmarshall architecture (Mouse → Win32Mouse → W3DMouse pattern)
- [ ] **Create SDL2Device Directory**: Parallel to existing SDL3Device/Win32Device
- [ ] **Implement SDL2Keyboard.h**: Adapt from SDL3Keyboard (keycode → scancode)
- [ ] **Implement SDL2Keyboard.cpp**: ~300 lines (copy from SDL3, change SDL APIs)
- [ ] **Implement SDL2Mouse.h**: Adapt from SDL3Mouse (nearly identical)
- [ ] **Implement SDL2Mouse.cpp**: ~600 lines (ANI parser fully reusable)
- [ ] **Create SDLGameWindow**: New window management using SDL2 window APIs
- [ ] **Update CMakeLists**: Add SDL2Device as build target
- [ ] **Test Event Loop Integration**: Wire SDL_Event → addSDLEvent() in main game loop
- [ ] **Test Cursor Rendering**: Verify ANI files load and animate correctly
- [ ] **Test Keyboard Mapping**: Verify all keys translate correctly to internal KeyDefs
- [ ] **Test Mouse Capture**: Verify capture/release works for menu vs. gameplay

---

## Files NOT Found

- **dsalzner-linux-attempt/** - Reference does not exist
- **Native SDL2 window management** - Not found in references; requires new implementation
- **SDL2-specific audio device** - References use Miles/OpenAL, not SDL audio

These items will need custom implementation following established patterns.

---

## Conclusion

**Key Takeaways:**

1. **Complete pattern reference**: jmarshall-win64-modern defines the correct abstraction hierarchy
2. **Working SDL implementation**: fighter19-dxvk-port provides 95% reusable SDL3 code (adaptable to SDL2)
3. **Animated cursors**: Full ANI file parser is platform-independent and fully reusable
4. **Minimal porting effort**: Only SDL3 API calls need changing; business logic remains identical
5. **Extensibility**: Pattern allows future W3D cursor rendering on top of SDL2 base

**Next Steps**: Begin with SDL2Mouse and SDL2Keyboard classes, adapting from fighter19 SDL3 implementations.
