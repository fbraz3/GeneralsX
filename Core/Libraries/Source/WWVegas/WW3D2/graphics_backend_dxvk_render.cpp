/**
 * @file graphics_backend_dxvk_render.cpp
 * 
 * Vulkan Graphics Backend - Rendering Operations
 * 
 * Implements drawing operations (DrawPrimitive, DrawIndexedPrimitive),
 * render state management, transformations, and texture/light binding.
 * 
 * Phase: 39.2.4 (Drawing Operations Implementation)
 * 
 * Created: October 30, 2025
 */

#include "graphics_backend_dxvk.h"

// ============================================================================
// Drawing Operations
// ============================================================================

/**
 * Draw non-indexed vertices.
 */
HRESULT DXVKGraphicsBackend::DrawPrimitive(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int startVertex,
    unsigned int primitiveCount) {
    
    if (!m_initialized || !m_inScene) {
        return E_FAIL;
    }
    
    if (m_vertexBuffers.empty() || m_graphicsPipeline == VK_NULL_HANDLE) {
        if (m_debugOutput) {
            printf("[DXVK] WARNING: DrawPrimitive - No vertex buffer or pipeline bound\n");
        }
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] DrawPrimitive: type=%d, start=%u, count=%u\n", primitiveType, startVertex, primitiveCount);
    }
    
    // Bind graphics pipeline
    vkCmdBindPipeline(
        m_commandBuffers[m_currentFrame],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_graphicsPipeline
    );
    
    // Bind vertex buffers
    for (const auto& vertexBufPair : m_vertexBuffers) {
        if (vertexBufPair.second && vertexBufPair.second->buffer != VK_NULL_HANDLE) {
            VkBuffer buffers[] = {vertexBufPair.second->buffer};
            VkDeviceSize offsets[] = {0};
            
            vkCmdBindVertexBuffers(
                m_commandBuffers[m_currentFrame],
                vertexBufPair.first,
                1,
                buffers,
                offsets
            );
        }
    }
    
    // Calculate vertex count from primitive count
    unsigned int vertexCount = 0;
    switch (primitiveType) {
        case D3DPT_POINTLIST:
            vertexCount = primitiveCount;
            break;
        case D3DPT_LINELIST:
            vertexCount = primitiveCount * 2;
            break;
        case D3DPT_LINESTRIP:
            vertexCount = primitiveCount + 1;
            break;
        case D3DPT_TRIANGLELIST:
            vertexCount = primitiveCount * 3;
            break;
        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            vertexCount = primitiveCount + 2;
            break;
        default:
            return E_INVALIDARG;
    }
    
    // Draw vertices
    vkCmdDraw(m_commandBuffers[m_currentFrame], vertexCount, 1, startVertex, 0);
    
    if (m_debugOutput) {
        printf("[DXVK] DrawPrimitive: submitted %u vertices\n", vertexCount);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Draw indexed vertices.
 */
HRESULT DXVKGraphicsBackend::DrawIndexedPrimitive(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int baseVertexIndex,
    unsigned int minVertexIndex,
    unsigned int numVertices,
    unsigned int startIndex,
    unsigned int primitiveCount) {
    
    if (!m_initialized || !m_inScene) {
        return E_FAIL;
    }
    
    if (!m_indexBuffer || m_indexBuffer->buffer == VK_NULL_HANDLE) {
        if (m_debugOutput) {
            printf("[DXVK] WARNING: DrawIndexedPrimitive - No index buffer bound\n");
        }
        return E_FAIL;
    }
    
    if (m_vertexBuffers.empty() || m_graphicsPipeline == VK_NULL_HANDLE) {
        if (m_debugOutput) {
            printf("[DXVK] WARNING: DrawIndexedPrimitive - No vertex buffer or pipeline bound\n");
        }
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] DrawIndexedPrimitive: type=%d, base=%u, min=%u, num=%u, start=%u, count=%u\n",
               primitiveType, baseVertexIndex, minVertexIndex, numVertices, startIndex, primitiveCount);
    }
    
    // Bind graphics pipeline
    vkCmdBindPipeline(
        m_commandBuffers[m_currentFrame],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_graphicsPipeline
    );
    
    // Bind vertex buffers
    for (const auto& vertexBufPair : m_vertexBuffers) {
        if (vertexBufPair.second && vertexBufPair.second->buffer != VK_NULL_HANDLE) {
            VkBuffer buffers[] = {vertexBufPair.second->buffer};
            VkDeviceSize offsets[] = {0};
            
            vkCmdBindVertexBuffers(
                m_commandBuffers[m_currentFrame],
                vertexBufPair.first,
                1,
                buffers,
                offsets
            );
        }
    }
    
    // Bind index buffer
    vkCmdBindIndexBuffer(
        m_commandBuffers[m_currentFrame],
        m_indexBuffer->buffer,
        0,
        VK_INDEX_TYPE_UINT32  // Assuming 32-bit indices
    );
    
    // Calculate index count from primitive count
    unsigned int indexCount = 0;
    switch (primitiveType) {
        case D3DPT_POINTLIST:
            indexCount = primitiveCount;
            break;
        case D3DPT_LINELIST:
            indexCount = primitiveCount * 2;
            break;
        case D3DPT_LINESTRIP:
            indexCount = primitiveCount + 1;
            break;
        case D3DPT_TRIANGLELIST:
            indexCount = primitiveCount * 3;
            break;
        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            indexCount = primitiveCount + 2;
            break;
        default:
            return E_INVALIDARG;
    }
    
    // Draw indexed vertices
    vkCmdDrawIndexed(m_commandBuffers[m_currentFrame], indexCount, 1, startIndex, baseVertexIndex, 0);
    
    if (m_debugOutput) {
        printf("[DXVK] DrawIndexedPrimitive: submitted %u indices\n", indexCount);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

// ============================================================================
// Viewport Management
// ============================================================================

/**
 * Set the viewport.
 */
HRESULT DXVKGraphicsBackend::SetViewport(
    unsigned int x,
    unsigned int y,
    unsigned int width,
    unsigned int height,
    float minZ,
    float maxZ) {
    
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] SetViewport: %u,%u %ux%u [%f..%f]\n", x, y, width, height, minZ, maxZ);
    }
    
    // Store viewport
    m_viewport.X = x;
    m_viewport.Y = y;
    m_viewport.Width = width;
    m_viewport.Height = height;
    m_viewport.MinZ = minZ;
    m_viewport.MaxZ = maxZ;
    
    // If we're in scene, update Vulkan viewport immediately
    if (m_inScene) {
        VkViewport viewport{};
        viewport.x = (float)x;
        viewport.y = (float)y;
        viewport.width = (float)width;
        viewport.height = (float)height;
        viewport.minDepth = minZ;
        viewport.maxDepth = maxZ;
        
        vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &viewport);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

