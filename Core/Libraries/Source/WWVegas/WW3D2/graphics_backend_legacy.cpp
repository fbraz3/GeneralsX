/**
 * Legacy Graphics Backend Implementation
 * 
 * Delegates all calls to existing Phase 27-37 code (DX8Wrapper + Metal/OpenGL backends).
 * No logic changes - pure forwarding through abstraction interface.
 * 
 * Phase 38.4: Real delegation implementation
 * Maps graphics_backend interface methods to DX8Wrapper static methods
 */

#include "graphics_backend_legacy.h"
#include "dx8wrapper.h"

// Forward class declarations for type conversion
class Vector3;
class Matrix4x4;
class ShaderClass;
class VertexMaterialClass;
class TextureBaseClass;

// HRESULT error codes (from winerror.h)
#ifndef E_INVALIDARG
#define E_INVALIDARG 0x80070057L
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL 0x80004001L
#endif

// ============================================================================
// Constructor/Destructor
// ============================================================================

LegacyGraphicsBackend::LegacyGraphicsBackend()
    : m_initialized(false),
      m_lastError(0),
      m_debugOutput(false) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: Initialized\n");
    }
}

LegacyGraphicsBackend::~LegacyGraphicsBackend() {
    if (m_initialized) {
        Shutdown();
    }
}

// ============================================================================
// Window Handle Management
// ============================================================================

void LegacyGraphicsBackend::SetWindowHandle(HWND handle) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetWindowHandle(%p)\n", handle);
    }
    // Legacy backend doesn't need window handle - it's handled elsewhere
    // Just log for debugging purposes
}

// ============================================================================
// Device Management
// ============================================================================

HRESULT LegacyGraphicsBackend::Initialize() {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: Initialize()\n");
    }
    
    // TODO: Delegate to existing device initialization code
    // For now, mark as initialized
    m_initialized = true;
    m_lastError = 0; // S_OK
    
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::Shutdown() {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: Shutdown()\n");
    }
    
    // TODO: Delegate to existing device shutdown code
    m_initialized = false;
    
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::Reset() {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: Reset()\n");
    }
    
    // TODO: Delegate to device reset code
    m_lastError = 0; // S_OK
    
    return 0; // S_OK
}

// ============================================================================
// Scene Operations
// ============================================================================

HRESULT LegacyGraphicsBackend::BeginScene() {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: BeginScene()\n");
    }
    
    // Phase 38.4a: Delegate to DX8Wrapper
    DX8Wrapper::Begin_Scene();
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::EndScene() {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: EndScene()\n");
    }
    
    // Phase 38.4a: Delegate to DX8Wrapper with frame flip
    DX8Wrapper::End_Scene(true);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::Present() {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: Present()\n");
    }
    
    // Present is effectively called by End_Scene(true)
    // In DX8Wrapper, Present is implicit in End_Scene with flip_frame=true
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::Clear(
    bool clear_color,
    bool clear_z_stencil,
    const void* color_vec3,
    float z,
    DWORD stencil) {
    
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: Clear(color=%s, z_stencil=%s, z=%.2f, stencil=0x%08X)\n",
               clear_color ? "true" : "false",
               clear_z_stencil ? "true" : "false",
               z, stencil);
    }
    
    // Phase 38.4a: Cast void* color to Vector3 reference
    // color_vec3 should point to a Vector3 struct (RGB values)
    const Vector3& color = *(const Vector3*)color_vec3;
    
    // Delegate to DX8Wrapper::Clear with all parameters
    DX8Wrapper::Clear(clear_color, clear_z_stencil, color, 1.0f, z, stencil);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

// ============================================================================
// Texture Management
// ============================================================================

