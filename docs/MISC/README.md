# Miscellaneous Documentation (MISC)

This directory contains supplementary project documentation that doesn't fit into other organized categories.

## Structure

```
docs/MISC/
├── DEV_ARCHIVES/          # Archived development session reports, summaries, and analysis
├── BIG_FILES_REFERENCE.md # .big file asset structure and VFS documentation
├── CRITICAL_VFS_DISCOVERY.md # VFS integration analysis and findings
├── LESSONS_LEARNED.md     # Technical lessons from phases and debugging sessions
├── VULKAN_ANALYSIS.md     # Vulkan implementation strategy and notes
└── [other analysis files] # Phase-specific analysis, integration audits, etc.
```

## Organization Rules

### ✅ BELONGS in docs/MISC/
- Analysis documents (e.g., `VULKAN_ANALYSIS.md`, `DX8WRAPPER_LEGACY_ANALYSIS.md`)
- Integration audits (e.g., `INTEGRATION_AUDIT_PHASES_32_35.md`)
- Technical lessons and discoveries (e.g., `LESSONS_LEARNED.md`)
- Cross-phase review documents
- Critical findings and investigations

### ✅ BELONGS in docs/MISC/DEV_ARCHIVES/
- Session summary reports (`YYYY-MM-SESSION_XX_SUMMARY.md`)
- Session analysis documents (`YYYY-MM-ANALYSIS_COMPLETE.md`)
- Session session reports (not monthly diaries)
- Temporary working documents
- Concluded work summaries

### ❌ DOES NOT BELONG in docs/MISC/
- Monthly development diaries → Use `docs/DEV_BLOG/YYYY-MM-DIARY.md`
- Phase-specific planning → Use `docs/PHASE##/`
- Build/compilation instructions → Use `docs/` root (e.g., `docs/MACOS_BUILD_INSTRUCTIONS.md`)

## File Naming Conventions

### Analysis Files
```
[TOPIC]_ANALYSIS.md
Examples: VULKAN_ANALYSIS.md, DX8WRAPPER_LEGACY_ANALYSIS.md
```

### Integration/Review Files
```
INTEGRATION_[DESCRIPTION]_PHASES_XX_XX.md
PHASE_XX_YY_REVIEW_[STATUS].md
Examples: INTEGRATION_AUDIT_PHASES_32_35.md, PHASE_16_20_REVIEW_COMPLETE.md
```

### Archived Session Files
Located in `DEV_ARCHIVES/`:
```
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
- **Phases**: See `docs/PHASE##/` for phase-specific documentation
- **Planning**: See `docs/PLANNING/` for roadmap and planning documents
