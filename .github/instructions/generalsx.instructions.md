---
applyTo: '**'
---

# Objectives

GeneralsX is a **community-driven Linux port** of Command & Conquer: Generals Zero Hour (2003), enabling the classic game to run natively on Linux systems while preserving the original gameplay experience.

This is a **massive C++ game engine** (~500k+ LOC) being carefully ported from Windows to Linux via MinGW/DXVK while maintaining compatibility with the original game's architecture and Windows builds.

**Critical Context**: This is NOT a greenfield project. You're working with 20+ year old game code. Our strategy is to learn from proven Linux port efforts (fighter19, jmarshall) while keeping Windows builds intact.

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
- **Status**: Our stable baseline for Windows compatibility

### fighter19's DXVK Port (Primary Linux Reference)
- **Repository**: `references/fighter19-dxvk-port/` (local copy)
- **Deepwiki Repo**: `Fighter19/CnC_Generals_Zero_Hour`
- **Strategy**: Linux port with DirectX→Vulkan via DXVK
- **Build System**: CMake + MinGW-w64 + DXVK + SDL3
- **Coverage**: Both Generals AND Zero Hour
- **Focus**: Graphics layer (DXVK), Linux POSIX compatibility, SDL windowing
- **Audio**: Currently broken (Miles Sound System stubs)
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

**Primary Goal**: Make *Generals: Zero Hour* (`GeneralsXZH`) run natively on Linux using DXVK for graphics and OpenAL for audio, while maintaining Windows build compatibility.

**Secondary Goal**: Implement the same changes for *Generals* base game (`GeneralsX`) when changes are clearly shared and low-risk.

**Non-negotiable**:
- **MUST preserve Windows builds**: VC6 and Win32 presets must continue to build and run
- **MUST preserve retail compatibility**: Original game files, replays, and mods must work
- **MUST isolate platform code**: Linux-specific changes confined to platform/backend layers

**Out of Scope (for now)**:
- macOS native builds (use Wine/CrossOver or virtualization)
- New gameplay features, balance changes, or "modern enhancements"
- Multiplayer/network compatibility testing (future work)

**Strategy**:
1. **Base**: TheSuperHackers code (stable Windows baseline)
2. **Graphics Reference**: fighter19's DXVK integration patterns
3. **Audio Reference**: jmarshall's OpenAL implementation
4. **Focus**: Zero Hour first; backport to Generals only when clearly applicable
5. **Method**: Research-first, minimal diffs, strict platform/backend isolation

## Golden Rules

1. **IMPERATIVE** - No band-aids or workarounds, only real solutions.
2. Use the `fighter19` reference repo because it successfully made it work on Linux.
3. The `jmarshall` repo may have interesting insights since it successfully created a 64-bit Windows build.
4. The focus is to create a Linux build without losing Windows compatibility, so be very careful when removing things.
5. Use DXVK which provides d3d8.h and translates DirectX8 → Vulkan.
6. The Linux build focus is SDL3 - no native POSIX calls.
7. Some solutions from `fighter19` and `jmarshall` may involve removing components; in that case, stop and ask the user what should be done, as it may be something the game depends on and cannot be removed (e.g., audio, video playback).
8. Any lesson learned should be documented in `docs/WORKDIR/lessons` for future reference and to avoid repeating mistakes. You should also consult that directory for insights from previous sessions.

## Modernization Strategy - Linux Port Phases

We follow a **multi-phase research-first approach** with clear acceptance criteria.

### Phase 0: Deep Analysis & Planning (CURRENT)
**Goal**: Thoroughly analyze reference repositories and document Linux port architecture before implementation.

**Deliverables**:
- fighter19's DXVK integration architecture documented (DirectX wrapping, device management, SDL3 windowing)
- jmarshall's OpenAL audio system documented (Miles→OpenAL mapping, audio pipeline)
- Platform abstraction layer design (Win32→POSIX, filesystem, threading)
- Build system strategy (MinGW presets, cross-compilation from macOS)
- Testing strategy (Windows builds must pass, Linux builds validated in VM)
- Risk assessment (determinism, stability, deployment, debugging)

