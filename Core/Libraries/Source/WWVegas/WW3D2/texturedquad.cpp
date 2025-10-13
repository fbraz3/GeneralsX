/**
 * @file texturedquad.cpp
 * @brief Textured quad renderer implementation (Phase 28.4)
 */

#include "texturedquad.h"
#include "texturecache.h"
#include "metalwrapper.h"
#include <cstring>
#include <cstdio>

namespace GX {

TexturedQuad::TexturedQuad()
    : m_texture(nullptr)
    , m_texture_path()
    , m_x(0.0f)
    , m_y(0.0f)
    , m_width(100.0f)
    , m_height(100.0f)
    , m_u0(0.0f)
    , m_v0(0.0f)
    , m_u1(1.0f)
    , m_v1(1.0f)
    , m_vertex_buffer(nullptr)
    , m_index_buffer(nullptr)
    , m_buffers_created(false)
    , m_vertices_dirty(true)
{
    // Initialize color to white (no tint)
    m_color[0] = 1.0f; // R
    m_color[1] = 1.0f; // G
    m_color[2] = 1.0f; // B
    m_color[3] = 1.0f; // A
    
    // Initialize index buffer (two triangles: 0-1-2, 2-3-0)
    m_indices[0] = 0;
    m_indices[1] = 1;
    m_indices[2] = 2;
    m_indices[3] = 2;
    m_indices[4] = 3;
    m_indices[5] = 0;
    
    // Initialize vertices (will be updated in UpdateVertices())
    memset(m_vertices, 0, sizeof(m_vertices));
}

TexturedQuad::~TexturedQuad()
{
    // Release texture reference
    if (!m_texture_path.empty()) {
        TextureCache::GetInstance()->ReleaseTexture(m_texture_path.c_str());
        m_texture = nullptr;
    }
    
    // Delete GPU buffers
    if (m_vertex_buffer) {
        MetalWrapper::DeleteVertexBuffer(m_vertex_buffer);
        m_vertex_buffer = nullptr;
    }
    if (m_index_buffer) {
        MetalWrapper::DeleteIndexBuffer(m_index_buffer);
        m_index_buffer = nullptr;
    }
}

bool TexturedQuad::SetTexture(const char* texture_path)
{
    if (!texture_path) {
        printf("TexturedQuad::SetTexture() - ERROR: NULL texture path\n");
        return false;
    }
    
    // Release previous texture
    if (!m_texture_path.empty()) {
        TextureCache::GetInstance()->ReleaseTexture(m_texture_path.c_str());
        m_texture = nullptr;
    }
    
    // Load new texture
    m_texture = TextureCache::GetInstance()->LoadTexture(texture_path);
    if (!m_texture) {
        printf("TexturedQuad::SetTexture() - ERROR: Failed to load texture '%s'\n", texture_path);
        m_texture_path.clear();
        return false;
    }
    
    m_texture_path = texture_path;
    printf("TexturedQuad::SetTexture() - SUCCESS: Loaded '%s'\n", texture_path);
    return true;
}

void TexturedQuad::SetPosition(float x, float y, float width, float height)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
    m_vertices_dirty = true;
}

void TexturedQuad::SetColor(float r, float g, float b, float a)
{
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
    m_color[3] = a;
    m_vertices_dirty = true;
}

void TexturedQuad::SetUVs(float u0, float v0, float u1, float v1)
{
    m_u0 = u0;
    m_v0 = v0;
    m_u1 = u1;
    m_v1 = v1;
    m_vertices_dirty = true;
}

