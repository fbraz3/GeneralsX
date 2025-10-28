/**
 * @file texture_cache.cpp
 * @brief Texture cache implementation
 */

#ifndef _WIN32  // OpenGL backend only

#include "texture_cache.h"
#include "dds_loader.h"
#include "tga_loader.h"
#include "texture_upload.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cctype>

// Singleton instance
static TextureCache* s_instance = nullptr;

/**
 * @brief Get singleton instance
 */
TextureCache* TextureCache::Get_Instance() {
    if (!s_instance) {
        s_instance = new TextureCache();
    }
    return s_instance;
}

/**
 * @brief Constructor
 */
TextureCache::TextureCache()
    : m_load_count(0)
    , m_cache_hit_count(0)
    , m_cache_miss_count(0)
{
    printf("TEXTURE CACHE: Initialized\n");
}

/**
 * @brief Destructor - cleanup all textures
 */
TextureCache::~TextureCache() {
    Clear_All();
    printf("TEXTURE CACHE: Destroyed\n");
}

/**
 * @brief Normalize file path to lowercase
 */
std::string TextureCache::Normalize_Path(const char* path) const {
    if (!path) {
        return "";
    }
    
    std::string normalized = path;
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    
    // Convert backslashes to forward slashes
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    return normalized;
}

/**
 * @brief Load texture from disk
 */
GLuint TextureCache::Load_Texture_From_Disk(const char* file_path, uint32_t& width, uint32_t& height) {
    if (!file_path) {
        printf("TEXTURE CACHE ERROR: NULL file path\n");
        return 0;
    }
    
    // Detect format by extension
    const char* ext = strrchr(file_path, '.');
    if (!ext) {
        printf("TEXTURE CACHE ERROR: No file extension in '%s'\n", file_path);
        return 0;
    }
    
    GLuint texture_id = 0;
    
    // Convert extension to lowercase for comparison
    char ext_lower[16] = {0};
    for (int i = 0; ext[i] && i < 15; ++i) {
        ext_lower[i] = std::tolower(ext[i]);
    }
    
    if (strcmp(ext_lower, ".dds") == 0) {
        // Load DDS texture
        DDSData* dds = LoadDDS(file_path);
        if (dds) {
            texture_id = Create_GL_Texture_From_DDS(dds);
            width = dds->width;
            height = dds->height;
            delete dds;
        } else {
            printf("TEXTURE CACHE ERROR: Failed to load DDS file '%s'\n", file_path);
        }
    } else if (strcmp(ext_lower, ".tga") == 0) {
        // Load TGA texture
        TGAData* tga = LoadTGA(file_path);
        if (tga) {
            texture_id = Create_GL_Texture_From_TGA(tga);
            width = tga->width;
            height = tga->height;
            delete tga;
        } else {
            printf("TEXTURE CACHE ERROR: Failed to load TGA file '%s'\n", file_path);
        }
    } else {
        printf("TEXTURE CACHE ERROR: Unsupported file extension '%s' (must be .dds or .tga)\n", ext_lower);
    }
    
    return texture_id;
}

/**
 * @brief Get texture by file path
 */
GLuint TextureCache::Get_Texture(const char* file_path) {
    if (!file_path) {
        printf("TEXTURE CACHE ERROR: NULL file path in Get_Texture\n");
        return 0;
    }
    
    // Normalize path for case-insensitive lookup
    std::string normalized = Normalize_Path(file_path);
    
    // Check if already cached
    auto it = m_cache.find(normalized);
    if (it != m_cache.end()) {
        // Cache hit - increment reference count
        it->second.ref_count++;
        m_cache_hit_count++;
        
        printf("TEXTURE CACHE HIT: '%s' (ID %u, refs %u)\n",
               file_path, it->second.texture_id, it->second.ref_count);
        
        return it->second.texture_id;
    }
    
    // Cache miss - load from disk
    m_cache_miss_count++;
    
    printf("TEXTURE CACHE MISS: Loading '%s' from disk...\n", file_path);
    
    uint32_t width = 0, height = 0;
    GLuint texture_id = Load_Texture_From_Disk(file_path, width, height);
    
    if (texture_id == 0) {
        printf("TEXTURE CACHE ERROR: Failed to load texture '%s'\n", file_path);
        return 0;
    }
    
    // Add to cache
    TextureEntry entry;
    entry.texture_id = texture_id;
    entry.width = width;
    entry.height = height;
    entry.ref_count = 1;  // Initial reference
    entry.original_path = file_path;
    
    m_cache[normalized] = entry;
    m_load_count++;
    
    printf("TEXTURE CACHE: Cached '%s' (ID %u, %ux%u, refs 1)\n",
           file_path, texture_id, width, height);
    
    return texture_id;
}

