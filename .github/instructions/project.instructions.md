---
applyTo: '**'
---
This project is a fork of the Command & Conquer Generals source code and its expansion Zero Hour.

# imperative-project-instructions

- Ensure the project have a read-only git remote called `thesupehackers` poiting to `git@github.com:TheSuperHackers/GeneralsGameCode.git`
- Once a day, before start working, merge the latest changes from `thesupehackers` main branch into local current branch.
 - In case of merge conflicts, resolve them carefully, ensuring no local changes are lost.
 - Avoid using `theirs` strategy unless absolutely necessary.
- It's imperative to use `tee` to log all compilation and debugging output to files for later analysis.
  - use `logs/phaseXX_build.log` for compilation logs - replace `XX` with the current phase number.
  - use `logs/phaseXX_debug.log` for debugging logs - replace `XX` with the current phase number.
  - You can skip log naming convention if, and only if, you need to test different targets/platforms in the same session.
- It's a very old codebase, so be patient and meticulous when dealing with legacy constructs.
- Total focus on Generals Zero Hour (`GeneralsXZH`) target as the primary stable target, at the end of the development process we can backport changes to the Generals (`GeneralsX`) target.
  - Avoid to direct copy-paste code between targets, always try to implement changes in a way that both targets can benefit from them.
  - There is a plenty of tools that can be used to get the differences between both targets, use them to understand what changed between targets.
- We have a few `references/` repositories as git submodules for comparative analysis and solution discovery. Use them wisely.
- Any support scripts must be placed in the `scripts/` directory.
- Commit at the end of each working session with a detailed message about the changes made.
- Create tags for major milestones or completed phases for easy reference and rollback.
- The local default branch is `vulkan-port`, ensure to keep it updated with `upstream/main` daily after 7:00 PM user time.

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

## Compilation Guidelines

- When compiling, use the `-j` flag with half the number of available CPU cores to avoid overloading the system.
- use `tee` to log the output of the compilation process to a file for later analysis.
- Command example:
```bash
# Example for macOS ARM64 - GeneralsXZH
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | tee /tmp/generalsxzh_build.log.txt

# Example for macOS ARM64 - GeneralsX
cmake --build build/macos-arm64 --target GeneralsX -j 4 2>&1 | tee /tmp/generalsx_build.log.txt
```

## Debugging Guidelines

- For debugging purposes, do not use commands like `grep` or `tail` into a command run to avoid hanging the terminal if the command produces a lot of output - Instead, use `tee` to log the output to a file and then use `grep` on the log file.
- Command example:
```bash
# Example for macOS ARM64 - GeneralsXZH
cd $HOME/GeneralsX/GeneralsMD/ && USE_METAL=1 ./GeneralsXZH 2>&1 | tee /tmp/generalszh.log
grep -i error /tmp/generalszh.log
```
- use `lldb` for debugging metal backend related crashes.
- Command example:
```bash
# Example for macOS ARM64 - GeneralsXZH
cd $HOME/GeneralsX/GeneralsMD/ && USE_METAL=1 lldb -o run -o bt -o quit ./GeneralsXZH 2>&1 |tee /tmp/generalszh_lldb.log
```

### Debugging Best Practices (Lessons Learned)

1. **Complete Log Capture is Essential**
   - **DO NOT** use short timeouts (5-15 seconds) during initial testing - game initialization can take significant time
   - **USE** `tee` to save logs to `logs/` directory for later analysis instead of `/tmp` (which may be cleared)
   - **WAIT** for game to stabilize before assuming a crash occurred
   - Example: Game may take 30+ seconds to load .big files, parse INI files, and initialize graphics

2. **Grep Filtering Can Hide Progress**
   - Repetitive logs (like BeginFrame/EndFrame cycles) may appear as infinite loops when filtered
   - **VERIFY** process state with `ps aux` before assuming crash
   - **CHECK** full unfiltered logs first, then apply targeted grep patterns
   - Example: `grep -A 5 -B 5 "error\|fatal"` provides context around errors

