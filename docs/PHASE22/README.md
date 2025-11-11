# PHASE22: World Management

## Phase Title

World Management - Object Container and Scene Graph

## Objective

Implement world/scene management to hold and manage all game objects with spatial organization.

## Dependencies

- PHASE21 (GameObject System)
- PHASE07 (Mesh Rendering)

## Key Deliverables

1. World/Scene container
2. Object management (spawn, despawn)
3. Spatial grid/partitioning
4. Update batching
5. Query interface (find objects, raycast)

## Acceptance Criteria

- [ ] Objects spawn and despawn correctly
- [ ] All objects update each frame
- [ ] Spatial queries work efficiently
- [ ] Memory managed correctly
- [ ] No update order issues
- [ ] Performance acceptable with 10000+ objects

## Technical Details

### Components to Implement

1. **World Class**
   - Object collection
   - Update scheduling
   - Spawn/despawn interface

2. **Spatial Partitioning**
   - Grid-based or quadtree
   - Dynamic object movement
   - Query acceleration

3. **Query System**
   - Find objects by tag/name
   - Raycast queries
   - Spatial range queries

### Code Location

```
GeneralsMD/Code/Game/World/
GeneralsMD/Code/Game/Spatial/
```

### World Management

```cpp
class World {
public:
    GameObject* Spawn(const std::string& name, const glm::vec3& position);
    void Despawn(GameObject* obj);
    void Update(float deltaTime);
    void RenderFrame();
    
    std::vector<GameObject*> QueryByTag(const std::string& tag);
    std::vector<GameObject*> QueryByRadius(const glm::vec3& center, float radius);
    
private:
    std::vector<std::unique_ptr<GameObject>> objects;
    SpatialGrid spatialIndex;
};
```

## Estimated Scope

**LARGE** (4-5 days)

- World structure: 1 day
- Object management: 1 day
- Spatial partitioning: 1.5 days
- Query system: 1 day
- Testing/optimization: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Spawn many objects and test queries
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- All objects update correctly
- Spatial queries efficient
- Smooth 60 FPS with 10000 objects
- No memory leaks
- Query results accurate

## Reference Documentation

- Spatial partitioning techniques (grids, quadtrees)
- Scene graph implementation
- Object pooling patterns
