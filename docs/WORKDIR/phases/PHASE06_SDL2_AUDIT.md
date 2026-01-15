# PHASE 06: SDL2 Implementation Audit

**Status**: Planning  
**Start Date**: 2026-01-15  
**Baseline**: Phase 05 Complete (VC6 32-bit, SDL2 + DirectX8 + Miles Audio)

---

## 🎯 Phase 06 Goal

Verify that game runs 100% on SDL2 abstraction layer. Any Win32 API calls in input/window management must be identified and rerouted through SDL2.

**Success Criteria**:
- [x] Zero direct Win32 API calls in input/window code
- [x] All events routed through SDL2
- [x] Game runs identically with no regression
- [x] Code audit documented and reviewed

---

## 📋 Detailed Roadmap

### 06.1: Map Current SDL2 Integration
**Effort**: 2 days  
**Owner**: Lead Architect

**Tasks**:
- [ ] Search codebase for SDL2 usage patterns
  ```bash
  grep -r "SDL_" GeneralsMD/Code/GameEngineDevice/ | wc -l
  grep -r "CreateWindow\|GetMessage\|DispatchMessage" GeneralsMD/Code --include="*.cpp" | grep -v "//"
  ```
- [ ] Document all SDL2 wrappers currently implemented:
  - [ ] SDL2 window creation
  - [ ] SDL2 event handling (keyboard, mouse)
  - [ ] SDL2 input callbacks
  - [ ] SDL2 timer functions
  
- [ ] Create matrix: "Win32 API vs SDL2 Alternative"
  - Current state: Which are already abstracted
  - Gaps: Which Win32 APIs bypass SDL2

**Deliverable**: `docs/WORKDIR/support/SDL2_AUDIT_REPORT.md` (detailed findings)

---

### 06.2: Identify Win32 Leakage
**Effort**: 3 days  
**Owner**: Code Reviewer

**Search for direct Win32 API calls in these modules**:

1. **Input System** (`GameEngineDevice/Source/Win32Device/GameClient/`)
   - [ ] Win32DIKeyboard.cpp - Check for DirectInput calls that bypass SDL2
   - [ ] Win32DIMouse.cpp - Check for DirectInput calls that bypass SDL2
   - [ ] Win32Mouse.cpp - Check for Windows cursor APIs
   
2. **Window Management** (`Core/GameEngineDevice/` + `Main/`)
   - [ ] Win32GameEngine.cpp - Check for CreateWindow, etc
   - [ ] Win32OSDisplay.cpp - Check for raw Win32 display calls
   
3. **Message Loop** (`Main/WinMain.cpp`)
   - [ ] GetMessage, DispatchMessage - Should be SDL2 event loop
   - [ ] PeekMessage - Should be SDL_PollEvent
   
4. **File System** (Already abstracted, but verify)
   - [ ] Win32LocalFileSystem.cpp - Should only do file I/O, not window stuff
   
5. **Audio System** (To be replaced in Phase 07)
   - [ ] Current Miles Audio calls - Document for removal

**Deliverable**: `docs/WORKDIR/support/SDL2_LEAKAGE_INVENTORY.md`
- List of all Win32 calls found
- Severity (critical vs informational)
- Suggested SDL2 replacement

---

### 06.3: Create SDL2 Wrapper Functions (if gaps exist)
**Effort**: 2-3 days  
**Owner**: SDL2 Specialist

**For each Win32 leakage found, create SDL2 wrapper**:

Example: If raw `GetCursorPos()` found:
```cpp
// GameEngineDevice/Include/SDL2Device/Common/SDL2Input.h
void SDL2_GetCursorPosition(int* outX, int* outY);

// GameEngineDevice/Source/SDL2Device/Common/SDL2Input.cpp
void SDL2_GetCursorPosition(int* outX, int* outY) {
    SDL_GetMouseState(outX, outY);
}
```

**Create wrapper headers for**:
- [ ] SDL2Input.h - Keyboard/Mouse input
- [ ] SDL2Window.h - Window management  
- [ ] SDL2Display.h - Display/cursor control
- [ ] SDL2Time.h - Timing (if not already done)

**Deliverable**: Complete SDL2 wrapper layer for all identified gaps

---

### 06.4: Build & Validate
**Effort**: 1 day  
**Owner**: Build Engineer

**Tasks**:
- [ ] Clean build: `cmake --preset vc6 && cmake --build build/vc6`
- [ ] Verify compilation clean (0 errors)
- [ ] Run game: `GeneralsXZH.exe -win -noshellmap`
- [ ] Test scenarios:
  - [ ] Navigate main menu (keyboard + mouse)
  - [ ] Start skirmish game
  - [ ] Click units (mouse input works)
  - [ ] Press ESC to quit (keyboard works)

**Deliverable**: `logs/phase06_validation.log` - All tests pass ✅

---

### 06.5: Documentation & Validation Report
**Effort**: 1 day  
**Owner**: Documentation Lead

**Create**:
- [ ] `docs/WORKDIR/support/SDL2_IMPLEMENTATION_COMPLETE.md`
  - Summary of audit findings
  - All Win32 leakage fixed
  - Validation results
  - Architecture diagram (Win32Device → SDL2 abstraction)

**Deliverable**: Documentation + passing validation tests

---

## 📅 Timeline Summary

| Task | Duration | Deliverable |
|------|----------|-------------|
| **06.1** | 2 days | SDL2 integration map |
| **06.2** | 3 days | Win32 leakage inventory |
| **06.3** | 2-3 days | SDL2 wrapper functions |
| **06.4** | 1 day | Validation tests pass |
| **06.5** | 1 day | Documentation + Report |
| **TOTAL** | **~10 days** | **SDL2 100% validated** ✅ |

---

## 🚀 Next Phase (Phase 07): OpenAL Audio Implementation

Once Phase 06 complete:
- Audio subsystem will use same abstraction pattern as window/input
- Ready for Phase 07 OpenAL migration

---

**Prepared by**: GeneralsX Development Team  
**Date**: 2026-01-15  
**Status**: Ready for Phase 06 Kickoff
