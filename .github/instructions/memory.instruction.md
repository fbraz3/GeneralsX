---
applyTo: '**'
---

# Agent Memory for GeneralsX Development

## Current Session Summary (Session 47)

### Completed Tasks ✅

1. **Build System Simplification** (Commit: `bdd4dc81`)
   - Removed legacy `macos-x64` preset from CMakePresets.json
   - Removed from configurePresets, buildPresets, and workflowPresets
   - Updated 6 build scripts to use unified `build/macos` directory
   - Updated 8 documentation files with new preset and target names
   - Validated: `cmake --preset macos` successfully configures
   - Validated: `cmake --build build/macos --target GeneralsXZH -j 4` builds without new errors
   - Status: ✅ COMPLETE and PUSHED

2. **DEV_BLOG Reorganization** (Commit: `de91a0f9`)
   - Enforced strict naming: `YYYY-MM-DIARY.md` only in `docs/DEV_BLOG/`
   - Moved 6 non-diary files to `docs/WORKDIR/reports/DEV_ARCHIVES/`
   - Created README.md documenting strict naming rules
   - Updated project.instructions.md and copilot-instructions.md
   - Status: ✅ COMPLETE and PUSHED

3. **Documentation Structure Cleanup** (Commits: `22e09bf5`, `229a27fa`, `9644034d`, `4c6bfa36`)
   - Moved `PHASE_43_5/` from docs root → `docs/WORKDIR/phases/4/PHASE43_5_FINAL/`
   - Moved analysis files to correct decade-based locations
   - Removed phantom `COMPLETED_PHASES` directory
   - Updated cross-references
   - Status: ✅ COMPLETE and PUSHED

4. **STRUCTURE.md Reference Updates** (Commit: `08cb973f`)
   - Updated line 60: FURTHER_PHASES → PLANNED_PHASES in structure diagram
   - Updated line 131: FURTHER_PHASES → PLANNED_PHASES in historical structure
   - Updated line 164: FURTHER_PHASES → PLANNED_PHASES in cross-references
   - Validated all 3 replacements applied correctly
   - Status: ✅ COMPLETE and PUSHED

### Current Documentation Structure (Validated)

```text
docs/WORKDIR/
├── 0/ through 5/        # Decade-based phase organization
├── PLANNED_PHASES/      # Future phases (50+)
│   ├── PHASE40/ through PHASE47/
├── LEGACY_DOCS/         # Historical attempts (Metal/OpenGL)
├── SIDEQUESTS/          # Optional improvements
└── PHASE_REORGANIZATION_NOTES.md
```

**Critical Rules**:

- `docs/DEV_BLOG/` → MUST use strict naming: `YYYY-MM-DIARY.md` (e.g., `2025-11-DIARY.md`)
- No other files in DEV_BLOG (session reports go to `docs/WORKDIR/reports/DEV_ARCHIVES/`)
- Phase documentation follows decade organization (0-5)
- All future phases (50+) go in `PLANNED_PHASES/`

### Build System Status

**Current Presets** (in CMakePresets.json):

- `vc6` - Windows 32-bit legacy
- `win32` - Windows compatibility
- `unix` - Generic Unix
- `macos` - **PRIMARY** - ARM64 native (Apple Silicon)
- `linux` - Linux 64-bit
- `windows` - Windows 64-bit (future Vulkan)

**Build Directory Simplified**:

- ✅ `build/macos/` is now unified (no more `build/macos-arm64` or `build/macos-x64`)

**Target Names** (synchronized everywhere):

- `GeneralsXZH` - Zero Hour expansion (PRIMARY - recommended)
- `GeneralsX` - Original Generals base game (SECONDARY)

### Known Issues & Blockers

1. **Linking Blocker**: 8 pre-existing Phase 43 undefined symbols (intentional - Gates Vulkan port progress)
   - Status: Expected, not addressed in this session
   - Impact: No executable generated on `cmake --build`
   - Context: Last successful executable Sept 2025

2. **Vulkan Port in Progress**: Phase 39+ architecture being implemented
   - Current status: Analysis phase (see `docs/WORKDIR/support/VULKAN_ANALYSIS.md`)

### Recent Commits Made This Session

| Commit | Message | Impact |
|--------|---------|--------|
| `bdd4dc81` | refactor(build): simplify macOS preset by removing legacy Intel support | Build system simplified |
| `de91a0f9` | refactor(docs): enforce strict DEV_BLOG naming convention | Documentation consistency |
| `22e09bf5` | refactor(docs): reorganize phase documentation | Structure cleanup |
| `229a27fa` | refactor(docs): move PHASE43_FINAL_SUCCESS | Structure correction |
| `9644034d` | chore(docs): remove duplicate PHASE_43_5 directory | Redundancy removal |
| `4c6bfa36` | refactor(docs): move Phase 43 docs to correct location | Final organization |
| `08cb973f` | refactor(docs): update documentation references from FURTHER_PHASES to PLANNED_PHASES | Reference updates |

### Branch Strategy

**Current Status**: Working on `main` branch exclusively

- ✅ No sync needed with `vulkan-port` branch
- ✅ All development commits go directly to `main`
- ✅ Commits from Session 47 were pushed to vulkan-port, but main branch is now primary
- 📝 Next commits should go to `main` branch

### Next Session Priorities

1. **Verification Tasks** (low priority - documentation is clean):
   - [ ] Spot-check all PLANNED_PHASES references resolve correctly
   - [ ] Verify no orphaned phase documentation remains

2. **Development Work** (high priority):
   - [ ] Continue Vulkan port implementation (Phase 39+)
   - [ ] Address 8 linking blockers when ready for Phase 44+

### Key Files to Remember

- **Build Configuration**: `CMakePresets.json` - Central build presets (simplified this session)
- **Build Documentation**: `.github/instructions/project.instructions.md` - Build commands reference
- **Build Scripts**: `scripts/build_zh.sh`, `scripts/deploy.sh` - Updated this session
- **Development Diary**: `docs/DEV_BLOG/YYYY-MM-DIARY.md` - STRICT naming enforced
- **Documentation Structure**: `docs/STRUCTURE.md` - Updated and validated this session

### Asset Directory Paths (Development Testing)

```bash
# Zero Hour expansion (primary target)
$HOME/GeneralsX/GeneralsMD/  # Contains: executable, Data/, Maps/

# Original Generals
$HOME/GeneralsX/Generals/    # Contains: executable, Data/, Maps/
```

### Important Reminders for Next Session

1. ✅ Documentation is now CLEAN and ORGANIZED - ready for feature development
2. ✅ Build system is SIMPLIFIED - ARM64 primary, no Intel legacy code
3. ✅ All references CONSISTENT - no outdated presets or target names
4. ⚠️ 8 linking blockers are INTENTIONAL - gates Vulkan port progress
5. 🔄 Vulkan port work should be next major task (Phase 39+)
6. 🌿 **WORKING ON `main` BRANCH NOW** - No sync needed with vulkan-port branch
7. 📝 Remember: Commit at end of session, push to main branch (when permitted)
8. ⏰ Respect commit time restrictions: Monday-Friday 09:00-18:00 local time - NO commits (unless explicitly requested)
