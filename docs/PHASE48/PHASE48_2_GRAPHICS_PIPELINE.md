# Phase 48.2: Graphics Pipeline Creation

## Overview

Creates the Vulkan graphics pipeline that defines how the GPU will render geometry. Includes shader compilation, render pass creation, and pipeline layout setup.

**Status**: Ready to implement after Task 1  
**Depends on**: Phase 48.1 (Swapchain created)  
**Complexity**: High  
**Duration**: 2 days

## Objectives

1. Create shader modules (vertex and fragment shaders compiled to SPIR-V)
2. Create render pass describing attachments and rendering operations
3. Create pipeline layout with descriptors and push constants
4. Create graphics pipeline with rasterization, blending, and viewport state
5. Create framebuffers from swapchain image views

## Acceptance Criteria

- Shaders compile from GLSL to SPIR-V without errors
- vkCreateRenderPass succeeds
- vkCreateGraphicsPipelines succeeds
- No Vulkan validation layer errors during pipeline creation
- Pipeline can be bound and used in command buffers

## Key Vulkan Concepts

**Shaders**: Programs running on GPU to compute colors

**SPIR-V**: Intermediate representation of shaders (portable, cross-platform)

**Render Pass**: Specifies what attachments are used and how rendering operations interact with them

**Pipeline**: Immutable GPU state machine for rendering (once created, cannot be modified)

**Framebuffer**: Collection of images to render into (color, depth, etc.)

## Implementation Steps

### Step 1: Compile Shaders to SPIR-V

First, create shader source files:

**resources/shaders/basic.vert** (Vertex Shader):

```glsl
#version 450

layout(location = 0) out vec3 fragColor;

// Hardcoded quad vertices for now
const vec2 positions[6] = vec2[](
    vec2(-0.5, -0.5),  // Triangle 1
    vec2(0.5, -0.5),
    vec2(0.5, 0.5),
    
    vec2(-0.5, -0.5),  // Triangle 2
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

const vec3 colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0),  // Red
    vec3(0.0, 1.0, 0.0),  // Green
    vec3(0.0, 0.0, 1.0),  // Blue
    
    vec3(1.0, 0.0, 0.0),  // Red
    vec3(0.0, 0.0, 1.0),  // Blue
    vec3(1.0, 1.0, 0.0)   // Yellow
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
```

**resources/shaders/basic.frag** (Fragment Shader):

```glsl
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

### Step 2: Compile Shaders with glslc

In CMakeLists.txt, add shader compilation:

```cmake
# In Core/GameEngineDevice/CMakeLists.txt

# Find glslc compiler (from Vulkan SDK)
find_program(GLSLC glslc HINTS ${Vulkan_GLSLC_EXECUTABLE})

if(GLSLC)
    message(STATUS "Found GLSL compiler: ${GLSLC}")
    
    set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/resources/shaders")
    set(SHADER_OUT_DIR "${CMAKE_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY "${SHADER_OUT_DIR}")
    
    set(SHADER_SOURCES
        ${SHADER_SRC_DIR}/basic.vert
        ${SHADER_SRC_DIR}/basic.frag
    )
    
    foreach(SHADER ${SHADER_SOURCES})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        set(SHADER_OUTPUT "${SHADER_OUT_DIR}/${SHADER_NAME}.spv")
        
        add_custom_command(
            OUTPUT ${SHADER_OUTPUT}
            COMMAND ${GLSLC} -o ${SHADER_OUTPUT} ${SHADER}
            DEPENDS ${SHADER}
        )
        list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
    endforeach()
    
    add_custom_target(shaders ALL DEPENDS ${SHADER_OUTPUTS})
endif()
```

### Step 3: Load Compiled Shaders

```cpp
// Helper function to load SPIR-V shader bytecode
std::vector<uint32_t> LoadShader(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    
    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));
    
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), file_size);
    file.close();
    
    return buffer;
}

// In graphics_backend_dxvk.cpp
std::vector<uint32_t> vert_code = LoadShader("shaders/basic.vert.spv");
std::vector<uint32_t> frag_code = LoadShader("shaders/basic.frag.spv");
```

### Step 4: Create Shader Modules

```cpp
VkShaderModuleCreateInfo vert_module_info = {};
vert_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
vert_module_info.codeSize = vert_code.size() * sizeof(uint32_t);
vert_module_info.pCode = vert_code.data();

VkShaderModule vert_module;
vkCreateShaderModule(device, &vert_module_info, nullptr, &vert_module);

VkShaderModuleCreateInfo frag_module_info = {};
frag_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
frag_module_info.codeSize = frag_code.size() * sizeof(uint32_t);
frag_module_info.pCode = frag_code.data();

VkShaderModule frag_module;
vkCreateShaderModule(device, &frag_module_info, nullptr, &frag_module);
```

### Step 5: Create Render Pass

```cpp
VkAttachmentDescription color_attachment = {};
color_attachment.format = swapchain_format.format;  // From Task 1
color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

VkAttachmentReference color_attachment_ref = {};
color_attachment_ref.attachment = 0;
color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

VkSubpassDescription subpass = {};
subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
subpass.colorAttachmentCount = 1;
subpass.pColorAttachments = &color_attachment_ref;

