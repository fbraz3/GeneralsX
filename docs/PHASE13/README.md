# PHASE13: UI Rendering

## Phase Title

UI Rendering - Orthographic Projection and Layers

## Objective

Implement UI rendering with orthographic projection and layering system for proper depth and z-ordering.

## Dependencies

- PHASE05 (Render Loop)
- PHASE07 (Mesh Rendering)
- PHASE11 (UI Mesh Format)
- PHASE12 (UI Texture Loading)

## Key Deliverables

1. Orthographic projection setup
2. UI layer system (background, normal, overlay)
3. Depth sorting for UI elements
4. Render state management for UI
5. Screen coordinate transformation

## Acceptance Criteria

- [ ] UI renders in correct 2D space
- [ ] Layers sort correctly (no z-fighting)
- [ ] Coordinates map screen space properly
- [ ] Transparency (alpha) works correctly
- [ ] Performance acceptable with many UI elements

## Technical Details

### Components to Implement

1. **Orthographic Rendering**
   - Projection matrix for 2D rendering
   - Screen space coordinate system
   - Viewport management

2. **Layer System**
   - Layer depth assignment
   - Sorting by layer/z-order
   - Batch rendering by layer

3. **Render State**
   - Alpha blending for transparency
   - UI-specific shader

### Code Location

```
GeneralsMD/Code/UI/Renderer/
GeneralsMD/Code/Graphics/UIRenderState/
```

### UI Rendering Pipeline

```cpp
class UIRenderer {
public:
    void RenderUI(const UILayer& layer);
    void SetScreenSize(uint32_t width, uint32_t height);
    void AddElement(UIElement* elem, uint32_t layer, float z);
    
private:
    glm::mat4 GetOrthographicMatrix() const;
    void RenderLayer(uint32_t layer);
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Orthographic setup: 0.5 day
- Layer system: 1 day
- Shader creation: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Render UI and verify layering
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "UI\|Layer"
```

## Success Criteria

- All UI elements visible in correct positions
- Proper z-ordering and layering
- No visual artifacts
- 60 FPS maintained

## Reference Documentation

- Orthographic projection matrices
- Z-order sorting algorithms
- 2D rendering techniques
