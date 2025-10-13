#ifndef TEXTURECACHE_H
#define TEXTURECACHE_H

/**
 * @file texturecache.h
 * @brief Texture cache manager for Command & Conquer Generals Zero Hour
 * 
 * Phase 28.3: Texture Upload & Binding Implementation
 * Manages loaded textures with reference counting and path-based caching.
 * Supports DDS (BC1/BC2/BC3) and TGA (RGB/RGBA, RLE) formats.
 * 
 * Usage:
 *   TextureCache* cache = TextureCache::GetInstance();
 *   id texture = cache->LoadTexture("Data/Textures/defeated.dds");
 *   // ... use texture ...
 *   cache->ReleaseTexture("Data/Textures/defeated.dds");
 */

#include <unordered_map>
#include <string>

/**
 * @brief Texture cache entry with reference counting
 * 
 * Stores Metal texture handle and reference count.
 * Textures are freed when refcount reaches 0.
 */
struct TextureEntry {
    void* metal_texture;    // id<MTLTexture> (Metal backend) or GLuint* (OpenGL backend)
    int refcount;           // Reference count for lifetime management
    unsigned int width;     // Texture width in pixels
    unsigned int height;    // Texture height in pixels
    unsigned int format;    // Texture format (DDS or TGA format enum)
    bool is_dds;            // True if DDS, false if TGA
    
    TextureEntry() 
        : metal_texture(nullptr)
        , refcount(0)
        , width(0)
        , height(0)
        , format(0)
        , is_dds(false)
    {}
};

/**
 * @brief Texture cache manager (singleton pattern)
 * 
 * Caches loaded textures by normalized path.
 * Automatically detects DDS vs TGA format.
 * Reference counting prevents premature deletion.
 */
class TextureCache {
public:
    /**
     * @brief Get singleton instance
     * 
     * @return Pointer to TextureCache singleton
     */
    static TextureCache* GetInstance();
    
    /**
     * @brief Load texture from file
     * 
     * Loads texture if not in cache, or returns cached texture.
     * Increments reference count.
     * Automatically detects DDS vs TGA from file extension.
     * 
     * @param filename Path to texture file (e.g., "Data/Textures/defeated.dds")
     * @return Metal texture handle (id<MTLTexture>) or nullptr on failure
     * 
     * Example:
     *   void* tex = cache->LoadTexture("Data/Textures/caust00.tga");
     *   if (tex) {
     *       // Bind texture to shader
     *   }
     */
    void* LoadTexture(const char* filename);
    
    /**
     * @brief Release texture reference
     * 
     * Decrements reference count.
     * Frees texture memory when refcount reaches 0.
     * 
     * @param filename Path to texture file (same as LoadTexture)
     */
    void ReleaseTexture(const char* filename);
    
    /**
     * @brief Get texture entry by filename
     * 
     * @param filename Path to texture file
     * @return Pointer to TextureEntry or nullptr if not found
     */
    TextureEntry* GetTextureEntry(const char* filename);
    
    /**
     * @brief Clear all cached textures
     * 
     * Frees all textures regardless of refcount.
     * Use with caution - only call at shutdown.
     */
    void ClearCache();
    
    /**
     * @brief Get cache statistics
     * 
     * @param out_total_textures Output: total number of cached textures
     * @param out_total_memory Output: estimated memory usage in bytes
     */
    void GetCacheStats(int& out_total_textures, unsigned long& out_total_memory) const;
    
    /**
     * @brief Print cache contents for debugging
     */
    void DebugPrintCache() const;
    
private:
    /**
     * @brief Private constructor (singleton pattern)
     */
    TextureCache();
    
    /**
     * @brief Destructor - clears cache
     */
    ~TextureCache();
    
    /**
     * @brief Normalize texture path for cache key
     * 
     * Converts backslashes to forward slashes.
     * Converts to lowercase for case-insensitive matching.
     * 
     * @param path Original path string
     * @return Normalized path string
     * 
     * Example:
     *   "Data\\Textures\\Defeated.DDS" → "data/textures/defeated.dds"
     */
    std::string NormalizePath(const char* path) const;
    
    /**
     * @brief Detect file format from extension
     * 
     * @param filename Path to texture file
     * @return True if DDS, false if TGA (or unsupported)
     */
    bool IsDDSFile(const char* filename) const;
    
    /**
     * @brief Load DDS texture from file
     * 
     * @param filename Path to DDS file
     * @param out_entry Output TextureEntry (on success)
     * @return True on success, false on failure
     */
    bool LoadDDSTexture(const char* filename, TextureEntry& out_entry);
    
    /**
     * @brief Load TGA texture from file
     * 
     * @param filename Path to TGA file
     * @param out_entry Output TextureEntry (on success)
     * @return True on success, false on failure
     */
    bool LoadTGATexture(const char* filename, TextureEntry& out_entry);
    
    // Singleton instance
    static TextureCache* s_instance;
    
    // Cache storage: normalized path → TextureEntry
    std::unordered_map<std::string, TextureEntry> m_cache;
};

#endif // TEXTURECACHE_H
