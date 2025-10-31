/**
 * @file graphics_backend_dxvk_draw_commands.cpp
 * 
 * Vulkan Graphics Backend - Draw Command Helpers & Validation
 * 
 * Implements draw command validation, primitive type conversion,
 * draw state verification, and diagnostic output for rendering operations.
 * 
 * Phase 44.4: Draw Commands Stack
 * 
 * This file consolidates draw command logic:
 * - DrawPrimitive & DrawIndexedPrimitive are in graphics_backend_dxvk_render.cpp
 * - This file provides validation, state checking, and helper functions
 * - Combines Phase 44.1 (pipeline) + 44.2 (vertices) + 44.3 (indices)
 * 
 * Created: October 31, 2025
 */

#include "graphics_backend_dxvk.h"

// ============================================================================
// Draw State Validation
// ============================================================================

/**
 * Validate that the graphics pipeline is in a valid state for drawing.
 * Checks: pipeline initialized, vertex/index buffers ready, render state valid.
 */
bool DXVKGraphicsBackend::ValidateDrawState() {
    // Check basic initialization
    if (!m_initialized) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateDrawState - Backend not initialized\n");
        return false;
    }
    
    // Check scene state
    if (!m_inScene) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateDrawState - Not in scene (BeginScene not called)\n");
        return false;
    }
    
    // Check pipeline exists
    if (m_graphicsPipeline == VK_NULL_HANDLE) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateDrawState - Graphics pipeline not initialized\n");
        return false;
    }
    
    // Check command buffer
    if (m_commandBuffers.empty() || m_commandBuffers[m_currentFrame] == VK_NULL_HANDLE) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateDrawState - Current command buffer is null\n");
        return false;
    }
    
    // Check vertex buffers
    if (m_vertexBuffers.empty()) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateDrawState - No vertex buffers bound\n");
        return false;
    }
    
    // Verify at least one vertex buffer has valid data
    bool hasValidBuffer = false;
    for (const auto& pair : m_vertexBuffers) {
        if (pair.second && pair.second->buffer != VK_NULL_HANDLE) {
            hasValidBuffer = true;
            break;
        }
    }
    
    if (!hasValidBuffer) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateDrawState - All vertex buffers are null\n");
        return false;
    }
    
    // Check render target is set
    if (m_renderPass == VK_NULL_HANDLE) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateDrawState - Render pass not initialized\n");
        return false;
    }
    
    // Viewport state is less critical - will be set to default if not specified
    
    return true;
}

/**
 * Validate indexed draw state specifically (DrawIndexedPrimitive requirements).
 */
bool DXVKGraphicsBackend::ValidateIndexedDrawState() {
    // Check base draw state
    if (!ValidateDrawState()) {
        return false;
    }
    
    // Check index buffer
    if (m_indexBuffer == VK_NULL_HANDLE) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateIndexedDrawState - Index buffer is null\n");
        return false;
    }
    
    // Check index count is non-zero
    if (m_indexCount == 0) {
        if (m_debugOutput) printf("[DXVK] WARNING: ValidateIndexedDrawState - Index count is zero\n");
        return false;
    }
    
    // Check index format is valid (UINT16 or UINT32)
    if (m_currentIndexFormat != VK_INDEX_TYPE_UINT16 && m_currentIndexFormat != VK_INDEX_TYPE_UINT32) {
        if (m_debugOutput) printf("[DXVK] ERROR: ValidateIndexedDrawState - Index format not set properly\n");
        return false;
    }
    
    return true;
}

/**
 * Report current draw state for diagnostics.
 */
