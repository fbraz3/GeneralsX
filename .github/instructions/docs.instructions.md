---
applyTo: '**/*.md'
---

## Documentation Guidelines

- Docs in English, Markdown only.
- Keep `docs/ETC/COMMAND_LINE_PARAMETERS.md` current for runtime-critical flags and caveats.
- Never add docs in root `docs/`; use `docs/WORKDIR/` for active work, `docs/DEV_BLOG/` for diary, `docs/ETC/` for reference/history.
- Update phase checklists in `docs/WORKDIR/phases/PHASEXX_*.md` at session end.

## Documentation Updates

- Dev diary: `docs/DEV_BLOG/YYYY-MM-DIARY.md`.
- Session reports: `docs/WORKDIR/reports/PHASEXX_SESSIONX_*.md`.
- Phase planning: `docs/WORKDIR/phases/PHASEXX_*.md`.
- Technical discoveries: `docs/WORKDIR/support/`.
- Lessons learned: `docs/WORKDIR/lessons/LESSONS_LEARNED.md`.
- Known issues live in GitHub Issues; no new markdown issue files.

## Documentation Organization

### `docs/WORKDIR/` - Active Working Directory
**Purpose**: All active project work during current development cycle
**Subdirectories**:

#### `docs/WORKDIR/phases/` - Phase-Specific Plans
Plans and checklists. Use `PHASEXX_purpose.md` filenames. Ignore week estimates.

Examples:
- PHASE01_INITIAL_RESEARCH.md
- PHASE02_ENGINE_SELECTION.md
- PHASE03_PROTOTYPING.md

#### `docs/WORKDIR/planning/` - Planning & Strategic Documents
Planning docs, roadmaps, architecture decisions. Use `PLAN-XXX_description.md` or `ROADMAP.md`.

Examples:
- PLAN-010_VISUAL_LAYOUT.md
- PLAN-013_PARTICLE_SYSTEM.md
- ROADMAP.md

#### `docs/WORKDIR/reports/` - Session Reports & Progress
Formal phase summaries. Use `PHASEXX_SESSIONX_description.md`.

Examples:
- PHASE01_SESSION1_INITIAL_RESEARCH_COMPLETE.md
- PHASE02_SESSION2_ENGINE_SELECTION_COMPLETE.md

#### `docs/WORKDIR/support/` - Findings & Support Documents
Technical discoveries, analysis, reference guides, and research support.

Examples:
- VFS_IMPLEMENTATION_FINDINGS.md
- PARTICLE_SYSTEM_DEEP_ANALYSIS.md
- TOOLTIP_CODE_REFERENCE.md

#### `docs/WORKDIR/audit/` - Audit & Verification Files
Audit logs, verification checklists, compliance docs, gap analysis.

Examples:
- ROADMAP_AUDIT_DECEMBER_2025.md
- GAP_ANALYSIS_FINDINGS.md

#### `docs/WORKDIR/lessons/` - Lessons Learned
Key insights, technical takeaways, process improvements. Main file: `LESSONS_LEARNED.md`.

### `docs/DEV_BLOG/` - Development Diary ONLY
Chronological diary entries only. One `YYYY-MM-DIARY.md` per month, newest first. Also `docs/DEV_BLOG/README.md`.

Not here: reports, analysis, phase progress.

## Issue Tracking — GitHub is the Source of Truth

Track issues, bugs, features, and enhancements in GitHub Issues (`https://github.com/fbraz3/GeneralsX/issues/`), not markdown.

### Creating New Issues

Use the `gh issue create` command to create issues:

```bash
gh issue create \
  --title "Brief, actionable title" \
  --body "## Context\n...\n## Goal\n...\n## Acceptance Criteria\n..." \
  --label "enhancement" \
  --label "Linux"
```

**Labels** (always apply 1-2):
- `enhancement` — feature or improvement
- `bug` — broken behavior
- `documentation` — docs work
- `Linux`, `macOS` — platform scope
- `Generals`, `Zero Hour` — game variant
- `Blocker` — blocks other work

### Markdown Documentation (Legacy)

Older `docs/KNOWN_ISSUES/` files are deprecated. Do not create new markdown issue files; archive or delete duplicates after moving content to GitHub.

### Deleted/Resolved/Archived Issues

If an issue is closed/resolved in GitHub:
1. Close the issue on GitHub with appropriate resolution
2. Delete the corresponding `.md` file from `docs/KNOWN_ISSUES/`
3. Do NOT keep markdown copies of resolved issues

### Legacy `.md` Issues (Historical Reference)

Keep historical markdown issues in `docs/ETC/archive/` only. Do not maintain them going forward.

### `docs/BUILD/` - Platform Build Instructions
Platform build/setup guides. One all-caps file per platform (`LINUX.md`, `MACOS.md`, `WINDOWS.md`). Keep them canonical and synced to build scripts/CI.

### `docs/ETC/` - Reference & Historical Materials
Archived reference, analysis, and misc docs only. No active work, reports, planning, or build instructions.
