# Phase 39: UI/Menu System & Rendering (FASE C - MÉDIO PRAZO)

**Status**: Ready (after Phase 37 & 38)  
**Estimated Time**: 4-5 days  
**Priority**: 🎮 **HIGH** - User-facing content

## Overview

Render menu system with proper texture/UI element display, validate mouse/keyboard input, and implement menu navigation. This is the first phase where user sees actual game content beyond the blue Metal screen. Depends on Phase 37 (asset loading) and Phase 38 (audio for UI feedback).

## Current Status

✅ **What's Working**:
- Metal rendering pipeline
- Texture loading from .big files (Phase 37)
- Audio system with UI sound support (Phase 38)
- SDL2 input system (mouse/keyboard event handling)

❌ **What's Missing**:
- Menu UI elements not rendering
- Mouse cursor not visible
- Button states (normal/hover/pressed) not displaying
- Menu transitions not working
- Keyboard shortcuts not responding

## Tasks

### Phase 39.1: Menu Rendering System (1.5 days)

**Objective**: Get main menu visible with all UI elements

**Checklist**:

- [ ] Load Shell map configuration
  - [ ] Shell.map loads without crashes
  - [ ] Menu background texture visible
  - [ ] UI layout configuration from INI

- [ ] Implement UI element rendering
  - [ ] Button quad rendering (with textures from Phase 37)
  - [ ] Text rendering (font atlas or system fonts)
  - [ ] Checkbox/toggle state visualization
  - [ ] Slider control rendering

- [ ] Set up menu camera
  - [ ] Orthographic projection for UI (2D overlay)
  - [ ] Z-ordering for multiple UI layers
  - [ ] Proper coordinate transformation (screen → world)

- [ ] Validate rendering pipeline
  - [ ] No Metal validation errors
  - [ ] Frame rate stable (60+ FPS) with UI elements
  - [ ] Texture sampling correct (no artifacts)

**Files to Modify**:
- `InGameUI.cpp` - Menu rendering logic
- `Shell.map` - Menu configuration
- Metal shaders - UI-specific rendering (if needed)

**Commands**:

```bash
# Build with UI rendering
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Run and test menu display
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 30 ./GeneralsXZH 2>&1 | tee /tmp/phase39_menu_render.log

# Check for UI rendering calls
grep -i "render\|draw\|button\|ui" /tmp/phase39_menu_render.log | head -30
```

### Phase 39.2: Input System Integration (1.5 days)

**Objective**: Validate mouse/keyboard input works with menu system

**Checklist**:

- [ ] Implement mouse cursor rendering
  - [ ] System cursor visible over menu
  - [ ] Cursor position tracking accurate (Retina display scaling)
  - [ ] Cursor changes on hover (pointer → hand over button)

- [ ] Connect mouse events to UI elements
  - [ ] Mouse hover detection for buttons
  - [ ] Button click detection (left mouse button)
  - [ ] Drag support for sliders
  - [ ] Right-click context menus

- [ ] Implement keyboard navigation
  - [ ] Tab key cycles through UI elements
  - [ ] Enter/Space activates focused button
  - [ ] ESC exits menu or goes back
  - [ ] Number keys for quick access (optional)

- [ ] Test input responsiveness
  - [ ] No lag between input and visual feedback
  - [ ] Multiple simultaneous inputs handled
  - [ ] Focus management correct

**Files to Modify**:
- `Mouse.cpp` - Cursor rendering + event handling
- `Keyboard.cpp` - Keyboard event handling
- `GameWindowManager.cpp` - Input dispatch
- `InGameUI.cpp` - UI element hit detection

**Commands**:

```bash
# Build input system
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Run and manually test mouse clicks
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 30 ./GeneralsXZH 2>&1 | tee /tmp/phase39_input_test.log

# Look for input events
grep -i "mouse\|click\|keyboard\|input" /tmp/phase39_input_test.log | head -20
```

### Phase 39.3: Menu Transitions & Navigation (1 day)

**Objective**: Implement menu flow and state management

**Checklist**:

- [ ] Main menu state machine
  - [ ] Display menu options (Play, Options, Credits, Exit)
  - [ ] Highlight selected option
  - [ ] Execute action on selection

- [ ] Submenu navigation
  - [ ] Options menu → Graphics, Sound, Gameplay settings
  - [ ] Credits menu → Display contributors
  - [ ] Back button returns to main menu

- [ ] Transition animations (optional, low priority)
  - [ ] Fade in/out between menus
  - [ ] Slide transitions
  - [ ] Smooth camera movement

- [ ] Game launch flow
  - [ ] Play button → Skirmish/Campaign selection
  - [ ] Skirmish button → Map/difficulty selection
  - [ ] Start game → Load map and transition to gameplay

**Files to Modify**:
- `Shell.cpp` - Menu state machine
- `GameLogic.cpp` - Game initialization from menu
- Menu transition logic

**Commands**:

```bash
# Build menu system
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Interactive menu test (manual testing required)
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 60 ./GeneralsXZH 2>&1 | tee /tmp/phase39_menu_test.log

# Verify state transitions
grep -i "menu\|state\|transition" /tmp/phase39_menu_test.log | head -20
```

## Success Criteria

- ✅ Main menu renders with visible buttons and text
- ✅ Mouse cursor visible and responsive to hover
- ✅ Button click audio plays (from Phase 38)
- ✅ Keyboard navigation works (Tab, Enter, ESC)
- ✅ Menu transitions execute without crashes
- ✅ Frame rate stable (60+ FPS) throughout menu navigation
- ✅ No Metal validation errors or crashes
- ✅ All UI textures load and display correctly

## Known Challenges

1. **Retina Display Coordinate Scaling**
   - macOS reports coordinates in points, not pixels
   - Retina display uses 2:1 scaling (2 pixels per point)
   - Solution: Account for backingScaleFactor in coordinate transforms

2. **Text Rendering**
   - Game was designed for DirectX font rendering
   - Options:
     - Use system font rendering via SDL2_ttf
     - Load pre-built font texture atlas from game assets
     - Generate dynamic glyph atlas at startup

3. **Mouse Coordinate Translation**
   - Windows HWND coordinates → macOS NSView coordinates
   - Y-axis inverted (top-left vs bottom-left origin)
   - Solution: Invert Y when converting from OpenGL to UI space

4. **UI Element Z-Ordering**
   - Must render UI on top of game content
   - Ensure Metal depth testing configured correctly
   - Test with multiple overlapping UI panels

## Debugging Strategy

1. **Enable UI rendering logs**:
   - Log each button/element as it renders
   - Log coordinate transforms
   - Log input events

2. **Use Metal frame capture** for graphics debugging:
   - Inspect actual rendered textures
   - Verify viewport/projection matrices
   - Check depth buffer state

3. **Test incrementally**:
   - Start with single button
   - Add more UI elements one at a time
   - Test input for each element before adding next

## Files to Monitor

| File | Purpose | Priority |
|------|---------|----------|
| `InGameUI.cpp` | Menu rendering | HIGH |
| `Mouse.cpp` | Mouse input | HIGH |
| `Keyboard.cpp` | Keyboard input | HIGH |
| `Shell.cpp` | Menu state machine | HIGH |
| Metal shaders | UI rendering | MEDIUM |

## References

- `docs/PHASE37/` - Texture loading (required for UI)
- `docs/PHASE38/` - Audio system (UI sounds)
- `docs/PHASE34/PHASE34_1_UI_SYSTEM_STATUS.md` - UI system architecture
- SDL2 documentation - Input event handling

## Dependencies

✅ **Complete**:
- Phase 30: Metal Backend
- Phase 31: Texture System

⏳ **Required**:
- Phase 37: Asset Loading (for UI textures)
- Phase 38: Audio System (for UI feedback sounds)

⏳ **Blocking Phase 40**:
- UI must work before gameplay map rendering

## Next Phase

**Phase 40**: Game Logic & Gameplay Systems (uses stable menu from Phase 39)

---

**Last Updated**: October 27, 2025  
**Status**: Ready after Phase 37 & 38 completion