/**
 * @brief Load texture from raw memory data (for VFS integration)
 */
GLuint TextureCache::Load_From_Memory(const char* cache_key, const void* pixel_data,
                                      uint32_t width, uint32_t height,
                                      GLenum format, size_t data_size) {
    if (!cache_key) {
        printf("TEXTURE CACHE ERROR: NULL cache key in Load_From_Memory\n");
        return 0;
    }
    
    if (!pixel_data) {
        printf("TEXTURE CACHE ERROR: NULL pixel data in Load_From_Memory\n");
        return 0;
    }
    
    if (width == 0 || height == 0) {
        printf("TEXTURE CACHE ERROR: Invalid dimensions %ux%u in Load_From_Memory\n", width, height);
        return 0;
    }
    
    // Normalize path for case-insensitive lookup
    std::string normalized = Normalize_Path(cache_key);
    
    // Check if already cached (VFS may request same texture multiple times)
    auto it = m_cache.find(normalized);
    if (it != m_cache.end()) {
        // Cache hit - increment reference count
        it->second.ref_count++;
        m_cache_hit_count++;
        
        printf("TEXTURE CACHE HIT (Memory): '%s' (ID %u, refs %u)\n",
               cache_key, it->second.texture_id, it->second.ref_count);
        
        return it->second.texture_id;
    }
    
    // Cache miss - create from memory
    m_cache_miss_count++;
    
    printf("TEXTURE CACHE MISS (Memory): Creating '%s' from memory (%ux%u, format 0x%04X)...\n",
           cache_key, width, height, format);
    
    // Upload to GPU (Metal or OpenGL) and get Metal texture pointer (Phase 37.5)
    void* metal_texture = nullptr;
    GLuint texture_id = Upload_Texture_From_Memory_With_Metal(pixel_data, width, height, format, data_size, &metal_texture);
    
    if (texture_id == 0) {
        printf("TEXTURE CACHE ERROR: Failed to upload texture '%s' from memory\n", cache_key);
        return 0;
    }
    
    // Add to cache
    TextureEntry entry;
    entry.texture_id = texture_id;
    entry.metal_texture_id = metal_texture;  // Phase 37.5: Store Metal texture pointer
    entry.width = width;
    entry.height = height;
    entry.ref_count = 1;  // Initial reference
    entry.original_path = cache_key;
    
    m_cache[normalized] = entry;
    m_load_count++;
    
    printf("TEXTURE CACHE: Cached (Memory) '%s' (ID %u, Metal=%p, %ux%u, refs 1)\n",
           cache_key, texture_id, metal_texture, width, height);
    
    return texture_id;
}

/**
 * @brief Release texture reference
 */
void TextureCache::Release_Texture(const char* file_path) {
    if (!file_path) {
        printf("TEXTURE CACHE WARNING: NULL file path in Release_Texture\n");
        return;
    }
    
    std::string normalized = Normalize_Path(file_path);
    
    auto it = m_cache.find(normalized);
    if (it == m_cache.end()) {
        printf("TEXTURE CACHE WARNING: Trying to release non-cached texture '%s'\n", file_path);
        return;
    }
    
    // Decrement reference count
    if (it->second.ref_count > 0) {
        it->second.ref_count--;
        printf("TEXTURE CACHE: Released '%s' (ID %u, refs %u)\n",
               file_path, it->second.texture_id, it->second.ref_count);
    } else {
        printf("TEXTURE CACHE WARNING: Texture '%s' already has 0 references\n", file_path);
    }
}

