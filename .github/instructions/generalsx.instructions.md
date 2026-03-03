---
applyTo: '**'
---

# Objectives

GeneralsX is a **community-driven cross-platform port** of Command & Conquer: Generals Zero Hour (2003), enabling the classic game to run natively on **Linux, macOS, and Windows** under a single modern codebase using SDL3 + DXVK + OpenAL + 64-bit.

This is a **massive C++ game engine** (~500k+ LOC) being carefully modernized from its original Windows-only DirectX 8 / Miles Sound System architecture to a portable stack while preserving the original gameplay experience.

**Critical Context**: This is NOT a greenfield project. You're working with 20+ year old game code. Our strategy is to learn from proven port efforts (fighter19, jmarshall) and converge all three platforms onto one unified build pipeline.

## Project Structure & Upstream Context

### EA Games Source Release (2024)
The original Command & Conquer Generals/Zero Hour source code was released by EA Games. Three major community projects emerged:

### TheSuperHackers (Our Upstream Base)
- **Repository**: `git@github.com:TheSuperHackers/GeneralsGameCode.git`
- **Strategy**: Maintain Windows compatibility with original executables
- **Build System**: VC6 + DirectX 8 + Miles Sound System
- **Coverage**: Both Generals AND Zero Hour
- **Focus**: Bugfixes, stability, code unification
- **VC6 Preset**: **STABLE** (production-ready, retail compatible)
- **Win32 Preset (MSVC 2022)**: Experimental (future modernization path)
- **Status**: Our stable baseline for upstream sync

### fighter19's DXVK Port (Primary Graphics/Platform Reference)
- **Repository**: `references/fighter19-dxvk-port/` (local copy)
- **Deepwiki Repo**: `Fighter19/CnC_Generals_Zero_Hour`
- **Strategy**: Linux port with DirectX to Vulkan via DXVK
- **Build System**: CMake + MinGW-w64 + DXVK + SDL3
- **Coverage**: Both Generals AND Zero Hour
- **Focus**: Graphics layer (DXVK), POSIX compatibility, SDL windowing
- **Audio**: Stubbed (Miles Sound System removed)
- **Status**: Fully functional Linux build with working graphics

### jmarshall's Port (Audio/Modernization Reference)
- **Repository**: `references/jmarshall-win64-modern/` (local copy)
- **Deepwiki Repo**: `jmarshall2323/CnC_Generals_Zero_Hour`
- **Strategy**: Aggressive modernization (VS2022 + Win64 + DX9/DX12)
- **Build System**: CMake + C++20 + d3d9on12 wrapper
- **Coverage**: Generals base game ONLY (Zero Hour not ported)
- **Focus**: Modern toolchain, OpenAL audio implementation
- **Status**: Fully functional modern Windows build with working audio

## Our Mission

**Primary Goal**: Make *Generals: Zero Hour* (`GeneralsXZH`) run natively on **Linux, macOS, and Windows** using a single modern codebase: SDL3 (windowing/input), DXVK (DirectX 8 to Vulkan graphics), OpenAL (audio), and 64-bit architecture.

**Secondary Goal**: Implement the same changes for *Generals* base game (`GeneralsX`) when changes are clearly shared and low-risk.

**Non-negotiable**:
- **Single codebase**: All three platforms build from the same source tree
- **SDL3 everywhere**: Windowing, input, and platform abstraction via SDL3 on all platforms -- no native Win32/Cocoa/X11 calls in game code
- **DXVK everywhere**: DirectX 8 to Vulkan translation via DXVK on all platforms (Vulkan is available on Linux, macOS via MoltenVK, and Windows)
- **OpenAL everywhere**: Audio via OpenAL on all platforms (replaces proprietary Miles Sound System)
- **64-bit native**: All platforms target x86_64 (32-bit legacy support via VC6 upstream only)
- **MUST preserve retail compatibility**: Original game files, replays, and mods must work
- **MUST isolate platform code**: Platform-specific changes confined to platform/backend layers
- **Determinism**: Never break gameplay determinism. Rendering/audio changes must not affect game logic.

**Out of Scope**:
- New gameplay features, balance changes, or "modern enhancements"
- Multiplayer/network compatibility with retail builds (future work)
- 32-bit builds for the new stack (VC6 preset handles legacy 32-bit Windows)

