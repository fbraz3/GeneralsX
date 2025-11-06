# Phase 49.2: Graphics Pipeline Creation

## Overview

Creates the graphics pipeline that defines HOW to render: shader compilation, render state, and GPU pipeline state machine configuration.

**Status**: Ready to Implement  
**Depends on**: Phase 49.1 (Swapchain complete)  
**Complexity**: High (GPU pipeline programming, shader compilation)  
**Duration**: 2 days

## Objectives

1. Compile GLSL shaders to SPIR-V bytecode
2. Create VkRenderPass (defines rendering operations and attachments)
3. Create VkPipelineLayout (descriptor sets and push constants)
4. Create VkGraphicsPipeline (rasterization state, blending, viewport)
5. Create VkFramebuffer for each swapchain image
6. Validate all Vulkan objects created successfully

## Acceptance Criteria

- ✅ GLSL shaders compile to SPIR-V without errors
- ✅ vkCreateRenderPass succeeds without validation errors
- ✅ vkCreateGraphicsPipelines succeeds without errors
- ✅ Framebuffers created for all swapchain images
- ✅ No Vulkan validation layer errors
- ✅ Shader reloading works correctly
- ✅ Pipeline objects properly destroyed on cleanup

## Key Vulkan Concepts

**VkShaderModule**: Compiled shader code (SPIR-V). Created from binary shader bytecode.

**VkRenderPass**: Describes rendering operations, attachments (color, depth), and subpasses. Defines what happens during rendering.

**VkPipelineLayout**: Describes pipeline resources (descriptor sets for textures/buffers, push constants for small data).

**VkGraphicsPipeline**: GPU state machine combining: shaders, rasterization state, blending, viewport, scissor, depth test, etc.

**VkFramebuffer**: Binding of image views to render pass attachments. One per swapchain image.

## Implementation Steps

### Step 1: Create Shader Files

File: `resources/shaders/basic.vert`

```glsl
#version 450

// Vertex input (quad vertices)
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

// Output to fragment shader
layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    fragColor = inColor;
}
```

File: `resources/shaders/basic.frag`

```glsl
#version 450

// Input from vertex shader
layout(location = 0) in vec3 fragColor;

// Output (render target)
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

### Step 2: Compile Shaders to SPIR-V

File: `resources/shaders/Makefile`

```makefile
GLSLC := glslc
SHADERS = basic.vert basic.frag
SPIRV = $(SHADERS:=.spv)

all: $(SPIRV)

%.vert.spv: %.vert
	$(GLSLC) -fshader-stage=vertex $< -o $@

%.frag.spv: %.frag
	$(GLSLC) -fshader-stage=fragment $< -o $@

clean:
	rm -f *.spv

.PHONY: all clean
```

Build script:

```bash
cd resources/shaders
make clean
make
ls -la *.spv  # Should see basic.vert.spv and basic.frag.spv
```

### Step 3: Create Shader Modules

File: `graphics_backend_dxvk_pipeline.cpp`

```cpp
// Load SPIR-V bytecode
std::vector<uint32_t> LoadShaderBytecode(const std::string& filename)
{
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        printf("[DXVK] ERROR: Failed to open shader file: %s\n", filename.c_str());
        return {};
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::vector<uint32_t> bytecode(size / 4);
    if (fread(bytecode.data(), 1, size, file) != size) {
        printf("[DXVK] ERROR: Failed to read shader file: %s\n", filename.c_str());
        fclose(file);
        return {};
    }

    fclose(file);
    printf("[DXVK] Loaded shader: %s (%zu bytes)\n", filename.c_str(), size);
    
    return bytecode;
}

