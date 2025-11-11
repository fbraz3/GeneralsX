# PHASE09: Frustum Culling

## Phase Title

Frustum Culling - Visibility Optimization

## Objective

Implement frustum culling to skip rendering of objects outside the camera view, reducing GPU load and improving performance.

## Dependencies

- PHASE07 (Mesh Rendering)
- PHASE08 (Camera System)
- Bounding volume calculations

## Key Deliverables

1. Frustum calculation from camera
2. AABB/Sphere intersection tests
3. Culling logic in render pipeline
4. Occlusion culling placeholder
5. Culling statistics (debug info)

## Acceptance Criteria

- [ ] Objects outside frustum are not rendered
- [ ] Culling calculations are accurate
- [ ] No visible pop-in at frustum edges
- [ ] Performance improvement measurable (20%+ expected)
- [ ] No false culling (objects disappearing when visible)
- [ ] Debug visualization available

## Technical Details

### Components to Implement

1. **Frustum Calculation**
   - Extract planes from view-projection matrix
   - Normalize plane equations

2. **Intersection Tests**
   - AABB vs Frustum
   - Sphere vs Frustum

3. **Culling System**
   - Spatial partitioning (octree/grid)
   - Cull checks before rendering

### Code Location

```
Core/GameEngineDevice/Source/Culling/
GeneralsMD/Code/Graphics/Culling/
```

### Frustum Culling Implementation

```cpp
class Frustum {
public:
    void CalculateFromMatrix(const glm::mat4& viewProj);
    bool IsAABBVisible(const AABB& box) const;
    bool IsSphereVisible(const glm::vec3& center, float radius) const;
    
private:
    glm::vec4 planes[6];  // left, right, top, bottom, near, far
};
```

## Estimated Scope

**SMALL** (2-3 days)

- Frustum math: 0.5 day
- Intersection tests: 0.5 day
- Integration: 0.5 day
- Optimization/testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Profile performance with and without culling
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "Culled\|FPS"
```

## Success Criteria

- 20%+ FPS improvement in complex scenes
- No visual artifacts
- Culling matches camera frustum exactly
- Statistics show correct cull counts

## Reference Documentation

- Frustum extraction from matrices
- AABB intersection algorithms
- Spatial partitioning data structures
