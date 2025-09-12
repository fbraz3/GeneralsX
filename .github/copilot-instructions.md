# AI Coding Agent Instructions

## Project Overview

**GeneralsGameCode** - Community effort to fix and improve C&C Generals/Zero Hour. Modernized from VC6/C++98 to VS2022/C++20 while maintaining retail compatibility.

**🚀 macOS Port Status**: All core libraries compiling + comprehensive Windows API compatibility layer implemented.

## Architecture

### Structure
- **Generals/**: Original C&C Generals (v1.08) 
- **GeneralsMD/**: Zero Hour expansion (v1.04) - **primary focus**
- **Core/**: Shared engine and libraries

### Key Cross-Platform APIs (Implemented)
- **Phase 1 ✅**: INI-based config system replacing Windows Registry
- **Phase 2 ✅**: Threading (pthread), File System (POSIX), Network (Win32Net), String APIs
- **Next Phase**: DirectX Graphics & Device APIs for full executable compilation
- **Primary compatibility layer**: `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`

### Port Phases Overview
- **✅ Phase 1**: Cross-platform configuration (RegOpenKeyEx → INI files)
- **✅ Phase 2**: Core Windows APIs (CreateThread, CreateDirectory, socket functions)
- **🎯 Phase 3**: DirectX/Graphics APIs (D3D device creation, rendering pipeline)
- **📋 Future**: Audio, Input, Advanced DirectX features

## Build System

### CMake Presets (Critical)
- **vc6**: VC6 compatible (retail compatibility required)
- **win32**: Modern VS2022 build
- Use: `cmake --preset <name>` then `cmake --build build/<name>`

### Retail Compatibility (Critical)
- VC6 builds required for replay compatibility
- Debug builds break retail compatibility
- Use `RTS_BUILD_OPTION_DEBUG=OFF` for testing

## Development Workflow

### Code Style
- Maintain consistency with legacy code
- Prefer C++98 unless modern features add significant value
- Present tense in documentation ("Fixes" not "Fixed")

### Cross-Platform Patterns
- Use include guards to prevent redefinition conflicts
- Coordinate Core/win32_compat.h and Generals/d3d8.h
- Check existing definitions before adding Windows types

### Testing
- **Replay compatibility**: `GeneralsReplays/` - critical for retail compatibility
- **macOS testing**: `cmake --build build/vc6 --target ww3d2 wwlib wwmath`

## Key Files
- `CMakePresets.json`: Build configurations
- `MACOS_PORT.md`: Detailed macOS porting progress  
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`: Main compatibility layer
- `GeneralsReplays/`: Compatibility test data