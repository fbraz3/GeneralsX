# WIKI_SCHEMA_PART2.md

## Page Format Examples

Detailed templates for each page type.

---

## Entity Page Example

### ENTITIES/PLATFORM.md

```markdown
# ENTITIES/PLATFORM.md

## Summary

Platform technologies — the external dependencies that enable cross-platform functionality.

---

## Purpose

Replace legacy Windows-specific APIs with modern, cross-platform alternatives.

---

## Responsibilities

| Technology | Replaces | Status |
|------------|----------|--------|
| SDL3 | Win32, Cocoa, X11 | Active |
| DXVK | DirectX 8 | Active |
| OpenAL | Miles Sound System | Active |
| FFmpeg | Bink Video | Active |

---

## SDL3 (Windowing & Input)

### Installation

**Linux:**
```bash
# Arch Linux
sudo pacman -S sdl3

# Debian/Ubuntu
sudo apt-get install libsdl3-dev

# macOS
# Bundled with vcpkg or Homebrew
brew install sdl3
```

### Platform-Specific Notes

| Platform | Video Driver | Audio Driver |
|----------|--------------|--------------|
| Linux | x11 (default) | pulse (default) |
| macOS | cocoa (default) | coreaudio (default) |

### Known Issues

- **Linux:** Case-sensitive asset lookup (fixed 2026-05-11)
- **macOS:** Transparency regression (fixed 2026-05-10)

---

## DXVK (Graphics)

### Configuration

**Default:** GitHub fork branch `generalsx-macos-v2.6` (auto-update enabled)

**Local mode:** `-DSAGE_DXVK_USE_LOCAL_FORK=ON`

### Platform Issues

| Issue | Platform | Status |
|-------|----------|--------|
| Transparency regression | macOS | Fixed |
| Headless texture path | Linux | Fixed |

---

## OpenAL (Audio)

### Reference Implementation

Based on **jmarshall-win64-modern** repository for architecture patterns.

### Known Issues

- None reported

---

## FFmpeg (Video)

### Packaging

| Platform | Package | Command |
|----------|---------|--------|
| Linux (Arch) | ffmpeg | `sudo pacman -S ffmpeg` |
| Linux (Debian) | ffmpeg | `sudo apt-get install ffmpeg` |
| macOS | ffmpeg | `brew install ffmpeg` |

### Known Issues

- **Linux:** Case-sensitive lookup caused video loading failures (fixed 2026-05-11)

---

## See Also

- [CONCEPTS/PORTING.md](../CONCEPTS/PORTING.md) — DX8→Vulkan, Miles→OpenAL
- [ARCHITECTURE/LAYERS.md](../ARCHITECTURE/LAYERS.md) — Layer boundaries

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, platform documentation
```

---

## Concept Page Example

### CONCEPTS/RAG.md

```markdown
# CONCEPTS/RAG.md

## Summary

Platform Isolation — never call native platform APIs in game logic.

---

## Purpose

Ensure game logic remains platform-agnostic for maximum portability and maintainability.

---

## Key Principles

### Principle 1: No Native Platform Calls

**Never** call Win32, Cocoa, X11, or any platform-specific API directly in:
- `GameEngine/` code
- `GameEngineDevice/` code  
- Any game-specific source files

**Use only:**
- SDL3 for all platform I/O
- Feature flags for optional platform behavior

### Principle 2: Isolation Layer

All platform code must sit in:
- `Core/GameEngineDevice/Source/`
- `Core/Libraries/Source/Platform/`

---

## Implementation Rules

### Rule 1: Compile Guards for Optional Behavior

When platform-specific behavior is needed:

```cpp
#ifdef __linux__
  // Linux-only code
#elif defined(__APPLE__)
  // macOS-only code
#else
  // Fallback or error
#endif
```

### Rule 2: Feature Flags for Experimental Code

```cpp
#ifndef FEATURE_DXVK
#define FEATURE_DXVK 1
#endif
```

### Rule 3: Platform Abstraction

**DO:**
```cpp
// Use SDL3 abstraction
SDL_SetWindowResizable(window, SDL_TRUE);
```

**DON'T:**
```cpp
// Direct X11 call
XInitThreads();
```

---

## Common Pitfalls

- **Pitfall 1:** Forgetting SDL3 initialization before platform calls
- **Pitfall 2:** Using platform-specific path separators
- **Pitfall 3:** Case-sensitive file access on Linux

---

## See Also

- [ARCHITECTURE/LAYERS.md](../ARCHITECTURE/LAYERS.md) — Layer boundaries
- [CONCEPTS/PORTING.md](../CONCEPTS/PORTING.md) — DX8→Vulkan, Miles→OpenAL

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, architecture docs
```

