# ARCHITECTURE/FLOW.md

## Summary

System lifecycle — boot sequence, rendering loop, and shutdown flow.

---

## Boot Sequence

```
┌──────────────────────────────────────────────────────────────┐
│                      Boot Sequence                            │
├──────────────────────────────────────────────────────────────┤
│  1. WinMain.cpp                                              │
│     └─ SDL3 initialization                                   │
│     └─ Platform driver selection                             │
│     └─ Core library initialization                           │
│  2. Game Engine Device (GED)                                 │
│     └─ DX8 wrapper initialization                            │
│     └─ Rendering device setup                                │
│     └─ Shader compilation                                    │
│  3. Core Game Engine                                         │
│     └─ Game state initialization                             │
│     └─ Asset loader setup                                    │
│  4. Game Logic (Zero Hour or Base)                           │
│     └─ Campaign state                                        │
│     └─ Difficulty level                                      │
└──────────────────────────────────────────────────────────────┘
```

---

## Rendering Loop

```
┌──────────────────────────────────────────────────────────────┐
│                    Rendering Loop                             │
├──────────────────────────────────────────────────────────────┤
│  1. FramePacer::CheckLimit()                                 │
│     └─ Enforce FPS cap                                       │
│  2. Game::Update()                                          │
│     └─ Game logic update                                     │
│     └─ Input handling                                       │
│  3. Game::Render()                                          │
│     └─ Scene setup                                          │
│     └─ Entity rendering                                     │
│     └─ UI rendering                                         │
│  4. Platform Layer::Present()                                │
│     └─ Swap buffers                                         │
│     └─ VSync control                                        │
└──────────────────────────────────────────────────────────────┘
```

---

## Shutdown Sequence

```
┌──────────────────────────────────────────────────────────────┐
│                     Shutdown Sequence                         │
├──────────────────────────────────────────────────────────────┤
│  1. Game Logic Cleanup                                      │
│     └─ Campaign state cleanup                               │
│  2. Core Game Engine Cleanup                                │
│     └─ Asset loader cleanup                                 │
│     └─ Memory release                                      │
│  3. Game Engine Device Cleanup                              │
│     └─ DX8 wrapper cleanup                                 │
│     └─ Shader cache cleanup                                │
│  4. Platform Layer Cleanup                                  │
│     └─ Rendering device release                            │
│     └─ Audio cleanup                                       │
│  5. WinMain Cleanup                                         │
│     └─ SDL3 shutdown                                       │
│     └─ Platform driver cleanup                             │
└──────────────────────────────────────────────────────────────┘
```

---

## Critical Path Flow

### 1. Initialization

**Platform Layer → Core → Game Logic**

```cpp
// WinMain.cpp
SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
// ↓
// Platform layer initialization
// ↓
// Core game engine initialization
// ↓
// Game logic initialization
```

---

### 2. Asset Loading

**Game Logic → Core → Platform Layer → Platform Drivers**

```cpp
// Request asset
game.RequestAsset("texture.tga");
// ↓
// Core asset loader
// ↓
// Platform layer (SDL3, DXVK)
// ↓
// Platform driver (file system, DXVK)
// ↓
// Asset loaded
```

---

### 3. Rendering

**Game Logic → Core → Platform Layer → Platform Drivers**

```cpp
// Render frame
game.Render();
// ↓
// Game logic rendering
// ↓
// Core rendering
// ↓
// Platform layer (DXVK, SDL3)
// ↓
// Platform driver (Vulkan, X11)
// ↓
// Frame presented
```

---

## Platform-Specific Flow Variations

### Linux

```
Boot: X11 → SDL3 → DXVK → Vulkan
Render: X11 → SDL3 → DXVK → Vulkan
Audio: PulseAudio → SDL3 → OpenAL
```

### macOS

```
Boot: Cocoa → SDL3 → DXVK → Vulkan
Render: Cocoa → SDL3 → DXVK → Vulkan
Audio: CoreAudio → SDL3 → OpenAL
```

---

## Frame Timing

### FPS Cap Enforcement

**Purpose:** Ensure consistent frame timing across platforms.

**Implementation:**
```cpp
FramePacer::CheckLimit();  // Called before each frame
```

**Why:**
- Prevents FPS uncapping on high-refresh monitors
- Ensures deterministic frame pacing
- Maintains replay compatibility

---

## Critical Timing Requirements

| Component | Timing | Reason |
|-----------|--------|--------|
| FramePacer | Per-frame | FPS cap enforcement |
| Asset Loading | Asynchronous | Non-blocking |
| Input Handling | Per-frame | Responsive controls |
| Audio Processing | Per-frame | Synchronized playback |

---

## See Also

- [ARCHITECTURE/LAYERS.md](./LAYERS.md) — Layer boundaries
- [ARCHITECTURE/DATA.md](./DATA.md) — Data handling
- [CONCEPTS/RETAIL.md](../CONCEPTS/RETAIL.md) — Retail compatibility

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, architecture documentation
