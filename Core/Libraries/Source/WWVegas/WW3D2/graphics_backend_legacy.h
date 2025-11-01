/**
 * Legacy Graphics Backend Wrapper
 * 
 * Implements IGraphicsBackend by delegating to existing Phase 27-37 code.
 * This enables the abstraction layer while keeping all existing graphics
 * code completely intact and unchanged.
 * 
 * Used when: USE_DXVK=OFF (default)
 * Delegates to: DX8Wrapper (DirectX mock) and MetalWrapper (Metal rendering)
 */

#pragma once

#ifndef GRAPHICS_BACKEND_LEGACY_H
#define GRAPHICS_BACKEND_LEGACY_H

#include "graphics_backend.h"

/**
 * Legacy graphics backend implementation.
 * 
 * Wraps existing Phase 27-37 graphics code (Metal/OpenGL).
 * All methods delegate to existing DX8Wrapper and MetalWrapper functions.
 * 
 * This is a pure delegation layer - no logic changes, just routing calls
 * through the abstraction interface.
 */
class LegacyGraphicsBackend : public IGraphicsBackend {
public:
    LegacyGraphicsBackend();
    virtual ~LegacyGraphicsBackend();
    
    // ========================================================================
    // Window Handle Management
    // ========================================================================
    
    void SetWindowHandle(HWND handle) override;
    
    // ========================================================================
    // Device Management
    // ========================================================================
    
    HRESULT Initialize() override;
    HRESULT Shutdown() override;
    HRESULT Reset() override;
    
    // ========================================================================
    // Scene Operations
    // ========================================================================
    
    HRESULT BeginScene() override;
    HRESULT EndScene() override;
    HRESULT Present() override;
    
    HRESULT Clear(
        bool clear_color,
        bool clear_z_stencil,
        const void* color_vec3,
        float z = 1.0f,
        DWORD stencil = 0
    ) override;
    
    // ========================================================================
    // Texture Management
    // ========================================================================
    
    HRESULT SetTexture(unsigned int stage, void* texture) override;
    HRESULT GetTexture(unsigned int stage, void** texture) override;
    
    HRESULT CreateTexture(
        unsigned int width,
        unsigned int height,
        D3DFORMAT format,
        void** texture
    ) override;
    
    HRESULT ReleaseTexture(void* texture) override;
    HRESULT LockTexture(void* texture, void** data, unsigned int* pitch) override;
    HRESULT UnlockTexture(void* texture) override;
    
    // ========================================================================
    // Render State Management
    // ========================================================================
    
    HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override;
    HRESULT GetRenderState(D3DRENDERSTATETYPE state, DWORD* value) override;
    HRESULT SetTextureOp(unsigned int stage, D3DTEXTUREOP operation) override;
    
    // ========================================================================
    // Vertex/Index Buffers
    // ========================================================================
    
    HRESULT CreateVertexBuffer(
        unsigned int size,
        unsigned int usage,
        unsigned int format,
        void** buffer
    ) override;
    
    HRESULT ReleaseVertexBuffer(void* buffer) override;
    HRESULT LockVertexBuffer(void* buffer, void** data, unsigned int flags) override;
    HRESULT UnlockVertexBuffer(void* buffer) override;
    
    HRESULT CreateIndexBuffer(
        unsigned int size,
        D3DFORMAT format,
        void** buffer
    ) override;
    
    HRESULT ReleaseIndexBuffer(void* buffer) override;
    HRESULT LockIndexBuffer(void* buffer, void** data, unsigned int flags) override;
    HRESULT UnlockIndexBuffer(void* buffer) override;
    
    HRESULT SetStreamSource(
        unsigned int stream,
        void* buffer,
        unsigned int stride
    ) override;
    
    HRESULT SetIndices(void* buffer) override;
    
    // ========================================================================
    // Drawing Operations
    // ========================================================================
    
    HRESULT DrawPrimitive(
        D3DPRIMITIVETYPE primitiveType,
        unsigned int startVertex,
        unsigned int primitiveCount
    ) override;
    
    HRESULT DrawIndexedPrimitive(
        D3DPRIMITIVETYPE primitiveType,
        unsigned int baseVertexIndex,
        unsigned int minVertexIndex,
        unsigned int numVertices,
        unsigned int startIndex,
        unsigned int primitiveCount
    ) override;
    
    // ========================================================================
    // Viewport and Transform
    // ========================================================================
    
    HRESULT SetViewport(
        unsigned int x,
        unsigned int y,
        unsigned int width,
        unsigned int height,
        float minZ,
        float maxZ
    ) override;
    
    HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const float* matrix) override;
    HRESULT GetTransform(D3DTRANSFORMSTATETYPE state, float* matrix) override;
    
    // ========================================================================
    // Lighting
    // ========================================================================
    
    HRESULT EnableLighting(bool enable) override;
    HRESULT SetLight(unsigned int index, const D3DLIGHT8* light) override;
    HRESULT LightEnable(unsigned int index, bool enable) override;
    HRESULT SetMaterial(const D3DMATERIAL8* material) override;
    HRESULT SetAmbient(D3DCOLOR color) override;
    
    // ========================================================================
    // Utility / Debug
    // ========================================================================
    
    const char* GetBackendName() const override;
    HRESULT GetLastError() const override;
    void SetDebugOutput(bool enable) override;
    
private:
    bool m_initialized;
    HRESULT m_lastError;
    bool m_debugOutput;
};

#endif // GRAPHICS_BACKEND_LEGACY_H
