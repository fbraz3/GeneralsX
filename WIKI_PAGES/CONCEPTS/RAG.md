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