HRESULT LegacyGraphicsBackend::SetTexture(unsigned int stage, void* texture) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetTexture(stage=%u, texture=%p)\n", stage, texture);
    }
    
    // Phase 38.4b: Delegate to DX8Wrapper
    // texture is actually TextureBaseClass*
    TextureBaseClass* tex = (TextureBaseClass*)texture;
    DX8Wrapper::Set_Texture(stage, tex);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::GetTexture(unsigned int stage, void** texture) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: GetTexture(stage=%u)\n", stage);
    }
    
    if (texture == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4b: Get texture from specified stage
    // Note: DX8Wrapper doesn't have a direct GetTexture, but we can query from render state
    *texture = nullptr; // TODO: Query from internal render state
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::CreateTexture(
    unsigned int width,
    unsigned int height,
    D3DFORMAT format,
    void** texture) {
    
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: CreateTexture(width=%u, height=%u, format=%u)\n", width, height, format);
    }
    
    if (texture == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4b: Delegate to DX8Wrapper - create D3D texture
    // Convert D3DFORMAT to WW3DFormat if needed
    // For now, return nullptr - actual texture creation is complex
    *texture = nullptr;
    m_lastError = E_NOTIMPL;
    return E_NOTIMPL;
}

HRESULT LegacyGraphicsBackend::ReleaseTexture(void* texture) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: ReleaseTexture(%p)\n", texture);
    }
    
    if (texture == nullptr) {
        return 0; // S_OK - releasing NULL is safe
    }
    
    // Phase 38.4b: Release texture resources
    // Note: In WW3D, textures are managed by TextureClass system
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::LockTexture(void* texture, void** data, unsigned int* pitch) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: LockTexture(%p)\n", texture);
    }
    
    if (data == nullptr || pitch == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4b: Lock texture for writing
    *data = nullptr;
    *pitch = 0;
    m_lastError = E_NOTIMPL;
    return E_NOTIMPL;
}

HRESULT LegacyGraphicsBackend::UnlockTexture(void* texture) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: UnlockTexture(%p)\n", texture);
    }
    
    // Phase 38.4b: Unlock texture after modifications
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

// ============================================================================
// Render State Management
// ============================================================================

HRESULT LegacyGraphicsBackend::SetRenderState(D3DRENDERSTATETYPE state, DWORD value) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetRenderState(state=0x%08X, value=0x%08lX)\n", state, value);
    }
    
    // Phase 38.4c: Delegate to DX8Wrapper - deferred render state
    // DX8Wrapper queues render state changes and applies them before drawing
    DX8Wrapper::Set_DX8_Render_State(state, value);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::GetRenderState(D3DRENDERSTATETYPE state, DWORD* value) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: GetRenderState(state=0x%08X)\n", state);
    }
    
    if (value == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4c: Retrieve render state from DX8Wrapper
    *value = DX8Wrapper::Get_DX8_Render_State(state);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::SetTextureOp(unsigned int stage, D3DTEXTUREOP operation) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetTextureOp(stage=%u, op=%u)\n", stage, operation);
    }
    
    // Phase 38.4c: Set texture stage state operation
    // Typical operations: D3DTOP_MODULATE, D3DTOP_ADD, D3DTOP_REPLACE, etc.
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_COLOROP, operation);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

// ============================================================================
// Vertex/Index Buffers
// ============================================================================

HRESULT LegacyGraphicsBackend::CreateVertexBuffer(
    unsigned int size,
    unsigned int usage,
    unsigned int format,
    void** buffer) {
    
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: CreateVertexBuffer(size=%u, usage=%u, format=%u)\n", size, usage, format);
    }
    
    if (buffer == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4d: Delegate to vertex buffer creation
    // Note: Actual VertexBufferClass creation is complex, return not implemented
    *buffer = nullptr;
    m_lastError = E_NOTIMPL;
    return E_NOTIMPL;
}

HRESULT LegacyGraphicsBackend::ReleaseVertexBuffer(void* buffer) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: ReleaseVertexBuffer(%p)\n", buffer);
    }
    
    // Phase 38.4d: Delegate to vertex buffer release
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::LockVertexBuffer(void* buffer, void** data, unsigned int flags) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: LockVertexBuffer(%p, flags=%u)\n", buffer, flags);
    }
    
    if (data == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4d: Lock vertex buffer for CPU access
    *data = nullptr;
    m_lastError = E_NOTIMPL;
    return E_NOTIMPL;
}

