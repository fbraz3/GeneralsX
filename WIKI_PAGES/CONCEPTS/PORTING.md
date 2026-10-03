# CONCEPTS/PORTING.md

## Summary

Platform porting — DX8→Vulkan, Miles→OpenAL, and other platform migrations.

---

## Purpose

Document the platform layer migrations that enable cross-platform functionality.

---

## Porting Strategy

### General Approach

1. **Identify legacy API** — Find Windows/Miles-specific code
2. **Select modern replacement** — Choose SDL3, DXVK, OpenAL, FFmpeg
3. **Create abstraction layer** — Wrap platform-specific behavior
4. **Test on target platform** — Verify functionality
5. **Update documentation** — Document patterns and pitfalls

---

## DX8 → Vulkan (via DXVK)

### Legacy API

**DirectX 8 (Windows)**
- D3D8 interfaces
- Direct rendering calls
- Texture management
- Shader compilation

### Modern Replacement

**DXVK (DX8 → Vulkan)**
- Vulkan-based translation layer
- Automatic shader compilation
- Cross-platform rendering
- Retained DX8 API compatibility

### Implementation

**Entry Point:**
- `Core/GameEngineDevice/Source/dx8wrapper.cpp`

**Key Functions:**
- `D3D8CreateDevice()` — Device creation
- `D3D8CreateTexture()` — Texture loading
- `D3D8BeginScene()` / `D3D8EndScene()` — Scene rendering

### Platform Issues

| Issue | Platform | Status |
|-------|----------|--------|
| Transparency regression | macOS | Fixed |
| Headless texture path | Linux | Fixed |

### References

- **fighter19-dxvk-port** — Integration patterns
- **dx8wrapper.cpp** — Implementation details

---

## Miles Sound System → OpenAL

### Legacy API

**Miles Sound System (Windows)**
- Proprietary audio API
- Direct hardware access
- Spatial audio
- Music streaming

### Modern Replacement

**OpenAL (Cross-platform)**
- Open standard audio API
- Platform abstraction
- 3D positional audio
- Music and effects support

### Implementation

**Reference:**
- **jmarshall-win64-modern** — Architecture patterns

**Key Changes:**
- Audio initialization abstraction
- Platform-specific driver selection
- Resource loading through SDL3

### Platform Issues

| Issue | Platform | Status |
|-------|----------|--------|
| None reported | N/A | Stable |

---

## SDL3 → Win32/Cocoa/X11

### Legacy API

**Windows/Mac/Linux Direct Calls**
- Win32 API (Windows)
- Cocoa API (macOS)
- X11 API (Linux)

### Modern Replacement

**SDL3 (Unified Abstraction)**
- Single API for all platforms
- Cross-platform windowing
- Unified input handling
- Platform-specific driver selection

### Implementation

**Key Features:**
- `SDL_VIDEODRIVER=dummy` — Headless mode
- `SDL_AUDIODRIVER=dummy` — Headless mode
- Automatic driver selection
- Platform-specific fallbacks

### Platform-Specific Notes

| Platform | Video Driver | Audio Driver |
|----------|--------------|--------------|
| Linux | x11 (default) | pulse (default) |
| macOS | cocoa (default) | coreaudio (default) |

---

## FFmpeg → Bink Video

### Legacy API

**Bink Video (Windows)**
- Proprietary video codec
- Windows-specific implementation
- Limited cross-platform support

### Modern Replacement

**FFmpeg (Cross-platform)**
- Open-source multimedia framework
- Multiple codec support
- Platform-independent
- High performance

### Implementation

**Packaging:**

| Platform | Package | Command |
|----------|---------|--------|
| Linux (Arch) | ffmpeg | `sudo pacman -S ffmpeg` |
| Linux (Debian) | ffmpeg | `sudo apt-get install ffmpeg` |
| macOS | ffmpeg | `brew install ffmpeg` |

### Platform Issues

| Issue | Platform | Status |
|-------|----------|--------|
| Case-sensitive lookup | Linux | Fixed (2026-05-11) |

---

## Porting Checklist

### Pre-Port
- [ ] Identify legacy API usage
- [ ] Select modern replacement
- [ ] Create abstraction plan

### During Port
- [ ] Replace API calls
- [ ] Add compile guards
- [ ] Update headers

### Post-Port
- [ ] Test on target platform
- [ ] Verify functionality
- [ ] Update documentation
- [ ] Add to platform issues list

---

## See Also

- [ENTITIES/PLATFORM.md](../ENTITIES/PLATFORM.md) — Platform technologies
- [ARCHITECTURE/LAYERS.md](../ARCHITECTURE/LAYERS.md) — Layer boundaries
- [CONCEPTS/RAG.md](./RAG.md) — Platform isolation

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, architecture docs
