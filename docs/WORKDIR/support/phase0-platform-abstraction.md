# Phase 0: Platform Abstraction Layer Design

**Created**: 2026-02-07
**Status**: In Progress

## Objective

Design a platform abstraction strategy that:
- Isolates Windows-specific code (Win32, DX8, Miles)
- Enables Linux code paths (POSIX, DXVK, OpenAL, SDL3)
- Preserves Windows build compatibility
- Maintains clean separation from game logic

## Core Principles

1. **Abstraction over Modification**
   - Create wrapper layers, don't modify core code
   - Game logic must remain platform-agnostic
   - All platform code behind compile flags

2. **Isolation Boundaries**
   - Platform code: `Core/GameEngineDevice/`, `Core/Libraries/Source/Platform/`
   - Game logic: `GeneralsMD/Code/GameEngine/`, `Core/GameEngine/`
   - Clear interface between layers

3. **Fallback Preservation**
   - Windows paths (DX8, Miles, Win32) must remain intact
   - Linux paths enabled via `#ifdef` guards
   - No runtime overhead for Windows builds

## Platform-Specific Areas

### Graphics Layer
**Location**: `Core/GameEngineDevice/`

**Abstraction Strategy**:
```cpp
// w3dgraphicsdevice.h
#ifdef BUILD_WITH_DXVK
    #include "dxvk_adapter.h"
    typedef IDXVKDevice* GraphicsDeviceHandle;
#else
    #include <d3d8.h>
    typedef IDirect3DDevice8* GraphicsDeviceHandle;
#endif

class W3DGraphicsDevice {
    GraphicsDeviceHandle GetDevice();
    // Core interface unchanged
};
```

**Isolation**: Game logic calls W3D interface, never DX8/DXVK directly.

### Windowing/Input Layer
**Location**: `Core/Libraries/Source/Platform/`

**Abstraction Strategy**:
- SDL3 for Linux (cross-platform library)
- Win32 API for Windows
- Unified event handling interface

**Key Areas**:
- Window creation/management
- Input (keyboard, mouse)
- Display mode enumeration
- Event loop

### Audio Layer
**Location**: `Core/GameEngine/Audio/` (new abstraction)

**Abstraction Strategy**:
```cpp
// audiomanager.h
#ifdef USE_OPENAL
    #include "openal_backend.h"
#else
    #include "miles_backend.h"
#endif

class AudioManager {
    // Miles-compatible API
    void PlaySound(const char* name);
    void PlayMusic(const char* filename);
    // Implementation delegates to backend
};
```

**Isolation**: Game logic calls AudioManager, backend handles platform.

### Filesystem Layer
**Location**: `Core/Libraries/Source/Platform/`

**Win32 → POSIX Considerations**:
- Path separators: `\` vs `/`
- Case sensitivity: Windows=no, Linux=yes
- File APIs: Win32 vs POSIX
- Registry reads: Windows-only (config files for Linux)

**Abstraction Strategy**:
- Path normalization functions
- Case-insensitive file lookup (for Linux)
- Config file fallback (instead of registry)

### Threading Layer
**Current State**: Check existing threading abstraction

**Potential Needs**:
- Win32 threads vs pthreads
- Synchronization primitives
- TLS (thread-local storage)

**Action**: Investigate if already abstracted

## Compile-Time Switches

### Graphics
- `BUILD_WITH_DXVK`: Enable DXVK wrapper (Linux)
- Default: DirectX 8 (Windows)

### Audio
- `USE_OPENAL`: Enable OpenAL backend (Linux/modern)
- Default: Miles Sound System (Windows)

### Windowing
- `USE_SDL3`: Enable SDL3 windowing (Linux)
- Default: Win32 API (Windows)

### Platform
- `__linux__`: Compiler-defined (Linux)
- `_WIN32`: Compiler-defined (Windows)

## CMake Preset Strategy

### Windows Presets (Existing)
- `vc6`: Visual Studio 6 + DX8 + Miles (STABLE BASELINE)
- `win32`: MSVC 2022 + DX8 + Miles (modern Windows)

### Linux Presets (To Add)
- `linux64-deploy`: GCC/Clang + DXVK + OpenAL + SDL3 (PRIMARY LINUX TARGET)
- `linux64-testing`: Debug variant

### MinGW Presets (Cross-compile)
- `mingw-w64-i686`: Cross-compile Windows .exe from Linux/macOS

## Isolation Checklist

### Good Examples
✅ Platform-specific code in `Core/GameEngineDevice/`
✅ Wrapper classes with uniform interface
✅ Compile-time platform selection
✅ Game logic never calls platform APIs directly

### Bad Examples
❌ `#ifdef __linux__` in game logic files
❌ Platform-specific hacks in gameplay code
❌ Breaking Windows builds to fix Linux
❌ Removing Windows code paths

