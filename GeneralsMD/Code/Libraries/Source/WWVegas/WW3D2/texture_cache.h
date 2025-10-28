/**
 * @file texture_cache.h
 * @brief Texture cache system with reference counting
 * 
 * Provides centralized texture management with:
 * - Case-insensitive filename lookup (for cross-platform compatibility)
 * - Reference counting (automatic cleanup)
 * - Duplicate load prevention
 * - Debug statistics
 * 
 * Thread-safety: Not thread-safe (assumes single-threaded OpenGL context)
 * 
 * Example usage:
 *   TextureCache* cache = TextureCache::Get_Instance();
 *   
 *   // Load texture (or get existing)
 *   GLuint tex_id = cache->Get_Texture("Data/Textures/Terrain.dds");
 *   
 *   // Use texture...
 *   
 *   // Release when done
 *   cache->Release_Texture("Data/Textures/Terrain.dds");
 *   
 *   // Cleanup all unused textures
 *   cache->Cleanup_Unused();
 */

#ifndef TEXTURE_CACHE_H
#define TEXTURE_CACHE_H

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <cstdint>

/**
 * @brief Cached texture entry with reference counting
 */
struct TextureEntry {
    GLuint texture_id;          ///< OpenGL texture ID
    void* metal_texture_id;     ///< Phase 37.5: Metal texture pointer (id<MTLTexture>*)
    uint32_t width;             ///< Texture width in pixels
    uint32_t height;            ///< Texture height in pixels
    uint32_t ref_count;         ///< Reference count (0 = can be deleted)
    std::string original_path;  ///< Original file path (for debugging)
    
    TextureEntry()
        : texture_id(0)
        , metal_texture_id(nullptr)
        , width(0)
        , height(0)
        , ref_count(0)
    {}
};

/**
 * @brief Texture cache manager (singleton)
 * 
 * Centralizes texture loading and prevents duplicate loads.
 * Uses reference counting to automatically cleanup unused textures.
 */
class TextureCache {
public:
    /**
     * @brief Get singleton instance
     */
    static TextureCache* Get_Instance();
    
    /**
     * @brief Destructor - cleanup all textures
     */
    ~TextureCache();
    
    /**
     * @brief Get texture by file path (case-insensitive)
     * 
     * If texture is already loaded, increments reference count.
     * If not loaded, loads from disk and adds to cache.
     * 
     * Supported formats:
     * - .dds (compressed DXT1/DXT3/DXT5)
     * - .tga (uncompressed RGB/RGBA)
     * 
     * @param file_path Path to texture file (relative or absolute)
     * @return OpenGL texture ID (0 on failure)
     */
    GLuint Get_Texture(const char* file_path);
    
    /**
     * @brief Load texture from raw memory data (for VFS integration)
     * 
     * Creates Metal/OpenGL texture from pixel data already in memory.
     * Used when texture data comes from .big files (Virtual File System).
     * 
     * Supported formats:
     * - GL_RGBA8 (32-bit RGBA uncompressed)
     * - GL_RGB8 (24-bit RGB uncompressed)
     * - GL_COMPRESSED_RGBA_S3TC_DXT1_EXT (BC1/DXT1)
     * - GL_COMPRESSED_RGBA_S3TC_DXT3_EXT (BC2/DXT3)
     * - GL_COMPRESSED_RGBA_S3TC_DXT5_EXT (BC3/DXT5)
     * 
     * @param cache_key Unique identifier for caching (e.g., original filepath)
     * @param pixel_data Raw pixel data in memory
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @param format OpenGL internal format (GL_RGBA8, GL_COMPRESSED_*, etc.)
     * @param data_size Size of pixel_data in bytes
     * @return OpenGL texture ID (0 on failure)
     */
    GLuint Load_From_Memory(const char* cache_key, const void* pixel_data, 
                            uint32_t width, uint32_t height, 
                            GLenum format, size_t data_size);
    
    /**
     * @brief Release texture reference
     * 
     * Decrements reference count. Does NOT delete texture immediately.
     * Use Cleanup_Unused() to actually delete unreferenced textures.
     * 
     * @param file_path Path used when loading texture
     */
    void Release_Texture(const char* file_path);
    
    /**
     * @brief Get Metal texture pointer from cache (Phase 37.5)
     * 
     * Retrieves the Metal texture pointer for a cached texture.
     * Metal texture must have been loaded via Load_From_Memory or Get_Texture.
     * 
     * @param file_path Path used when loading texture
     * @return Metal texture pointer (id<MTLTexture>*) or nullptr if not cached
     */
    void* Get_Metal_Texture(const char* file_path) const;
    
    /**
     * @brief Delete all textures with ref_count == 0
     * 
     * Should be called periodically (e.g., after loading screen)
     * to free memory from unused textures.
     * 
     * @return Number of textures deleted
     */
    uint32_t Cleanup_Unused();
    
    /**
     * @brief Delete all cached textures (regardless of ref count)
     * 
     * WARNING: Only call during shutdown or when you're sure
     * no textures are in use!
     */
    void Clear_All();
    
    /**
     * @brief Get cache statistics
     */
    void Print_Statistics() const;
    
    /**
     * @brief Check if texture is cached
     * 
     * @param file_path Path to check (case-insensitive)
     * @return true if texture is in cache
     */
    bool Is_Cached(const char* file_path) const;
    
    /**
     * @brief Get texture info from cache
     * 
     * @param file_path Path to texture
     * @param entry Output texture entry (filled if found)
     * @return true if texture found in cache
     */
    bool Get_Texture_Info(const char* file_path, TextureEntry& entry) const;

private:
    // Singleton - private constructor
    TextureCache();
    
    // Non-copyable
    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;
    
    /**
     * @brief Normalize file path to lowercase for case-insensitive lookup
     * 
     * Also converts backslashes to forward slashes for cross-platform.
     * 
     * @param path Original path
     * @return Normalized lowercase path
     */
    std::string Normalize_Path(const char* path) const;
    
    /**
     * @brief Load texture from disk
     * 
     * Detects format by extension (.dds or .tga) and loads accordingly.
     * 
     * @param file_path Path to texture file
     * @param width Output width
     * @param height Output height
     * @return OpenGL texture ID (0 on failure)
     */
    GLuint Load_Texture_From_Disk(const char* file_path, uint32_t& width, uint32_t& height);
    
    // Cache storage (key = normalized lowercase path)
    std::unordered_map<std::string, TextureEntry> m_cache;
    
    // Statistics
    uint32_t m_load_count;          ///< Total textures loaded from disk
    uint32_t m_cache_hit_count;     ///< Cache hits (texture already loaded)
    uint32_t m_cache_miss_count;    ///< Cache misses (had to load from disk)
};

#endif // TEXTURE_CACHE_H