**Acceptance Criteria**:
Phase 0 is **COMPLETE** when:
- [ ] Current renderer architecture documented (DX8 interfaces, entry points)
- [ ] fighter19 DXVK patterns documented with "what applies to Zero Hour" notes
- [ ] jmarshall OpenAL patterns documented with "how to adapt for Zero Hour" notes
- [ ] Platform abstraction layer design approved (minimal changes to game logic)
- [ ] Docker build workflow configured and tested (native Linux ELF compilation)
- [ ] Windows build validation pipeline established (VC6/Win32 must pass)
- [ ] Phase 1 implementation plan written with step-by-step checklist

### Phase 1: Linux Graphics - DXVK Integration
**Goal**: Port fighter19's DXVK graphics layer to GeneralsXZH, enabling Linux rendering via Vulkan.

**Scope**:
- Implement DXVK DirectX8→Vulkan wrapper integration
- Port SDL3 windowing/input layer (replace Win32 window management)
- Add MinGW build preset for Linux (32-bit i686, 64-bit x86_64)
- Isolate graphics changes to `Core/GameEngineDevice/` and platform headers
- Keep Win32 DX8 path intact behind compile flags

**Non-scope**:
- Audio (will be silent/stubbed for Phase 1)
- Video playback (defer to Phase 3)

**Acceptance Criteria**:
Phase 1 is **COMPLETE** when:
- [ ] GeneralsXZH builds with Linux preset (`linux64-deploy`) via Docker
- [ ] Native Linux ELF binary launches and reaches main menu with graphics rendered
- [ ] Basic gameplay (skirmish map) runs without crashes
- [ ] Windows builds (VC6/Win32) still compile and run correctly
- [ ] Platform-specific code properly isolated (no Linux code in game logic)

### Phase 2: Linux Audio - OpenAL Integration
**Goal**: Replace Miles Sound System with OpenAL, using jmarshall's implementation as reference.

**Scope**:
- Implement OpenAL audio backend (adapt from jmarshall, Zero Hour specifics)
- Create Miles→OpenAL API compatibility layer
- Add audio format conversion (Miles formats→OpenAL formats)
- Isolate audio changes to `Core/GameEngine/Audio/` (new abstraction layer)
- Keep Miles path intact behind compile flags for Windows

**Acceptance Criteria**:
Phase 2 is **COMPLETE** when:
- [ ] GeneralsXZH Linux build has working audio (sound effects, music, voices)
- [ ] Audio quality comparable to original (no pops, clicks, or timing issues)
- [ ] Windows builds still use Miles Sound System (no regressions)
- [ ] Audio backend selection via compile flag (`USE_OPENAL` vs `USE_MILES`)

### Phase 3: Video Playback - Bink Alternative (Spike)
**Goal**: Investigate and prototype Bink video replacement for intro/campaign videos.

**Scope**:
- Research alternative video codecs (FFmpeg, custom decoder)
- Investigate fighter19/jmarshall approaches to video playback
- Prototype minimal viable solution
- Document findings and recommend path forward

**This is a SPIKE**: May result in "defer to later" decision if too complex.

**Acceptance Criteria**:
Phase 3 is **COMPLETE** when:
- [ ] Video playback options researched and documented
- [ ] At least one prototype implemented and tested
- [ ] Recommendation written: "implement X" or "defer indefinitely"
- [ ] If deferred: game fails gracefully without videos (skip to menu)

### Phase 4: Polish & Hardening (Future)
**Goal**: Address Linux-specific bugs, performance, and compatibility issues.

**Scope**: TBD based on Phase 1-3 findings

## Development Environment & Build System

### Primary Development Platform
**You are developing on macOS**. This has implications:

1. **Cross-compilation required**: Cannot build Windows (VC6/Win32) or Linux natively on macOS
2. **Testing requires VMs or containers**: Windows builds in Windows VM, Linux builds in Linux VM or Docker
3. **Docker recommended**: Best way to compile Linux builds from macOS (native ELF binaries)

### Build Presets

#### Windows Builds (Requires Windows or VM)
- **`vc6`** - Visual Studio 6 (C++98) - **STABLE BASELINE** (retail compatible)
  - Primary compatibility target
  - Must pass replay tests
  - Required for any gameplay/logic changes
- **`win32`** - MSVC 2022 (C++20) - Experimental modernization
  - Secondary Windows target
  - Not required for Linux port work

#### Linux Builds (Native ELF via Docker/VM)
- **`linux64-deploy`** - GCC/Clang 64-bit (x86_64) - **PRIMARY LINUX TARGET**
  - Requires Linux environment (Docker/VM)
  - Native ELF binaries (not Windows PE)
  - Uses DXVK for DirectX→Vulkan translation
  - Uses SDL3 for windowing/input
