# Phase 28.4.1: TexturedQuad Helper Function - Complete ✅

**Status**: COMPLETE (January 14, 2025)  
**Compilation**: ✅ 0 errors, 129 warnings (non-critical)  
**Files Created**: 2 new files, 6 modified files

---

## Overview

Implemented complete `TexturedQuad` class for rendering 2D textured quads with Metal backend. This provides a simple API for UI elements, menu backgrounds, and 2D sprites.

### Key Features

- **Simple API**: SetTexture(), SetPosition(), SetColor(), SetUVs(), Render()
- **TextureCache Integration**: Automatic reference counting and texture management
- **GPU Buffer Management**: Vertex/index buffers with dynamic updates
- **Metal Backend**: Full DrawIndexedPrimitive implementation
- **UV Mapping Support**: Customizable UVs for texture atlases

---

## Files Created

### 1. `texturedquad.h` (175 lines)

**Purpose**: TexturedQuad class declaration and vertex structure

**Key Components**:

```cpp
namespace GX {

// Vertex structure matching shader input
struct TexturedVertex {
    float position[3];  // X, Y, Z
    float normal[3];    // Normal (unused for 2D)
    float color[4];     // RGBA (0-1 range)
    float texcoord[2];  // U, V
};

class TexturedQuad {
public:
    TexturedQuad();
    ~TexturedQuad();
    
    // Texture management
    bool SetTexture(const char* texture_path);
    
    // Positioning
    void SetPosition(float x, float y, float width, float height);
    
    // Color tint
    void SetColor(float r, float g, float b, float a = 1.0f);
    
    // Custom UVs for atlases
    void SetUVs(float u0, float v0, float u1, float v1);
    
    // Rendering
    void Render();
    
    // Accessors
    void* GetTexture() const { return m_texture; }
    bool HasTexture() const { return m_texture != nullptr; }
    
private:
    void UpdateVertices();
    void EnsureBuffersCreated();
    
    // Members
    void* m_texture;            // Metal texture handle
    std::string m_texture_path; // Path for cache
    TexturedVertex m_vertices[4]; // Quad vertices
    unsigned short m_indices[6];  // Index buffer
    float m_x, m_y, m_width, m_height; // Position/size
    float m_u0, m_v0, m_u1, m_v1;      // UV coords
    float m_color[4];                   // RGBA tint
    void* m_vertex_buffer;              // GPU buffer
    void* m_index_buffer;               // GPU buffer
    bool m_buffers_created;
    bool m_vertices_dirty;
};

} // namespace GX
```

**Design Patterns**:
- **Lazy Initialization**: GPU buffers created on first render
- **Dirty Flag**: Vertices updated only when changed
- **RAII**: Automatic texture release in destructor

---

### 2. `texturedquad.cpp` (253 lines)

**Purpose**: TexturedQuad class implementation

**Key Methods**:

#### `SetTexture(const char* texture_path)`
```cpp
bool TexturedQuad::SetTexture(const char* texture_path) {
    // Release previous texture
    if (!m_texture_path.empty()) {
        TextureCache::GetInstance()->ReleaseTexture(m_texture_path.c_str());
    }
    
    // Load new texture with reference counting
    m_texture = TextureCache::GetInstance()->LoadTexture(texture_path);
    m_texture_path = texture_path;
    return m_texture != nullptr;
}
```

#### `SetPosition(float x, float y, float width, float height)`
```cpp
void TexturedQuad::SetPosition(float x, float y, float width, float height) {
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
    m_vertices_dirty = true; // Mark for update
}
```

#### `UpdateVertices()`
```cpp
void TexturedQuad::UpdateVertices() {
    // Vertex 0: Top-left (x, y) → UV (u0, v0)
    // Vertex 1: Top-right (x+width, y) → UV (u1, v0)
    // Vertex 2: Bottom-right (x+width, y+height) → UV (u1, v1)
    // Vertex 3: Bottom-left (x, y+height) → UV (u0, v1)
    
    // Set position, texcoord, normal, color for all 4 vertices
    m_vertices_dirty = false;
}
```

