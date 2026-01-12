# PHASE 03: SDL2 Input and IME Integration

**Status**: Ready to Start  
**Estimated Effort**: 3-4 development sessions  
**Priority**: High (blocks gameplay testing)  
**Dependencies**: PHASE 02 (completed)  
**Exit Criteria**: Full menu navigation with keyboard/mouse/IME; basic input testing complete

---

## Overview

PHASE 03 focuses on replacing Win32 input APIs (mouse, keyboard, IME) with SDL2 equivalents. This enables:
- Menu navigation with keyboard and mouse
- Text input for player names, chat, etc.
- Support for international (non-ASCII) characters via IME

This phase is critical for moving past splash screens into interactive gameplay testing.

---

## Architecture Review

### From PHASE 02
- SDL2GameEngine has event loop with `serviceSDL2OS()` method
- Event pump already handles SDL_MOUSEBUTTONDOWN/UP/MOTION, SDL_KEYDOWN/UP placeholders
- SDL2Mouse stub class exists but empty
- Global TheSDL2Mouse instance available

### Input Translation Boundary
All SDL2 input events are translated in `SDL2GameEngine::serviceSDL2OS()`:
```cpp
case SDL_MOUSEBUTTONDOWN:
case SDL_MOUSEBUTTONUP:
case SDL_MOUSEMOTION:
case SDL_MOUSEWHEEL:
  // Handle mouse events
case SDL_KEYDOWN:
case SDL_KEYUP:
  // Handle keyboard events
case SDL_TEXTEDITING:
case SDL_TEXTINPUT:
  // Handle IME and text input
```

### Engine-Level Input System
- **TheMouseInput**: Engine's mouse manager
  - Methods: `setMousePos()`, `setMouseButton()`, `setMouseWheelDelta()`
  - Maps to internal `GameMessage` system
- **TheKeyboardInput**: Engine's keyboard manager
  - Methods: `setKeyState()`, `getKeyState()`
  - Supports key repeats
- **TheIMEManager**: Text input and composition
  - Methods: `startComposition()`, `updateComposition()`, `endComposition()`
  - Feeds typed text to UI widgets

---

## Tasks

### Task 1: SDL2Mouse Button and Motion Handling
**Effort**: 6-8 hours  
**Files**:
- GeneralsMD/Code/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Mouse.h
- GeneralsMD/Code/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Mouse.cpp
- GeneralsMD/Code/GameEngineDevice/Source/SDL2Device/Common/SDL2GameEngine.cpp (event handlers)

**Requirements**:
- [ ] Implement `TheSDL2Mouse::onMouseButtonDown()` with button identification (left/middle/right)
- [ ] Implement `TheSDL2Mouse::onMouseButtonUp()` with button identification
- [ ] Implement `TheSDL2Mouse::onMouseMotion()` with position tracking
- [ ] Implement `TheSDL2Mouse::onMouseWheel()` with delta tracking
- [ ] Translate SDL button codes (SDL_BUTTON_LEFT, etc.) to engine codes
- [ ] Inject mouse events into TheMouseInput system
- [ ] Handle relative vs absolute positioning
- [ ] Update SDL2GameEngine to call SDL2Mouse handlers from event pump

**Testing Checklist**:
- [ ] Mouse moves and position updates in menu
- [ ] Mouse buttons (left, middle, right) register correctly
- [ ] Mouse wheel scrolls lists (if applicable)
- [ ] Double-click detection works

---

### Task 2: SDL2Keyboard Handling
**Effort**: 6-8 hours  
**Files**:
- GeneralsMD/Code/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Keyboard.h (new)
- GeneralsMD/Code/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Keyboard.cpp (new)
- GeneralsMD/Code/GameEngineDevice/Source/SDL2Device/Common/SDL2GameEngine.cpp (event handlers)

**Requirements**:
- [ ] Create SDL2Keyboard class (parallel to SDL2Mouse)
- [ ] Implement `onKeyDown()` with key translation
- [ ] Implement `onKeyUp()` with key translation
- [ ] Translate SDL keys to engine key codes (SDL_SCANCODE_* → internal codes)
- [ ] Handle modifier keys (Shift, Ctrl, Alt, Meta/Win)
- [ ] Track key states for repeated presses
- [ ] Inject keyboard events into TheKeyboardInput system
- [ ] Handle Tab key (menu navigation), Enter (selection), Escape (quit/back)
- [ ] Update SDL2GameEngine to call SDL2Keyboard handlers from event pump

**Key Bindings to Support**:
- ESC: Quit or back button
- Tab: Next menu item
- Shift+Tab: Previous menu item
- Enter/Space: Select
- Arrow keys: Navigate lists
- Ctrl+C: Copy (if supported)
- Ctrl+V: Paste (if supported)

**Testing Checklist**:
- [ ] Tab navigates menu items
- [ ] Shift+Tab reverses navigation
- [ ] Enter selects menu items
- [ ] ESC quits or goes back
- [ ] Arrow keys navigate lists/sliders
- [ ] Keyboard repeat works (holding key repeats)
- [ ] Modifier keys register correctly

