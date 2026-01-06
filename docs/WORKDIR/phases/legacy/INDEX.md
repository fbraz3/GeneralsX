# 📚 GeneralsX Documentation Index

**Quick Start for Project Orientation**

## 🎯 Your Starting Point

**If you're new to this project, read in this order:**

1. **THIS FILE** - You're reading it
2. `docs/PHASES_SUMMARY.md` - 2-minute overview
3. `docs/ROADMAP.md` - Full project roadmap  
4. `docs/PHASE_ROADMAP_V2.md` - **NEW: DXVK Hybrid Architecture** ← Read this for latest strategy
5. `docs/WORKDIR/phases/3/PHASE37/README.md` - Current work (Phase 37)

## 🗂️ Phase Documentation

### ✅ Completed Phases (27-36)

These are done - use as reference only.

- `docs/WORKDIR/27/PHASE27/` - W3D Graphics Engine analysis
- `docs/WORKDIR/28/PHASE28/` - Texture loading (DDS/TGA)
- `docs/WORKDIR/29/PHASE29/` - Metal render states
- `docs/WORKDIR/phases/3/PHASE30/` - Metal backend implementation
- `docs/WORKDIR/phases/3/PHASE31/` - Texture system integration
- `docs/WORKDIR/phases/3/PHASE32/` - Audio investigation
- `docs/WORKDIR/phases/3/PHASE33/` - OpenAL implementation
- `docs/WORKDIR/phases/3/PHASE34/` - Game logic study
- `docs/WORKDIR/phases/3/PHASE35/` - Code cleanup
- `docs/WORKDIR/phases/3/PHASE36/` - Upstream merge

### 🚀 Current/Next Phases (37-40)

**Start here:**

- `docs/WORKDIR/phases/3/PHASE37/README.md` - **START NOW** Asset loading diagnostics
- `docs/WORKDIR/phases/3/PHASE38/README.md` - Audio system validation
- `docs/WORKDIR/phases/3/PHASE39/README.md` - UI/menu system
- `docs/WORKDIR/phases/4/PHASE40/README.md` - Game logic & gameplay

### 🌐 Future Phase (XX)

Final stretch goal after Phase 40.

- `docs/PHASE_XX/README.md` - Multiplayer & networking
- `docs/PHASE_XX_POSITIONING.md` - How XX fits in timeline

## 📖 Reference Documentation

### Core References

| Document | Purpose | Read When |
|----------|---------|-----------|
| `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)` | Development diary | Always up-to-date status |
| `docs/ROADMAP.md` | Full project roadmap | Understanding big picture |
| `docs/PHASE_ROADMAP_V2.md` | **NEW: DXVK hybrid strategy** | **Latest architecture decisions** |
| `docs/PHASES_SUMMARY.md` | Quick reference | Quick orientation |
| `docs/README.md` | Project overview | First time reading |

### Technical References

| Document | Topic | Use For |
|----------|-------|---------|
| `docs/WORKDIR/support/BIG_FILES_REFERENCE.md` | .big archive structure | Asset loading issues |
| `docs/WORKDIR/support/GRAPHICS_BACKENDS.md` | Metal vs OpenGL | Graphics questions |
| `docs/WORKDIR/support/AUDIO_BACKEND_STATUS.md` | Audio system | Audio debugging |
| `docs/WORKDIR/lessons/LESSONS_LEARNED.md` | Anti-patterns | Understanding past bugs |
| `docs/WORKDIR/support/CRITICAL_FIXES.md` | Emergency fixes | Last-resort troubleshooting |

### Build/Setup References

| Document | Topic | Use For |
|----------|-------|---------|
| `docs/MACOS_BUILD_INSTRUCTIONS.md` | macOS build | Building on Mac |
| `docs/LINUX_BUILD_INSTRUCTIONS.md` | Linux build | Building on Linux |
| `docs/TESTING.md` | Test procedures | Running tests |
| `.github/instructions/project.instructions.md` | Compilation basics | Build commands |

## 🎯 By Role

### 👨‍💻 Software Developer (Working on Code)

**Read first:**

1. `docs/WORKDIR/phases/3/PHASE37/README.md` - Your next task
2. `.github/copilot-instructions.md` - Project context
3. `.github/instructions/project.instructions.md` - Build commands

**Reference during work:**

- `docs/WORKDIR/lessons/LESSONS_LEARNED.md` - Avoid known pitfalls
- `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)` - Current status
- Relevant PHASE*/README.md - Current phase details

### 📊 Project Manager (Planning & Status)

**Read first:**

1. `docs/ROADMAP.md` - Timeline & dependencies
2. `docs/PHASES_SUMMARY.md` - Current status
3. `docs/PHASE_XX_POSITIONING.md` - How XX fits in

**Reference for:**

- `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)` - Weekly progress tracking
- Phase READMEs - Task breakdown per phase

### 🧪 QA/Tester (Testing Features)

**Read first:**

1. `docs/TESTING.md` - Test procedures
2. `docs/WORKDIR/phases/3/PHASE37/README.md` - Current phase goals
3. `docs/PHASES_SUMMARY.md` - What's working vs not

**Reference for:**

- `docs/KNOWN_ISSUES/` - Known bugs to avoid
- `docs/WORKDIR/support/CRITICAL_FIXES.md` - Workarounds