#### `EnsureBuffersCreated()`
```cpp
void TexturedQuad::EnsureBuffersCreated() {
    if (m_buffers_created) return;
    
    // Create vertex buffer (4 vertices * sizeof(TexturedVertex))
    m_vertex_buffer = MetalWrapper::CreateVertexBuffer(vertex_size, m_vertices);
    
    // Create index buffer (6 indices: 0-1-2, 2-3-0)
    m_index_buffer = MetalWrapper::CreateIndexBuffer(index_size, m_indices);
    
    m_buffers_created = true;
}
```

#### `Render()`
```cpp
void TexturedQuad::Render() {
    // Update vertices if dirty
    if (m_vertices_dirty) UpdateVertices();
    
    // Create GPU buffers if needed
    if (!m_buffers_created) EnsureBuffersCreated();
    
    // Update vertex buffer
    MetalWrapper::UpdateVertexBuffer(m_vertex_buffer, m_vertices, vertex_size);
    
    // Bind texture
    MetalWrapper::BindTexture(m_texture, 0);
    
    // Set vertex/index buffers
    MetalWrapper::SetVertexBuffer(m_vertex_buffer, 0, 0);
    MetalWrapper::SetIndexBuffer(m_index_buffer, 0);
    
    // Draw 2 triangles (6 indices)
    MetalWrapper::DrawIndexedPrimitive(
        4,  // D3DPT_TRIANGLELIST
        0,  // base_vertex_index
        0,  // min_vertex
        4,  // num_vertices
        0,  // start_index
        2   // primitive_count
    );
    
    // Unbind texture
    MetalWrapper::UnbindTexture(0);
}
```

---

## Files Modified

### 3. `metalwrapper.h` - Updated Buffer & Draw APIs

**Changes**:
```cpp
// Buffer Management (void* instead of id)
static void* CreateVertexBuffer(unsigned int size, const void* data, bool dynamic = false);
static void* CreateIndexBuffer(unsigned int size, const void* data, bool dynamic = false);
static void DeleteVertexBuffer(void* buffer);
static void DeleteIndexBuffer(void* buffer);
static void UpdateVertexBuffer(void* buffer, const void* data, unsigned int size, unsigned int offset = 0);
static void UpdateIndexBuffer(void* buffer, const void* data, unsigned int size, unsigned int offset = 0);

// Buffer Binding
static void SetVertexBuffer(void* buffer, unsigned int offset, unsigned int slot);
static void SetIndexBuffer(void* buffer, unsigned int offset);

// Draw Calls
static void DrawPrimitive(unsigned int primitiveType, unsigned int startVertex, unsigned int vertexCount);
static void DrawIndexedPrimitive(unsigned int primitiveType, int baseVertexIndex,
                                 unsigned int minVertex, unsigned int numVertices,
                                 unsigned int startIndex, unsigned int primitiveCount);

// Texture Binding (void* instead of id)
static void BindTexture(void* texture, unsigned int slot = 0);
```

**Rationale**: Changed `id` to `void*` for C++ compatibility (avoid Objective-C types in .cpp files)

---

### 4. `metalwrapper.mm` - Implemented Buffer & Draw APIs

**New Implementations**:

#### Buffer Creation (220-270)
```objectivec++
void* MetalWrapper::CreateVertexBuffer(unsigned int size, const void* data, bool dynamic) {
    id<MTLBuffer> buffer = [(id<MTLDevice>)s_device newBufferWithBytes:data 
        length:size options:MTLResourceStorageModeShared];
    return (__bridge_retained void*)buffer;
}
```

#### Buffer Binding (318-348)
```objectivec++
static void* s_currentVertexBuffer = nullptr;
static void* s_currentIndexBuffer = nullptr;

void MetalWrapper::SetVertexBuffer(void* buffer, unsigned int offset, unsigned int slot) {
    id<MTLBuffer> mtlBuffer = (__bridge id<MTLBuffer>)buffer;
    [(id<MTLRenderCommandEncoder>)s_renderEncoder 
        setVertexBuffer:mtlBuffer offset:offset atIndex:slot];
}

void MetalWrapper::SetIndexBuffer(void* buffer, unsigned int offset) {
    s_currentIndexBuffer = buffer;
    s_currentIndexBufferOffset = offset;
}
```

