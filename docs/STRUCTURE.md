# GeneralsX Documentation Structure

This document describes the new organization of the GeneralsX project documentation.

## Overview

The documentation has been reorganized into three main areas:

- **`docs/`** - Root documentation (build instructions, status, planning overview)
- **`docs/PLANNING/`** - All phase documentation organized by decade
- **`docs/DEV_BLOG/`** - Development diary organized by month
- **`docs/MISC/`** - Miscellaneous reference documentation

---

## Root Documentation (`docs/`)

Essential documentation files for quick reference:

### Build Instructions

- **`LINUX_BUILD_INSTRUCTIONS.md`** - Step-by-step Linux build guide
- **`MACOS_BUILD_INSTRUCTIONS.md`** - Step-by-step macOS build guide

### Project Overview

- **`PROJECT_STATUS.md`** - Current project status and phase progression
- **`PHASE_INDEX.md`** - Index of all phase documentation
- **`COMPREHENSIVE_VULKAN_PLAN.md`** - Complete Vulkan port plan overview

---

## Planning Documentation (`docs/PLANNING/`)

All phase documentation organized by decade for easier navigation.

### Structure

```text
docs/PLANNING/
├── 0/              # Phases 00-09
│   ├── PHASE00/
│   ├── PHASE00_5/
│   ├── PHASE01/
│   └── ...
├── 1/              # Phases 10-19
│   ├── PHASE10/
│   ├── PHASE11/
│   └── ...
├── 2/              # Phases 20-29
├── 3/              # Phases 30-39
├── 4/              # Phases 40-49
│   ├── PHASE40/
│   ├── PHASE41/
│   ├── PHASE42/
│   ├── PHASE43/
│   ├── PHASE40_QUICK_START.md
│   ├── PHASES_40_41_42_INDEX.md
│   └── VULKAN_PORT_EXECUTIVE_SUMMARY.md
├── FURTHER_PHASES/ # Future phases (50+)
├── LEGACY_DOCS/    # Historical documentation (Metal/OpenGL attempts)
├── SIDEQUESTS/     # Optional tasks and improvements
└── PHASE_REORGANIZATION_NOTES.md
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
├── 2024-12-DIARY.md          # December 2024 entries
├── 2025-01-DIARY.md          # January 2025 entries
├── 2025-01-SESSION_36-38_SUMMARY.md
├── 2025-08-DIARY.md          # August 2025 entries
├── 2025-09-DIARY.md          # September 2025 entries
├── 2025-10-DIARY.md          # October 2025 entries
├── 2025-11-DIARY.md          # November 2025 entries
├── 2025-11-SESSION_44_REPORT.md
├── 2025-11-SESSION_44_SUMMARY.md
└── 2025-11-SESSION_SUMMARY.md
```

### Naming Convention

- **Diary files**: `YYYY-MM-DIARY.md` (e.g., `2025-11-DIARY.md`)
- **Session reports**: `YYYY-MM-SESSION_XX_REPORT.md` or `YYYY-MM-SESSION_SUMMARY.md`

### Purpose

- Track daily/weekly development progress
- Document discoveries and solutions
- Reference critical breakthroughs
- Historical record of decision-making process

---

## Miscellaneous Documentation (`docs/MISC/`)

Reference documentation for specific topics and discoveries.

### Current Files

- `BIG_FILES_REFERENCE.md` - Asset structure and .big file contents
- `CRITICAL_VFS_DISCOVERY.md` - VFS integration analysis
- `CRITICAL_FIXES.md` - Emergency fixes and workarounds
- `LESSONS_LEARNED.md` - Key discoveries and architectural insights
- `MULTITHREADING_ANALYSIS.md` - Multithreading strategy
- And other specialized reference documents

---

## Historical Structure (Before Reorganization)

The old structure was:

```text
docs/
├── PHASE00/, PHASE01/, ..., PHASE43/
├── PHASE00_5/, PHASE02_5/
├── FURTHER_PHASES/
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
- [Phase 40 Documentation](docs/PLANNING/4/PHASE40/README.md)
- [Phase 41 Review](docs/PLANNING/4/PHASE41/REVIEW_COMPLETE.md)
- [Phases 0-9](docs/PLANNING/0/)
```

### To Reference Development Blog

```markdown
- [Development Diary - November 2025](docs/DEV_BLOG/2025-11-DIARY.md)
- [Session 44 Summary](docs/DEV_BLOG/2025-11-SESSION_44_SUMMARY.md)
```

### To Reference Planning Index

```markdown
- [All Phase Documentation](docs/PLANNING/)
- [Further Phases](docs/PLANNING/FURTHER_PHASES/)
- [Legacy Documentation](docs/PLANNING/LEGACY_DOCS/)
```

---

## Updating Documentation

### When Adding a New Phase

1. Create `docs/PLANNING/{decade}/PHASE{number}/` directory
2. Add phase documentation files there
3. Update relevant index files

### When Documenting Development Progress

1. Add entries to the appropriate month's diary file
   - `docs/DEV_BLOG/YYYY-MM-DIARY.md`
   - Use the `[x]` syntax for completed tasks
2. Create session reports as needed
   - `docs/DEV_BLOG/YYYY-MM-SESSION_XX_SUMMARY.md`

### When Creating Reference Documentation

1. Add to `docs/MISC/` if it's a general reference
2. Add to the relevant phase directory if it's phase-specific
3. Update this structure document if necessary

---

## Quick Navigation

- **Building the project?** → Start with `MACOS_BUILD_INSTRUCTIONS.md` or `LINUX_BUILD_INSTRUCTIONS.md`
- **Need current status?** → Check `PROJECT_STATUS.md`
- **Looking for specific phase?** → Use `PHASE_INDEX.md` or browse `docs/PLANNING/{decade}/`
- **Tracking recent progress?** → Check `docs/DEV_BLOG/2025-11-DIARY.md`
- **Need technical reference?** → Browse `docs/MISC/`
- **Understanding past attempts?** → Review `docs/PLANNING/LEGACY_DOCS/`

---

## Notes

- All documentation is in English
- All filenames are in English
- Markdown files use consistent formatting
- Cross-references use relative paths for portability
- Historical documents are preserved in `docs/PLANNING/LEGACY_DOCS/` for reference
