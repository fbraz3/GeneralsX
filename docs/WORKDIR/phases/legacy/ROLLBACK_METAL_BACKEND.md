# Rollback: Re-enable Metal Backend (macOS)

**Date Created:** November 4, 2025  
**Reason:** Metal backend was disabled in favor of Vulkan-only execution during Phase 48 implementation. This document provides instructions to restore Metal as an option.

---

## Quick Restore

If you need to restore Metal backend to macOS builds, follow these steps:

### Step 1: Restore WinMain.cpp Backend Selection Logic

In file: `GeneralsMD/Code/Main/WinMain.cpp`

**Around line 900-912**, restore this code block:

```cpp
#ifdef __APPLE__
    // macOS: Metal is the default backend (can be disabled with USE_OPENGL=1)
    const char* use_opengl = getenv("USE_OPENGL");
    g_useMetalBackend = (use_opengl == nullptr); // Metal unless OpenGL explicitly requested
    printf("===== WinMain: macOS detected, backend = %s =====\n", 
        g_useMetalBackend ? "METAL (default)" : "OPENGL (via USE_OPENGL=1)");
#else
    // Linux/other: OpenGL is default (Metal opt-in with USE_METAL=1)
    const char* use_metal = getenv("USE_METAL");
    g_useMetalBackend = (use_metal != nullptr);
    printf("===== WinMain: Linux detected, backend = %s =====\n", 
        g_useMetalBackend ? "METAL (via USE_METAL=1)" : "OPENGL (default)");
#endif
```

Current state (disabled): This logic is replaced with `g_useMetalBackend = false;` (always disabled)

### Step 2: Restore DX8Wrapper Metal Initialization

In file: `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

**Around line ~2000**, the function `DX8Wrapper::Begin_Scene()` should conditionally initialize Metal:

```cpp
if (g_useMetalBackend) {
    MetalWrapper::BeginFrame(...);
} else {
    // OpenGL fallback
}
```

If this is commented out, uncomment it.

### Step 3: Re-enable MetalWrapper Functions

File: `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm`

The entire file contains Metal implementation. Key functions to verify are active:
- `MetalWrapper::Initialize()` - Metal device/queue setup
- `MetalWrapper::BeginFrame()` - Render pass creation (has dispatch_sync serialization from Nov 4 fixes)
- `MetalWrapper::EndFrame()` - Command buffer presentation

### Step 4: Recompile

```bash
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

### Step 5: Test Metal Backend

```bash
# Force Metal (macOS default, no environment variable needed)
cd $HOME/GeneralsX/GeneralsMD
./GeneralsXZH

# Or explicitly:
USE_METAL=1 ./GeneralsXZH  # On non-macOS
```

---

## Known Issues with Metal Backend (As of Nov 4, 2025)

### Issue: Random crashes during render encoder creation
- **Symptom**: SIGSEGV in AGXMetal13_3 during `renderCommandEncoderWithDescriptor`
- **Root Cause**: Race condition between AGX background compiler threads and main render thread
- **Attempted Fixes**: 
  - Added 100ms delay in file I/O initialization
  - Added dispatch_sync serialization in BeginFrame()
  - Added 15ms usleep before encoder creation
- **Status**: Partially mitigated but not fully resolved

### Issue: Intermittent hangs during initialization
- **Symptom**: Process hangs at render encoder creation, requires force kill
- **Root Cause**: Metal driver deadlock with AttributeGraph framework
- **Attempted Fix**: 10-15ms usleep delays
- **Status**: Reduces frequency but doesn't eliminate

### Issue: FileFactory NULL pointer crash during WW3D::Init()
- **Symptom**: SIGSEGV at address 0x0000000000000810
- **Root Cause**: Initialization ordering issue - FileFactory may not be ready when WW3D::Init() called
- **Fix Applied**: Added null check before accessing _TheFileFactory

---

## Metal Backend Status

**Last Tested:** November 4, 2025, 21:48 UTC-3  
**Crash Rate:** ~50% (5 crashes in 10 runs, mostly AGX driver issues)  
**Recommendation:** Use OpenGL backend via `USE_OPENGL=1` for more stability during development

---

## Related Files

- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm` - Metal implementation (1698 lines)
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h` - Metal header/interface
- `GeneralsMD/Code/Main/WinMain.cpp` - Backend selection logic (line 900-912)
- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - DX8 mock that calls Metal (line ~2052)

---

## Commit History

- **Commit 09b344bc**: "fix: resolve random crash in StdBIGFileSystem with 100ms delay"
- **Commit 06d722b8**: "docs: update diary with crash fix details"
- Previous commits contain Metal wrapper implementation and fixes

