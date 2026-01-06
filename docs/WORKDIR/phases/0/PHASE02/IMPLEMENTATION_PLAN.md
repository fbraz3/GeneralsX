# Phase 01: SDL2 Window & Event Loop - Implementation Plan

**Phase**: 01  
**Title**: SDL2 Window & Event Loop  
**Area**: OS API Layer (win32_sdl_api_compat.h)  
**Scope**: MEDIUM  
**Status**: IN PROGRESS  
**Dependencies**: Phase 00, Phase 00.5

---

## Session Plan

This document tracks the Phase 01 implementation session for creating a cross-platform SDL2-based window and event loop system.

---

## Key Objectives

1. Create `win32_sdl_api_compat.h` - SDL2 compatibility layer header
2. Integrate SDL2 into CMake build system
3. Implement cross-platform window creation via SDL2
4. Replace Windows message loop with SDL2 event polling
5. Implement input event translation (SDL2 → Win32 constants)
6. Support window sizing, fullscreen, and VSync configuration
7. Add comprehensive documentation and testing

---

## Architecture Overview

### Three-Layer Compatibility System

```text
┌─────────────────────────────────────────────────────┐
│ Application Layer (WinMain, GameEngine)             │
│   Uses: HWND, MSG, traditional Win32 message loop   │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│ Layer 1: Core Compatibility (win32_sdl_api_compat.h)│
│   ├─ SDL2 window creation → HWND-like handle        │
│   ├─ SDL2 event polling → MSG-like structure        │
│   ├─ Input translation → Win32 key codes            │
│   └─ Window management → fullscreen, VSync, sizing  │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│ Layer 2: Platform Backend (SDL2 + Graphics Backend) │
│   ├─ SDL2: Window creation, event polling           │
│   ├─ Metal: Graphics rendering (macOS)              │
│   └─ OpenGL: Graphics rendering (Linux, Windows)    │
└─────────────────────────────────────────────────────┘
```

### Pattern: `source_dest_type_compat`

**Example**: `win32_sdl_api_compat`

- **source**: `win32` (Windows API target)
- **dest**: `sdl` (SDL2 implementation)
- **type**: `api` (functional API compatibility)
- **purpose**: Provide Win32-compatible interface over SDL2

---

## Implementation Details

### 1. Core Layer: win32_sdl_api_compat.h

**Location**: `Core/Libraries/Source/WWVegas/WW3D2/win32_sdl_api_compat.h`

**Responsibilities**:

1. **Window Creation**
   - `CreateWindow_SDL2()` - Create SDL2 window with Win32-like parameters
   - Handle window flags, sizing, positioning
   - Return HWND-like handle (opaque pointer to SDL_Window)

2. **Event Loop Integration**
   - `PollEvents_SDL2()` - Poll SDL2 events into MSG-like structure
   - Handle SDL_QUIT, SDL_KEYDOWN, SDL_MOUSEMOTION, etc.
   - Translate SDL2 event data to Win32 constants

3. **Input Event Translation**
   - SDL_Keycode → VK_* constants (VK_ESCAPE, VK_RETURN, etc.)
   - SDL_MouseButton → WM_LBUTTONDOWN, WM_RBUTTONDOWN, etc.
   - Mouse coordinates → LPARAM encoding (LOWORD/HIWORD)

4. **Window Management**
   - SetFullscreen()
   - SetVSync()
   - SetWindowSize()
   - GetWindowHandle()

### 2. Integration Points

**File**: `GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp`

**Function**: `Win32GameEngine::serviceWindowsOS()`

**Changes**:

```cpp
void Win32GameEngine::serviceWindowsOS( void )
{
    #ifdef _WIN32
        // Windows: Traditional Windows message loop
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
            // ... existing Windows code ...
        }
    #else
        // Cross-platform: SDL2 event loop
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    setQuitting(true);
                    break;
                case SDL_KEYDOWN:
                    // ... handle keyboard ...
                    break;
                case SDL_MOUSEMOTION:
                    // ... handle mouse ...
                    break;
                // ... more cases ...
            }
        }
    #endif
}
```

### 3. CMake Integration

**File**: `CMakeLists.txt` (root level) or `cmake/config.cmake`

**Changes**:

```cmake
# Find SDL2
find_package(SDL2 3.0 REQUIRED)

# Link SDL2 to targets
target_link_libraries(${TARGET} PUBLIC SDL2::SDL2)

# Optional: Set SDL2 definitions
option(ENABLE_SDL2 "Enable SDL2 for cross-platform window/input" ON)
if(ENABLE_SDL2)
    add_definitions(-DENABLE_SDL2)
endif()
```

---

## Development Steps

### Step 1: Create win32_sdl_api_compat.h Header

- [ ] Create file structure
- [ ] Define SDL2 window handle wrapper (HWND-compatible)
- [ ] Define SDL2 message structure (MSG-compatible)
- [ ] Implement window creation functions
- [ ] Implement event polling functions
- [ ] Implement input translation functions

### Step 2: CMake Integration

- [ ] Add SDL2 find_package directive
- [ ] Link SDL2 to GeneralsXZH target
- [ ] Link SDL2 to GeneralsX target
- [ ] Validate CMake configuration
- [ ] Test cmake --preset macos-arm64

