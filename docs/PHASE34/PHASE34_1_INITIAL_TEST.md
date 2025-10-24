# Phase 34.1: Initial UI System Test

**Test Date**: October 21, 2025  
**Test Type**: Manual UI Interaction Validation  
**Build**: GeneralsXZH (Zero Hour) - macOS ARM64 Metal Backend

## Pre-Test Checklist

- ✅ Build completed successfully (`cmake --build build/macos-arm64 --target GeneralsXZH`)
- ✅ Metal backend enabled (`USE_METAL=1`)
- ✅ Game assets in place (`$HOME/GeneralsX/GeneralsMD/Data/`)
- ✅ Log directory prepared (`logs/` with README.md)

## Test Environment

**Hardware**:

- Platform: macOS (Apple Silicon recommended)
- Display: Retina display (2x scaling) or standard
- Input: Mouse + Keyboard

**Software**:

- Build: `build/macos-arm64/GeneralsMD/GeneralsXZH`
- Backend: Metal (via `USE_METAL=1` environment variable)
- Assets: Original game files in `$HOME/GeneralsX/GeneralsMD/`

## Test Execution Commands

### Launch Game with Full Logging

```bash
# Navigate to game directory
cd $HOME/GeneralsX/GeneralsMD

# Launch with Metal backend and full logging
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/phase34_1_ui_test_$(date +%Y%m%d_%H%M%S).log
```

### Analysis Commands (Run After Test)

```bash
# Check for UI-related events
grep -i "GWM_\|GBM_\|button\|menu" logs/phase34_1_ui_test_*.log | head -200

# Check for mouse events
grep -i "mouse\|cursor\|click" logs/phase34_1_ui_test_*.log | head -200

# Check for keyboard events
grep -i "keyboard\|key_\|GWM_CHAR" logs/phase34_1_ui_test_*.log | head -200

# Check for focus changes
grep -i "focus\|GWM_INPUT_FOCUS" logs/phase34_1_ui_test_*.log | head -100
```

## Test Procedures

### Test 1.1: Game Launch & Main Menu Display

**Objective**: Verify game launches and displays main menu correctly.

**Steps**:

1. Launch game with command above
2. Observe splash screens (if any)
3. Wait for main menu to appear
4. Observe mouse cursor appearance
5. Move mouse around screen

**Expected Results**:

- ✅ Game window opens
- ✅ Main menu renders correctly
- ✅ Mouse cursor visible
- ✅ Mouse moves smoothly (no jitter)
- ✅ Menu background/animations playing (if applicable)

**Observed Results**:

- [ ] PASS / [ ] FAIL / [ ] PARTIAL
- Notes: _______________________________________________

### Test 1.2: Button Hover Effects

**Objective**: Verify mouse hover detection and visual feedback.

**Steps**:

1. Move mouse over "Single Player" button
2. Observe visual change (highlight, color, border)
3. Move mouse away from button
4. Repeat for "Multiplayer", "Options", "Exit" buttons

**Expected Results**:

- ✅ Button highlights when mouse hovers
- ✅ Button returns to normal when mouse leaves
- ✅ Hover effect appears immediately (no delay)
- ✅ Smooth transition (no flickering)

**Observed Results**:

- [ ] PASS / [ ] FAIL / [ ] PARTIAL
- Notes: _______________________________________________

### Test 1.3: Button Click Response

**Objective**: Verify mouse click detection and button activation.

**Steps**:

1. Click "Single Player" button
2. Observe menu transition
3. If submenu appears, click "Back" button
4. Click "Options" button
5. If options menu appears, click "Back" or "Cancel"
6. Hover over "Exit" button (do NOT click yet)

**Expected Results**:

- ✅ Button responds to click
- ✅ Click sound plays (Phase 33 dependency - may not work yet)
- ✅ Menu transitions smoothly
- ✅ No double-click required (single click works)
- ✅ "Back" button returns to previous menu

**Observed Results**:

- [ ] PASS / [ ] FAIL / [ ] PARTIAL
- Notes: _______________________________________________

### Test 1.4: Keyboard Navigation

**Objective**: Verify keyboard input and menu navigation shortcuts.

**Steps**:

1. From main menu, press ESC key
2. Observe behavior (exit dialog or no effect)
3. If in submenu, press ESC to return to main menu
4. Press TAB key to cycle through buttons (if supported)
5. Press ENTER key to activate highlighted button (if supported)

**Expected Results**:

- ✅ ESC key closes current menu or shows exit dialog
- ✅ TAB key cycles focus through buttons (visual indicator)
- ✅ ENTER key activates focused button
- ✅ No keyboard delay or repeat issues

**Observed Results**:

- [ ] PASS / [ ] FAIL / [ ] PARTIAL
- Notes: _______________________________________________

### Test 1.5: Options Menu Controls

**Objective**: Verify various control types work correctly.

**Steps**:

1. Navigate to Options menu
2. Locate volume slider control
3. Click and drag slider thumb
4. Locate checkbox control (e.g., "Show FPS")
5. Click checkbox to toggle
6. Locate dropdown/combobox control (e.g., "Resolution")
7. Click to expand dropdown
8. Click to select an option

**Expected Results**:

