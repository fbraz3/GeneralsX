# PHASE06: Mesh Loading

## Phase Title

Mesh Loading - Parse 3D Models from .big Archives

## Objective

Implement mesh file parsing directly from the game's .big archives. Support the game's native mesh formats with vertex data, normals, and material assignments.

## Dependencies

- PHASE01 (Geometry Rendering)
- PHASE02 (Texture System)
- PHASE05 (Render Loop)
- .big archive file I/O (from existing engine)

## Key Deliverables

1. .big archive reading integration
2. Mesh format parser
3. Vertex buffer creation
4. Index buffer creation
5. Mesh metadata (material assignments, bounds)

## Acceptance Criteria

- [ ] Game mesh files parse without crashes
- [ ] Vertex data correctly transferred to GPU
- [ ] Index buffers properly reference vertices
- [ ] Bounds calculated correctly
- [ ] Multiple meshes can be loaded simultaneously
- [ ] No memory leaks in loader

## Technical Details

### CRITICAL: Asset System Pattern
Meshes are stored in `.big` archives. This phase implements post-DirectX interception pattern discovered in Phase 28.4:
- DirectX loads and decompresses mesh data from .big
- Intercept at D3D8 resource level
- Extract to GPU-compatible format

See `docs/PHASE00/ASSET_SYSTEM.md` for complete architecture.

### Components to Implement

1. **Archive Integration**
   - .big file interface usage
   - Memory stream reading
   - Decompression if needed

2. **Mesh Parser**
   - Header parsing (magic number, version)
   - Vertex array parsing
   - Index array parsing
   - Material references

3. **GPU Resources**
   - Vertex buffer creation
   - Index buffer creation
   - GPU upload

### Code Location

```
Core/GameEngineDevice/Source/Mesh/
GeneralsMD/Code/Graphics/MeshLoader/
```

### Mesh Structure Example

```cpp
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<MeshPart> parts;  // Material ranges
    AABB bounds;
};

struct MeshPart {
    uint32_t startIndex;
    uint32_t indexCount;
    uint32_t materialId;
};
```

## Estimated Scope

**MEDIUM** (3-4 days)

- Archive integration: 0.5 day
- Format analysis: 1 day
- Parser implementation: 1.5 days
- Testing: 1 day

## Status

**not-started**

## Important References
- **Asset System**: `docs/PHASE00/ASSET_SYSTEM.md` - .big file handling and VFS patterns
- **Critical Lesson 1**: `docs/PHASE00/CRITICAL_LESSONS.md` - Post-DirectX interception (Phase 28.4 discovery)

## Testing Strategy

```bash
# Load test mesh from .big files
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "Mesh\|loaded"
```

## Success Criteria

- Game meshes render without distortion
- Correct vertex counts and topology
- Fast loading (< 100ms per mesh)
- No memory corruption

## Reference Documentation

- See `docs/MISC/BIG_FILES_REFERENCE.md` for mesh archive locations
- Look at original game code for format spec
- See PHASE02 for texture integration patterns