### 🤖 AI Agent (Copilot/ChatGPT)

**Context files:**

1. `.github/copilot-instructions.md` - **PRIMARY** - Read first
2. `.github/instructions/project.instructions.md` - Build guidelines
3. This file - Documentation index
4. Relevant PHASE*/README.md - Task specifics

## 🔍 Find Information By Topic

### Graphics/Rendering

- Phase 28-31: Texture system
- Phase 29: Metal render states  
- Phase 30: Metal backend
- `docs/WORKDIR/support/GRAPHICS_BACKENDS.md` - Graphics architecture

### Audio

- Phase 32: Audio investigation
- Phase 33: OpenAL implementation
- Phase 38: Audio validation
- `docs/WORKDIR/support/AUDIO_BACKEND_STATUS.md` - Audio details

### Game Logic

- Phase 34: Game logic analysis
- Phase 40: Gameplay implementation
- Game logic code in `GeneralsMD/Code/GameEngine/`

### Multiplayer

- Phase XX: Multiplayer & networking
- `docs/PHASE_XX_POSITIONING.md` - When/how it's done

### Cross-Platform

- All phases: macOS, Linux, Windows support
- `docs/MACOS_BUILD_INSTRUCTIONS.md` - macOS specific
- `docs/LINUX_BUILD_INSTRUCTIONS.md` - Linux specific

### Asset Loading

- Phase 28: Texture loading
- Phase 31: Texture integration
- Phase 37: Asset loading diagnostics
- `docs/WORKDIR/support/BIG_FILES_REFERENCE.md` - Asset structure

## 📋 Quick Command Reference

### Build

```bash
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

### Run

```bash
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/run.log
```

### Test

```bash
cd /Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode
bash run_tests.sh
```

## 🎓 Learning Path

### Level 1: Orientation

1. Read `docs/README.md`
2. Read `docs/PHASES_SUMMARY.md`
3. Read `docs/WORKDIR/phases/3/PHASE37/README.md`

**Time: 15 minutes**

### Level 2: Deep Dive

1. Read `docs/ROADMAP.md`
2. Read `.github/copilot-instructions.md`
3. Read Phase-specific docs (current work)

**Time: 45 minutes**

### Level 3: Expert

1. Read all PHASE docs (27-40)
2. Read `docs/WORKDIR/lessons/LESSONS_LEARNED.md`
3. Study relevant source files
4. Understand reference repositories

**Time: Several hours**

## 🆘 Common Situations

### "I don't know where to start"

→ Read `docs/PHASES_SUMMARY.md` (2 min)  
→ Read `docs/WORKDIR/phases/3/PHASE37/README.md` (15 min)  
→ Start Phase 37.1

### "Game only shows blue screen"

→ Read `docs/WORKDIR/phases/3/PHASE37/README.md`  
→ This is BLOCKED on asset loading (Phase 37)

### "Build fails with error"

→ Check `.github/instructions/project.instructions.md` - Compilation section  
→ Check `docs/KNOWN_ISSUES/` for similar issues

### "Where's the audio?"

→ Read `docs/WORKDIR/phases/3/PHASE38/README.md`  
→ Audio system exists but needs validation (Phase 38)

### "How do I test multiplayer?"

→ Read `docs/PHASE_XX_POSITIONING.md`  
→ Multiplayer is Phase XX (after Phase 40)

## 📅 Timeline At a Glance

```
Oct 27:  Phase organization (TODAY)
Nov 1-3:   Phase 37 - Asset loading
Nov 4-7:   Phase 38 - Audio validation
Nov 8-12:  Phase 39 - Menu system
Nov 13-19: Phase 40 - Gameplay
Nov 20:    PLAYABLE GAME 🎮
Dec 10-30: Phase XX - Multiplayer
Dec 30:    COMPLETE GAME 🌐
```

## 💾 File Structure

```
docs/
├── README.md                    ← Project overview
├── ROADMAP.md                   ← Full timeline
├── PHASES_SUMMARY.md            ← Quick reference
├── PHASE_XX_POSITIONING.md      ← Where Phase XX fits
├── MACOS_PORT_DIARY.md          ← Development log
├── MACOS_BUILD_INSTRUCTIONS.md  ← macOS setup
├── LINUX_BUILD_INSTRUCTIONS.md  ← Linux setup
├── TESTING.md                   ← Test procedures
│
├── PHASE27/ through PHASE40/    ← Phase documentation
├── PHASE_XX/                    ← Multiplayer phase
├── KNOWN_ISSUES/                ← Bug reference
└── Misc/
    ├── BIG_FILES_REFERENCE.md
    ├── GRAPHICS_BACKENDS.md
    ├── AUDIO_BACKEND_STATUS.md
    ├── LESSONS_LEARNED.md
    └── CRITICAL_FIXES.md
```

---

## 🚀 Ready to Work?

### For Development on Phase 37

1. cd to project root
2. Read `docs/WORKDIR/phases/3/PHASE37/README.md` carefully
3. Open the files mentioned in Phase 37.1
4. Start adding logging to texture pipeline

### Questions?

Check `docs/PHASE_XX_POSITIONING.md` or other relevant docs above.

---

**Last Updated**: October 27, 2025  
**Status**: Organization Complete - Ready for Phase 37
