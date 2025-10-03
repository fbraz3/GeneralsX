---
applyTo: '**'
---
This project is a fork of the Command & Conquer Generals source code and its expansion Zero Hour.

# Project directory Structure
- `/Generals` - The main game source code.
- `/GeneralsMD` - The expansion pack source code.
- `/Core` - Shared libraries and utilities.
- `/Dependencies` - External libraries and frameworks.

# Compilation Parameters
- `-DRTS_BUILD_ZEROHOUR='ON'` - compiles the Zero Hour expansion
- `-DRTS_BUILD_GENERALS='ON'` - compiles the Generals base game
- `-DENABLE_OPENGL='ON'` - enables OpenGL graphics support
- `-DCMAKE_OSX_ARCHITECTURES=arm64` - enables native ARM64 compilation on Apple Silicon (recommended for macOS)
- `-DRTS_BUILD_CORE_TOOLS='ON'` - compiles the core tools
- `-DRTS_BUILD_GENERALS_TOOLS='ON'` - compiles the Generals tools
- `-DRTS_BUILD_ZEROHOUR_TOOLS='ON'` - compiles the Zero Hour tools
- `-DRTS_BUILD_CORE_EXTRAS='ON'` - compiles the core extras
- `-DRTS_BUILD_GENERALS_EXTRAS='ON'` - compiles the Generals extras
- `-DRTS_BUILD_ZEROHOUR_EXTRAS='ON'` - compiles the Zero Hour extras

# Build Presets (Platform-Specific)
- `vc6` - Windows 32-bit (Visual C++ 6 compatibility)
- `macos-arm64` - **macOS Apple Silicon (ARM64) - PRIMARY TARGET**
- `macos-x64` - macOS Intel (x86_64) 
- `linux` - Linux 64-bit (x86_64)

# Build Targets
- `GeneralsXZH` - Zero Hour expansion executable (PRIMARY TARGET - recommended)
- `GeneralsX` - Original Generals base game executable (SECONDARY TARGET)
- `ww3d2` - Core 3D graphics library
- `wwlib` - Core Windows compatibility library  
- `wwmath` - Core mathematics library

# Project Specific Instructions
1. The main goal is to port the game to run on Windows, Linux, and macOS systems, starting by updating the graphics library to OpenGL, the extras and tools will be implemented afterwards.
2. The name of this project is "GeneralsX", please use this name in commit messages and pull requests.
3. All tests must be moved into a dedicated `tests/` directory to improve project organization and maintainability.
4. The game uses Windows Registry keys for configuration and settings storage. When porting to other platforms, these keys need to be replaced with equivalent configuration files or system settings.
5. before finish a session, update the files `MACOS_PORT.md` and `NEXT_STEPS.md` with the progress made and the next steps to be taken.
6. Commit and push changes before finish a session (and after changing markdown files from step 4).
7. For game base (generals), there is a crash log in `$HOME/Documents/Command\ and\ Conquer\ Generals\ Data/ReleaseCrashInfo.txt` that can be used to debug runtime issues.
8. For game expansion (zero hour), there is a crash log in `$HOME/Documents/Command\ and\ Conquer\ Generals\ Zero\ Hour\ Data/ReleaseCrashInfo.txt` that can be used to debug runtime issues.
9. When compiling the project, try to use half of the available CPU cores to avoid overloading the system.
10. For understanding the game asset structure and debugging INI-related issues, refer to `BIG_FILES_REFERENCE.md` which contains complete documentation of .big file contents and their relationships to INI files.
11. **Primary build workflow**: Use `cmake --preset macos-arm64` for ARM64 native compilation on Apple Silicon, then `cmake --build build/macos-arm64 --target GeneralsXZH -j 4` for Zero Hour target.
12. **Platform-specific workflows**: 
    - **macOS ARM64**: `cmake --preset macos-arm64` → `cmake --build build/macos-arm64 --target GeneralsXZH -j 4`
    - **macOS Intel**: `cmake --preset macos-x64` → `cmake --build build/macos-x64 --target GeneralsXZH -j 4`
    - **Linux**: `cmake --preset linux` → `cmake --build build/linux --target GeneralsXZH -j 4`
    - **Windows**: `cmake --preset vc6` → `cmake --build build/vc6 --target GeneralsXZH -j 4`
13. **Target priority**: `GeneralsXZH` (Zero Hour) is the primary stable target, `GeneralsX` (Original Generals) is secondary. Core libraries (`ww3d2`, `wwlib`, `wwmath`) can be tested independently.

# Reference Repositories (Git Submodules)
The project includes reference repositories as git submodules for comparative analysis and solution discovery:

## Available Reference Repositories
- **`references/jmarshall-win64-modern/`** - Windows 64-bit modernization with comprehensive fixes - Game base (Generals) Only
  - **Primary use**: Cross-platform compatibility solutions, INI parser fixes, memory management
  - **Key success**: Provided the breakthrough End token parsing solution (Phase 22.7-22.8)
  - **Coverage**: Full Windows 64-bit port with modern toolchain compatibility

- **`references/fighter19-dxvk-port/`** - Linux port with DXVK graphics integration  
  - **Primary use**: Graphics layer solutions (DirectX→Vulkan via DXVK), Linux compatibility
  - **Focus areas**: OpenGL/Vulkan rendering, graphics pipeline modernization
  - **Coverage**: Complete Linux port with advanced graphics compatibility

- **`references/dsalzner-linux-attempt/`** - Linux port attempt with POSIX compatibility
  - **Primary use**: Win32→POSIX API translations, Linux-specific fixes
  - **Focus areas**: System calls, file handling, threading compatibility
  - **Coverage**: Partial Linux port focusing on core system compatibility

- **`references/dxgldotorg-dxgl/`** - DirectDraw/Direct3D7 to OpenGL compatibility layer
  - **Primary use**: DirectX→OpenGL wrapper techniques, mock interface patterns, graphics compatibility
  - **Focus areas**: DirectX API stubbing, OpenGL rendering pipeline, device capability emulation
  - **Coverage**: Complete DirectDraw/D3D7 wrapper with mature OpenGL backend
  - **Note**: While focused on D3D7, provides excellent patterns for DirectX8 compatibility layer development

## Reference Analysis Workflow
1. **Comparative debugging**: When encountering complex cross-platform issues, examine equivalent code in reference repositories
2. **Solution discovery**: Use `git submodule update --init --recursive` to initialize all references
3. **Code comparison**: Compare implementations across repositories to identify proven solutions
4. **Best practices**: Extract successful patterns and apply with GeneralsX-specific adaptations

## Reference Repository Commands
```bash
# Initialize all reference repositories
git submodule update --init --recursive

# Update references to latest versions  
git submodule update --remote

# Navigate to specific reference
cd references/jmarshall-win64-modern/
cd references/fighter19-dxvk-port/
cd references/dsalzner-linux-attempt/

# Compare implementations
diff -r GeneralsMD/Code/GameEngine/ references/jmarshall-win64-modern/GeneralsMD/Code/GameEngine/
```

**Success Story**: The End token parsing issue (Phase 22.7) was resolved through comparative analysis of `jmarshall-win64-modern`, leading to a simple but effective solution that eliminated all INI parser exceptions.