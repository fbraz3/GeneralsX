# PHASE 03 Task 4: Integration Testing - Planning Document

## Objective
Validate that mouse, keyboard, and IME input systems work correctly together in real game environment (menu navigation, text input, composition).

## Prerequisites Status
- ✅ SDL2Mouse implementation: Complete
- ✅ SDL2Keyboard implementation: Complete  
- ✅ SDL2IMEManager implementation: Complete
- ❌ Build executable: Does not exist (build/vc6/GeneralsMD/generalszh.exe)
- ❌ Deployment: Directory not created yet

## Phase 03 Task 4 Execution Steps

### Step 1: Rebuild Project (Validation Compilation)
**Objective**: Ensure all SDL2 input changes compile cleanly, no regressions

**Actions**:
1. Clean build cache if needed
2. Build GeneralsXZH (VC6 32-bit) target
3. Verify no SDL2-related compilation errors
4. Log output to `logs/phase03_integration_build.log`

**Success Criteria**:
- ✓ Build completes without errors (LZHL warnings acceptable)
- ✓ Executable created: `build/vc6/GeneralsMD/generalszh.exe`
- ✓ No SDL2Mouse, SDL2Keyboard, or SDL2IMEManager compilation errors

### Step 2: Deploy Executable
**Objective**: Copy compiled binary to deployment location with assets

