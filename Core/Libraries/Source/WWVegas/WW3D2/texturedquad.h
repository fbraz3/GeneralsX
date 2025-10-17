#ifndef TEXTUREDQUAD_H
#define TEXTUREDQUAD_H

/**
 * @file texturedquad.h
 * @brief Textured quad renderer for UI elements (Phase 28.4)
 * 
 * Provides simple API for rendering textured quads with UV mapping.
 * Used for menu backgrounds, UI overlays, and 2D sprites.
 * 
 * Usage:
 *   TexturedQuad quad;
 *   quad.SetTexture("Data/Textures/menu_bg.dds");
 *   quad.SetPosition(0, 0, 1024, 768);
 *   quad.Render();
 */

#include <string>

namespace GX {

/**
 * @brief Vertex structure for textured quad
 * 
 * Contains position, color, and UV coordinates.
 * Matches shader VertexInput structure.
 */
struct TexturedVertex {
    float position[3];  // X, Y, Z
    float normal[3];    // Normal (unused for 2D quads)
    float color[4];     // RGBA (0-1 range)
    float texcoord[2];  // U, V
};

/**
 * @brief Simple textured quad renderer
 * 
 * Renders a 2D quad with texture mapping.
 * Uses TextureCache for texture management.
 */
class TexturedQuad {
public:
    /**
     * @brief Constructor
     */
    TexturedQuad();
    
    /**
     * @brief Destructor - releases texture reference
     */
    ~TexturedQuad();
    
    /**
     * @brief Set texture from file path
     * 
     * @param texture_path Path to texture file (DDS or TGA)
     * @return True on success, false on failure
     * 
     * Example:
     *   quad.SetTexture("Data/Textures/defeated.dds");
     */
    bool SetTexture(const char* texture_path);
    
    /**
     * @brief Set texture directly from handle (for programmatic textures)
     * 
     * @param texture_handle Metal texture handle pointer
     * @return true if successful, false if texture_handle is null
     * 
     * Use this when you have a manually-created texture (e.g. test patterns)
     * instead of loading from a file path.
     */
    bool SetTextureHandle(void* texture_handle);
    
    /**
     * @brief Set quad position and size (screen space)
     * 
     * @param x Left position (pixels)
     * @param y Top position (pixels)
     * @param width Width (pixels)
     * @param height Height (pixels)
     * 
     * Coordinates are in screen space (0,0) = top-left.
     */
    void SetPosition(float x, float y, float width, float height);
    
    /**
     * @brief Set quad color tint (modulates texture)
     * 
     * @param r Red (0-1)
     * @param g Green (0-1)
     * @param b Blue (0-1)
     * @param a Alpha (0-1)
     */
    void SetColor(float r, float g, float b, float a = 1.0f);
    
    /**
     * @brief Set custom UV coordinates
     * 
     * @param u0 Top-left U (0-1)
     * @param v0 Top-left V (0-1)
     * @param u1 Bottom-right U (0-1)
     * @param v1 Bottom-right V (0-1)
     * 
     * Default UVs are (0,0) to (1,1) for full texture.
     * Use custom UVs for texture atlases.
     */
    void SetUVs(float u0, float v0, float u1, float v1);
    
    /**
     * @brief Render the quad
     * 
     * Requires active Metal render pass.
     * Call between BeginFrame() and EndFrame().
     */
    void Render();
    
    /**
     * @brief Get current texture handle
     * 
     * @return Texture handle or nullptr if not loaded
     */
    void* GetTexture() const { return m_texture; }
    
    /**
     * @brief Check if texture is loaded
     * 
     * @return True if texture is valid
     */
    bool HasTexture() const { return m_texture != nullptr; }
    
    /**
     * @brief Set viewport size for screen-space to clip-space conversion
     * 
     * @param width Viewport width in pixels
     * @param height Viewport height in pixels
     * 
     * Call once during initialization with window dimensions.
     * Default is 1280x768.
     */
    static void SetViewportSize(float width, float height);
    
private:
    /**
     * @brief Update vertex buffer with current position/UVs/color
     */
    void UpdateVertices();
    
    /**
     * @brief Create GPU buffers if needed
     */
    void EnsureBuffersCreated();
    
    // Texture
    void* m_texture;            // Metal texture handle
    std::string m_texture_path; // Path for cache management
    
    // Geometry
    TexturedVertex m_vertices[4]; // Quad vertices (2 triangles)
    unsigned short m_indices[6];  // Index buffer (0,1,2, 2,3,0)
    
    // Position/UVs
    float m_x, m_y;             // Position
    float m_width, m_height;    // Size
    float m_u0, m_v0;           // UV top-left
    float m_u1, m_v1;           // UV bottom-right
    
    // Color
    float m_color[4];           // RGBA tint
    
    // GPU buffers
    void* m_vertex_buffer;      // Metal vertex buffer
    void* m_index_buffer;       // Metal index buffer
    bool m_buffers_created;     // Flag to track buffer creation
    bool m_vertices_dirty;      // Flag to track vertex updates
};

} // namespace GX

#endif // TEXTUREDQUAD_H
