# Part 2: Wiki Schema - Page Formats

## Page Formats

### 1. Index Page (README.md)

The wiki's catalog — organized by category with one-line summaries and metadata.

**Format:**

```
# GeneralsX Wiki Index

## Entities (Core project components)
- [PLATFORM.md](ENTITIES/PLATFORM.md) — SDL3, DXVK, OpenAL, FFmpeg
  > Date: 2026-05 | Sources: 12 | Last updated: 2026-05-18
- [GAME.md](ENTITIES/GAME.md) — Generals, Zero Hour, GeneralsMD
  > Date: 2026-05 | Sources: 8 | Last updated: 2026-05-17

## Concepts (Architectural principles)
- [RAG.md](CONCEPTS/RAG.md) — Platform isolation, determinism
  > Date: 2026-05 | Synthesis: Critical for retail compat

## Topics (Procedural knowledge)
- [BUILD.md](TOPICS/BUILD.md) — Presets, CI/CD, deployment

## Architecture (System design)
- [LAYERS.md](ARCHITECTURE/LAYERS.md) — Platform, GameLogic, Shared

## Quick Reference

| Query Type              | Look Here                    |
|-------------------------|------------------------------|
| "What is SDL3's role?"  | ENTITIES/PLATFORM.md         |
| "How to build on Linux" | TOPICS/BUILD.md              |
| "Backport rules"        | ENTITIES/CORE.md             |
| "Platform isolation"    | CONCEPTS/RAG.md              |
```

---

### 2. Entity Pages

Document **core project components** — the immutable nouns that make up GeneralsX.

**Template:**

```
# ENTITIES/PLATFORM.md

## Summary

SDL3, DXVK, OpenAL, FFmpeg — the four platform technologies that replace Win32, DirectX 8, Miles Sound, and Bink Video.

## SDL3

### Role
Unified windowing, input, and platform abstraction. Replaces all native Win32, Cocoa, and X11 calls in game code.

### Key Constraints
- **Never** call native APIs in game logic — use SDL3 only
- Compile guards required for any SDL-specific behavior
- SDL3 from source via CMake FetchContent — no system package

### Common Patterns
```
// Platform isolation
#ifdef __linux__
  // Linux-specific SDL3 code
#elif defined(__APPLE__)
  // macOS-specific SDL3 code
#else
  // Fallback or error
#endif
```

### Known Issues
- Linux case-sensitive asset lookup (fixed in StdLocalFileSystem)
- Headless mode requires `SDL_VIDEODRIVER=dummy`

## DXVK

### Role
DirectX 8 → Vulkan translation layer. Single source of truth for all DX8 rendering.

### Source of Truth
GitHub fork branch `generalsx-macos-v2.6` (auto-update enabled in CMake).

### Key Constraints
- **Never** edit `build/_deps/...` — commit fixes in fork repo first
- Use `-DSAGE_DXVK_USE_LOCAL_FORK=ON` only when debugging
- DXVK needs Vulkan: `vulkan-tools`, `mesa-vulkan-drivers`

### Reference
`references/fighter19-dxvk-port/` — CMake presets, SDL3 hooks, device wrappers.

## OpenAL

### Role
Cross-platform audio stack. Replaces Miles Sound System.

### Reference
`references/jmarshall-win64-modern/Code/Audio/` — Generals-only, adapt carefully for Zero Hour.

### Known Gaps
- See `docs/WORKDIR/support/SPIKE_AUDIO_SDL3_VS_OPENAL.md`

## FFmpeg

### Role
Video playback (intro, campaign, tutorial videos).

### Packaging
Runtime libraries bundled with Linux gzip/AppImage (libavcodec, libavformat, libavutil, libswresample, libswscale).

## Relationships

```
Game Logic ─[SDL3]─► Platform Layer
                 └─[DXVK]─► Vulkan (GPU)
                 └─[OpenAL]─► Audio
                 └─[FFmpeg]─► Video
```

## Recent Changes

- 2026-05-18: CI migrated to macOS + Flatpak
- 2026-05-17: SDL3 headless guard added

---

**See also**: [OVERVIEW.md](../OVERVIEW.md), [CONCEPTS/PORTING.md](../CONCEPTS/PORTING.md)
```

---

### 3. Concept Pages

Document **architectural principles** — the rules and patterns that govern how the project is built.

**Template:**

```
# CONCEPTS/RAG.md

## Summary

Platform Isolation (RAG) — the critical principle that keeps GeneralsX cross-platform without breaking retail compatibility.

## The Problem

The original Generals codebase is **native Windows**:
- Win32 API for windows, input, file I/O
- DirectX 8 for rendering
- Miles Sound System for audio
- Bink Video for intro/campaign videos

Porting to Linux/macOS requires replacing all these technologies — but **game logic must remain unchanged** to support retail replays and mods.

## The Solution: Platform Isolation

### Three Layers

```
┌─────────────────────────────────────────┐
│         Game Logic (Unchanged)          │
│  - BaseType.h, BaseType.cpp            │
│  - GameEngine/ (core gameplay)         │
│  - GameEngineDevice/ (render loop)     │
└───────────────────┬─────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────┐
│       Platform Layer (Isolated)         │
│  - SDL3 (windowing, input)             │
│  - DXVK (DX8→Vulkan)                   │
│  - OpenAL (audio)                      │
│  - FFmpeg (video)                      │
└───────────────────┬─────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────┐
│              Host Platform              │
│         Linux / macOS / Windows         │
└─────────────────────────────────────────┘
```

## Key Rules

### Rule 1: No Native Platform Calls in Game Code

**Never** call Win32, Cocoa, X11, or any platform-specific API directly in:
- `GameEngine/` code
- `GameEngineDevice/` code
- Any game-specific source files

**Use only**:
- SDL3 for all platform I/O
- Feature flags for optional platform behavior

### Rule 2: Compile Guards for Optional Behavior

When platform-specific behavior is needed:

```
#ifdef __linux__
  // Linux-only code
#elif defined(__APPLE__)
  // macOS-only code
#else
  // Fallback or error
#endif
```

### Rule 3: Determinism is Non-Negotiable

Rendering/audio changes **must not affect gameplay logic**:

- FPS cap fixes go in shared code, not game logic
- Math consistency via `-ffp-contract=off` for non-MSVC builds
- All deterministic math changes attributed to upstream PR

## Critical Concepts

### Platform Guards

Required for any SDL3/DXVK/OpenAL code:

```cpp
// Always use SDL3 via GameEngineDevice layer
// Never call SDL_* directly in game logic

// Platform-specific behavior requires guards
#ifdef __linux__
  // Linux-specific
#elif defined(__APPLE__)
  // macOS-specific
#endif
```

### Feature Flags

Optional platform behavior:

```cpp
// Add feature flags for experimental platform code
#ifndef FEATURE_DXVK
#define FEATURE_DXVK 1
#endif
```

### Shared Library Backport Rules

**Backport to Generals when:**
- Change is platform/backend (SDL3, DXVK, OpenAL)
- Change is in shared Core libraries
- Change is low-risk and clearly applicable

**Do NOT backport:**
- Zero Hour-specific gameplay/logic
- Expansion-specific features
- High-risk changes to Zero Hour

## See Also

- [CONCEPTS/PORTING.md](./PORTING.md) — DX8→Vulkan, Miles→OpenAL
- [CONCEPTS/RETAIL.md](./RETAIL.md) — Compatibility guarantees
- [ENTITIES/PLATFORM.md](../ENTITIES/PLATFORM.md) — Platform technologies

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, dev blog, architecture docs
```