**Actions**:
1. Create deployment directory: `%USERPROFILE%\GeneralsX\GeneralsMD\`
2. Copy executable: `generalszh.exe`
3. Copy game assets (big files, config files) to `Data/` subdirectory
4. Verify INI configuration files are present

**Success Criteria**:
- ✓ Executable deployed to `%USERPROFILE%\GeneralsX\GeneralsMD\generalszh.exe`
- ✓ Game assets available or game loads fallback state
- ✓ Ready to launch game

### Step 3: Initial Launch Test
**Objective**: Verify game starts and reaches main menu without input system crashes

**Actions**:
1. Launch game: `-win -noshellmap` flags (windowed, skip shell map)
2. Wait for main menu to appear (30-60 seconds)
3. Capture output to `logs/phase03_integration_run1.log`
4. Verify no SDL2 input handler crashes

**Success Criteria**:
- ✓ Game starts without segfault/crash
- ✓ Main menu visible (or splash screen)
- ✓ No exception from SDL2Mouse, SDL2Keyboard, SDL2IMEManager
- ✓ Game accepts Escape key to exit cleanly

### Step 4: Mouse Input Test (Menu Navigation)
**Objective**: Verify mouse clicks work in menu navigation

**Test Cases**:
1. **Click on main menu buttons** - Single click on "New Game", "Skirmish", "Load Game", etc.
   - Expected: Button highlights/activates, transitions to next menu
2. **Hover detection** - Mouse motion over buttons
   - Expected: Button visual feedback (highlight)
3. **Double-click prevention** - Rapid clicks on same button
   - Expected: Only one activation (not two)
4. **Wheel interaction** (if applicable) - Scroll in list menus
   - Expected: List scrolls or no action (graceful)

**Success Criteria**:
- ✓ Single clicks consistently activate buttons
- ✓ Double-clicks do not cause duplicate actions
- ✓ Mouse movement provides visual feedback
- ✓ No input handler crashes during mouse interaction

### Step 5: Keyboard Input Test (Menu Navigation)
**Objective**: Verify keyboard navigation works in menus

**Test Cases**:
1. **Tab navigation** - Tab key cycles through menu buttons
   - Expected: Button focus moves (visual highlight)
2. **Shift+Tab** - Reverse tab navigation
   - Expected: Focus cycles backward
3. **Enter key** - Activate focused button
   - Expected: Same as mouse click on focused button
4. **Escape key** - Cancel/back in menus
   - Expected: Return to previous menu or quit
5. **Arrow keys** - Navigate list items
   - Expected: Selection moves up/down in lists
6. **Modifier keys** - Hold Shift/Ctrl during navigation
   - Expected: No conflicts with single-key actions

**Success Criteria**:
- ✓ Tab key navigation works consistently
- ✓ Enter activates focused element
- ✓ Escape returns to previous menu or quits
- ✓ Arrow keys navigate lists correctly
- ✓ No keyboard state conflicts or stuck keys

### Step 6: Text Input Test (IME & Composition)
**Objective**: Verify text input and IME composition works

**Test Cases**:
1. **ASCII text input** - Type in text fields (if game has them)
   - Expected: Text appears in field as typed
2. **Delete/Backspace** - Remove entered text
   - Expected: Text erased character by character
3. **Composition start** (if IME available on system)
   - Expected: Composition string appears/updates
4. **Composition commit**
   - Expected: Final text inserted into field

**Success Criteria**:
- ✓ ASCII text input works
- ✓ Backspace/Delete work correctly
- ✓ No UTF-8 decoder crashes (invalid sequences handled)
- ✓ IME composition displays (if IME available)

### Step 7: Combined Input Test (All Systems)
**Objective**: Verify all three input systems work together without interference

**Test Cases**:
1. **Navigate menu with keyboard, activate with mouse**
   - Expected: Both work independently without conflict
2. **Alternate between mouse and keyboard**
   - Expected: Focus tracking remains consistent
3. **Type text while holding modifier keys**
   - Expected: Modifiers don't interfere with text input
4. **Rapid input** - Fast clicks/keys in succession
   - Expected: All inputs processed, no dropped events

**Success Criteria**:
- ✓ Mouse and keyboard can be used interchangeably
- ✓ No input priority conflicts
- ✓ Event order preserved
- ✓ No corrupted or dropped input events

### Step 8: Stress Test (Optional)
**Objective**: Verify input systems remain stable under load

**Actions**:
1. Perform rapid clicking/typing for 1-2 minutes
2. Monitor for memory leaks, input lag, or crashes
3. Log performance metrics

**Success Criteria**:
- ✓ No crashes under sustained input
- ✓ No significant input lag increase
- ✓ Stable frame rates maintained

## Testing Environment

**Platform**: Windows (VC6 build, 32-bit)
**Game Mode**: `-win -noshellmap` (windowed, skip shell map)
**Logging**: All runs logged to `logs/phase03_integration_*.log`

## Known Limitations to Document

1. **Candidate List (IME)**
   - SDL2 does not provide candidate list data in SDL_TEXTEDITING
   - Test will verify composition works, document limitation

2. **Platform-Specific Input**
   - Gamepad/joystick: Not tested in this phase
   - Touch input: Not tested in this phase
   - Hardware-specific keys: May not map correctly on all keyboards

3. **Legacy Input Conflicts**
   - Original Win32 input handlers may still be active
   - Test will verify SDL2 handlers take priority without crashes

## Documentation Output

**Deliverables**:
1. `logs/phase03_integration_build.log` - Build compilation output
2. `logs/phase03_integration_run1.log` - Initial launch test
3. `logs/phase03_integration_run2.log` - Mouse navigation test
4. `logs/phase03_integration_run3.log` - Keyboard navigation test
5. `logs/phase03_integration_run4.log` - Text input test
6. `PHASE03_TASK4_RESULTS.md` - Final test summary with pass/fail for each case

## Decision Points

**If tests pass**:
→ PHASE 03 complete, proceed to PHASE 04 (Timing & OS Services)

**If critical failures**:
→ Debug and fix in order of severity:
  1. Crashes (segfault, exceptions)
  2. Input not recognized (core functionality)
  3. Input conflicts (priority/interference)
  4. Visual feedback issues (nice-to-have)

**If minor issues**:
→ Document as known issues, proceed with documentation and commits

## Build Status (2026-01-12)

**❌ BLOCKER FOUND**: LZHL compression library compilation failure

**Error Location**: `CMakeFiles/liblzhl.dir/_deps/lzhl-src/CompLibHeader/Huff.cpp.obj`
**Error Type**: C++ compilation error in dependency (pre-existing, not SDL2-related)
**Impact**: Cannot produce executable, integration tests cannot proceed

**Root Cause**: This is a pre-existing issue with LZHL library, not caused by SDL2 input implementation.

**Resolution Path**:
1. Investigate LZHL compilation issue (separate issue tracking)
2. Fix LZHL build errors
3. Return to PHASE 03 Task 4 integration tests
4. Alternative: Use previous version if available

## Next Actions

**Current Status**: PHASE 03 Task 4 blocked by LZHL compilation failure

1. ~~Build project (Step 1)~~ - BLOCKED by LZHL error
2. ~~Deploy executable (Step 2)~~ - Blocked due to no executable
3. ~~Run tests 3-7~~ - Cannot proceed without executable
4. Document blocker in dev diary
5. Commit planning document to git
6. Escalate: Either fix LZHL or defer integration tests to after LZHL resolution
