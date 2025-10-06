# GeneralsX - Cross-Platform AI Instructions

## Project Overview
**GeneralsX** - Cross-platform port of Command & Conquer: Generals/Zero Hour. Modernized C++20 engine with native macOS/Linux/Windows compatibility.

## Quick Start Commands

### Primary Build Workflow (macOS ARM64)
```bash
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4  # Zero Hour (primary)
cmake --build build/macos-arm64 --target GeneralsX -j 4  # Original (secondary)
```

### Cross-Platform Alternatives
```bash
# Windows (VC6 compatibility): cmake --preset vc6
# macOS Intel: cmake --preset macos-x64  
# Linux: cmake --preset linux
```

### Runtime Testing & Debugging
```bash
# Setup assets directories with original game files
mkdir -p $HOME/GeneralsX/Generals
mkdir -p $HOME/GeneralsX/GeneralsMD

# Deploy Zero Hour (primary target)
cp ./build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
# Copy original Zero Hour game assets (Data/, Maps/) to $HOME/GeneralsX/GeneralsMD/

# Deploy Generals (secondary target)
cp ./build/macos-arm64/Generals/GeneralsX $HOME/GeneralsX/Generals/
# Copy original Generals game assets (Data/, Maps/) to $HOME/GeneralsX/Generals/

# Run Zero Hour with debug output  
cd $HOME/GeneralsX/GeneralsMD && ./GeneralsXZH

# Run Generals with debug output
cd $HOME/GeneralsX/Generals && ./GeneralsX

# Debug with LLDB when crashes occur
cd $HOME/GeneralsX/GeneralsMD && lldb ./GeneralsXZH
cd $HOME/GeneralsX/Generals && lldb ./GeneralsX
```

## Architecture Overview

### Directory Structure
- **GeneralsMD/**: Zero Hour expansion (PRIMARY TARGET) - main development focus
- **Generals/**: Original game (secondary) - used for comparison
- **Core/**: Shared 50MB+ libraries with `win32_compat.h` compatibility layer
- **references/**: Git submodules with working implementations for comparative analysis

### Cross-Platform Strategy
- **Pattern**: `#ifdef _WIN32` conditional compilation throughout codebase
- **Compatibility**: 200+ Windows APIs → POSIX alternatives via `win32_compat.h`  
- **Graphics**: DirectX8 → OpenGL through `dx8wrapper.cpp` (OpenGL chosen over Vulkan for simplicity and macOS native support)

## Critical Development Context

### 🎉 Phase 23.3 Universal INI Protection System (Phase 23.2)
**BREAKTHROUGH**: ControlBar crash COMPLETELY RESOLVED - engine progression unlocked
```cpp
// Early TheControlBar initialization in GameEngine::init() (SUCCESSFUL SOLUTION)
printf("GameEngine::init() - CRITICAL FIX: Initializing TheControlBar before INI parsing\n");
if (TheControlBar == NULL) {
    TheControlBar = NEW ControlBar;
    TheControlBar->init();
}
```
**Impact**: Engine advanced from TheThingFactory to TheGameClient (5+ subsystems progression)


### ✅ Memory Corruption Protection
```cpp
// AsciiString corruption detection in multiple places
if (m_data && ((uintptr_t)m_data < 0x1000)) {
    self->m_data = nullptr; // Reset corrupted pointer  
}

// Vector size validation with W3D PROTECTION logging
if (vectorSize > 100000) {
    printf("W3D PROTECTION: Vector corruption detected! Size %zu\n", vectorSize);
    return false;
}

// Font rendering macOS protection
if (!src_ptr || !data || !data->Buffer) {
    printf("FONT PROTECTION: NULL pointer in Blit_Char\n");
    return; // Safe fallback
}
```

## Reference Repository Strategy (Critical for Solutions)

### Available Git Submodules (Use these for comparative analysis!)
- **`references/jmarshall-win64-modern/`** - Windows 64-bit with modern features (console, D3D12, x64) - Base game (Generals) Only
- **`references/fighter19-dxvk-port/`** - Linux DXVK graphics integration with ARM64 support  
- **`references/dsalzner-linux-attempt/`** - POSIX compatibility focus

### Breakthrough Methodology
```bash
git submodule update --init --recursive  # Initialize all references
diff -r GeneralsMD/ references/jmarshall-win64-modern/GeneralsMD/  # Find working solutions
diff -r Core/ references/fighter19-dxvk-port/Core/  # Compare compatibility layers
```

**Success Example**: End token parsing crash resolved by comparing with jmarshall-win64-modern implementation

## Platform-Specific Patterns

### macOS (Primary Development Platform)
- **Native ARM64 builds** preferred for performance
- **Compatibility layers**: Win32 → POSIX in `win32_compat.h` (2270+ lines)
- **Graphics**: OpenGL preferred over Vulkan (avoids MoltenVK complexity)  
- **Build system**: CMake/Ninja with Apple Silicon optimizations

### Critical Files for AI Agents
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h` - 200+ Windows API mappings
- `NEXT_STEPS.md` - Current development status and crash analysis
- `MACOS_BUILD.md` - Complete build instructions and troubleshooting
- `BIG_FILES_REFERENCE.md` - Asset structure and INI file locations in .BIG archives

## INI File System (Game Configuration)

### .BIG File Structure (Critical for Debugging)
| INI Pattern | Primary Location | Fallback | Notes |
|-------------|------------------|----------|-------|  
| `Data\INI\*.ini` | INIZH.big | INI.big | Root configuration files |
| `Data\INI\Object\*.ini` | INIZH.big | INI.big | Object definitions |
| `Data\INI\Object\*General.ini` | **INIZH.big ONLY** | - | Zero Hour exclusive generals |

**Crash Investigation**: Check `$HOME/Documents/Command\ and\ Conquer\ Generals\ Zero\ Hour\ Data/ReleaseCrashInfo.txt`

**Build Tip**: Use `-j 4` (half CPU cores) to prevent system overload during compilation.

**AI Agent Priority**: When investigating crashes, always check reference implementations first for working solutions before implementing new fixes.

**Universal INI Protection System**: Comprehensive field parser exception handling enables engine continuation through hundreds of unknown exceptions while processing complex INI files. This breakthrough enabled progression from immediate GameLOD.ini crashes to advanced GameClient subsystem initialization.

**ControlBar Resolution Success**: Early TheControlBar initialization in GameEngine::init() completely eliminated parseCommandSetDefinition crashes, unlocking major engine progression through 5+ additional subsystems.