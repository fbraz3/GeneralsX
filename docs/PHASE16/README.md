# PHASE16: Input Routing

## Phase Title

Input Routing - SDL2 Events to UI System

## Objective

Route SDL2 input events to UI system with proper focus management and event propagation.

## Dependencies

- PHASE13 (UI Rendering) - need UI hierarchy
- PHASE15 (Button System)
- SDL2 library

## Key Deliverables

1. SDL2 event polling
2. Input event conversion
3. Hit test dispatching
4. Focus management
5. Event propagation (bubbling)

## Acceptance Criteria

- [ ] Mouse events reach UI elements
- [ ] Keyboard input routed to focused element
- [ ] Focus can be changed programmatically
- [ ] Event propagation works correctly
- [ ] No input lag or missed events
- [ ] Performance acceptable with many input sources

## Technical Details

### Components to Implement

1. **Input Manager**
   - SDL2 event polling
   - Event queue management
   - Focus tracking

2. **Event System**
   - Mouse events (move, click, drag)
   - Keyboard events
   - Event propagation

3. **Focus System**
   - Focus element tracking
   - Tab cycling
   - Click-to-focus

### Code Location

```
GeneralsMD/Code/Input/InputManager/
GeneralsMD/Code/UI/Event/
Core/GameEngineDevice/Source/Input/
```

### Input Manager

```cpp
class InputManager {
public:
    void Update(float deltaTime);
    void SetUIRootElement(UIElement* root);
    bool HandleMouseClick(float x, float y);
    bool HandleKeyPress(int keyCode);
    void SetFocusElement(UIElement* elem);
    
private:
    UIElement* focusedElement;
    UIElement* uiRoot;
};
```

## Estimated Scope

**MEDIUM** (3-4 days)

- SDL2 integration: 1 day
- Event conversion: 0.5 day
- Hit testing dispatch: 0.5 day
- Focus management: 0.5 day
- Testing: 1.5 days

## Status

**not-started**

## Testing Strategy

```bash
# Test mouse and keyboard input
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "Input"
```

## Success Criteria

- All input events reach UI
- Focus works correctly
- No input lag
- Event propagation accurate

## Reference Documentation

- SDL2 event documentation
- Event propagation and bubbling patterns
- Input management best practices
- See PHASE15 for button integration
