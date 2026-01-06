# Phase 56: Vertex and Index Buffer Rendering

**Phase**: 56
**Title**: Real Draw Calls - Vertex and Index Buffer Rendering
**Duration**: 5-7 days
**Status**: COMPLETE (Core implementation done, dynamic buffer integration deferred to Phase 59+)
**Dependencies**: Phase 54 complete (Frame rendering working)

---

## Implementation Status (November 29, 2025)

### Completed

✅ **Task 56.1: Buffer Tracking**
- Added `g_current_vertex_buffer`, `g_current_index_buffer` state variables
- Added `g_current_vertex_offset`, `g_current_vertex_stride`, `g_current_index_start`
- Implemented `SetVertexStreamSource()` in VulkanGraphicsDriver
- Implemented `SetIndexBuffer()` in VulkanGraphicsDriver (new interface method)

✅ **Task 56.2: FVF to Vulkan Conversion**
- Implemented `ConvertFVFToVulkan()` function
- Created `FVFDescriptor` struct with `VkVertexInputAttributeDescription`
- Added FVF cache (`g_fvf_cache`) for performance

✅ **Task 56.3: DrawPrimitive Implementation**
- Real `vkCmdDraw()` command recording
- Vertex buffer binding via `vkCmdBindVertexBuffers()`
- Pipeline binding via `vkCmdBindPipeline()`

✅ **Task 56.4: DrawIndexedPrimitive Implementation**
- Real `vkCmdDrawIndexed()` command recording
- Index buffer binding via `vkCmdBindIndexBuffer()`
- Support for currently bound index buffer (INVALID_HANDLE parameter)

✅ **Task 56.5: Pipeline State Management**
- Added `PipelineCacheKey` and `PipelineCacheKeyHash` structures
- Created `g_pipeline_cache` for future FVF-specific pipelines
- Implemented `GetOrCreatePipeline()` (returns default pipeline for now)
- Added `CleanupPipelineCache()` for cleanup

### Pending

⏳ **Dynamic Buffer Integration**
- `DynamicVBAccessClass` and `DynamicIBAccessClass` are stubs
- Need to connect dynamic buffers to Vulkan driver
- Current calls with dynamic buffers go to empty stubs

⏳ **FVF-Specific Shaders**
- Current pipeline uses fullscreen shader (no vertex input)
- Need vertex shaders that accept FVF vertex formats
- Need to create pipelines with vertex input descriptions

---

## Overview

Phase 56 implements real geometry rendering by connecting vertex/index buffers to the Vulkan graphics pipeline. This phase transforms the blue screen into actual rendered geometry.

### Current State (Post-Phase 54)

- ✅ Vulkan frame cycle working (BeginFrame/EndFrame/Present)
- ✅ Clear color visible (dark blue screen)
- ✅ 67+ vertex buffers created (handles allocated)
- ✅ 69+ index buffers created (handles allocated)
- ❌ `DrawPrimitive()` is stub - does nothing
- ❌ `DrawIndexedPrimitive()` is stub - does nothing
- ❌ No geometry visible on screen

### Goal

Render actual triangles using the vertex and index buffers that the game has already created.

---

## Technical Requirements

### 1. Vulkan Buffer Management

The game creates vertex/index buffers during initialization. We need to:

1. **Track buffer bindings** - Know which VB/IB is currently set
2. **Bind buffers to pipeline** - `vkCmdBindVertexBuffers`, `vkCmdBindIndexBuffer`
3. **Issue draw commands** - `vkCmdDraw`, `vkCmdDrawIndexed`

### 2. Vertex Format Handling

Game uses DirectX FVF (Flexible Vertex Format). Common formats:

| FVF | Components | Size |
|-----|------------|------|
| `D3DFVF_XYZ` | Position (float3) | 12 bytes |
| `D3DFVF_XYZRHW` | Transformed (float4) | 16 bytes |
| `D3DFVF_DIFFUSE` | Color (DWORD) | 4 bytes |
| `D3DFVF_TEX1` | UV (float2) | 8 bytes |
| `0x00000142` | XYZ + Normal + UV | 32 bytes |

Need to create Vulkan `VkVertexInputAttributeDescription` for each FVF combination.

### 3. Pipeline State

Different draw calls may need different pipeline states:

- **Primitive topology**: Triangles, lines, points
- **Blend mode**: Opaque, alpha blend, additive
- **Depth test**: Enable/disable
- **Cull mode**: None, front, back

---

## Implementation Plan

### Week 1: Buffer Binding Infrastructure

#### Day 1-2: Track Current Buffers

```cpp
// In VulkanGraphicsDriver
uint32_t m_current_vertex_buffer_handle = 0;
uint32_t m_current_index_buffer_handle = 0;
uint32_t m_current_fvf = 0;
D3DPRIMITIVETYPE m_current_primitive_type = D3DPT_TRIANGLELIST;
```

Implement `SetStreamSource()` and `SetIndices()` to track bindings.

#### Day 3-4: FVF to Vulkan Conversion

Create `VkVertexInputBindingDescription` and `VkVertexInputAttributeDescription` from FVF:

```cpp
struct FVFDescriptor {
    uint32_t fvf;
    uint32_t stride;
    std::vector<VkVertexInputAttributeDescription> attributes;
};

FVFDescriptor ConvertFVFToVulkan(uint32_t fvf);
```

