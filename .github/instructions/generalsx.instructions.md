---
applyTo: '**'
---

# Objectives

GeneralsX is downstream cross-platform port of Command & Conquer: Generals Zero Hour, focused on native Linux/macOS with one SDL3 + DXVK + OpenAL + 64-bit codebase. Windows stays exploratory, not active target.

This is old, large C++ game code, not greenfield. Modernize carefully, keep gameplay intact, and learn from fighter19 and jmarshall instead of inventing new patterns.

## Project Structure & Upstream Context

### EA Games Source Release (2024)
The original Generals/Zero Hour source was released by EA. These refs matter:

### TheSuperHackers (Our Upstream Base)
- **Repository**: `git@github.com:TheSuperHackers/GeneralsGameCode.git`
- **Use**: upstream sync baseline, Windows compatibility, bugfixes.

### fighter19's DXVK Port (Primary Graphics/Platform Reference)
- **Repository**: `references/fighter19-dxvk-port/`
- **Deepwiki Repo**: `Fighter19/CnC_Generals_Zero_Hour`
- **Use**: DXVK, SDL3, Linux platform patterns.

### jmarshall's Port (Audio/Modernization Reference)
- **Repository**: `references/jmarshall-win64-modern/`
- **Deepwiki Repo**: `jmarshall2323/CnC_Generals_Zero_Hour`
- **Use**: OpenAL and modern toolchain patterns.

Use local Deepwiki repo `fbraz3/GeneralsX` for navigation when helpful.

## Our Mission

**Primary Goal**: Make `GeneralsXZH` run natively on Linux/macOS with SDL3, DXVK, OpenAL, and 64-bit.

**Secondary Goal**: Backport same changes to `GeneralsX` when shared and low risk.

**Non-negotiable**:
- Single codebase.
- SDL3 for windowing/input/platform.
- DXVK for DirectX 8 to Vulkan.
- OpenAL for audio.
- 64-bit native.
- Preserve retail compatibility.
- Keep platform code isolated.
- Never break determinism.

**Out of Scope**:
- New gameplay, balance, or modern enhancements.
- Retail multiplayer compatibility.
- 32-bit builds for the new stack.

**Strategy**:
1. Base on TheSuperHackers.
2. Adapt fighter19 for graphics/platform.
3. Adapt jmarshall for audio.
4. Use SDL3 on all platforms.
5. Prefer Zero Hour first, then shared Generals backports.
6. Keep diffs small and platform code isolated.