**Strategy**:
1. **Base**: TheSuperHackers code (stable upstream baseline)
2. **Graphics**: fighter19's DXVK integration patterns (adapted for cross-platform)
3. **Audio**: jmarshall's OpenAL implementation (adapted for Zero Hour)
4. **Windowing/Input**: SDL3 on all platforms
5. **Focus**: Zero Hour first; backport to Generals only when clearly applicable
6. **Method**: Research-first, minimal diffs, strict platform/backend isolation
7. **Branching**: Separate branches per platform, merged to `main` as they stabilize

**Far-Future Plans (long-term)**:

- SDL3 graphics backend (bypassing DXVK) if performance is good enough
- [GeneralsOnline](https://github.com/GeneralsOnlineDevelopmentTeam/GameClient/) project compatibility (multiplayer, mod support)
- Webgl version via Emscripten

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
2. **Single codebase** -- All three platforms (Linux, macOS, Windows) must build from the same source. No platform-forked copies.
3. Use the `fighter19` reference repo because it successfully made DXVK + SDL3 work on Linux.
4. The `jmarshall` repo has the working OpenAL implementation and 64-bit insights.
5. The focus is cross-platform without breaking any existing platform -- be very careful when removing things.
6. Use DXVK which provides d3d8.h and translates DirectX 8 to Vulkan.
7. SDL3 is the unified platform layer -- no native POSIX, Win32, or Cocoa calls in game code.
8. Some solutions from `fighter19` and `jmarshall` may involve removing components; in that case, stop and ask the user what should be done, as it may be something the game depends on and cannot be removed (e.g., audio, video playback).
9. Any lesson learned should be documented in `docs/WORKDIR/lessons` for future reference and to avoid repeating mistakes. You should also consult that directory for insights from previous sessions.

## Modernization Phases

We follow a **multi-phase research-first approach** with clear acceptance criteria. Phases apply cross-platform but each platform branch may progress independently.

### Phase 0: Deep Analysis & Planning
**Goal**: Thoroughly analyze reference repositories and document cross-platform architecture before implementation.

**Deliverables**:
- fighter19's DXVK integration architecture documented
- jmarshall's OpenAL audio system documented
- Platform abstraction layer design (SDL3 as common layer)
- Build system strategy (CMake presets per platform)
- Testing strategy per platform

**Acceptance Criteria**:
- [ ] Current renderer architecture documented (DX8 interfaces, entry points)
- [ ] fighter19 DXVK patterns documented with "what applies to Zero Hour" notes
- [ ] jmarshall OpenAL patterns documented with "how to adapt for Zero Hour" notes
- [ ] Platform abstraction layer design approved (minimal changes to game logic)
- [ ] Build workflow configured and tested per platform
- [ ] Phase 1 implementation plan written with step-by-step checklist

### Phase 1: Graphics -- DXVK + SDL3 Integration
**Goal**: Render the game via DXVK (DirectX 8 to Vulkan) with SDL3 windowing on all target platforms.

**Scope**:
- Implement DXVK DirectX 8 to Vulkan wrapper integration
- Port SDL3 windowing/input layer (replace Win32 window management)
- Add CMake build presets for each platform (Linux, macOS, Windows)
- Isolate graphics changes to `Core/GameEngineDevice/` and platform headers
- Keep original DX8 path intact behind compile flags (for VC6 baseline)

**Non-scope**: Audio (silent/stubbed), Video playback (deferred)

**Acceptance Criteria**:
- [ ] Game builds on Linux, macOS, and Windows with respective presets
- [ ] Native binary launches and reaches main menu with DXVK-rendered graphics
- [ ] Basic gameplay (skirmish map) runs without crashes
- [ ] Original VC6 Windows build still compiles and runs correctly
- [ ] Platform-specific code properly isolated (no platform code in game logic)

### Phase 2: Audio -- OpenAL Integration
**Goal**: Replace Miles Sound System with OpenAL on all platforms.

**Scope**:
- Implement OpenAL audio backend (adapt from jmarshall, Zero Hour specifics)
- Create Miles to OpenAL API compatibility layer
- Isolate audio changes to `Core/GameEngine/Audio/`
- Keep Miles path intact behind compile flags for legacy VC6 builds

**Acceptance Criteria**:
- [ ] Working audio on all three platforms (sound effects, music, voices)
- [ ] Audio quality comparable to original
- [ ] Legacy VC6 build still uses Miles Sound System (no regressions)
- [ ] Audio backend selection via compile flag (`USE_OPENAL` vs `USE_MILES`)

### Phase 3: Video Playback -- Bink Alternative (Spike)
**Goal**: Investigate and prototype Bink video replacement for intro/campaign videos.

**Scope**: Research FFmpeg or alternative codecs, prototype, recommend path forward.

**This is a SPIKE**: May result in "defer to later" decision if too complex.

**Acceptance Criteria**:
- [ ] Video playback options researched and documented
- [ ] At least one prototype implemented and tested
- [ ] Recommendation written: "implement X" or "defer indefinitely"
- [ ] If deferred: game fails gracefully without videos (skip to menu)

### Phase 4: Polish & Hardening
**Goal**: Address platform-specific bugs, performance, and compatibility.

**Scope**: TBD based on Phase 1-3 findings.

## Development Environment & Build System

### Build Presets

#### Legacy Windows (Upstream Compatibility)
- **`vc6`** -- Visual Studio 6 (C++98) -- **UPSTREAM BASELINE** (retail compatible)
  - Must pass replay tests
  - Required validation target for any gameplay/logic changes
  - 32-bit, DirectX 8, Miles Sound System
- **`win32`** -- MSVC 2022 (C++20) -- Experimental upstream modernization
  - Secondary Windows target from upstream

#### Cross-Platform Builds (SDL3 + DXVK + OpenAL)
- **`linux64-deploy`** -- Linux x86_64 (GCC/Clang) -- **PRIMARY LINUX TARGET**
  - Native ELF binaries
  - DXVK + SDL3 + OpenAL
- **`linux64-testing`** -- Linux debug variant
- **`macos-vulkan`** -- macOS ARM64 (Apple Silicon) -- **ACTIVE**
  - Native Mach-O binaries
  - DXVK + MoltenVK + SDL3 + OpenAL
  - macOS 15.0+ minimum; Universal binary (arm64 + x86_64) planned
- **`windows64-deploy`** -- Windows x86_64 (MSVC or MinGW) -- **TBD**
- **`mingw-w64-i686`** -- MinGW cross-compile (32-bit Windows .exe from Linux/macOS)

### Build Workflow

**Docker-based builds (Linux host or CI)**:
```bash
# Linux native build
./scripts/docker-configure-linux.sh linux64-deploy
./scripts/docker-build-linux-zh.sh linux64-deploy

# MinGW cross-compile (Windows .exe)
./scripts/docker-build-mingw-zh.sh mingw-w64-i686
```

**Native builds (on respective platforms)**:
```bash
# Linux
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals

# Windows (VC6 upstream baseline)
cmake --preset vc6
cmake --build build/vc6 --target z_generals
```

### Testing Strategy

1. **Per-platform smoke tests**: Launch game, reach main menu, load skirmish map
2. **Replay compatibility**: VC6 optimized builds with `RTS_BUILD_OPTION_DEBUG=OFF` (determinism)
3. **Cross-platform validation**: Same replays must remain valid across platforms (rendering/audio changes must not affect game logic)

## CRITICAL: DXVK Ephemeral Patches

**The Problem**: DXVK headers are fetched via CMake `FetchContent` into `build/_deps/dxvk-src/`. Clean builds or reconfigures refetch pristine DXVK from git, **losing all manual patches**.

**Impact**: Type conflicts return (MEMORYSTATUS, IUnknown redefinitions) after:
- `rm -rf build/` (clean build)
- CMake reconfigure
- CMake cache invalidation

**The Solution -- Pre-Guard Pattern (PREFERRED)**:

Add type guards **BEFORE** including DXVK headers in `GeneralsMD/Code/CompatLib/Include/windows_compat.h`:

```cpp
// Pre-define guards to prevent DXVK from defining incomplete types
#ifndef _WIN32
    #define _MEMORYSTATUS_DEFINED 1
    #define _IUNKNOWN_DEFINED 1
#endif

// Now DXVK will skip these definitions
#include <windows_base.h>
```

**When To Reapply**: Only if you see these errors after reconfigure:
- `redefinition of 'struct MEMORYSTATUS'`
- `redefinition of 'struct IUnknown'`
- `conflicting declaration 'typedef ... LARGE_INTEGER'`

**Manual Patch (If Pre-Guards Fail)**:

Edit these files in `build/_deps/dxvk-src/include/dxvk/`:

1. **windows_base.h** -- Add guard around MEMORYSTATUS:
```cpp
#ifndef _MEMORYSTATUS_DEFINED
typedef struct MEMORYSTATUS { /* ... */ } MEMORYSTATUS;
#define _MEMORYSTATUS_DEFINED 1
#endif
```

2. **unknwn.h** -- Add guard around IUnknown:
```cpp
#ifndef _IUNKNOWN_DEFINED
struct IUnknown { /* ... */ };
#define _IUNKNOWN_DEFINED 1
#endif
```

**Future TODO**: Automate patching via CMake `CMAKE_PATCH_COMMAND` or `file(APPEND)` in `CMakeLists.txt`.

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
- Build logs: `logs/phase<N>_build_<preset>.log`
- Run logs: `logs/phase<N>_run_<preset>.log`
- Debug logs: `logs/phase<N>_debug_<preset>.log`

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
1. Read relevant Phase documentation
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
    /phases            - Phase-specific plans and checklists
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
```

For a full list, see `docs/ETC/COMMAND_LINE_PARAMETERS.md` (create if missing).

## Update Dev Blog Before Commits

Before committing changes, update the development diary at `docs/DEV_BLOG/YYYY-MM-DIARY.md` (YYYY-MM is the current year and month). Follow `.github/instructions/docs.instructions.md`.

```bash
mkdir -p docs/DEV_BLOG
touch docs/DEV_BLOG/$(date +%Y-%m)-DIARY.md
```

## VS Code Tasks

Tasks configured for multi-platform development. See `.vscode/tasks.json` for complete task definitions.

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
./scripts/docker-configure-linux.sh linux64-deploy
./scripts/docker-build-linux-zh.sh linux64-deploy
```

### Build Presets
- **`linux64-deploy`** -- GCC/Clang x86_64, Release -- **PRIMARY**
  - Native ELF binaries
  - DXVK + SDL3 + OpenAL
- **`linux64-testing`** -- Debug variant for development

### Run & Test
```bash
# Deploy game data + binary
./scripts/deploy-linux-zh.sh

# Run
./scripts/run-linux-zh.sh -win

# Smoke test via Docker
./scripts/docker-smoke-test-zh.sh linux64-deploy

# Debug with GDB backtrace
mkdir -p logs && gdb -batch -ex "run -win" -ex "bt full" -ex "thread apply all bt" \
  ./build/linux64-deploy/GeneralsMD/GeneralsXZH 2>&1 | tee logs/gdb.log
```

### Linux-Specific Notes

- **Case-sensitive filesystem**: Include paths must match exact case. Use `scripts/cpp/fixIncludesCase.sh` to fix.
- **DXVK requires Vulkan**: Ensure Vulkan drivers are installed (`vulkan-tools`, `mesa-vulkan-drivers` or proprietary GPU drivers).
- **SDL3 from source**: SDL3 is fetched via CMake FetchContent. No system package needed.
- **DXVK ephemeral patches**: See "CRITICAL: DXVK Ephemeral Patches" section above -- type guard pattern prevents redefinition errors after clean builds.
- **CompatLib**: Linux builds use `GeneralsMD/Code/CompatLib/` for Win32 API compatibility shims (windows_compat.h, etc.).
- **No native POSIX calls**: Use SDL3 abstractions for timers, threads, file I/O. Do not add raw `pthread_*`, `open()`, etc. into game code.

### Current Progress (as of February 2026)
- Phase 0 (Planning): Complete
- Phase 1 (Graphics/DXVK): Complete -- game renders and runs skirmish maps
- Phase 2 (Audio/OpenAL): In progress
- Phase 3 (Video/FFmpeg): Not started

---

## macOS

**Branch**: `main` (active development alongside Linux)
**Status**: Active development -- Phase 1 (Graphics/DXVK) complete on ARM64 Apple Silicon

### Architecture
- SDL3 for windowing/input (same as Linux)
- DXVK + MoltenVK for DirectX 8 to Vulkan to Metal translation (DX8 → Vulkan → Metal chain)
- OpenAL for audio (same as Linux)
- Current target: **ARM64 (Apple Silicon)** -- macOS 15.0+
- Universal binary (arm64 + x86_64) planned for future

### Key Considerations
- MoltenVK translates Vulkan to Metal; DXVK sits on top of it (DX8 to Vulkan to Metal chain)
- DXVK is built via Meson as an ExternalProject -- must pass `-arch arm64` explicitly via `cmake/meson-arm64-native.ini` to avoid Rosetta2 confusing the host arch
- The full DXVK patch series (~13 patches) is applied automatically by `cmake/dxvk-macos-patches.py` -- do not apply manually
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
./scripts/build-macos-zh.sh          # configure + build
./scripts/deploy-macos-zh.sh         # copy to ~/GeneralsX/GeneralsMD/
./scripts/run-macos-zh.sh -win       # launch windowed
```

### macOS-Specific Notes
- **Rosetta2 + Meson adversarial**: Use `cmake/meson-arm64-native.ini` to force `-arch arm64` in DXVK sub-build
- **DXVK patches are scripted**: `cmake/dxvk-macos-patches.py` applies all 13 MoltenVK/ARM64 compatibility patches automatically at configure time
- **SDL3 from source**: Fetched via CMake FetchContent -- no system package needed
- **Vulkan SDK path**: `~/VulkanSDK/<version>/macOS/` -- must contain `libvulkan.dylib` and `libMoltenVK.dylib`
- **No Cocoa/Metal calls in game code**: All platform access goes through SDL3 + DXVK layers

### Current Progress (as of February 2026)
- Phase 0 (Planning): Complete
- Phase 1 (Graphics/DXVK): Complete -- game renders and runs on ARM64 Apple Silicon
- Phase 2 (Audio/OpenAL): In progress (CMake flag `SAGE_USE_OPENAL=ON` set; full integration pending)
- Phase 3 (Video/FFmpeg): CMake flag `RTS_BUILD_OPTION_FFMPEG=ON` set; playback integration not started

### Remaining Items
- [ ] Audio (OpenAL) fully wired and tested
- [ ] Video (FFmpeg) playback integrated
- [ ] App bundle (.app) packaging
- [ ] Code signing workflow
- [ ] Universal binary (arm64 + x86_64)
- [ ] CI/CD pipeline for macOS builds

---

## Windows (Modern SDL3/DXVK Stack)

**Branch**: TBD
**Status**: Not yet started

**Note**: This is about the **new** Windows build using SDL3 + DXVK + OpenAL (64-bit). The legacy VC6/win32 builds using native DirectX 8 + Miles remain as the upstream baseline and are NOT part of this section.

### Architecture Plan
- SDL3 for windowing/input (replacing native Win32 window management)
- DXVK for DirectX 8 to Vulkan (Vulkan is natively supported on Windows)
- OpenAL for audio (replacing Miles Sound System)
- 64-bit native (x86_64)
- MSVC 2022 or MinGW-w64 toolchain

### Key Considerations
- Windows users with Vulkan-capable GPUs can use the modern stack directly
- Legacy fallback: VC6/win32 presets continue to work for DX8 + Miles path
- Must coexist with upstream TheSuperHackers builds in same repo
- Potential for better performance via Vulkan vs legacy DX8

### Build Presets (Planned)
- **`windows64-deploy`** -- MSVC 2022 or MinGW x86_64, Release
- **`windows64-testing`** -- Debug variant
- **`mingw-w64-i686`** -- MinGW cross-compile (existing, 32-bit)

### TBD Items
- [ ] DXVK on Windows validated (Vulkan drivers required)
- [ ] CMake preset created and tested
- [ ] MSVC 2022 or MinGW-w64 toolchain selected
- [ ] OpenAL integration on Windows
- [ ] Installer/packaging strategy
- [ ] CI/CD pipeline for Windows builds
