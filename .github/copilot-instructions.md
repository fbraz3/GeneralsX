# AI Coding Agent Instructions

## Project Overview

**GeneralsGameCode** - Cross-platform port of Command & Conquer: Generals/Zero Hour. Modernized from VC6/C++98 to modern C++20 with comprehensive macOS/Linux compatibility.

**🎯 Current Status**: Phase 13 - Final 93 compilation errors, 95% complete, focusing on Vector3/Vector4 conflicts and DirectX harmonization.

## Architecture & Critical Patterns

### Directory Structure
- **Core/**: Shared engine (50MB+ compiled libraries)
  - `win32_compat.h` (1,800+ lines) - **Primary compatibility layer**
  - `Libraries/Source/WWVegas/WW3D2/` - 3D graphics engine  
  - `Libraries/Source/debug/` - Cross-platform debug system
- **GeneralsMD/**: Zero Hour expansion - **PRIMARY TARGET**
- **Generals/**: Original game - secondary target  

### Build System Commands
```bash
# Primary workflow - ALWAYS use vc6 preset for retail compatibility
cmake --preset vc6
cmake --build build/vc6 --target z_generals  # Main target (4 errors)
cmake --build build/vc6 --target g_generals  # Secondary (5 errors)

# Test core libraries independently
cmake --build build/vc6 --target ww3d2 wwlib wwmath
```

### Cross-Platform Compatibility Strategy
**Essential Pattern**: `#ifdef _WIN32` conditional compilation throughout
- Windows APIs isolated in `win32_compat.h` 
- POSIX alternatives for macOS/Linux
- 200+ Windows functions replaced (HeapAlloc→malloc, wsprintf→snprintf, etc.)
- **Never break Windows compatibility** - all original code paths preserved

## Critical Development Issues

### Vector3/Vector4 Type Conflicts (Current Blocker)
**Root Cause**: `Core/Libraries/Include/GraphicsAPI/GraphicsRenderer.h` mock types conflict with WWMath classes
```cpp
// WRONG - causes redefinition errors
struct Vector3 { float x, y, z; };

// CORRECT - forward declarations only
class Vector3;  // From WWMath/vector3.h
```
**Fix Pattern**: Use forward declarations, avoid mock types when real WWMath types available

### DirectX Interface Harmonization  
**Pattern**: Consistent `IDirect3DTexture8*` usage across Core/Generals/GeneralsMD
- Files: `dx8wrapper.cpp`, `texture.h`, `d3d8.h` in both Generals and GeneralsMD
- **Issue**: `CORE_IDirect3DTexture8` vs `IDirect3DTexture8` naming conflicts
- **Solution**: Unified typedef pattern in compatibility headers

### Debug System Architecture (Recently Completed)
**Pattern**: Complete Windows API isolation
```cpp
#ifdef _WIN32
    // Windows-specific: wsprintf, _itoa, MessageBox, etc.
#else  
    // Cross-platform: snprintf, console output, etc.
#endif
```

## Development Workflows

### Error Resolution Priority
1. **Vector/Type Conflicts** - Fix GraphicsRenderer.h mock types first
2. **DirectX Harmonization** - Unify texture interface naming  
3. **Missing Types** - Add forward declarations for RTS3DScene, TerrainTextureClass
4. **sprintf Deprecation** - Replace with snprintf (security warnings)

### Testing Strategy
- **Retail Compatibility**: Always test with `RTS_BUILD_OPTION_DEBUG=OFF`
- **Library Verification**: Build core libraries first before game targets
- **Cross-Platform**: Test macOS builds regularly (`#ifdef _WIN32` guards)

### Memory Management Patterns
**Critical**: Game uses custom memory pools extensively
```cpp
MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ClassName, "PoolName")
// Generates operator new/delete with pool allocation
```
**Never modify** pool macros - they maintain exact retail memory layout

## Key Integration Points

### Graphics Pipeline
- **W3D Engine**: 25MB compiled graphics libraries (libww3d2.a)
- **DirectX8→OpenGL**: Compatibility via `dx8wrapper.cpp` abstraction
- **Texture Management**: `TextureBaseClass` hierarchy with DirectX interfaces

### Platform-Specific Files
- **Windows**: Native DirectX, all original functionality preserved
- **macOS**: OpenGL rendering, POSIX APIs via win32_compat.h  
- **Cross-Platform**: Conditional compilation, no platform-specific branches

## Essential Files for AI Context
- `MACOS_PORT.md` - Detailed progress tracking and error analysis
- `NEXT_STEPS.md` - Current phase status and remaining work  
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h` - All platform abstractions
- `CMakePresets.json` - Build configurations (use "vc6" preset)

**Debugging Tip**: Current errors focus on type redefinitions and missing forward declarations - check include order and conditional compilation guards first.