void DXVKGraphicsBackend::ReportDrawState() {
    printf("\n[DXVK] === DRAW STATE REPORT ===\n");
    printf("  Initialized: %s\n", m_initialized ? "YES" : "NO");
    printf("  In Scene: %s\n", m_inScene ? "YES" : "NO");
    printf("  Current Frame: %u\n", m_currentFrame);
    printf("  Graphics Pipeline: %s\n", m_graphicsPipeline != VK_NULL_HANDLE ? "VALID" : "NULL");
    printf("  Command Buffer: %s\n", 
           (m_commandBuffers.empty() || m_commandBuffers[m_currentFrame] == VK_NULL_HANDLE) ? "NULL" : "VALID");
    printf("  Vertex Buffers: %zu\n", m_vertexBuffers.size());
    for (const auto& pair : m_vertexBuffers) {
        printf("    [%u]: %s (size=%zu bytes)\n", 
               pair.first,
               pair.second && pair.second->buffer != VK_NULL_HANDLE ? "VALID" : "NULL",
               pair.second ? pair.second->size : 0);
    }
    printf("  Index Buffer: %s (count=%u, stride=%u bytes)\n",
           m_indexBuffer != VK_NULL_HANDLE ? "VALID" : "NULL",
           m_indexCount,
           m_indexStride);
    printf("  Render Pass: %s\n", m_renderPass != VK_NULL_HANDLE ? "VALID" : "NULL");
    printf("  Viewport: (%u,%u) %ux%u [%.2f..%.2f]\n",
           m_viewport.X, m_viewport.Y, m_viewport.Width, m_viewport.Height,
           m_viewport.MinZ, m_viewport.MaxZ);
    printf("  Last Error: 0x%08lX\n", m_lastError);
    printf("[DXVK] === END REPORT ===\n\n");
}

// ============================================================================
// Primitive Type Conversion & Validation
// ============================================================================

/**
 * Convert D3D primitive type to Vulkan topology and calculate vertex/index count.
 * Returns true if conversion succeeds, false if invalid type.
 */
bool DXVKGraphicsBackend::ConvertPrimitiveType(
    D3DPRIMITIVETYPE d3dType,
    VkPrimitiveTopology& outTopology,
    unsigned int primitiveCount,
    unsigned int& outVertexIndexCount) {
    
    outVertexIndexCount = 0;
    
    switch (d3dType) {
        case D3DPT_POINTLIST:
            outTopology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            outVertexIndexCount = primitiveCount;
            return true;
            
        case D3DPT_LINELIST:
            outTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            outVertexIndexCount = primitiveCount * 2;
            return true;
            
        case D3DPT_LINESTRIP:
            outTopology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            outVertexIndexCount = primitiveCount + 1;
            return true;
            
        case D3DPT_TRIANGLELIST:
            outTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            outVertexIndexCount = primitiveCount * 3;
            return true;
            
        case D3DPT_TRIANGLESTRIP:
            outTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            outVertexIndexCount = primitiveCount + 2;
            return true;
            
        case D3DPT_TRIANGLEFAN:
            outTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            outVertexIndexCount = primitiveCount + 2;
            return true;
            
        default:
            if (m_debugOutput) {
                printf("[DXVK] ERROR: ConvertPrimitiveType - Unknown primitive type: %d\n", d3dType);
            }
            return false;
    }
}

/**
 * Get human-readable name for primitive type (debug output).
 */
const char* DXVKGraphicsBackend::GetPrimitiveTypeName(D3DPRIMITIVETYPE type) {
    switch (type) {
        case D3DPT_POINTLIST:       return "POINTLIST";
        case D3DPT_LINELIST:        return "LINELIST";
        case D3DPT_LINESTRIP:       return "LINESTRIP";
        case D3DPT_TRIANGLELIST:    return "TRIANGLELIST";
        case D3DPT_TRIANGLESTRIP:   return "TRIANGLESTRIP";
        case D3DPT_TRIANGLEFAN:     return "TRIANGLEFAN";
        default:                    return "UNKNOWN";
    }
}

// ============================================================================
// Draw Parameter Validation
// ============================================================================

/**
 * Validate DrawPrimitive parameters.
 */
