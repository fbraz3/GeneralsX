---
applyTo: '**'
---

# Objectives

GeneralsX is a port of Command & Conquer: Generals (2002) and its expansion Zero Hour aka GeneralsMD (2003) modernizing the legacy DirectX 8 Windows codebase with SDL2 for input/windowing and Vulkan for graphics rendering.

This is a **massive C++ game engine** (~500k+ LOC) being ported from Visual C++ 6 (C++98) to modern C++20 with improved code structure and platform abstraction layers.

**Critical Context**: This is NOT a greenfield project. You're working with 20+ year old game code that assumes Windows everywhere. Respect the existing architecture while modernizing the codebase.

**Platform Strategy**:
- **Phase 1 (Current)**: Windows 32-bit via VC6 with SDL2 for windowing/input and OpenAL for audio
- **Phase 2**: Windows 32-bit via VC6 with Vulkan graphics backend (DirectX 8 → Vulkan translation)
- **Phase 3**: Cross-platform support via Wine emulation layer on macOS and Linux

## Windows-First Development Strategy

All development focuses on **Windows 32-bit (VC6 toolchain)** as the primary and only native compilation target. This ensures maximum compatibility with the legacy codebase.

Registry keys must be replaced by configuration files, details about file formats and locations can be found into `assets/ini/README.md` file.

When implementing SDL2 and Vulkan abstractions, use platform abstraction layers that will eventually support Wine on macOS/Linux without code duplication.

## Primary Goals (Phase 1: SDL2 + OpenAL)

**Architecture**: Replace Win32 windowing and input code with SDL2 abstractions; replace DirectSound with OpenAL for audio.
**Testing Focus**: Windows 32-bit VC6 only.
**Acceptance Criteria**: All core gameplay works identically on Windows with SDL2/OpenAL instead of Win32/DirectSound.

After completing Phase 1, the game must be able to (on Windows VC6):

- Load game assets (.big files, models, textures, sounds, etc)
- Load initial screen and main menu
- Navigate menus and launch games
- Start and play a skirmish game (single-player vs AI) successfully
- Play Campaign missions successfully
- Play Generals Challenge successfully
- All audio output handled via OpenAL (no DirectSound dependencies)

## Secondary Goals (Phase 2: Vulkan Graphics Backend)

**Architecture**: Replace DirectX 8 graphics pipeline with Vulkan abstraction layer.
**Testing Focus**: Windows 32-bit VC6 only.
**Acceptance Criteria**: All rendering works identically on Vulkan as on DirectX 8.

For Phase 2 we focus on graphics backend:

- Implement DirectX 8 to Vulkan translation layer (abstraction at the graphics API boundary)
- Port all render state, texture stage operations, and draw calls to Vulkan
- Optimize graphics performance for Vulkan backend
- Ensure all features from Phase 1 work flawlessly with Vulkan backend
- Maintain equivalent visual fidelity and performance to DirectX 8 baseline

## Tertiary Goals (Phase 3: Wine Cross-Platform Support)

**Architecture**: Build Wine abstraction layer for macOS and Linux support.
**Testing Focus**: Windows 32-bit VC6 first, then Wine on macOS/Linux.
**Refs**: `docs\WORKDIR\analysis\WINE_ANALYSIS_AND_STRATEGY.md`

- Establish Wine build configuration and toolchain
- Test SDL2 and Vulkan functionality through Wine on macOS and Linux
- Verify file system paths and asset loading in Wine environments
- Optimize Wine compatibility

## Quaternary Goals (Post-Launch)

**Architecture**: Enhanced gameplay features.
**Testing Focus**: Windows VC6 as baseline, Wine cross-platform validation.

- Multiplayer support (LAN only initially, online will be a future goal)
- Modding support (loading mods from .big files and directories)

## Quinary Goals (Future: Windows Modernization)

**Architecture**: Extended Windows support with modern toolchains.
**Testing Focus**: Windows 32-bit VC6 as primary, optional Windows 64-bit via MSVC.

