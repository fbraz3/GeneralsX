# Additional Work Items - PHASE 04 Extension & Future Phases

**Date**: 2026-01-12  
**Status**: Planning & Prioritization  

---

## Overview

After completing PHASE 04 Tasks 1-3, the following work items are needed:

1. **PHASE 04 Task 4**: Clipboard & File Dialogs (optional SDL2 abstraction)
2. **PHASE 04+ Code Review**: Comprehensive review of all SDL2 implementations
3. **LZHL Fix Investigation**: Resolve pre-existing compilation blocker
4. **Merge Strategy**: How to handle future upstream pulls from thesuperhackers

---

## PHASE 04 Task 4: Clipboard & File Dialogs (Optional)

### Status
🟡 **Not Started** - Low priority (optional)  
⏱️ **Estimate**: 1-2 hours  
🎯 **Depends on**: None (independent)

### Objective
Create SDL2 wrapper for clipboard and file dialog operations (if needed for game).

### Scope
**Clipboard Operations**:
- Get text from clipboard
- Set text to clipboard
- Get file paths from clipboard (drag-drop)

**File Dialogs** (Optional):
- Open file dialog
- Save file dialog
- Directory picker dialog

### Win32 APIs Affected
- `GetClipboardData()` / `SetClipboardData()`
- `OpenClipboard()` / `CloseClipboard()`
- `GetOpenFileName()` / `GetSaveFileName()` (Common Dialog)
- `SHBrowseForFolder()` (Shell)

### SDL2 Equivalents
- **Clipboard**: `SDL_GetClipboardText()` / `SDL_SetClipboardText()`
- **File Dialogs**: No SDL2 equivalent (use platform-specific or tinyfiledialogs library)

### Decision Points
1. **Is clipboard used in game?** → Check codebase for clipboard references
2. **Do we need file dialogs?** → Only if map editor or modding tools required
3. **Dependency on external library?** → tinyfiledialogs is lightweight alternative

### Implementation Approach
1. Create `SDL2Clipboard.h` (abstract interface)
2. Create `SDL2ClipboardImpl.cpp` (SDL2 implementation)
3. Create `ClipboardSystem.h/cpp` (lifecycle management)
4. (Optional) Create `SDL2FileDialog.h` with platform-specific implementations

### Decision: Skip for Now
**Reasoning**: 
- Clipboard likely not used during skirmish/campaign gameplay
- File dialogs only needed for map editor (future tool)
- Can be added later without blocking primary goal

---

## PHASE 04+ Code Review & Integration

### Status
🟡 **Ready to Start** - All PHASE 04 Tasks 1-3 complete  
⏱️ **Estimate**: 2-3 hours  
🎯 **Depends on**: PHASE 04 Tasks 1-3 (done)

### Objective
Comprehensive review of all SDL2 implementations for quality, consistency, and integration readiness.

### Review Checklist

#### Code Quality
- [ ] All files follow C++ style guide (consistent indentation, naming)
- [ ] No compiler warnings (when compiled with -Wall -Wextra)
- [ ] Error handling is defensive (null-checks, bounds checking)
- [ ] Memory management is sound (no leaks, proper cleanup)
- [ ] Comments are clear and comprehensive
- [ ] No TODO or FIXME comments left behind

#### Architecture Consistency
- [ ] All systems follow singleton pattern correctly
- [ ] Lifecycle management (create/initialize/shutdown/destroy) consistent
- [ ] Global extern declarations in headers
- [ ] Proper use of const, virtual, override
- [ ] No circular dependencies between modules

#### Integration Points
- [ ] SDL2GameEngine properly initializes all subsystems
- [ ] Event handlers properly integrated in serviceSDL2OS()
- [ ] No Win32-specific code in SDL2Device implementations
- [ ] Path handling works for Windows/macOS/Linux
- [ ] CMakeLists.txt includes all new files

#### Cross-Platform Readiness
- [ ] No hardcoded path separators (use PATH_SEP or forward slash)
- [ ] No Win32 API calls in new code
- [ ] No platform-specific #include statements
- [ ] All external dependencies available on Windows/macOS/Linux

#### Documentation
- [ ] Public API documented (parameter descriptions, return values)
- [ ] Design decisions documented
- [ ] Known limitations documented
- [ ] Usage examples provided (in comments or separate doc)

### Files to Review
- [ ] `Core/GameEngine/Include/Common/GameTiming.h` (344 lines)
- [ ] `Core/GameEngineDevice/Include/SDL2Device/Common/SDL2GameTiming.h` (98 lines)
- [ ] `Core/GameEngineDevice/Source/SDL2Device/Common/SDL2GameTiming.cpp` (284 lines)
- [ ] `Core/GameEngineDevice/Include/SDL2Device/Common/GameTimingSystem.h` (32 lines)
- [ ] `Core/GameEngineDevice/Source/SDL2Device/Common/GameTimingSystem.cpp` (71 lines)
- [ ] `Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Window.h` (277 lines)
- [ ] `Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Window.cpp` (426 lines)
- [ ] `Core/GameEngineDevice/Include/SDL2Device/GameClient/WindowSystem.h` (60 lines)
- [ ] `Core/GameEngineDevice/Source/SDL2Device/GameClient/WindowSystem.cpp` (41 lines)
- [ ] (From PHASE 03) SDL2Mouse, SDL2Keyboard, SDL2IMEManager