---

## Topic Page Example

### TOPICS/BUILD.md

```markdown
# TOPICS/BUILD.md

## Summary

Build instructions — creating and testing GeneralsX on Linux and macOS.

---

## Prerequisites

### Linux

- [Docker](https://www.docker.com/) — Recommended for CI compatibility
- [Git](https://git-scm.com/) — Version control
- Build tools: `cmake`, `ninja`, `make`

### macOS

- [Homebrew](https://brew.sh/) — Package manager
- [Git](https://git-scm.com/) — Version control
- [vcpkg](https://vcpkg.io/) — C++ library manager (for SDL3, DXVK)

---

## Step-by-Step Guide

### Step 1: Clone Repository

```bash
git clone https://github.com/yourusername/GeneralsX.git
cd GeneralsX
```

### Step 2: Configure Build

#### Docker (Linux, Recommended)

```bash
# Linux build
./scripts/build/linux/docker-configure-linux.sh linux64-deploy

# Optional: Windows via MinGW cross-build
./scripts/build/linux/docker-build-mingw-zh.sh mingw-w64-i686
```

#### Native Linux

```bash
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals
```

#### macOS

```bash
./scripts/build/macos/build-macos-zh.sh
```

### Step 3: Verify Build

```bash
# Check binary exists
ls -lh build/linux64-deploy/GeneralsMD/GeneralsXZH

# Run smoke test
./scripts/qa/smoke/docker-smoke-test-zh.sh linux64-deploy
```

---

## Platform-Specific Notes

| Platform | Command | Notes |
|----------|---------|-------|
| Linux | `./scripts/build/linux/deploy-linux-zh.sh` | Deployment script |
| macOS | `./scripts/build/macos/deploy-macos-zh.sh` | Deployment script |

---

## Troubleshooting

### "SDL3 not found" error

**Cause:** Missing or incomplete vcpkg installation

**Solution:**
```bash
# macOS
VCPKG_ROOT=$(resolve_vcpkg_root)
export VCPKG_ROOT
```

### "vcpkg not found" error

**Cause:** VCPKG_ROOT not set or vcpkg not installed

**Solution:**
```bash
# Install vcpkg
https://vcpkg.io/docs/getting-started.html

# Set VCPKG_ROOT
export VCPKG_ROOT=/path/to/vcpkg
```

---

## See Also

- [CONCEPTS/PORTING.md](../CONCEPTS/PORTING.md) — Platform technologies
- [TOPICS/TESTING.md](./TESTING.md) — Testing protocol

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, build documentation
```

---

## Changes Page Example

### CHANGES/2026-05.md

```markdown
# CHANGES/2026-05.md

## Summary

May 2026 changes summary — a chronological record of all wiki-relevant changes.

---

## Overview

This month has focused on:
1. Platform layer hardening (SDL3, DXVK, OpenAL)
2. Retail compatibility fixes (FPS caps, case-sensitive assets, replay paths)
3. CI/CD improvements (macOS migration, Flatpak integration)
4. Upstream sync with thesuperhackers

---

## Detailed Changes

### 2026-05-18

#### CI and Deployment

**Changes:**
- Migrated CI to keep only macOS and Linux Flatpak builds
- Switched Linux headless replay tests to run inside Flatpak
- Removed Linux gzip and Linux AppImage jobs from `.github/workflows/ci.yml`

**Wiki Updates:**
- Updated `ENTITIES/PLATFORM.md` (FFmpeg packaging section)
- Updated `TOPICS/BUILD.md` (Docker/Flatpak workflow)
- Updated `TOPICS/TESTING.md` (headless replay in Flatpak)

**Related:**
- Dev blog: 2026-05-18 CI migration to macOS + Flatpak

---

### 2026-05-11

#### FPS Uncapped Fix (ISSUE #132)

**Changes:**
1. **FramePacer API guard:** Added `#ifdef _WIN32` guards for `timeBeginPeriod()`/`timeEndPeriod()` in `FramePacer.cpp`
2. **FPS limit validation:** Added early-exit handling for invalid FPS targets in `FrameRateLimit::wait()`
3. **FPS init logic fix:** Changed condition to always ensure valid BaseFps (30) default
4. **Build flag:** Added `-ffp-contract=off` for non-MSVC builds

**Wiki Updates:**
- Created `CONCEPTS/DETERMINISM.md` (FPS caps, math consistency)
- Updated `ENTITIES/PLATFORM.md` (FFmpeg section)
- Updated `CONCEPTS/RETAIL.md` (determinism rules)