---

### Task 3: IME and Text Input
**Effort**: 4-6 hours  
**Files**:
- GeneralsMD/Code/GameEngineDevice/Include/SDL2Device/GameClient/SDL2IMEManager.h (new)
- GeneralsMD/Code/GameEngineDevice/Source/SDL2Device/GameClient/SDL2IMEManager.cpp (new)
- GeneralsMD/Code/GameEngineDevice/Source/SDL2Device/Common/SDL2GameEngine.cpp (event handlers)

**Requirements**:
- [ ] Create SDL2IMEManager class for text composition
- [ ] Implement `onTextEditing()` for composition events (CJK, Cyrillic, etc.)
- [ ] Implement `onTextInput()` for committed text
- [ ] Handle composition start/update/end lifecycle
- [ ] Support multiple IME systems (SDL2 handles OS-level IME)
- [ ] Integrate with TheIMEManager for composition display
- [ ] Inject text events into UI input fields
- [ ] Update SDL2GameEngine to call IME handlers from event pump

**IME Support**:
- Chinese (Simplified/Traditional) - Pinyin
- Japanese (Hiragana/Katakana)
- Korean (Hangul)
- Cyrillic (Russian, etc.)
- Any IME supported by OS (Linux fcitx, macOS native, Windows native)

**Testing Checklist**:
- [ ] ASCII text input works (a-z, 0-9, symbols)
- [ ] Backspace deletes characters
- [ ] Copy/paste works (if supported)
- [ ] IME composition shows correctly (if testing with CJK input)
- [ ] Text input in menus (player name field)
- [ ] Text input in game chat (if applicable)

---

### Task 4: Integration Testing and Validation
**Effort**: 4-6 hours  
**Files**: Test harness (command-line options, logging)

**Requirements**:
- [ ] Create test harness for input validation
  - Flag to log all input events: `-log-input`
  - Flag to run input test sequence: `-test-input`
- [ ] Test menu navigation with both keyboard and mouse
- [ ] Test text input in menu (player name field)
- [ ] Test game startup with input subsystem
- [ ] Log all input events to file for debugging
- [ ] Verify no crashes on rapid input
- [ ] Stress test with continuous input

**Test Scenarios**:
1. Menu navigation: TAB → ENTER → Back (ESC)
2. Mouse navigation: Click menu items, click buttons
3. Text input: Enter player name, verify in game
4. Mixed input: Keyboard + mouse in same session
5. Input under gameplay (if game loads)
6. IME input (if testing with CJK/Cyrillic)

**Testing Checklist**:
- [ ] Full menu navigation works
- [ ] Mouse clicks register on menu items
- [ ] Keyboard navigation works (TAB/arrows/ENTER/ESC)
- [ ] Text input field accepts typed characters
- [ ] No input lag or missed events
- [ ] No crashes on rapid input
- [ ] Input logging works correctly

---

## Detailed Checklist

- [ ] Task 1: SDL2Mouse implementation complete
  - [ ] Mouse button events (down/up) implemented
  - [ ] Mouse motion events implemented
  - [ ] Mouse wheel events implemented
  - [ ] Button code translation complete
  - [ ] Integration with TheMouseInput complete
  - [ ] Relative/absolute mouse mode support
  - [ ] SDL2GameEngine event handlers added
- [ ] Task 2: SDL2Keyboard implementation complete
  - [ ] Keyboard event handlers (down/up) implemented
  - [ ] Key code translation complete
  - [ ] Modifier key support (Shift/Ctrl/Alt/Meta)
  - [ ] Key repeat tracking implemented
  - [ ] Integration with TheKeyboardInput complete
  - [ ] Tab/Enter/ESC/Arrow key handling complete
  - [ ] SDL2GameEngine event handlers added
- [ ] Task 3: SDL2IMEManager implementation complete
  - [ ] IME class created
  - [ ] Text editing events (composition) handled
  - [ ] Text input events handled
  - [ ] Integration with TheIMEManager complete
  - [ ] IME support for CJK/Cyrillic
  - [ ] SDL2GameEngine event handlers added
- [ ] Task 4: Integration testing complete
  - [ ] Test harness created with logging
  - [ ] Menu navigation testing passed
  - [ ] Text input testing passed
  - [ ] No crashes on rapid input
  - [ ] Input logging works correctly
- [ ] Code reviewed and committed
- [ ] Documentation updated
- [ ] Dev diary updated

---

## Exit Criteria
- ✅ Full menu navigation works with keyboard (TAB/arrows/ENTER/ESC)
- ✅ Full menu navigation works with mouse
- ✅ Text input works in menu text fields
- ✅ No input lag or missed events
- ✅ No crashes on rapid or continuous input
- ✅ Core gameplay input works (camera, selection, commands)
- ✅ All Phase 03 tasks completed