VkRenderPassCreateInfo render_pass_info = {};
render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
render_pass_info.attachmentCount = 1;
render_pass_info.pAttachments = &color_attachment;
render_pass_info.subpassCount = 1;
render_pass_info.pSubpasses = &subpass;

VkRenderPass render_pass;
vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass);
```

### Step 6: Create Pipeline Layout

```cpp
VkPipelineLayoutCreateInfo pipeline_layout_info = {};
pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
pipeline_layout_info.setLayoutCount = 0;  // No descriptor sets for now
pipeline_layout_info.pushConstantRangeCount = 0;

VkPipelineLayout pipeline_layout;
vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout);
```

### Step 7: Create Graphics Pipeline

```cpp
VkPipelineShaderStageCreateInfo vert_stage_info = {};
vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
vert_stage_info.module = vert_module;
vert_stage_info.pName = "main";

VkPipelineShaderStageCreateInfo frag_stage_info = {};
frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
frag_stage_info.module = frag_module;
frag_stage_info.pName = "main";

VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
vertex_input_info.vertexBindingDescriptionCount = 0;
vertex_input_info.vertexAttributeDescriptionCount = 0;

VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
input_assembly.primitiveRestartEnable = VK_FALSE;

VkViewport viewport = {};
viewport.x = 0.0f;
viewport.y = 0.0f;
viewport.width = (float)swapchain_extent.width;
viewport.height = (float)swapchain_extent.height;
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;

VkRect2D scissor = {};
scissor.offset = {0, 0};
scissor.extent = swapchain_extent;

VkPipelineViewportStateCreateInfo viewport_state = {};
viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
viewport_state.viewportCount = 1;
viewport_state.pViewports = &viewport;
viewport_state.scissorCount = 1;
viewport_state.pScissors = &scissor;

VkPipelineRasterizationStateCreateInfo rasterizer = {};
rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
rasterizer.depthClampEnable = VK_FALSE;
rasterizer.rasterizerDiscardEnable = VK_FALSE;
rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
rasterizer.lineWidth = 1.0f;
rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
rasterizer.depthBiasEnable = VK_FALSE;

VkPipelineMultisampleStateCreateInfo multisampling = {};
multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
multisampling.sampleShadingEnable = VK_FALSE;
multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

VkPipelineColorBlendAttachmentState color_blend_attachment = {};
color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
color_blend_attachment.blendEnable = VK_FALSE;

VkPipelineColorBlendStateCreateInfo color_blending = {};
color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
color_blending.logicOpEnable = VK_FALSE;
color_blending.attachmentCount = 1;
color_blending.pAttachments = &color_blend_attachment;

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
pipeline_info.layout = pipeline_layout;
pipeline_info.renderPass = render_pass;
pipeline_info.subpass = 0;

VkPipeline graphics_pipeline;
vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline);
```

### Step 8: Create Framebuffers

```cpp
// After swapchain image views are created (Task 1)
for (uint32_t i = 0; i < swapchain_image_count; i++) {
    VkImageView attachments[] = {swapchain_image_views[i]};
    
    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = swapchain_extent.width;
    framebuffer_info.height = swapchain_extent.height;
    framebuffer_info.layers = 1;
    
    VkFramebuffer framebuffer;
    vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffer);
    swapchain_framebuffers.push_back(framebuffer);
}
```

## Code Location

**Primary file**: `Core/GameEngineDevice/Source/DXVKGraphicsBackend/graphics_backend_dxvk_pipeline.cpp`

**Shader files**: `resources/shaders/basic.vert` and `resources/shaders/basic.frag`

**CMake**: Add shader compilation to `Core/GameEngineDevice/CMakeLists.txt`

## Error Handling

```cpp
// Check shader compilation success
if (vert_code.empty() || frag_code.empty()) {
    // Shaders failed to load or compile
    printf("[ERROR] Shader compilation failed\n");
    return false;
}

// Check VkResult for all Vulkan calls
VkResult result = vkCreateGraphicsPipelines(...);
if (result != VK_SUCCESS) {
    printf("[ERROR] vkCreateGraphicsPipelines failed: %d\n", result);
    return false;
}
```

## Testing Approach

### Test 1: Shader Compilation

```bash
# After building with shader compilation enabled
ls -la build/macos-arm64/shaders/
# Expected: basic.vert.spv, basic.frag.spv files exist

# Verify shader validity
file build/macos-arm64/shaders/basic.vert.spv
# Expected: ELF 32-bit LSB relocatable or similar binary output
```

### Test 2: Pipeline Creation

```bash
# After running GeneralsXZH
grep "vkCreateRenderPass\|vkCreateGraphicsPipelines" /tmp/pipeline_test.log
# Expected: Success messages with object handles (0x...)
```

## Storage Considerations

**Member variables in DXVKGraphicsBackend class**:

```cpp
std::vector<VkShaderModule> shader_modules;
VkRenderPass render_pass = VK_NULL_HANDLE;
VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
VkPipeline graphics_pipeline = VK_NULL_HANDLE;
std::vector<VkFramebuffer> framebuffers;
VkExtent2D swapchain_extent;
```

## Next Phase

After Task 2 completion, proceed to Phase 48.3: First Quad Rendering

---

**Status**: Not started  
**Created**: November 2, 2025  