// ============================================================================
// Texture Management
// ============================================================================

/**
 * Set active texture.
 */
HRESULT DXVKGraphicsBackend::SetTexture(unsigned int stage, void* texture) {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] SetTexture: stage=%u, texture=%p\n", stage, texture);
    }
    
    if (texture == nullptr) {
        // Unbind texture
        m_textures.erase(stage);
    } else {
        // Bind texture
        auto texHandle = (DXVKTextureHandle*)texture;
        m_textures[stage] = std::make_shared<DXVKTextureHandle>(*texHandle);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Get currently bound texture.
 */
HRESULT DXVKGraphicsBackend::GetTexture(unsigned int stage, void** texture) {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    auto it = m_textures.find(stage);
    if (it != m_textures.end() && it->second) {
        *texture = it->second.get();
        m_lastError = S_OK;
        return S_OK;
    }
    
    *texture = nullptr;
    return E_FAIL;
}

// ============================================================================
// Lighting Management
// ============================================================================

/**
 * Enable/disable lighting.
 */
HRESULT DXVKGraphicsBackend::EnableLighting(bool enable) {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    m_lightingEnabled = enable;
    
    if (m_debugOutput) {
        printf("[DXVK] Lighting: %s\n", enable ? "ENABLED" : "DISABLED");
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Set light source.
 */
HRESULT DXVKGraphicsBackend::SetLight(unsigned int index, const D3DLIGHT8* light) {
    if (!m_initialized || light == nullptr) {
        return E_INVALIDARG;
    }
    
    if (index >= m_lights.size()) {
        m_lights.resize(index + 1);
    }
    
    m_lights[index] = *light;
    
    if (m_debugOutput) {
        printf("[DXVK] SetLight: index=%u, type=%d\n", index, light->Type);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Enable/disable individual light.
 */
HRESULT DXVKGraphicsBackend::LightEnable(unsigned int index, bool enable) {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (index >= m_lights.size()) {
        return E_INVALIDARG;
    }
    
    // In Vulkan, we handle this by managing which lights are active
    // This would normally update a shader uniform
    
    if (m_debugOutput) {
        printf("[DXVK] LightEnable: index=%u, %s\n", index, enable ? "ON" : "OFF");
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Set material properties.
 */
HRESULT DXVKGraphicsBackend::SetMaterial(const D3DMATERIAL8* material) {
    if (!m_initialized || material == nullptr) {
        return E_INVALIDARG;
    }
    
    m_material = *material;
    
    if (m_debugOutput) {
        printf("[DXVK] SetMaterial: ambient=(%f,%f,%f), diffuse=(%f,%f,%f), specular=(%f,%f,%f), power=%f\n",
               material->Ambient.r, material->Ambient.g, material->Ambient.b,
               material->Diffuse.r, material->Diffuse.g, material->Diffuse.b,
               material->Specular.r, material->Specular.g, material->Specular.b,
               material->Power);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Set ambient light color.
 */
HRESULT DXVKGraphicsBackend::SetAmbient(D3DCOLOR color) {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    m_ambientColor = color;
    
    if (m_debugOutput) {
        printf("[DXVK] SetAmbient: 0x%08X\n", color);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

// ============================================================================
// Buffer Management
// ============================================================================

/**
 * Set active vertex buffer stream.
 */
HRESULT DXVKGraphicsBackend::SetStreamSource(
    unsigned int stream,
    void* buffer,
    unsigned int stride) {
    
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] SetStreamSource: stream=%u, buffer=%p, stride=%u\n", stream, buffer, stride);
    }
    
    if (buffer == nullptr) {
        m_vertexBuffers.erase(stream);
    } else {
        auto bufHandle = (DXVKBufferHandle*)buffer;
        m_vertexBuffers[stream] = std::make_shared<DXVKBufferHandle>(*bufHandle);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Set active index buffer.
 */
HRESULT DXVKGraphicsBackend::SetIndices(void* buffer) {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] SetIndices: buffer=%p\n", buffer);
    }
    
    if (buffer == nullptr) {
        m_indexBuffer = nullptr;
    } else {
        auto bufHandle = (DXVKBufferHandle*)buffer;
        m_indexBuffer = std::make_shared<DXVKBufferHandle>(*bufHandle);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

// ============================================================================
// State Management
// ============================================================================

/**
 * Set render state.
 */
HRESULT DXVKGraphicsBackend::SetRenderState(D3DRENDERSTATETYPE state, DWORD value) {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    // Store render state
    m_renderStates[state] = value;
    
    if (m_debugOutput) {
        printf("[DXVK] SetRenderState: state=%u, value=%u\n", state, value);
    }
    
    // In Phase 39.2.4, these states are stored but not yet applied to Vulkan pipeline
    // Phase 39.2.5+ will implement actual pipeline state updates
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Get render state.
 */
HRESULT DXVKGraphicsBackend::GetRenderState(D3DRENDERSTATETYPE state, DWORD* value) {
    if (!m_initialized || value == nullptr) {
        return E_INVALIDARG;
    }
    
    auto it = m_renderStates.find(state);
    if (it != m_renderStates.end()) {
        *value = it->second;
        m_lastError = S_OK;
        return S_OK;
    }
    
    m_lastError = E_FAIL;
    return E_FAIL;
}

/**
 * Set texture operation (blend mode).
 */
HRESULT DXVKGraphicsBackend::SetTextureOp(unsigned int stage, D3DTEXTUREOP operation) {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] SetTextureOp: stage=%u, operation=%u\n", stage, operation);
    }
    
    // Store texture operation for later processing
    // Will be applied to texture descriptor sets in Phase 39.2.5+
    
    m_lastError = S_OK;
    return S_OK;
}
