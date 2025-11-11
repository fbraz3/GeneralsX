# PHASE28: Physics Stub

## Phase Title

Physics Stub - Placeholder Implementation

## Objective

Implement basic physics system for collisions and movement. Full physics deferred for optimization.

## Dependencies

- PHASE21 (GameObject System)
- PHASE22 (World Management)
- PHASE26 (Command System)
- Understanding of Critical Lesson 1: VFS/Asset System Pattern

## Key Deliverables

1. Collision detection (AABB/sphere)
2. Collision response
3. Rigid body components
4. Gravity simulation
5. Static/dynamic objects

## Acceptance Criteria

- [ ] Collisions detected accurately
- [ ] Objects collide and respond correctly
- [ ] No objects fall through terrain
- [ ] Performance acceptable with 1000+ bodies
- [ ] Physics is deterministic for testing

## Technical Details

### Components to Implement

1. **Physics Engine**
   - AABB/sphere collision
   - Response calculation
   - Constraint solving

2. **Body System**
   - Rigid body component
   - Mass/friction/restitution
   - Force application

3. **Simulation**
   - Gravity
   - Velocity integration
   - Constraint relaxation

### Code Location

```
Core/GameEngineDevice/Source/Physics/
GeneralsMD/Code/Game/Physics/
```

### Physics System

```cpp
class RigidBody : public Component {
public:
    void SetVelocity(const glm::vec3& vel);
    void ApplyForce(const glm::vec3& force);
    bool IsColliding() const;
    
private:
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float mass;
};

class PhysicsSystem {
public:
    void Update(float deltaTime);
    void CheckCollisions();
    void ResolveCollisions();
};
```

## Estimated Scope

**LARGE** (4-5 days)

- Collision detection: 1.5 days
- Response calculation: 1 day
- Integration: 1 day
- Optimization: 1 day
- Testing: 0.5 day

## Status

**not-started**

## Important References
- **Critical Lesson 1**: `docs/PHASE00/CRITICAL_LESSONS.md` - VFS Pattern & Post-DirectX Interception (Phase 28.4 discovery)
- **Asset System**: `docs/PHASE00/ASSET_SYSTEM.md` - Complete .big file handling and texture loading patterns

## Priority

**MEDIUM** - Important but deferrable for polish phase

## Testing Strategy

```bash
# Test collisions and physics interaction
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Accurate collision detection
- Realistic physics response
- Performance acceptable
- No physics instability
- Deterministic for testing

## Reference Documentation

- Box2D/Bullet physics algorithms
- AABB collision detection
- Impulse-based collision response
- GJK algorithm (optional)
