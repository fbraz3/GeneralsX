---
applyTo: '**'
---

# Objectives

GeneralsX is a cross-platform port of Command & Conquer: Generals (2002) and its expansion Zero Hour aka GeneralsMD (2003) from Windows-only DirectX 8 to macOS/Linux/Windows using Vulkan and SDL2 backends.

This is a **massive C++ game engine** (~500k+ LOC) being ported from Visual C++ 6 (C++98) to modern C++20 with platform abstraction layers.

**Critical Context**: This is NOT a greenfield project. You're working with 20+ year old game code that assumes Windows everywhere. Respect the existing architecture while adding cross-platform support.

## Multi-Platform Support

The project must run natively on Windows, macOS and Linux, using Vulkan as graphics API and SDL2 as main backend library for windowing, input and audio.

Registry keys must be replaced by configuration files, details about file formats and locations can be found into `assets/ini/README.md` file.

When converting to SDL2, try to be more plataform-agnostic as possible, avoid Windows-specific assumptions.

## Primary Goals

**Architecture**: Build with cross-platform abstraction from the root using SDL2, making the codebase platform-agnostic.
**Testing Focus**: Windows only (VC6 build preset) - validate all functionality works on Windows first.

For this primary goal we will not mess with graphics backend, only port native win32 calls into universal SDL2 calls.

After completing this primary goal, the game must be able to (on Windows):

- Load game assets (.big files, models, textures, sounds, etc)
- Load initial screen and main menu
- Be able to navigate menus
- Start and play a skirmish game (single-player vs AI) successfully
- Play Campaign missions successfully
- Play Generals Challenge successfully

## Secondary Goals

**Architecture**: Continue cross-platform abstraction strategy for graphics layer.
**Testing Focus**: Windows only (VC6 build preset) - ensure Vulkan rendering works on Windows.

For this secondary goal we will focus on graphics backend, porting DirectX 8 calls into Vulkan calls using existing abstraction layers.

- Investigate which is the best approach for DirectX 8 to Vulkan translation (dxvk maybe?)
- Implement the chosen approach
- Optimize graphics performance for Vulkan backend
- Ensure all features from primary goals work flawlessly with Vulkan backend

## Tertiary Goals

**Architecture**: Maintain cross-platform abstractions.
**Testing Focus**: Windows only.

- Multiplayer support (LAN only initially, online will be a future goal)
- Modding support (loading mods from .big files and directories)

## Quaternary Goals

**Architecture**: Optimize for Windows platform variants while maintaining cross-platform code.
**Testing Focus**: Windows 32-bit (VC6), Windows 64-bit (MSVC/MinGW), Windows ARM64.

- Windows 64-bit support with modern toolchain (Visual Studio 2022, MinGW-w64)
- Windows ARM64 support (Surface Pro X, etc)
- Performance optimizations for Windows variants

## Quinary Goals

**Architecture**: Activate platform-specific code paths and optimizations for non-Windows targets.
**Testing Focus**: Full compilation and runtime testing on Linux and macOS.

- Achieve similar functionality on Linux (x86_64) natively with Vulkan.
  - Activate platform-agnostic code paths for Linux
  - Test Virtual File System and file path handling (case sensitivity, slashes, etc)
  - Validate Big files handling and memory mapping on Linux
- Achieve similar functionality on macOS (Apple Silicon ARM64) natively with Vulkan.
  - Activate platform-agnostic code paths for macOS
  - Handle macOS specific file system quirks (resource forks, extended attributes, etc)
  - Implement Metal backend via MoltenVK for Vulkan support on macOS

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
- Vulkan-only and SDL2-only focus; do not include DirectX or native Windows API-specific code/instructions.
- **IMPORTANT: Write code to be platform-agnostic and cross-platform ready**, even during Windows-only testing phases. Use platform abstraction layers from the root. Avoid Windows-specific assumptions in implementation.
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
- `vc6` - Windows 32-bit (Visual C++ 6 compatibility) - LEGACY
- `macos` - **macOS Apple Silicon (ARM64) - PRIMARY TARGET**
- `linux` - Linux 64-bit (x86_64) - IN DEVELOPMENT
- `windows` - Windows 64-bit Vulkan - FUTURE

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

**IMPORTANT**: Always prefer using VS Code Tasks over manual terminal commands for build, deploy, sign, and run operations. Tasks are pre-configured with correct paths and logging.

### Available Tasks (macOS)

| Task | Description | Log Output |
|------|-------------|------------|
| `Build GeneralsXZH (macOS)` | Compiles Zero Hour expansion | `logs/build.log` |
| `Build GeneralsX (macOS)` | Compiles Generals base game | `logs/build_generals.log` |
| `Deploy GeneralsXZH (macOS)` | Copies ZH binary to `$HOME/GeneralsX/GeneralsMD/` | - |
| `Deploy GeneralsX (macOS)` | Copies Generals binary to `$HOME/GeneralsX/Generals/` | - |
| `Sign GeneralsXZH (macOS)` | Ad-hoc code signs ZH binary | - |
| `Sign GeneralsX (macOS)` | Ad-hoc code signs Generals binary | - |
| `Run GeneralsXZH Terminal (macOS)` | **Runs ZH in external Terminal.app** | `logs/runTerminal.log` |
| `Run GeneralsX Terminal (macOS)` | **Runs Generals in external Terminal.app** | `logs/runTerminal_generals.log` |
| `Debug GeneralsXZH Terminal (macOS)` | **Runs ZH under LLDB debugger** | `logs/debugTerminal.log` |
| `Debug GeneralsX Terminal (macOS)` | **Runs Generals under LLDB debugger** | `logs/debugTerminal_generals.log` |

