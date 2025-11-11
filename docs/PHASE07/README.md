# PHASE07: Mesh Rendering

## Phase Title

Mesh Rendering - Render 3D Meshes with Materials

## Objective

Render loaded meshes with proper material and lighting applied. This integrates PHASE06 mesh loading with material/lighting systems.

## Dependencies

- PHASE03 (Material System)
- PHASE04 (Lighting System)
- PHASE05 (Render Loop)
- PHASE06 (Mesh Loading)

## Key Deliverables

1. Mesh rendering command generation
2. Material binding per mesh part
3. Draw call optimization
4. Frustum culling placeholder
5. Transform matrices (model/world)

## Acceptance Criteria

- [ ] Loaded meshes render with correct materials
- [ ] Lighting applies correctly to mesh surfaces
- [ ] Normals display correctly (verify with normal map visualization)
- [ ] Multiple meshes render without z-fighting
- [ ] Performance acceptable with 100+ meshes visible
- [ ] Transform matrices update correctly

## Technical Details

### Components to Implement

1. **Mesh Renderer**
   - Draw call recording
   - Material binding
   - Texture binding per part

2. **Transform System**
   - Model matrix calculation
   - Position/Rotation/Scale
   - Hierarchical transforms (optional for Phase 1)

3. **Batch Optimization**
   - Combine similar materials
   - Reduce state changes
   - Instance rendering (optional)

### Code Location

```
GeneralsMD/Code/Graphics/MeshRenderer/
GeneralsMD/Code/Graphics/Transform/
```

### Mesh Renderer Class

```cpp
class MeshRenderer {
public:
    void Render(const Mesh* mesh, const glm::mat4& transform, const Material* override = nullptr);
    void RenderBatch(const std::vector<RenderCommand>& commands);
    
private:
    void BindMaterial(const Material* mat);
    void DrawMeshPart(const MeshPart& part, const Mesh* mesh);
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Transform system: 0.5 day
- Rendering logic: 1 day
- Material binding: 0.5 day
- Testing/optimization: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Render scene with multiple meshes
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | tail -20
```

## Success Criteria

- Mesh renders with correct lighting
- No z-fighting or sorting issues
- 60 FPS maintained with 100 visible meshes
- Materials apply correctly per part

## Reference Documentation

- Phong lighting implementation
- Transform matrices and hierarchy
- Frustum culling overview (for PHASE09)