3. **Distinguish Real Crashes from Slow Initialization**
   - **CHECK** if process is still running: `ps aux | grep GeneralsXZH`
   - **VERIFY** CPU usage: High CPU = still processing, Low CPU = might be stuck
   - **LOOK** for crash logs: `$HOME/Documents/Command and Conquer Generals Zero Hour Data/ReleaseCrashInfo.txt`
   - **WAIT** for complete initialization before investigating "apparent" hangs

4. **Memory Protection Systems Work**
   - Phase 30.6 memory protections (`GameMemory.cpp::isValidMemoryPointer()`) successfully prevent driver bugs
   - AGXMetal13_3 crash was resolved by validation checks, not workarounds
   - Trust protection layers but verify with extensive testing

5. **Persistent Log Directory**
   - **USE** `logs/` directory in project root (gitignored) for all debug output
   - **AVOID** `/tmp` logs that disappear between sessions
   - **DOCUMENT** log purpose in filename: `metal_texture_test_20251017.log`
   - **KEEP** logs for resolved bugs as reference for similar issues

## General Instructions

1. The main goal is to port the game to run on Windows, Linux, and macOS systems, starting by updating the graphics library to OpenGL, the extras and tools will be implemented afterwards.
2. The name of this project is "GeneralsX", please use this name in commit messages and pull requests.
3. All tests must be moved into a dedicated `tests/` directory to improve project organization and maintainability.
4. **Documentation Organization**: All new markdown documentation files must be created in the `docs/` directory unless explicitly instructed otherwise. Only essential GitHub files (README.md, LICENSE.md, CONTRIBUTING.md, SECURITY.md) should remain in the repository root.
4.1 The phases documentation files must have its own directory inside `docs/` named `docs/PHASEXX` where `XX` is the phase number (e.g., `docs/PHASE01`, `docs/PHASE02`, etc.)
4.2 The phase documentation files must be named `PHASEXX/DESCRIPTION.md` where `XX` is the phase number and `DESCRIPTION` is a short description of the phase (e.g., `docs/PHASE01/INITIAL_REFACTOR.md`, `docs/PHASE02/OPENGL_IMPLEMENTATION.md`, etc.)
4.3 **Build Instructions**: Platform-specific build instructions are in `docs/MACOS_BUILD_INSTRUCTIONS.md` and `docs/LINUX_BUILD_INSTRUCTIONS.md` (NOT `MACOS_BUILD.md` or `LINUX_BUILD.md`)
5. The game uses Windows Registry keys for configuration and settings storage. When porting to other platforms, these keys need to be replaced with equivalent configuration files or system settings.
6. before finish a session, update the technical development diary `docs/MACOS_PORT_DIARY.md` with the progress made.
7. For game base (generals), there is a crash log in `$HOME/Documents/Command\ and\ Conquer\ Generals\ Data/ReleaseCrashInfo.txt` that can be used to debug runtime issues.
8. For game expansion (zero hour), there is a crash log in `$HOME/Documents/Command\ and\ Conquer\ Generals\ Zero\ Hour\ Data/ReleaseCrashInfo.txt` that can be used to debug runtime issues.
9. When compiling the project, try to use half of the available CPU cores to avoid overloading the system.
10. For understanding the game asset structure and debugging INI-related issues, refer to `docs/Misc/BIG_FILES_REFERENCE.md` which contains complete documentation of .big file contents and their relationships to INI files.
11. **Game Assets Location**: Use standardized directories for runtime testing:
    - **Generals (base game)**: `$HOME/GeneralsX/Generals/` - Copy original game assets (Data/, Maps/) here
    - **Zero Hour (expansion)**: `$HOME/GeneralsX/GeneralsMD/` - Copy Zero Hour assets (Data/, Maps/) here
    - Deploy executables to their respective directories for testing
