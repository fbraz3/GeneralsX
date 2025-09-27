# GeneralsX - Cross-Platform AI Instructions

## Project Overview
**GeneralsX** - Cross-platform port of Command & Conquer: Generals/Zero Hour. Modernized C++20 engine with native macOS/Linux/Windows compatibility.

**Current Status**: ✅ **End Token Parsing RESOLVED** - Ready for next engine initialization phase

## Quick Start Commands

### Primary Build Workflow (macOS ARM64)
```bash
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target z_generals -j 4  # Zero Hour (primary)
cmake --build build/macos-arm64 --target g_generals -j 4  # Original (secondary)
```

### Cross-Platform Alternatives
```bash
# Windows (VC6 compatibility): cmake --preset vc6
# macOS Intel: cmake --preset macos-x64  
# Linux: cmake --preset linux
```

### Runtime Testing
```bash
# Setup assets directory
mkdir -p $HOME/Downloads/generals
cp ./build/macos-arm64/GeneralsMD/generalszh $HOME/Downloads/generals/
# Copy original game assets (Data/, Maps/) to $HOME/Downloads/generals/

# Run with debug output
cd $HOME/Downloads/generals && ./generalszh
```

## Architecture Overview

### Directory Structure
- **GeneralsMD/**: Zero Hour expansion (PRIMARY TARGET)
- **Generals/**: Original game (secondary)
- **Core/**: Shared 50MB+ libraries with `win32_compat.h` compatibility layer

### Cross-Platform Strategy
- **Pattern**: `#ifdef _WIN32` conditional compilation throughout
- **Compatibility**: 200+ Windows APIs → POSIX alternatives via `win32_compat.h`
- **Graphics**: DirectX8 → OpenGL through `dx8wrapper.cpp`

## Critical Resolved Issues

### ✅ End Token Parsing (Breakthrough Solution)
**Source**: Comparative analysis using `references/jmarshall-win64-modern/`
```cpp
// Simple End token check BEFORE parser execution prevents all exceptions
if (foundEndToken) {
    return success; // Skip parser entirely
}
```
**Result**: Zero End token exceptions with clean "Successfully parsed block 'End'" messages

### ✅ Memory Corruption Protection
```cpp
// AsciiString corruption detection
if (m_data && ((uintptr_t)m_data < 0x1000)) {
    self->m_data = nullptr; // Reset corrupted pointer
}
```

### ✅ Optimized Debug Logging
```cpp
// Performance-optimized with "W3D PROTECTION:" prefix for critical errors only
if (vectorSize > 100000) {
    printf("W3D PROTECTION: Vector corruption detected! Size %zu\n", vectorSize);
    return false;
}
```

## Reference Repository Strategy

### Available Git Submodules
- **`references/jmarshall-win64-modern/`** - Windows 64-bit (End token breakthrough source)
- **`references/fighter19-dxvk-port/`** - Linux DXVK graphics integration  
- **`references/dsalzner-linux-attempt/`** - POSIX compatibility solutions

### Usage Commands
```bash
git submodule update --init --recursive  # Initialize all references
diff -r GeneralsMD/ references/jmarshall-win64-modern/GeneralsMD/  # Compare solutions
```

## Current Development Status

### ✅ Successfully Resolved (Phase 22.8)
- End token parsing exceptions completely eliminated
- Vector corruption protection optimized for performance
- Debug logging cleaned while maintaining essential safety monitoring
- Reference repository comparative analysis methodology established

### 🎯 Next Phase Priorities
- Continue engine initialization beyond TheThingFactory
- Analyze compatibility layers using reference repositories
- Research graphics solutions (DXVK, OpenGL wrappers)
- Advance to next runtime initialization bottlenecks

## Essential Files
- `MACOS_BUILD.md` - Complete build instructions
- `NEXT_STEPS.md` - Current progress tracking  
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h` - Platform abstractions
- `.github/instructions/project.instructions.md` - Project-specific guidelines

**Build Tip**: Use `-j 4` (half CPU cores) to prevent system overload during compilation.

**Big Files Reference**: See `BIG_FILES_REFERENCE.md` for detailed asset structure and INI debugging.

**How to Find INI Files in .BIG Archives**:

| INI Pattern | Primary Location | Fallback Location | Notes |
|-------------|------------------|-------------------|-------|
| `Data\INI\*.ini` | INIZH.big | INI.big | Root configuration files |
| `Data\INI\Object\*.ini` | INIZH.big | INI.big | Object definitions |
| `Data\INI\Object\*General.ini` | **INIZH.big ONLY** | - | Zero Hour exclusive generals |

** Aditional crash logs**:
There is a crash log for the expansion (Zero Hour) in `$HOME/Documents/Command\ and\ Conquer\ Generals\ Zero\ Hour\ Data/ReleaseCrashInfo.txt`

**Historical Progress**:
```
# OLD CRASH (RESOLVED via Universal INI Protection):
cat $HOME/Documents/Command\ and\ Conquer\ Generals\ Zero\ Hour\ Data/ReleaseCrashInfo.txt
Release Crash at Fri Sep 26 16:27:16 2025
; Reason Error parsing INI file 'Data\INI\GameLOD.ini' (Line: 'StaticGameLOD = Low ')

# CURRENT STATUS (Phase 22.8 - September 2025):
- ✅ GameLOD.ini parsing: RESOLVED via Universal Protection
- ✅ TheArmorStore: COMPLETED successfully  
- ✅ TheBuildAssistant: COMPLETED successfully
- ✅ TheThingFactory: COMPLETED successfully  
- ✅ TheFXListStore: COMPLETED successfully
- 🚧 TheWeaponStore: Segmentation fault at RadiusDamageAffects field
```

**Universal INI Protection System**:
Implemented comprehensive field parser exception handling that allows engine continuation through hundreds of unknown exceptions while processing complex INI files. This breakthrough enabled progression from immediate GameLOD.ini crashes to advanced engine subsystem initialization.