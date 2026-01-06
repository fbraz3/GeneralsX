# Phase 62: W3D Model Rendering - Units and Buildings

**Phase**: 62
**Title**: W3D Mesh Loading and Rendering
**Duration**: 7-10 days
**Status**: NOT STARTED
**Dependencies**: Phase 61 complete (Terrain working)

---

## Overview

Phase 62 implements W3D model rendering for units, buildings, and props. This enables the actual game objects to appear in the world.

### Current State (Post-Phase 61)

- ✅ Terrain renders correctly
- ✅ Skybox displays
- ❌ W3D models not rendering
- ❌ Unit meshes not visible
- ❌ Building meshes not visible
- ❌ Animations not playing

### Goal

Render W3D models with proper materials, textures, and basic animation.

---

## W3D Format Overview

### File Structure

```
.W3D file
├── Header (version, chunk count)
├── Mesh chunks
│   ├── Vertices
│   ├── Normals
│   ├── UVs
│   ├── Faces (triangles)
│   └── Materials
├── Hierarchy (bones/skeleton)
├── Animation data
└── Aggregate (multiple meshes)
```

### W3D Mesh Structure

```cpp
struct W3DMesh {
    std::string name;
    std::vector<W3DVertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<W3DMaterial> materials;
    BoundingBox bounds;
};

struct W3DVertex {
    float x, y, z;        // Position
    float nx, ny, nz;     // Normal
    float u, v;           // Texture coordinates
    uint8_t bone_indices[4];  // For skinning
    float bone_weights[4];     // For skinning
};
```

---

## Technical Requirements

### 1. W3D Parser

The codebase already has W3D loading code. Need to:
1. Verify mesh data reaches Vulkan backend
2. Create vertex/index buffers from W3D data
3. Load associated textures

### 2. Material System

W3D materials define:
- Diffuse texture
- Shader type (vertex lit, pixel lit, etc.)
- Blend mode (opaque, alpha, additive)
- Two-sided flag

```cpp
struct W3DMaterial {
    uint32_t diffuse_texture;
    uint32_t shader_type;
    bool alpha_blend;
    bool two_sided;
    float specular_power;
};
```

### 3. Skinned Mesh (Animation)

For animated units:

```glsl
// Vertex shader with skeletal animation
uniform mat4 bones[MAX_BONES];

void main() {
    mat4 skinMatrix = 
        bones[bone_indices[0]] * bone_weights[0] +
        bones[bone_indices[1]] * bone_weights[1] +
        bones[bone_indices[2]] * bone_weights[2] +
        bones[bone_indices[3]] * bone_weights[3];
    
    vec4 skinnedPos = skinMatrix * vec4(inPosition, 1.0);
    gl_Position = mvp * skinnedPos;
}
```

---

## Implementation Plan

### Week 1: Static Meshes

#### Day 1-2: W3D to Vulkan Buffer

```cpp
uint32_t LoadW3DMesh(const W3DMesh& mesh) {
    // Create vertex buffer
    uint32_t vb = CreateVertexBuffer(
        mesh.vertices.size() * sizeof(W3DVertex),
        mesh.vertices.data()
    );
    
    // Create index buffer
    uint32_t ib = CreateIndexBuffer(
        mesh.indices.size() * sizeof(uint16_t),
        mesh.indices.data(),
        false  // 16-bit indices
    );
    
    return RegisterMesh(vb, ib, mesh.indices.size());
}
```

#### Day 3-4: Material Binding

```cpp
void ApplyW3DMaterial(const W3DMaterial& mat) {
    // Set texture
    SetTexture(0, mat.diffuse_texture);
    
    // Set blend mode
    if (mat.alpha_blend) {
        SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    } else {
        SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }
    
    // Set culling
    if (mat.two_sided) {
        SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    } else {
        SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    }
}
```

#### Day 5: Model Rendering Pipeline

```cpp
void RenderW3DModel(const W3DModel& model, const D3DXMATRIX& transform) {
    SetTransform(D3DTS_WORLD, &transform);
    
    for (const auto& mesh : model.meshes) {
        // Bind mesh buffers
        SetStreamSource(0, mesh.vertex_buffer);
        SetIndices(mesh.index_buffer);
        
        // Apply material
        ApplyW3DMaterial(mesh.material);
        
        // Draw
        DrawIndexedPrimitive(
            D3DPT_TRIANGLELIST,
            0, 0, mesh.vertex_count,
            0, mesh.triangle_count
        );
    }
}
```

### Week 2: Animation

#### Day 6-7: Bone Hierarchy

