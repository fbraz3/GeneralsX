# Phase 59: Render States and Blend Modes

**Phase**: 59
**Title**: Render States, Blend Modes, and Depth Testing
**Duration**: 4-5 days
**Status**: NOT STARTED
**Dependencies**: Phase 58 complete (Transforms working)

---

## Overview

Phase 59 implements render state management including blend modes, depth testing, culling, and other GPU state. This enables proper transparency, depth sorting, and visual effects.

### Current State (Post-Phase 58)

- ✅ Geometry transforms correctly
- ✅ Textures display
- ❌ `SetRenderState()` is stub
- ❌ No alpha blending
- ❌ No depth testing
- ❌ All polygons render solid

### Goal

Enable alpha blending, depth buffer, culling, and other render states.

---

## Technical Requirements

### 1. Critical Render States

| D3D State | Vulkan Equivalent | Pipeline Stage |
|-----------|-------------------|----------------|
| `D3DRS_ZENABLE` | depthTestEnable | Depth/Stencil |
| `D3DRS_ZWRITEENABLE` | depthWriteEnable | Depth/Stencil |
| `D3DRS_ZFUNC` | depthCompareOp | Depth/Stencil |
| `D3DRS_ALPHABLENDENABLE` | blendEnable | Color Blend |
| `D3DRS_SRCBLEND` | srcColorBlendFactor | Color Blend |
| `D3DRS_DESTBLEND` | dstColorBlendFactor | Color Blend |
| `D3DRS_CULLMODE` | cullMode | Rasterization |
| `D3DRS_FILLMODE` | polygonMode | Rasterization |
| `D3DRS_LIGHTING` | (shader uniform) | Fragment |
| `D3DRS_ALPHATESTENABLE` | (shader discard) | Fragment |
| `D3DRS_ALPHAREF` | (shader uniform) | Fragment |

### 2. State Tracking

```cpp
struct RenderStateCache {
    // Depth
    bool depth_test_enable = true;
    bool depth_write_enable = true;
    VkCompareOp depth_compare = VK_COMPARE_OP_LESS_OR_EQUAL;
    
    // Blend
    bool blend_enable = false;
    VkBlendFactor src_blend = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dst_blend = VK_BLEND_FACTOR_ZERO;
    VkBlendOp blend_op = VK_BLEND_OP_ADD;
    
    // Rasterization
    VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
    VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL;
    
    // Alpha test (implemented in shader)
    bool alpha_test_enable = false;
    float alpha_ref = 0.0f;
};
```

### 3. Dynamic State

Vulkan 1.3+ supports dynamic state to avoid pipeline recreation:

```cpp
// At pipeline creation
std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
    VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
    VK_DYNAMIC_STATE_CULL_MODE,
};
```

---

## Implementation Plan

### Day 1: State Cache

```cpp
// In VulkanGraphicsDriver
RenderStateCache m_render_state;
bool m_state_dirty = true;

void VulkanGraphicsDriver::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
    switch (state) {
        case D3DRS_ZENABLE:
            m_render_state.depth_test_enable = (value != D3DZB_FALSE);
            break;
        case D3DRS_ZWRITEENABLE:
            m_render_state.depth_write_enable = (value != FALSE);
            break;
        case D3DRS_ALPHABLENDENABLE:
            m_render_state.blend_enable = (value != FALSE);
            break;
        case D3DRS_SRCBLEND:
            m_render_state.src_blend = ConvertBlendFactor((D3DBLEND)value);
            break;
        case D3DRS_DESTBLEND:
            m_render_state.dst_blend = ConvertBlendFactor((D3DBLEND)value);
            break;
        case D3DRS_CULLMODE:
            m_render_state.cull_mode = ConvertCullMode((D3DCULL)value);
            break;
        // ... more states
    }
    m_state_dirty = true;
}
```

### Day 2: Blend Factor Conversion

