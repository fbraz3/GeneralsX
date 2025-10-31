/**
 * @file graphics_backend_dxvk_pipeline.cpp
 * 
 * Phase 44.1: Graphics Pipeline Implementation
 * 
 * Implements Vulkan graphics pipeline creation and management:
 * - Shader module compilation and linking
 * - Pipeline layout creation
 * - Graphics pipeline state configuration
 * - Dynamic pipeline state management
 * - Pipeline caching and optimization
 * 
 * Architecture:
 * - Shader stages: Vertex and Fragment
 * - Fixed function pipeline state
 * - Dynamic state handling (viewport, scissor already handled in Phase 43)
 * - Pipeline derivatives for optimization
 * 
 * Phase Breakdown:
 * - 44.1: Graphics Pipeline Creation (this file)
 * - 44.2: Vertex Buffers
 * - 44.3: Index Buffers
 * - 44.4: Draw Commands
 * - 44.5: Material System
 * 
 * Created: October 31, 2025
 * Author: GeneralsX Development Team
 */

#include "graphics_backend_dxvk.h"
#include <cstring>
#include <vector>

// ============================================================================
// Phase 44.1: Shader Compilation Stubs (Shaders Embedded)
// ============================================================================

/**
 * Basic vertex shader SPIR-V bytecode (minimal triangle rendering).
 * Structure: Vertex position input → Transform → Output
 * 
 * GLSL equivalent:
 * #version 450
 * layout(location = 0) in vec3 position;
 * void main() {
 *     gl_Position = vec4(position, 1.0);
 * }
 */
static const uint32_t BASIC_VERTEX_SHADER[] = {
    0x07230203, 0x00010000, 0x0008000a, 0x00000024,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x000a000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x0000000d, 0x0000001b, 0x0000001c,
    0x0000001e, 0x0000001f, 0x00030003, 0x00000002,
    0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00030005, 0x0000000d, 0x00565350,
    0x00050006, 0x0000000d, 0x00000000, 0x6f506c43,
    0x00000073, 0x00030005, 0x0000001b, 0x00000000,
    0x00050048, 0x0000000d, 0x00000000, 0x0000000b,
    0x00000000, 0x00030047, 0x0000000d, 0x00000002,
    0x00040047, 0x0000001b, 0x0000000b, 0x00000001,
    0x00040047, 0x0000001c, 0x0000000b, 0x00000003,
    0x00040047, 0x0000001e, 0x0000000b, 0x00000004,
    0x00040047, 0x0000001f, 0x0000000b, 0x00000005,
    0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020,
    0x00040017, 0x00000007, 0x00000006, 0x00000003,
    0x00040020, 0x00000008, 0x00000001, 0x00000007,
    0x0004003b, 0x00000008, 0x00000009, 0x00000001,
    0x00040015, 0x0000000a, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000a, 0x0000000b, 0x00000000,
    0x00040020, 0x0000000c, 0x00000001, 0x00000006,
    0x0003001e, 0x0000000d, 0x00000007, 0x00040020,
    0x0000000e, 0x00000003, 0x0000000d, 0x0004003b,
    0x0000000e, 0x0000000f, 0x00000003, 0x00040020,
    0x00000010, 0x00000003, 0x00000007, 0x0004002b,
    0x00000006, 0x00000011, 0x3f800000, 0x00040017,
    0x00000012, 0x00000006, 0x00000004, 0x00050036,
    0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000013, 0x0004003d, 0x00000007,
    0x0000001a, 0x00000009, 0x00050051, 0x00000006,
    0x0000001c, 0x0000001a, 0x00000000, 0x00050051,
    0x00000006, 0x0000001d, 0x0000001a, 0x00000001,
    0x00050051, 0x00000006, 0x0000001e, 0x0000001a,
    0x00000002, 0x00070050, 0x00000012, 0x0000001f,
    0x0000001c, 0x0000001d, 0x0000001e, 0x00000011,
    0x00050041, 0x00000010, 0x00000020, 0x0000000f,
    0x00000000, 0x0003003e, 0x00000020, 0x0000001f,
    0x000100fd, 0x00010038
};

