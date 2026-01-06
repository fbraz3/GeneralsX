# Quick Start: Phase 40 Implementation

**Before you begin Phase 40, read this first.**

---

## What is Phase 40?

**Objective**: Remove all Win32 APIs from game code, making SDL2 the exclusive cross-platform abstraction layer.

**Duration**: 3-4 weeks of focused work

**Expected Outcome**: SDL2-only codebase that compiles identically on Windows, macOS, and Linux.

---

## Quick Facts

- **Total Documentation**: `docs/WORKDIR/phases/4/PHASE40/README.md` (507 lines)
- **Build Targets**: `GeneralsXZH` (primary), `GeneralsX` (secondary)
- **Platforms**: Windows, macOS ARM64, macOS Intel, Linux
- **No Code Changes Yet**: Phase 40 is not started until you run the audit

---

## How to Start Phase 40

### Step 1: Read the Documentation

```bash
# Read this first (quick overview)
open docs/WORKDIR/phases/4/PHASE40/INDEX.md

# Then read the main documentation
open docs/WORKDIR/phases/4/PHASE40/README.md

# Reference for session context
open docs/WORKDIR/phases/4/PHASE40/SESSION_SUMMARY.md
```

### Step 2: Run the Win32 API Audit (Day 1)

```bash
# Find all #ifdef _WIN32 checks
grep -r "#ifdef _WIN32" Generals/ GeneralsMD/ Core/GameEngine/ --include="*.cpp" --include="*.h" 2>/dev/null | tee logs/phase40_win32_ifdef.log

# Find all #include <windows.h>
grep -r "#include <windows.h>" Generals/ GeneralsMD/ Core/GameEngine/ --include="*.cpp" --include="*.h" 2>/dev/null | tee logs/phase40_windows_h.log

# Find Win32 API calls
grep -r "hwnd\|HWND\|GetModuleFileName\|RegOpenKey" Generals/ GeneralsMD/ Core/GameEngine/ --include="*.cpp" --include="*.h" 2>/dev/null | tee logs/phase40_win32_api.log
```

### Step 3: Analyze Results

```bash
# Count findings
wc -l logs/phase40_win32_ifdef.log logs/phase40_windows_h.log logs/phase40_win32_api.log

# Review in detail
grep -l "hwnd" logs/phase40_win32_api.log | head -20
```

### Step 4: Create Your Task List

Based on audit results, create a detailed breakdown of:
- Which files need modification
- Which Win32 APIs need replacement
- Which SDL2 APIs to use instead

---

## Key Files to Modify (Expected)

These are the files you'll likely touch in Phase 40:

```
Generals/Code/GameEngine/GameLogic/GameClient/WinMain.cpp
GeneralsMD/Code/GameEngine/GameLogic/GameClient/WinMain.cpp
Core/GameEngine/[various initialization files]
Any file with #ifdef _WIN32 in game code directories
```

**NOT Modifying**:
- `Core/Libraries/Source/WWVegas/win32_compat.h` (system layer, keep)
- Build system files (CMakeLists.txt - changes may be needed but not primary focus)
- External dependencies

---

## Build Commands (Keep These Handy)

### Quick Build (macOS ARM64)

```bash
cmake --build build/macos-arm64-vulkan --target z_generals -j 4 2>&1 | tee logs/phase40_build.log
```

### Full Clean Build (macOS ARM64)

```bash
rm -rf build/macos-arm64-vulkan
cmake --preset macos-arm64-vulkan
cmake --build build/macos-arm64-vulkan --target z_generals -j 4 2>&1 | tee logs/phase40_clean_build.log
```

### All Platforms

```bash
# macOS ARM64
cmake --preset macos-arm64-vulkan
cmake --build build/macos-arm64-vulkan --target z_generals -j 4 2>&1 | tee logs/phase40_mac_arm64.log

# macOS Intel
cmake --preset macos-x64
cmake --build build/macos-x64 --target z_generals -j 4 2>&1 | tee logs/phase40_mac_x64.log

# Linux
cmake --preset linux
cmake --build build/linux --target z_generals -j 4 2>&1 | tee logs/phase40_linux.log

# Windows VC6
cmake --preset vc6
cmake --build build/vc6 --target z_generals -j 4 2>&1 | tee logs/phase40_windows.log
```

---

## Important Rules for Phase 40

1. **Never skip logging**: Always use `2>&1 | tee logs/` for builds
2. **Never rush changes**: Read each file completely before modifying
3. **Never defer issues**: If you find a problem, fix it immediately
4. **Never commit incomplete work**: Each commit is a complete component
5. **Always test builds**: After each major change, compile at least one platform
6. **Always verify**: Run the game after each modification phase

