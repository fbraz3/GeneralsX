# Phase 61: 3D World Rendering - Terrain and Sky

**Phase**: 61
**Title**: Terrain, Heightmap, and Skybox Rendering
**Duration**: 5-7 days
**Status**: NOT STARTED
**Dependencies**: Phase 60 complete (UI working)

---

## Overview

Phase 61 implements terrain and skybox rendering, enabling the game world to appear. This is critical for gameplay as terrain forms the foundation of all battles.

### Current State (Post-Phase 60)

- ✅ UI/Menu renders correctly
- ❌ Terrain mesh not rendering
- ❌ Terrain textures not blending
- ❌ Skybox not visible
- ❌ Water surfaces not rendering

### Goal

Render complete game world with terrain, sky, and environmental elements.

---

## Terrain System Overview

### W3D Terrain Architecture

```
HeightMapRenderObj
├── Heightmap data (elevation grid)
├── Terrain textures (blend layers)
├── Cliff textures
├── Shore textures
└── Passability data
```

### Terrain Rendering Path

```
1. TerrainVisual loads .map file
2. HeightMapRenderObj processes heightmap
3. Terrain chunks generated for LOD
4. Texture blending computed
5. Chunks rendered with multi-texture
```

---

## Technical Requirements

### 1. Heightmap Mesh Generation

```cpp
struct TerrainVertex {
    float x, y, z;      // Position (Y is height)
    float nx, ny, nz;   // Normal
    float u, v;         // Base texture UV
    float bu, bv;       // Blend texture UV
    float blend;        // Blend factor
};
```

### 2. Multi-Texture Blending

Terrain uses multiple texture layers:

- Layer 0: Base texture (e.g., grass)
- Layer 1: Blend texture (e.g., dirt)
- Blend map: Per-vertex blend weight

```glsl
// terrain.frag
uniform sampler2D baseTexture;
uniform sampler2D blendTexture;

in float blendFactor;

void main() {
    vec4 base = texture(baseTexture, fragTexCoord0);
    vec4 blend = texture(blendTexture, fragTexCoord1);
    outColor = mix(base, blend, blendFactor);
}
```

### 3. Skybox Rendering

```cpp
// Skybox is a cube with 6 face textures
// Rendered first, with depth write disabled
// Camera centered at origin (no translation)
```

---

## Implementation Plan

### Week 1: Terrain Mesh

#### Day 1-2: Heightmap Parsing

The .map file contains heightmap data:

```cpp
struct HeightmapData {
    uint32_t width, height;
    std::vector<float> elevations;  // Y values
    float scale_x, scale_z;
    float min_height, max_height;
};
```

#### Day 3-4: Normal Calculation

Calculate normals for lighting:

```cpp
vec3 CalculateNormal(int x, int z) {
    float hL = GetHeight(x - 1, z);
    float hR = GetHeight(x + 1, z);
    float hD = GetHeight(x, z - 1);
    float hU = GetHeight(x, z + 1);
    
    vec3 normal = normalize(vec3(hL - hR, 2.0, hD - hU));
    return normal;
}
```

#### Day 5: Terrain Chunks

Divide terrain into chunks for:
- Frustum culling
- LOD (Level of Detail)
- Efficient rendering

### Week 2: Texturing and Sky

#### Day 6: Multi-Texture Binding

Bind multiple textures for terrain:

```cpp
SetTexture(0, base_texture);
SetTexture(1, blend_texture);
// Shader samples both and blends
```

#### Day 7: Skybox

```cpp
void RenderSkybox() {
    // Disable depth write
    SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    
    // Remove camera translation
    D3DXMATRIX view = m_view_matrix;
    view._41 = view._42 = view._43 = 0;  // Zero translation
    SetTransform(D3DTS_VIEW, &view);
    
    // Draw skybox cube
    DrawSkyboxCube();
    
    // Restore
    SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    SetTransform(D3DTS_VIEW, &m_view_matrix);
}
```

---

## Terrain Shader

### terrain.vert

```glsl
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord0;
layout(location = 3) in vec2 inTexCoord1;
layout(location = 4) in float inBlend;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 world;
    vec3 lightDir;
} pc;

layout(location = 0) out vec2 fragTexCoord0;
layout(location = 1) out vec2 fragTexCoord1;
layout(location = 2) out float fragBlend;
layout(location = 3) out float fragLighting;

void main() {
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    
    fragTexCoord0 = inTexCoord0;
    fragTexCoord1 = inTexCoord1;
    fragBlend = inBlend;
    
    // Simple diffuse lighting
    vec3 worldNormal = mat3(pc.world) * inNormal;
    fragLighting = max(dot(normalize(worldNormal), normalize(pc.lightDir)), 0.3);
}
```

### terrain.frag

```glsl
#version 450

layout(location = 0) in vec2 fragTexCoord0;
layout(location = 1) in vec2 fragTexCoord1;
layout(location = 2) in float fragBlend;
layout(location = 3) in float fragLighting;

layout(binding = 0) uniform sampler2D baseTexture;
layout(binding = 1) uniform sampler2D blendTexture;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 base = texture(baseTexture, fragTexCoord0);
    vec4 blend = texture(blendTexture, fragTexCoord1);
    vec4 terrainColor = mix(base, blend, fragBlend);
    
    outColor = vec4(terrainColor.rgb * fragLighting, 1.0);
}
```

---

## Skybox Shader

### skybox.vert

```glsl
#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;  // View (without translation) * Projection
} pc;

layout(location = 0) out vec3 fragTexCoord;

void main() {
    fragTexCoord = inPosition;
    vec4 pos = pc.viewProj * vec4(inPosition, 1.0);
    gl_Position = pos.xyww;  // Z = W for max depth
}
```

### skybox.frag

```glsl
#version 450

layout(location = 0) in vec3 fragTexCoord;

layout(binding = 0) uniform samplerCube skybox;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(skybox, fragTexCoord);
}
```

---

## Testing Strategy

### Test 1: Flat Terrain

Load simple flat heightmap, verify mesh renders.

### Test 2: Hills

Load heightmap with elevation, verify 3D terrain.

### Test 3: Terrain Textures

Verify base texture tiles correctly.

### Test 4: Texture Blending

Verify blend transitions look smooth.

### Test 5: Skybox

Verify skybox surrounds camera.

### Test 6: Complete Map

Load full game map, verify terrain and sky.

---

## Success Criteria

- [ ] Heightmap mesh generates correctly
- [ ] Terrain normals compute correctly
- [ ] Base texture maps to terrain
- [ ] Blend textures transition smoothly
- [ ] Skybox renders around camera
- [ ] Skybox follows camera rotation
- [ ] Full map loads and displays

---

## Files to Modify

1. `vulkan_graphics_driver.cpp` - Multi-texture support
2. `shaders/terrain.vert` - Terrain vertex shader
3. `shaders/terrain.frag` - Terrain fragment shader
4. `shaders/skybox.vert` - Skybox vertex shader
5. `shaders/skybox.frag` - Skybox fragment shader

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Heightmap parsing errors | High | Verify with known map |
| Texture coordinate issues | Medium | Debug with UV visualization |
| Skybox seams | Low | Ensure seamless textures |
| LOD popping | Low | Smooth LOD transitions |

---

**Created**: November 28, 2025
**Author**: GeneralsX Development Team