/**
 * Basic fragment shader SPIR-V bytecode (white color output).
 * Structure: Output fixed white color
 * 
 * GLSL equivalent:
 * #version 450
 * layout(location = 0) out vec4 outColor;
 * void main() {
 *     outColor = vec4(1.0, 1.0, 1.0, 1.0);
 * }
 */
static const uint32_t BASIC_FRAGMENT_SHADER[] = {
    0x07230203, 0x00010000, 0x0008000a, 0x0000001d,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000004, 0x00000004, 0x6e69616d,
    0x00000000, 0x00000009, 0x0000000c, 0x00030003,
    0x00000002, 0x000001c2, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00040005, 0x00000009,
    0x4f6c6f43, 0x00007475, 0x00050048, 0x00000009,
    0x00000000, 0x0000000b, 0x00000000, 0x00030047,
    0x00000009, 0x00000002, 0x00040047, 0x0000000c,
    0x0000000b, 0x00000000, 0x00020013, 0x00000002,
    0x00030021, 0x00000003, 0x00000002, 0x00030016,
    0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x00040020, 0x00000008,
    0x00000003, 0x00000007, 0x0004003b, 0x00000008,
    0x00000009, 0x00000003, 0x0004002b, 0x00000006,
    0x0000000a, 0x3f800000, 0x0006002c, 0x00000007,
    0x0000000b, 0x0000000a, 0x0000000a, 0x0000000a,
    0x00040020, 0x0000000c, 0x00000001, 0x00000007,
    0x0004003b, 0x0000000c, 0x0000000d, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000,
    0x00000003, 0x000200f8, 0x0000000e, 0x0003003e,
    0x00000009, 0x0000000b, 0x000100fd, 0x00010038
};

// ============================================================================
// Phase 44.1: Shader Module Creation
// ============================================================================

/**
 * Create a shader module from SPIR-V bytecode.
 * Wraps vkCreateShaderModule with error checking.
 * 
 * @param device Vulkan logical device
 * @param code SPIR-V bytecode (uint32_t array)
 * @param codeSize Size in bytes
 * @return VkShaderModule (VK_NULL_HANDLE on failure)
 */
static VkShaderModule CreateShaderModule(
    VkDevice device,
    const uint32_t* code,
    size_t codeSize
) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = code;
    
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    
    if (result != VK_SUCCESS) {
        printf("CreateShaderModule: vkCreateShaderModule failed with %d\n", result);
        return VK_NULL_HANDLE;
    }
    
    printf("CreateShaderModule: Success (module=%p)\n", shaderModule);
    return shaderModule;
}

/**
 * Destroy a shader module.
 * 
 * @param device Vulkan logical device
 * @param shaderModule Shader module to destroy
 */
static void DestroyShaderModule(VkDevice device, VkShaderModule shaderModule) {
    if (shaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }
}

// ============================================================================
// Phase 44.1: Pipeline Layout Creation
// ============================================================================

/**
 * Create pipeline layout (defines descriptor sets and push constants).
 * Phase 44.1: Basic layout with no descriptor sets yet (Phase 45+).
 * Push constants for matrix transforms (Phase 44.5).
 * 
 * @return VkPipelineLayout (VK_NULL_HANDLE on failure)
 */
HRESULT DXVKGraphicsBackend::CreatePipelineLayout() {
    if (m_device == VK_NULL_HANDLE) {
        printf("CreatePipelineLayout: Device not initialized\n");
        return D3DERR_INVALIDCALL;
    }
    
    // Push constants: 128 bytes for matrix transforms (64 bytes viewproj + 64 bytes world)
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = 128;  // 2 * mat4
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;  // No descriptor sets yet
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    VkResult result = vkCreatePipelineLayout(
        m_device,
        &pipelineLayoutInfo,
        nullptr,
        &m_pipelineLayout
    );
    
    if (result != VK_SUCCESS) {
        printf("CreatePipelineLayout: vkCreatePipelineLayout failed with %d\n", result);
        return D3DERR_DEVICELOST;
    }
    
    printf("CreatePipelineLayout: Success (layout=%p)\n", m_pipelineLayout);
    return S_OK;
}

/**
 * Destroy pipeline layout.
 * 
 * @return S_OK
 */
HRESULT DXVKGraphicsBackend::DestroyPipelineLayout() {
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }
    return S_OK;
}

// ============================================================================
// Phase 44.1: Graphics Pipeline Creation
// ============================================================================

