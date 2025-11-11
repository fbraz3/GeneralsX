# PHASE20: Menu Interaction

## Phase Title

Menu Interaction - Clicks and Actions Working

## Objective

Complete menu interactivity by connecting button clicks to actions (start game, options, quit, etc.)

## Dependencies

- PHASE15 (Button System)
- PHASE16 (Input Routing)
- PHASE18 (Menu State Machine)
- PHASE19 (Menu Rendering)

## Key Deliverables

1. Action dispatch system
2. Menu action handlers
3. Game launch from menu
4. Option persistence
5. Save/load integration

## Acceptance Criteria

- [ ] Button clicks execute correct actions
- [ ] Menu can launch game
- [ ] Menu can quit application
- [ ] Options can be changed
- [ ] Changes persist between sessions
- [ ] No crashes from menu actions

## Technical Details

### Components to Implement

1. **Action System**
   - Action enumeration
   - Action dispatch
   - Handler registration

2. **Action Handlers**
   - Start game
   - Show options
   - Quit application
   - Load/save game

3. **Persistence**
   - Configuration file saving
   - Option loading on startup

### Code Location

```
GeneralsMD/Code/Menu/Action/
GeneralsMD/Code/Menu/Handler/
Core/GameEngine/Config/
```

### Action System

```cpp
enum MenuAction {
    StartGame,
    ShowOptions,
    QuitGame,
    LoadGame,
    HighScores,
};

class ActionHandler {
public:
    void RegisterHandler(MenuAction action, std::function<void()> handler);
    void Dispatch(MenuAction action);
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Action system: 0.5 day
- Handlers: 1 day
- Persistence: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Test menu actions
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "Action\|Menu"
```

## Success Criteria

- All menu actions work correctly
- Game launches from menu
- Options save and load
- No crashes from invalid actions
- User can quit cleanly

## Reference Documentation

- Observer pattern for action dispatch
- Configuration file formats
- State persistence patterns
- See PHASE30 for game launch integration
