# GeneralsX Documentation Guide

## Quick Navigation

### 🚀 Getting Started

1. **New to Project?** → Read `START_HERE.md`
2. **Need Build Info?** → Read `MACOS_BUILD_INSTRUCTIONS.md` or `LINUX_BUILD_INSTRUCTIONS.md`
3. **Want Overview?** → Read `PHASES_SUMMARY.md`
4. **Current Status?** → Read `MACOS_PORT_DIARY.md`

### 📖 Core Documentation

| Document | Purpose |
|----------|---------|
| `README.md` | Project overview and goals |
| `ROADMAP.md` | Development roadmap and timeline |
| `READING_GUIDE.md` | How to read this documentation |
| `START_HERE.md` | Entry point for new developers |
| `REFERENCE_REPOSITORIES.md` | Reference git submodules |

### 🔧 Build & Setup

| Document | Platform |
|----------|----------|
| `MACOS_BUILD_INSTRUCTIONS.md` | macOS ARM64/Intel |
| `LINUX_BUILD_INSTRUCTIONS.md` | Linux x86_64 |
| `TESTING.md` | Running tests |

### 📊 Development Tracking

| Document | Content |
|----------|---------|
| `MACOS_PORT_DIARY.md` | Complete session history (CRITICAL - always up-to-date) |
| `PHASES_SUMMARY.md` | Quick status of all phases |

### 🏗️ Active Development

| Phase | Status | Location |
|-------|--------|----------|
| **Phase 49** | Reference/Completed | `PHASE49/` |
| **Phase 50** | Current Development | `PHASE50/` (Vulkan-Only Refactor) |

### 📚 Reference Materials

- `Misc/LESSONS_LEARNED.md` - Architectural insights from phases 38-48
- `Misc/CRITICAL_FIXES.md` - Emergency fixes and validation patterns
- `Misc/BIG_FILES_REFERENCE.md` - Game asset structure (.big files)
- `Known_Issues/` - Known bugs and workarounds
- `Vulkan/` - Vulkan SDK documentation and references

### 📦 Historical Documentation

- `PHASES_ARCHIVED/` - Complete history of Phases 38-48
  - These are kept for reference and architectural context
  - See `PHASES_ARCHIVED/README.md` for migration guide
  - Use when understanding why certain decisions were made

## 📋 By Role

### 👨‍💻 Developer

**Start with:**

- `.github/copilot-instructions.md` (AI context)
- `.github/instructions/project.instructions.md` (Build info)
- `PHASE50/VULKAN_ONLY_CLEAN_REFACTOR.md` (Current architecture)

**Reference during work:**

- `Misc/LESSONS_LEARNED.md` - Avoid pitfalls
- `MACOS_PORT_DIARY.md` - Latest status
- `references/fighter19-dxvk-port/` - Vulkan patterns

### 🧪 QA/Tester

**Start with:**

- `TESTING.md` - Test procedures
- `PHASES_SUMMARY.md` - What's working
- `Known_Issues/` - Known bugs

### 👔 Project Manager

**Start with:**

- `ROADMAP.md` - Timeline and dependencies
- `MACOS_PORT_DIARY.md` - Progress tracking
- `PHASES_SUMMARY.md` - Current status

### 🤖 AI Agent (Copilot)

**Context files (in order):**

1. `.github/copilot-instructions.md` - **PRIMARY**
2. `.github/instructions/project.instructions.md` - Build specifics
3. This file - Documentation structure
4. Task-specific phase docs

## 🎯 Find Information By Topic

### Graphics & Rendering

- `PHASE50/` - Current Vulkan architecture
- `Misc/CRITICAL_FIXES.md` - Graphics debugging
- `references/fighter19-dxvk-port/` - Vulkan templates
- `PHASES_ARCHIVED/PHASE48/` - Vulkan validation reference

### Audio System

- `Misc/AUDIO_BACKEND_STATUS.md` - Audio implementation status
- Will be implemented in Phase 51+

### Game Assets