- Optional Windows 64-bit support via MSVC 2022 toolchain (for future Windows modernization)
- Performance optimizations for Windows variants
- Extended audio/video codec support via modern libraries

## Wishlist (for a far future)

- Performance optimizations.
- Parallelize loading and processing where possible.
- Enhance computer AI (Ideas: rewrite AI code, training AI models with machine learning, etc).
- rewrite online algorithms to use modern approach (dedicated servers, peer-to-peer, etc).

# Update Daily Blog before committing any changes

Before committing changes, make sure to update the development diary located at `docs/DEV_BLOG/YYYY-MM-DIARY.md`, where YYYY-MM is the current year and month. You also must follow the development diary guidelines from `.github/instructions/docs.instructions.md` file.

# Guidelines

- The problem cannot be solved by patching over the symptoms; we must address the **root cause** of the problem.
- Use the available reference repositories to identify and implement proper solutions.
- You can also search on GitHub and other online resources for similar problems and solutions.
- **IMPORTANT** All code, including documentation files and comments, **must be in English** regardless of the user's language.
- You may use the `deepwiki` tool that will help you understand the codebase and its architecture.
- **SDL2-only focus for windowing/input**; do not include native Windows API (Win32) windowing or input code.
- **Vulkan-only focus for graphics**; do not include DirectX 8 or platform-specific graphics code (DirectX, Metal, OpenGL).
- **OpenAL-only focus for audio**; do not include DirectSound or platform-specific audio code.
- **IMPORTANT: Use abstraction layers consistently**, so code can eventually run through Wine on non-Windows platforms without code duplication. Avoid Windows-specific API calls at the business logic level.
- Even if errors come from earlier phases, we must fix the gaps that were left behind.
- Prefer to use vscode tasks for building and running the project instead of external scripts.
- Do not use workarounds such as empty stubs, try/catch blocks, or disabling/commenting out components; identify and fix the **root cause** of the problem.
- Do not use emojis or exclamation marks in terminal commands unless absolutely necessary.
- **IMPORTANT:** Always compile with `tee` according to the project instructions.

- It's imperative to use `tee` to log all compilation and debugging output to files for later analysis.
  - use `logs/phaseXX_build.log` for compilation logs - replace `XX` with the current phase number.
  - use `logs/phaseXX_debug.log` for debugging logs - replace `XX` with the current phase number.
  - You can skip log naming convention if, and only if, you need to test different targets/platforms in the same session.

- Total focus on Generals Zero Hour (`GeneralsXZH`) target as the primary stable target, at the end of the development process we can backport changes to the Generals (`GeneralsX`) target.
  - Avoid to direct copy-paste code between targets, always try to implement changes in a way that both targets can benefit from them.
  - There is a plenty of tools that can be used to get the differences between both targets, use them to understand what changed between targets.

- It's a very old codebase, so be patient and meticulous when dealing with legacy constructs.
- Create tags for major milestones or completed phases for easy reference and rollback.
- Any support scripts must be placed in the `scripts/` directory.

# Merge from thesupehackers upstream

- Ensure the project have a read-only git remote called `thesupehackers` poiting to `git@github.com:TheSuperHackers/GeneralsGameCode.git`
- Once a day, before start working, merge the latest changes from `thesupehackers` main branch into local current branch.
 - In case of merge conflicts, resolve them carefully, ensuring no local changes are lost.
 - Avoid using `theirs` strategy unless absolutely necessary.
- Test the build after merging to ensure stability.


# Available Reference Repositories
- **`references/jmarshall-win64-modern/`** - Windows 64-bit modernization with comprehensive fixes - Game base (Generals) Only
  - **Primary use**: Cross-platform compatibility solutions, INI parser fixes, memory management
  - **Key success**: Provided the breakthrough End token parsing solution (Phase 22.7-22.8)
  - **Coverage**: Full Windows 64-bit port with modern toolchain compatibility

