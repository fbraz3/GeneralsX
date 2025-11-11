# PHASE04: Lighting System

## Phase Title

Lighting System - D3D Lights to Vulkan/Metal

## Objective

Port the DirectX 8 lighting system to modern graphics APIs. Implement support for directional lights, point lights, and ambient lighting with proper material interactions.

## Dependencies

- PHASE01 (Geometry Rendering)
- PHASE03 (Material System)
- Uniform buffer structures for light data

## Key Deliverables

1. Directional light support
2. Point light support
3. Ambient light support
4. Light array management
5. Normal map support (if applicable)
6. Shadow map placeholder

## Acceptance Criteria

- [ ] Directional lights illuminate geometry correctly
- [ ] Point lights have proper falloff
- [ ] Lighting changes update in real-time
- [ ] Maximum of 64 active lights maintained
- [ ] Proper normal calculations in fragment shader
- [ ] No lighting artifacts (banding, aliasing)

## Technical Details

### Components to Implement

1. **Light Types**
   - DirectionalLight (sun)
   - PointLight (street lamps, explosions)
   - SpotLight (optional for Phase 1)

2. **Lighting Calculations**
   - Phong/Blinn-Phong model
   - Normal distribution
   - Ambient + Diffuse + Specular

3. **Light Manager**
   - Maximum 64 lights
   - Dynamic add/remove
   - Frustum culling for lights

### Code Location

```
Core/GameEngineDevice/Source/Lighting/
GeneralsMD/Code/Graphics/Lighting/
```

### Light Structure

```cpp
struct Light {
    glm::vec4 position;      // w = 0 for directional
    glm::vec4 direction;     // normalized
    glm::vec4 ambient;       // RGBA
    glm::vec4 diffuse;       // RGBA
    glm::vec4 specular;      // RGBA
    float range;
    float attenuation;
    uint32_t type;  // 0=dir, 1=point, 2=spot
    uint32_t _pad;
};
```

## Estimated Scope

**LARGE** (4-5 days)

- Light structures: 0.5 day
- Shader implementation: 1.5 days
- Light manager: 1 day
- Normal mapping: 1 day
- Testing/optimization: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Test with various light configurations
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
# Verify lighting matches original game
```

## Success Criteria

- Lit scene visually matches game screenshots
- No lighting pop-in or flicker
- Performance acceptable with 64 lights
- Proper shadow/normal interactions

## Reference Documentation

- Phong lighting model
- OpenGL lighting vs DirectX differences
- See `references/dxgldotorg-dxgl/` for lighting patterns
