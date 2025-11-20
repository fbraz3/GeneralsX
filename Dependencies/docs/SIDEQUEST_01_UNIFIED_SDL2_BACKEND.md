# Side Quest 01: Unified SDL2 Backend - Architecture & Implementation Plan

**Status**: Planning Document (NOT STARTED)  
**Priority**: MEDIUM (Foundation improvement)  
**Complexity**: MEDIUM-HIGH  
**Estimated Effort**: 40-60 hours  
**Target Timeline**: Phase 41-45 (Post Phase 40 integration)

---

## Executive Summary

Currently, GeneralsX uses a hybrid approach with SDL2 compatibility layers (`win32_sdl_api_compat.h/cpp`) providing basic window and event management, but **graphics rendering remains platform-specific** (Metal for macOS, OpenGL for Linux/Windows). This side quest proposes consolidating SDL2 as a **unified backend for window management, event handling, and graphics context creation** across all three platforms (macOS, Linux, Windows).

**Goal**: Replace platform-specific window/event handling with a single, clean SDL2 abstraction that reduces code duplication and improves maintainability.

---

## Current Architecture Analysis

### What We Have ✅

**Phase 02 - SDL2 Window & Event Loop** (COMPLETE):

```text
win32_sdl_api_compat.h/cpp
├── SDL2 window creation (SDL_CreateWindow)
├── Event polling (SDL_PollEvent)
├── Input translation (SDL2 keycode → VK_*)
├── Mouse coordinate encoding
└── VSync/fullscreen support
```

**Integration Points**:

- `GeneralsMD/Code/GameEngine/Source/WinMain.cpp` - Uses SDL2_CreateWindow()
- `GeneralsMD/Code/GameEngine/Source/Win32GameEngine.cpp` - Uses SDL2_PollEvent() in serviceWindowsOS()
- CMake: `cmake/sdl2.cmake` - Finds and links SDL2

**Compilation Guard**: `#ifdef _WIN32` for Windows-specific code, SDL2 fallback for others

### What's Still Platform-Specific ❌

**Graphics Context Creation**:

- **macOS**: Metal context via `MTLDevice`, `MTLCommandQueue`
- **Linux**: OpenGL context via GLFW or custom X11 setup
- **Windows**: DirectX 8 or OpenGL context

**Window Properties**:

- **macOS**: Native window handle via `SDL_GetWindowFromID()`
- **Linux**: X11 Window ID
- **Windows**: HWND

**Event Loop Integration**:

- Graphics setup happens AFTER SDL2 window creation
- Platform-specific rendering loop in each target

---

## Proposed Unified Architecture

### Three-Tier Design

```text
┌─────────────────────────────────────────────────────┐
│ Application Layer                                   │
│ (WinMain, Win32GameEngine, W3DDisplay)             │
│ Uses: Abstract GraphicsContext interface           │
└────────────────────┬────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────┐
│ SDL2 Abstraction Layer (NEW)                        │
│ (sdl2_graphics_backend.h/cpp)                       │
│ ├─ SDL2GraphicsContext (platform-agnostic)         │
│ ├─ Window management                                │
│ ├─ Event loop coordination                          │
│ ├─ Graphics context factory                         │
│ └─ Platform abstraction                            │
└────────────────────┬────────────────────────────────┘
                     │
        ┌────────────┼────────────┐
        ▼            ▼            ▼
   ┌────────┐  ┌─────────┐  ┌──────────┐
   │ Metal  │  │ OpenGL  │  │OpenGL/DX │
   │(macOS) │  │(Linux)  │  │(Windows) │
   └────────┘  └─────────┘  └──────────┘
```

### Layer 2: SDL2 Graphics Backend Specification

**File**: `Core/Libraries/Source/WWVegas/WW3D2/sdl2_graphics_backend.h`

```cpp
// Platform-agnostic graphics context interface
typedef struct {
    void (*destroy)(void* context);
    void (*make_current)(void* context);
    void (*swap_buffers)(void* context);
    void (*resize)(void* context, int width, int height);
} GraphicsContextVtable;

// SDL2-based graphics context
typedef struct {
    SDL_Window* window;
    void* native_context;      // MTLDevice*, GLXContext, HGLRC
    GraphicsContextVtable vtable;
    int width, height;
    int vsync_enabled;
} SDL2GraphicsContext;

// Factory function (platform-specific implementation)
SDL2GraphicsContext* SDL2_CreateGraphicsContext(
    const char* title,
    int width,
    int height,
    int flags
);

void SDL2_DestroyGraphicsContext(SDL2GraphicsContext* ctx);
void SDL2_MakeContextCurrent(SDL2GraphicsContext* ctx);
void SDL2_SwapBuffers(SDL2GraphicsContext* ctx);
```

---

## Implementation Phases

### Phase 1: Core SDL2 Graphics Backend (15-20 hours)

