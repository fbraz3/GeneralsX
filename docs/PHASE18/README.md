# PHASE18: Menu State Machine

## Phase Title

Menu State Machine - Menu Transitions

## Objective

Implement state machine for menu transitions and state management (main menu, options, loading, etc.)

## Dependencies

- PHASE17 (Menu Definitions)
- PHASE13 (UI Rendering)

## Key Deliverables

1. Menu state enumeration
2. State transition logic
3. State change callbacks
4. Transition animations (optional)
5. State persistence

## Acceptance Criteria

- [ ] Menu states transition correctly
- [ ] State transitions are smooth
- [ ] No invalid state transitions
- [ ] State changes trigger appropriate callbacks
- [ ] Performance acceptable
- [ ] No memory leaks on state change

## Technical Details

### Components to Implement

1. **State Management**
   - State enum (MainMenu, Options, Loading, etc.)
   - Current state tracking
   - Previous state for back navigation

2. **Transition Logic**
   - Allowed transitions
   - Transition callbacks
   - Error handling for invalid transitions

3. **State Callbacks**
   - On enter state
   - On exit state
   - On transition complete

### Code Location

```
GeneralsMD/Code/Menu/State/
GeneralsMD/Code/Menu/StateMachine/
```

### State Machine Implementation

```cpp
class MenuStateMachine {
public:
    enum State { MainMenu, Options, Credits, Loading, InGame };
    
    void TransitionTo(State newState);
    State GetCurrentState() const { return currentState; }
    void Update(float deltaTime);
    
private:
    State currentState;
    void OnEnterState(State state);
    void OnExitState(State state);
};
```

## Estimated Scope

**SMALL** (1-2 days)

- State machine setup: 0.5 day
- Transition logic: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Test menu state transitions
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "State"
```

## Success Criteria

- All state transitions work
- Smooth transitions without flicker
- Callbacks execute reliably
- No crashes on invalid transitions

## Reference Documentation

- State machine design patterns
- Finite state machines (FSM)
- UI state management
