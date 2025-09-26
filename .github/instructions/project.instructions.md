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
- `z_generals` - Zero Hour expansion executable (PRIMARY TARGET - recommended)
- `g_generals` - Original Generals base game executable (SECONDARY TARGET)
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
11. **Primary build workflow**: Use `cmake --preset macos-arm64` for ARM64 native compilation on Apple Silicon, then `cmake --build build/macos-arm64 --target z_generals -j 4` for Zero Hour target.
12. **Platform-specific workflows**: 
    - **macOS ARM64**: `cmake --preset macos-arm64` → `cmake --build build/macos-arm64 --target z_generals -j 4`
    - **macOS Intel**: `cmake --preset macos-x64` → `cmake --build build/macos-x64 --target z_generals -j 4`
    - **Linux**: `cmake --preset linux` → `cmake --build build/linux --target z_generals -j 4`
    - **Windows**: `cmake --preset vc6` → `cmake --build build/vc6 --target z_generals -j 4`
13. **Target priority**: `z_generals` (Zero Hour) is the primary stable target, `g_generals` (Original Generals) is secondary. Core libraries (`ww3d2`, `wwlib`, `wwmath`) can be tested independently.