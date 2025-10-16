#include "texturecache.h"
#include "ddsloader.h"
#include "tgaloader.h"
#include "metalwrapper.h"

using namespace GX;
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <cctype>

/**
 * @file texturecache.cpp
 * @brief Texture cache manager implementation
 * 
 * Phase 28.3.1: TextureCache Class Implementation
 */

// Singleton instance
TextureCache* TextureCache::s_instance = nullptr;

// Constructor
TextureCache::TextureCache() {
    printf("TextureCache: Initialized\n");
}

// Destructor
TextureCache::~TextureCache() {
    ClearCache();
    printf("TextureCache: Destroyed\n");
}

// Get singleton instance
TextureCache* TextureCache::GetInstance() {
    if (s_instance == nullptr) {
        s_instance = new TextureCache();
    }
    return s_instance;
}

// Normalize path for cache key
std::string TextureCache::NormalizePath(const char* path) const {
    if (!path) {
        return "";
    }
    
    std::string normalized = path;
    
    // Convert backslashes to forward slashes
    for (char& c : normalized) {
        if (c == '\\') {
            c = '/';
        }
    }
    
    // Convert to lowercase for case-insensitive matching
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    return normalized;
}

// Detect file format from extension
bool TextureCache::IsDDSFile(const char* filename) const {
    if (!filename) {
        return false;
    }
    
    size_t len = strlen(filename);
    if (len < 4) {
        return false;
    }
    
    // Check last 4 characters (case-insensitive)
    const char* ext = filename + len - 4;
    return (strcasecmp(ext, ".dds") == 0);
}

// Load DDS texture from file
bool TextureCache::LoadDDSTexture(const char* filename, TextureEntry& out_entry) {
    // Load DDS file
    DDSTextureData dds_data;
    if (!DDSLoader::Load(filename, &dds_data)) {
        printf("TextureCache ERROR: Failed to load DDS file '%s'\n", filename);
        return false;
    }
    
    if (!dds_data.isValid) {
        printf("TextureCache ERROR: DDS file invalid '%s'\n", filename);
        return false;
    }
    
    // Create Metal texture
    void* metal_texture = MetalWrapper::CreateTextureFromDDS(
        dds_data.width,
        dds_data.height,
        (unsigned int)dds_data.format,
        dds_data.data,
        (unsigned int)dds_data.dataSize,
        dds_data.mipMapCount
    );
    
    // Free DDS data (Metal has its own copy)
    DDSLoader::Free(&dds_data);
    
    if (!metal_texture) {
        printf("TextureCache ERROR: Failed to create Metal texture for '%s'\n", filename);
        return false;
    }
    
    // Fill output entry
    out_entry.metal_texture = metal_texture;
    out_entry.refcount = 1;
    out_entry.width = dds_data.width;
    out_entry.height = dds_data.height;
    out_entry.format = (unsigned int)dds_data.format;
    out_entry.is_dds = true;
    
    printf("TextureCache: Loaded DDS texture '%s' (%ux%u, format %u)\n",
        filename, dds_data.width, dds_data.height, (unsigned int)dds_data.format);
    
    return true;
}

// Load TGA texture from file
bool TextureCache::LoadTGATexture(const char* filename, TextureEntry& out_entry) {
    // Load TGA file
    TGATextureData tga_data = TGALoader::Load(filename);
    if (!tga_data.is_valid) {
        printf("TextureCache ERROR: Failed to load TGA file '%s'\n", filename);
        return false;
    }
    
    // Create Metal texture (will be implemented in Phase 28.3.2)
    void* metal_texture = MetalWrapper::CreateTextureFromTGA(
        tga_data.width,
        tga_data.height,
        tga_data.data,
        tga_data.data_size
    );
    
    // Free TGA data (Metal has its own copy)
    TGALoader::Free(tga_data);
    
    if (!metal_texture) {
        printf("TextureCache ERROR: Failed to create Metal texture for '%s'\n", filename);
        return false;
    }
    
    // Fill output entry
    out_entry.metal_texture = metal_texture;
    out_entry.refcount = 1;
    out_entry.width = tga_data.width;
    out_entry.height = tga_data.height;
    out_entry.format = (unsigned int)tga_data.format;
    out_entry.is_dds = false;
    
    printf("TextureCache: Loaded TGA texture '%s' (%ux%u, format %u)\n",
        filename, tga_data.width, tga_data.height, (unsigned int)tga_data.format);
    
    return true;
}

