# GeneralsX Wiki Schema

This document defines the **wiki schema** — the key configuration file that tells the LLM how to operate as a disciplined wiki maintainer for GeneralsX.

---

## Overview

GeneralsX is a cross-platform port of Command & Conquer: Generals Zero Hour, porting legacy Windows DirectX 8 + Miles Sound code to a modern stack (SDL3 + DXVK + OpenAL + 64-bit). The codebase is massive (~500k LOC) and spans multiple games, shared libraries, and platform layers.

The wiki is a **persistent, compounding artifact** — an interlinked collection of markdown files that grows richer with every change, every discovery, and every question.

---

## Directory Structure

```
GeneralsX/
├── AGENTS.md                    ← Main project instructions
├── WIKI_SCHEMA.md               ← This file (wiki meta-schema)
├── WIKI_PAGES/                  ← The wiki itself (LLM-maintained)
│   ├── README.md                ← Index/catalog of all wiki pages
│   ├── LATEST.md                ← Chronological log of wiki changes
│   ├── OVERVIEW.md              ← Project synthesis & high-level summary
│   ├── ENTITIES/
│   │   ├── PLATFORM.md          ← SDL3, DXVK, OpenAL, FFmpeg
│   │   ├── GAME.md              ← Generals, Zero Hour, GeneralsMD
│   │   ├── CORE.md              ← Shared libraries, backport rules
│   │   └── REFERENCES.md        ← fighter19, jmarshall, thesuperhackers
│   ├── CONCEPTS/
│   │   ├── RAG.md               ← Platform isolation, determinism
│   │   ├── PORTING.md           ← DX8→Vulkan, Miles→OpenAL
│   │   ├── RETAIL.md            ← Compatibility, replay support
│   │   └── DETERMINISM.md       ← FPS caps, math consistency
│   ├── TOPICS/
│   │   ├── BUILD.md             ← Presets, CI/CD, deployment
│   │   ├── TESTING.md           ← Headless, smoke tests, QA
│   │   ├── BRANCHING.md         ← Sync strategy, conflict resolution
│   │   └── DEBUGGING.md         ← Diagnostics, logToCon, GDB
│   ├── ARCHITECTURE/
│   │   ├── LAYERS.md            ← Platform, GameLogic, Shared
│   │   ├── FLOW.md              ← Bootstrap, render loop, shutdown
│   │   └── DATA.md              ← VFS, INI, assets, replays
│   └── CHANGES/
│       ├── 2026-05.md            ← Recent changes (auto-linked)
│       ├── ISSUE-132.md          ← FPS uncapped fix
│       ├── ISSUE-128.md          ← Linux case-sensitive assets
│       └── ISSUE-131.md          ← macOS transparency
├── docs/DEV_BLOG/               ← Official dev diary (source of truth)
├── docs/WORKDIR/                ← Active work (LLM should index)
├── docs/ETC/                    ← Reference materials (LLM should archive)
└── docs/BUILD/                  ← Platform build docs (LLM should maintain)
```
