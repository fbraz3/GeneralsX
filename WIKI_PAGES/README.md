# GeneralsX Wiki Index

This is the main catalog of the GeneralsX wiki — an interlinked knowledge base that grows with every change, discovery, and question.

---

## Overview

GeneralsX is a cross-platform port of Command & Conquer: Generals Zero Hour, porting legacy Windows DirectX 8 + Miles Sound code to a modern stack (SDL3 + DXVK + OpenAL + 64-bit).

### Key Principles

1. **Platform Isolation** — No native platform calls in game logic; SDL3, DXVK, OpenAL, FFmpeg handle all platform I/O
2. **Determinism** — Rendering/audio changes must not affect gameplay logic; FPS caps, math consistency
3. **Retail Compatibility** — Original replays and mods must work; debug options break replays
4. **Backport Rules** — Platform code goes to both games; expansion-specific logic stays in Zero Hour

---

## Entities (Core Project Components)

| Entity | Description | Last Updated |
|--------|-------------|--------------|
| [PLATFORM.md](./ENTITIES/PLATFORM.md) | SDL3, DXVK, OpenAL, FFmpeg — platform technologies | 2026-05-18 |
| [GAME.md](./ENTITIES/GAME.md) | Generals, Zero Hour, GeneralsMD — game entities | 2026-05-17 |
| [CORE.md](./ENTITIES/CORE.md) | Shared libraries, backport rules | 2026-05-11 |
| [REFERENCES.md](./ENTITIES/REFERENCES.md) | fighter19, jmarshall, thesuperhackers | 2026-05-07 |

---

## Concepts (Architectural Principles)

| Concept | Description | Last Updated |
|---------|-------------|--------------|
| [RAG.md](./CONCEPTS/RAG.md) | Platform isolation, determinism | 2026-05-18 |
| [PORTING.md](./CONCEPTS/PORTING.md) | DX8→Vulkan, Miles→OpenAL | 2026-05-17 |
| [RETAIL.md](./CONCEPTS/RETAIL.md) | Compatibility, replay support | 2026-05-11 |
| [DETERMINISM.md](./CONCEPTS/DETERMINISM.md) | FPS caps, math consistency | 2026-05-11 |

---

## Topics (Procedural Knowledge)

| Topic | Description | Last Updated |
|-------|-------------|--------------|
| [BUILD.md](./TOPICS/BUILD.md) | Presets, CI/CD, deployment | 2026-05-18 |
| [TESTING.md](./TOPICS/TESTING.md) | Headless, smoke tests, QA | 2026-05-17 |
| [BRANCHING.md](./TOPICS/BRANCHING.md) | Sync strategy, conflict resolution | 2026-05-07 |
| [DEBUGGING.md](./TOPICS/DEBUGGING.md) | Diagnostics, logToCon, GDB | 2026-05-18 |

---

## Architecture (System Design)

| Page | Description | Last Updated |
|------|-------------|--------------|
| [LAYERS.md](./ARCHITECTURE/LAYERS.md) | Platform, GameLogic, Shared boundaries | 2026-05-18 |
| [FLOW.md](./ARCHITECTURE/FLOW.md) | Bootstrap, render loop, shutdown | 2026-05-17 |
| [DATA.md](./ARCHITECTURE/DATA.md) | VFS, INI, assets, replays | 2026-05-17 |

---

## Changes (Recent History)

| Change | Description | Last Updated |
|--------|-------------|--------------|
| [2026-05.md](./CHANGES/2026-05.md) | May 2026 changes summary | 2026-05-18 |
| [ISSUE-132.md](./CHANGES/ISSUE-132.md) | FPS uncapped fix | 2026-05-11 |
| [ISSUE-128.md](./CHANGES/ISSUE-128.md) | Linux case-sensitive assets | 2026-05-11 |
| [ISSUE-131.md](./CHANGES/ISSUE-131.md) | macOS transparency | 2026-05-10 |

---

## Quick Reference

| Query Type | Look Here |
|------------|-----------|
| "What is SDL3's role?" | [ENTITIES/PLATFORM.md](./ENTITIES/PLATFORM.md) |
| "How to build on Linux" | [TOPICS/BUILD.md](./TOPICS/BUILD.md) |
| "Backport rules" | [ENTITIES/CORE.md](./ENTITIES/CORE.md) |
| "FPS uncapped fix" | [CHANGES/ISSUE-132.md](./CHANGES/ISSUE-132.md) |
| "Platform isolation" | [CONCEPTS/RAG.md](./CONCEPTS/RAG.md) |
| "DX8→Vulkan" | [CONCEPTS/PORTING.md](./CONCEPTS/PORTING.md) |
| "Shared library rules" | [ENTITIES/CORE.md](./ENTITIES/CORE.md) |
| "Debug logging" | [TOPICS/DEBUGGING.md](./TOPICS/DEBUGGING.md) |

---

## Navigation

### For New Topics
- Adding sources: See [WIKI_SCHEMA.md](./WIKI_SCHEMA.md)
- Querying: Search index.md → Read relevant pages → Synthesize
- Maintaining: Run lint periodically; check for contradictions, orphans, gaps

### For Common Questions

**Building:**
- Linux: [TOPICS/BUILD.md](./TOPICS/BUILD.md)
- macOS: [TOPICS/BUILD.md](./TOPICS/BUILD.md)
- Presets: [ENTITIES/PLATFORM.md](./ENTITIES/PLATFORM.md)

**Architecture:**
- Platform isolation: [CONCEPTS/RAG.md](./CONCEPTS/RAG.md)
- Layer boundaries: [ARCHITECTURE/LAYERS.md](./ARCHITECTURE/LAYERS.md)
- Platform technologies: [ENTITIES/PLATFORM.md](./ENTITIES/PLATFORM.md)

**Development:**
- Branching/sync: [TOPICS/BRANCHING.md](./TOPICS/BRANCHING.md)
- Testing: [TOPICS/TESTING.md](./TOPICS/TESTING.md)
- Debugging: [TOPICS/DEBUGGING.md](./TOPICS/DEBUGGING.md)

**Reference:**
- Project instructions: [AGENTS.md](./AGENTS.md)
- Dev blog: `docs/DEV_BLOG/YYYY-MM-DIARY.md`
- Active work: `docs/WORKDIR/`

---

## Statistics

| Metric | Count |
|--------|-------|
| Total pages | 17 |
| Entities | 4 |
| Concepts | 4 |
| Topics | 4 |
| Architecture | 3 |
| Changes | 4 |

---

## See Also

- [WIKI_SCHEMA.md](./WIKI_SCHEMA.md) — Wiki meta-schema
- [CONCEPTS/RAG.md](./CONCEPTS/RAG.md) — Platform isolation principles
- [CONCEPTS/DETERMINISM.md](./CONCEPTS/DETERMINISM.md) — Determinism principles
- [TOPICS/BUILD.md](./TOPICS/BUILD.md) — Build instructions
