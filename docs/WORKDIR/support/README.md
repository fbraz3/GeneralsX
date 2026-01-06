# Support Documentation (Cross-Phase References)

This directory contains cross-phase technical references: investigations, architectural notes, and “how it works” docs that are useful across multiple phases.

## Organization Rules

### ✅ BELONGS in docs/WORKDIR/support/

- Analysis documents (e.g., `VULKAN_ANALYSIS.md`, `DX8WRAPPER_LEGACY_ANALYSIS.md`)
- Backend and renderer notes (e.g., `GRAPHICS_BACKENDS.md`, `OPENGL_TESTING.md`)
- Technical lessons and discoveries (e.g., `LESSONS_LEARNED.md`, `CRITICAL_VFS_DISCOVERY.md`)
- Cross-phase investigations and references (e.g., `.big` file structure)

### ✅ BELONGS in docs/WORKDIR/reports/DEV_ARCHIVES/

- Session summary reports (`YYYY-MM-SESSION_XX_SUMMARY.md`)
- Session analysis documents (`YYYY-MM-ANALYSIS_COMPLETE.md`)
- Session session reports (not monthly diaries)
- Temporary working documents
- Concluded work summaries

### ❌ DOES NOT BELONG in docs/WORKDIR/support/

- Monthly development diaries → Use `docs/DEV_BLOG/YYYY-MM-DIARY.md`
- Phase-specific implementation notes → Keep in the relevant `docs/WORKDIR/phases/.../PHASEXX.../` directory
- Build/compilation instructions → Use `docs/ETC/` (e.g., `docs/ETC/MACOS_BUILD_INSTRUCTIONS.md`)

## File Naming Conventions

### Analysis Files

```text
[TOPIC]_ANALYSIS.md
Examples: VULKAN_ANALYSIS.md, DX8WRAPPER_LEGACY_ANALYSIS.md
```

### Integration/Review Files

```text
INTEGRATION_[DESCRIPTION]_PHASES_XX_XX.md
PHASE_XX_YY_REVIEW_[STATUS].md
Examples: INTEGRATION_AUDIT_PHASES_32_35.md, PHASE_16_20_REVIEW_COMPLETE.md
```

### Archived Session Files

Located in `docs/WORKDIR/reports/DEV_ARCHIVES/`:

```text
YYYY-MM-SESSION_XX_[TYPE].md
YYYY-MM-[DESCRIPTION]_[TYPE].md
Examples:
- 2025-11-SESSION_44_SUMMARY.md
- 2025-11-ANALYSIS_COMPLETE.md
```

## Purpose

This directory serves as a catch-all for:

- **Research and analysis** documents
- **Architectural investigations** and findings
- **Cross-phase integration** reviews
- **Technical discoveries** and lessons learned
- **Historical session reports** and summaries (in DEV_ARCHIVES)
- **Reference materials** for complex systems

## Related Documentation

- **Development Diaries**: See `docs/DEV_BLOG/README.md` for chronological session logs
- **Phases**: See `docs/WORKDIR/phases/` for phase-specific documentation
- **Planning**: See `docs/WORKDIR/planning/` for roadmap and planning documents