- ✅ Slider responds to mouse drag
- ✅ Slider value updates visually
- ✅ Checkbox toggles on/off with click
- ✅ Dropdown expands on click
- ✅ Dropdown options selectable
- ✅ Dropdown collapses after selection

**Observed Results**:

- [ ] PASS / [ ] FAIL / [ ] PARTIAL
- Notes: _______________________________________________

### Test 1.6: Mouse Coordinate Accuracy

**Objective**: Verify mouse clicks register at correct screen positions.

**Steps**:

1. Click on small buttons (close buttons, small icons)
2. Click on buttons at screen edges
3. Click on buttons at screen center
4. On Retina display, verify no coordinate offset issues

**Expected Results**:

- ✅ Small buttons respond to precise clicks
- ✅ Edge buttons respond correctly
- ✅ No visible offset between cursor and click point
- ✅ No difference between Retina and non-Retina displays

**Observed Results**:

- [ ] PASS / [ ] FAIL / [ ] PARTIAL
- Notes: _______________________________________________

## Issues to Watch For

### Critical Issues

1. **Mouse Cursor Not Visible**
   - Symptom: Cursor doesn't render
   - Likely Cause: Texture loading failure (Phase 31 dependency)
   - Log Search: `grep -i "cursor.*texture\|cursor.*load" logs/*.log`

2. **Buttons Not Responding to Clicks**
   - Symptom: Click events not detected
   - Likely Cause: Mouse coordinate translation issue
   - Log Search: `grep -i "GWM_LEFT_DOWN\|GWM_LEFT_UP\|click" logs/*.log`

3. **Menu Not Rendering**
   - Symptom: Black screen or incomplete menu
   - Likely Cause: UI texture/rendering issue
   - Log Search: `grep -i "menu.*draw\|window.*draw" logs/*.log`

### Non-Critical Issues

1. **No Hover Effects**
   - Symptom: Buttons don't highlight on hover
   - Likely Cause: `GWM_MOUSE_ENTERING` events not firing
   - Impact: LOW (functionality works, visual feedback missing)

2. **No Click Sounds**
   - Symptom: Silent button clicks
   - Likely Cause: Phase 33 (audio backend) not complete
   - Impact: LOW (expected, not a blocker)

3. **Keyboard Shortcuts Don't Work**
   - Symptom: ESC, TAB, ENTER keys ignored
   - Likely Cause: Keyboard focus not set or modifier key mapping
   - Impact: MEDIUM (mouse works, keyboard convenience missing)

## Log Analysis Patterns

### Successful UI Interaction Pattern

```text
GameWindowManager::winProcessMouseEvent - GWM_MOUSE_POS (x=512, y=384)
GameWindowManager::winProcessMouseEvent - GWM_MOUSE_ENTERING (window=MainMenu.wnd:ButtonSinglePlayer)
GameWindow::winSendInputMsg - GWM_MOUSE_ENTERING
GadgetPushButton::handleInput - GWM_MOUSE_ENTERING (highlight enabled)
GameWindowManager::winProcessMouseEvent - GWM_LEFT_DOWN (x=512, y=384)
GadgetPushButton::handleInput - GWM_LEFT_DOWN
GameWindowManager::winSendSystemMsg - GBM_SELECTED (buttonID=MainMenu.wnd:ButtonSinglePlayer)
MainMenuSystem - GBM_SELECTED - Button pressed: Single Player
```

### Failed Interaction Pattern (Mouse Coordinate Issue)

```text
GameWindowManager::winProcessMouseEvent - GWM_LEFT_DOWN (x=1024, y=768)  // Wrong position
GameWindowManager::findWindowUnderMouse - No window at position (1024, 768)
// Expected position should be (512, 384) if screen is 1024x768
```

### Failed Interaction Pattern (No Event Dispatch)

```text
GameWindowManager::winProcessMouseEvent - GWM_LEFT_DOWN (x=512, y=384)
// Missing: GameWindow::winSendInputMsg
// Missing: GadgetPushButton::handleInput
// Cause: Event not routed to correct window
```

## Post-Test Actions

### If All Tests Pass

1. Document success in `PHASE34_1_UI_SYSTEM_STATUS.md`
2. Mark Phase 34.1 as COMPLETE
3. Move to Phase 34.2 (Input System)
4. Update `MACOS_PORT_DIARY.md`

### If Tests Fail

1. Analyze log files using patterns above
2. Document failures in detail
3. Create fix plan based on root cause
4. Implement fixes incrementally
5. Re-test after each fix

### Specific Failure Actions

**Mouse Not Visible**:

- Check `Mouse.cpp` cursor initialization
- Check cursor texture loading (`INI.big` assets)
- Verify Metal texture upload working

**Buttons Not Responding**:

- Add debug logging to `GameWindowManager::winProcessMouseEvent()`
- Print mouse coordinates and window bounds
- Check coordinate transformation in `Display` class

**Menu Not Rendering**:

- Check UI texture loading
- Verify Metal draw calls for UI elements
- Check window hierarchy and z-order

## Exit Strategy

**Clean Exit**: Press ESC → Click "Exit" → Confirm

**Force Exit**: Cmd+Q (macOS) or Ctrl+C in terminal

**Log Preservation**: Logs automatically saved with timestamp in filename

---

**Status**: Ready to execute  
**Prerequisites**: Build complete, assets in place  
**Next Step**: Run Test 1.1 and document results