12. **Primary build workflow**: Use `cmake --preset macos-arm64` for ARM64 native compilation on Apple Silicon, then `cmake --build build/macos-arm64 --target GeneralsXZH -j 4` for Zero Hour target.
13. **Platform-specific workflows**:
    - **macOS ARM64**: `cmake --preset macos-arm64` → `cmake --build build/macos-arm64 --target GeneralsXZH -j 4 | tee /tmp/generalsxzh_build.log.txt`
    - **macOS Intel**: `cmake --preset macos-x64` → `cmake --build build/macos-x64 --target GeneralsXZH -j 4 | tee /tmp/generalsx_build.log.txt`
    - **Linux**: `cmake --preset linux` → `cmake --build build/linux --target GeneralsXZH -j 4 | tee /tmp/generalsxzh_build.log.txt`
    - **Windows**: `cmake --preset vc6` → `cmake --build build/vc6 --target GeneralsXZH -j 4 | tee /tmp/generalsxzh_build.log.txt`
14. **Target priority**: `GeneralsXZH` (Zero Hour) is the primary stable target, `GeneralsX` (Original Generals) is secondary. Core libraries (`ww3d2`, `wwlib`, `wwmath`) can be tested independently.

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

# Vulkan SDK Documentation Reference (Phase 39 & Beyond)

**Location**: `docs/Vulkan/` directory (local documentation)

## Setup & Documentation

### Vulkan SDK Installation
- **Official SDK**: https://vulkan.lunarg.com/ (for updates and official resources)
- **macOS Specific**: https://vulkan.lunarg.com/doc/sdk/1.4.328.1/mac/
- **Status**: Vulkan SDK 1.4.328.1 installed and validated on macOS ARM64
- **Validation**: `vkcube` and `vkconfig-gui` confirmed working

### Local Documentation

**Download Documentation** (if not already present):
```bash
# Set Vulkan version
VULKAN_VERSION="1.4.328.1"

# Download documentation to docs/Vulkan/
cd docs/Vulkan/
wget "https://files.lunarg.com/VulkanSDK/VulkanSDK-Mac-Docs-${VULKAN_VERSION}.zip"
unzip "VulkanSDK-Mac-Docs-${VULKAN_VERSION}.zip"
```

**Key Documentation Files**:
- `getting_started.html` - **START HERE** for Phase 39
  - Creating a Vulkan Instance
  - Physical Device Selection
  - Creating a Logical Device
  - Command Buffers & Recording
  - Render Passes & Framebuffers
  - Presentation Engine (Swapchain)

- `best_practices.html` - **CRITICAL** for performance
  - Queue Management
  - Memory Management & Allocation Strategies
  - Pipeline Cache Management
  - Synchronization (Semaphores, Fences)
  - Performance Optimization Tips

- `layer_user_guide.html` - Debugging and validation
  - Validation Layers (detecting errors)
  - Performance Layers (profiling)
  - Debugging Techniques for Graphics Issues

- `runtime_guide.html` - macOS-specific information
  - Layer Configuration
  - Performance Monitoring
  - macOS Runtime Considerations

### Phase 39 Reference

**Architecture Analysis**: See `docs/MISC/VULKAN_ANALYSIS.md`
- Complete DirectX 8 → Vulkan method mapping
- All 47 IGraphicsBackend methods documented
- DXVK integration strategy explained
- Risk assessment and validation plan

**DXVK Implementation**: Phase 39 uses Vulkan SDK for:
- VkInstance creation
- Physical device enumeration
- VkDevice and VkQueue management
- Command buffer recording
- Render pass setup
- Resource management (buffers, images, samplers)

### When You Need Vulkan Docs

**During Phase 39 Implementation**:
- Implementing DXVKGraphicsBackend methods → Reference `getting_started.html`
- Optimizing frame rendering performance → Reference `best_practices.html`
- Debugging GPU validation errors → Reference `layer_user_guide.html`
- Troubleshooting macOS-specific issues → Reference `runtime_guide.html`

**During Phase 40+ Graphics Development**:
- Advanced rendering features
- Performance optimization
- Cross-platform compatibility issues
- GPU driver-specific workarounds

### GitIgnore Note

Large Vulkan SDK documentation files are gitignored:
```
docs/Vulkan/*.zip
docs/Vulkan/VulkanSDK-Mac-Docs-*/
```

Only `docs/Vulkan/README.md` is tracked (contains setup instructions).