# Phase 50: Platform-Specific Presets and Compatibility Architecture

## Quick Reference

### Preset Naming Convention
Format: `<platform>[-<architecture>]-<backend>`

**Examples**:
- `macos-arm64-vulkan` - macOS ARM64 with Vulkan (Phase 50 primary)
- `linux-vulkan` - Linux with Vulkan
- `windows-vulkan` - Windows with Vulkan (Phase 51+ future)

### Compatibility Header Pattern
Format: `<source_api>_<backend>_<subsystem>_compat.h`

**Examples**:
- `d3d8_vulkan_graphics_compat.h` - DirectX 8 → Vulkan graphics
- `win32_posix_api_compat.h` - Win32 API → POSIX API
- `dsound_vulkan_audio_compat.h` - DirectSound → Vulkan audio (Phase 51+)

---

## Build Presets Overview

### Phase 50 Current Presets

| Preset | Description | Platform | Architecture | Backend | Build Dir |
|--------|-------------|----------|--------------|---------|-----------|
| `macos-arm64-vulkan` | macOS ARM64 (Vulkan via MoltenVK) | macOS | ARM64 | Vulkan | `build/macos-arm64-vulkan/` |
| `macos-arm64` | Alias for macos-arm64-vulkan | macOS | ARM64 | Vulkan | `build/macos-arm64/` |
| `macos-x64-vulkan` | macOS Intel (Vulkan via MoltenVK) | macOS | x86_64 | Vulkan | `build/macos-x64-vulkan/` |
| `macos-x64` | Alias for macos-x64-vulkan | macOS | x86_64 | Vulkan | `build/macos-x64/` |
| `linux-vulkan` | Linux 64-bit (native Vulkan) | Linux | x86_64 | Vulkan | `build/linux-vulkan/` |
| `linux` | Alias for linux-vulkan | Linux | x86_64 | Vulkan | `build/linux/` |
| `vc6` | Windows 32-bit (Visual C++ 6, DirectX) | Windows | x86 | DirectX | `build/vc6/` |

### Phase 51+ Future Presets

| Preset | Description | Status |
|--------|-------------|--------|
| `windows-vulkan` | Windows with native Vulkan | Placeholder (hidden) |
| `macos-arm64-opengl` | macOS ARM64 with OpenGL | Phase 51+ planned |
| `linux-opengl` | Linux with OpenGL | Phase 51+ planned |
| `macos-metal` | macOS with native Metal | Phase 52+ planned |

---

## Compatibility Layer Architecture

### Three-Layer Design

#### Layer 1: Core Platform Compatibility
**Pattern**: `win32_<dest>_<type>_compat.h`

Headers for platform-independent type definitions and system API translations.

**Examples**:
- `win32_posix_api_compat.h` - System calls (file I/O, memory, threading)
- `win32_posix_core_compat.h` - Type definitions (HWND, HRESULT, RECT)

**Key Principle**: Never modify game code. Add compatibility shims here instead.

#### Layer 2: Graphics Backend Translation
**Pattern**: `d3d8_<backend>_graphics_compat.h`

Headers for DirectX 8 → Graphics Backend translations.

**Phase 50**:
- `d3d8_vulkan_graphics_compat.h` - DirectX 8 → Vulkan (active)
- Stub implementations (return 0, non-fatal)
- Real implementation delegated to game engine

**Phase 51+**:
- `d3d8_opengl_graphics_compat.h` - DirectX 8 → OpenGL (placeholder)
- Parallel implementations without conflicts

#### Layer 3: Audio Backend Translation
**Pattern**: `dsound_<backend>_audio_compat.h`

Headers for DirectSound → Audio Backend translations.

**Phase 50**: Placeholder stubs only

**Phase 51+**: Full audio system implementation

---

## Naming Convention Explained

### Pattern: `<source_api>_<backend>_<subsystem>_compat.h`

```
<source_api>_<backend>_<subsystem>_compat.h
│            │        │
│            │        └─ api, graphics, audio, core
│            └────────── vulkan, opengl, metal, posix, sdl, wasapi
└───────────────────── win32, d3d8, dsound, d3d
```

### Why This Matters

This naming convention **encodes the translation path directly in the filename**:

1. **`win32_posix_api_compat.h`**
   - Translates: Win32 API → POSIX API
   - Location: Core compatibility layer
   - Use: Platform-independent system calls

2. **`d3d8_vulkan_graphics_compat.h`**
   - Translates: DirectX 8 → Vulkan graphics
   - Location: Graphics backend abstraction
   - Use: Cross-platform graphics compilation

3. **`dsound_vulkan_audio_compat.h`** (Phase 51+)
   - Translates: DirectSound → Vulkan audio
   - Location: Audio backend abstraction
   - Use: Cross-platform audio compilation

### Benefits

- **Explicit**: Immediately clear what each header does
- **Scalable**: Adding new backends doesn't require refactoring
- **Maintainable**: grep for `d3d8_*` finds all DirectX graphics compatibility code
- **Testable**: Each backend is isolated

---

## Build Commands Reference

### Configure Phase 50 (Vulkan-Only)

```bash
# Using specific backend-named preset
cmake --preset macos-arm64-vulkan

# Or use legacy alias (backward compatible)
cmake --preset macos-arm64
```

### Build Zero Hour (Primary Target)

```bash
# With specific preset
cmake --build build/macos-arm64-vulkan --target GeneralsXZH -j 4 2>&1 | tee logs/build.log

# With alias
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | tee logs/build.log
```

### Build Original Generals (Secondary Target)

```bash
cmake --build build/macos-arm64 --target GeneralsX -j 4 2>&1 | tee logs/build.log
```

### Linux Example

```bash
cmake --preset linux-vulkan
cmake --build build/linux-vulkan --target GeneralsXZH -j 4 2>&1 | tee logs/build.log
```

---

## Migration Path: Adding Backends

### Step 1: Create New Compatibility Header

For Phase 51 OpenGL backend:

```text
d3d8_opengl_graphics_compat.h
```

Same interface as `d3d8_vulkan_graphics_compat.h`, different implementation.

### Step 2: Add New CMake Preset

```json
{
    "name": "macos-arm64-opengl",
    "inherits": "macos-arm64-base",
    "displayName": "macOS ARM64 with OpenGL",
    "cacheVariables": {
        "ENABLE_OPENGL": "ON",
        "ENABLE_VULKAN": "OFF"
    }
}
```

### Step 3: Parallel Compilation

```bash
# Compile Vulkan version
cmake --preset macos-arm64-vulkan
cmake --build build/macos-arm64-vulkan --target GeneralsXZH -j 4

# Compile OpenGL version
cmake --preset macos-arm64-opengl
cmake --build build/macos-arm64-opengl --target GeneralsXZH -j 4
```

Both executables coexist. No game code changes required.

---

## Status

**Phase 50** ✅
- Platform-specific presets with backend naming
- Vulkan-only graphics implementation
- Three-layer compatibility architecture
- Documentation and naming conventions

**Phase 51** ⏳ (Planned)
- OpenGL backend parallel implementation
- Audio system via DirectSound translation
- Runtime backend selection (future)

**Phase 52** ⏳ (Planned)
- Native Metal backend for macOS
- Consolidated multi-backend support

---

## Related Files

- **CMake Presets**: `CMakePresets.json`
- **Project Instructions**: `.github/instructions/project.instructions.md`
- **AI Instructions**: `.github/copilot-instructions.md`
- **Compatibility Headers**: `Core/Libraries/Source/WWVegas/WW3D2/*.h`
