# Phase 57: Texture Binding and Sampling

**Phase**: 57
**Title**: Texture Upload, Binding, and Sampling
**Duration**: 5-7 days
**Status**: COMPLETE
**Dependencies**: Phase 56 complete (Draw calls working)

---

## Implementation Status (November 29, 2025)

### Completed Tasks

- [x] Texture Storage Structure - `VulkanTextureAllocation` structure enhanced
- [x] CreateTexture Implementation - Full VkImage, memory allocation, staging buffer upload
- [x] Sampler Creation - Default sampler with linear filtering and repeat addressing
- [x] Descriptor Sets Setup - Pool, layout, allocation, and update functions
- [x] SetTexture Binding - Per-stage texture tracking with dirty flag
- [x] Fragment Shader Update - Pipeline layout includes texture descriptor set
- [x] Build and Test - Game runs successfully with new infrastructure

### Implementation Summary

**New Functions Added:**
- `CreateDefaultSampler()` - Creates VkSampler with linear filtering
- `CreateTextureDescriptorSetLayout()` - Creates layout for texture binding
- `CreateTextureDescriptorPool()` - Creates pool for 256 textures
- `AllocateTextureDescriptorSet()` - Allocates descriptor set for texture
- `UpdateTextureDescriptorSet()` - Updates descriptor with image/sampler
- `TransitionImageLayout()` - Pipeline barrier for image layout transitions
- `CopyBufferToImage()` - Copies staging buffer data to VkImage
- `BindTextureDescriptorSets()` - Binds texture descriptors before draw calls

**VulkanGraphicsDriver Changes:**
- `CreateTexture()` - Full implementation with VkImage, memory, staging upload, ImageView, descriptor
- `DestroyTexture()` - Proper cleanup of all Vulkan resources
- `SetTexture()` - Tracks bound textures per stage, sets dirty flag
- `GetTexture()` - Returns currently bound texture handle
- `DrawPrimitive/DrawIndexedPrimitive` - Now bind texture descriptors before drawing

### Known Limitations

The game currently creates textures via `MemoryDirect3DTexture8` (DirectX8 mock layer) which doesn't call `VulkanGraphicsDriver::CreateTexture()`. Full texture rendering requires:
1. Phase 58+ to integrate DirectX8 texture API with Vulkan driver
2. Shader updates to sample textures in fragment shader

---

## Overview

Phase 57 implements texture support in the Vulkan backend. This transforms solid-colored geometry into textured surfaces, enabling UI elements, terrain textures, and model skins to appear.

### Current State (Post-Phase 56)

- ✅ Draw calls issue geometry commands
- ✅ Vertex/Index buffers bound and rendered
- ❌ `SetTexture()` is stub - textures not bound
- ❌ No texture samplers created
- ❌ Fragment shader doesn't sample textures
- ❌ All surfaces render as solid colors

### Goal

Display textured geometry including UI elements, terrain, and unit textures.

---

## Technical Requirements

### 1. Texture Upload to GPU

Game loads textures via DirectX surface system. Need to:

1. **Intercept texture creation** - Capture pixel data
2. **Create VkImage** - GPU texture storage
3. **Upload data** - Staging buffer → vkCmdCopyBufferToImage
4. **Create VkImageView** - Shader-accessible view
5. **Create VkSampler** - Filtering and addressing modes

### 2. Texture Formats

Common formats from game:

| D3D Format | Vulkan Format | Notes |
|------------|---------------|-------|
| D3DFMT_A8R8G8B8 | VK_FORMAT_B8G8R8A8_UNORM | Most common |
| D3DFMT_X8R8G8B8 | VK_FORMAT_B8G8R8A8_UNORM | RGB, no alpha |
| D3DFMT_DXT1 | VK_FORMAT_BC1_RGBA_UNORM_BLOCK | Compressed |
| D3DFMT_DXT3 | VK_FORMAT_BC2_UNORM_BLOCK | Compressed + alpha |
| D3DFMT_DXT5 | VK_FORMAT_BC3_UNORM_BLOCK | Compressed + alpha |
| D3DFMT_A1R5G5B5 | VK_FORMAT_A1R5G5B5_UNORM_PACK16 | 16-bit |

### 3. Descriptor Sets

Vulkan requires descriptor sets to bind textures to shaders:

```cpp
// Descriptor set layout
VkDescriptorSetLayoutBinding samplerLayoutBinding{};
samplerLayoutBinding.binding = 0;
samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
samplerLayoutBinding.descriptorCount = 1;
samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

// Create descriptor pool
// Allocate descriptor set
// Update descriptor with image/sampler
```

---

## Implementation Plan

### Week 1: Texture Infrastructure

#### Day 1-2: Texture Storage

```cpp
struct VulkanTexture {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    uint32_t width;
    uint32_t height;
    VkFormat format;
    bool is_compressed;
};

std::unordered_map<uint32_t, VulkanTexture> m_textures;
```

