# PHASE30: State Transitions

## Phase Title

State Transitions - Menu to Game and Back

## Objective

Implement state transitions between menu system and gameplay, allowing seamless switching between modes.

## Dependencies

- PHASE18 (Menu State Machine)
- PHASE20 (Menu Interaction)
- PHASE23 (Game Loop)

## Key Deliverables

1. Application state machine
2. Menu → Game transition
3. Game → Menu transition
4. State cleanup/setup
5. Persistent state preservation

## Acceptance Criteria

- [ ] Smooth transition from menu to game
- [ ] Smooth transition from game back to menu
- [ ] No crash on state change
- [ ] Memory properly cleaned up
- [ ] Game state preserved when needed
- [ ] Performance unaffected

## Technical Details

### Components to Implement

1. **Application State Machine**
   - Menu state
   - Game state
   - Loading state

2. **Transitions**
   - Save/restore game state
   - Cleanup old resources
   - Initialize new state

3. **Persistence**
   - Pause state
   - Resume state
   - Game progress saving

### Code Location

```
GeneralsMD/Code/Application/
GeneralsMD/Code/Game/State/
```

### Application State Machine

```cpp
enum AppState {
    MainMenu,
    OptionsMenu,
    InGame,
    Paused,
    Loading,
};

class Application {
public:
    void TransitionToGame();
    void TransitionToMenu();
    void Pause();
    void Resume();
    void Update(float deltaTime);
    void Render();
    
private:
    AppState currentState;
    World* currentWorld;
    Menu* currentMenu;
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- State machine: 0.5 day
- Transition logic: 1 day
- Resource cleanup: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Start game, play briefly, return to menu
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "State\|Transition"
```

## Success Criteria

- Transitions smooth with no visible glitches
- No crashes when switching states
- Memory properly managed
- Game continues correctly after menu return
- 60 FPS maintained

## Reference Documentation

- State machine design patterns
- Resource lifecycle management
- Game architecture patterns