#### Draw Indexed Primitives (389-439)
```objectivec++
void MetalWrapper::DrawIndexedPrimitive(unsigned int primitiveType, int baseVertexIndex,
                                        unsigned int minVertex, unsigned int numVertices,
                                        unsigned int startIndex, unsigned int primitiveCount) {
    // Map D3D primitive type to Metal
    MTLPrimitiveType metalPrimitiveType = MTLPrimitiveTypeTriangle;
    unsigned int indicesPerPrimitive = 3;
    
    switch (primitiveType) {
        case 4: // D3DPT_TRIANGLELIST
            metalPrimitiveType = MTLPrimitiveTypeTriangle;
            indicesPerPrimitive = 3;
            break;
        // ... other primitive types ...
    }
    
    unsigned int indexCount = primitiveCount * indicesPerPrimitive;
    id<MTLBuffer> mtlIndexBuffer = (__bridge id<MTLBuffer>)s_currentIndexBuffer;
    
    [(id<MTLRenderCommandEncoder>)s_renderEncoder 
        drawIndexedPrimitives:metalPrimitiveType
        indexCount:indexCount
        indexType:MTLIndexTypeUInt16
        indexBuffer:mtlIndexBuffer
        indexBufferOffset:indexOffset * sizeof(unsigned short)
        instanceCount:1
        baseVertex:baseVertexIndex
        baseInstance:0];
}
```

---

### 5. `dx8vertexbuffer.h/cpp` - Changed `id` to `void*`

**Changes**:
```cpp
// dx8vertexbuffer.h (line 243, 273)
void* Get_Metal_Vertex_Buffer() { return MetalVertexBuffer; }
void* MetalVertexBuffer;  // MTLBuffer stored as void* (opaque pointer)

// dx8vertexbuffer.cpp (line 262, 705, 715)
MetalWrapper::UpdateVertexBuffer(vb->Get_Metal_Vertex_Buffer(), vb->MetalVertexData, buffer_size, 0);
MetalVertexBuffer = MetalWrapper::CreateVertexBuffer(buffer_size, nullptr, is_dynamic);
MetalWrapper::DeleteVertexBuffer(MetalVertexBuffer);
```

**Rationale**: Avoid mixing Objective-C types (`id`) with C++ code

---

### 6. `dx8indexbuffer.h/cpp` - Changed `id` to `void*`

**Changes**:
```cpp
// dx8indexbuffer.h (line 198, 218)
void* Get_Metal_Index_Buffer() { return MetalIndexBuffer; }
void* MetalIndexBuffer;  // MTLBuffer stored as void* (opaque pointer)

// dx8indexbuffer.cpp (line 264, 455, 465)
MetalWrapper::UpdateIndexBuffer(ib->Get_Metal_Index_Buffer(), ib->MetalIndexData, buffer_size, 0);
MetalIndexBuffer = MetalWrapper::CreateIndexBuffer(buffer_size, nullptr, is_dynamic);
MetalWrapper::DeleteIndexBuffer(MetalIndexBuffer);
```

---

### 7. `CMakeLists.txt` - Added texturedquad to build

**Change**:
```cmake
# Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt (line 67-74)
ddsloader.cpp
ddsloader.h
tgaloader.cpp
tgaloader.h
texturecache.cpp
texturecache.h
texturedquad.cpp    # ← NEW
texturedquad.h      # ← NEW
dynamesh.cpp
```

---

## API Usage Example

```cpp
#include "texturedquad.h"

// Create quad
GX::TexturedQuad backgroundQuad;

// Load texture (DDS or TGA)
backgroundQuad.SetTexture("Data/Textures/menu_bg.dds");

// Set position (screen coordinates)
backgroundQuad.SetPosition(0, 0, 1024, 768);

// Optional: Set color tint
backgroundQuad.SetColor(1.0f, 1.0f, 1.0f, 0.8f); // 80% opacity

// Optional: Custom UVs for atlas
backgroundQuad.SetUVs(0.0f, 0.0f, 0.5f, 0.5f); // Top-left quarter

// Render within BeginFrame/EndFrame
MetalWrapper::BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);
backgroundQuad.Render();
MetalWrapper::EndFrame();
```

