# PHASE24: Input Handling (Game)

## Phase Title

Input Handling - Game Input Processing

## Objective

Implement game-mode input handling for unit control, camera manipulation, and menu hotkeys.

## Dependencies

- PHASE16 (Input Routing - UI foundation)
- PHASE21 (GameObject System)
- PHASE23 (Game Loop)

## Key Deliverables

1. Keyboard input mapping
2. Mouse input handling (clicks, drag)
3. Input action mapping
4. Rebindable controls
5. Input buffering

## Acceptance Criteria

- [ ] Keyboard input works correctly
- [ ] Mouse clicks/drags detected properly
- [ ] Input actions map to game commands
- [ ] Controls are rebindable
- [ ] No input lag
- [ ] Multi-key combinations supported

## Technical Details

### Components to Implement

1. **Input System**
   - Key state tracking
   - Mouse state tracking
   - Input event buffering

2. **Action Mapping**
   - Input → Action mapping
   - Rebinding system
   - Persisted keybinds

3. **Command Processing**
   - Queue commands from input
   - Execute in game loop
   - Undo/redo support (optional)

### Code Location

```
GeneralsMD/Code/Input/GameInput/
GeneralsMD/Code/Input/ActionMap/
```

### Input System

```cpp
enum GameAction {
    MoveForward,
    MoveBack,
    TurnLeft,
    TurnRight,
    SelectUnit,
    CommandAttack,
    // ... etc
};

class GameInputManager {
public:
    void ProcessInput(float deltaTime);
    void SetActionKeyBinding(GameAction action, int key);
    bool IsActionPressed(GameAction action) const;
    void GetMouseDelta(float& dx, float& dy) const;
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Input polling: 0.5 day
- Action mapping: 1 day
- Rebinding system: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Test keyboard and mouse input
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- All input types responsive
- No input lag
- Actions map correctly
- Rebinding works and persists
- Multi-key combinations work

## Reference Documentation

- Input buffering patterns
- Action mapping systems
- Configuration file format
