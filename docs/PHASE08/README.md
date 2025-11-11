# PHASE08: Camera System

## Phase Title

Camera System - Viewpoint Control and Modes

## Objective

Implement camera system with multiple viewpoint modes (free camera, isometric, strategy view) and smooth transitions between perspectives.

## Dependencies

- PHASE01 (Geometry Rendering)
- PHASE05 (Render Loop)
- PHASE07 (Mesh Rendering)
- Input system (partial - PHASE16)

## Key Deliverables

1. Free camera implementation
2. Isometric/strategy camera
3. Camera transitions
4. Look-at and projection matrix calculations
5. Camera bounds/constraints

## Acceptance Criteria

- [ ] Free camera moves smoothly
- [ ] Strategy camera behaves like original game
- [ ] Camera transitions are smooth (0.5-1.0 second)
- [ ] Clipping planes prevent near-plane artifacts
- [ ] No gimbal lock issues
- [ ] Performance unaffected by camera changes

## Technical Details

### Components to Implement

1. **Camera Types**
   - FreeCamera (6DOF movement)
   - IsometricCamera (fixed angle, panning)
   - StrategyCamera (top-down for RTS)

2. **View/Projection Matrices**
   - LookAt matrix calculation
   - Perspective projection
   - Orthographic projection

3. **Camera Control**
   - Input mapping (will integrate PHASE16)
   - Smooth animation
   - Constraint enforcement

### Code Location

```
GeneralsMD/Code/Graphics/Camera/
GeneralsMD/Code/Input/Camera/
```

### Camera Class

```cpp
class Camera {
public:
    virtual glm::mat4 GetViewMatrix() const = 0;
    virtual glm::mat4 GetProjectionMatrix() const = 0;
    virtual void Update(float deltaTime) = 0;
    
protected:
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 up;
    float fov;
    float aspect;
    float zNear, zFar;
};
```

## Estimated Scope

**MEDIUM** (3-4 days)

- Camera math: 1 day
- Free camera: 0.5 day
- Strategy camera: 1 day
- Transitions: 1 day
- Testing: 0.5 day

## Status

**not-started**

## Testing Strategy

```bash
# Test camera modes and transitions
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep Camera
```

## Success Criteria

- All camera modes function correctly
- Smooth 60 FPS with camera movement
- No visual artifacts (clipping, distortion)
- Game camera matches original feel

## Reference Documentation

- GLM matrix math library
- 3D graphics camera systems
- Original game camera implementation
