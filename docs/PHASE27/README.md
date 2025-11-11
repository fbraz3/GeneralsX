# PHASE27: Pathfinding Stub

## Phase Title

Pathfinding Stub - Placeholder Implementation

## Objective

Implement basic pathfinding stub using A* algorithm. Deferred for optimization until core gameplay works.

## Dependencies

- PHASE22 (World Management - spatial queries)
- PHASE26 (Command System)

## Key Deliverables

1. A* pathfinding algorithm
2. Walkable terrain detection
3. Path caching (optional)
4. Smooth path following
5. Dynamic obstacle avoidance (stub)

## Acceptance Criteria

- [ ] Units find paths to destinations
- [ ] Paths avoid basic obstacles
- [ ] Pathfinding completes in reasonable time
- [ ] Paths are smooth
- [ ] Performance acceptable for 100+ units

## Technical Details

### Components to Implement

1. **Pathfinding**
   - A* implementation
   - Grid-based navigation
   - Path smoothing

2. **Movement**
   - Follow path
   - Obstacle avoidance
   - Arrival detection

3. **Terrain**
   - Walkability checking
   - Height awareness
   - Dynamic updates

### Code Location

```
GeneralsMD/Code/Game/Pathfinding/
GeneralsMD/Code/Game/Navigation/
```

### Pathfinding Interface

```cpp
class Pathfinder {
public:
    bool FindPath(const glm::vec3& start, const glm::vec3& goal,
                  std::vector<glm::vec3>& outPath);
    void InvalidateCache();
    
private:
    bool IsWalkable(const glm::vec3& pos) const;
    float Heuristic(const glm::vec3& a, const glm::vec3& b) const;
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- A* implementation: 1 day
- Terrain integration: 0.5 day
- Path smoothing: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Priority

**MEDIUM** - Important for gameplay but can be optimized later

## Testing Strategy

```bash
# Send units to move and verify pathfinding
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Units navigate to target
- Reasonable path quality
- No infinite loops
- Performance acceptable
- Obstacle avoidance works

## Reference Documentation

- A* algorithm documentation
- RTS pathfinding techniques
- Grid-based navigation
- Path smoothing algorithms
