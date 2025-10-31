/**
 * @file graphics_backend_dxvk_drawing.cpp
 * 
 * Phase 41: Drawing Operations Implementation
 * 
 * Implements primitive rendering, render states, materials, and lighting.
 * 
 * Key Components:
 * - DrawPrimitive: Render non-indexed geometry
 * - DrawIndexedPrimitive: Render indexed geometry
 * - SetRenderState: Configure GPU render states
 * - Material/Light system: Manage material and lighting properties
 * 
 * Architecture:
 * - Vulkan command buffer recording
 * - Pipeline state management
 * - Render state translation (DirectX 8 → Vulkan)
 * - Lighting calculations
 * 
 * Phase: 41 (Drawing Operations)
 * Status: Implementation in progress
 */

#include "graphics_backend_dxvk.h"
#include <cstring>
#include <algorithm>

// DirectX error codes (defined here if not available)
#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL 0x8876 + 0x0086
#endif

#ifndef D3DERR_NOTAVAILABLE
#define D3DERR_NOTAVAILABLE 0x8876 + 0x0091
#endif

// Render state constants
#ifndef D3DRS_FOGMODE
#define D3DRS_FOGMODE 5
#endif

/**
 * DrawPrimitive: Render non-indexed geometry
 * 
 * Renders a primitive (triangle, line, point) from vertex buffer data.
 * 
 * Parameters:
 * - primitiveType: D3DPRIMITIVETYPE (D3DPT_TRIANGLELIST, D3DPT_LINESTRIP, etc.)
 * - startVertex: First vertex in buffer to render
 * - primitiveCount: Number of primitives to render
 * 
 * DirectX to Vulkan mapping:
 * D3DPT_TRIANGLELIST    → VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST (3 vertices per tri)
 * D3DPT_TRIANGLESTRIP   → VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
 * D3DPT_TRIANGLEFAN     → VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN (if supported)
 * D3DPT_LINESTRIP       → VK_PRIMITIVE_TOPOLOGY_LINE_STRIP
 * D3DPT_LINELIST        → VK_PRIMITIVE_TOPOLOGY_LINE_LIST
 * D3DPT_POINTLIST       → VK_PRIMITIVE_TOPOLOGY_POINT_LIST
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::DrawPrimitive(
    D3DPRIMITIVETYPE primitiveType,
    uint32_t startVertex,
    uint32_t primitiveCount)
{
    if (!m_initialized) {
        printf("Phase 41: DrawPrimitive - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }

    if (primitiveCount == 0) {
        printf("Phase 41: DrawPrimitive - Invalid primitive count (0)\n");
        return D3DERR_INVALIDCALL;
    }

    if (!m_currentVertexBuffer || m_currentVertexBuffer->buffer == VK_NULL_HANDLE) {
        printf("Phase 41: DrawPrimitive - No vertex buffer bound\n");
        return D3DERR_NOTAVAILABLE;
    }

    // Calculate vertex count from primitive count
    uint32_t vertexCount = 0;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    
    switch (primitiveType) {
        case D3DPT_TRIANGLELIST:
            vertexCount = primitiveCount * 3;
            topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case D3DPT_TRIANGLESTRIP:
            vertexCount = primitiveCount + 2;
            topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case D3DPT_LINESTRIP:
            vertexCount = primitiveCount + 1;
            topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case D3DPT_LINELIST:
            vertexCount = primitiveCount * 2;
            topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case D3DPT_POINTLIST:
            vertexCount = primitiveCount;
            topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        default:
            printf("Phase 41: DrawPrimitive - Unknown primitive type (%d)\n", primitiveType);
            return D3DERR_INVALIDCALL;
    }

    // Verify vertex count doesn't exceed buffer
    if (startVertex + vertexCount > m_currentVertexBuffer->size / sizeof(float)) {
        printf("Phase 41: DrawPrimitive - Vertex count exceeds buffer size\n");
        return D3DERR_INVALIDCALL;
    }

    // Bind vertex buffer to command buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_commandBuffers[m_currentFrame], 0, 1, &m_currentVertexBuffer->buffer, &offset);

    // Bind pipeline with topology
    // TODO: Phase 41.2 - Create/bind pipeline based on topology
    if (m_graphicsPipeline != VK_NULL_HANDLE) {
        vkCmdBindPipeline(m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    }

    // Record draw command
    vkCmdDraw(m_commandBuffers[m_currentFrame], vertexCount, 1, startVertex, 0);

    printf("Phase 41: DrawPrimitive - vertices=%u, primitives=%u, type=%d (EXECUTED)\n",
           vertexCount, primitiveCount, primitiveType);

    return D3D_OK;
}

/**
 * DrawIndexedPrimitive: Render indexed geometry
 * 
 * Renders indexed primitives using separate index buffer.
 * More efficient for meshes with shared vertices.
 * 
 * Parameters:
 * - primitiveType: D3DPRIMITIVETYPE
 * - baseVertexIndex: Index of first vertex in vertex buffer
 * - minVertexIndex: Minimum vertex index value
 * - numVertices: Number of vertices to render
 * - startIndex: First index in index buffer
 * - primitiveCount: Number of primitives
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::DrawIndexedPrimitive(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int baseVertexIndex,
    unsigned int minVertexIndex,
    unsigned int numVertices,
    unsigned int startIndex,
    unsigned int primitiveCount)
{
    if (!m_initialized) {
        printf("Phase 41: DrawIndexedPrimitive - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }

    if (primitiveCount == 0) {
        printf("Phase 41: DrawIndexedPrimitive - Invalid primitive count (0)\n");
        return D3DERR_INVALIDCALL;
    }

    if (!m_currentVertexBuffer || m_currentVertexBuffer->buffer == VK_NULL_HANDLE) {
        printf("Phase 41: DrawIndexedPrimitive - No vertex buffer bound\n");
        return D3DERR_NOTAVAILABLE;
    }

    if (!m_currentIndexBuffer || m_currentIndexBuffer->buffer == VK_NULL_HANDLE) {
        printf("Phase 41: DrawIndexedPrimitive - No index buffer bound\n");
        return D3DERR_NOTAVAILABLE;
    }

    // Calculate index count from primitive count
    uint32_t indexCount = 0;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    
    switch (primitiveType) {
        case D3DPT_TRIANGLELIST:
            indexCount = primitiveCount * 3;
            topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case D3DPT_TRIANGLESTRIP:
            indexCount = primitiveCount + 2;
            topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case D3DPT_LINESTRIP:
            indexCount = primitiveCount + 1;
            topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case D3DPT_LINELIST:
            indexCount = primitiveCount * 2;
            topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case D3DPT_POINTLIST:
            indexCount = primitiveCount;
            topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        default:
            printf("Phase 41: DrawIndexedPrimitive - Unknown primitive type (%d)\n", primitiveType);
            return D3DERR_INVALIDCALL;
    }

    // Verify indices don't exceed buffer
    if (startIndex + indexCount > m_currentIndexBuffer->size / sizeof(uint16_t)) {
        printf("Phase 41: DrawIndexedPrimitive - Index count exceeds buffer size\n");
        return D3DERR_INVALIDCALL;
    }

    // Bind vertex buffer to command buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_commandBuffers[m_currentFrame], 0, 1, &m_currentVertexBuffer->buffer, &offset);

    // Bind index buffer to command buffer
    vkCmdBindIndexBuffer(m_commandBuffers[m_currentFrame], m_currentIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT16);

    // Bind pipeline with topology
    // TODO: Phase 41.2 - Create/bind pipeline based on topology
    if (m_graphicsPipeline != VK_NULL_HANDLE) {
        vkCmdBindPipeline(m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    }

    // Record indexed draw command
    vkCmdDrawIndexed(m_commandBuffers[m_currentFrame], indexCount, 1, startIndex, baseVertexIndex, 0);

    printf("Phase 41: DrawIndexedPrimitive - indices=%u, primitives=%u, baseVertex=%d, type=%d (EXECUTED)\n",
           indexCount, primitiveCount, baseVertexIndex, primitiveType);

    return D3D_OK;
}

/**
 * SetRenderState: Configure GPU render state
 * 
 * Configures rendering pipeline state options.
 * Examples: depth testing, blending, culling, etc.
 * 
 * DirectX 8 Render States and Vulkan equivalents:
 * 
 * D3DRS_ZENABLE              → VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE
 * D3DRS_ZFUNC                → vkCmdSetCompareOp (VkCompareOp)
 * D3DRS_ZWRITEENABLE         → VkPipelineDepthStencilStateCreateInfo.depthWriteEnable
 * D3DRS_ALPHATESTENABLE      → VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT
 * D3DRS_ALPHAFUNC            → vkCmdSetCompareOp
 * D3DRS_ALPHAREF             → Push constant or UBO
 * D3DRS_SRCBLEND             → VkPipelineColorBlendAttachmentState.srcColorBlendFactor
 * D3DRS_DESTBLEND            → VkPipelineColorBlendAttachmentState.dstColorBlendFactor
 * D3DRS_CULLMODE             → VkPipelineRasterizationStateCreateInfo.cullMode
 * D3DRS_FILLMODE             → VkPipelineRasterizationStateCreateInfo.polygonMode
 * D3DRS_LIGHTING             → UBO/push constant (vertex shader control)
 * D3DRS_FOGMODE              → UBO/push constant (fragment shader)
 * 
 * Parameters:
 * - state: D3DRENDERSTATETYPE (which state to set)
 * - value: Value for the state
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::SetRenderState(
    D3DRENDERSTATETYPE state,
    DWORD value)
{
    if (!m_initialized) {
        printf("Phase 41: SetRenderState - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }

    // Store render state in member variable for later use
    m_renderStates[state] = value;

    switch (state) {
        // Depth testing
        case D3DRS_ZENABLE:
            m_depthTestEnabled = (value != FALSE);
            printf("Phase 41: SetRenderState - ZENABLE=%s\n", 
                   m_depthTestEnabled ? "true" : "false");
            break;

        case D3DRS_ZWRITEENABLE:
            m_depthWriteEnabled = (value != FALSE);
            printf("Phase 41: SetRenderState - ZWRITEENABLE=%s\n",
                   m_depthWriteEnabled ? "true" : "false");
            break;

        case D3DRS_ZFUNC:
            m_depthFunc = value;
            printf("Phase 41: SetRenderState - ZFUNC=%u\n", value);
            break;

        // Blending
        case D3DRS_ALPHABLENDENABLE:
            m_blendEnabled = (value != FALSE);
            printf("Phase 41: SetRenderState - ALPHABLENDENABLE=%s\n",
                   m_blendEnabled ? "true" : "false");
            break;

        case D3DRS_SRCBLEND:
            m_srcBlend = value;
            printf("Phase 41: SetRenderState - SRCBLEND=%u\n", value);
            break;

        case D3DRS_DESTBLEND:
            m_dstBlend = value;
            printf("Phase 41: SetRenderState - DESTBLEND=%u\n", value);
            break;

        // Culling
        case D3DRS_CULLMODE:
            m_cullMode = value;
            printf("Phase 41: SetRenderState - CULLMODE=%u\n", value);
            break;

        case D3DRS_FILLMODE:
            m_fillMode = value;
            printf("Phase 41: SetRenderState - FILLMODE=%u\n", value);
            break;

        // Lighting
        case D3DRS_LIGHTING:
            m_lightingEnabled = (value != FALSE);
            printf("Phase 41: SetRenderState - LIGHTING=%s\n",
                   m_lightingEnabled ? "true" : "false");
            break;

        // Fog
        case D3DRS_FOGENABLE:
            m_fogEnabled = (value != FALSE);
            printf("Phase 41: SetRenderState - FOGENABLE=%s\n",
                   m_fogEnabled ? "true" : "false");
            break;

        case D3DRS_FOGMODE:
            m_fogMode = value;
            printf("Phase 41: SetRenderState - FOGMODE=%u\n", value);
            break;

        // TODO: Phase 41.2 - Implement remaining render states
        
        default:
            printf("Phase 41: SetRenderState - Unknown render state type (%u)\n", state);
            return D3DERR_INVALIDCALL;
    }

    return D3D_OK;
}

/**
 * SetMaterial: Set material properties for subsequent rendering
 * 
 * Configures diffuse, specular, ambient, and emissive colors
 * plus shininess for lighting calculations.
 * 
 * Parameters:
 * - material: Pointer to D3DMATERIAL8 structure
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::SetMaterial(const D3DMATERIAL8* material)
{
    if (!m_initialized) {
        printf("Phase 41: SetMaterial - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }

    if (!material) {
        printf("Phase 41: SetMaterial - Null material pointer\n");
        return D3DERR_INVALIDCALL;
    }

    std::memcpy(&m_material, material, sizeof(D3DMATERIAL8));

    printf("Phase 41: SetMaterial - material set (%zu bytes)\n",
           sizeof(D3DMATERIAL8));

    // TODO: Phase 41.2 - Update material UBO in vertex/fragment shaders

    return D3D_OK;
}

/**
 * SetLight: Configure a light source
 * 
 * Sets up directional, point, or spotlight for scene illumination.
 * 
 * Supported light types:
 * - D3DLIGHT_DIRECTIONAL: Sun-like infinitely distant light
 * - D3DLIGHT_POINT: Light radiating from a point
 * - D3DLIGHT_SPOT: Spotlight with limited cone
 * 
 * Parameters:
 * - index: Light index (0-7 typically)
 * - light: Pointer to D3DLIGHT8 structure
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::SetLight(
    uint32_t index,
    const D3DLIGHT8* light)
{
    if (!m_initialized) {
        printf("Phase 41: SetLight - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }

    if (!light) {
        printf("Phase 41: SetLight - Null light pointer\n");
        return D3DERR_INVALIDCALL;
    }

    if (index >= MAX_LIGHTS) {
        printf("Phase 41: SetLight - Light index (%u) exceeds maximum (%u)\n",
               index, MAX_LIGHTS);
        return D3DERR_INVALIDCALL;
    }

    std::memcpy(&m_lights[index], light, sizeof(D3DLIGHT8));
    m_activeLights[index] = true;

    const char* typeStr = "";
    switch (light->Type) {
        case D3DLIGHT_DIRECTIONAL: typeStr = "DIRECTIONAL"; break;
        case D3DLIGHT_POINT: typeStr = "POINT"; break;
        case D3DLIGHT_SPOT: typeStr = "SPOT"; break;
        default: typeStr = "UNKNOWN"; break;
    }

    printf("Phase 41: SetLight[%u] - type=%s, diffuse=(%f,%f,%f), "
           "position=(%f,%f,%f), direction=(%f,%f,%f)\n",
           index, typeStr,
           light->Diffuse.r, light->Diffuse.g, light->Diffuse.b,
           light->Position.x, light->Position.y, light->Position.z,
           light->Direction.x, light->Direction.y, light->Direction.z);

    // TODO: Phase 41.2 - Update light UBO in vertex/fragment shaders

    return D3D_OK;
}

/**
 * LightEnable: Enable or disable a light
 * 
 * Parameters:
 * - index: Light index
 * - enable: true to enable, false to disable
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::LightEnable(
    unsigned int index,
    bool enable)
{
    if (!m_initialized) {
        printf("Phase 41: LightEnable - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }

    if (index >= MAX_LIGHTS) {
        printf("Phase 41: LightEnable - Light index (%u) exceeds maximum (%u)\n",
               index, MAX_LIGHTS);
        return D3DERR_INVALIDCALL;
    }

    m_activeLights[index] = enable;

    printf("Phase 41: LightEnable[%u] - %s\n",
           index, enable ? "enabled" : "disabled");

    return D3D_OK;
}

/**
 * SetViewport: Configure viewport transformation
 * 
 * Defines the screen-space viewport (scissor region and depth range).
 * 
 * Parameters:
 * - x, y: Top-left corner of viewport in pixels
 * - width, height: Viewport dimensions in pixels
 * - minZ, maxZ: Depth range (typically 0.0 to 1.0)
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::SetViewport(
    unsigned int x,
    unsigned int y,
    unsigned int width,
    unsigned int height,
    float minZ,
    float maxZ)
{
    if (!m_initialized) {
        printf("Phase 41: SetViewport - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }

    m_viewport.X = x;
    m_viewport.Y = y;
    m_viewport.Width = width;
    m_viewport.Height = height;
    m_viewport.MinZ = minZ;
    m_viewport.MaxZ = maxZ;

    // Set Vulkan viewport if we're in a scene
    if (m_inScene && m_commandBuffers[m_currentFrame] != VK_NULL_HANDLE) {
        VkViewport vkViewport{};
        vkViewport.x = (float)x;
        vkViewport.y = (float)y;
        vkViewport.width = (float)width;
        vkViewport.height = (float)height;
        vkViewport.minDepth = minZ;
        vkViewport.maxDepth = maxZ;
        
        vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &vkViewport);
        
        // Also set scissor rect to match viewport
        VkRect2D scissor{};
        scissor.offset.x = (int32_t)x;
        scissor.offset.y = (int32_t)y;
        scissor.extent.width = width;
        scissor.extent.height = height;
        
        vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);
    }

    printf("Phase 41: SetViewport - x=%u, y=%u, width=%u, height=%u, minz=%f, maxz=%f (SET)\n",
           x, y, width, height, minZ, maxZ);

    // TODO: Phase 41.2 - Update Vulkan viewport and scissor rect

    return D3D_OK;
}

// ============================================================================
// Helper Functions: DirectX to Vulkan State Mapping
// ============================================================================

/**
 * Convert DirectX blend mode to Vulkan blend factor.
 * 
 * Maps D3DBLEND values to VkBlendFactor for color blending.
 */