```cpp
VkBlendFactor ConvertBlendFactor(D3DBLEND blend)
{
    switch (blend) {
        case D3DBLEND_ZERO: return VK_BLEND_FACTOR_ZERO;
        case D3DBLEND_ONE: return VK_BLEND_FACTOR_ONE;
        case D3DBLEND_SRCCOLOR: return VK_BLEND_FACTOR_SRC_COLOR;
        case D3DBLEND_INVSRCCOLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case D3DBLEND_SRCALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
        case D3DBLEND_INVSRCALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case D3DBLEND_DESTALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
        case D3DBLEND_INVDESTALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case D3DBLEND_DESTCOLOR: return VK_BLEND_FACTOR_DST_COLOR;
        case D3DBLEND_INVDESTCOLOR: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case D3DBLEND_SRCALPHASAT: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        default: return VK_BLEND_FACTOR_ONE;
    }
}
```

### Day 3: Pipeline Variants

Create pipelines for common state combinations:

```cpp
struct PipelineKey {
    uint32_t fvf;
    bool depth_test;
    bool depth_write;
    bool blend_enable;
    VkBlendFactor src_blend;
    VkBlendFactor dst_blend;
    VkCullModeFlags cull_mode;
    
    bool operator==(const PipelineKey& other) const;
};

std::unordered_map<PipelineKey, VkPipeline> m_pipeline_cache;

VkPipeline GetOrCreatePipeline(const PipelineKey& key);
```

### Day 4: Dynamic State Application

```cpp
void VulkanGraphicsDriver::ApplyDynamicState()
{
    VkCommandBuffer cmd = m_command_buffers[m_current_frame];
    
    // Vulkan 1.3+ dynamic state
    vkCmdSetDepthTestEnable(cmd, m_render_state.depth_test_enable);
    vkCmdSetDepthWriteEnable(cmd, m_render_state.depth_write_enable);
    vkCmdSetCullMode(cmd, m_render_state.cull_mode);
}
```

### Day 5: Alpha Test in Shader

Alpha test must be implemented in fragment shader:

```glsl
// In fragment shader
layout(push_constant) uniform PushConstants {
    mat4 mvp;
    float alphaRef;
    bool alphaTestEnable;
} pc;

void main() {
    vec4 color = texture(texSampler, fragTexCoord);
    
    // Alpha test
    if (pc.alphaTestEnable && color.a < pc.alphaRef) {
        discard;
    }
    
    outColor = color;
}
```

---

## Common Blend Modes

### Opaque

```cpp
blend_enable = false;
// No blending, output replaces framebuffer
```

### Alpha Blend (Standard)

```cpp
blend_enable = true;
src_blend = VK_BLEND_FACTOR_SRC_ALPHA;
dst_blend = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
// result = src.rgb * src.a + dst.rgb * (1 - src.a)
```

### Additive

```cpp
blend_enable = true;
src_blend = VK_BLEND_FACTOR_ONE;
dst_blend = VK_BLEND_FACTOR_ONE;
// result = src.rgb + dst.rgb
```

### Multiplicative

```cpp
blend_enable = true;
src_blend = VK_BLEND_FACTOR_DST_COLOR;
dst_blend = VK_BLEND_FACTOR_ZERO;
// result = src.rgb * dst.rgb
```

---

## Testing Strategy

### Test 1: Depth Buffer

Verify near objects occlude far objects.

### Test 2: Alpha Blend

Render semi-transparent UI elements.

### Test 3: Alpha Test

Render sprites with cutout alpha.

### Test 4: Additive Blend

Verify particle effects glow correctly.

### Test 5: Culling

Verify back-face culling works.

---

## Success Criteria

- [ ] `SetRenderState()` updates state cache
- [ ] Depth testing enables/disables correctly
- [ ] Depth writing enables/disables correctly
- [ ] Alpha blending works (standard, additive)
- [ ] Alpha test discards pixels correctly
- [ ] Cull mode affects visible faces
- [ ] Transparent UI elements visible
- [ ] Particle effects render correctly

---

## Files to Modify

1. `vulkan_graphics_driver.h` - State cache structure
2. `vulkan_graphics_driver.cpp` - SetRenderState implementation
3. `shaders/alpha_test.frag` - Alpha test shader

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Pipeline explosion | High | Cache common combinations |
| State leaks | Medium | Reset state per frame |
| Blend mode errors | Medium | Test each mode separately |
| Dynamic state support | Low | Fall back to pipeline variants |

---

## Dependencies

- **Phase 58**: Transforms working
- **Vulkan 1.3**: For dynamic state (optional)

---

**Created**: November 28, 2025
**Author**: GeneralsX Development Team