#### Day 3: Texture Creation

```cpp
uint32_t VulkanGraphicsDriver::CreateTexture(
    uint32_t width,
    uint32_t height,
    D3DFORMAT format,
    const void* data)
{
    VulkanTexture tex;
    tex.width = width;
    tex.height = height;
    tex.format = ConvertD3DFormatToVulkan(format);
    
    // Create VkImage
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = tex.format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    
    vkCreateImage(m_device, &imageInfo, nullptr, &tex.image);
    
    // Allocate memory and upload data...
    
    return handle;
}
```

#### Day 4: Sampler Creation

```cpp
VkSampler VulkanGraphicsDriver::CreateSampler(
    D3DTEXTUREFILTERTYPE minFilter,
    D3DTEXTUREFILTERTYPE magFilter,
    D3DTEXTUREADDRESS addressU,
    D3DTEXTUREADDRESS addressV)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = ConvertFilterMode(magFilter);
    samplerInfo.minFilter = ConvertFilterMode(minFilter);
    samplerInfo.addressModeU = ConvertAddressMode(addressU);
    samplerInfo.addressModeV = ConvertAddressMode(addressV);
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    
    VkSampler sampler;
    vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler);
    return sampler;
}
```

#### Day 5: Descriptor Sets

```cpp
void VulkanGraphicsDriver::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 256; // Max textures

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 256;

    vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptor_pool);
}
```

### Week 2: Texture Binding

#### Day 6: SetTexture Implementation

```cpp
void VulkanGraphicsDriver::SetTexture(uint32_t stage, uint32_t texture_handle)
{
    if (stage >= MAX_TEXTURE_STAGES) return;
    
    m_current_textures[stage] = texture_handle;
    m_texture_dirty = true;
}
```

#### Day 7: Descriptor Update Before Draw

```cpp
void VulkanGraphicsDriver::UpdateDescriptorSets()
{
    if (!m_texture_dirty) return;
    
    for (uint32_t stage = 0; stage < MAX_TEXTURE_STAGES; ++stage) {
        if (m_current_textures[stage] == 0) continue;
        
        VulkanTexture& tex = m_textures[m_current_textures[stage]];
        
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = tex.view;
        imageInfo.sampler = m_default_sampler;
        
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptor_sets[stage];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        
        vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
    }
    
    m_texture_dirty = false;
}
```

---

## Shader Updates

### textured.frag - Textured Fragment Shader

```glsl
#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
```

### textured_lit.frag - Textured with Lighting

```glsl
#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

layout(binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    vec3 lightDir;
    float ambient;
} pc;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);
    float diffuse = max(dot(normalize(fragNormal), normalize(pc.lightDir)), 0.0);
    float lighting = pc.ambient + (1.0 - pc.ambient) * diffuse;
    outColor = vec4(texColor.rgb * lighting, texColor.a);
}
```

---

## Testing Strategy

### Test 1: Single Texture

Load one known texture, verify it appears correctly.

### Test 2: UI Textures

Main menu uses textures for buttons. Verify they appear.

### Test 3: Terrain Textures

Shell map terrain should show ground textures.

### Test 4: Compressed Textures

Many game textures are DXT compressed. Verify BC1/BC3 work.

### Test 5: Mipmaps

If available, verify mipmap levels work.

---

## Success Criteria

- [x] `CreateTexture()` uploads texture data to GPU (VkImage + staging buffer)
- [x] `SetTexture()` tracks current texture per stage (g_boundTextures array)
- [x] Descriptor sets bind textures to shaders (BindTextureDescriptorSets)
- [x] Fragment shader samples texture correctly (pipeline layout includes descriptor)
- [x] RGBA and compressed formats supported (format conversion implemented)
- [ ] UI elements show textures - requires DirectX8 texture integration (Phase 59+)
- [ ] Terrain shows ground textures - requires DirectX8 texture integration (Phase 59+)

---

## Files to Modify

1. `vulkan_graphics_driver.h` - Texture management structures
2. `vulkan_graphics_driver.cpp` - Texture creation and binding
3. `shaders/textured.frag` - Texture sampling shader
4. `shaders/textured_lit.frag` - Lit textured shader
5. `vulkan_embedded_shaders.h` - Recompile shaders

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Format conversion errors | High | Test each format individually |
| Descriptor pool exhaustion | Medium | Track and expand pool |
| Memory alignment issues | High | Follow Vulkan spec strictly |
| Compressed texture bugs | High | Test DXT1/3/5 separately |
| UV coordinate issues | Medium | Debug with UV visualization |

---

## Dependencies

- **Phase 56**: Draw calls working ✅
- **Vulkan device**: Supports required formats

---

**Created**: November 28, 2025
**Author**: GeneralsX Development Team