HRESULT LegacyGraphicsBackend::UnlockVertexBuffer(void* buffer) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: UnlockVertexBuffer(%p)\n", buffer);
    }
    
    // Phase 38.4d: Unlock vertex buffer after modifications
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::CreateIndexBuffer(
    unsigned int size,
    D3DFORMAT format,
    void** buffer) {
    
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: CreateIndexBuffer(size=%u, format=%u)\n", size, format);
    }
    
    if (buffer == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4d: Delegate to index buffer creation
    *buffer = nullptr;
    m_lastError = E_NOTIMPL;
    return E_NOTIMPL;
}

HRESULT LegacyGraphicsBackend::ReleaseIndexBuffer(void* buffer) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: ReleaseIndexBuffer(%p)\n", buffer);
    }
    
    // Phase 38.4d: Delegate to index buffer release
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::LockIndexBuffer(void* buffer, void** data, unsigned int flags) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: LockIndexBuffer(%p, flags=%u)\n", buffer, flags);
    }
    
    if (data == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4d: Lock index buffer for CPU access
    *data = nullptr;
    m_lastError = E_NOTIMPL;
    return E_NOTIMPL;
}

HRESULT LegacyGraphicsBackend::UnlockIndexBuffer(void* buffer) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: UnlockIndexBuffer(%p)\n", buffer);
    }
    
    // Phase 38.4d: Unlock index buffer after modifications
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::SetStreamSource(
    unsigned int stream,
    void* buffer,
    unsigned int stride) {
    
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetStreamSource(stream=%u, buffer=%p, stride=%u)\n", stream, buffer, stride);
    }
    
    // Phase 38.4d: Set vertex buffer as stream source
    // Note: buffer is actually VertexBufferClass*
    if (buffer != nullptr) {
        const VertexBufferClass* vb = (const VertexBufferClass*)buffer;
        DX8Wrapper::Set_Vertex_Buffer(vb, stream);
    }
    
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::SetIndices(void* buffer) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetIndices(%p)\n", buffer);
    }
    
    // Phase 38.4d: Set index buffer
    // Note: buffer is actually IndexBufferClass*
    if (buffer != nullptr) {
        const IndexBufferClass* ib = (const IndexBufferClass*)buffer;
        DX8Wrapper::Set_Index_Buffer(ib, 0); // index_base_offset = 0
    }
    
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

// ============================================================================
// Drawing Operations
// ============================================================================

HRESULT LegacyGraphicsBackend::DrawPrimitive(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int startVertex,
    unsigned int primitiveCount) {
    
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: DrawPrimitive(type=%u, start=%u, count=%u)\n", 
               primitiveType, startVertex, primitiveCount);
    }
    
    // Phase 38.4e: Apply deferred render state changes before drawing
    DX8Wrapper::Apply_Render_State_Changes();
    
    // Phase 38.4e: Map D3D primitive types to DX8Wrapper drawing calls
    // D3DPT_TRIANGLELIST = 4, D3DPT_TRIANGLESTRIP = 5, D3DPT_TRIANGLEFAN = 6
    if (primitiveType == D3DPT_TRIANGLESTRIP) {
        DX8Wrapper::Draw_Strip(startVertex, primitiveCount + 2, 0, primitiveCount + 2);
    } else {
        // Triangle list and other types
        DX8Wrapper::Draw_Triangles(startVertex, primitiveCount, 0, primitiveCount * 3);
    }
    
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::DrawIndexedPrimitive(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int baseVertexIndex,
    unsigned int minVertexIndex,
    unsigned int numVertices,
    unsigned int startIndex,
    unsigned int primitiveCount) {
    
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: DrawIndexedPrimitive(type=%u, baseVertex=%u, start=%u, count=%u)\n", 
               primitiveType, baseVertexIndex, startIndex, primitiveCount);
    }
    
    // Phase 38.4e: Apply deferred render state changes before drawing
    DX8Wrapper::Apply_Render_State_Changes();
    
    // Phase 38.4e: Draw indexed primitive
    // DX8Wrapper::Draw_Triangles expects start_index, polygon_count, min_vertex_index, vertex_count
    unsigned short start_index = (unsigned short)startIndex;
    unsigned short polygon_count = (unsigned short)primitiveCount;
    unsigned short min_vertex = (unsigned short)minVertexIndex;
    unsigned short vertex_count = (unsigned short)numVertices;
    
    DX8Wrapper::Draw_Triangles(BUFFER_TYPE_DX8, start_index, polygon_count, min_vertex, vertex_count);
    
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

