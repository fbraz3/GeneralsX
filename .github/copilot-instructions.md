# AI Coding Agent Instructions

## Project Overview

This is the **GeneralsGameCode** project - a community-driven effort to fix and improve the classic RTS games *Command & Conquer: Generals* and *Zero Hour*. The codebase has been modernized from Visual Studio 6/C++98 to Visual Studio 2022/C++20 while maintaining retail compatibility.

**🎉 MAJOR BREAKTHROUGH**: macOS port achieved - all core libraries successfully compiling with comprehensive Windows API compatibility layer.

**🚀 SEPTEMBER 11, 2025 BREAKTHROUGH**: DirectX typedef redefinition resolution completed! 120+ compilation errors resolved with dx8wrapper.cpp achieving 0 errors through comprehensive Windows API implementation and multi-layer DirectX architecture coordination.

## Architecture

### Dual Game Structure
- **Generals/**: Original C&C Generals (v1.08) codebase
- **GeneralsMD/**: Zero Hour expansion (v1.04) codebase - **primary focus**
- **Core/**: Shared game engine and libraries used by both games

### Key Components
- **Core/GameEngine/**: Base game engine with GameClient/GameLogic separation
- **Core/Libraries/**: Internal libraries including WWVegas graphics framework
- **Core/GameEngineDevice/**: Platform-specific rendering (DirectX 8)
- **Core/Tools/**: Development tools (W3DView, texture compression, etc.)
- **Dependencies/**: External dependencies (MaxSDK for VC6, utilities)

### Cross-Platform Compatibility Architecture
- **Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h**: Primary Windows API compatibility layer
- **Core/Libraries/Include/windows.h**: 16+ compatibility headers (mmsystem.h, winerror.h, objbase.h, etc.)
- **Multi-layer DirectX system**: Core layer + Generals layer coordination with include guards
- **Profile/Debug systems**: Full `__forceinline` macOS compatibility
- **Unified Configuration System**: Cross-platform INI-based config files replacing Windows Registry

### Configuration System Architecture (Phase 1 Implementation)

**Design Philosophy**: Complete removal of Windows Registry dependency in favor of unified cross-platform configuration files.

**Configuration File Locations**:
- **Linux/macOS**: `~/.config/cncgenerals.conf`, `~/.config/cncgeneralszh.conf`
- **Windows**: `%APPDATA%\CNC\cncgenerals.conf`, `%APPDATA%\CNC\cncgeneralszh.conf`

**Game Data Locations**:
- **macOS**: `~/Library/Application Support/CNC_Generals`, `~/Library/Application Support/CNC_GeneralsZH`
- **Linux**: `~/Games/CNC_Generals`, `~/Games/CNC_GeneralsZH`
- **Windows**: `%USERPROFILE%\Documents\Command and Conquer Generals Data`

**Registry API Replacement Strategy**:
- All `RegOpenKeyEx`, `RegQueryValueEx`, `RegSetValueEx`, `RegCloseKey` calls → INI file operations
- Registry paths like `SOFTWARE\Electronic Arts\EA Games\Generals` → INI sections `[EA.Games.Generals]`
- HKEY_LOCAL_MACHINE → system-wide defaults, HKEY_CURRENT_USER → user-specific overrides
- Automatic migration of existing Windows Registry values to INI format on first run

**Implementation Components**:
- **config_manager.h/cpp**: Cross-platform configuration management
- **ini_parser.h/cpp**: Lightweight INI file parser/writer
- **registry_compat.h**: Registry API → Config API mapping layer
- **path_resolver.h/cpp**: Platform-specific path resolution for game data and configs

**Key Advantages**:
- **Full Cross-Platform Compatibility**: Identical behavior on Windows/macOS/Linux
- **Human-Readable Configs**: INI format is transparent and easily editable
- **Version Control Friendly**: Config files can be versioned and shared
- **No System Dependencies**: Eliminates reliance on Windows Registry or platform-specific stores
- **Migration Support**: Automatic one-time migration from existing Windows Registry values

## Build System

### CMake Presets (Critical)
- **vc6**: Visual Studio 6 compatible build (retail compatibility required)
- **win32**: Modern Visual Studio 2022 build
- **vc6-debug/vc6-profile**: Debug/profiling variants
- Use `cmake --preset <preset-name>` followed by `cmake --build build/<preset>`

### Build Commands
```bash
# Configure with specific preset
cmake --preset vc6

# Build (from project root)
cmake --build build/vc6

# Build with tools and extras
cmake --build build/vc6 --target <game>_tools <game>_extras

# macOS Core Libraries (ALL SUCCESSFULLY COMPILING!)
cmake --build build/vc6 --target ww3d2 wwlib wwmath
```

### Retail Compatibility (Critical)
- VC6 builds are required for replay compatibility testing
- Debug builds break retail compatibility  
- Use RTS_BUILD_OPTION_DEBUG=OFF for compatibility testing

## Development Workflow

### Pull Request Guidelines
- Title format: `type: Description starting with action verb`
- Types: `bugfix:`, `feat:`, `fix:`, `refactor:`, `perf:`, `build:`
- Zero Hour changes take precedence over Generals
- Changes must be identical between both games when applicable

### Code Style
- Maintain consistency with surrounding legacy code
- Prefer C++98 style unless modern features add significant value
- No big refactors mixed with logical changes
- Use present tense in documentation ("Fixes" not "Fixed")

### Cross-Platform Development Patterns
- **Include Guards**: Use `#ifndef SYMBOL` guards to prevent redefinition conflicts between compatibility layers
- **DirectX Compatibility**: Coordinate between Core/win32_compat.h and Generals/d3d8.h definitions
- **Type Definitions**: Check for existing definitions before adding Windows types (DWORD, LARGE_INTEGER, etc.)
- **Compiler Intrinsics**: Use `__forceinline inline __attribute__((always_inline))` for macOS compatibility

## Testing

### Replay Compatibility Testing
Located in `GeneralsReplays/` - critical for ensuring retail compatibility:
```bash
generalszh.exe -jobs 4 -headless -replay subfolder/*.rep
```
- Requires VC6 optimized build with RTS_BUILD_OPTION_DEBUG=OFF
- Copies replays to `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Replays`
- CI automatically tests GeneralsMD builds against known replays

### Build Validation
- CI tests multiple presets: vc6, vc6-profile, vc6-debug, win32 variants
- Path-based change detection triggers relevant builds
- Tools and extras are built with `+t+e` flags

### macOS Compatibility Testing
```bash
# Test core libraries compilation
cmake --build build/vc6 --target ww3d2 wwlib wwmath

# Test Windows API compatibility headers
echo '#include "windows.h"' | c++ -x c++ -c -

# Test DirectX compatibility coordination
cmake --build build/vc6 --target g_ww3d2 2>&1 | grep -c "error:"
```

## Common Patterns

### Memory Management
- Manual memory management (delete/delete[]) - this is legacy C++98 code
- STLPort for VC6 compatibility (see `cmake/stlport.cmake`)

### Game Engine Separation
- **GameLogic**: Game state, rules, simulation
- **GameClient**: Rendering, UI, platform-specific code
- Clean separation maintained for potential future networking

### DirectX Wrapper Pattern
- **DX8Wrapper**: Central abstraction layer for DirectX 8 calls
- All DirectX calls go through wrapper for stat tracking and cross-platform compatibility
- Device management, render states, and resource creation centralized

### Module Structure
```
Core/
├── GameEngine/Include/Common/     # Shared interfaces
├── GameEngine/Include/GameLogic/  # Game simulation
├── GameEngine/Include/GameClient/ # Rendering/UI
├── Libraries/Include/rts/         # RTS-specific utilities
└── Libraries/Source/WWVegas/      # Graphics framework
    ├── WW3D2/win32_compat.h      # macOS compatibility layer
    └── WWLib/windows.h            # Windows API compatibility
```

## External Dependencies

### Required for Building
- **VC6 builds**: Requires MSVC 6.0 toolchain (automated in CI via itsmattkc/MSVC600)
- **Modern builds**: Visual Studio 2022, Ninja generator
- **vcpkg** (optional): zlib, ffmpeg for enhanced builds

### Platform-Specific
- **Windows**: DirectX 8, Miles Sound System, Bink Video
- **macOS**: Comprehensive Windows API compatibility layer (16+ headers)
- **Registry detection**: Automatic game install path detection from EA registry keys

## Tools and Utilities

### Development Scripts (`scripts/cpp/`)
- `fixInludesCase.sh`: Fix include case sensitivity
- `refactor_*.py`: Code refactoring utilities
- `remove_trailing_whitespace.py`: Code cleanup

### Build Tools
- W3DView: 3D model viewer
- TextureCompress: Asset optimization
- MapCacheBuilder: Map preprocessing

## Key Files to Understand
- `CMakePresets.json`: All build configurations
- `cmake/config-build.cmake`: Build options and feature flags
- `Core/GameEngine/Include/`: Core engine interfaces
- `**/Code/Main/WinMain.cpp`: Application entry points
- `GeneralsReplays/`: Compatibility test data
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`: macOS compatibility layer
- `MACOS_PORT.md`: Comprehensive macOS porting progress and technical details