VkBlendFactor ConvertD3DBlendMode(DWORD d3dBlend) {
    switch (d3dBlend) {
        case 1:  // D3DBLEND_ZERO
            return VK_BLEND_FACTOR_ZERO;
        case 2:  // D3DBLEND_ONE
            return VK_BLEND_FACTOR_ONE;
        case 3:  // D3DBLEND_SRCCOLOR
            return VK_BLEND_FACTOR_SRC_COLOR;
        case 4:  // D3DBLEND_INVSRCCOLOR
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case 5:  // D3DBLEND_SRCALPHA
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case 6:  // D3DBLEND_INVSRCALPHA
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case 7:  // D3DBLEND_DESTALPHA
            return VK_BLEND_FACTOR_DST_ALPHA;
        case 8:  // D3DBLEND_INVDESTALPHA
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case 9:  // D3DBLEND_DESTCOLOR
            return VK_BLEND_FACTOR_DST_COLOR;
        case 10: // D3DBLEND_INVDESTCOLOR
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        default:
            return VK_BLEND_FACTOR_ONE;
    }
}

/**
 * Convert DirectX depth comparison function to Vulkan.
 * 
 * Maps D3DCMPFUNC values to VkCompareOp for depth testing.
 */
VkCompareOp ConvertD3DCompareFunc(DWORD d3dCmpFunc) {
    switch (d3dCmpFunc) {
        case 1:  // D3DCMP_NEVER
            return VK_COMPARE_OP_NEVER;
        case 2:  // D3DCMP_LESS
            return VK_COMPARE_OP_LESS;
        case 3:  // D3DCMP_EQUAL
            return VK_COMPARE_OP_EQUAL;
        case 4:  // D3DCMP_LESSEQUAL
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case 5:  // D3DCMP_GREATER
            return VK_COMPARE_OP_GREATER;
        case 6:  // D3DCMP_NOTEQUAL
            return VK_COMPARE_OP_NOT_EQUAL;
        case 7:  // D3DCMP_GREATEREQUAL
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case 8:  // D3DCMP_ALWAYS
            return VK_COMPARE_OP_ALWAYS;
        default:
            return VK_COMPARE_OP_LESS;
    }
}

/**
 * Convert DirectX cull mode to Vulkan.
 * 
 * Maps D3DCULL values to VkCullModeFlagBits.
 */
VkCullModeFlags ConvertD3DCullMode(DWORD d3dCull) {
    switch (d3dCull) {
        case 1:  // D3DCULL_NONE
            return VK_CULL_MODE_NONE;
        case 2:  // D3DCULL_CW
            return VK_CULL_MODE_BACK_BIT;
        case 3:  // D3DCULL_CCW
            return VK_CULL_MODE_FRONT_BIT;
        default:
            return VK_CULL_MODE_BACK_BIT;
    }
}

/**
 * Convert DirectX fill mode to Vulkan.
 * 
 * Maps D3DFILLMODE values to VkPolygonMode.
 */
VkPolygonMode ConvertD3DFillMode(DWORD d3dFill) {
    switch (d3dFill) {
        case 1:  // D3DFILL_POINT
            return VK_POLYGON_MODE_POINT;
        case 2:  // D3DFILL_WIREFRAME
            return VK_POLYGON_MODE_LINE;
        case 3:  // D3DFILL_SOLID
            return VK_POLYGON_MODE_FILL;
        default:
            return VK_POLYGON_MODE_FILL;
    }
}
