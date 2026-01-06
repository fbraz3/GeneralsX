# GeneralsX Documentation Structure

This document describes the new organization of the GeneralsX project documentation.

## Overview

The documentation is organized into three main areas:

- **`docs/ETC/`** - General documentation (build instructions, structure, indexes)
- **`docs/WORKDIR/`** - Working documentation (phases, planning, reports, support, audits)
- **`docs/DEV_BLOG/`** - Development diary organized by month

---

## General Documentation (`docs/ETC/`)

Essential documentation files for quick reference:

### Build Instructions

- **`LINUX_BUILD_INSTRUCTIONS.md`** - Step-by-step Linux build guide
- **`MACOS_BUILD_INSTRUCTIONS.md`** - Step-by-step macOS build guide

### Navigation / Overview

- **`PHASE_INDEX.md`** - Index of phase documentation
- **`PHASE_ROADMAP_OVERVIEW.md`** - High-level roadmap
- **`STRUCTURE.md`** - This document

---

## Work Documentation (`docs/WORKDIR/`)

All phase documentation organized by decade for easier navigation.

### Layout

```text
docs/WORKDIR/
├── phases/         # Phase documentation (organized by decade)
│   ├── 0/
│   ├── 1/
│   ├── 2/
│   ├── 3/
│   ├── 4/
│   ├── 5/
│   ├── planned/
│   └── legacy/
├── planning/       # Roadmaps, sidequests, design notes
├── reports/        # Session summaries / archives
│   └── DEV_ARCHIVES/
├── audit/          # Audits (integration, compatibility, etc.)
├── support/        # Cross-phase technical references
└── lessons/        # Lessons learned
```

### Each Phase Directory Contains

- `README.md` - Phase overview and objectives
- Implementation documentation files
- Session progress reports
- Completion summaries

---

## Development Blog (`docs/DEV_BLOG/`)

Technical development diary organized by month for historical tracking.

### Structure

```text
docs/DEV_BLOG/
├── README.md
├── 2024-12-DIARY.md
├── 2025-01-DIARY.md
├── 2025-08-DIARY.md
├── 2025-09-DIARY.md
├── 2025-10-DIARY.md
└── 2025-11-DIARY.md
```

### Naming Convention

- **Diary files**: `YYYY-MM-DIARY.md` (e.g., `2025-11-DIARY.md`)
- **No session reports in this folder** (use `docs/WORKDIR/reports/`)

### Purpose

- Track daily/weekly development progress
- Document discoveries and solutions
- Reference critical breakthroughs
- Historical record of decision-making process

---

## Technical References (`docs/WORKDIR/support/`)

Cross-phase technical references (VFS, assets, backends, investigations).

---

## Historical Structure (Before Reorganization)

The old structure was:

```text
docs/
├── PHASE00/, PHASE01/, ..., PHASE43/
├── PHASE00_5/, PHASE02_5/
├── PLANNED_PHASES/
├── LEGACY_DOCS/
├── MACOS_PORT_DIARY.md (now split into docs/DEV_BLOG/YYYY-MM-DIARY.md)
├── DEVELOPMENT_DIARY.md (removed)
└── Other .md files
```

---

## Cross-References in Documentation

When referencing other documentation:

### To Reference a Phase

```markdown
- [Phase 40 Documentation](../WORKDIR/phases/4/PHASE40/README.md)
- [Phase 41 Review](../WORKDIR/phases/4/PHASE41/REVIEW_COMPLETE.md)
- [Phases 0-9](../WORKDIR/phases/0/)
```

### To Reference Development Blog

```markdown
- [Development Diary - November 2025](../DEV_BLOG/2025-11-DIARY.md)
- [Session 44 Summary](../WORKDIR/reports/DEV_ARCHIVES/2025-11-SESSION_44_SUMMARY.md)
```

### To Reference Planning Index

```markdown
- [All Phase Documentation](docs/WORKDIR/)
- [Planned Phases](docs/WORKDIR/phases/planned/)
- [Legacy Documentation](docs/WORKDIR/phases/legacy/)
```

---

## Updating Documentation

### When Adding a New Phase

1. Create `docs/WORKDIR/phases/{decade}/PHASE{number}/` directory
2. Add phase documentation files there
3. Update relevant index files

### When Documenting Development Progress

1. Add entries to the appropriate month's diary file
   - `docs/DEV_BLOG/YYYY-MM-DIARY.md`
   - Use the `[x]` syntax for completed tasks
2. Create session reports as needed under `docs/WORKDIR/reports/`

### When Creating Reference Documentation

1. Add to `docs/WORKDIR/support/` if it's a cross-phase reference
2. Add to the relevant phase directory if it's phase-specific
3. Update this structure document if necessary

---

## Quick Navigation

- **Building the project?** → Start with `docs/ETC/MACOS_BUILD_INSTRUCTIONS.md` or `docs/ETC/LINUX_BUILD_INSTRUCTIONS.md`
- **Looking for specific phase?** → Use `docs/ETC/PHASE_INDEX.md` or browse `docs/WORKDIR/phases/`
- **Tracking recent progress?** → Check `docs/DEV_BLOG/2025-11-DIARY.md`
- **Need technical reference?** → Browse `docs/WORKDIR/support/`
- **Understanding past attempts?** → Review `docs/WORKDIR/phases/legacy/`

---

## Notes

- All documentation is in English
- All filenames are in English
- Markdown files use consistent formatting
- Cross-references use relative paths for portability
- Historical documents are preserved in `docs/WORKDIR/phases/legacy/` for reference