/**
 * @brief Get Metal texture pointer from cache (Phase 37.5)
 */
void* TextureCache::Get_Metal_Texture(const char* file_path) const {
    if (!file_path) {
        printf("TEXTURE CACHE WARNING: NULL file path in Get_Metal_Texture\n");
        return nullptr;
    }
    
    std::string normalized = Normalize_Path(file_path);
    
    auto it = m_cache.find(normalized);
    if (it == m_cache.end()) {
        printf("TEXTURE CACHE WARNING: Metal texture not found in cache for '%s'\n", file_path);
        return nullptr;
    }
    
    if (it->second.metal_texture_id == nullptr) {
        printf("TEXTURE CACHE DEBUG: Texture '%s' has no Metal texture (OpenGL-only or not yet created)\n", file_path);
    }
    
    return it->second.metal_texture_id;
}

/**
 * @brief Cleanup unused textures
 */
uint32_t TextureCache::Cleanup_Unused() {
    uint32_t deleted_count = 0;
    
    printf("TEXTURE CACHE: Cleaning up unused textures...\n");
    
    // Iterate and delete textures with ref_count == 0
    auto it = m_cache.begin();
    while (it != m_cache.end()) {
        if (it->second.ref_count == 0) {
            printf("TEXTURE CACHE: Deleting unused texture '%s' (ID %u)\n",
                   it->second.original_path.c_str(), it->second.texture_id);
            
            Delete_GL_Texture(it->second.texture_id);
            it = m_cache.erase(it);
            deleted_count++;
        } else {
            ++it;
        }
    }
    
    printf("TEXTURE CACHE: Cleanup complete - deleted %u texture(s)\n", deleted_count);
    return deleted_count;
}

/**
 * @brief Clear all cached textures
 */
void TextureCache::Clear_All() {
    printf("TEXTURE CACHE: Clearing all textures (total: %zu)...\n", m_cache.size());
    
    for (auto& pair : m_cache) {
        Delete_GL_Texture(pair.second.texture_id);
    }
    
    m_cache.clear();
    printf("TEXTURE CACHE: All textures cleared\n");
}

/**
 * @brief Print cache statistics
 */
void TextureCache::Print_Statistics() const {
    printf("=== TEXTURE CACHE STATISTICS ===\n");
    printf("Total textures loaded: %u\n", m_load_count);
    printf("Cache hits: %u\n", m_cache_hit_count);
    printf("Cache misses: %u\n", m_cache_miss_count);
    printf("Currently cached: %zu\n", m_cache.size());
    
    if (m_cache_hit_count + m_cache_miss_count > 0) {
        float hit_rate = (float)m_cache_hit_count / (m_cache_hit_count + m_cache_miss_count) * 100.0f;
        printf("Cache hit rate: %.1f%%\n", hit_rate);
    }
    
    printf("\nCached textures:\n");
    for (const auto& pair : m_cache) {
        printf("  - %s (ID %u, %ux%u, refs %u)\n",
               pair.second.original_path.c_str(),
               pair.second.texture_id,
               pair.second.width,
               pair.second.height,
               pair.second.ref_count);
    }
    printf("================================\n");
}

/**
 * @brief Check if texture is cached
 */
bool TextureCache::Is_Cached(const char* file_path) const {
    if (!file_path) {
        return false;
    }
    
    std::string normalized = Normalize_Path(file_path);
    return m_cache.find(normalized) != m_cache.end();
}

/**
 * @brief Get texture info from cache
 */
bool TextureCache::Get_Texture_Info(const char* file_path, TextureEntry& entry) const {
    if (!file_path) {
        return false;
    }
    
    std::string normalized = Normalize_Path(file_path);
    auto it = m_cache.find(normalized);
    
    if (it != m_cache.end()) {
        entry = it->second;
        return true;
    }
    
    return false;
}

#endif  // !_WIN32