- **`linux64-testing`** - Debug variant
  - For development and testing

### Build Workflow (macOS Developer)

**Option 1: Docker-based builds (RECOMMENDED)** - Keep your Mac clean!

**A) Linux Builds (Native ELF)**
```bash
# Native Linux compilation in Docker
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && apt install -y build-essential cmake ninja-build git && 
  cmake --preset linux64-deploy && 
  cmake --build build/linux64-deploy --target z_generals
"

# Test in Linux Docker container
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 \
  /work/build/linux64-deploy/GeneralsMD/GeneralsXZH -win
```

**B) Windows Builds (MinGW cross-compile)**
```bash
# Cross-compile Windows .exe in Docker (no brew install needed!)
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && apt install -y build-essential cmake ninja-build git mingw-w64 && 
  cmake --preset mingw-w64-i686 && 
  cmake --build build/mingw-w64-i686 --target z_generals
"

# Result: Windows .exe (test in Windows VM or Wine)
```

**Option 2: VMs for native builds**
```bash
# Linux VM: Native ELF builds
sudo apt install build-essential cmake ninja-build
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals

# Windows VM: VC6/Win32 builds (required for compatibility testing)
cmake --preset vc6
cmake --build build/vc6 --target z_generals
```

### Testing Strategy

**Phase 0-1** (Graphics only):
1. Validate Windows builds in Windows VM (VC6 preset)
2. Build Linux with Docker (linux64-deploy preset)
3. Test Linux in Docker/VM (native ELF execution)
4. Run basic smoke tests (launch, menu navigation, load map)

**Phase 2+** (Audio/Video):
1. Full integration testing required
2. Replay compatibility tests (Windows VC6 only)
3. Performance benchmarks (compare Linux vs Windows)

## Update Dev Blog Before Commits

Before committing changes, update the development diary at `docs/DEV_BLOG/YYYY-MM-DIARY.md` (YYYY-MM is the current year and month). Follow `.github/instructions/docs.instructions.md`.

Create the diary file if it doesn't exist:
```bash
mkdir -p docs/DEV_BLOG
touch docs/DEV_BLOG/$(date +%Y-%m)-DIARY.md
```

## CRITICAL: DXVK Ephemeral Patches

**The Problem**: DXVK headers are fetched via CMake `FetchContent` into `build/_deps/dxvk-src/`. Clean builds or reconfigures refetch pristine DXVK from git, **losing all manual patches**.

**Impact**: Type conflicts return (MEMORYSTATUS, IUnknown redefinitions) after:
- `rm -rf build/` (clean build)
- `docker-configure-linux.sh` (reconfigure)
- CMake cache invalidation

**The Solution - Pre-Guard Pattern (PREFERRED)**:

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

1. **windows_base.h** - Add guard around MEMORYSTATUS:
```cpp
#ifndef _MEMORYSTATUS_DEFINED
typedef struct MEMORYSTATUS { /* ... */ } MEMORYSTATUS;
#define _MEMORYSTATUS_DEFINED 1
#endif
```

2. **unknwn.h** - Add guard around IUnknown:
```cpp
#ifndef _IUNKNOWN_DEFINED
struct IUnknown { /* ... */ };
#define _IUNKNOWN_DEFINED 1
#endif
```

**Future TODO**: Automate patching via CMake `CMAKE_PATCH_COMMAND` or `file(APPEND)` in `CMakeLists.txt`.

**Reference**: See Session 28 writeup in `docs/DEV_BLOG/2026-02-DIARY.md` for full context.

## Guidelines (Linux Port Branch)

### Code Quality & Maintainability
- **Scope discipline**: Focus on Linux port (DXVK, OpenAL, POSIX). Avoid unrelated refactors.
- **Root cause**: Fix underlying issues, not symptoms. No lazy workarounds.
- **Isolation**: Platform-specific code stays in platform layers (`Core/GameEngineDevice/`, `Core/Libraries/Source/Platform/`)
- **Fallback paths**: Keep Windows paths (DX8, Miles) intact behind `#ifdef` guards
- **Determinism**: Never break gameplay determinism. Rendering/audio changes must not affect logic.

