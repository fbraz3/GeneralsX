# PHASE15: Button System

## Phase Title

Button System - Hit Testing and Interactive States

## Objective

Implement interactive UI buttons with hit testing, state management (normal/hover/pressed), and click callbacks.

## Dependencies

- PHASE14 (UI Layout)
- PHASE16 (Input Routing - partial)

## Key Deliverables

1. Button state machine (normal, hover, pressed, disabled)
2. AABB hit testing for buttons
3. Click event callbacks
4. Visual feedback for state changes
5. Hover detection

## Acceptance Criteria

- [ ] Buttons respond to mouse over (hover state)
- [ ] Click detection works accurately
- [ ] Visual state changes correspond to interactions
- [ ] Callbacks execute correctly
- [ ] Disabled buttons don't respond to input
- [ ] Performance acceptable with many buttons

## Technical Details

### Components to Implement

1. **Button Component**
   - State tracking
   - Hit testing
   - State change callbacks

2. **Event System**
   - Click/press/release events
   - Listener registration
   - Event dispatching

3. **Visual Feedback**
   - Material/color changes per state
   - Animation triggers (optional)

### Code Location

```
GeneralsMD/Code/UI/Button/
GeneralsMD/Code/UI/Event/
```

### Button Implementation

```cpp
class Button : public UIElement {
public:
    enum State { Normal, Hover, Pressed, Disabled };
    
    void SetOnClick(std::function<void()> callback);
    bool HitTest(float screenX, float screenY);
    void SetState(State newState);
    State GetState() const { return state; }
    
private:
    State state;
    std::function<void()> onClickCallback;
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- State machine: 0.5 day
- Hit testing: 0.5 day
- Event system: 1 day
- Visual feedback: 0.5 day
- Testing: 0.5 day

## Status

**not-started**

## Testing Strategy

```bash
# Click buttons and verify state changes
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- All button interactions work
- Visual feedback matches state
- Callbacks execute reliably
- No input lag or missed clicks

## Reference Documentation

- UI event systems
- Hit testing algorithms
- State machine patterns
- See PHASE16 for input integration