**Related:**
- Dev blog: 2026-05-11 Fix FPS uncapped and frame counter instability

---

## Summary Statistics

| Category | Count |
|----------|-------|
| New wiki pages | 4 |
| Updated wiki pages | 15 |
| Platform changes | SDL3, DXVK, OpenAL |
| Retail fixes | FPS caps, case-sensitive assets, replay paths |
| CI improvements | macOS migration, Flatpak integration |

---

## See Also

- [LATEST.md](../LATEST.md) — Change log
- [CONCEPTS/DETERMINISM.md](../CONCEPTS/DETERMINISM.md) — Determinism principles

---

**Last updated**: 2026-05-18 | **Sources**: Dev blog, commit history
```

---

## Architecture Page Example

### ARCHITECTURE/LAYERS.md

```markdown
# ARCHITECTURE/LAYERS.md

## Summary

Layer boundaries — defining the separation between platform, game logic, and shared code.

---

## Three-Layer Architecture

```
┌─────────────────────────────────────────┐
│         Game Logic (Unchanged)          │
│  - BaseType.h, BaseType.cpp            │
│  - GameEngine/ (core gameplay)         │
│  - GameEngineDevice/ (render loop)     │
│  - All game-specific code              │
└───────────────────┬─────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────┐
│       Platform Layer (Isolated)         │
│  - SDL3 (windowing, input)             │
│  - DXVK (DX8→Vulkan)                   │
│  - OpenAL (audio)                      │
│  - FFmpeg (video)                      │
│  - Platform-specific file system       │
└───────────────────┬─────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────┐
│              Host Platform              │
│         Linux / macOS / Windows         │
└─────────────────────────────────────────┘
```

---

## Layer Boundaries

### Game Logic Layer

**Location:**
- `GeneralsMD/Code/GameEngine/`
- `GeneralsMD/Code/`
- `Generals/Code/`

**Responsibilities:**
- All gameplay mechanics
- Unit AI and behavior
- Campaign and replay logic
- Network multiplayer

**Constraints:**
- **Never** call platform APIs directly
- **Never** contain platform-specific code
- Must work identically on all platforms

### Platform Layer

**Location:**
- `Core/GameEngineDevice/Source/`
- `Core/Libraries/Source/Platform/`

**Responsibilities:**
- Window creation and management (SDL3)
- Input handling (SDL3)
- Rendering (DXVK wrapper)
- Audio (OpenAL)
- Video (FFmpeg)
- File I/O platform-specific behavior

**Constraints:**
- Must not affect game logic
- Must be testable independently
- Changes require platform isolation review

### Shared Libraries

**Location:**
- `Core/GameEngine/`
- `Core/GameEngineDevice/`
- `Core/Libraries/`

**Responsibilities:**
- Utilities used by both games
- Platform-agnostic game logic
- Cross-game compatibility layer

**Constraints:**
- Changes apply to both Generals and Zero Hour
- Must maintain backward compatibility

---

## Platform Isolation Rules

### Rule 1: No Native Platform Calls in Game Code

**Never** call Win32, Cocoa, X11, or any platform-specific API directly in:
- `GameEngine/` code
- `GameEngineDevice/` code
- Any game-specific source files

**Use only:**
- SDL3 for all platform I/O
- Feature flags for optional platform behavior

### Rule 2: Compile Guards for Optional Behavior

When platform-specific behavior is needed:

```cpp
#ifdef __linux__
  // Linux-only code
#elif defined(__APPLE__)
  // macOS-only code
#else
  // Fallback or error
#endif
```

### Rule 3: Feature Flags for Experimental Code

```cpp
#ifndef FEATURE_DXVK
#define FEATURE_DXVK 1
#endif
```

---

## See Also

- [CONCEPTS/RAG.md](../CONCEPTS/RAG.md) — Platform isolation principles
- [CONCEPTS/PORTING.md](../CONCEPTS/PORTING.md) — DX8→Vulkan, Miles→OpenAL
- [ENTITIES/PLATFORM.md](../ENTITIES/PLATFORM.md) — Platform technologies
- [CONCEPTS/RETAIL.md](../CONCEPTS/RETAIL.md) — Retail compatibility

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, architecture docs
```

---

## See Also

- [WIKI_SCHEMA.md](./WIKI_SCHEMA.md) — Main schema
- [WIKI_SCHEMA_PART3.md](./WIKI_SCHEMA_PART3.md) — Operations guide
- [README.md](./README.md) — Wiki index

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, wiki schema design
