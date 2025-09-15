# AI Coding Agent Instructions

## Project Overview

**GeneralsGameCode** - Community effort to fix and improve C&C Generals/Zero Hour. Modernized from VC6/C++98 to VS2022/C++20 while maintaining retail compatibility.

**🚀 macOS Port Status**: 95% complete - 12 core libraries compiled, only DirectX interface harmonization and debug isolation remaining.

## Architecture

### Directory Structure
- **Generals/**: Original C&C Generals (v1.08)
- **GeneralsMD/**: Zero Hour expansion (v1.04) - **primary focus**
- **Core/**: Shared engine and libraries

### Executable Targets
- **g_generals**: Generals original → `generalsv` executable (5 compile errors)
- **z_generals**: Zero Hour expansion → `generalszh` executable (4 compile errors) 
- **generalszh**: Alias for z_generals (same target, different name)

**Priority Order**: z_generals (main focus) → g_generals → generalszh

### Cross-Platform Compatibility (95% Complete)
- **✅ Windows API Layer**: 200+ functions implemented in `Core/win32_compat.h`
- **✅ 12 Core Libraries**: 50MB+ compiled successfully (libww3d2.a, libwwmath.a, etc.)
- **🔧 Final Blockers**: DirectX type conflicts, debug component isolation, process APIs

## Build System

### CMake Configuration
- **Preset**: `cmake --preset vc6` (retail compatibility)
- **Build**: `cmake --build build/vc6 --target <target>`
- **Test Libraries**: `cmake --build build/vc6 --target ww3d2 wwlib wwmath`

### Critical Build Flags
- `RTS_BUILD_ZEROHOUR=ON` - enables Zero Hour (main focus)
- `RTS_BUILD_GENERALS=ON` - enables original Generals
- `RTS_BUILD_OPTION_DEBUG=OFF` - maintains retail compatibility

## Current Development Focus

### DirectX Interface Harmonization (Priority 1)
- **Issue**: `CORE_IDirect3DTexture8` vs `IDirect3DTexture8` type conflicts
- **Files**: `dx8wrapper.cpp`, Core/Generals/GeneralsMD `d3d8.h` files
- **Fix**: Consistent casting pattern and type guards

### Debug Component Isolation (Priority 2)  
- **Issue**: Windows-specific debug I/O compiled on macOS (19 errors/file)
- **Files**: `debug_io_*.cpp` in `Core/Libraries/Source/debug/`
- **Fix**: `#ifdef _WIN32` guards and macOS alternatives

### Process Management APIs (Priority 3)
- **Issue**: `SECURITY_ATTRIBUTES`, `CreatePipe` missing POSIX implementation
- **Files**: `Core/GameEngine/Source/Common/WorkerProcess.cpp`
- **Fix**: POSIX pipe/fork/exec implementation

## Key Files
- `MACOS_PORT.md`: Detailed port status and implementation roadmap
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`: Main compatibility layer
- `GeneralsReplays/`: Retail compatibility test data
- `CMakePresets.json`: Build system configurations