### Critical: Running the Game

**NEVER run the game directly in VS Code's integrated terminal** - it will block the terminal indefinitely and may require force-killing the process.

**ALWAYS use the `Run GeneralsXZH Terminal (macOS)` or `Run GeneralsX Terminal (macOS)` tasks** which:
1. Open macOS Terminal.app (external window)
2. Execute the game with output captured via `tee`
3. Auto-close the terminal when the game exits
4. Keep logs in `logs/runTerminal*.log` for analysis

### Debug Tasks (LLDB)

For crash investigation, use **`Debug GeneralsXZH Terminal (macOS)`** or **`Debug GeneralsX Terminal (macOS)`** tasks which:
1. Run the executable under LLDB debugger
2. Automatically capture backtrace (20 frames) on crash
3. Show exact crash location with source file and line number
4. Exit cleanly after debugging session

**Debug workflow:**
```
run_task("shell: Debug GeneralsXZH Terminal (macOS)", workspaceFolder)
# After crash, analyze: logs/debugTerminal.log
```

**Example debug output (crash):**
```
* thread #1, stop reason = EXC_BAD_ACCESS (code=1, address=0x0)
    frame #0: 0x0000000000000000
    frame #1: GeneralsXZH`parseInit at GameWindowManagerScript.cpp:2534
    frame #2: GeneralsXZH`parseLayoutBlock at GameWindowManagerScript.cpp:2656
    ...
```

The debug tasks use `scripts/lldb_debug.sh` wrapper which handles both normal exits and crashes gracefully.

### How to Run Tasks

Use `run_task` tool with the task label:
```
run_task("shell: Build GeneralsXZH (macOS)", workspaceFolder)
run_task("shell: Run GeneralsXZH Terminal (macOS)", workspaceFolder)
```

### Typical Workflow

1. **Build**: `run_task("shell: Build GeneralsXZH (macOS)")`
2. **Deploy**: `run_task("shell: Deploy GeneralsXZH (macOS)")`
3. **Sign**: `run_task("shell: Sign GeneralsXZH (macOS)")`
4. **Run**: `run_task("shell: Run GeneralsXZH Terminal (macOS)")`
5. **Analyze**: Read `logs/runTerminal.log` after game closes

## Debugging Guidelines

- For debugging purposes, do not use commands like `grep` or `tail` into a command run to avoid hanging the terminal if the command produces a lot of output - Instead, use `tee` to log the output to a file and then use `grep` on the log file.
- **PREFER using VS Code Tasks** for debugging instead of manual terminal commands:
  - `Debug GeneralsXZH Terminal (macOS)` - Runs under LLDB with automatic backtrace
  - `Debug GeneralsX Terminal (macOS)` - Same for Generals base game
- Command example (manual):
```bash
# Example for macOS ARM64 - GeneralsXZH
cd $HOME/GeneralsX/GeneralsMD/ && ./GeneralsXZH 2>&1 | tee logs/run_zh.log
grep -i error logs/run_zh.log

# Example for macOS ARM64 - GeneralsX
cd $HOME/GeneralsX/Generals/ && ./GeneralsX 2>&1 | tee logs/run_g.log
grep -i error logs/run_g.log
```
- Use `lldb` for debugging crashes (or use the Debug tasks which wrap lldb automatically).
- The `scripts/lldb_debug.sh` wrapper handles:
  - Automatic backtrace capture on crash
  - Clean exit on normal termination
  - Proper LLDB session cleanup
- Command example (manual lldb):
```bash
# Example for macOS ARM64 - GeneralsXZH
scripts/lldb_debug.sh $HOME/GeneralsX/GeneralsMD/GeneralsXZH 2>&1 | tee logs/lldb_zh.log
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


# Vulkan Docs

We should have the Vulkan documentation available locally for reference. To download and extract the Vulkan SDK documentation for macOS, you can use the following commands:

```
mkdir -p docs/Support/Vulkan
cd docs/Support/Vulkan
wget https://vulkan.lunarg.com/doc/download/VulkanSDK-Mac-Docs-1.4.335.1.zip
unzip VulkanSDK-Mac-Docs-1.4.335.1.zip
rm -rf VulkanSDK-Mac-Docs-1.4.335.1.zip
```

The example above is for macOS, you can adjust the URL and commands accordingly for Windows or Linux versions of the Vulkan SDK documentation.

```
https://vulkan.lunarg.com/doc/download/VulkanSDK-Windows-Docs-1.4.335.0.zip
https://vulkan.lunarg.com/doc/download/VulkanSDK-Linux-Docs-1.4.335.0.zip
wget https://vulkan.lunarg.com/doc/download/VulkanSDK-Mac-Docs-1.4.335.1.zip
```
# Command Line parameters

these are the most common command line parameters for GeneralsX and GeneralsXZH executables:

- `-win` : Forces windowed mode (`m_windowed = true`)
- `-fullscreen` : Forces fullscreen mode (`m_windowed = false`)
- `-noshellmap` : Disables the shell map (`m_shellMapOn = false`)
- `-mod <path>` : Loads a mod from a directory or `.big` file (absolute path or relative to user data path).

for a full list of command line parameters, please refer to `docs/ETC/COMMAND_LINE_PARAMETERS.md` file.