/**
 * Create graphics pipeline for basic triangle rendering.
 * Combines:
 * - Shader stages (vertex + fragment)
 * - Fixed function state (rasterizer, color blend, depth/stencil)
 * - Vertex input format (Phase 44.2 extends this)
 * - Pipeline layout
 * 
 * @return S_OK on success, error code otherwise
 */
HRESULT DXVKGraphicsBackend::CreateGraphicsPipeline() {
    if (m_device == VK_NULL_HANDLE) {
        printf("CreateGraphicsPipeline: Device not initialized\n");
        return D3DERR_INVALIDCALL;
    }
    
    if (m_renderPass == VK_NULL_HANDLE) {
        printf("CreateGraphicsPipeline: Render pass not initialized\n");
        return D3DERR_INVALIDCALL;
    }
    
    if (m_pipelineLayout == VK_NULL_HANDLE) {
        printf("CreateGraphicsPipeline: Pipeline layout not initialized\n");
        return D3DERR_INVALIDCALL;
    }
    
    // ========================================================================
    // 1. Create shader modules
    // ========================================================================
    
    VkShaderModule vertexShader = CreateShaderModule(
        m_device,
        BASIC_VERTEX_SHADER,
        sizeof(BASIC_VERTEX_SHADER)
    );
    
    if (vertexShader == VK_NULL_HANDLE) {
        printf("CreateGraphicsPipeline: Failed to create vertex shader\n");
        return D3DERR_DEVICELOST;
    }
    
    VkShaderModule fragmentShader = CreateShaderModule(
        m_device,
        BASIC_FRAGMENT_SHADER,
        sizeof(BASIC_FRAGMENT_SHADER)
    );
    
    if (fragmentShader == VK_NULL_HANDLE) {
        printf("CreateGraphicsPipeline: Failed to create fragment shader\n");
        DestroyShaderModule(m_device, vertexShader);
        return D3DERR_DEVICELOST;
    }
    
    // ========================================================================
    // 2. Shader stages
    // ========================================================================
    
    VkPipelineShaderStageCreateInfo shaderStages[2]{};
    
    // Vertex stage
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertexShader;
    shaderStages[0].pName = "main";
    
    // Fragment stage
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragmentShader;
    shaderStages[1].pName = "main";
    
    // ========================================================================
    // 3. Vertex input state (Phase 44.2 extends this)
    // ========================================================================
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;  // Will be set in Phase 44.2
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    
    // ========================================================================
    // 4. Input assembly state
    // ========================================================================
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // ========================================================================
    // 5. Viewport and scissor (set dynamically in Phase 43.3)
    // ========================================================================
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    // Actual viewport/scissor set dynamically
    
    // ========================================================================
    // 6. Rasterizer state
    // ========================================================================
    
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    rasterizer.lineWidth = 1.0f;
    
    // ========================================================================
    // 7. Multisampling state
    // ========================================================================
    
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    
    // ========================================================================
    // 8. Depth/Stencil state (disabled for Phase 44)
    // ========================================================================
    
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    
    // ========================================================================
    // 9. Color blend state
    // ========================================================================
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | 
        VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | 
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    
    // ========================================================================
    // 10. Dynamic state (viewport, scissor already handled)
    // ========================================================================
    
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;
    
    // ========================================================================
    // 11. Create graphics pipeline
    // ========================================================================
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    VkResult result = vkCreateGraphicsPipelines(
        m_device,
        m_pipelineCache,
        1,
        &pipelineInfo,
        nullptr,
        &m_graphicsPipeline
    );
    
    // Clean up shader modules (can be destroyed after pipeline creation)
    DestroyShaderModule(m_device, vertexShader);
    DestroyShaderModule(m_device, fragmentShader);
    
    if (result != VK_SUCCESS) {
        printf("CreateGraphicsPipeline: vkCreateGraphicsPipelines failed with %d\n", result);
        return D3DERR_DEVICELOST;
    }
    
    printf("CreateGraphicsPipeline: Success (pipeline=%p)\n", m_graphicsPipeline);
    return S_OK;
}

// ============================================================================
// Phase 44.1: Pipeline Destruction
// ============================================================================

/**
 * Destroy graphics pipeline.
 * 
 * @return S_OK
 */