- **`references/fighter19-dxvk-port/`** - Linux port with DXVK graphics integration
  - **Primary use**: Graphics layer solutions (DirectX→Vulkan via DXVK), Linux compatibility, could be useful for Windows and MACOS Vulkan backend
  - **Focus areas**: OpenGL/Vulkan rendering, graphics pipeline modernization
  - **Coverage**: Complete Linux port with advanced graphics compatibility

- **`docs/Vulkan/`** - Local Vulkan SDK documentation for reference
  - **Primary use**: Vulkan API reference, best practices, platform-specific notes
  - **Coverage**: Complete Vulkan SDK docs for macOS, Windows, Linux

# Project directory Structure
- `/Generals` - The main game source code.
- `/GeneralsMD` - The expansion pack source code.
- `/Core` - Shared libraries and utilities.
- `/Dependencies` - External libraries and frameworks.

# Build Presets (Platform-Specific)
- `vc6` - **Windows 32-bit (Visual C++ 6 compatibility) - PRIMARY TARGET**
- `vc6-wine` - Wine/Windows compatibility layer (planned for future cross-platform support)
- `windows` - Windows 64-bit via MSVC (planned for future modernization)

# Build Targets
- `GeneralsXZH` - Zero Hour expansion executable (PRIMARY TARGET - recommended)
- `GeneralsX` - Original Generals base game executable (SECONDARY TARGET)
- `ww3d2` - Core 3D graphics library
- `wwlib` - Core Windows compatibility library
- `wwmath` - Core mathematics library

# Compilation Parameters
- `-DRTS_BUILD_ZEROHOUR='ON'` - compiles the Zero Hour expansion
- `-DRTS_BUILD_GENERALS='ON'` - compiles the Generals base game
- `-DRTS_BUILD_CORE_TOOLS='ON'` - compiles the core tools
- `-DRTS_BUILD_GENERALS_TOOLS='ON'` - compiles the Generals tools
- `-DRTS_BUILD_ZEROHOUR_TOOLS='ON'` - compiles the Zero Hour tools
- `-DRTS_BUILD_CORE_EXTRAS='ON'` - compiles the core extras
- `-DRTS_BUILD_GENERALS_EXTRAS='ON'` - compiles the Generals extras
- `-DRTS_BUILD_ZEROHOUR_EXTRAS='ON'` - compiles the Zero Hour extras

# VS Code Tasks

**IMPORTANT**: Always prefer using VS Code Tasks over manual terminal commands for build, deploy, and run operations. Tasks are pre-configured with correct paths and logging.

### Available Tasks (Windows VC6)

