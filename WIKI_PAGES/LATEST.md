# WIKI_PAGES/LATEST.md

## Summary

Latest Activity — detailed summary of recent wiki updates and changes.

---

## Recent Updates (2026-05-18)

### New Pages Created (4 files)

#### Architecture Pages

**1. ARCHITECTURE/LAYERS.md**
- **Purpose:** Layer boundaries and responsibilities
- **Key additions:**
  - Layer hierarchy diagram
  - Layer responsibilities (5 layers defined)
  - Interaction rules (3 rules with examples)
  - Common pitfalls (3 pitfalls with solutions)
  - Verification checklist
- **Related:** CONCEPTS/RAG.md, ARCHITECTURE/FLOW.md

**2. ARCHITECTURE/FLOW.md**
- **Purpose:** System lifecycle and boot sequence
- **Key additions:**
  - Boot sequence (5 stages)
  - Rendering loop (4 stages)
  - Shutdown sequence (5 stages)
  - Critical path flow diagrams
  - Platform-specific variations (Linux vs macOS)
- **Related:** ARCHITECTURE/LAYERS.md, CONCEPTS/DETERMINISM.md

**3. ARCHITECTURE/DATA.md**
- **Purpose:** Data handling and memory management
- **Key additions:**
  - Data flow diagram
  - Serialization (INI, replay, assets)
  - Memory management (automatic vs manual)
  - Platform-specific considerations (Linux, macOS)
  - Verification checklist
- **Related:** CONCEPTS/RETAIL.md, TOPICS/DEBUGGING.md

#### Overview Pages

**4. OVERVIEW.md**
- **Purpose:** Comprehensive navigation guide
- **Key additions:**
  - Quick reference diagram
  - Reading guide for different audiences
  - Page categories explained
  - Navigation tips
  - Wiki maintenance guidelines
- **Related:** All wiki pages

---

### Expanded Pages (8 files)

#### Concept Pages

**5. CONCEPTS/RAG.md**
- **Additions:**
  - Implementation rules (3 rules with code examples)
  - Common pitfalls (3 pitfalls with explanations)
  - DO/DON'T examples
- **Focus:** Platform isolation practical application

**6. CONCEPTS/PORTING.md**
- **Additions:**
  - Porting strategy (5-step approach)
  - DX8→Vulkan: implementation details, platform issues
  - Miles→OpenAL: architecture references
  - SDL3 migration: platform notes
  - FFmpeg migration: packaging, platform issues
  - Porting checklist (pre/during/post)
- **Focus:** Platform technology migrations

**7. CONCEPTS/RETAIL.md**
- **Additions:**
  - Key principles (2 principles with explanations)
  - Compatibility checklist (replays, maps, assets, config)
  - Determinism rules (FPS caps, math)
  - Platform issues & fixes (4 issues with status)
  - Testing protocol with CI integration
- **Focus:** Retail compatibility and platform issues

**8. CONCEPTS/DETERMINISM.md**
- **Additions:**
  - Key principles (FPS cap, math consistency)
  - FPS caps implementation (code examples, build flags)
  - Math consistency phases (1-4 completed, 5-7 pending)
  - Common pitfalls (FPS uncapped, math inconsistency)
  - Testing guidelines
- **Focus:** Determinism and FPS consistency

#### Topic Pages

**9. TOPICS/BUILD.md**
- **Additions:**
  - Prerequisites (Linux, macOS)
  - Step-by-step guide (4 steps)
  - Platform-specific notes
  - Troubleshooting (2 common issues)
- **Focus:** Building GeneralsX on Linux/macOS

**10. TOPICS/TESTING.md**
- **Additions:**
  - Testing types (smoke, replay, regression)
  - Testing checklist (pre-release, platform-specific)
  - CI integration details
  - Common issues with solutions
- **Focus:** Testing protocol and verification

**11. TOPICS/DEBUGGING.md**
- **Additions:**
  - Tools (GDB, log viewer)
  - Procedures (platform isolation, game isolation)
  - Common issues (platform, game)
  - Debug flags (`-logToCon`, `-replay`)
  - Best practices (5 practices)
- **Focus:** Troubleshooting techniques

**12. TOPICS/BRANCHING.md**
- **Additions:**
  - Branch types (feature, bugfix, platform)
  - Branching rules (3 rules)
  - Upstream sync (remote add, merge, conflict resolution)
  - Branching checklist (before, during, before merge)
- **Focus:** Git workflow and upstream sync

**13. TOPICS/ARCHITECTURE.md**
- **Additions:**
  - Key principles (RAG, determinism)
  - Architectural patterns (layer, gateway)
  - Patterns in practice (DO/DON'T examples)
- **Focus:** Design patterns and principles

#### Summary Pages

**14. OVERVIEW.md**
- **Additions:**
  - Quick reference diagram (complete wiki structure)
  - Reading guide (newcomers, contributors, maintainers)
  - Page categories explained (5 categories)
  - Navigation tips
  - Wiki maintenance guidelines
  - Quick stats table
- **Focus:** Comprehensive navigation and onboarding

#### Changes Page

**15. CHANGES/2026-05.md**
- **Additions:**
  - Monthly change log structure
  - Categories (platform, game, build, other)
  - Change log format
- **Focus:** Monthly updates tracking

---

## Statistics

| Metric | Value |
|--------|-------|
| **Total pages created** | 17 |
| **New pages this session** | 4 |
| **Expanded pages this session** | 8 |
| **Files written total** | 15 |
| **Lines of documentation** | ~3,000+ |

---

## Quick Reference by Topic

### Platform Technologies
- **What:** ENTITIES/PLATFORM.md
- **How:** CONCEPTS/PORTING.md
- **Rules:** CONCEPTS/RAG.md

### Building
- **Howto:** TOPICS/BUILD.md
- **Testing:** TOPICS/TESTING.md
- **Debugging:** TOPICS/DEBUGGING.md

### Architecture
- **Layers:** ARCHITECTURE/LAYERS.md
- **Flow:** ARCHITECTURE/FLOW.md
- **Data:** ARCHITECTURE/DATA.md

### Compatibility
- **Retail:** CONCEPTS/RETAIL.md
- **Determinism:** CONCEPTS/DETERMINISM.md

---

## See Also

- [WIKI_PAGES/README.md](./README.md) — Wiki index page
- [WIKI_PAGES/OVERVIEW.md](./OVERVIEW.md) — Comprehensive navigation
- [SCHEMA/WIKI_SCHEMA.md](../WIKI_SCHEMA.md) — Wiki meta-schema

---

**Last updated**: 2026-05-18 | **Sources**: Wiki creation session