**Objective**: Create platform-agnostic graphics context abstraction

**Deliverables**:

1. `sdl2_graphics_backend.h` - Header with interfaces and factory
2. `sdl2_graphics_backend_metal.cpp` - macOS Metal implementation
3. `sdl2_graphics_backend_opengl.cpp` - Linux OpenGL implementation
4. `sdl2_graphics_backend_windows.cpp` - Windows DX8/OpenGL implementation

**Key Tasks**:

- [ ] Define `SDL2GraphicsContext` struct (platform-agnostic)
- [ ] Define virtual function table for graphics operations
- [ ] Implement Metal context creation (macOS)
  - Use `SDL_GetWindowProperty()` to get Metal view
  - Initialize Metal device, command queue, layer
  - Implement buffer swapping via Metal rendering
- [ ] Implement OpenGL context creation (Linux)
  - Query SDL2 for OpenGL extensions
  - Create GLX context
  - Load GL function pointers
- [ ] Implement OpenGL context creation (Windows)
  - Use existing D3D8 mock or OpenGL fallback
  - Initialize GLEW or GLAD for GL functions

**Complexity**: MEDIUM

- Metal context creation is straightforward but requires Metal framework knowledge
- OpenGL setup is well-established pattern
- Windows support can reuse existing D3D8 layer or leverage OpenGL

**Testing**:

- [ ] Create context successfully on each platform
- [ ] Verify vsync works
- [ ] Verify buffer swapping produces visible output

---

### Phase 2: Event Loop Integration (10-15 hours)

**Objective**: Consolidate event handling across all platforms

**Deliverables**:

1. `sdl2_event_loop.h` - Unified event loop interface
2. Platform-specific event handlers

**Architecture**:

```cpp
// Platform-agnostic event loop
typedef struct {
    SDL2GraphicsContext* graphics_context;
    int running;
    void (*on_frame)(float delta_time);
    void (*on_event)(const SDL_Event* event);
    void (*on_resize)(int width, int height);
} SDL2EventLoop;

void SDL2_RunEventLoop(SDL2EventLoop* loop);
void SDL2_ProcessEvent(SDL2EventLoop* loop, const SDL_Event* event);
```

**Key Tasks**:

- [ ] Replace Win32GameEngine::serviceWindowsOS() with SDL2_RunEventLoop()
- [ ] Map SDL2 events to game engine callbacks
  - SDL_QUIT → engine shutdown
  - SDL_WINDOWEVENT_RESIZED → on_resize()
  - SDL_KEYDOWN/KEYUP → input translation
  - SDL_MOUSEMOTION/BUTTON → mouse input
- [ ] Handle platform-specific edge cases
  - macOS: Touch bar integration (if needed)
  - Linux: X11-specific events
  - Windows: Alt+Tab handling

**Complexity**: MEDIUM-LOW

- Mostly straightforward event dispatching
- Platform differences minimal at this level

---

### Phase 3: Integration with Existing Code (15-20 hours)

**Objective**: Adapt game engine to use unified SDL2 backend

**Files to Modify**:

1. `GeneralsMD/Code/GameEngine/Source/WinMain.cpp`
   - Replace `initializeAppWindows()` with SDL2 call
   - Keep existing subsystem initialization
2. `GeneralsMD/Code/GameEngine/Source/Win32GameEngine.cpp`
   - Replace `serviceWindowsOS()` with SDL2 event loop
3. `GeneralsMD/Code/GameEngineDevice/Source/W3DDisplay.cpp`
   - Replace platform-specific graphics initialization
   - Use SDL2GraphicsContext factory

**Integration Pattern**:

```cpp
// Before (platform-specific)
#ifdef _WIN32
    HWND hwnd = CreateWindow(...);
    IDirect3DDevice8* d3d_device = CreateD3D8Device(hwnd);
#elif __APPLE__
    NSWindow* nswin = [NSWindow new];
    MTLDevice* device = MTLCreateSystemDefaultDevice();
#elif __linux__
    GLXWindow glxwin = glXCreateWindow(...);
    GLXContext glx_ctx = glXCreateContext(...);
#endif

// After (unified)
SDL2GraphicsContext* ctx = SDL2_CreateGraphicsContext(
    "GeneralsX",
    1024, 768,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
);

// Platform abstraction handles the rest
SDL2_MakeContextCurrent(ctx);
```

**Compatibility Guarantees**:

- Windows GetMessage/DispatchMessage still available (via legacy layer)
- macOS NSWindow integration optional
- Linux X11 integration optional

**Complexity**: MEDIUM

- Requires careful refactoring to preserve existing functionality
- Extensive testing needed to ensure no regressions

---

### Phase 4: Testing & Validation (10-15 hours)

**Objective**: Verify unified backend works across all platforms

**Test Matrix**:

```text
┌─────────────┬───────────┬──────────────┬──────────────┐
│ Platform    │ Window    │ Event Loop   │ Graphics     │
├─────────────┼───────────┼──────────────┼──────────────┤
│ macOS ARM64 │ ✓ SDL2    │ ✓ SDL2       │ ✓ Metal      │
│ macOS x64   │ ✓ SDL2    │ ✓ SDL2       │ ✓ Metal      │
│ Linux       │ ✓ SDL2    │ ✓ SDL2       │ ✓ OpenGL     │
│ Windows     │ ✓ SDL2    │ ✓ SDL2       │ ✓ OpenGL/DX  │
└─────────────┴───────────┴──────────────┴──────────────┘
```

**Test Cases**:

1. **Window Creation**
   - [ ] Default windowed mode
   - [ ] Fullscreen toggle
   - [ ] Resize handling
   - [ ] Multi-display support

2. **Event Loop**
   - [ ] Keyboard input translation
   - [ ] Mouse movement & clicks
   - [ ] Window close handling
   - [ ] Frame timing (vsync)

3. **Graphics Context**
   - [ ] Context initialization
   - [ ] Buffer swapping
   - [ ] Framerate monitoring
   - [ ] Resolution changes

4. **Integration**
   - [ ] Game initializes without crashes
   - [ ] Assets load successfully
   - [ ] Rendering produces visible output
   - [ ] Performance acceptable (60+ FPS)

5. **Cross-Platform**
   - [ ] Same code compiles on all platforms
   - [ ] Behavioral consistency across OSes
   - [ ] Platform-specific workarounds tested

**Complexity**: MEDIUM

- Requires access to all three platforms for testing
- CI/CD integration important for ongoing validation

---

## Benefits

### Code Organization ✅

- **Before**: 3 separate code paths (Windows, macOS, Linux)
- **After**: 1 unified SDL2 backend with platform-specific implementations
- **Reduction**: ~40% less window/event code duplication

### Maintainability ✅

- Single source of truth for window/event behavior
- Platform differences isolated to graphics context layer
- Easier to add new platforms (e.g., Web via Emscripten)

### Performance 🚀

- SDL2 is highly optimized for each platform
- No overhead vs platform-native APIs
- Event loop consolidation may improve cache locality

### Interoperability 🔗

- SDL2 standardized API across all platforms
- Easier integration with existing SDL2 tools (ImGui, etc.)
- Better support for cross-platform physics/audio libraries

---

## Risks & Mitigation

| Risk | Severity | Mitigation |
|------|----------|-----------|
| **Regression in existing functionality** | HIGH | Extensive testing on all platforms before merge |
| **Performance regression** | MEDIUM | Profile critical paths, compare before/after |
| **SDL2 version incompatibilities** | LOW | Pin SDL2 3.0+ requirement, test early |
| **Platform-specific bugs** | MEDIUM | Dedicated testing on each platform, CI/CD validation |
| **Timeline slippage** | MEDIUM | Break into smaller PRs for incremental validation |

---

## Success Criteria

- ✅ All platforms build successfully with unified SDL2 backend
- ✅ Zero regressions in existing functionality (A/B testing)
- ✅ Performance within 5% of platform-specific implementations
- ✅ Code coverage >80% for new backend code
- ✅ Documentation complete (API, porting guide, troubleshooting)
- ✅ CI/CD pipeline validates all platforms on every commit

---

## References

### SDL2 Documentation

- SDL2 Official: https://wiki.libsdl.org/SDL2/
- SDL2 Graphics: https://wiki.libsdl.org/SDL2/CategoryVideo
- SDL2 Events: https://wiki.libsdl.org/SDL2/CategoryEvents

### Metal Integration

- Metal Rendering (macOS): https://developer.apple.com/documentation/metal
- Metal View Creation from SDL2: https://github.com/libsdl-org/SDL/wiki/Metal

### OpenGL Integration

- OpenGL Best Practices: https://www.khronos.org/opengl/
- SDL2 OpenGL Setup: https://wiki.libsdl.org/SDL2/Tutorial-using-OpenGL-with-SDL2

### Project References

- Phase 02 SDL2 Implementation: `docs/PHASE02/README.md`
- Win32 Compatibility Layer: `Core/Libraries/Source/WWVegas/WW3D2/win32_sdl_api_compat.h`
- Event Loop Implementation: `GeneralsMD/Code/GameEngine/Source/Win32GameEngine.cpp`

---

## Next Steps

1. **Proposal Review**: Get stakeholder buy-in on architecture
2. **Spike Investigation**: Create POC for Metal/OpenGL context creation
3. **Phasing**: Schedule for Phase 41-45 window
4. **Resource Allocation**: Ensure testing resources available for 3 platforms
5. **Documentation**: Create detailed implementation guide before coding starts

---

*Document created: November 12, 2025*  
*Last updated: November 12, 2025*