### Output Artifacts
- [ ] Code Review Report (findings + recommendations)
- [ ] Updated documentation if issues found
- [ ] List of integration blockers (if any)

---

## LZHL Compilation Failure Investigation

### Status
🔴 **Blocked** - Pre-existing issue, blocking PHASE 03 Task 4  
⏱️ **Estimate**: 2-4 hours (investigation), 1-2 hours (fix if found)  
🎯 **Blocker for**: Integration testing, executable build

### Problem Statement
**Error**:
```
FAILED: CMakeFiles/liblzhl.dir/_deps/lzhl-src/CompLibHeader/Huff.cpp.obj
C:\VC6\VC6SP6\VC98\BIN\cl.exe - compilation error
ninja: build stopped: subcommand failed
```

**Impact**:
- Cannot build generalszh.exe or generalsv.exe
- Prevents PHASE 03 Task 4 (Integration Testing)
- Blocks executable testing for all new code

### Investigation Plan

**Step 1**: Check if it's a known issue (30 min)
- [ ] Search LZHL repository for similar errors
- [ ] Check if VC6 patch needed (LZHL designed for modern compilers)
- [ ] Review CMakeLists.txt for LZHL configuration

**Step 2**: Examine error details (1 hour)
- [ ] Run build with verbose output: `cmake --build . --verbose`
- [ ] Capture full compiler error message
- [ ] Check Huff.cpp source code for syntax issues
- [ ] Verify include paths and dependencies

**Step 3**: Try solutions (1-2 hours)
- [ ] Update LZHL to latest version
- [ ] Disable LZHL temporarily and use stub (allow build to proceed)
- [ ] Apply VC6-specific patches if available
- [ ] Check if it's a parallel build issue (-j 1 vs -j 4)

**Step 4**: Document solution (30 min)
- [ ] Create LZHL_FIX.md with findings
- [ ] Document workaround if no permanent fix found

### Alternative Approach
If LZHL cannot be fixed quickly:
1. **Stub compression**: Create fake LZHL lib that doesn't compress (allows game load)
2. **Defer compression**: Focus on game logic, compress assets later
3. **Use alternative**: Check if there's a replacement compression library

### Success Criteria
- [ ] LZHL compiles without errors
- [ ] generalszh.exe builds successfully
- [ ] generalszh.exe launches without crashes
- [ ] Fix is documented for future reference

---

## Merge Strategy: Handle Upstream Pulls

### Problem
When pulling from `thesuperhackers/main`, we risk losing local SDL2 work.

### Solution: Create Integration Branch

**Approach**:
1. **Main branches** (in parallel):
   - `vanilla-test` - Our SDL2 work (primary)
   - `upstream-track` - Tracking thesuperhackers/main (secondary)

2. **Merge strategy**:
   - Pull thesuperhackers/main into `upstream-track`
   - Review changes (what was added/fixed upstream)
   - Selectively merge upstream improvements into `vanilla-test`
   - Avoid wholesale overwrites

3. **Conflict resolution**:
   - Use `git merge --no-commit` to preview conflicts
   - Manually resolve using `ours` or `theirs` strategy
   - Test build after each merge
   - Commit only if everything works

### Branching Plan
```
upstream-track ←  thesuperhackers/main (track upstream)
    ↓
  (review)
    ↓
vanilla-test   ← selective cherry-picks or merges
```

### Prevention Checklist
- [ ] Always merge with `--no-commit` first
- [ ] Review `git diff --name-only` before accepting
- [ ] Test build after merge
- [ ] Back up vanilla-test before merging
- [ ] Document any conflicts resolved

---

## Prioritization Matrix

| Item | Priority | Impact | Effort | Timeline |
|------|----------|--------|--------|----------|
| PHASE 04 Task 4 | 🟢 Low | Features | 1-2h | Optional |
| Code Review | 🟡 Medium | Quality | 2-3h | Immediate |
| LZHL Fix | 🔴 High | Blocker | 2-4h | ASAP |
| Merge Strategy | 🟡 Medium | Process | 1h | Setup now |

---

## Recommended Next Steps

### Immediate (Next 1-2 hours)
1. **Start LZHL Investigation**
   - Run build with verbose output
   - Capture exact error
   - Check for known issues

2. **Set up Merge Strategy**
   - Create `upstream-track` branch
   - Document merge procedures
   - Test merge with thesuperhackers/main

### Short-term (Next 2-3 hours)
3. **Code Review & Quality Check**
   - Go through PHASE 04 Task 1-2 implementations
   - Fix any issues found
   - Ensure cross-platform readiness

4. **Optional: PHASE 04 Task 4**
   - Assess if clipboard/dialogs needed
   - If yes, implement quickly
   - If no, skip to next phase

### Medium-term (After resolution)
5. **Integration Testing** (when LZHL fixed)
   - Build executables
   - Test input systems
   - Validate gameplay

---

## Success Criteria for All Items

✅ **PHASE 04 Task 4**: (If implemented) Clipboard working + documentation  
✅ **Code Review**: No critical issues found, documentation updated  
✅ **LZHL Fix**: Executables build successfully  
✅ **Merge Strategy**: Documented and tested  

---

**Status**: Planning complete - Ready for execution  
**Next**: Choose which item to tackle first
