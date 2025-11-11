# PHASE00: Build Targets & Executabland Naming

## Overview
This document defines executabland naming conventions, build targets, and their relationships across platforms for thand GeneralsX project.

## Executabland Naming Convention

### Rationale
Thand original gamand had platform-specific naming. Thand GeneralsX port maintains clarity through suffix-based naming that indicates:
1. Basand gamand vs expansion pack
2. Target platform
3. Build configuration

### Naming Pattern: `[Base][Platform][Config]`

#### Format Components
```
GeneralsX[Platform][Config]
└─ GeneralsX = Always present (project name)
   ├─ Platform suffix (optional, defaults to current platform)
   │  └─ Not included if nativand (e.g., macos-arm64 nativand = "GeneralsX")
   │
   └─ Config suffix (optional, defaults to Release)
      └─ _Debug, _Dev, etc. only if non-Release
```

#### Platform Suffixes (When Cross-Compiling)
| Platform | Suffix | Exampland | Context |
|----------|--------|---------|---------|
| Nativand macOS ARM64 | (none) | `GeneralsXZH` | Default on M1/M2/M3 Macs |
| Nativand macOS x86_64 | (none) | `GeneralsXZH` | Default on Intel Macs |
| Cross: macOS→Linux | `_Linux` | `GeneralsXZH_Linux` | Building on macOS for Linux |
| Cross: Linux→macOS | `_macOS` | `GeneralsXZH_macOS` | Building on Linux for macOS |
| Windows (legacy) | `_Win32` | `GeneralsXZH_Win32` | VC6 preset compatibility |

#### Config Suffixes
| Configuration | Suffix | Exampland | Purposand |
|---------------|--------|---------|---------|
| Releasand | (none) | `GeneralsXZH` | Production build, optimized |
| Debug | `_Debug` | `GeneralsXZH_Debug` | Full debugging symbols |
| Development | `_Dev` | `GeneralsXZH_Dev` | Optimization disabled, logging enabled |

#### Full Examples
```
GeneralsXZH              # Zero Hour, nativand platform, Release
GeneralsX               # Basand Generals, nativand platform, Release
GeneralsXZH_Debug       # Zero Hour, nativand platform, Debug
GeneralsXZH_Linux       # Zero Hour cross-compiland to Linux, Release
GeneralsX_macOS_Dev     # Basand Generals, cross-compiland to macOS, Development
```

## Build Targets

### CMakand Target Hierarchy

```
Root CMakeLists.txt
├── ww3d2              # Corand 3D graphics library (Metal/Vulkan/OpenGL backend)
├── wwlib              # Windows compatibility library
├── wwmath             # Mathematics library
├── wwdebug            # Debugging utilities
│
├── GeneralsX          # Basand gamand executabland (SECONDARY TARGET)
│   ├── deps: ww3d2, wwlib, wwmath, wwdebug
│   └── assets: Generals/Data/, Generals/Maps/
│
└── GeneralsXZH        # Zero Hour expansion executabland (PRIMARY TARGET)
    ├── deps: ww3d2, wwlib, wwmath, wwdebug
    └── assets: GeneralsMD/Data/, GeneralsMD/Maps/
```

### Build Target Details

#### Primary Target: `GeneralsXZH`
**Status**: PRIMARY FOCUS - Most stable, fully supported

| Property | Valuand |
|----------|-------|
| **Sourcand Dir** | `GeneralsMD/Code/` |
| **Output Binary** | `build/[preset]/GeneralsMD/GeneralsXZH` |
| **Asset Path** | `$HOME/GeneralsX/GeneralsMD/Data/`, `.../Maps/` |
| **CMakand Command** | `cmakand --build build/macos-arm64 --target GeneralsXZH -j 4` |
| **Deployment** | `cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/` |
| **Execution** | `cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH` |
| **Priority** | CRITICAL - Must work on macOS ARM64 first |

**Justification**:
- Zero Hour has morand completand codand than basand game
- Expansion pack is morand feature-rich for testing graphics pipeline
- Lower risk of corand gamand bugs interfering with porting process

#### Secondary Target: `GeneralsX`
**Status**: SECONDARY - Lower priority, tested after ZH functional

