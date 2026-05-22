# WIKI_SCHEMA.md

## Purpose

This is the meta-schema that defines how to maintain the GeneralsX wiki. Read this first before contributing.

---

## Quick Start

1. **New page:** Copy from `WIKI_PAGES/README.md` → Paste to new file → Fill content
2. **Update:** Read file → Make changes → Commit with schema-compliant message
3. **Delete:** Update `README.md` index → Delete file → Document reason

---

## Page Categories

| Category | Purpose | Naming Convention |
|----------|---------|-------------------|
| **Entities** | Core components (games, platform, core, references) | `ENTITIES/NAME.md` |
| **Concepts** | Design principles (RAG, porting, retail, determinism) | `CONCEPTS/NAME.md` |
| **Topics** | Operational guides (build, testing, branching, debugging) | `TOPICS/NAME.md` |
| **Architecture** | System-level views (layers, flow, data) | `ARCHITECTURE/NAME.md` |
| **Changes** | Chronological change logs (monthly) | `CHANGES/YYYY-MM.md` |

---

## Page Formats

### Standard Page Structure

```
# TITLE.md

## Summary

One-liner describing the page content.

---

## [SECTION] - Detailed content

- Use **terse bullets**, not prose paragraphs
- Include **code blocks** for commands/examples
- Use **tables** for comparisons/lists
- Reference **related pages** with `[link](./LINK.md)

---

## See Also

- [Related page 1](./LINK.md) — Context
- [Related page 2](./LINK.md) — Extension

---

**Last updated**: YYYY-MM-DD | **Sources**: AGENTS.md, documentation
```

### Entity Page Format

```
# ENTITIES/NAME.md

## Summary

Brief description of the entity.

---

## Purpose

What problem does this entity solve?

---

## Responsibilities

- List key responsibilities
- Use bullet points for clarity

---

## Location

```
┌─────────────────────────────────────────┐
│         Entity Overview                │
│  - File structure                    │
│  - Key interfaces                    │
│  - Dependencies                      │
└─────────────────────────────────────────┘
```

---

## Platform Considerations

| Platform | Status | Notes |
|----------|--------|-------|
| Linux | Active | Standard implementation |
| macOS | Active | Standard implementation |
| Windows | N/A | Upstream only |

---

## See Also

- [Related entity](../ENTITIES/NAME.md)

---

**Last updated**: YYYY-MM-DD | **Sources**: AGENTS.md, architecture docs
```

### Concept Page Format

```
# CONCEPTS/NAME.md

## Summary

Brief description of the concept.

---

## Purpose

Why does this concept matter?

---

## Key Principles

- **Principle 1:** Description
- **Principle 2:** Description

---

## Implementation Rules

### Rule 1

**When to apply:**
- Condition A
- Condition B

**How to implement:**
```
code or command example
```

---

## Common Pitfalls

- **Pitfall 1:** What happens if X is not followed
- **Pitfall 2:** What breaks if Y is violated

---

## See Also

- [Related concept](../CONCEPTS/NAME.md)

---

**Last updated**: YYYY-MM-DD | **Sources**: AGENTS.md, design docs
```

### Topic Page Format

```
# TOPICS/NAME.md

## Summary

Brief description of the topic.

---

## Prerequisites

- [ ] Tool/dependency 1
- [ ] Tool/dependency 2

---

## Step-by-Step Guide

### Step 1: Preparation

1. **Setup:** Command or action
2. **Verify:** How to confirm success

### Step 2: Execution

1. **Command:** Exact command to run
2. **Output:** Expected output

### Step 3: Verification

1. **Check:** What to look for
2. **Troubleshoot:** Common issues

---

## Platform-Specific Notes

| Platform | Command/Note |
|----------|--------------|
| Linux | Command or note |
| macOS | Command or note |

---

## See Also

- [Related topic](../TOPICS/NAME.md)

---

**Last updated**: YYYY-MM-DD | **Sources**: AGENTS.md, operational docs
```

### Changes Page Format

```
# CHANGES/YYYY-MM.md

## Summary

Month summary — a chronological record of all wiki-relevant changes.

---

## Overview

Brief paragraph summarizing the month's focus areas.

---

## Detailed Changes

### YYYY-MM-DD

#### [Category]

**Changes:**
1. Change 1
2. Change 2

**Wiki Updates:**
- Updated `PAGE1.md` — Reason
- Updated `PAGE2.md` — Reason

**Related:**
- Dev blog: YYYY-MM-DD [title]

---

## Summary Statistics

| Category | Count |
|----------|-------|
| New pages | X |
| Updated pages | X |

---

## See Also

- [LATEST.md](../LATEST.md) — Change log
- [CHANGES/YYYY-MM+1.md](./CHANGES/YYYY-MM+1.md) — Next month

---

**Last updated**: YYYY-MM-DD | **Sources**: Dev blog, commit history
```

### Architecture Page Format

```
# ARCHITECTURE/NAME.md

## Summary

Brief description of the architectural view.

---

## [Diagram Title]

```
┌─────────────────────────────────────────┐
│         Architectural View             │
│  - Visual representation              │
│  - Data flow                          │
└─────────────────────────────────────────┘
```

---

## [Section] - Detailed explanation

- Use terse bullets
- Include relevant code snippets
- Reference related pages

---

## See Also

- [Related architecture page](./NAME.md)

---

**Last updated**: YYYY-MM-DD | **Sources**: Architecture docs
```

---

## Cross-Reference Rules

### Internal Links

- Use relative paths: `[page](./LINK.md)`
- Use descriptive anchor text
- Link to **concepts** and **architecture** more often than **entities**

### External References

- Dev blog: `docs/DEV_BLOG/YYYY-MM-DIARY.md`
- AGENTS.md: `.github/instructions/generalsx.instructions.md`
- Upstream: `thesuperhackers/GeneralsGameCode`

---

## Attribution

Always include at the bottom:

```
**Last updated**: YYYY-MM-DD | **Sources**: AGENTS.md, documentation
```

---

## See Also

- [README.md](./README.md) — Wiki index
- [LATEST.md](./LATEST.md) — Change log
- [WIKI_SCHEMA_PART2.md](./WIKI_SCHEMA_PART2.md) — Page formats
- [WIKI_SCHEMA_PART3.md](./WIKI_SCHEMA_PART3.md) — Operations guide

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, wiki schema design
