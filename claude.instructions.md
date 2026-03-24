---
applyTo: '**'
---

# GeneralsX: Instructions for AI Coding Agents (Claude)

This file ensures that Claude Code, Claude in VS Code, and other AI agents working on GeneralsX read the core project conventions before implementing changes.

**⚠️ BEFORE YOU START**: Read the core instruction files in order:

1. **[`.github/copilot-instructions.md`](.github/copilot-instructions.md)** — Quick reference (5 min read)
   - Project scope and structure
   - Build presets and testing hotspots
   - Common pitfalls to avoid
   - Where to tweak build flags

2. **[`.github/instructions/generalsx.instructions.md`](.github/instructions/generalsx.instructions.md)** — Core strategy document (CRITICAL)
   - Project mission and golden rules
   - Platform strategy (SDL3 + DXVK + OpenAL)
   - Phase structure and acceptance criteria
   - Reference repositories and how to use them
   - Dev environment and directory structure

3. **[`.github/instructions/git-commit.instructions.md`](.github/instructions/git-commit.instructions.md)** — Commit message standards
   - Conventional Commits format adapted for GeneralsX
   - Type, scope, description guidelines
   - When to squash/amend commits
   - Examples of well-formed commits

4. **[`.github/instructions/docs.instructions.md`](.github/instructions/docs.instructions.md)** — Documentation guidelines
   - Dev blog updates required before commits
   - Phase checklist management
   - Where to place active work docs vs. reference materials

5. **[`.github/instructions/scripts.instructions.md`](.github/instructions/scripts.instructions.md)** — Script organization rules
   - Where to place new build/test scripts
   - Naming conventions
   - Standard shell practices

---

## TL;DR — Critical Points

### Code Changes
- **Every user-facing code change** needs a comment: `// GeneralsX @keyword author DD/MM/YYYY Description`
  - Keywords: `@bugfix` / `@feature` / `@performance` / `@refactor` / `@tweak` / `@build`
- **No band-aids**: Root cause fixes only, no empty stubs, no lazy workarounds
- **Keep determinism**: Never break gameplay logic when touching rendering/audio
- **English only**: All code, comments, identifiers in English

### Platform Isolation (CRITICAL)
- **No native platform code in game logic** (GameLogic, GameEngine gameplay)
- **Isolate platform code** to `Core/GameEngineDevice/` and `Core/Libraries/Source/Platform/`
- **SDL3 is the unified abstraction** — no Win32, Cocoa, or POSIX calls directly in game code
- **Keep legacy paths working** (DX8, Miles Sound System) behind `#ifdef` for VC6 baseline

### Before Committing
- Follow [`.github/instructions/git-commit.instructions.md`](.github/instructions/git-commit.instructions.md) for commit message format (Conventional Commits)
- Update [development diary](docs/DEV_BLOG/) (create `docs/DEV_BLOG/YYYY-MM-DIARY.md` if needed)
- Follow `.github/instructions/docs.instructions.md` for diary format
- Mark completed checklist items `[x]` in phase docs

### Build & Test
- **Linux**: `./scripts/docker-build-linux-zh.sh linux64-deploy`
- **macOS**: `./scripts/build-macos-zh.sh`
- **Windows (VC6 baseline)**: `cmake --preset vc6` then `cmake --build build/vc6`
- **Smoke test**: `./scripts/qa/smoke/docker-smoke-test-zh.sh linux64-deploy`

---

## Quick Reference by Task

### I'm implementing a graphics feature
→ Read: `generalsx.instructions.md` (Platform Strategy, Phase 1)
→ Reference: `references/fighter19-dxvk-port/` (DXVK patterns)
→ Build: `cmake --preset macos-vulkan` (macOS) or Linux Docker preset

### I'm implementing audio/sound
→ Read: `generalsx.instructions.md` (Platform Strategy, Phase 2)
→ Reference: `references/jmarshall-win64-modern/Code/Audio/` (adapt for Zero Hour!)
→ Remember: Replace Miles Sound System with OpenAL behind feature flags

### I'm adding a script
→ Read: `.github/instructions/scripts.instructions.md` (structure + naming)
→ Place in: `scripts/build/`, `scripts/env/`, `scripts/qa/`, etc. (not root)
→ Follow: Bash standards (`set -e`, proper error handling)

### I'm updating documentation
→ Read: `.github/instructions/docs.instructions.md` (organization rules)
→ Dev diary: `docs/DEV_BLOG/YYYY-MM-DIARY.md` (newest-first entries)
→ Active work: `docs/WORKDIR/` (phases, planning, reports, support, audit, lessons)
→ Never: Drop docs in root `/docs/` folder directly

### I'm fixing a bug
→ **Find the root cause** — no empty try-catch blocks or disabling features
→ Keep VC6 builds passing (regression test)
→ Add comment: `// GeneralsX @bugfix author DD/MM/YYYY ...`
→ Update dev diary before commit

---

## When You Get Stuck

**Is my change breaking the build?**
→ Check `CMakePresets.json` for active presets
→ Check `cmake/config-build.cmake` for build flags
→ Replay determinism check: Did I touch game logic from a graphics/audio change? **Don't do that.**

**Should I backport to Generals (base game)?**
→ YES if: Platform/backend code, shared Core libraries, low-risk
→ NO if: Zero Hour-specific gameplay, risky refactor

**Which reference repo shows the pattern I need?**
→ **fighter19** (DXVK/graphics/SDL3/CMake) or **jmarshall** (OpenAL/audio/modernization) or **TheSuperHackers** (upstream baseline)
→ See `generalsx.instructions.md` sections for specific guidance per reference

**Where's the dev diary?**
→ `docs/DEV_BLOG/YYYY-MM-DIARY.md` (create if missing, newest-first)
→ Update before every commit

---

## Conventions You'll See in Code

```cpp
// GeneralsX @feature fbraz 24/03/2026 Add SDL3 event loop integration
void Engine::UpdateInput() {
    // Cross-platform input via SDL3
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // ... handle event
    }
}
```

**Format**: `// GeneralsX @keyword author DD/MM/YYYY Description`
- Always above the changed code or function
- Keywords guide future reviewers on change type
- Essential for commit clarity

---

## Golden Rules (Never Compromise)

1. **SINGLE CODEBASE** — Linux, macOS, Windows build from same source
2. **SDL3 EVERYWHERE** — Platform abstraction layer for all windowing/input
3. **DXVK EVERYWHERE** — DirectX 8 to Vulkan translation on all platforms
4. **OPENAL EVERYWHERE** — Cross-platform audio (replaces Miles)
5. **DETERMINISM** — Rendering/audio changes must NEVER affect gameplay logic
6. **RETAIL COMPATIBILITY** — Original game files, replays, mods must work
7. **ZERO HOUR FIRST** — Expansion is primary target; backport to Generals carefully

---

## Getting Help

- **Graphics/DXVK**: See `references/fighter19-dxvk-port/` + `generalsx.instructions.md` Phase 1
- **Audio/OpenAL**: See `references/jmarshall-win64-modern/Code/Audio/` + `generalsx.instructions.md` Phase 2
- **Upstream changes**: See `generalsx.instructions.md` Merge From TheSuperHackers section
- **Documentation**: See `.github/instructions/docs.instructions.md`
- **Scripts**: See `.github/instructions/scripts.instructions.md`

---

**Last updated**: 24 March 2026
**For questions or clarifications**: Consult the referenced instruction files or submit an issue.
