# ENTITIES/CORE.md

## Summary

Core components — GameEngine, GameEngineDevice, and shared libraries.

---

## Core Components

### GameEngine

**Location:** `Core/GameEngine/`

**Purpose:** Shared game logic between both games.

**Responsibilities:**
- Base game logic
- Unit behavior
- Combat resolution
- Pathfinding
- AI core

**Key Features:**
- Shared between Generals and Zero Hour
- Business logic only
- No platform API calls

---

### GameEngineDevice

**Location:** `Core/GameEngineDevice/`

**Purpose:** Platform-specific device setup and rendering.

**Responsibilities:**
- Rendering device setup
- DX8 wrapper initialization
- Shader compilation
- Platform-specific device management

**Key Features:**
- Platform-specific code
- Uses SDL3 abstraction
- DXVK integration

---

### Libraries

**Location:** `Core/Libraries/`

**Purpose:** Shared utility libraries.

**Responsibilities:**
- Common utilities
- Platform abstraction
- Helper functions

**Key Features:**
- Cross-platform compatibility
- Reusable code
- Well-tested utilities

---

## Component Interaction

```
┌─────────────────────────────────────────────────────────────┐
│                    Core Component Flow                        │
├─────────────────────────────────────────────────────────────┤
│  Game Logic (GeneralsMD)                                     │
│         ↓                                                    │
│  Core Game Engine                                           │
│         ↓                                                    │
│  GameEngineDevice (Platform Layer)                          │
│         ↓                                                    │
│  Libraries (Utilities)                                      │
└─────────────────────────────────────────────────────────────┘
```

---

## Library Structure

```
Core/Libraries/
├── Source/
│   ├── Platform/              ← Platform abstraction
│   ├── Utilities/             ← Common utilities
│   └── Math/                  ← Math helpers
└── Headers/
    ├── Platform.h             ← Platform API
    ├── Utilities.h            ← Utility functions
    └── Math.h                 ← Math helpers
```

---

## Key Design Decisions

### Shared vs. Separate

**Shared (GameEngine):**
- Base game logic
- Common mechanics
- Reusable components

**Separate (GeneralsMD):**
- Zero Hour expansion
- Campaign logic
- Difficulty settings

**Platform-Specific (GED):**
- Rendering
- Device management
- Platform APIs

---

## Maintenance Guidelines

### Adding Features

1. **Game logic:** Add to GameEngine (if shared) or GeneralsMD (if ZH-only)
2. **Platform code:** Add to GameEngineDevice
3. **Utilities:** Add to Libraries

### Code Organization

- **GameEngine:** Business logic, no platform calls
- **GameEngineDevice:** Platform-specific code only
- **Libraries:** Reusable, well-tested utilities

---

## See Also

- [ENTITIES/PLATFORM.md](./PLATFORM.md) — Platform technologies
- [ENTITIES/GAME.md](./GAME.md) — Projects overview
- [ARCHITECTURE/LAYERS.md](../ARCHITECTURE/LAYERS.md) — Layer boundaries

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, architecture docs