### Step 3: Adapt serviceWindowsOS()

- [ ] Update GeneralsMD Win32GameEngine.cpp
- [ ] Update Generals Win32GameEngine.cpp
- [ ] Add SDL_PollEvent loop
- [ ] Implement event case handling
- [ ] Test event polling without crashes

### Step 4: Input Event Translation

- [ ] Implement key code translation table
- [ ] Implement mouse button translation
- [ ] Implement mouse coordinate encoding
- [ ] Test keyboard input
- [ ] Test mouse input

### Step 5: Build & Test

- [ ] Clean build: `rm -rf build/macos-arm64 && cmake --preset macos-arm64`
- [ ] Compile GeneralsXZH: `cmake --build build/macos-arm64 --target GeneralsXZH -j 4`
- [ ] Deploy executable
- [ ] Runtime test: window creation, event loop, input handling

### Step 6: Documentation

- [ ] Complete README.md with actual results
- [ ] Add code examples
- [ ] Document known issues
- [ ] Add testing summary

---

## Success Criteria

### Build & Compilation

- [ ] Compiles without new errors on macOS ARM64
- [ ] All platforms build successfully (macOS x86_64, Linux, Windows)
- [ ] No regression in existing functionality
- [ ] ccache improves incremental build times

### Runtime Behavior

- [ ] Window creation works cross-platform
- [ ] Event loop polls SDL2 events without crashes
- [ ] Input events translated correctly to Win32 constants
- [ ] Window can be resized, minimized, fullscreen toggled
- [ ] Application closes cleanly on quit
- [ ] No memory leaks (valgrind/ASan clean)

### Testing

- [ ] Unit tests pass (event translation logic)
- [ ] Integration tests pass (window + event loop + input)
- [ ] Cross-platform validation on macOS (ARM64 & x86_64), Linux, Windows
- [ ] Performance acceptable (no frame drops from event handling)

---

## Known Considerations

### Existing State

The project currently has:

- DirectX 8 mock layer (`d3d8.h`)
- Windows compatibility layer structure (mentioned in docs)
- SDL2 already used in some build systems (evident from CMakeLists.txt references)
- Graphics backend abstraction layer (Phase 38)

### Potential Challenges

1. **SDL2 Version Compatibility**: Need to check SDL2 3.0+ vs 2.28
2. **Input System Integration**: Mouse input system may be separate from event loop
3. **Window Handle Compatibility**: Some code may directly use HWND
4. **Event Order**: SDL2 event order may differ from Win32 queue model
5. **macOS Specific Issues**: Metal backend integration with SDL2

### Design Decisions

1. **HWND Abstraction**: Keep HWND as opaque pointer to SDL_Window internally
2. **MSG Structure**: Create MSG-compatible struct from SDL_Event data
3. **Backward Compatibility**: Provide both Windows (GetMessage) and SDL2 (SDL_PollEvent) paths
4. **Conditional Compilation**: Use `#ifdef _WIN32` for platform-specific code

---

## Testing Strategy

### Unit Tests

- [ ] Input translation functions (key codes, mouse buttons)
- [ ] Event filtering/routing logic
- [ ] Window parameter validation

### Integration Tests

- [ ] SDL2 window creation and destruction
- [ ] Event loop stability over 1000+ frames
- [ ] Input event propagation to game systems
- [ ] Window resize event handling
- [ ] Fullscreen toggle

### Cross-Platform Validation

- [ ] macOS ARM64 (primary)
- [ ] macOS x86_64 (secondary)
- [ ] Linux x86_64 (testing)
- [ ] Windows x86_64 (if available)

---

## Estimated Timeline

- Header creation & SDL2 functions: 30 mins
- CMake integration: 20 mins
- serviceWindowsOS() adaptation: 40 mins
- Input translation implementation: 50 mins
- Build & testing: 40 mins
- Documentation: 20 mins

**Total Estimated**: ~3 hours

---

## Reference Materials

- **Comprehensive Vulkan Plan**: `/docs/COMPREHENSIVE_VULKAN_PLAN.md`
- **Critical Lessons**: `/docs/WORKDIR/lessons/LESSONS_LEARNED.md`
- **Build Instructions**: `/docs/MACOS_BUILD_INSTRUCTIONS.md`
- **SDL2 Documentation**: See SDL2 wiki online
- **Input System**: Need to review existing input handling code

---

## Session Notes

(To be updated as implementation progresses)

### Entry State

- Phase 00 & 00.5 complete
- No win32_sdl_api_compat.h exists yet
- Win32GameEngine still uses Windows message loop
- CMake may or may not have SDL2 integration

### Progress Log

- ...

---

## Deliverables Checklist

- [ ] `Core/Libraries/Source/WWVegas/WW3D2/win32_sdl_api_compat.h` - Header with SDL2 functions
- [ ] Updated `GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp`
- [ ] Updated `Generals/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp`
- [ ] CMake configuration with SDL2 integration
- [ ] Complete `docs/WORKDIR/phases/0/PHASE01/README.md` with results
- [ ] Unit tests in `tests/` directory (if applicable)
- [ ] Commit to vulkan-port branch

---

**Next Phase**: Phase 02 (Texture System)
