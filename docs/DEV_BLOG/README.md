---
applyTo: 'docs/DEV_BLOG/**'
---

# Development Diary (DEV_BLOG)

This directory contains **chronological development diaries** organized by year-month format.

## Naming Convention

**All files in this directory MUST follow this exact naming pattern:**

```
YYYY-MM-DIARY.md
```

### Examples
- `2024-12-DIARY.md` - December 2024 development log
- `2025-01-DIARY.md` - January 2025 development log
- `2025-11-DIARY.md` - November 2025 development log

### Pattern Rules
- `YYYY` = 4-digit year (e.g., 2025)
- `MM` = 2-digit month with leading zero (e.g., 01, 11)
- Separator = hyphen `-`
- Fixed suffix = `-DIARY.md` (lowercase)
- **NO exceptions** for session reports, summaries, or analysis files

## File Organization

### ✅ ALLOWED in this directory
- `YYYY-MM-DIARY.md` - Monthly development diaries (only)

### ❌ NOT ALLOWED in this directory
- `YYYY-MM-SESSION_XX_SUMMARY.md` - Move to `docs/MISC/DEV_ARCHIVES/`
- `YYYY-MM-ANALYSIS_COMPLETE.md` - Move to `docs/MISC/DEV_ARCHIVES/`
- `YYYY-MM-DIARY_SESSION_XX.md` - Move to `docs/MISC/DEV_ARCHIVES/`
- Any other variations - Move to `docs/MISC/DEV_ARCHIVES/`

## Purpose

Development diaries serve as:
- **Chronological log** of all work sessions and decisions
- **Technical decision tracking** - reasoning behind major changes
- **Phase progress tracking** - completion status of implementation phases
- **Discovery log** - critical insights and breakthroughs
- **Reference** - easily locate historical context by month

## Content Guidelines

Each diary should include:

1. **Session entries** with:
   - Date and time (UTC timezone preferred)
   - Objective/task being worked on
   - Key decisions made
   - Commits hash references
   - Issues encountered and solutions

2. **Phase progress** tracking:
   - Current phase number
   - Symbols resolved/remaining
   - Compilation status (errors, warnings)
   - Testing results

3. **Important discoveries**:
   - Breakthrough moments
   - Root cause analysis for bugs
   - Performance findings
   - Integration challenges

## Archive Location

Non-diary files are organized in:
```
docs/MISC/DEV_ARCHIVES/
```

This includes:
- Session summaries and reports
- Analysis documents
- Meeting notes
- Temporary documentation

## Monthly Rotation

- **Create new diary** on first working day of month: `YYYY-MM-DIARY.md`
- **Archive old entries** (session reports) to `docs/MISC/DEV_ARCHIVES/` as needed
- **Keep only diaries** in this directory for clean, organized workspace

---

**Critical Rule**: This directory contains ONLY chronological diaries. All supplementary documentation must be archived in appropriate `docs/MISC/` subdirectories.