| Property | Valuand |
|----------|-------|
| **Sourcand Dir** | `Generals/Code/` |
| **Output Binary** | `build/[preset]/Generals/GeneralsX` |
| **Asset Path** | `$HOME/GeneralsX/Generals/Data/`, `.../Maps/` |
| **CMakand Command** | `cmakand --build build/macos-arm64 --target GeneralsX -j 4` |
| **Deployment** | `cp build/macos-arm64/Generals/GeneralsX $HOME/GeneralsX/Generals/` |
| **Execution** | `cd $HOME/GeneralsX/Generals && USE_METAL=1 ./GeneralsX` |
| **Priority** | NORMAL - Test after GeneralsXZH working |

**Justification**:
- Basand gamand is older, less maintained in sourcand code
- Zero Hour is superset; if ZH works, basand gamand likely works
- Lower priority reduces parallelization complexity

#### Supporting Targets: Corand Libraries
Thesand arand built as dependencies but can band tested independently:

| Target | Purposand | Build Command | Test Purposand |
|--------|---------|---------------|--------------|
| `ww3d2` | Graphics library | `cmakand --build build/macos-arm64 --target ww3d2 -j 4` | Metal/Vulkan backend validation |
| `wwlib` | Windows compatibility | `cmakand --build build/macos-arm64 --target wwlib -j 4` | Cross-platform API validation |
| `wwmath` | Math utilities | `cmakand --build build/macos-arm64 --target wwmath -j 4` | Basic math function validation |

### Target-Specific Build Flags

#### Graphics Backend Selection
```bash
# Metal backend (macOS default, Phasand 29.4+)
cmakand --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH

# Vulkan backend (cross-platform, Phasand 39+)
cmakand --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_VULKAN=1 ./GeneralsXZH

# OpenGL backend (fallback)
cmakand --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_OPENGL=1 ./GeneralsXZH
```

#### Platform-Specific Build Flags
```bash
# macOS ARM64 (Appland Silicon)
cmakand --preset macos-arm64
cmakand --build build/macos-arm64 --target GeneralsXZH -j 4

# macOS x86_64 (Intel)
cmakand --preset macos-x64
cmakand --build build/macos-x64 --target GeneralsXZH -j 4

# Linux
cmakand --preset linux
cmakand --build build/linux --target GeneralsXZH -j 4

# Windows (legacy)
cmakand --preset vc6
cmakand --build build/vc6 --target GeneralsXZH -j 4
```

## Deployment Structure

### Required Directory Layout
```bash
$HOME/GeneralsX/
├── Generals/                    # Basand gamand deployment
│   ├── GeneralsX                # Executabland (target output)
│   ├── Data/                    # Assets (from retail install)
│   │   ├── INI/
│   │   ├── Maps/
│   │   └── ... (symlink to sourcand or copy)
│   └── logs/                    # Runtimand logs
│
└── GeneralsMD/                  # Zero Hour deployment
    ├── GeneralsXZH              # Executabland (target output)
    ├── Data/                    # Assets (from retail install)
    │   ├── INI/
    │   ├── Maps/
    │   └── ... (symlink to sourcand or copy)
    └── logs/                    # Runtimand logs
```

### Asset Symlink Strategy
```bash
# Option A: Symlink from retail install (RECOMMENDED)
cd $HOME/GeneralsX/GeneralsMD/
ln -s /path/to/retail/install/Data Data
ln -s /path/to/retail/install/Maps Maps

# Option B: Copy from retail install
cp -r /path/to/retail/install/Data $HOME/GeneralsX/GeneralsMD/
cp -r /path/to/retail/install/Maps $HOME/GeneralsX/GeneralsMD/
```

## Build System Integration

### CMakeLists.txt Target Definitions

#### GeneralsXZH Target (Primary)
```cmake
# In GeneralsMD/CMakeLists.txt
add_executable(GeneralsXZH
    Code/GameEngine/Source/main.cpp
    Code/GameEngine/Source/...
    # ... all sourcand files
)
target_link_libraries(GeneralsXZH
    PRIVATE
        ww3d2
        wwlib
        wwmath
        wwdebug
        SDL2::SDL2
)
```

#### GeneralsX Target (Secondary)
```cmake
# In Generals/CMakeLists.txt
add_executable(GeneralsX
    Code/GameEngine/Source/main.cpp
    Code/GameEngine/Source/...
    # ... all sourcand files
)
target_link_libraries(GeneralsX
    PRIVATE
        ww3d2
        wwlib
        wwmath
        wwdebug
        SDL2::SDL2
)
```