## Testing Strategy

### Windows Compatibility
- VC6 preset MUST build and run
- Win32 preset MUST build and run
- Replay compatibility MUST pass

### Linux Functionality
- linux64-deploy MUST build native ELF
- Graphics via DXVK MUST render
- Audio via OpenAL MUST play

### Platform Isolation Validation
- Windows builds MUST NOT include Linux code
- Linux builds MUST NOT include Win32 code
- Shared code MUST be platform-agnostic

## Reference Patterns

### fighter19 (DXVK/SDL3)
To document: How platform abstraction was implemented

### jmarshall (OpenAL)
To document: How audio abstraction was implemented

## Notes

- This is a DESIGN document, not implementation
- Patterns will be refined during Phase 0 investigation
- Design must be APPROVED before Phase 1 starts
- When in doubt: more abstraction, less modification

## Next Steps

1. Confirm pattern with fighter19 analysis
2. Confirm pattern with jmarshall analysis
3. Validate against current codebase structure
4. Refine design based on findings
5. Get approval before Phase 1

## Refinements From Reference Analysis

- `DX8Wrapper` already exists in `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.{h,cpp}`; use it as the canonical device entry point.
- Minimal DXVK switch: replace `LoadLibrary("D3D8.DLL")` with `#ifdef _WIN32` / `#else LoadLibrary("libdxvk_d3d8.so")` to enable DXVK at runtime.
- SDL3 entry point: add `SDL3Main.cpp` (creates `SDL_CreateWindow` with `SDL_WINDOW_VULKAN`) and set `DXVK_WSI_DRIVER=SDL3` before Vulkan init.
- Audio: use `AudioManager` factory pattern (create `OpenALAudioManager` vs `MilesAudioManager` in `SDL3GameEngine::createAudioManager()`).
- Filesystem: implement case-insensitive lookup layer on Linux (VFS helper methods) to avoid asset load failures caused by case differences.

## Concrete Interface Rules

- Game logic must only call the abstract interfaces:
    - Graphics: use `DX8Wrapper` / `W3D` interfaces
    - Windowing/Input: use `GameEngine` / `Mouse` / `Keyboard` abstract classes
    - Audio: use `AudioManager` factory + virtual methods
    - Filesystem: use `LocalFileSystem` and `ArchiveFileSystem` factory

- Platform selection via CMake cache variables:
    - `SAGE_USE_SDL3` → enable SDL3 code paths
    - `SAGE_USE_OPENAL` → enable OpenAL audio backend
    - `SAGE_USE_DX8` → prefer native DX8 (Windows)

## Implementation Notes

- Keep `dx8wrapper.cpp` logic intact; prefer conditional compilation and small runtime/loader changes rather than refactoring internals.
- Add adapter headers where needed (e.g., `dxvk_adapter.h`) but avoid leaking adapter types into game logic headers.
- Ensure unit of change is small and reversible (one PR per backend: graphics, windowing, audio).