bool DXVKGraphicsBackend::ValidateDrawPrimitiveParams(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int startVertex,
    unsigned int primitiveCount,
    unsigned int maxVertices) {
    
    // Validate primitive type
    VkPrimitiveTopology topology;
    unsigned int vertexCount;
    if (!ConvertPrimitiveType(primitiveType, topology, primitiveCount, vertexCount)) {
        return false;
    }
    
    // Check primitive count is non-zero
    if (primitiveCount == 0) {
        if (m_debugOutput) printf("[DXVK] WARNING: DrawPrimitive - primitiveCount is zero\n");
        return false;
    }
    
    // Check start vertex + vertex count doesn't exceed buffer size
    if (startVertex + vertexCount > maxVertices) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: DrawPrimitive - Vertex range [%u..%u) exceeds buffer size %u\n",
                   startVertex, startVertex + vertexCount, maxVertices);
        }
        return false;
    }
    
    return true;
}

/**
 * Validate DrawIndexedPrimitive parameters.
 */
bool DXVKGraphicsBackend::ValidateDrawIndexedParams(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int baseVertexIndex,
    unsigned int minVertexIndex,
    unsigned int numVertices,
    unsigned int startIndex,
    unsigned int primitiveCount,
    unsigned int maxIndices,
    unsigned int maxVertices) {
    
    // Validate primitive type
    VkPrimitiveTopology topology;
    unsigned int indexCount;
    if (!ConvertPrimitiveType(primitiveType, topology, primitiveCount, indexCount)) {
        return false;
    }
    
    // Check primitive count is non-zero
    if (primitiveCount == 0) {
        if (m_debugOutput) printf("[DXVK] WARNING: DrawIndexedPrimitive - primitiveCount is zero\n");
        return false;
    }
    
    // Check index range
    if (startIndex + indexCount > maxIndices) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: DrawIndexedPrimitive - Index range [%u..%u) exceeds buffer size %u\n",
                   startIndex, startIndex + indexCount, maxIndices);
        }
        return false;
    }
    
    // Check vertex range
    if (minVertexIndex + numVertices > maxVertices) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: DrawIndexedPrimitive - Vertex range [%u..%u) exceeds buffer size %u\n",
                   minVertexIndex, minVertexIndex + numVertices, maxVertices);
        }
        return false;
    }
    
    // Check base vertex offset
    if (baseVertexIndex > 100000) {  // Sanity check - reasonable max offset
        if (m_debugOutput) {
            printf("[DXVK] WARNING: DrawIndexedPrimitive - baseVertexIndex seems unreasonable: %u\n",
                   baseVertexIndex);
        }
    }
    
    return true;
}

// ============================================================================
// Draw Statistics & Debugging
// ============================================================================

/**
 * Track draw call statistics for performance analysis.
 */
void DXVKGraphicsBackend::RecordDrawStatistics(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int vertexCount,
    unsigned int indexCount,
    bool isIndexed) {
    
    // Track per-frame statistics
    if (!isIndexed) {
        m_statsDrawCallsPerFrame++;
        m_statsVerticesPerFrame += vertexCount;
    } else {
        m_statsIndexedDrawCallsPerFrame++;
        m_statsIndicesPerFrame += indexCount;
    }
    
    // Track primitive types
    switch (primitiveType) {
        case D3DPT_TRIANGLELIST:
        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            m_statsTrianglesPerFrame += (isIndexed ? indexCount : vertexCount) / 3;
            break;
        case D3DPT_LINELIST:
        case D3DPT_LINESTRIP:
            m_statsLinesPerFrame += (isIndexed ? indexCount : vertexCount) / 2;
            break;
        case D3DPT_POINTLIST:
            m_statsPointsPerFrame += (isIndexed ? indexCount : vertexCount);
            break;
    }
}

/**
 * Reset draw statistics at frame start.
 */