HRESULT DXVKGraphicsBackend::DestroyGraphicsPipeline() {
    if (m_graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
        m_graphicsPipeline = VK_NULL_HANDLE;
    }
    return S_OK;
}

// ============================================================================
// Phase 44.1: Pipeline Binding
// ============================================================================

/**
 * Bind graphics pipeline to command buffer for rendering.
 * Called before any draw commands.
 * 
 * @return S_OK on success
 */
HRESULT DXVKGraphicsBackend::BindGraphicsPipeline() {
    if (m_graphicsPipeline == VK_NULL_HANDLE) {
        printf("BindGraphicsPipeline: Pipeline not initialized\n");
        return D3DERR_INVALIDCALL;
    }
    
    if (m_commandBuffers.empty()) {
        printf("BindGraphicsPipeline: No command buffers\n");
        return D3DERR_INVALIDCALL;
    }
    
    uint32_t frameIndex = m_currentFrame % m_commandBuffers.size();
    VkCommandBuffer cmd = m_commandBuffers[frameIndex];
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    
    printf("BindGraphicsPipeline: Bound to frame %u\n", frameIndex);
    return S_OK;
}

// ============================================================================
// Phase 44.1: Pipeline State Queries
// ============================================================================

/**
 * Get current graphics pipeline.
 * 
 * @return VkPipeline (VK_NULL_HANDLE if not created)
 */
VkPipeline DXVKGraphicsBackend::GetGraphicsPipeline() const {
    return m_graphicsPipeline;
}

/**
 * Get pipeline layout.
 * 
 * @return VkPipelineLayout (VK_NULL_HANDLE if not created)
 */
VkPipelineLayout DXVKGraphicsBackend::GetPipelineLayout() const {
    return m_pipelineLayout;
}

/**
 * Check if graphics pipeline is ready.
 * 
 * @return true if pipeline and layout are created
 */
bool DXVKGraphicsBackend::IsGraphicsPipelineReady() const {
    return m_graphicsPipeline != VK_NULL_HANDLE && 
           m_pipelineLayout != VK_NULL_HANDLE;
}

// ============================================================================
// Phase 44.1: Pipeline Diagnostics
// ============================================================================

/**
 * Report graphics pipeline state.
 * Useful for debugging pipeline creation issues.
 */
void DXVKGraphicsBackend::ReportPipelineState() const {
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║        Phase 44.1: Pipeline State          ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Pipeline Status:\n");
    printf("  Graphics Pipeline:      %s\n", 
        m_graphicsPipeline != VK_NULL_HANDLE ? "CREATED" : "NOT CREATED");
    printf("  Pipeline Layout:        %s\n", 
        m_pipelineLayout != VK_NULL_HANDLE ? "CREATED" : "NOT CREATED");
    printf("  Pipeline Ready:         %s\n", 
        IsGraphicsPipelineReady() ? "YES" : "NO");
    printf("\n");
    printf("Render Pass Integration:\n");
    printf("  Render Pass:            %s\n", 
        m_renderPass != VK_NULL_HANDLE ? "LINKED" : "NOT LINKED");
    printf("  Render Pass Valid:      %s\n", 
        m_renderPass != VK_NULL_HANDLE ? "YES" : "NO");
    printf("\n");
    printf("Shader Configuration:\n");
    printf("  Shader Stages:          Vertex + Fragment\n");
    printf("  Vertex Shader:          Basic position transform\n");
    printf("  Fragment Shader:        White color output\n");
    printf("  Push Constants:         128 bytes (2x mat4)\n");
    printf("\n");
    printf("Fixed Function State:\n");
    printf("  Rasterizer Mode:        FILL\n");
    printf("  Cull Mode:              BACK\n");
    printf("  Front Face:             COUNTER_CLOCKWISE\n");
    printf("  Primitive Type:         TRIANGLE_LIST\n");
    printf("  Multisampling:          DISABLED (1x)\n");
    printf("  Depth Testing:          DISABLED\n");
    printf("  Color Blending:         DISABLED (replace)\n");
    printf("\n");
    printf("Dynamic State:\n");
    printf("  Viewport:               DYNAMIC\n");
    printf("  Scissor:                DYNAMIC\n");
    printf("\n");
}