- `Misc/BIG_FILES_REFERENCE.md` - .big archive structure
- `PHASES_ARCHIVED/PHASE28/` - Texture loading (historical)

### Cross-Platform Support

- `MACOS_BUILD_INSTRUCTIONS.md` - macOS specific
- `LINUX_BUILD_INSTRUCTIONS.md` - Linux specific
- All build presets in `.github/instructions/project.instructions.md`

### Project Architecture

- `.github/copilot-instructions.md` - Three-layer system architecture
- `PHASE50/` - Current Vulkan refactor approach
- `Misc/LESSONS_LEARNED.md` - Architectural decisions

## 🔍 Common Questions

**Q: How do I build the project?**
→ `MACOS_BUILD_INSTRUCTIONS.md` or `.github/instructions/project.instructions.md`

**Q: What's the current status?**
→ `MACOS_PORT_DIARY.md` (always up-to-date)

**Q: How does the graphics system work?**
→ `PHASE50/VULKAN_ONLY_CLEAN_REFACTOR.md` + `.github/copilot-instructions.md`

**Q: Why were certain design decisions made?**
→ `Misc/LESSONS_LEARNED.md` or relevant PHASES_ARCHIVED phase

**Q: What are known issues?**
→ `Known_Issues/` directory

**Q: How do I run tests?**
→ `TESTING.md`

## Directory Structure

```
docs/
├── README.md                    # Project overview
├── START_HERE.md                # New developer entry point
├── ROADMAP.md                   # Development roadmap
├── PHASES_SUMMARY.md            # Quick phase status
├── MACOS_PORT_DIARY.md          # Development log (ALWAYS UPDATE)
├── DOCUMENTATION_GUIDE.md       # This file
│
├── MACOS_BUILD_INSTRUCTIONS.md
├── LINUX_BUILD_INSTRUCTIONS.md
├── TESTING.md
├── REFERENCE_REPOSITORIES.md
├── READING_GUIDE.md
│
├── PHASE49/                     # Reference implementation
├── PHASE50/                     # Current: Vulkan-Only Refactor
│
├── PHASES_ARCHIVED/             # Phase 38-48 history
├── Misc/                        # Lessons, fixes, references
├── Known_Issues/                # Bug tracking
├── Vulkan/                      # Vulkan SDK docs
└── metal_port/                  # Historical Metal work
```

## ⚠️ Important Notes

### MACOS_PORT_DIARY.md is CRITICAL

- Always up-to-date development status
- Update at end of every session
- Source of truth for what's been done

### Phase 50 is Current Focus

- Vulkan-Only clean refactor (started from upstream)
- Removed Metal backend completely
- Platform-conditional (not graphics-backend conditional)
- Based on fighter19-dxvk-port architecture

### Archived Phases (38-48) Are Reference Only

- Historical value: shows architectural evolution
- Lessons learned documented in `Misc/LESSONS_LEARNED.md`
- Use for understanding past decisions, not current work

### Reference Repositories

- Compare with `references/fighter19-dxvk-port/` for Vulkan patterns
- Use `references/jmarshall-win64-modern/` for cross-platform issues
- Compare with `references/fbraz3-generalsX-main/` for fork progress

## 🚀 Quick Start Checklist

- [ ] Read this file (you are here)
- [ ] Read `START_HERE.md`
- [ ] Read `MACOS_BUILD_INSTRUCTIONS.md` or `LINUX_BUILD_INSTRUCTIONS.md`
- [ ] Read `.github/copilot-instructions.md`
- [ ] Read `PHASE50/VULKAN_ONLY_CLEAN_REFACTOR.md`
- [ ] Run: `cmake --preset macos-arm64 && cmake --build build/macos-arm64 -j 4`
- [ ] Ready to contribute!

## 📞 Need Help?

1. Check `Known_Issues/` for your problem
2. Check `Misc/CRITICAL_FIXES.md` for emergency solutions
3. Check `Misc/LESSONS_LEARNED.md` for architectural context
4. Check `MACOS_PORT_DIARY.md` for latest status
5. Review relevant phase documentation
