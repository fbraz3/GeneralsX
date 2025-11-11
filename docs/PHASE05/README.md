# PHASE05: Render Loop

## Phase Title

Render Loop - Frame Timing and 60 FPS Target

## Objective

Implement a stable render loop that maintains 60 FPS frame rate with proper synchronization, delta time tracking, and frame pacing.

## Dependencies

- PHASE01 (Geometry Rendering)
- PHASE02 (Texture System)
- PHASE03 (Material System)
- PHASE04 (Lighting System)

## Key Deliverables

1. Frame pacing mechanism (60 FPS target)
2. Delta time calculation
3. Frame rate monitoring
4. Vsync support
5. Frame profiling hooks

## Acceptance Criteria

- [ ] Stable 60 FPS achieved and maintained
- [ ] Frame time variance less than 2ms
- [ ] Delta time accurate for physics/animation
- [ ] Vsync toggleable without crash
- [ ] FPS counter displays correctly
- [ ] No frame skipping or stuttering

## Technical Details

### Components to Implement

1. **Timer System**
   - High-resolution clock (nanoseconds if possible)
   - Frame time calculation
   - Accumulated time tracking

2. **Frame Pacing**
   - Target frame time (16.67ms for 60 FPS)
   - Sleep mechanism
   - Spin-wait fallback

3. **Profiling Hooks**
   - Frame time metrics
   - CPU/GPU time separation (if possible)
   - Bottleneck detection

### Code Location

```
Core/GameEngineDevice/Source/RenderLoop/
GeneralsMD/Code/Graphics/FrameTimer/
```

### Frame Timer Implementation

```cpp
class FrameTimer {
public:
    void Update();
    float GetDeltaTime() const { return deltaTime; }
    float GetFPS() const { return 1.0f / deltaTime; }
    void SetTargetFPS(float fps) { targetFrameTime = 1.0f / fps; }
    
private:
    float deltaTime;
    float targetFrameTime;  // 1/60 = 0.0167
    uint64_t lastTime;
};
```

## Estimated Scope

**SMALL** (1-2 days)

- Timer implementation: 0.5 day
- Frame pacing: 0.5 day
- Testing/validation: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Run with FPS counter
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep FPS
```

## Success Criteria

- FPS consistently at 60 (within 0.5 FPS variance)
- No frame drops or stuttering
- Delta time accurate for physics simulation
- Performance profile shows balanced CPU/GPU

## Reference Documentation

- See PHASE01-04 for graphics setup
- High-resolution timing APIs (std::chrono)
- Platform-specific vsync APIs
