# ENTITIES/GAME.md

## Summary

Game entities — GeneralsX and GeneralsXZH projects, scope, and vision.

---

## Projects Overview

### GeneralsX

**Original:** Command & Conquer: Generals (Zero Hour base game)

**Scope:** Base game port, core gameplay mechanics

**Features:**
- Basic unit types
- Standard buildings
- Original campaigns
- Basic AI

---

### GeneralsXZH

**Original:** Command & Conquer: Generals Zero Hour

**Scope:** Expansion port, complete feature set

**Features:**
- All base game content
- Zero Hour expansion content
- New unit types
- New buildings
- Extended campaigns
- Enhanced AI

---

## Architecture

```
GeneralsX
├── Core/GameEngine/           ← Shared between both games
├── Generals/                  ← Base game code
└── GeneralsMD/                ← Zero Hour code
    └── Code/GameEngine/       ← Zero Hour-specific logic
```

---

## Key Decisions

### Shared Core

**Why shared?**
- Reduce code duplication
- Easier maintenance
- Consistent behavior

**What's shared:**
- Base game logic
- Shared libraries
- Common utilities

**What's separate:**
- Zero Hour expansion logic
- Campaign-specific code
- Difficulty adjustments

---

## Scope & Vision

### Current Scope

**Completed:**
- ✅ Platform layer (SDL3, DXVK, OpenAL)
- ✅ Core game engine
- ✅ Basic gameplay mechanics
- ✅ Asset loading

**In Progress:**
- 🚧 Platform layer refinements
- 🚧 Advanced AI features
- 🚧 Enhanced replay system

### Future Vision

**Short-term:**
- Complete platform layer
- Full replay compatibility
- Enhanced AI

**Long-term:**
- Additional platforms
- Community contributions
- Ongoing maintenance

---

## See Also

- [ENTITIES/PLATFORM.md](./PLATFORM.md) — Platform technologies
- [ENTITIES/CORE.md](./CORE.md) — Shared components
- [CONCEPTS/RAG.md](../CONCEPTS/RAG.md) — Platform isolation

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, project documentation
