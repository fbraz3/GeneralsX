# Rollback: Re-enable OpenGL Backend (Cross-platform)

**Date Created:** November 4, 2025  
**Reason:** OpenGL backend was disabled in favor of Vulkan-only execution during Phase 48 implementation. This document provides instructions to restore OpenGL as an option.

---

## Quick Restore

If you need to restore OpenGL backend for cross-platform testing, follow these steps:

### Step 1: Restore WinMain.cpp Backend Selection Logic

In file: `GeneralsMD/Code/Main/WinMain.cpp`

**Around line 900-912**, restore the same code shown in ROLLBACK_METAL_BACKEND.md

The key is that OpenGL should be available as fallback on all platforms:
- macOS: Requires `USE_OPENGL=1` environment variable
- Linux: OpenGL is default (no environment variable needed)
- Windows: Check platform-specific logic

Current state (disabled): Backend selection logic is replaced with `g_useMetalBackend = false;` (Metal disabled, but OpenGL logic may also be disabled)

### Step 2: Verify DX8Wrapper OpenGL Fallback

In file: `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

**Around line ~2050-2060**, verify fallback logic exists:

```cpp
if (g_useMetalBackend) {
    MetalWrapper::BeginFrame(...);
} else {
    // OpenGL code path should activate here
    // Typical implementation uses glClear, glBegin, glEnd, etc.
}
```

If OpenGL path is missing or commented, uncomment it or restore from git history.

### Step 3: Verify OpenGL Initialization

File: `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

Function `DX8Wrapper::Init()` should have OpenGL setup code:

```cpp
// OpenGL device creation, context setup, etc.
// Typically involves:
// - glGenBuffers()
// - glGenVertexArrays()
// - glCreateProgram()
// - Shader compilation and linking
```

### Step 4: Recompile

```bash
cmake --preset macos-arm64      # or linux, depending on platform
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

### Step 5: Test OpenGL Backend

```bash
# macOS: Force OpenGL (Metal is default)
cd $HOME/GeneralsX/GeneralsMD
USE_OPENGL=1 ./GeneralsXZH

# Linux: OpenGL is default (no variable needed)
cd $HOME/GeneralsX/GeneralsMD
./GeneralsXZH

# Explicitly verify which backend is active
USE_OPENGL=1 ./GeneralsXZH 2>&1 | grep -i "backend\|opengl\|metal"
```

---

## Expected Behavior

When OpenGL backend is active, startup logs should show:

```
===== WinMain: macOS detected, backend = OPENGL (via USE_OPENGL=1) =====
```

Or on Linux:

```
===== WinMain: Linux detected, backend = OPENGL (default) =====
```

---

## OpenGL Implementation Status

**Implementation Level:** Phase 27-28 (W3D graphics engine, basic rendering)

### Features Implemented

- Basic shader compilation and linking
- Vertex buffer creation
- Texture loading (DDS, TGA formats with BC3 decompression)
- Render pass state management
- Viewport and scissor rect setup
- Lighting and fog state (render state mapping)

### Known Limitations

- No advanced graphics features (stencil shadows, post-processing)
- Limited to basic triangle/quad rendering
- Texture coordinates and normal mapping basic implementation
- No MSAA (multi-sample anti-aliasing)

---

## Differences: Metal vs OpenGL

| Feature | Metal | OpenGL |
|---------|-------|--------|
| **Platform** | macOS only | Cross-platform |
| **Stability** | ~50% crash rate (Nov 4) | Stable |
| **Performance** | Native GPU access | Driver translation layer |
| **Bugs Found** | AGX compiler race condition | None known |
| **Recommendation** | Not recommended for dev | Recommended for testing |

---

## Related Files

- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - DX8 mock with OpenGL fallback (line ~2050)
- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h` - DX8 mock interface
- `GeneralsMD/Code/Main/WinMain.cpp` - Backend selection logic (line 900-912)
- Shader files (typically in `resources/shaders/` or embedded in C++)

---

## Compilation Flags (CMake)

If OpenGL was previously disabled via CMake, check:

```bash
# In CMakeLists.txt or cmake/config.cmake
-DENABLE_OPENGL=ON    # Should be ON to compile OpenGL support
```

If OFF, rebuild with:

```bash
cmake --preset macos-arm64 -DENABLE_OPENGL=ON
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

---

## Testing Workflow

1. **Verify OpenGL is available:**
   ```bash
   glxinfo | grep version  # Linux
   # or check system OpenGL support
   ```

2. **Compile with OpenGL enabled:**
   ```bash
   cmake --preset macos-arm64
   cmake --build build/macos-arm64 --target GeneralsXZH -j 4
   ```

3. **Run with OpenGL backend:**
   ```bash
   USE_OPENGL=1 ./GeneralsXZH
   ```

4. **Monitor for errors:**
   ```bash
   USE_OPENGL=1 ./GeneralsXZH 2>&1 | tee /tmp/opengl_test.log
   grep -i "error\|fail\|shader" /tmp/opengl_test.log
   ```

---

## Last Status

**Last Verified:** November 4, 2025  
**Disabled Reason:** Focus shifted to Vulkan-only (Phase 48)  
**Recommendation:** Re-enable if cross-platform testing is needed

---

## Commit History

See git history for OpenGL-related implementations:
```bash
git log --oneline | grep -i "opengl\|graphics\|render"
```