// Load texture from file
void* TextureCache::LoadTexture(const char* filename) {
    if (!filename) {
        printf("TextureCache ERROR: NULL filename\n");
        return nullptr;
    }
    
    // Normalize path for cache lookup
    std::string normalized_path = NormalizePath(filename);
    
    // Check if already in cache
    auto it = m_cache.find(normalized_path);
    if (it != m_cache.end()) {
        // Found in cache - increment refcount
        it->second.refcount++;
        printf("TextureCache: Reusing cached texture '%s' (refcount=%d)\n",
            filename, it->second.refcount);
        return it->second.metal_texture;
    }
    
    // Not in cache - load new texture
    TextureEntry entry;
    bool success = false;
    
    if (IsDDSFile(filename)) {
        success = LoadDDSTexture(filename, entry);
    } else {
        success = LoadTGATexture(filename, entry);
    }
    
    if (!success) {
        return nullptr;
    }
    
    // Add to cache
    m_cache[normalized_path] = entry;
    
    return entry.metal_texture;
}

// Release texture reference
void TextureCache::ReleaseTexture(const char* filename) {
    if (!filename) {
        return;
    }
    
    // Normalize path for cache lookup
    std::string normalized_path = NormalizePath(filename);
    
    // Find in cache
    auto it = m_cache.find(normalized_path);
    if (it == m_cache.end()) {
        printf("TextureCache WARNING: Attempt to release non-cached texture '%s'\n", filename);
        return;
    }
    
    // Decrement refcount
    it->second.refcount--;
    printf("TextureCache: Released texture '%s' (refcount=%d)\n",
        filename, it->second.refcount);
    
    // Free texture if refcount reaches 0
    if (it->second.refcount <= 0) {
        if (it->second.metal_texture) {
            MetalWrapper::DeleteTexture(it->second.metal_texture);
            printf("TextureCache: Freed texture '%s'\n", filename);
        }
        m_cache.erase(it);
    }
}

// Get texture entry by filename
TextureEntry* TextureCache::GetTextureEntry(const char* filename) {
    if (!filename) {
        return nullptr;
    }
    
    std::string normalized_path = NormalizePath(filename);
    auto it = m_cache.find(normalized_path);
    
    if (it != m_cache.end()) {
        return &it->second;
    }
    
    return nullptr;
}

// Clear all cached textures
void TextureCache::ClearCache() {
    printf("TextureCache: Clearing cache (%zu textures)...\n", m_cache.size());
    
    for (auto& pair : m_cache) {
        if (pair.second.metal_texture) {
            MetalWrapper::DeleteTexture(pair.second.metal_texture);
        }
    }
    
    m_cache.clear();
    printf("TextureCache: Cache cleared\n");
}

// Get cache statistics
void TextureCache::GetCacheStats(int& out_total_textures, unsigned long& out_total_memory) const {
    out_total_textures = (int)m_cache.size();
    out_total_memory = 0;
    
    for (const auto& pair : m_cache) {
        const TextureEntry& entry = pair.second;
        
        // Estimate memory usage (conservative)
        unsigned long texture_memory = entry.width * entry.height * 4; // RGBA8 baseline
        
        if (entry.is_dds) {
            // Compressed formats use less memory
            // BC1: 0.5 bytes/pixel, BC2/BC3: 1 byte/pixel
            if (entry.format == 1) { // BC1
                texture_memory = entry.width * entry.height / 2;
            } else if (entry.format == 2 || entry.format == 3) { // BC2/BC3
                texture_memory = entry.width * entry.height;
            }
        }
        
        out_total_memory += texture_memory;
    }
}

// Print cache contents for debugging
void TextureCache::DebugPrintCache() const {
    printf("=== TextureCache Contents ===\n");
    printf("Total textures: %zu\n", m_cache.size());
    
    for (const auto& pair : m_cache) {
        const std::string& path = pair.first;
        const TextureEntry& entry = pair.second;
        
        printf("  %s\n", path.c_str());
        printf("    Dimensions: %ux%u\n", entry.width, entry.height);
        printf("    Format: %u (%s)\n", entry.format, entry.is_dds ? "DDS" : "TGA");
        printf("    Refcount: %d\n", entry.refcount);
        printf("    Handle: %p\n", entry.metal_texture);
    }
    
    int total_textures;
    unsigned long total_memory;
    GetCacheStats(total_textures, total_memory);
    printf("Estimated memory usage: %.2f MB\n", total_memory / (1024.0 * 1024.0));
    printf("=============================\n");
}