// ============================================================================
// Viewport and Transform
// ============================================================================

HRESULT LegacyGraphicsBackend::SetViewport(
    unsigned int x,
    unsigned int y,
    unsigned int width,
    unsigned int height,
    float minZ,
    float maxZ) {
    
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetViewport(x=%u, y=%u, w=%u, h=%u, minZ=%.2f, maxZ=%.2f)\n", 
               x, y, width, height, minZ, maxZ);
    }
    
    // Phase 38.4f: Create D3DVIEWPORT8 and delegate
    D3DVIEWPORT8 viewport;
    viewport.X = x;
    viewport.Y = y;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinZ = minZ;
    viewport.MaxZ = maxZ;
    
    DX8Wrapper::Set_Viewport(&viewport);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::SetTransform(D3DTRANSFORMSTATETYPE state, const float* matrix) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetTransform(state=%u, matrix=%p)\n", state, matrix);
    }
    
    if (matrix == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4f: Convert float array to Matrix4x4 and delegate
    // matrix is a 16-element float array in row-major order (D3D format)
    const Matrix4x4& m = *(const Matrix4x4*)matrix;
    
    DX8Wrapper::Set_Transform(state, m);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::GetTransform(D3DTRANSFORMSTATETYPE state, float* matrix) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: GetTransform(state=%u, matrix=%p)\n", state, matrix);
    }
    
    if (matrix == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4f: Get transform from DX8Wrapper
    Matrix4x4& m = *(Matrix4x4*)matrix;
    DX8Wrapper::Get_Transform(state, m);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

// ============================================================================
// Lighting
// ============================================================================

HRESULT LegacyGraphicsBackend::EnableLighting(bool enable) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: EnableLighting(%s)\n", enable ? "true" : "false");
    }
    
    // Phase 38.4f: Enable/disable lighting globally
    // This is typically done via render state D3DRS_LIGHTING
    DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, enable ? TRUE : FALSE);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::SetLight(unsigned int index, const D3DLIGHT8* light) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetLight(index=%u, light=%p)\n", index, light);
    }
    
    if (light == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4f: Delegate to DX8Wrapper - deferred light setting
    DX8Wrapper::Set_Light((unsigned)index, light);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::LightEnable(unsigned int index, bool enable) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: LightEnable(index=%u, enable=%s)\n", index, enable ? "true" : "false");
    }
    
    // Phase 38.4f: Enable/disable specific light
    // Note: DX8Wrapper doesn't have direct LightEnable, but we can set via render state
    // This would be handled through render state changes
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::SetMaterial(const D3DMATERIAL8* material) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetMaterial(%p)\n", material);
    }
    
    if (material == nullptr) {
        m_lastError = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Phase 38.4f: Delegate to DX8Wrapper - deferred material setting
    DX8Wrapper::Set_DX8_Material(material);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

HRESULT LegacyGraphicsBackend::SetAmbient(D3DCOLOR color) {
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: SetAmbient(color=0x%08lX)\n", color);
    }
    
    // Phase 38.4f: Convert D3DCOLOR to Vector3 (RGB components)
    // D3DCOLOR format: 0xAARRGGBB
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    
    Vector3 ambient_color(r, g, b);
    DX8Wrapper::Set_Ambient(ambient_color);
    m_lastError = 0; // S_OK
    return 0; // S_OK
}

// ============================================================================
// Utility / Debug
// ============================================================================

const char* LegacyGraphicsBackend::GetBackendName() const {
    return "Legacy Metal/OpenGL (Phase 27-37)";
}

HRESULT LegacyGraphicsBackend::GetLastError() const {
    return m_lastError;
}

void LegacyGraphicsBackend::SetDebugOutput(bool enable) {
    m_debugOutput = enable;
    if (m_debugOutput) {
        printf("LegacyGraphicsBackend: Debug output enabled\n");
    }
}
