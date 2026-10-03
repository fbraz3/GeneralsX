# WIKI_SCHEMA_PART3.md

## Operations Guide

Detailed procedures for creating, updating, and maintaining the wiki.

---

## Creating New Pages

### 1. Determine Page Type

| Question | Answer → Category |
|----------|-------------------|
| Is it a core component (game, platform, core, reference)? | **Entities** |
| Is it a design principle or rule? | **Concepts** |
| Is it a how-to guide or procedure? | **Topics** |
| Is it a system-level view? | **Architecture** |
| Is it a change log? | **Changes** |

### 2. Choose Naming Convention

```
CATEGORY/NAME.md
```

Examples:
- `ENTITIES/PLATFORM.md` — Platform technologies
- `CONCEPTS/RAG.md` — Platform isolation
- `TOPICS/BUILD.md` — Build instructions
- `ARCHITECTURE/LAYERS.md` — Layer boundaries

### 3. Copy Template

Copy from `WIKI_PAGES/README.md` → Paste to new file → Fill content

### 4. Follow Page Format

Refer to `WIKI_SCHEMA_PART2.md` for detailed templates:
- Entity page format
- Concept page format  
- Topic page format
- Changes page format
- Architecture page format

### 5. Add Cross-References

- Link to related pages using `[text](./LINK.md)`
- Add to `README.md` index
- Include "See Also" section

### 6. Verify Schema Compliance

- [ ] Title is clear and descriptive
- [ ] Summary is concise (1-2 sentences)
- [ ] Follows appropriate page format
- [ ] Includes cross-references
- [ ] Has attribution line
- [ ] Added to `README.md` index

---

## Updating Existing Pages

### Standard Update Process

1. **Read current content** — Understand existing structure
2. **Plan changes** — What needs to be added/modified
3. **Make changes** — Follow schema rules
4. **Verify** — Check schema compliance
5. **Commit** — Use schema-compliant message

### Content Updates

#### Adding New Section

```markdown
## [NEW SECTION] - Description

- Use terse bullets
- Include relevant examples
- Reference related pages
```

#### Updating Existing Content

- Keep existing structure intact
- Update timestamps in attribution
- Maintain cross-reference integrity

#### Removing Content

- Preserve context with brief explanation
- Update related page cross-references
- Add to change log in `CHANGES/`

---

## Deleting Pages

### When to Delete

- Obsolete information (outdated procedures, deprecated features)
- Duplicate content
- Pages that were never properly integrated

### When NOT to Delete

- Historical reference (move to archive section)
- Partially complete work (mark as draft)
- Controversial topics (move to discussion section)

### Deletion Procedure

1. **Document reason** — Why is deletion necessary?
2. **Update `README.md`** — Remove from index
3. **Update cross-references** — Fix broken links in related pages
4. **Add to change log** — Record in `CHANGES/`
5. **Delete file** — Remove from file system

---

## Cross-Reference Management

### Creating Links

**Correct:**
```markdown
- [Platform technologies](./ENTITIES/PLATFORM.md)
- [Platform isolation](./CONCEPTS/RAG.md)
- [Build instructions](./TOPICS/BUILD.md)
```

**Incorrect:**
```markdown
- platform.md (missing ./ prefix)
- ../ENTITIES/PLATFORM.md (wrong path)
- [platform](PLATFORM.md) (wrong anchor)
```

### Resolving Broken Links

**Method 1: Manual Check**
1. Use `grep` to find broken link references
2. Verify if page exists
3. Fix path or remove reference

**Method 2: Automated (if available)**
```bash
# Find broken links (requires tool support)
find WIKI_PAGES -name "*.md" -exec grep -l "\[\[" {} \;
```

### Link Styles

| Style | Use Case |
|-------|----------|
| `./LINK.md` | Same directory |
| `../LINK.md` | Parent directory |
| `[text](./LINK.md)` | Descriptive anchor |
| `[text](./LINK.md#anchor)` | Specific section |

---

## Commit Messages

### Schema-Compliant Format

```
[page] description

Detailed description (wrap at 72 characters)

---

- [ ] Checklist item
- [ ] Checklist item

Co-authored-by: Name <email>
```

### Examples

**New page:**
```
[CONCEPTS] Add platform isolation principles

Document RAG (Remote Attestation of Game) principles for platform isolation.

- Establish no native platform API calls in game logic
- Define isolation layer boundaries
- Document compile guards for optional behavior

Co-authored-by: AI Assistant <ai@generalsx.internal>
```

**Update:**
```
[CONCEPTS/RAG] Update FPS uncapped fix details

Add FPS cap implementation details and math consistency rules.

- FramePacer API guard for Windows
- FPS limit validation
- Build flag -ffp-contract=off

Co-authored-by: AI Assistant <ai@generalsx.internal>
```

**Deletion:**
```
[ARCHITECTURE] Remove obsolete layer diagram

Diagram was superseded by updated version in ARCHITECTURE/NEW.md.

- Original diagram outdated
- New diagram has improved clarity
- Cross-references updated

Co-authored-by: AI Assistant <ai@generalsx.internal>
```

---

## Verification Checklist

### Before Saving

- [ ] File path follows naming convention
- [ ] File extension is `.md`
- [ ] Content is in markdown format
- [ ] Headings use correct level (`#`, `##`, `###`)
- [ ] Links are properly formatted
- [ ] Code blocks are properly indented
- [ ] Tables use proper markdown syntax

### Before Committing

- [ ] Schema compliance verified
- [ ] Cross-references are valid
- [ ] Attribution line included
- [ ] Commit message follows format
- [ ] Changes are minimal and focused
- [ ] No unintended whitespace changes

### Before Pushing

- [ ] File is in correct location
- [ ] No secrets or sensitive data
- [ ] Git diff looks correct
- [ ] No merge conflicts

---

## Common Mistakes

### Path Errors

**Wrong:**
```markdown
- [Link](LINK.md)
```

**Right:**
```markdown
- [Link](./LINK.md)
```

### Missing Attribution

**Wrong:**
```
## Summary

Content here...
```

**Right:**
```
## Summary

Content here...

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, documentation
```

### Broken Cross-References

**Wrong:**
```
See also [Platform](ENTITIES/PLATFORM.md)
```

**Right:**
```
See also [Platform technologies](./ENTITIES/PLATFORM.md)
```

---

## Maintenance

### Regular Tasks

- **Weekly:** Review recent changes for schema compliance
- **Monthly:** Update `CHANGES/` entries
- **Quarterly:** Audit cross-references for broken links
- **Annually:** Review and archive obsolete pages

### Tools

| Tool | Purpose |
|------|--------|
| `grep` | Find cross-reference patterns |
| `find` | Locate pages by pattern |
| `git log` | Track page history |
| `git diff` | Review changes |

---

## See Also

- [WIKI_SCHEMA.md](./WIKI_SCHEMA.md) — Main schema
- [WIKI_SCHEMA_PART2.md](./WIKI_SCHEMA_PART2.md) — Page formats
- [README.md](./README.md) — Wiki index

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, wiki schema design