void DXVKGraphicsBackend::ResetDrawStatistics() {
    m_statsDrawCallsPerFrame = 0;
    m_statsIndexedDrawCallsPerFrame = 0;
    m_statsVerticesPerFrame = 0;
    m_statsIndicesPerFrame = 0;
    m_statsTrianglesPerFrame = 0;
    m_statsLinesPerFrame = 0;
    m_statsPointsPerFrame = 0;
}

/**
 * Report draw statistics for current frame.
 */
void DXVKGraphicsBackend::ReportDrawStatistics() {
    printf("\n[DXVK] === DRAW STATISTICS (Frame %u) ===\n", m_currentFrame);
    printf("  Non-Indexed Draw Calls: %u\n", m_statsDrawCallsPerFrame);
    printf("  Indexed Draw Calls: %u\n", m_statsIndexedDrawCallsPerFrame);
    printf("  Total Draw Calls: %u\n", m_statsDrawCallsPerFrame + m_statsIndexedDrawCallsPerFrame);
    printf("  Vertices Submitted: %u\n", m_statsVerticesPerFrame);
    printf("  Indices Submitted: %u\n", m_statsIndicesPerFrame);
    printf("  Triangles: %u\n", m_statsTrianglesPerFrame);
    printf("  Lines: %u\n", m_statsLinesPerFrame);
    printf("  Points: %u\n", m_statsPointsPerFrame);
    printf("[DXVK] === END STATISTICS ===\n\n");
}

// ============================================================================
// Command Buffer Recording Helpers
// ============================================================================

/**
 * Setup viewport and scissor rect for current command buffer.
 * Called automatically during BeginScene, can be re-called to update.
 */
void DXVKGraphicsBackend::UpdateViewportAndScissor() {
    if (!m_inScene || m_commandBuffers.empty()) {
        return;
    }
    
    // Update viewport
    VkViewport viewport{};
    viewport.x = (float)m_viewport.X;
    viewport.y = (float)m_viewport.Y;
    viewport.width = (float)m_viewport.Width;
    viewport.height = (float)m_viewport.Height;
    viewport.minDepth = m_viewport.MinZ;
    viewport.maxDepth = m_viewport.MaxZ;
    
    vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &viewport);
    
    // Setup scissor to match viewport
    VkRect2D scissor{};
    scissor.offset.x = (int32_t)m_viewport.X;
    scissor.offset.y = (int32_t)m_viewport.Y;
    scissor.extent.width = m_viewport.Width;
    scissor.extent.height = m_viewport.Height;
    
    vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);
}

/**
 * Ensure pipeline is bound to current command buffer.
 * Safe to call multiple times (no-op if already bound).
 */
void DXVKGraphicsBackend::EnsurePipelineBinding() {
    if (m_inScene && m_graphicsPipeline != VK_NULL_HANDLE) {
        vkCmdBindPipeline(
            m_commandBuffers[m_currentFrame],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_graphicsPipeline
        );
    }
}

// ============================================================================
// Query Functions for Draw State
// ============================================================================

/**
 * Get the number of pending draw calls in current frame.
 */
unsigned int DXVKGraphicsBackend::GetPendingDrawCalls() const {
    return m_statsDrawCallsPerFrame + m_statsIndexedDrawCallsPerFrame;
}

/**
 * Check if a primitive type is valid.
 */
bool DXVKGraphicsBackend::IsPrimitiveTypeValid(D3DPRIMITIVETYPE type) {
    switch (type) {
        case D3DPT_POINTLIST:
        case D3DPT_LINELIST:
        case D3DPT_LINESTRIP:
        case D3DPT_TRIANGLELIST:
        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            return true;
        default:
            return false;
    }
}

/**
 * Get total vertices submitted in current frame.
 */
unsigned int DXVKGraphicsBackend::GetFrameVertexCount() const {
    return m_statsVerticesPerFrame;
}

/**
 * Get total indices submitted in current frame.
 */
unsigned int DXVKGraphicsBackend::GetFrameIndexCount() const {
    return m_statsIndicesPerFrame;
}

// End of file
