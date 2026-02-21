---
applyTo: '**/*.md'
---

## Documentation Guidelines

- All documentation **MUST BE** in English
- Use Markdown format
- Don't add documentation files directly in the root `docs/` folder
- The root folder `/` should only contain project-level files (README.md, LICENSE, etc.)
- **Active Work**: Place in `docs/WORKDIR/` with appropriate subdirectory (phases, planning, reports, support, audit, lessons)
- **Development Diary**: Update `docs/DEV_BLOG/YYYY-MM-DIARY.md` with daily entries
  - Create new file each month (YYYY-MM-DIARY.md)
  - Order entries newest to oldest (recent at top after Overview section)
  - Keep entries informal and concise
- **Reference & Historical**: Place in `docs/ETC/` (older reference materials, archived analysis)
- **Phase Checklist Updates**: At the end of each session working on a phase, update the corresponding `docs/WORKDIR/phases/PHASEXX_*.md` file to mark completed tasks with `[x]`

**Key Rule**: DEV_BLOG is for diary only. Active work goes to WORKDIR. Reference/historical materials go to ETC.

## Documentation Updates

- **Dev diary** (`docs/DEV_BLOG/YYYY-MM-DIARY.md`): Informal session notes, newest first
- **Session reports** (`docs/WORKDIR/reports/PHASEXX_SESSIONX_*.md`): Formal summary after significant progress
- **Phase planning** (`docs/WORKDIR/phases/PHASEXX_*.md`): Update `[x]` checklist at session end
- **Technical discoveries**: Place in `docs/WORKDIR/support/` (e.g., `CRITICAL_VFS_DISCOVERY.md`)
- **Lessons learned** (`docs/WORKDIR/lessons/LESSONS_LEARNED.md`): Key takeaways from phases and work cycles
- **Known Issues**: Place in `docs/KNOWN_ISSUES/` with format `ISSUE-XXX_description.md` (e.g., `ISSUE-001_shell_map_unit_immortality.md`)

## Documentation Organization

### `docs/WORKDIR/` - Active Working Directory
**Purpose**: All active project work during current development cycle
**Subdirectories**:

#### `docs/WORKDIR/phases/` - Phase-Specific Plans
**Purpose**: Detailed phase plans and checklists
**Guideline**: use `PHASEXX_purpose.md` format for filenames - XX is phase number, purpose is brief description
**Restriction**: Avoid using `weeks` for phase work segmentation, don't try to guess completion times in calendar weeks, just ignore this information entirely
**Rationale**: Sprints provide a standardized Agile framework terminology, ensuring consistency with sprint-based development methodologies

**Naming Examples:**
- PHASE01_INITIAL_RESEARCH.md
- PHASE02_ENGINE_SELECTION.md
- PHASE03_PROTOTYPING.md
- etc.

#### `docs/WORKDIR/planning/` - Planning & Strategic Documents
**Purpose**: Planning documents, roadmaps, architectural decisions
**Naming Convention**:
- `PLAN-XXX_description.md` for individual plans
- `ROADMAP.md` for overall project roadmap
- Other strategic planning documents

**Examples:**
- PLAN-010_VISUAL_LAYOUT.md
- PLAN-013_PARTICLE_SYSTEM.md
- ROADMAP.md

#### `docs/WORKDIR/reports/` - Session Reports & Progress
**Purpose**: Formal summaries after significant progress on phases
**Naming Convention**: `PHASEXX_SESSIONX_description.md`

**Examples:**
- PHASE01_SESSION1_INITIAL_RESEARCH_COMPLETE.md
- PHASE02_SESSION2_ENGINE_SELECTION_COMPLETE.md

#### `docs/WORKDIR/support/` - Findings & Support Documents
**Purpose**: Technical discoveries, analysis, reference materials for active phases
**Content Types**:
- Technical analysis documents
- Implementation findings
- Code reference guides
- Research supporting active work

**Examples:**
- VFS_IMPLEMENTATION_FINDINGS.md
- PARTICLE_SYSTEM_DEEP_ANALYSIS.md
- TOOLTIP_CODE_REFERENCE.md

#### `docs/WORKDIR/audit/` - Audit & Verification Files
**Purpose**: Audit logs, verification checklists, compliance documents
**Content Types**:
- Structure audits
- Implementation checklists
- Gap analysis documents
- Compliance verification

**Examples:**
- ROADMAP_AUDIT_DECEMBER_2025.md
- GAP_ANALYSIS_FINDINGS.md

#### `docs/WORKDIR/lessons/` - Lessons Learned
**Purpose**: Key insights from phases and work cycles
**Main File**: `LESSONS_LEARNED.md` - Central repository for all lessons
**Content**: Phase-specific learnings, technical insights, process improvements

### `docs/DEV_BLOG/` - Development Diary ONLY
**Purpose**: Chronological development diary entries
- YYYY-MM-DIARY.md - Monthly diary (ONE file per month)
  - YYYY is the current year
  - MM is the current month
  - DIARY is a fixed literal
  - Entries newest to oldest (most recent at top, after Overview)
  - Informal, daily/session notes
  - Short summaries of work done
- `docs/DEV_BLOG/README.md` - Index of available diaries and overview of diary purpose with details on structure and usage

**Only this goes here**: Diary entries and README

**Not here**: Session reports, summaries, analysis, phase progress

### `docs/KNOWN_ISSUES/` - Issue Tracking
**Purpose**: Documented known issues, bugs, limitations, and pending investigations
**Structure**: One issue per file
**Naming Convention**: `ISSUE-XXX_slug_description.md`
  - `XXX` = Zero-padded issue number (001, 002, 003, etc.)
  - `slug_description` = Brief lowercase, underscore-separated description

**Examples**:
- `ISSUE-001_shell_map_unit_immortality.md`
- `ISSUE-002_audio_crackling_on_startup.md`
- `ISSUE-003_replay_desync_multiplayer.md`

**Status Values**:
- OPEN — Confirmed issue, awaiting investigation or fix
- INVESTIGATING — Currently being researched; uncertain root cause
- BLOCKED — Waiting for external feedback, data access, or prerequisites
- RESOLVED — Fixed; waiting for verification or release
- WONTFIX — Intentionally deferred; rationale documented

**Severity Levels**:
- Critical — Game-breaking, prevents progress
- High — Major feature impaired, significant gameplay impact
- Medium — Observable but workaroundable, cosmetic impact
- Low — Cosmetic only, no gameplay impact

**Component Categories**:
- Graphics, Audio, Gameplay, Platform, Build, Other

**Structure**:
- Status, Session Discovered, Severity, Component, Reproducibility (header)
- Symptom (observable behavior)
- Investigation Summary (root cause analysis, hypotheses)
- Code Audit Results (what was checked)
- Next Steps (actionable items for future investigation)
- Workaround (if available)
- Impact (gameplay, stability, determinism, release blocker)
- Reference (links to code, dev diary, etc.)

See `docs/KNOWN_ISSUES/README.md` for detailed template and guidelines.

### `docs/ETC/` - Reference & Historical Materials
**Purpose**: Older reference materials, archived analysis, and miscellaneous documentation
- General reference materials
- Archived technical documentation
- Historical analysis documents
- Miscellaneous project materials not fitting other categories

**Guidelines**:
- New active work should NOT go here
- Use for long-term reference materials
- Archive completed analysis here if still needed for reference

**Not here**: Active phase work, current session reports, active planning
