# Cross-Platform Architecture Specification

**Phase**: 42 (Documentation)
**Status**: Phase 42 Week 2 Day 5
**Date**: 22 de novembro de 2025

---

## Overview

This document specifies the architecture and behavior expectations for cross-platform consistency across Windows, macOS, and Linux. All platforms must behave identically at the application level.

---

## Graphics Layer Specification

### Vulkan Backend

**All platforms use Vulkan for rendering**.

**Interface**: `IGraphicsDriver` (Phase 41)

**Expected Behavior (Platform-Independent)**:

- Window creation: 800x600 default
- Depth testing: Enabled
- Backface culling: Enabled by default (CW)
- Render target format: BGRA8 or RGBA8
- Depth target format: D32F or D24S8
- Frame sync: Enabled (vsync)
- Clear color: Same RGB values produce identical visual results

**Vulkan Features Required (All Platforms)**:

- VkInstance creation with validation layers (debug)
- Physical device enumeration and selection
- Logical device creation with graphics queue
- Swapchain creation (minimum 2 images)
- Render pass setup with color + depth attachments
- Command buffer recording and submission
- Synchronization (semaphores, fences)

**Platform-Specific Vulkan Extensions**:

| Platform | Required Extensions | Notes |
|----------|-------------------|-------|
| macOS | VK_KHR_portability_subset | Metal backend via MoltenVK |
| Linux | VK_KHR_wayland_surface or VK_KHR_xcb_surface | Wayland or X11 |
| Windows | VK_KHR_win32_surface | Native Windows surface |

### Metal Backend (macOS-Only)

While Vulkan is primary, Metal may be used as fallback on macOS via environment variable `USE_METAL=1`.

**Metal Implementation**: Direct Metal calls via MetalGraphicsDriver (Phase 29.4+)

**Behavior**: Identical rendering output as Vulkan backend.

---

## Input/Event System Specification

### Window Events

**Cross-Platform Contracts**:

All platforms must generate identical window event sequences:

```
1. Window Created
2. Window Resized (initial size)
3. Window Focused
4. First Frame Update
...
5. Window Unfocused (on focus loss)
6. Window Closed
7. Application Exit
```

**Expected Behavior**:

- Keyboard input: Immediate response to key press/release
- Mouse input: Cursor position updates every frame
- Window resize: Triggers resize event with new dimensions
- Window focus: Pause rendering when unfocused (optional, configurable)
- Close button: Generates window close event (not immediate shutdown)

### Input Handling

**SDL2 Implementation** (All platforms):

- Keyboard input via SDL_KEYDOWN, SDL_KEYUP
- Mouse input via SDL_MOUSEMOTION, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP
- Window events via SDL_WINDOWEVENT

**Consistent Behavior Across Platforms**:

- Key repeat rate: Same on all platforms
- Mouse sensitivity: Same on all platforms
- Joystick support: Consistent mapping
- Text input: UTF-8 encoding (SDL_TEXTINPUT)

**FunctionLexicon** (Phase 41 responsibility):

Maps SDL events to game callback functions. Must dispatch identically on all platforms.

---

## File System Specification

### Asset Loading

**VFS Layer** (Virtual File System):

All asset access goes through TheFileSystem abstraction.

**Cross-Platform Contracts**:

- Path separators: Forward slash `/` (internally), converted to platform-specific
- Case sensitivity: Case-insensitive comparison (matches Windows behavior)
- Asset location: `$HOME/GeneralsX/{Generals|GeneralsMD}/Data/` (relative to executable)
- Large files: .big archives containing INI, textures, models, sound

**Example Paths** (Normalized Internally):

```
Data/INI/Object/AirforceGeneral.ini
Data/Textures.big
Maps/DefaultMap01.map
```

**Asset Search Order**:

1. Check loose file in Data/
2. Check inside .big archive
3. Report missing if not found

**Result**: Identical asset loading behavior on Windows, macOS, Linux.

### Configuration Files

**INI Parser**:

- Encoding: UTF-8
- Line endings: Normalized (CRLF → LF)
- Comment character: `;` (start of line or after value)
- Array handling: Indexed by number (e.g., `Array[0]`, `Array[1]`)
- String escaping: Consistent across platforms

**Registry Replacement** (Windows-specific):

Windows Registry keys are mapped to configuration files:

```
HKEY_LOCAL_MACHINE\Software\GeneralsX\ → ~/.config/GeneralsX/system.ini
HKEY_CURRENT_USER\Software\GeneralsX\ → ~/.local/share/GeneralsX/user.ini
```

Result: Identical behavior for configuration lookup.

---

## Memory Management Specification

### Allocation Strategy

**Phase 42 Requirement**: Zero platform-specific memory issues.

**Cross-Platform Contracts**:

- Allocations: Use GameMemory allocation functions (Phase 30.6+)
- Alignment: 16-byte alignment for SIMD operations
- Bounds checking: Via `isValidMemoryPointer()` validation
- Leak detection: ASAN on debug builds (all platforms)

**Memory Protection**:

All platforms must validate pointers before dereferencing:

```cpp
// Bad: Direct dereference without validation
void* ptr = GetSomePointer();
int value = *(int*)ptr;  // May crash on any platform

// Good: Validated access
void* ptr = GetSomePointer();
if (isValidMemoryPointer(ptr, sizeof(int))) {
    int value = *(int*)ptr;
}
```

### Stack Unwinding

**FillStackAddresses()** (Phase 41 responsibility):

Must provide identical stack traces on all platforms:

```
[0] main
[1] GameEngine::Update
[2] RenderLoop::Frame
[3] IGraphicsDriver::Present
```

**Symbol Resolution**:

- macOS: Use dladdr() or DWARF debug info
- Linux: Use dladdr() or DWARF debug info
- Windows: Use StackWalk64()

Result: Identical crash logs and debugging experience.

---

## Rendering Consistency Specification

### Coordinate System

**All platforms use identical coordinate system**:

- Screen: Origin at top-left (0,0)
- X-axis: Increases rightward
- Y-axis: Increases downward
- Z-axis: Increases away from viewer (right-hand rule)

**Texture Coordinates**: Standard UV mapping (0,0) = top-left

### Color Representation

**RGBA Format** (All platforms):

- Channel order: Red, Green, Blue, Alpha
- Range: 0.0-1.0 (float) or 0-255 (int)
- Byte order in memory: Platform-dependent (handled by Vulkan)
- Result: Identical visual output

### Frame Timing

**VSync Behavior** (All platforms):

- Enabled by default
- Targets system refresh rate (60 Hz typical)
- Frames per second: Consistent ±1 FPS across platforms

**Frame Budget**: 16.67 ms per frame (60 Hz target)

### Texture Format Support

**Required Formats** (All platforms):

- RGBA8: Standard 32-bit RGBA
- RGB8: 24-bit RGB (no alpha)
- DXT1/BC1: 1-bit alpha or opaque
- DXT3/BC2: 4-bit alpha
- DXT5/BC3: Interpolated alpha
- Depth/D32F: 32-bit float depth
- Stencil/D24S8: 24-bit depth + 8-bit stencil

**Format Conversion**:

All conversions must produce byte-for-byte identical results:

```cpp
// Example: Convert ARGB8 to RGBA8
ARGB8 input = {...};
RGBA8 output;
output.r = input.r;
output.g = input.g;
output.b = input.b;
output.a = input.a;
// Result identical on all platforms
```

---

## Audio Layer Specification

### OpenAL Backend (Phase 33+)

**All platforms use OpenAL for audio**.

**Expected Behavior**:

- 3D positional audio: Works identically
- Volume levels: Same dB values produce same perceived volume
- Frequency: CD-quality (44.1 kHz default)
- Channels: Mono, Stereo, 5.1 Surround (if supported)

**Result**: Identical audio experience on all platforms.

---

## Network System Specification

### GameSpy Legacy (Phase 42 Decision Pending)

**Current Status**: GameSpy functions declared but undefined (Phase 41 blocker).

**Options**:

1. **Remove entirely** (Phase 43+) - Simplest
2. **Implement stub** - Return error codes
3. **Keep for compatibility** - Full implementation

**Expected Behavior** (if kept):

- Lobby communication: Identical protocol on all platforms
- Match making: Same matchmaking logic
- Lobby chat: UTF-8 message encoding
- Result: Cross-platform multiplayer compatibility

---

## Platform-Specific Deviations (Documented)

While aiming for 100% consistency, some deviations are allowed IF documented:

### macOS Specific

- **Metal as fallback**: USE_METAL=1 environment variable
- **HiDPI support**: Automatic retina display scaling
- **App bundle**: .app package structure for distribution
- **Notarization**: Required for distribution (codesigning)

### Linux Specific

- **X11 vs Wayland**: Auto-detect and use appropriate backend
- **Desktop environments**: GNOME, KDE, etc. (transparent to app)
- **Package formats**: AppImage, Flatpak, or system packages
- **Library paths**: LD_LIBRARY_PATH, rpath, etc.

### Windows Specific

- **DirectX compatibility mode**: Separate DirectX 8 wrapper (Phase 39.x)
- **Registry paths**: Backward compatible registry entries
- **UAC**: Admin elevation if needed (cache permissions)
- **Installer**: MSI or NSIS-based installer

---

## Testing Strategy for Cross-Platform Consistency

### Binary Equivalence Testing

**Goal**: Identical behavior on all platforms

**Test Categories**:

1. **Asset Loading**: Same files loaded identically
2. **Rendering**: Same visual output (pixel-for-pixel comparison)
3. **Input**: Same keyboard/mouse event handling
4. **Memory**: No platform-specific leaks or crashes
5. **Timing**: Frame timing within ±1 FPS
6. **Audio**: Same sound output (byte comparison)

### Platform Test Matrix

| Feature | Windows | macOS | Linux | Notes |
|---------|---------|-------|-------|-------|
| Vulkan rendering | ✓ | ✓ | ✓ | All platforms |
| Metal fallback | - | ✓ | - | macOS only |
| SDL2 events | ✓ | ✓ | ✓ | All platforms |
| Asset loading | ✓ | ✓ | ✓ | All platforms |
| Memory safety | ✓ | ✓ | ✓ | All platforms |
| Performance | ✓ | ✓ | ✓ | Within ±10% |

---

## Compliance Checklist (Phase 42 Completion)

- [ ] Vulkan backend: Renders identically on all platforms
- [ ] SDL2 events: Input works identically on all platforms
- [ ] Asset loading: Same files loaded on all platforms
- [ ] Configuration: INI parsing works identically
- [ ] Memory: Zero leaks on all platforms (ASAN passes)
- [ ] Crashes: Same crash logs on all platforms
- [ ] Performance: Frame timing consistent ±1 FPS
- [ ] Documentation: Platform deviations clearly noted

---

## Future Enhancements (Phase 43+)

- Platform-specific optimizations (texture compression, etc.)
- Advanced rendering features (ray tracing if GPU supports)
- Networking (modern multiplayer implementation)
- Audio enhancements (spatial audio, DSP effects)
- Accessibility features (colorblind modes, voice commands)

---

**Approval**: Pending Phase 42 Week 2 completion
**Last Updated**: 22 de novembro de 2025