void TexturedQuad::UpdateVertices()
{
    // Vertex 0: Top-left
    m_vertices[0].position[0] = m_x;
    m_vertices[0].position[1] = m_y;
    m_vertices[0].position[2] = 0.0f;
    m_vertices[0].texcoord[0] = m_u0;
    m_vertices[0].texcoord[1] = m_v0;
    
    // Vertex 1: Top-right
    m_vertices[1].position[0] = m_x + m_width;
    m_vertices[1].position[1] = m_y;
    m_vertices[1].position[2] = 0.0f;
    m_vertices[1].texcoord[0] = m_u1;
    m_vertices[1].texcoord[1] = m_v0;
    
    // Vertex 2: Bottom-right
    m_vertices[2].position[0] = m_x + m_width;
    m_vertices[2].position[1] = m_y + m_height;
    m_vertices[2].position[2] = 0.0f;
    m_vertices[2].texcoord[0] = m_u1;
    m_vertices[2].texcoord[1] = m_v1;
    
    // Vertex 3: Bottom-left
    m_vertices[3].position[0] = m_x;
    m_vertices[3].position[1] = m_y + m_height;
    m_vertices[3].position[2] = 0.0f;
    m_vertices[3].texcoord[0] = m_u0;
    m_vertices[3].texcoord[1] = m_v1;
    
    // Set normals (unused for 2D, but required by shader)
    for (int i = 0; i < 4; i++) {
        m_vertices[i].normal[0] = 0.0f;
        m_vertices[i].normal[1] = 0.0f;
        m_vertices[i].normal[2] = 1.0f;
    }
    
    // Set color for all vertices
    for (int i = 0; i < 4; i++) {
        m_vertices[i].color[0] = m_color[0];
        m_vertices[i].color[1] = m_color[1];
        m_vertices[i].color[2] = m_color[2];
        m_vertices[i].color[3] = m_color[3];
    }
    
    m_vertices_dirty = false;
}

void TexturedQuad::EnsureBuffersCreated()
{
    if (m_buffers_created) {
        return;
    }
    
    // Create vertex buffer
    unsigned int vertex_size = sizeof(TexturedVertex) * 4;
    m_vertex_buffer = MetalWrapper::CreateVertexBuffer(vertex_size, m_vertices);
    if (!m_vertex_buffer) {
        printf("TexturedQuad::EnsureBuffersCreated() - ERROR: Failed to create vertex buffer\n");
        return;
    }
    
    // Create index buffer
    unsigned int index_size = sizeof(unsigned short) * 6;
    m_index_buffer = MetalWrapper::CreateIndexBuffer(index_size, m_indices);
    if (!m_index_buffer) {
        printf("TexturedQuad::EnsureBuffersCreated() - ERROR: Failed to create index buffer\n");
        MetalWrapper::DeleteVertexBuffer(m_vertex_buffer);
        m_vertex_buffer = nullptr;
        return;
    }
    
    m_buffers_created = true;
    printf("TexturedQuad::EnsureBuffersCreated() - SUCCESS: Created GPU buffers\n");
}

void TexturedQuad::Render()
{
    // Check if texture is loaded
    if (!m_texture) {
        printf("TexturedQuad::Render() - WARNING: No texture loaded, skipping render\n");
        return;
    }
    
    // Update vertices if needed
    if (m_vertices_dirty) {
        UpdateVertices();
    }
    
    // Create GPU buffers if needed
    if (!m_buffers_created) {
        EnsureBuffersCreated();
        if (!m_buffers_created) {
            printf("TexturedQuad::Render() - ERROR: Failed to create GPU buffers\n");
            return;
        }
    }
    
    // Update vertex buffer if vertices changed
    if (m_vertex_buffer) {
        unsigned int vertex_size = sizeof(TexturedVertex) * 4;
        MetalWrapper::UpdateVertexBuffer(m_vertex_buffer, m_vertices, vertex_size);
    }
    
#ifdef __APPLE__
    // Bind texture
    MetalWrapper::BindTexture(m_texture, 0);
    
    // Set vertex buffer
    MetalWrapper::SetVertexBuffer(m_vertex_buffer, 0, 0);
    
    // Set index buffer
    MetalWrapper::SetIndexBuffer(m_index_buffer, 0);
    
    // Draw indexed primitives (2 triangles = 6 indices)
    MetalWrapper::DrawIndexedPrimitive(
        3,  // D3DPT_TRIANGLELIST
        0,  // base_vertex_index
        0,  // min_vertex
        4,  // num_vertices
        0,  // start_index
        2   // primitive_count (2 triangles)
    );
    
    // Unbind texture
    MetalWrapper::UnbindTexture(0);
#endif
    
    printf("TexturedQuad::Render() - Rendered quad at (%.0f, %.0f) size (%.0f x %.0f)\n",
           m_x, m_y, m_width, m_height);
}

} // namespace GX
