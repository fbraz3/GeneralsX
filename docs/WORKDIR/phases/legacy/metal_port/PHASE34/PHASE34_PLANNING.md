# Phase 34: Game Logic & Gameplay Systems - Planning & Analysis

**Start Date**: October 21, 2025  
**Estimated Duration**: 2-3 weeks  
**Priority**: MEDIUM - Core gameplay functionality validation

## Overview

Phase 34 focuses on validating and fixing core game logic systems to ensure proper cross-platform compatibility. Most systems are already implemented from the original Windows code - this phase validates they work correctly on macOS and fixes platform-specific issues.

## Dependencies Status

- ✅ **Phase 30**: Metal Backend - COMPLETE (rendering operational)
- ✅ **Phase 31**: Texture System - COMPLETE (UI visuals working)
- ✅ **Phase 32**: Audio Pipeline Investigation - COMPLETE (audio subsystem mapped)
- ⏳ **Phase 33**: OpenAL Audio Backend - PLANNED (for UI feedback sounds)

## Phase Structure

### Phase 34.1: UI Interactive System (4-5 days) ✅ CURRENT

**Goal**: Validate menu navigation and button interaction systems work on macOS.

**Key Systems to Validate**:
1. Menu navigation (mouse hover, click detection)
2. Button state management (normal, hover, pressed, disabled)
3. Text input fields
4. Checkbox and radio button controls
5. Dropdown menus and list boxes

**Primary Files**:
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindow.cpp` - Core window system
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GameWindowManager.cpp` - Window message routing
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Gadget/GadgetPushButton.cpp` - Button input
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/*Menu.cpp` - Menu handlers

**Validation Strategy**:
- Run game and navigate main menu
- Test button clicks (Single Player, Multiplayer, Options, Exit)
- Test Options menu controls (sliders, checkboxes, dropdowns)
- Check mouse hover effects (highlighting, tooltips)

**Expected Issues**:
- Mouse coordinate translation (Windows → macOS)
- Message handling differences (GWM_LEFT_DOWN, GBM_SELECTED)
- Focus management for keyboard input
- Window z-order and click-through detection

### Phase 34.2: Input System (3-4 days)

**Goal**: Validate keyboard/mouse input handling and camera controls.

**Key Systems to Validate**:
1. Mouse input (position, buttons, wheel scroll)
2. Keyboard input (key press, release, modifiers: Cmd/Ctrl/Shift/Alt)
3. Input event queue management
4. Camera controls (WASD pan, mouse wheel zoom, middle-click rotate)
5. Unit selection (left-click, drag box selection)

**Primary Files**:
- `GeneralsMD/Code/GameEngine/Source/GameClient/Input/Mouse.cpp` - Mouse handling
- `GeneralsMD/Code/GameEngine/Source/GameClient/Input/Keyboard.cpp` - Keyboard handling
- `GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/*Translator.cpp` - Input message translation

**Validation Strategy**:
- Test camera movement (keyboard + mouse)
- Test unit selection (click, box selection)
- Test hotkeys (build menu shortcuts, unit commands)
- Verify drag-and-drop operations

**Expected Issues**:
- Keyboard mapping (Windows scan codes → macOS keycodes)
- Mouse button order (Windows right-click = macOS secondary click)
- Mouse wheel delta differences
- Modifier key mapping (Windows Ctrl = macOS Cmd in many contexts)

### Phase 34.3: Basic AI System (5-6 days)

**Goal**: Validate AI unit behavior and command execution.

**Key Systems to Validate**:
1. Unit state machines (idle, move, attack, patrol, guard)
2. Target acquisition and tracking
3. Formation movement (maintain squad positions)
4. Build queue management (production, upgrades)
5. Resource gathering logic (workers to supply piles)

**Primary Files**:
- `GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIStates.cpp` - State machine definitions
- `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate.cpp` - AI update loop
- `GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPathfind.cpp` - Pathfinding interface
- `GeneralsMD/Code/GameEngine/Source/GameLogic/AI/Squad.cpp` - Formation logic

**Validation Strategy**:
- Create units in skirmish mode
- Command units to move, attack, patrol
- Test AI opponent behavior (build, attack)
- Verify worker resource gathering
- Check formation cohesion during movement

**Expected Issues**:
- Floating-point precision differences (pathfinding)
- Thread safety in AI updates (if any threading)
- Performance differences (AI decision timing)
- Memory layout differences affecting state machines

