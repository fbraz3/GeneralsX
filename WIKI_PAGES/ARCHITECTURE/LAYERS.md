# ARCHITECTURE/LAYERS.md

## Summary

Layer boundaries — what each layer does and how they interact.

---

## Layer Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                    Game Logic Layer                          │
│  (GeneralsMD/Code/GameEngine/, ZeroHour-specific logic)       │
├─────────────────────────────────────────────────────────────┤
│                    Core Game Engine Layer                    │
│  (Core/GameEngine/ — shared between Generals and ZH)          │
├─────────────────────────────────────────────────────────────┤
│              Platform Abstraction Layer (RAG)                 │
│  (SDL3, DXVK, OpenAL, FFmpeg — isolates platform calls)      │
├─────────────────────────────────────────────────────────────┤
│              Platform Implementation Layer                    │
│  (Core/GameEngineDevice/, Core/Libraries/Platform/)           │
├─────────────────────────────────────────────────────────────┤
│                   Platform Drivers Layer                      │
│  (SDL3, DXVK, OpenAL, FFmpeg — actual platform APIs)         │
└─────────────────────────────────────────────────────────────┘
```

---

## Layer Responsibilities

### Layer 1: Game Logic

**Location:** `GeneralsMD/Code/GameEngine/`

**Responsibilities:**
- Zero Hour expansion-specific logic
- Campaign progression
- Difficulty adjustments
- New units/buildings

**Rules:**
- NEVER calls platform APIs
- Platform-agnostic by design
- Zero Hour only (no backport to base game)

---

### Layer 2: Core Game Engine

**Location:** `Core/GameEngine/`

**Responsibilities:**
- Shared game logic between both games
- Unit behavior
- Combat resolution
- Pathfinding
- AI core

**Rules:**
- NEVER calls platform APIs
- Shared between Generals and ZH
- Business logic only

---

### Layer 3: Platform Abstraction (RAG)

**Location:** `Core/GameEngineDevice/`, `Core/Libraries/Source/Platform/`

**Responsibilities:**
- SDL3 window and input abstraction
- DX8→Vulkan rendering abstraction
- Miles→OpenAL audio abstraction
- Bink→FFmpeg video abstraction

**Rules:**
- NEVER calls native platform APIs directly
- Uses feature flags for optional behavior
- Platform code only

---

### Layer 4: Platform Implementation

**Location:** `Core/GameEngineDevice/Source/`, `Core/Libraries/Source/Platform/`

**Responsibilities:**
- Platform-specific initialization
- Driver selection
- Resource loading
- Error handling

**Rules:**
- Compile guards for platform-specific code
- Uses SDL3 abstraction layer
- No native platform calls

---

### Layer 5: Platform Drivers

**Location:** External libraries (SDL3, DXVK, OpenAL, FFmpeg)

**Responsibilities:**
- Actual platform API calls
- Hardware interaction
- Platform-specific rendering/audio

**Rules:**
- Platform-specific APIs
- Configured via SDL3
- No game code dependencies

---

## Layer Interaction Rules

### Rule 1: No Upward Calls

**Never call platform APIs from game logic.**

**Example:**
```cpp
// ❌ WRONG: Game logic calling platform API
void Game::Render() {
    D3D8BeginScene();  // Platform API call!
}
```

**Example:**
```cpp
// ✅ CORRECT: Platform layer handles rendering
void Game::Render() {
    // Game logic only
    Update();
}
```

---

### Rule 2: Feature Flags for Optional Behavior

**When platform-specific behavior is needed:**

```cpp
// Feature flag for experimental code
#ifndef FEATURE_DXVK
#define FEATURE_DXVK 1
#endif
```

---

### Rule 3: Platform Abstraction

**All platform I/O must go through SDL3:**

| Operation | SDL3 API |
|-----------|----------|
| Window creation | `SDL_CreateWindow()` |
| Input handling | `SDL_PollEvent()` |
| File I/O | `SDL_LoadFile()` |
| Thread creation | `SDL_CreateThread()` |

---

## Common Pitfalls

### Pitfall 1: Forgetting Platform Guards

**Problem:** Platform-specific code accidentally included

**Solution:**
```cpp
#ifdef __linux__
  // Linux-only code
#elif defined(__APPLE__)
  // macOS-only code
#else
  // Fallback or error
#endif
```

### Pitfall 2: Direct Platform Calls

**Problem:** Game logic directly calling platform APIs

**Solution:**
- Review code against layer boundaries
- Ensure all calls go through SDL3
- Use static analysis tools

### Pitfall 3: Shared State Across Layers

**Problem:** Game logic accessing platform state

**Solution:**
- Clear separation of concerns
- Platform state in platform layer only
- Game state in game engine layer only

---

## Verification Checklist

### Platform Layer

- [ ] No native platform API calls
- [ ] All I/O through SDL3
- [ ] Feature flags for optional behavior
- [ ] Compile guards for platform-specific code

### Game Layer

- [ ] No platform API calls
- [ ] All rendering through platform layer
- [ ] All audio through platform layer
- [ ] All file I/O through platform layer

---

## See Also

- [ARCHITECTURE/FLOW.md](./FLOW.md) — System lifecycle
- [ARCHITECTURE/DATA.md](./DATA.md) — Data handling
- [CONCEPTS/RAG.md](../CONCEPTS/RAG.md) — Platform isolation

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, architecture documentation
