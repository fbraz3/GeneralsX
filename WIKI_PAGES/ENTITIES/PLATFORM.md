# ENTITIES/PLATFORM.md

## Summary

Platform technologies — SDL3, DXVK, OpenAL, FFmpeg, and other platform components.

---

## Platform Stack

```
┌─────────────────────────────────────────────────────────────┐
│                    PLATFORM TECHNOLOGIES                      │
├─────────────────────────────────────────────────────────────┤
│  WINDOWING:    SDL3                                        │
│  RENDERING:    DXVK (DX8 → Vulkan)                         │
│  AUDIO:        OpenAL                                     │
│  VIDEO:        FFmpeg (Bink → FFmpeg)                      │
│  PLATFORM I/O: SDL3                                       │
└─────────────────────────────────────────────────────────────┘
```

---

## Windowing: SDL3

### Purpose

Unified windowing and input abstraction for cross-platform compatibility.

### Key Features

- **Window management:** Create, destroy, resize windows
- **Input handling:** Keyboard, mouse, gamepad
- **Multi-threading:** Thread-safe operations
- **Multi-monitor:** Multi-monitor support
- **Virtual desktop:** Virtual desktop support

### Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| Linux | ✅ Full | x11, wayland |
| macOS | ✅ Full | cocoa |
| Windows | ✅ Full | winrt, wgl |

### Initialization

```cpp
SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
SDL_Window* window = SDL_CreateWindow("GeneralsX", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720);
```

### Platform-Specific Notes

**Linux:**
- Default video driver: `x11`
- Wayland support: `wayland`
- Environment variable: `SDL_VIDEODRIVER`

**macOS:**
- Default video driver: `cocoa`
- Environment variable: `SDL_VIDEODRIVER`

---

## Rendering: DXVK

### Purpose

DirectX 8 to Vulkan translation layer for cross-platform rendering.

### Key Features

- **DX8 compatibility:** Full DirectX 8 API compatibility
- **Vulkan backend:** Modern Vulkan rendering
- **Shader compilation:** Automatic shader compilation
- **Cross-platform:** Windows, Linux, macOS

### Implementation Details

**Entry Point:** `Core/GameEngineDevice/Source/dx8wrapper.cpp`

**Key Functions:**
- `D3D8CreateDevice()` — Device creation
- `D3D8CreateTexture()` — Texture loading
- `D3D8BeginScene()` / `D3D8EndScene()` — Scene rendering

### Platform Issues & Fixes

| Issue | Platform | Status |
|-------|----------|--------|
| Transparency regression | macOS | Fixed |
| Headless texture path | Linux | Fixed |

### References

- **fighter19-dxvk-port** — Integration patterns
- **dx8wrapper.cpp** — Implementation details

---

## Audio: OpenAL

### Purpose

Cross-platform audio API for 3D positional audio.

### Key Features

- **3D positional audio:** Spatial audio with head tracking
- **Music playback:** Streaming music support
- **Effects:** Sound effects playback
- **Platform abstraction:** Unified API across platforms

### Legacy Replacement

**Miles Sound System → OpenAL**

**Key Changes:**
- Audio initialization abstraction
- Platform-specific driver selection
- Resource loading through SDL3

### References

**jmarshall-win64-modern** — Architecture patterns and implementation details.

---

## Video: FFmpeg

### Purpose

Cross-platform multimedia framework replacing Bink video.

### Key Features

- **Video decoding:** Multiple codec support
- **Audio decoding:** Multiple format support
- **Platform-independent:** Works on all platforms
- **High performance:** Optimized for performance

### Legacy Replacement

**Bink Video → FFmpeg**

**Packaging:**

| Platform | Package | Command |
|----------|---------|--------|
| Linux (Arch) | ffmpeg | `sudo pacman -S ffmpeg` |
| Linux (Debian) | ffmpeg | `sudo apt-get install ffmpeg` |
| macOS | ffmpeg | `brew install ffmpeg` |

### Platform Issues & Fixes

| Issue | Platform | Status |
|-------|----------|--------|
| Case-sensitive lookup | Linux | Fixed (2026-05-11) |

---

## Platform Abstraction: SDL3 (I/O)

### Purpose

Unified file I/O, threading, and platform-specific operations.

### Key Features

- **File I/O:** Load, save, read, write
- **Threading:** Thread creation, synchronization
- **Signal handling:** Unix signals, Windows events
- **Process management:** Fork, exec, spawn

### File I/O Examples

```cpp
// Load file
SDL_LoadFile("texture.tga", &size, &data);

// Save file
SDL_SaveFile("output.bin", data, size);
```

### Threading Examples

```cpp
// Create thread
SDL_CreateThread(thread_func, "thread_name");

// Wait for thread completion
SDL_WaitThread(thread, &return_code);
```

---

## Platform-Specific Considerations

### Linux

**Video Driver:**
- Default: `x11`
- Alternative: `wayland`

**Audio Driver:**
- Default: `pulse`
- Alternative: `alsa`

**Case-Sensitive File Access:**
- Issue: Linux file system is case-sensitive
- Solution: Case-insensitive VFS lookup

### macOS

**Video Driver:**
- Default: `cocoa`
- Alternative: `metal`

**Audio Driver:**
- Default: `coreaudio`
- Alternative: `alsa`

**Transparency Issues:**
- Issue: Render target switching caused transparency problems
- Solution: Pillarbox render-target switching through DX8Wrapper

---

## Platform Issues Summary

### FPS Uncapped (ISSUE #132)

**Problem:** FPS limit not enforced on Linux/macOS

**Fix:**
1. FramePacer API guard for Windows
2. FPS limit validation
3. FPS init logic fix
4. Build flag: `-ffp-contract=off`

**Status:** Fixed (2026-05-11)

### Case-Sensitive Assets (ISSUE #128)

**Problem:** Linux case-sensitive file access broke asset loading

**Fix:** Case-insensitive traversal in VFS

**Status:** Fixed (2026-05-11)

### macOS Transparency (ISSUE #131)

**Problem:** Render target switching caused transparency issues

**Fix:**
1. Pillarbox render-target switching through DX8Wrapper
2. Post-blit `Invalidate_Cached_Render_States()`
3. Pillarbox presentation fix

**Status:** Fixed (2026-05-10)

### Headless Texture Path

**Problem:** Texture loading failed in headless replay mode

**Fix:**
1. Null guards in `D3DXCreateTexture`
2. Safe `DX8Wrapper::_Create_DX8_Texture`
3. Graceful failure in texture load tasks

**Status:** Fixed (2026-05-04)

---

## See Also

- [CONCEPTS/PORTING.md](../CONCEPTS/PORTING.md) — Platform migrations
- [CONCEPTS/RAG.md](../CONCEPTS/RAG.md) — Platform isolation
- [ARCHITECTURE/LAYERS.md](../ARCHITECTURE/LAYERS.md) — Layer boundaries
- [TOPICS/BUILD.md](../TOPICS/BUILD.md) — Build instructions

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, architecture docs