### Phase 34.4: Pathfinding & Collision (3-4 days)

**Goal**: Validate pathfinding algorithm and collision detection.

**Key Systems to Validate**:
1. A* pathfinding algorithm (find routes around obstacles)
2. Obstacle avoidance (static + dynamic)
3. Unit collision detection (prevent overlap)
4. Terrain traversability checks (land, water, air layers)
5. Dynamic path updates (blocked paths, new obstacles)

**Primary Files**:
- `GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPathfind.cpp` - Pathfinding implementation
- `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate.cpp` - Collision processing
- `GeneralsMD/Code/GameEngine/Source/GameLogic/Physics/*` - Physics/collision system

**Validation Strategy**:
- Command units to navigate around obstacles
- Test unit-to-unit collision avoidance
- Create blocked paths, verify rerouting
- Test air/land/water unit pathfinding separately
- Stress test with large unit groups

**Expected Issues**:
- Pathfinding grid alignment (coordinate system)
- Collision radius calculations (floating-point precision)
- Path caching consistency
- Performance with many units (O(n²) collision checks)

## Testing Methodology

### Manual Testing Priority
1. **Critical Path**: Main menu → Skirmish game → Basic gameplay
2. **UI Testing**: All menu screens, all button types
3. **Gameplay Testing**: Unit creation, movement, combat
4. **Edge Cases**: Large unit counts, complex paths, UI spam-clicking

### Automated Testing Opportunities
- Unit test pathfinding algorithm (compare to Windows results)
- Unit test collision detection (known scenarios)
- Input message translation unit tests

### Platform-Specific Concerns

**macOS-Specific**:
- Retina display coordinate scaling (2x pixel density)
- Cmd vs Ctrl key for hotkeys
- Right-click = two-finger tap or Ctrl+Click
- Window focus behavior differences

**Cross-Platform Validation**:
- Test on macOS ARM64 (Apple Silicon - primary target)
- Compare behavior with Windows reference (if available)
- Check reference repositories for similar fixes

## Success Criteria

### Phase 34.1 (UI System)
- ✅ All menu buttons respond to clicks
- ✅ Hover effects work correctly
- ✅ Keyboard navigation works (Tab, Enter, Esc)
- ✅ Text input accepts characters
- ✅ Dropdowns open/close properly

### Phase 34.2 (Input System)
- ✅ Camera responds to WASD + mouse
- ✅ Units can be selected (click + box)
- ✅ Hotkeys execute commands
- ✅ Mouse wheel zooms camera
- ✅ No input lag or dropped events

### Phase 34.3 (AI System)
- ✅ Units move to commanded positions
- ✅ Units attack enemies when ordered
- ✅ AI opponent builds structures + units
- ✅ Workers gather resources
- ✅ Formations maintain during movement

### Phase 34.4 (Pathfinding & Collision)
- ✅ Units navigate around obstacles
- ✅ Units avoid each other (no overlap)
- ✅ Blocked paths trigger rerouting
- ✅ Air units ignore ground obstacles
- ✅ No units stuck in walls/terrain

## Risk Assessment

**HIGH RISK**:
- Mouse coordinate translation (Windows HWND → macOS NSWindow)
- State machine serialization (save/load compatibility)

**MEDIUM RISK**:
- Input event ordering (race conditions)
- Pathfinding float precision (cross-platform consistency)
- UI z-order and focus management

**LOW RISK**:
- Button graphics (already handled by texture system)
- AI logic (pure C++ algorithms, platform-agnostic)

## Reference Materials

### Existing Fixes from Reference Repositories
- `references/jmarshall-win64-modern/` - Windows 64-bit port fixes
- `references/fighter19-dxvk-port/` - Linux input handling
- `references/dsalzner-linux-attempt/` - POSIX input translation

### Key Documents
- `docs/WORKDIR/support/GRAPHICS_BACKENDS.md` - Metal/OpenGL context for UI rendering
- `docs/WORKDIR/phases/3/PHASE32/AUDIO_SYSTEM_OVERVIEW.md` - Audio event system (for UI sounds)
- `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)` - Development history

## Next Steps (After Phase 34.1 Analysis)

1. Run game with Metal backend enabled
2. Navigate to main menu
3. Document all UI interaction issues
4. Identify platform-specific mouse/keyboard code
5. Implement fixes incrementally
6. Test each fix with manual validation

---

**Status**: Phase 34.1 analysis in progress  
**Last Updated**: October 21, 2025