---

## Technical Implementation Details

### Vertex Structure Alignment

```cpp
struct TexturedVertex {
    float position[3];  // 12 bytes, offset 0
    float normal[3];    // 12 bytes, offset 12
    float color[4];     // 16 bytes, offset 24
    float texcoord[2];  // 8 bytes, offset 40
};  // Total: 48 bytes per vertex
```

**Shader Compatibility**: Matches `VertexInput` structure in `basic.metal`:
```metal
struct VertexInput {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float4 color    [[attribute(2)]];
    float2 texCoord [[attribute(3)]];
};
```

### Index Buffer Layout

```cpp
// Two triangles forming a quad
unsigned short m_indices[6] = {
    0, 1, 2,  // Triangle 1: top-left → top-right → bottom-right
    2, 3, 0   // Triangle 2: bottom-right → bottom-left → top-left
};
```

### Memory Management

- **TextureCache Integration**: Automatic reference counting
- **GPU Buffers**: Lazy creation, persists across renders
- **Dirty Flag Optimization**: Vertices updated only when position/UVs/color change
- **RAII Destructor**: Releases texture reference and deletes GPU buffers

---

## Compilation Results

```
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

Result: ✅ SUCCESS
- Files compiled: 923/923 (100%)
- Errors: 0
- Warnings: 129 (format specifiers, unused variables - non-critical)
- Linking: ✅ Success
- Output: build/macos-arm64/GeneralsMD/GeneralsXZH
```

---

## Testing Plan (Phase 28.4.2-28.4.4)

### Phase 28.4.2: Load Menu Textures (Next)
- Load `defeated.dds` (1024x256 BC3 texture)
- Load `GameOver.tga` (1024x256 RGBA8 texture)
- Handle .big file extraction if needed
- Test texture loading from game assets directory

### Phase 28.4.3: Render Textured Quads
- Create test scene with multiple quads at different positions
- Render defeated.dds quad at (100, 100, 512, 256)
- Render GameOver.tga quad at (100, 400, 512, 256)
- Verify proper UV mapping and texture sampling

### Phase 28.4.4: Visual Validation
- Screenshot capture of rendered quads
- Verify textures appear correctly (no distortion, correct colors)
- Test alpha blending (set color alpha < 1.0)
- Test custom UVs (render portion of texture)

---

## Known Limitations

1. **2D Screen Space Only**: No 3D transformations (use for UI/HUD only)
2. **Single Texture**: No multi-texturing support
3. **No Batching**: Each quad is a separate draw call (performance consideration for many quads)
4. **Fixed Sampler State**: Linear filtering, clamp to edge (no customization yet)

---

## Future Enhancements (Post Phase 28)

1. **Sprite Batch**: Combine multiple quads into single draw call
2. **Custom Sampler States**: Nearest filtering, wrap modes, mipmapping control
3. **9-Slice Scaling**: For UI panels with borders
4. **Rotation/Transform**: 2D matrix transformations
5. **Shader Variants**: Different effects (blur, glow, color grading)

---

## Commits

- **Phase 28.4.1 Complete**: `texturedquad.h/cpp implementation + MetalWrapper buffer/draw APIs`
- **Files**: 2 created, 6 modified, 0 errors

---

## Summary

Phase 28.4.1 successfully implemented a complete `TexturedQuad` class with:
- ✅ Simple API for 2D textured quad rendering
- ✅ TextureCache integration with automatic reference counting
- ✅ GPU buffer management (vertex/index buffers)
- ✅ Metal backend DrawIndexedPrimitive implementation
- ✅ Full compilation success (0 errors)
- ✅ Ready for UI rendering validation in Phase 28.4.2-28.4.4

**Next Step**: Phase 28.4.2 - Load menu textures from game assets and test rendering
