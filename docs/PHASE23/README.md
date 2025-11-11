# PHASE23: Game Loop

## Phase Title

Game Loop - Main Tick and Update Cycle

## Objective

Implement main game loop that coordinates world updates, input processing, physics simulation, and rendering.

## Dependencies

- PHASE05 (Render Loop)
- PHASE16 (Input Routing)
- PHASE21 (GameObject System)
- PHASE22 (World Management)

## Key Deliverables

1. Game loop structure
2. Phase coordination
3. Time management
4. Update synchronization
5. Profiling hooks

## Acceptance Criteria

- [ ] Game loop runs at stable 60 FPS
- [ ] All subsystems update in correct order
- [ ] Input processes before physics
- [ ] Rendering is last
- [ ] No race conditions
- [ ] Performance profiling available

## Technical Details

### Components to Implement

1. **Game Loop**
   - Input → Physics → Logic → Rendering order
   - Fixed timestep for physics
   - Variable timestep for rendering

2. **Phase Coordination**
   - Input processing
   - Fixed update tick
   - Late update
   - Rendering pass

3. **Profiling**
   - Per-phase timing
   - CPU/GPU utilization
   - Bottleneck detection

### Code Location

```
GeneralsMD/Code/Game/GameLoop/
GeneralsMD/Code/Game/Time/
```

### Game Loop Implementation

```cpp
class Game {
public:
    void Run();
    void Update(float deltaTime);
    void FixedUpdate(float fixedDeltaTime);
    void LateUpdate(float deltaTime);
    void Render();
    
private:
    World* world;
    float accumulatedTime;
    float fixedTimestep;  // 0.0167 for 60 FPS
};
```

## Estimated Scope

**MEDIUM** (3-4 days)

- Loop structure: 1 day
- Phase coordination: 1 day
- Time management: 0.5 day
- Profiling: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Run game and verify loop timing
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | tail -20
```

## Success Criteria

- Stable 60 FPS during gameplay
- All subsystems update correctly
- Profiling shows expected CPU/GPU distribution
- No jittering or stuttering
- Update order correct (verified by logging)

## Reference Documentation

- Game loop design patterns
- Fixed timestep vs variable timestep
- Time management in real-time systems