// Create shader module
VkShaderModule DXVKGraphicsBackend::CreateShaderModule(
    const std::string& filename)
{
    std::vector<uint32_t> bytecode = LoadShaderBytecode(filename);
    if (bytecode.empty()) {
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo module_info = {};
    module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.codeSize = bytecode.size() * 4;
    module_info.pCode = bytecode.data();

    VkShaderModule module = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(m_device, &module_info, nullptr, &module);
    ASSERT(result == VK_SUCCESS, "Failed to create shader module");

    printf("[DXVK] Shader module created: %p\n", (void*)module);
    
    return module;
}
```

### Step 4: Create Render Pass

File: `graphics_backend_dxvk_pipeline.cpp`

```cpp
VkResult DXVKGraphicsBackend::CreateRenderPass()
{
    // Define color attachment
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = m_swapchain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Define attachment reference for subpass
    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Define subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    // Create render pass
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass);
    ASSERT(result == VK_SUCCESS, "Failed to create render pass");

    printf("[DXVK] Render pass created: %p\n", (void*)m_render_pass);
    
    return result;
}
```

### Step 5: Create Pipeline Layout

File: `graphics_backend_dxvk_pipeline.cpp`

```cpp
VkResult DXVKGraphicsBackend::CreatePipelineLayout()
{
    // For simple quad: no descriptors, no push constants
    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 0;
    layout_info.pushConstantRangeCount = 0;

    VkResult result = vkCreatePipelineLayout(m_device, &layout_info, nullptr, &m_pipeline_layout);
    ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout");

    printf("[DXVK] Pipeline layout created: %p\n", (void*)m_pipeline_layout);
    
    return result;
}
```

### Step 6: Create Graphics Pipeline

File: `graphics_backend_dxvk_pipeline.cpp`

```cpp
VkResult DXVKGraphicsBackend::CreateGraphicsPipeline()
{
    // Load shaders
    VkShaderModule vert_module = CreateShaderModule("resources/shaders/basic.vert.spv");
    VkShaderModule frag_module = CreateShaderModule("resources/shaders/basic.frag.spv");
    
    ASSERT(vert_module != VK_NULL_HANDLE, "Failed to load vertex shader");
    ASSERT(frag_module != VK_NULL_HANDLE, "Failed to load fragment shader");

    // Shader stages
    VkPipelineShaderStageCreateInfo shader_stages[2] = {};
    
    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vert_module;
    shader_stages[0].pName = "main";

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag_module;
    shader_stages[1].pName = "main";

    // Vertex input (positions and colors)
    VkVertexInputBindingDescription binding_desc = {};
    binding_desc.binding = 0;
    binding_desc.stride = sizeof(float) * 6;  // 3 pos + 3 color
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attr_descs[2] = {};
    attr_descs[0].binding = 0;
    attr_descs[0].location = 0;
    attr_descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;  // Position
    attr_descs[0].offset = 0;

    attr_descs[1].binding = 0;
    attr_descs[1].location = 1;
    attr_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;  // Color
    attr_descs[1].offset = sizeof(float) * 3;

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = 2;
    vertex_input_info.pVertexAttributeDescriptions = attr_descs;

    // Input assembly (triangles)
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapchain_extent.width;
    viewport.height = (float)m_swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain_extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    // Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling (disabled)
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Blending (disabled, opaque)
    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    // Create pipeline
    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = m_pipeline_layout;
    pipeline_info.renderPass = m_render_pass;
    pipeline_info.subpass = 0;

    VkResult result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline);
    ASSERT(result == VK_SUCCESS, "Failed to create graphics pipeline");

    printf("[DXVK] Graphics pipeline created: %p\n", (void*)m_pipeline);

    // Cleanup shader modules (no longer needed after pipeline creation)
    vkDestroyShaderModule(m_device, vert_module, nullptr);
    vkDestroyShaderModule(m_device, frag_module, nullptr);

    return result;
}
```

### Step 7: Create Framebuffers

File: `graphics_backend_dxvk_pipeline.cpp`

```cpp
VkResult DXVKGraphicsBackend::CreateFramebuffers()
{
    m_framebuffers.resize(m_swapchain_image_views.size());

    for (size_t i = 0; i < m_swapchain_image_views.size(); i++) {
        VkImageView attachments[] = {
            m_swapchain_image_views[i]
        };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = m_swapchain_extent.width;
        framebuffer_info.height = m_swapchain_extent.height;
        framebuffer_info.layers = 1;

        VkResult result = vkCreateFramebuffer(m_device, &framebuffer_info, nullptr, &m_framebuffers[i]);
        ASSERT(result == VK_SUCCESS, "Failed to create framebuffer");

        printf("[DXVK] Framebuffer %zu created: %p\n", i, (void*)m_framebuffers[i]);
    }

    return VK_SUCCESS;
}
```

## Class Members to Add

File: `graphics_backend_dxvk.h`

```cpp
private:
    // Pipeline resources
    VkRenderPass m_render_pass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;

public:
    VkRenderPass GetRenderPass() const { return m_render_pass; }
    VkPipeline GetPipeline() const { return m_pipeline; }
    VkFramebuffer GetFramebuffer(uint32_t index) const { 
        return index < m_framebuffers.size() ? m_framebuffers[index] : VK_NULL_HANDLE;
    }

private:
    VkResult CreateRenderPass();
    VkResult CreatePipelineLayout();
    VkResult CreateGraphicsPipeline();
    VkResult CreateFramebuffers();
    VkShaderModule CreateShaderModule(const std::string& filename);
```

## Cleanup on Destruction

```cpp
// In destructor
for (auto& framebuffer : m_framebuffers) {
    if (framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
}

if (m_pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
}

if (m_pipeline_layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
}

if (m_render_pass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(m_device, m_render_pass, nullptr);
}
```

## Success Checklist

- [ ] GLSL shaders compile to SPIR-V without errors
- [ ] Render pass created successfully
- [ ] Pipeline layout created successfully
- [ ] Graphics pipeline created without validation errors
- [ ] Framebuffers created for all swapchain images
- [ ] Shader modules properly loaded from disk
- [ ] All objects properly destroyed in cleanup
- [ ] No Vulkan validation layer errors

## Next Steps

- ✅ Understand graphics pipeline architecture
- ✅ Implement shader compilation and modules
- ✅ Implement render pass and pipeline creation
- ⏳ Move to Task 3: First Quad Rendering

---

**Created**: November 6, 2025  
**Status**: Ready to Implement