| Task | Description | Log Output |
|------|-------------|------------|
| `Build GeneralsXZH (Windows VC6)` | Compiles Zero Hour expansion | `logs/build_generalszh.log` |
| `Build GeneralsX (Windows VC6)` | Compiles Generals base game | `logs/build_generalsv.log` |
| `Deploy GeneralsXZH (Windows)` | Copies ZH binary to `%USERPROFILE%\GeneralsX\GeneralsMD\` | - |
| `Deploy GeneralsX (Windows)` | Copies Generals binary to `%USERPROFILE%\GeneralsX\Generals\` | - |
| `Run GeneralsXZH (Windows)` | **Runs ZH with SDL2 windowing** | `logs/runTerminal_vc6.log` |
| `Run GeneralsX (Windows)` | **Runs Generals with SDL2 windowing** | `logs/runTerminal_generalsv.log` |
| `Debug GeneralsXZH (Windows)` | **Runs ZH under WinDbg debugger** | `logs/debugTerminal_vc6.log` |
| `Debug GeneralsX (Windows)` | **Runs Generals under WinDbg debugger** | `logs/debugTerminal_generalsv.log` |

### Critical: Running the Game

Use the Windows-native VS Code tasks for building, deploying, and running:

- `Run GeneralsXZH (Windows)` - Executes the game with output logged to `logs/runTerminal_vc6.log`
- `Run GeneralsX (Windows)` - Executes Generals with output logged to `logs/runTerminal_generalsv.log`

### Debug Tasks (WinDbg)

For crash investigation, use WinDbg-based debug tasks:

- `Debug GeneralsXZH (Windows)` - Runs ZH under WinDbg debugger
- `Debug GeneralsX (Windows)` - Runs Generals under WinDbg debugger

Output is captured in `logs/debugTerminal*.log` for analysis.

### How to Run Tasks

Use `run_task` tool with the task label:
```
run_task("shell: Build GeneralsXZH (Windows VC6)", workspaceFolder)
run_task("shell: Run GeneralsXZH (Windows)", workspaceFolder)
```

### Typical Workflow

1. **Build**: `run_task("shell: Build GeneralsXZH (Windows VC6)")`
2. **Deploy**: `run_task("shell: Deploy GeneralsXZH (Windows)")`
3. **Run**: `run_task("shell: Run GeneralsXZH (Windows)")`
4. **Analyze**: Read `logs/runTerminal_vc6.log` after game closes

## Debugging Guidelines

- Use VS Code Tasks for debugging instead of manual terminal commands
- **PREFER using VS Code Tasks**:
  - `Debug GeneralsXZH (Windows)` - Runs under WinDbg debugger
  - `Debug GeneralsX (Windows)` - Same for Generals base game
- WinDbg provides comprehensive debugging support for crash analysis
- Always use `tee` to log output to files (e.g., `logs/debugTerminal_vc6.log`) for later analysis
- Command example (manual WinDbg):
```cmd
windbg.exe %USERPROFILE%\GeneralsX\GeneralsMD\GeneralsXZH.exe -win
```

### Debugging Best Practices

1. **Complete Log Capture is Essential**
   - **USE** `tee` to save logs to `logs/` directory for later analysis
   - **WAIT** for game to stabilize before assuming a crash occurred
   - Example: Game may take 30+ seconds to load .big files, parse INI files, and initialize graphics

2. **Grep Filtering for Analysis**
   - Repetitive logs (like BeginFrame/EndFrame cycles) may appear as infinite loops when filtered
   - **CHECK** full unfiltered logs first, then apply targeted grep patterns
   - Example: `findstr /i "error" logs/runTerminal_vc6.log` provides error context

3. **Distinguish Real Crashes from Slow Initialization**
   - **CHECK** process state in Task Manager before assuming crash
   - **VERIFY** CPU usage: High CPU = still processing, Low CPU = might be stuck
   - **LOOK** for crash logs in game data directory
   - **WAIT** for complete initialization before investigating "apparent" hangs

4. **Memory Protection Systems**
   - Trust validation checks in `GameMemory.cpp::isValidMemoryPointer()` to catch memory issues
   - Extensive logging helps identify memory-related problems early

5. **Persistent Log Directory**
   - **USE** `logs/` directory in project root (gitignored) for all debug output
   - **DOCUMENT** log purpose in filename: `crash_analysis_20260115.log`
   - **KEEP** logs for resolved bugs as reference for similar issues


# Vulkan Documentation

During Phase 2 (Vulkan graphics backend), download and extract the Vulkan SDK documentation locally for reference:

```
mkdir -p docs/Support/Vulkan
cd docs/Support/Vulkan
wget https://vulkan.lunarg.com/doc/download/VulkanSDK-Windows-Docs-1.4.335.0.zip
unzip VulkanSDK-Windows-Docs-1.4.335.0.zip
rm -rf VulkanSDK-Windows-Docs-1.4.335.0.zip
```

Refer to [Vulkan Specification](https://www.khronos.org/registry/vulkan/specs/1.3/html/) for API reference during graphics backend implementation.
# Command Line parameters

these are the most common command line parameters for GeneralsX and GeneralsXZH executables:

- `-win` : Forces windowed mode (`m_windowed = true`)
- `-fullscreen` : Forces fullscreen mode (`m_windowed = false`)
- `-noshellmap` : Disables the shell map (`m_shellMapOn = false`)
- `-mod <path>` : Loads a mod from a directory or `.big` file (absolute path or relative to user data path).

for a full list of command line parameters, please refer to `docs/ETC/COMMAND_LINE_PARAMETERS.md` file.
