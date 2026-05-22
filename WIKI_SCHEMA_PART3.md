# Part 3: Wiki Schema - Operations and Indexing

## Operations

### Ingest

When adding a new source (code change, documentation, external reference):

1. **Read** the source carefully
2. **Extract** key information:
   - What changed? (for code)
   - Why did it change? (for decisions)
   - What does it connect to? (for cross-references)
3. **Update** relevant wiki pages:
   - Create new entity/page if needed
   - Revise existing pages with new information
   - Add cross-references
4. **Update** index.md and log.md
5. **Verify** by checking links from related pages

**Example flow:**
```
New FPS fix → Read fix → Update CONCEPTS/DETERMINISM.md 
           → Update ENTITIES/GAME.md → Update index.md 
           → Update log.md → Verify cross-links
```

### Query

When answering a question:

1. **Search** index.md for relevant categories
2. **Read** relevant pages in order:
   - Index page → Entity pages → Concept pages → Topic pages
3. **Synthesize** answer with citations
4. **Consider** filing the answer as a new page (e.g., [CHANGES/ISSUE-XXX.md])

**Example query flow:**
```
"How to fix FPS uncapped?" 
  → Search index → Find CONCEPTS/DETERMINISM.md 
  → Read page → Find reference to ISSUE-132 
  → Read CHANGES/ISSUE-132.md 
  → Synthesize answer with citations from both
```

### Lint

Periodically health-check the wiki:

1. **Contradictions**: Check for conflicting information across pages
2. **Stale claims**: Verify claims against newer sources
3. **Orphan pages**: Find pages with no inbound links
4. **Missing pages**: Identify concepts mentioned but lacking their own page
5. **Missing cross-references**: Find important links that are absent
6. **Data gaps**: Identify areas that could be filled with web research or code analysis

---

## Indexing and Logging

### index.md (Content-Oriented)

The main catalog of wiki content.

**Format:**

```
# GeneralsX Wiki Index

## Entities
- [PLATFORM.md](ENTITIES/PLATFORM.md) — SDL3, DXVK, OpenAL, FFmpeg
  > Date: 2026-05 | Sources: 12

## Concepts
- [RAG.md](CONCEPTS/RAG.md) — Platform isolation, determinism
  > Date: 2026-05 | Synthesis: Critical

## Changes
- [2026-05.md](CHANGES/2026-05.md) — May 2026 changes
```

**Updates required:**
- Update index.md on every ingest
- Add new pages with category and one-line summary
- Keep summaries concise and accurate

### log.md (Chronological)

Append-only record of wiki evolution.

**Format:**

```
## [2026-05-18] ingest | CI migrated to macOS + Flatpak

- Created: CONCEPTS/RAG.md (new)
- Updated: ENTITIES/PLATFORM.md
- Cross-linked: CHANGES/2026-05.md

## [2026-05-17] ingest | FPS uncapped fix (ISSUE-132)

- Created: CONCEPTS/DETERMINISM.md (new)
- Updated: ENTITIES/GAME.md
- Cross-linked: CHANGES/ISSUE-132.md

## [2026-05-11] ingest | Linux case-sensitive assets (ISSUE-128)

- Updated: ENTITIES/PLATFORM.md (SDL3 section)
- Cross-linked: CHANGES/ISSUE-128.md
```

**Parsing tip:**
Each entry starts with `## [YYYY-MM-DD] type | description`
Last 5 entries: `grep "^## \[" log.md | tail -5`

---

## Frontmatter Convention

For advanced querying (e.g., with Dataview), LLM should add YAML frontmatter:

```yaml
---
entity: platform
type: entity
updated: 2026-05-18
sources: 12
---
```

**Types:** entity, concept, topic, architecture, change

---

## LLM Maintenance Rules

### When to Create Pages

**Always create:**
- New entity (core component with significant documentation)
- New concept (architectural principle or rule)
- New change summary (important bugfix, feature, or decision)

**Consider creating:**
- New topic (repeated question that deserves its own page)
- New architecture page (new system design insight)

### When to Update Pages

**Always update:**
- New information that contradicts or extends existing content
- Missing cross-references to newly created pages
- Outdated dates and source counts

**Consider updating:**
- Related pages that could benefit from context
- Index page for better navigation

### When to Archive

**Move to docs/ETC/ when:**
- Entity is deprecated or removed
- Concept is superseded by a new one
- Change summary is no longer relevant

**Never delete** — archive and reference instead.

---

## Cross-Reference Rules

### Internal Cross-References

**Always add:**
1. Links from entity pages to related concepts
2. Links from concept pages to related entities
3. Links from change pages to affected entities/concepts
4. Links from topics to relevant entities and concepts

**Format:**

```
**See also:** [CONCEPTS/DETERMINISM.md](../CONCEPTS/DETERMINISM.md)
```

### External Cross-References

**Add when:**
- Referencing upstream PRs, issues, or commits
- Referencing external tools or resources
- Referencing related documentation in docs/

**Format:**

```
**Source:** TheSuperHackers PR #2670 (deterministic math)
```

---

## Verification Checklist

Before completing any ingest:

- [ ] Created/updated relevant pages
- [ ] Updated index.md with new/changed pages
- [ ] Appended to log.md
- [ ] Added cross-references
- [ ] Verified links from related pages
- [ ] Checked for contradictions
- [ ] Updated dates and source counts
- [ ] Added frontmatter if applicable

---

## See Also

- [WIKI_SCHEMA.md](./WIKI_SCHEMA.md) — Main schema document
- AGENTS.md — Project instructions
- docs/DEV_BLOG/YYYY-MM-DIARY.md — Official development diary