### Code Style & Conventions
- **English only**: All code, comments, identifiers, documentation in English
- **No lazy solutions**: No empty stubs, empty `catch` blocks, or commented-out code
- **Terminal hygiene**: No emojis or exclamation marks in terminal commands
- **C++ heritage**: Maintain consistency with surrounding legacy code (C++98 patterns)

### Platform Isolation Patterns

**Good** - Platform-specific code isolated:
```cpp
// Core/GameEngineDevice/Include/w3dgraphicsdevice.h
#ifdef BUILD_WITH_DXVK
    #include "dxvk_adapter.h"
#else
    #include <d3d8.h>
#endif

class W3DGraphicsDevice {
public:
#ifdef BUILD_WITH_DXVK
    IDXVKDevice* GetDevice();
#else
    IDirect3DDevice8* GetDevice();
#endif
};
```

**Bad** - Platform code leaking into game logic:
```cpp
// GeneralsMD/Code/GameEngine/GameLogic/object.cpp - WRONG
#ifdef __linux__
    // Linux-specific hack in gameplay code
#endif
```

### Build & Run Workflow

**Prefer VS Code Tasks** for build/deploy/run/debug:
- Tasks automatically set up build environment
- Capture logs to `logs/` directory
- Handle platform-specific quirks (PowerShell on Windows, bash on Linux)

**Logging Conventions**:
- Build logs: `logs/phase<N>_build_<preset>.log`
- Run logs: `logs/phase<N>_run_<preset>.log`
- Debug logs: `logs/phase<N>_debug_<preset>.log`

Example:
```bash
logs/phase1_build_mingw.log     # Phase 1 Linux build
logs/phase1_run_linux.log       # Phase 1 Linux execution
logs/phase2_build_vc6.log       # Phase 2 Windows validation
```

## Target Priority

1. **Primary**: `GeneralsXZH` (Zero Hour expansion) - Most feature-complete
2. **Secondary**: `GeneralsX` (Base game) - Backport when changes are clearly shared

**When to backport Generals ← Zero Hour**:
- Change is platform/backend code (DXVK, OpenAL, POSIX)
- Change is in shared Core libraries
- Change is low-risk and clearly applicable

**When NOT to backport**:
- Change is Zero Hour-specific gameplay/logic
- Change touches expansion-specific features
- Risk of breaking Generals outweighs benefit

## Merge From TheSuperHackers Upstream

**Daily sync workflow**:
```bash
# Ensure remote exists
git remote add thesuperhackers git@github.com:TheSuperHackers/GeneralsGameCode.git

# Daily before starting work
git fetch thesuperhackers
git merge thesuperhackers/main

# Resolve conflicts (prefer our platform changes, their logic changes)
# Test: configure + build VC6 preset
cmake --preset vc6
cmake --build build/vc6 --target z_generals
```

**Conflict resolution strategy**:
- **Platform code** (`Core/GameEngineDevice/`, SDL, DXVK): Keep ours
- **Game logic** (`GeneralsMD/Code/GameEngine/`, `Core/GameEngine/`): Keep theirs
- **Build system** (CMake, presets): Merge carefully (we have extra MinGW presets)
- **When in doubt**: Create a backup branch and test both versions

## Available Reference Repositories

**CRITICAL**: Use references to understand patterns and architecture, **NOT** for copy-paste. Each reference has different scope and must be adapted for Zero Hour.

### Primary References (In Order of Consultation)

#### 1. fighter19's DXVK Port (`references/fighter19-dxvk-port/`)
**Use for**: Graphics layer (DXVK), SDL3 integration, MinGW build setup, POSIX compat
**Coverage**: Generals + Zero Hour (full coverage)
**Status**: Fully functional Linux build

Key files to study:
- `GeneralsMD/Code/GameEngineDevice/` - DXVK device management
- `Core/Libraries/Source/Platform/` - POSIX abstractions
- `CMakePresets.json` - MinGW build configuration
- `cmake/mingw.cmake` - Cross-compilation setup

**Adaptation notes**:
- Already Zero Hour compatible (direct port possible)
- Audio is stubbed (Phase 2 will replace with OpenAL)
- SDL3 usage is well-isolated (good pattern to follow)

#### 2. jmarshall's Modern Port (`references/jmarshall-win64-modern/`)
**Use for**: OpenAL audio implementation, modern C++ patterns
**Coverage**: Generals base game ONLY (**NOT Zero Hour**)
**Status**: Fully functional Windows build with working audio

