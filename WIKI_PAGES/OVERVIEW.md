# WIKI_PAGES/OVERVIEW.md

## Summary

Wiki Overview — comprehensive guide to navigating and understanding the GeneralsX wiki.

---

## Quick Reference

### Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    WIKI STRUCTURE                            │
├─────────────────────────────────────────────────────────────┤
│  ROOT                                                            │
│  ├── SCHEMA/                          ← How to use this wiki  │
│  ├── ENTITIES/                        ← What GeneralsX is     │
│  │   ├── PLATFORM.md                  ← Technologies used     │
│  │   ├── GAME.md                      ← Projects & scope      │
│  │   ├── CORE.md                      ← Shared components     │
│  │   └── REFERENCES.md                ← External resources    │
│  ├── CONCEPTS/                       ← How GeneralsX works   │
│  │   ├── RAG.md                       ← Platform isolation    │
│  │   ├── PORTING.md                   ← Platform migrations  │
│  │   ├── RETAIL.md                    ← Compatibility        │
│  │   └── DETERMINISM.md               ← FPS, math rules      │
│  ├── TOPICS/                          ← How-to guides         │
│  │   ├── BUILD.md                     ← Building instructions│
│  │   ├── TESTING.md                   ← Test protocol        │
│  │   ├── DEBUGGING.md                 ← Troubleshooting      │
│  │   ├── BRANCHING.md                 ← Git workflow         │
│  │   └── ARCHITECTURE.md              ← Design patterns      │
│  ├── ARCHITECTURE/                    ← System design         │
│  │   ├── LAYERS.md                    ← Layer boundaries     │
│  │   ├── FLOW.md                      ← System lifecycle     │
│  │   └── DATA.md                      ← Data handling        │
│  ├── CHANGES/                         ← What changed recently │
│  │   └── 2026-05.md                   ← May 2026 updates      │
│  └── SUMMARY/                         ← Quick navigation      │
│      ├── OVERVIEW.md                    ← You are here        │
│      └── LATEST.md                      ← Recent activity     │
└─────────────────────────────────────────────────────────────┘
```

---

## Reading Guide

### For Newcomers

1. **Start with OVERVIEW.md** — Understand wiki structure
2. **Read ENTITIES/PLATFORM.md** — Learn platform technologies
3. **Review CONCEPTS/RAG.md** — Understand platform isolation
4. **Follow ARCHITECTURE/LAYERS.md** — Learn system architecture
5. **Check TOPICS/BUILD.md** — Get started building

### For Contributors

1. **Read SCHEMA/WIKI_SCHEMA.md** — Understand wiki structure
2. **Review TOPICS/BRANCHING.md** — Learn contribution workflow
3. **Check TOPICS/TESTING.md** — Understand testing protocol
4. **Follow CONCEPTS/RETAIL.md** — Know compatibility requirements

### For Maintainers

1. **Review SCHEMA/WIKI_SCHEMA.md** — Understand wiki structure
2. **Check CHANGES/2026-05.md** — Track recent changes
3. **Monitor CONCEPTS/PORTING.md** — Platform migration status
4. **Review ARCHITECTURE/FLOW.md** — System lifecycle changes

---

## Page Categories Explained

### ENTITIES

**What GeneralsX is** — Concrete components and technologies

- **PLATFORM.md** — SDL3, DXVK, OpenAL, FFmpeg, Bink
- **GAME.md** — GeneralsX, GeneralsXZH, scope and vision
- **CORE.md** — GameEngine, GameEngineDevice, Libraries
- **REFERENCES.md** — fighter19, jmarshall, thesuperhackers

---

### CONCEPTS

**How GeneralsX works** — Principles and abstractions

- **RAG.md** — Platform Isolation (RAG) — never call native APIs
- **PORTING.md** — DX8→Vulkan, Miles→OpenAL, platform migrations
- **RETAIL.md** — Retail Compatibility — replays, mods, determinism
- **DETERMINISM.md** — FPS caps, math consistency, predictability

---

### TOPICS

**How-to guides** — Practical procedures and best practices

- **BUILD.md** — Creating and testing builds
- **TESTING.md** — Smoke, replay, regression tests
- **DEBUGGING.md** — GDB, procedures, common issues
- **BRANCHING.md** — Git workflow, upstream sync
- **ARCHITECTURE.md** — Design patterns, DO/DON'T examples

---

### ARCHITECTURE

**System design** — How components fit together

- **LAYERS.md** — Layer boundaries, responsibilities, rules
- **FLOW.md** — Boot, render, shutdown sequences
- **DATA.md** — Serialization, memory, platform issues

---

### CHANGES

**Recent updates** — Monthly change logs

- **2026-05.md** — May 2026 changes
- Future: **2026-06.md**, **2026-07.md**, etc.

---

## Navigation Tips

### Finding Information

1. **Platform technologies?** → ENTITIES/PLATFORM.md
2. **How to build?** → TOPICS/BUILD.md
3. **Platform issues?** → CONCEPTS/RETAIL.md
4. **Git workflow?** → TOPICS/BRANCHING.md
5. **System design?** → ARCHITECTURE/LAYERS.md

### Cross-Reference Pattern

Each page includes "See Also" section at bottom for quick navigation.

---

## Wiki Maintenance

### Adding New Pages

1. **Choose category** — ENTITY, CONCEPT, TOPIC, ARCHITECTURE, or CHANGE
2. **Use naming convention** — `CATEGORY/NAME.md` (e.g., `CONCEPTS/NEW_FEATURE.md`)
3. **Follow schema** — Include summary, purpose, key principles
4. **Add cross-references** — Link to related pages
5. **Update OVERVIEW.md** — Add to appropriate section

### Updating Existing Pages

1. **Preserve format** — Keep headings, tables, code blocks
2. **Update dates** — Change "Last updated" timestamp
3. **Maintain structure** — Don't remove key sections
4. **Add sources** — Reference AGENTS.md, documentation

---

## Quick Stats

| Category | Pages | Focus |
|----------|-------|-------|
| Entities | 4 | What GeneralsX is |
| Concepts | 4 | How GeneralsX works |
| Topics | 5 | How-to guides |
| Architecture | 3 | System design |
| Changes | 1 | Recent updates |
| **Total** | **17** | Complete wiki |

---

**Last updated**: 2026-05-18 | **Sources**: Wiki schema, page contents