```cpp
struct W3DBone {
    std::string name;
    int parent_index;  // -1 for root
    D3DXMATRIX bind_pose;
    D3DXMATRIX inverse_bind;
};

struct W3DSkeleton {
    std::vector<W3DBone> bones;
    
    void CalculateWorldMatrices(std::vector<D3DXMATRIX>& out) {
        for (int i = 0; i < bones.size(); ++i) {
            if (bones[i].parent_index < 0) {
                out[i] = bones[i].bind_pose;
            } else {
                out[i] = bones[i].bind_pose * out[bones[i].parent_index];
            }
        }
    }
};
```

#### Day 8-9: Animation Playback

```cpp
struct W3DAnimation {
    std::string name;
    float duration;
    std::vector<W3DAnimChannel> channels;  // Per-bone animation
};

struct W3DAnimChannel {
    int bone_index;
    std::vector<float> times;
    std::vector<D3DXVECTOR3> positions;
    std::vector<D3DXQUATERNION> rotations;
};

D3DXMATRIX EvaluateAnimation(const W3DAnimation& anim, int bone, float time) {
    // Interpolate between keyframes
    // Return animated bone transform
}
```

#### Day 10: Skinning Shader

UBO for bone matrices:

```cpp
struct SkeletonUBO {
    D3DXMATRIX bones[MAX_BONES];  // 64 bones max
};

// Update UBO before drawing skinned mesh
void UpdateSkeletonUBO(const std::vector<D3DXMATRIX>& bones) {
    memcpy(m_skeleton_ubo_data, bones.data(), bones.size() * sizeof(D3DXMATRIX));
    // Upload to GPU
}
```

---

## Shader for W3D Models

### w3d_static.vert

```glsl
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 world;
    vec3 lightDir;
} pc;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float fragLighting;

void main() {
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    
    vec3 worldNormal = normalize(mat3(pc.world) * inNormal);
    fragLighting = std::max(dot(worldNormal, normalize(pc.lightDir)), 0.2);
}
```

### w3d_skinned.vert

```glsl
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uvec4 inBoneIndices;
layout(location = 4) in vec4 inBoneWeights;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 world;
    vec3 lightDir;
} pc;

layout(set = 0, binding = 0) uniform SkeletonUBO {
    mat4 bones[64];
} skeleton;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float fragLighting;

void main() {
    mat4 skinMatrix = 
        skeleton.bones[inBoneIndices.x] * inBoneWeights.x +
        skeleton.bones[inBoneIndices.y] * inBoneWeights.y +
        skeleton.bones[inBoneIndices.z] * inBoneWeights.z +
        skeleton.bones[inBoneIndices.w] * inBoneWeights.w;
    
    vec4 skinnedPos = skinMatrix * vec4(inPosition, 1.0);
    vec3 skinnedNormal = mat3(skinMatrix) * inNormal;
    
    gl_Position = pc.mvp * skinnedPos;
    fragTexCoord = inTexCoord;
    
    vec3 worldNormal = normalize(mat3(pc.world) * skinnedNormal);
    fragLighting = std::max(dot(worldNormal, normalize(pc.lightDir)), 0.2);
}
```

---

## Testing Strategy

### Test 1: Simple Static Mesh

Load simple W3D model (crate, barrel), verify renders.

### Test 2: Textured Model

Verify texture maps correctly to mesh.

### Test 3: Building

Load building model, verify all meshes render.

### Test 4: Unit (Static Pose)

Load unit model without animation.

### Test 5: Unit (Animated)

Play idle animation, verify movement.

### Test 6: Multiple Units

Render multiple units, verify performance.

---

## Success Criteria

- [ ] W3D mesh data converts to Vulkan buffers
- [ ] Static meshes render correctly
- [ ] Textures apply to meshes
- [ ] Materials affect rendering
- [ ] Bone hierarchy calculated
- [ ] Animation keyframes interpolate
- [ ] Skinned meshes animate
- [ ] Units visible in game world

---

## Files to Modify

1. `vulkan_graphics_driver.cpp` - UBO support for bones
2. `shaders/w3d_static.vert` - Static mesh shader
3. `shaders/w3d_skinned.vert` - Skinned mesh shader
4. `shaders/w3d.frag` - Model fragment shader

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| W3D parsing issues | High | Use reference implementations |
| Bone order mismatch | High | Debug with known model |
| Animation interpolation | Medium | Test with simple animation |
| Performance (many units) | Medium | Implement instancing later |

---

**Created**: November 28, 2025
**Author**: GeneralsX Development Team
