---
applyTo: '**/*.{cpp,h,hpp,c}'
---

## Code Quality & Maintainability

- **Scope discipline**: Focus on cross-platform port (SDL3, DXVK, OpenAL). Avoid unrelated refactors.
- **Root cause**: Fix underlying issues, not symptoms. No lazy workarounds.
- **Isolation**: Platform-specific code stays in platform layers (`Core/GameEngineDevice/`, `Core/Libraries/Source/Platform/`)
- **Fallback paths**: Keep legacy Windows paths (DX8, Miles) intact behind `#ifdef` guards for VC6 baseline.
- **Determinism**: Never break gameplay determinism. Rendering/audio changes must not affect logic.
- **Audio Backporting**: All audio improvements or fixes must be implemented in OpenAL first, and subsequently backported to the MiniAudio implementation.
- **Generals Backporting**: All general bugfixes and improvements must be backported to the Generals base game.

## Code Style & Conventions

- **English only**: All code, comments, identifiers in English.
- **No lazy solutions**: No empty stubs, empty `catch` blocks, or commented-out code.
- **C++ heritage**: Maintain consistency with surrounding legacy code patterns.
- **Change annotation**: Every user-facing code change needs `// GeneralsX @keyword author DD/MM/YYYY Description` above it.
  - Keywords: `@bugfix` / `@feature` / `@performance` / `@refactor` / `@tweak` / `@build`
- **Upstream PR attribution**: When implementing work derived from a specific upstream PR, add an adjacent comment:
  - `// Upstream reference: <author>, PR #<id>` and the full GitHub URL.

## Platform Isolation Patterns

**Good** — Platform-specific code isolated in device/platform layers:
```cpp
// Core/GameEngineDevice/Include/w3dgraphicsdevice.h
#ifdef BUILD_WITH_DXVK
    #include "dxvk_adapter.h"
#else
    #include <d3d8.h>
#endif
```

**Bad** — Platform code leaking into game logic:
```cpp
// GeneralsMD/Code/GameEngine/GameLogic/object.cpp -- WRONG
#ifdef __linux__
    // Linux-specific hack in gameplay code
#endif
```

SDL3 is the unified platform layer — no native POSIX (`pthread_*`, `open()`), Win32, or Cocoa calls in game code.
