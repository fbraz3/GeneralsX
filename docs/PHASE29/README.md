# PHASE29: Gameplay Rendering

## Phase Title

Gameplay Rendering - Render Game Objects

## Objective

Render gameplay objects with materials, lighting, and animations during gameplay.

## Dependencies

- PHASE07 (Mesh Rendering)
- PHASE21 (GameObject System)
- PHASE25 (Selection System)

## Key Deliverables

1. Game object rendering
2. Animated mesh support
3. Unit state visualization
4. Selection highlighting in-game
5. Damage/status indicators

## Acceptance Criteria

- [ ] All units render correctly
- [ ] Animations play smoothly
- [ ] Selection highlighting visible
- [ ] Status effects display properly
- [ ] Performance at 60 FPS with many units
- [ ] No z-fighting or rendering artifacts

## Technical Details

### Components to Implement

1. **Renderer**
   - Draw game objects
   - Apply materials/lighting
   - Animation state management

2. **Animation System**
   - Skeletal animation
   - Animation blending
   - State machine for animations

3. **Visual Effects**
   - Damage indicators
   - Status effects (glowing, etc.)
   - Particle emission

### Code Location

```
GeneralsMD/Code/Game/Rendering/
GeneralsMD/Code/Game/Animation/
GeneralsMD/Code/Game/Effects/
```

### Game Renderer

```cpp
class GameRenderer {
public:
    void RenderGameObjects(const World* world, const Camera* camera);
    void RenderSelection();
    void RenderEffects();
    
private:
    void RenderUnit(const Unit* unit, const glm::mat4& view);
    void ApplyAnimationState(Mesh* mesh, const AnimationState& state);
};
```

## Estimated Scope

**LARGE** (4-5 days)

- Object rendering: 1 day
- Animation system: 1.5 days
- Status effects: 1 day
- Optimization: 1 day
- Testing: 0.5 day

## Status

**not-started**

## Testing Strategy

```bash
# Render gameplay with units
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "Unit\|Render"
```

## Success Criteria

- All units visible and properly rendered
- Animations smooth and synchronized
- Selection and effects clearly visible
- 60 FPS maintained
- No visual anomalies

## Reference Documentation

- Skeletal animation
- Animation blending
- State machine for animations
- Particle systems