---

## Phase 40 Week-by-Week

### Week 1: Audit & Identification
- Day 1-2: Run audit commands above
- Day 3-4: Categorize findings
- Day 5: Create removal strategy

### Week 2: Core API Replacement
- Replace hwnd with SDL_Window*
- Replace Registry with INI files
- Replace path operations with SDL2

### Week 3: System API Replacement
- Replace threading (if needed)
- Replace timing (Sleep, GetTickCount)
- Replace keyboard/mouse (verify Phase 39.5 covered this)

### Week 4: Cleanup & Verification
- Remove all #ifdef _WIN32 from game code
- Cross-platform testing (Windows/macOS/Linux)
- Final validation

---

## Success Criteria for Phase 40

When you complete Phase 40, all of these must be TRUE:

- [ ] Zero `#ifdef _WIN32` in `Generals/Code/` or `GeneralsMD/Code/`
- [ ] Zero `#include <windows.h>` in game code
- [ ] All Registry operations replaced with file-based config
- [ ] SDL2 used for all window management
- [ ] Game compiles on Windows
- [ ] Game compiles on macOS ARM64
- [ ] Game compiles on macOS Intel (if available)
- [ ] Game compiles on Linux
- [ ] Game runs without crashes on all platforms
- [ ] Input works (keyboard/mouse)
- [ ] Configuration persists (INI files working)

---

## If You Get Stuck

### Issue: "Can't find where hwnd is used"

```bash
grep -r "hwnd" --include="*.cpp" --include="*.h" | grep -v "win32_compat"
```

### Issue: "Registry code scattered everywhere"

```bash
grep -r "RegOpenKey\|RegQueryValue\|RegSetValue" --include="*.cpp" --include="*.h"
```

### Issue: "Build fails after changes"

1. Check the full error: `tail -100 logs/phase40_build.log`
2. Understand what's missing
3. Add equivalent SDL2 call or fix the code
4. Rebuild with `2>&1 | tee logs/phase40_build_retry.log`

### Issue: "Game crashes on startup"

1. Check crash log: `$HOME/Documents/Command\ and\ Conquer\ Generals\ Zero\ Hour\ Data/ReleaseCrashInfo.txt`
2. Identify the failing operation
3. Verify SDL2 equivalent is called
4. Test with debug logging

---

## Resources at Your Fingertips

### Documentation
- Main guide: `docs/WORKDIR/phases/4/PHASE40/README.md`
- Overview: `docs/WORKDIR/phases/4/PHASE40/INDEX.md`
- Session notes: `docs/WORKDIR/phases/4/PHASE40/SESSION_SUMMARY.md`
- Diary: `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)`

### SDL2 API Reference
- https://wiki.libsdl.org/

### Project References
- `.github/copilot-instructions.md` (project context)
- `.github/instructions/project.instructions.md` (build procedures)
- `CMakeLists.txt` (build configuration)

---

## Before Each Work Session

1. **Update from upstream**:
   ```bash
   git fetch thesupehackers
   git merge thesupehackers/main
   ```

2. **Review documentation**:
   - Reread relevant section of Phase 40 README
   - Check today's planned tasks

3. **Ensure no stale builds**:
   ```bash
   # Optional: clean build if issues
   rm -rf build/macos-arm64-vulkan
   cmake --preset macos-arm64-vulkan
   ```

4. **Check logs from previous session**:
   ```bash
   ls -lhtr logs/phase40_* | tail -5
   ```

---

## After Each Work Session

1. **Commit your changes**:
   ```bash
   git add .
   git commit -m "feat(phase40): [specific change description]

   - What changed
   - Why it changed
   - What was tested"
   ```

2. **Update the diary**:
   - Add entry to `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)`
   - Note progress and any discoveries

3. **Log session summary**:
   ```bash
   echo "Phase 40 Progress: [summary]" >> logs/phase40_session_log.txt
   ```

---

## Phase 40 Success = Phase 41 Ready

When Phase 40 is complete:
- SDL2 is the only cross-platform layer
- Game code has no platform-specific conditionals
- Foundation is set for Phase 41 (graphics driver architecture)

---

**Ready to start Phase 40?**

→ Read `docs/WORKDIR/phases/4/PHASE40/README.md` now  
→ Run the audit commands  
→ Follow the week-by-week plan  
→ Mark each item as complete  

**Good luck!**