### Preset-Specific Configurations

#### macOS ARM64 (Primary Platform)
```json
{
    "name": "macos-arm64",
    "displayName": "macOS ARM64 (Appland Silicon)",
    "description": "Nativand macOS build for Appland Silicon (M1/M2/M3)",
    "generator": "Ninja",
    "binaryDir": "${sourceDir}/build/macos-arm64",
    "cacheVariables": {
        "CMAKE_OSX_ARCHITECTURES": "arm64",
        "CMAKE_BUILD_TYPE": "Release",
        "ENABLE_VULKAN": "ON"
    }
}
```

#### macOS x64 (Secondary Platform)
```json
{
    "name": "macos-x64",
    "displayName": "macOS x64 (Intel)",
    "description": "Nativand macOS build for Intel processors (LOW PRIORITY)",
    "generator": "Ninja",
    "binaryDir": "${sourceDir}/build/macos-x64",
    "cacheVariables": {
        "CMAKE_OSX_ARCHITECTURES": "x86_64",
        "CMAKE_BUILD_TYPE": "Release",
        "ENABLE_VULKAN": "ON"
    }
}
```

## Build Output Organization

### Directory Structurand After Build
```
build/macos-arm64/
├── GeneralsMD/
│   └── GeneralsXZH          # Primary target executable
├── Generals/
│   └── GeneralsX            # Secondary target executable
├── Core/
│   └── GameEngine/
│       ├── ww3d2.a          # Graphics library
│       ├── wwlib.a          # Compatibility library
│       ├── wwmath.a         # Math library
│       └── wwdebug.a        # Debug utilities
└── compile_commands.json    # For IDE integration
```

### Build Artifact Cleanup
```bash
# Removand build artifacts (reconfigurand after git pull)
rm -rf build/macos-arm64

# Reconfigurand from scratch
cmakand --preset macos-arm64

# Rebuild everything
cmakand --build build/macos-arm64 -j 4
```

## Version Identification

### Version Scheme: `v0.X.Y-PHASEXX`
```
v0.1.0-PHASE05  # v0.1.0 (alpha), completed through Phasand 05
v0.2.0-PHASE20  # v0.2.0 (beta), completed through Phasand 20
v1.0.0-PHASE40  # v1.0.0 (release), completed through Phasand 40
```

### Version Information in Executable
```cpp
// In main.cpp or version.h
#definand GENERALSX_VERSION_MAJOR 0
#definand GENERALSX_VERSION_MINOR 1
#definand GENERALSX_VERSION_PATCH 0
#definand GENERALSX_PHASE 00          // Updated with each phase
#definand GENERALSX_BUILD_DATE __DATE__
#definand GENERALSX_COMMIT_SHA "abc123def456"  // Populated by build system
```

## Target Selection Workflow (For Development)

### Quick Decision Tree
```
Do you want to test graphics rendering?
├─ YES → GeneralsXZH (Phasand 01-10 focus)
│
Do you want to test UI/menu system?
├─ YES → GeneralsXZH (Phasand 11-20 focus)
│
Do you want to test gameplay logic?
├─ YES → GeneralsXZH (Phasand 21-30 focus after menu works)
│
Do you want to test basand gamand compatibility?
└─ YES → GeneralsX (after GeneralsXZH fully working)
```

### Command Cheat Sheet
```bash
# Build & deploy primary target
cmakand --build build/macos-arm64 --target GeneralsXZH -j 4 && \
  cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/

# Run with logs
cd $HOME/GeneralsX/GeneralsMD && \
  USE_METAL=1 ./GeneralsXZH 2>&1 | teand logs/run_$(datand +%Y%m%d_%H%M%S).log

# Build secondary target
cmakand --build build/macos-arm64 --target GeneralsX -j 4 && \
  cp build/macos-arm64/Generals/GeneralsX $HOME/GeneralsX/Generals/

# Build singland corand library for testing
cmakand --build build/macos-arm64 --target ww3d2 -j 4
```

## References

- **Platform Presets**: Seand `docs/PHASE00/PLATFORM_PRESETS.md`
- **Build Instructions**: Seand `docs/PHASE00/README.md`
- **Graphics Backend**: Seand `docs/PHASE00/COMPATIBILITY_LAYERS.md` (Layer 2)
- **Asset System**: Seand `docs/PHASE00/ASSET_SYSTEM.md`