**Far-Future Plans**:
- SDL3 graphics backend if it makes sense.
- [GeneralsOnline](https://github.com/GeneralsOnlineDevelopmentTeam/GameClient/) compatibility.
- WebGL via Emscripten.

## Unified Technology Stack

| Layer        | Technology   | Replaces                | Notes                                            |
|--------------|-------------|-------------------------|--------------------------------------------------|
| Graphics     | DXVK        | DirectX 8 (d3d8.dll)   | DX8 to Vulkan translation, works on all 3 platforms |
| Windowing    | SDL3        | Win32 API (HWND, etc.)  | Window creation, input, events                   |
| Audio        | OpenAL      | Miles Sound System      | Cross-platform audio, 3D positional sound        |
| Video        | FFmpeg      | Bink Video              | Intro/campaign video playback (spike/TBD)        |
| Platform     | SDL3 + libc | Win32 API               | Timers, threads, filesystem paths                |
| Architecture | x86_64      | x86 (32-bit)            | 64-bit native on all platforms                   |

## Golden Rules

1. **IMPERATIVE** -- No band-aids or workarounds, only real solutions.
2. **Single codebase** -- Linux and macOS must build from the same source. Keep Windows-compatible architectural decisions when practical, but do not treat Windows as an equal active target unless the user explicitly asks for it.
3. Use the `fighter19` reference repo because it successfully made DXVK + SDL3 work on Linux.
4. The `jmarshall` repo has the working OpenAL implementation and 64-bit insights.
5. The focus is cross-platform without breaking any existing platform -- be very careful when removing things.
6. Use DXVK which provides d3d8.h and translates DirectX 8 to Vulkan.
7. SDL3 is the unified platform layer -- no native POSIX, Win32, or Cocoa calls in game code.
8. Some solutions from `fighter19` and `jmarshall` may involve removing components; in that case, stop and ask the user what should be done, as it may be something the game depends on and cannot be removed (e.g., audio, video playback).
9. Any lesson learned should be documented in `docs/WORKDIR/lessons` for future reference and to avoid repeating mistakes. You should also consult that directory for insights from previous sessions.

## Development Environment & Build System

### Build Presets

#### Legacy Compatibility Presets (Maintenance Only)
- **`vc6`** -- Visual Studio 6 (C++98), 32-bit, DirectX 8 + Miles
- **`win32`** -- MSVC 2022 (C++20), experimental upstream path

These presets remain useful for regression checks and upstream sync, but they are no longer the primary release path.

#### Cross-Platform Builds (SDL3 + DXVK + OpenAL)
- **`linux64-deploy`** -- Linux x86_64 (GCC/Clang) -- **PRIMARY LINUX TARGET**
  - Native ELF binaries
  - DXVK + SDL3 + OpenAL
- **`linux64-testing`** -- Linux debug variant
- **`macos-vulkan`** -- macOS ARM64 (Apple Silicon) -- **ACTIVE**
  - Native Mach-O binaries
  - DXVK + MoltenVK + SDL3 + OpenAL
  - macOS 15.0+ minimum; Universal binary (arm64 + x86_64) planned
- **`mingw-w64-i686`** -- exploratory MinGW-w64 cross-compile for Windows
- **`windows64-deploy`** -- planned MinGW-w64 x86_64 preset (issue #29, not an active release target)

### Build Workflow

**Docker-based builds (Linux host or CI)**:
```bash
# Linux native build
./scripts/build/linux/docker-configure-linux.sh linux64-deploy
./scripts/build/linux/docker-build-linux-zh.sh linux64-deploy

# Optional exploratory MinGW cross-compile (Windows .exe)
./scripts/build/linux/docker-build-mingw-zh.sh mingw-w64-i686
```

**Native builds (on respective platforms)**:
```bash
# Linux
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals

# macOS
cmake --preset macos-vulkan
cmake --build build/macos-vulkan --target z_generals

# Optional Windows via MinGW cross-build
cmake --preset mingw-w64-i686
cmake --build build/mingw-w64-i686 --target z_generals
```

### Testing Strategy

1. **Per-platform smoke tests**: Launch game, reach main menu, load skirmish map
2. **Replay compatibility**: VC6 optimized builds with `RTS_BUILD_OPTION_DEBUG=OFF` (determinism)
3. **Cross-platform validation**: Same replays must remain valid across platforms (rendering/audio changes must not affect game logic)

## CRITICAL: DXVK Source of Truth (macOS)

**Current Model**:
- DXVK fixes must live in the fork repository source (`references/fbraz3-dxvk`) and be pushed upstream to the fork branch.
- macOS build defaults to a GitHub clone source (`build/_deps/dxvk-src-fbraz3`) and tracks branch `generalsx-macos-v2.6`.
- CMake remote mode keeps update step enabled (`UPDATE_DISCONNECTED FALSE`) so configure/build can pull the latest fork branch state.

**Local Development Mode**:
- Use local fork source only when explicitly requested:
  - `-DSAGE_DXVK_USE_LOCAL_FORK=ON`
- In this mode, source is `references/fbraz3-dxvk` and update/fetch are disabled by design.

**Rules**:
1. Do not patch DXVK files directly inside `build/_deps/...`.
2. Do not rely on transient patch scripts as the primary maintenance path.
3. Keep Linux/macOS aligned on DXVK 2.6 branch policy unless explicitly changed by project decision.
4. If a DXVK fix is validated locally, commit/push it in the DXVK fork first, then consume it from the tracked branch.

## Guidelines

### Code Quality & Maintainability
- **Scope discipline**: Focus on cross-platform port (SDL3, DXVK, OpenAL). Avoid unrelated refactors.
- **Root cause**: Fix underlying issues, not symptoms. No lazy workarounds.
- **Isolation**: Platform-specific code stays in platform layers (`Core/GameEngineDevice/`, `Core/Libraries/Source/Platform/`)
- **Fallback paths**: Keep legacy Windows paths (DX8, Miles) intact behind `#ifdef` guards for VC6 baseline
- **Determinism**: Never break gameplay determinism. Rendering/audio changes must not affect logic.

### Code Style & Conventions
- **English only**: All code, comments, identifiers, documentation in English
- **No lazy solutions**: No empty stubs, empty `catch` blocks, or commented-out code
- **Terminal hygiene**: No emojis or exclamation marks in terminal commands
- **C++ heritage**: Maintain consistency with surrounding legacy code patterns
- **Change annotation**: Every user-facing code change needs `// GeneralsX @keyword author DD/MM/YYYY Description` above it. Keywords: `@bugfix` / `@feature` / `@performance` / `@refactor` / `@tweak` / `@build`
- **Upstream PR attribution**: When implementing work derived from a specific upstream PR, add an adjacent comment with original author and PR link (for example: `// Upstream reference: <author>, PR #<id>` and the full GitHub URL).

### Platform Isolation Patterns

**Good** -- Platform-specific code isolated in device/platform layers:
```cpp
// Core/GameEngineDevice/Include/w3dgraphicsdevice.h
#ifdef BUILD_WITH_DXVK
    #include "dxvk_adapter.h"
#else
    #include <d3d8.h>
#endif
```

**Bad** -- Platform code leaking into game logic:
```cpp
// GeneralsMD/Code/GameEngine/GameLogic/object.cpp -- WRONG
#ifdef __linux__
    // Linux-specific hack in gameplay code
#endif
```

### Build & Run Workflow

**Prefer VS Code Tasks** for build/deploy/run/debug:
- Tasks automatically set up build environment
- Capture logs to `logs/` directory

**Logging Conventions**:
- Build logs: `logs/build_<preset>.log`
- Run logs: `logs/run_<preset>.log`
- Debug logs: `logs/debug_<preset>.log`

## Target Priority

1. **Primary**: `GeneralsXZH` (Zero Hour expansion) -- Most feature-complete
2. **Secondary**: `GeneralsX` (Base game) -- Backport when changes are clearly shared

**When to backport Generals <- Zero Hour**:
- Change is platform/backend code (SDL3, DXVK, OpenAL)
- Change is in shared Core libraries
- Change is low-risk and clearly applicable

**When NOT to backport**:
- Change is Zero Hour-specific gameplay/logic
- Change touches expansion-specific features
- Risk of breaking Generals outweighs benefit

## Merge From TheSuperHackers Upstream

**Sync workflow**:
```bash
git remote add thesuperhackers git@github.com:TheSuperHackers/GeneralsGameCode.git
git fetch thesuperhackers
git merge thesuperhackers/main
```

**Conflict resolution strategy**:
- **Platform code** (`Core/GameEngineDevice/`, SDL, DXVK, OpenAL): Keep ours
- **Game logic** (`GeneralsMD/Code/GameEngine/`, `Core/GameEngine/`): Keep theirs
- **Build system** (CMake, presets): Merge carefully (we have extra cross-platform presets)
- **When in doubt**: Create a backup branch and test both versions

## Available Reference Repositories

**CRITICAL**: Use references to understand patterns and architecture, **NOT** for copy-paste. Each reference has different scope and must be adapted for Zero Hour.

### 1. fighter19's DXVK Port (`references/fighter19-dxvk-port/`)
**Use for**: Graphics layer (DXVK), SDL3 integration, MinGW build setup, POSIX compat
**Coverage**: Generals + Zero Hour
**Status**: Fully functional Linux build

Key files to study:
- `GeneralsMD/Code/GameEngineDevice/` -- DXVK device management
- `Core/Libraries/Source/Platform/` -- POSIX abstractions
- `CMakePresets.json` -- MinGW build configuration
- `cmake/mingw.cmake` -- Cross-compilation setup

### 2. jmarshall's Modern Port (`references/jmarshall-win64-modern/`)
**Use for**: OpenAL audio implementation, 64-bit porting insights
**Coverage**: Generals base game ONLY (**NOT Zero Hour**)
**Status**: Fully functional modern Windows build with working audio

Key files to study:
- `Code/Audio/` -- OpenAL implementation (adapt for Zero Hour)
- `Code/Audio/MusicManager.cpp` -- Audio format handling

**Caution**: Generals-only; must adapt for Zero Hour expansion features.

### 3. TheSuperHackers Main (`references/thesuperhackers-main/`)
**Use for**: Upstream baseline, Windows behavior reference
**Coverage**: Generals + Zero Hour (upstream source)

### Reference Consultation Workflow

**Before implementing a feature**:
1. Read relevant active design/support documentation in `docs/WORKDIR/`
2. Study fighter19 implementation (if graphics/platform)
3. Study jmarshall implementation (if audio/modernization)
4. Document differences and adaptation strategy
5. Implement with platform isolation in mind
6. Validate builds on all affected platforms

## Project Directory Structure

```
/Generals              - Base game (C&C Generals) source code
/GeneralsMD            - Zero Hour expansion source code (PRIMARY TARGET)
/Core                  - Shared libraries/utilities
  /GameEngine          - Core game engine (logic, rendering, audio)
  /GameEngineDevice    - Platform-specific rendering device (DX8/DXVK)
  /Libraries           - Utility libraries (WWVegas, math, containers)
  /Tools               - Development tools (W3DView, MapCacheBuilder)
/Dependencies          - External dependencies (MaxSDK, utilities)
/references            - Reference implementations (fighter19, jmarshall)
  /fighter19-dxvk-port/   - DXVK Linux port (PRIMARY GRAPHICS REFERENCE)
  /jmarshall-win64-modern/ - OpenAL port (PRIMARY AUDIO REFERENCE)
  /thesuperhackers-main/   - Upstream snapshot (MERGE REFERENCE)
/docs                  - Project documentation
  /DEV_BLOG            - Development diary (REQUIRED before commits)
  /WORKDIR             - Active work documentation
    /planning          - Planning & strategic documents
    /reports           - Session reports
    /support           - Technical discoveries
    /audit             - Audit records
    /lessons           - Lessons learned
  /ETC                 - Reference & historical materials
/scripts               - Build/utility scripts
/logs                  - Build/run/debug logs (gitignored)
```

## Command Line Parameters

Common parameters for testing `GeneralsX` and `GeneralsXZH`:

```bash
# Windowed mode (easier for debugging)
./GeneralsXZH -win

# Fullscreen mode
./GeneralsXZH -fullscreen

# Skip intro (disable shell map)
./GeneralsXZH -noshellmap

# Quick launch for testing
./GeneralsXZH -win -noshellmap

# Load a mod (testing mod compatibility)
./GeneralsXZH -mod /path/to/mod.big

# Route legacy debug logging to console path (diagnostics; debug builds only)
./GeneralsXZH -logToCon
```

Important Linux note:
- `-logToCon` enables `DEBUG_LOG` console routing, but critical runtime traces may still require direct `fprintf(stderr, ...)` probes because `OutputDebugString`-based paths are not reliably visible on Linux.
- **`-logToCon` is only available in debug builds** (`RTS_BUILD_OPTION_DEBUG=ON`); it is unrecognized and has no effect in release builds.
- For diagnostics, prefer capturing stderr to a log file and grep targeted markers.

Recommended debugging command:
```bash
cd ~/GeneralsX/GeneralsMD
./run.sh -win -logToCon 2>&1 | grep -v "D3DRS_PATCHSEGMENTS" | tee ~/Projects/GeneralsX/logs/manual_run.log
```

For a full list, see `docs/ETC/COMMAND_LINE_PARAMETERS.md`.

## Update Dev Blog Before Commits

Before committing changes, update the development diary at `docs/DEV_BLOG/YYYY-MM-DIARY.md` (YYYY-MM is the current year and month). Follow `.github/instructions/docs.instructions.md`.

```bash
mkdir -p docs/DEV_BLOG
touch docs/DEV_BLOG/$(date +%Y-%m)-DIARY.md
```

## GitHub CLI PR/Issue Body Formatting

When creating PRs or issues with `gh`, avoid literal `\n` sequences in the body.

- Prefer `--body-file` with a real Markdown file (temporary file is fine).
- If using `--body`, pass a multi-line quoted string with actual line breaks.
- After creation, verify body formatting:

```bash
body=$(gh pr view <number> --json body --jq .body)
printf "%s" "$body" | rg '\\n' && echo "HAS_LITERAL_BACKSLASH_N=YES" || echo "HAS_LITERAL_BACKSLASH_N=NO"
```

Issue validation uses the same pattern:

```bash
body=$(gh issue view <number> --json body --jq .body)
printf "%s" "$body" | rg '\\n' && echo "HAS_LITERAL_BACKSLASH_N=YES" || echo "HAS_LITERAL_BACKSLASH_N=NO"
```

## VS Code Tasks

Tasks are configured for multi-platform development. Prefer task-first execution before manual commands.

Primary task labels:
- Linux: `[Linux] Configure (Docker)`, `[Linux] Build GeneralsXZH`, `[Linux] Deploy GeneralsXZH`, `[Linux] Run GeneralsXZH`
- Linux pipeline: `[Linux] Pipeline: Build + Deploy + Run ZH`
- Linux smoke test: `[Linux] Smoke Test GeneralsXZH`
- macOS: `[macOS] Configure`, `[macOS] Build GeneralsXZH`, `[macOS] Deploy GeneralsXZH`, `[macOS] Run GeneralsXZH`

Task commands should map to organized script paths under `scripts/build/...`, `scripts/env/...`, and `scripts/qa/...`.

---

# Platform-Specific Sections

## Linux

**Branch**: `main` (current primary focus)
**Status**: Active development -- DXVK graphics working, OpenAL audio in progress

### Build Environment

**Native (on Linux)**:
```bash
sudo apt install build-essential cmake ninja-build git
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals
```

**Docker (from any host)**:
```bash
./scripts/build/linux/docker-configure-linux.sh linux64-deploy
./scripts/build/linux/docker-build-linux-zh.sh linux64-deploy
```

### Build Presets
- **`linux64-deploy`** -- GCC/Clang x86_64, Release -- **PRIMARY**
  - Native ELF binaries
  - DXVK + SDL3 + OpenAL
- **`linux64-testing`** -- Debug variant for development

### Run & Test
```bash
# Deploy game data + binary
./scripts/build/linux/deploy-linux-zh.sh

# Run
./scripts/build/linux/run-linux-zh.sh -win

# Smoke test via Docker
./scripts/qa/smoke/docker-smoke-test-zh.sh linux64-deploy

# Debug with GDB backtrace
mkdir -p logs && gdb -batch -ex "run -win" -ex "bt full" -ex "thread apply all bt" \
  ./build/linux64-deploy/GeneralsMD/GeneralsXZH 2>&1 | tee logs/gdb.log
```

### Linux-Specific Notes

- **Case-sensitive filesystem**: Include paths must match exact case. Use `scripts/tooling/cpp/maintenance/fixIncludesCase.sh` to fix.
- **DXVK requires Vulkan**: Ensure Vulkan drivers are installed (`vulkan-tools`, `mesa-vulkan-drivers` or proprietary GPU drivers).
- **SDL3 from source**: SDL3 is fetched via CMake FetchContent. No system package needed.
- **DXVK source policy**: Keep DXVK fixes in the fork source of truth and avoid editing `build/_deps/...` directly.
- **CompatLib**: Linux builds use `GeneralsMD/Code/CompatLib/` for Win32 API compatibility shims (windows_compat.h, etc.).
- **No native POSIX calls**: Use SDL3 abstractions for timers, threads, file I/O. Do not add raw `pthread_*`, `open()`, etc. into game code.

---

## macOS

**Branch**: `main` (active development alongside Linux)
**Status**: Active release branch on ARM64 Apple Silicon

### Architecture
- SDL3 for windowing/input (same as Linux)
- DXVK + MoltenVK for DirectX 8 to Vulkan to Metal translation (DX8 → Vulkan → Metal chain)
- OpenAL for audio (same as Linux)
- Current target: **ARM64 (Apple Silicon)** -- macOS 15.0+
- Universal binary (arm64 + x86_64) planned for future

### Key Considerations
- MoltenVK translates Vulkan to Metal; DXVK sits on top of it (DX8 to Vulkan to Metal chain)
- DXVK is built via Meson as an ExternalProject -- must pass `-arch arm64` explicitly via `cmake/meson-arm64-native.ini` to avoid Rosetta2 confusing the host arch
- DXVK source of truth is the fork branch `generalsx-macos-v2.6`; by default CMake tracks the remote branch, with optional local-fork mode via `SAGE_DXVK_USE_LOCAL_FORK`
- Vulkan SDK **must** be installed from LunarG (not Homebrew) -- it provides the MoltenVK ICD JSON required to route Vulkan calls to Metal
- Code signing and notarization requirements for distribution (future)
- macOS-specific SDL3 quirks (Retina scaling, fullscreen spaces, etc.)

### Build Presets
- **`macos-vulkan`** -- macOS ARM64, RelWithDebInfo -- **PRIMARY MACOS TARGET**
  - Native Mach-O binaries (`GeneralsXZH`)
  - DXVK + MoltenVK + SDL3 + OpenAL + FFmpeg (video TBD)

### Build Workflow
```bash
# Prerequisites (once): brew install cmake ninja meson
# + LunarG Vulkan SDK: https://vulkan.lunarg.com/sdk/home#mac
./scripts/build/macos/build-macos-zh.sh          # configure + build
./scripts/build/macos/deploy-macos-zh.sh         # copy to ~/GeneralsX/GeneralsMD/
./scripts/build/macos/run-macos-zh.sh -win       # launch windowed
```

### macOS-Specific Notes
- **Rosetta2 + Meson adversarial**: Use `cmake/meson-arm64-native.ini` to force `-arch arm64` in DXVK sub-build
- **DXVK update policy**: Default is remote tracked branch (`generalsx-macos-v2.6`); set `SAGE_DXVK_USE_LOCAL_FORK=ON` only for active local DXVK development
- **SDL3 from source**: Fetched via CMake FetchContent -- no system package needed
- **Vulkan SDK path**: `~/VulkanSDK/<version>/macOS/` -- must contain `libvulkan.dylib` and `libMoltenVK.dylib`
- **No Cocoa/Metal calls in game code**: All platform access goes through SDL3 + DXVK layers

---

## Windows (Future / Exploratory)

**Branch**: TBD
**Status**: Not an active repository focus

**Note**: Follow issue #29 direction for Windows modernization only when the work is explicitly requested. MinGW-w64 is the likely future toolchain path for SDL3 + DXVK + OpenAL + FFmpeg, but Windows is currently not an active delivery target for this repository. Legacy `vc6` and `win32` remain useful for compatibility checks and upstream alignment.

### Possible Architecture Direction
- SDL3 for windowing/input
- DXVK for DirectX 8 to Vulkan translation
- OpenAL for audio
- FFmpeg for video playback
- MinGW-w64 toolchain as default Windows path

### Build Presets
- **`mingw-w64-i686`** -- exploratory Windows cross-build preset
- **`mingw-w64-i686-debug`** -- debug variant
- **`mingw-w64-i686-profile`** -- profiling variant
- **`windows64-deploy`** -- planned MinGW-w64 x86_64 preset (issue #29)

### Build Workflow
```bash
# Cross-build Windows binary from Linux/macOS host using Docker
./scripts/build/linux/docker-build-mingw-zh.sh mingw-w64-i686

# Direct preset build (when MinGW toolchain is locally available)
cmake --preset mingw-w64-i686
cmake --build build/mingw-w64-i686 --target z_generals
```

### Longer-Term Windows Work
- [ ] Add and validate `windows64-deploy` (MinGW x86_64)
- [ ] Validate DXVK + Vulkan runtime requirements on Windows
- [ ] Validate OpenAL integration and runtime audio
- [ ] Validate FFmpeg intro/briefing videos with audio
- [ ] Confirm menu and skirmish playability with determinism checks
- [ ] Publish/update Windows build documentation under `docs/BUILD/`