Key files to study:
- `Code/Audio/` - OpenAL implementation (adapt for Zero Hour)
- `Code/Audio/MusicManager.cpp` - Audio format handling
- INI parser improvements (may be useful for POSIX file handling)

**Adaptation notes**:
- **CAREFUL**: Generals-only, must adapt for Zero Hour expansion features
- OpenAL patterns are solid but need Zero Hour audio hook points
- Some modernization may not apply (we maintain Windows compatibility)

#### 3. TheSuperHackers Main (`references/thesuperhackers-main/`)
**Use for**: Upstream baseline, Windows behavior reference
**Coverage**: Generals + Zero Hour (upstream source)
**Status**: Reference copy for comparison during merges

### Reference Consultation Workflow

**Before implementing a feature**:
1. Read relevant Phase documentation (Phase 0/1/2/3)
2. Study fighter19 implementation (if graphics/platform)
3. Study jmarshall implementation (if audio/modernization)
4. Document differences and adaptation strategy
5. Implement with platform isolation in mind
6. Validate Windows builds still work

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

## Vulkan Documentation (For DXVK Development)

If you need Vulkan API reference locally for DXVK work:

```bash
mkdir -p docs/Support/Vulkan
cd docs/Support/Vulkan

# Download Vulkan SDK docs for your platform
# macOS
wget https://vulkan.lunarg.com/doc/download/VulkanSDK-macOS-Docs-latest.dmg

# Or Linux (for VM work)
wget https://vulkan.lunarg.com/doc/download/VulkanSDK-Linux-Docs-latest.tar.gz
tar -xzf VulkanSDK-Linux-Docs-latest.tar.gz
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

## Next Steps (You Should Do These Now)

### Immediate Actions (Phase 0 Setup)

1. **Create documentation structure**:
   ```bash
   mkdir -p docs/{DEV_BLOG,WORKDIR/{phases,planning,reports,support,audit,lessons},ETC}
   touch docs/DEV_BLOG/2026-02-DIARY.md
   ```

2. **Set up Docker** (for Linux AND Windows builds - no brew installs!):
   ```bash
   # Install Docker Desktop for Mac
   brew install --cask docker
   # Or download from: https://www.docker.com/products/docker-desktop
   
   # Verify Docker is installed
   docker --version
   
   # Pull Ubuntu base image (used for both Linux and MinGW builds)
   docker pull ubuntu:22.04
   ```

3. **Configure build presets** (if missing):
   ```bash
   # Check if linux64-deploy exists in CMakePresets.json
   grep -A 5 "linux64-deploy" CMakePresets.json
   
   # Check if mingw-w64-i686 exists
   grep -A 5 "mingw-w64-i686" CMakePresets.json
   
   # If missing, copy from references/fighter19-dxvk-port/CMakePresets.json
   ```

4. **Set up reference remotes** (for easy diffing):
   ```bash
   cd references/fighter19-dxvk-port
   git remote add upstream https://github.com/Fighter19/CnC_Generals_Zero_Hour.git
   
   cd ../jmarshall-win64-modern
   git remote add upstream https://github.com/jmarshall2323/CnC_Generals_Zero_Hour.git
   ```

5. **Start Phase 0 analysis**:
   - Document current renderer architecture (where does DX8 initialize?)
   - Map fighter19's DXVK changes (what files changed, what approach used?)
   - Map jmarshall's OpenAL changes (what's the audio abstraction layer?)
   - See `docs/WORKDIR/planning/NEXT_STEPS.md` for detailed guidance

6. **Update tasks.json** with Docker build tasks (needs updating)

## VS Code Tasks

Tasks configured for multi-platform development:

**Linux Build Tasks** (via Docker):
- Configure (Linux native preset)
- Build GeneralsXZH (Docker container)
- Build GeneralsX (Docker container)

**Windows Build Tasks** (requires VM/remote):
- Configure (VC6/Win32 presets)
- Build GeneralsXZH (Windows)
- Build GeneralsX (Windows)

**Testing Tasks**:
- Run GeneralsXZH (Linux Docker)
- Run GeneralsXZH (Windows VM)
- Validate Prerequisites (Docker/toolchain)

**Development Tasks**:
- Update Dev Blog
- Full Pipeline (configure + build + test)

See `.vscode/tasks.json` for complete task definitions.
