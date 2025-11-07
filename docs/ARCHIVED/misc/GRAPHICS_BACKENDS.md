# Graphics Backend Selection - GeneralsX

## Overview

GeneralsX supports multiple graphics backends for cross-platform compatibility:
- **Metal**: Native high-performance backend for macOS (default on Apple platforms)
- **OpenGL**: Cross-platform backend for macOS/Linux/Windows

## Platform Defaults

### macOS (Apple Silicon & Intel)
**Default Backend**: Metal (Phase 29.4+)
- Native Metal API for maximum performance
- Automatic detection via `__APPLE__` macro
- Zero configuration required

### Linux
**Default Backend**: OpenGL
- Standard OpenGL 3.3 Core Profile
- Automatic detection on non-Apple platforms

### Windows
**Default Backend**: DirectX 8
- Original Windows implementation
- No changes to existing behavior

## Backend Override Options

### Force OpenGL on macOS
Use when debugging graphics issues or comparing performance:

```bash
cd $HOME/GeneralsX/GeneralsMD
USE_OPENGL=1 ./GeneralsXZH
```

**When to use OpenGL override**:
- Comparing Metal vs OpenGL performance
- Debugging graphics-specific issues
- Testing cross-platform OpenGL compatibility
- Older macOS versions without Metal support

### Force Metal on Linux (Experimental)
Not recommended - Metal requires Apple hardware:

```bash
# This will fail on Linux (Metal not available)
USE_METAL=1 ./GeneralsXZH
```

## Backend Detection Logic

The game automatically detects the best backend at startup:

```cpp
// Phase 29.4: Auto-detection in WinMain.cpp
#ifdef __APPLE__
    // macOS: Metal default, OpenGL opt-in
    const char* use_opengl = getenv("USE_OPENGL");
    g_useMetalBackend = (use_opengl == nullptr);
#else
    // Linux: OpenGL default, Metal opt-in (not functional)
    const char* use_metal = getenv("USE_METAL");
    g_useMetalBackend = (use_metal != nullptr);
#endif
```

## Startup Messages

### Metal Backend (macOS default)
```
===== WinMain: macOS detected, backend = METAL (default) =====
```

### OpenGL Backend (override on macOS)
```
===== WinMain: macOS detected, backend = OPENGL (via USE_OPENGL=1) =====
```

### OpenGL Backend (Linux default)
```
===== WinMain: Linux detected, backend = OPENGL (default) =====
```

## Implementation Status

### Phase 29.4: Metal as Default ✅
- **Completed**: October 13, 2025
- **Platform Detection**: Automatic via `__APPLE__` macro
- **Validation**: Tested on macOS ARM64 (Apple Silicon)
- **Runtime**: Confirmed working with blue-gray screen + colored triangle

### Phase 29.3: Metal Stubs ✅
- **Completed**: October 12, 2025
- **Components**: CPU-side buffers, texture stubs, viewport protection
- **Protection**: All OpenGL calls wrapped with `g_useMetalBackend` checks
- **Validation**: Full engine execution without crashes

### Phase 30: Metal GPU Buffers (Planned)
- **Status**: Not started
- **Goal**: Implement MTLBuffer for actual GPU rendering
- **Components**: Vertex buffers, index buffers, texture upload

## Performance Comparison

### Metal (macOS)
- **Advantages**:
  - Native Apple Silicon performance
  - Lower CPU overhead
  - Better battery life on laptops
  - Direct hardware access
- **Disadvantages**:
  - Apple-only (not cross-platform)
  - Requires macOS 10.13+ (Metal 2)

### OpenGL (Cross-platform)
- **Advantages**:
  - Works on macOS, Linux, Windows
  - Mature, stable implementation
  - Wide driver support
- **Disadvantages**:
  - Higher CPU overhead on macOS
  - Deprecated by Apple (still functional)
  - Slower on Apple Silicon vs Metal

## Troubleshooting

### "Metal backend = OPENGL" when not expected
**Cause**: `USE_OPENGL` environment variable is set
**Solution**: Unset the variable or restart terminal
```bash
unset USE_OPENGL
./GeneralsXZH
```

### Black screen or no rendering
**Possible causes**:
1. Wrong backend selected for platform
2. Graphics drivers outdated
3. Missing game assets

**Solutions**:
1. Try alternative backend: `USE_OPENGL=1 ./GeneralsXZH` on macOS
2. Update macOS/graphics drivers
3. Verify game assets in `$HOME/GeneralsX/GeneralsMD/`

### Performance issues on macOS
**Recommendation**: Ensure Metal backend is active (default)
```bash
# Check startup message
./GeneralsXZH 2>&1 | grep "backend ="

# Should output:
# ===== WinMain: macOS detected, backend = METAL (default) =====
```

If OpenGL is active by mistake, remove any `USE_OPENGL` variables.

## Development Notes

### Adding New Backend
To add a new graphics backend (e.g., Vulkan):

1. Update `WinMain.cpp` detection logic
2. Add new backend flag (e.g., `g_useVulkanBackend`)
3. Implement wrapper functions in `dx8wrapper.cpp`
4. Protect all rendering calls with backend checks
5. Update documentation

### Testing Both Backends
```bash
# Test Metal (should work on macOS)
cd $HOME/GeneralsX/GeneralsMD && ./GeneralsXZH

# Test OpenGL (should work on macOS/Linux)
cd $HOME/GeneralsX/GeneralsMD && USE_OPENGL=1 ./GeneralsXZH

# Compare startup performance
time ./GeneralsXZH
time USE_OPENGL=1 ./GeneralsXZH
```

## References

- **Metal Documentation**: [Apple Developer - Metal](https://developer.apple.com/metal/)
- **OpenGL Documentation**: [Khronos OpenGL](https://www.khronos.org/opengl/)
- **Implementation**: See `WinMain.cpp`, `dx8wrapper.cpp`, `metalwrapper.mm`
- **Project Docs**: `docs/MACOS_BUILD_INSTRUCTIONS.md`, `docs/METAL_BACKEND_SUCCESS.md`