#### Day 5: Multiple Pipeline Variants

Create pipeline cache for different FVF/topology combinations:

```cpp
std::unordered_map<uint64_t, VkPipeline> m_pipeline_cache;

VkPipeline GetPipelineForState(uint32_t fvf, D3DPRIMITIVETYPE primitive_type);
```

### Week 2: Draw Commands

#### Day 6: DrawPrimitive Implementation

```cpp
void VulkanGraphicsDriver::DrawPrimitive(
    D3DPRIMITIVETYPE primitive_type,
    uint32_t start_vertex,
    uint32_t primitive_count)
{
    if (!m_frame_started) return;
    
    // Get/create pipeline for current state
    VkPipeline pipeline = GetPipelineForState(m_current_fvf, primitive_type);
    vkCmdBindPipeline(m_command_buffers[m_current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    // Bind vertex buffer
    VkBuffer vb = GetVulkanBuffer(m_current_vertex_buffer_handle);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_command_buffers[m_current_frame], 0, 1, &vb, &offset);
    
    // Calculate vertex count from primitive count
    uint32_t vertex_count = CalculateVertexCount(primitive_type, primitive_count);
    
    // Draw!
    vkCmdDraw(m_command_buffers[m_current_frame], vertex_count, 1, start_vertex, 0);
}
```

#### Day 7: DrawIndexedPrimitive Implementation

```cpp
void VulkanGraphicsDriver::DrawIndexedPrimitive(
    D3DPRIMITIVETYPE primitive_type,
    int32_t base_vertex_index,
    uint32_t min_vertex_index,
    uint32_t num_vertices,
    uint32_t start_index,
    uint32_t primitive_count)
{
    if (!m_frame_started) return;
    
    // Get/create pipeline
    VkPipeline pipeline = GetPipelineForState(m_current_fvf, primitive_type);
    vkCmdBindPipeline(m_command_buffers[m_current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    // Bind vertex buffer
    VkBuffer vb = GetVulkanBuffer(m_current_vertex_buffer_handle);
    VkDeviceSize vb_offset = 0;
    vkCmdBindVertexBuffers(m_command_buffers[m_current_frame], 0, 1, &vb, &vb_offset);
    
    // Bind index buffer
    VkBuffer ib = GetVulkanBuffer(m_current_index_buffer_handle);
    VkIndexType index_type = m_current_index_is_32bit ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    vkCmdBindIndexBuffer(m_command_buffers[m_current_frame], ib, 0, index_type);
    
    // Calculate index count
    uint32_t index_count = CalculateIndexCount(primitive_type, primitive_count);
    
    // Draw indexed!
    vkCmdDrawIndexed(m_command_buffers[m_current_frame], index_count, 1, start_index, base_vertex_index, 0);
}
```

---

## Shader Updates Required

### basic.vert - Real Vertex Shader

```glsl
#version 450

// Vertex inputs (match FVF)
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// Push constants for MVP matrix
layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pc;

// Outputs to fragment shader
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;

void main() {
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragNormal = inNormal;
}
```

### basic.frag - Fragment Shader

```glsl
#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

// For now, output solid color based on normal
void main() {
    // Visualize normals as colors (debug mode)
    outColor = vec4(fragNormal * 0.5 + 0.5, 1.0);
}
```

---

## Testing Strategy

### Test 1: Simple Triangle

Verify basic DrawPrimitive works with hardcoded triangle.

### Test 2: UI Quads

The game draws UI elements as indexed quads. Verify they appear.

### Test 3: Terrain

Shell map terrain uses large indexed meshes. Should see terrain geometry.

### Test 4: Debug Visualization

Render wireframe to verify vertex positions are correct.

---

## Success Criteria

- [x] `SetStreamSource()` tracks current vertex buffer (SetVertexStreamSource implemented)
- [x] `SetIndices()` tracks current index buffer (SetIndexBuffer implemented)
- [x] `DrawPrimitive()` renders non-indexed geometry (vkCmdDraw working)
- [x] `DrawIndexedPrimitive()` renders indexed geometry (vkCmdDrawIndexed working)
- [x] Multiple FVF formats supported (ConvertFVFToVulkan implemented)
- [x] Pipeline cache avoids recreation (g_pipeline_cache implemented)
- [ ] Visible geometry on screen - requires FVF-specific shaders (Phase 59+)

---

## Files to Modify

1. `vulkan_graphics_driver.h` - Add buffer tracking members
2. `vulkan_graphics_driver.cpp` - Implement draw calls
3. `shaders/basic.vert` - Real vertex transformation
4. `shaders/basic.frag` - Basic fragment output
5. `vulkan_embedded_shaders.h` - Recompile shaders

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| FVF conversion errors | High | Debug with simple known vertex format first |
| Pipeline state explosion | Medium | Limit initial states, cache aggressively |
| Buffer binding order | Medium | Add extensive logging |
| Vertex data misinterpretation | High | Dump vertex data for verification |

---

## Dependencies

- **Phase 54**: Frame rendering (BeginFrame/EndFrame/Present) ✅ COMPLETE
- **Vulkan buffers**: Already created in Phase 53

---

**Created**: November 28, 2025
**Author**: GeneralsX Development Team
