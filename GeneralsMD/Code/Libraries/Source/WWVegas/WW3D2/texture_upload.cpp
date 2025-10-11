/**
 * @file texture_upload.cpp
 * @brief OpenGL texture upload implementation
 */

#ifndef _WIN32  // OpenGL backend only

#include "texture_upload.h"
#include <cstdio>
#include <cstring>

#ifdef __APPLE__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

// Extension constants (defined if not already present)
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

/**
 * @brief Convert TextureFilter to OpenGL filter constant
 */
static GLenum Get_GL_Filter(TextureFilter filter, bool is_mipmap_filter) {
    switch (filter) {
    case TextureFilter::NEAREST:
        return is_mipmap_filter ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    case TextureFilter::LINEAR:
        return is_mipmap_filter ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
    case TextureFilter::LINEAR_MIPMAP_LINEAR:
    case TextureFilter::ANISOTROPIC:
        return is_mipmap_filter ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    default:
        return GL_LINEAR;
    }
}

/**
 * @brief Convert TextureWrap to OpenGL wrap constant
 */
static GLenum Get_GL_Wrap(TextureWrap wrap) {
    switch (wrap) {
    case TextureWrap::REPEAT:
        return GL_REPEAT;
    case TextureWrap::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    case TextureWrap::MIRRORED_REPEAT:
        return GL_MIRRORED_REPEAT;
    default:
        return GL_REPEAT;
    }
}

/**
 * @brief Check if anisotropic filtering is supported
 */
bool Is_Anisotropic_Filtering_Supported() {
    // Check for extension (GLAD should have loaded it)
    return GLAD_GL_EXT_texture_filter_anisotropic != 0;
}

/**
 * @brief Get maximum anisotropic filtering level
 */
float Get_Max_Anisotropy() {
    if (!Is_Anisotropic_Filtering_Supported()) {
        return 1.0f;
    }
    
    float max_anisotropy = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    return max_anisotropy;
}

/**
 * @brief Apply texture parameters (filtering, wrapping, anisotropy)
 */
static void Apply_Texture_Parameters(const TextureUploadParams& params, bool has_mipmaps) {
    // Minification filter (uses mipmaps if available)
    GLenum min_filter = has_mipmaps 
        ? Get_GL_Filter(params.min_filter, true)
        : Get_GL_Filter(params.min_filter, false);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    
    // Magnification filter (never uses mipmaps)
    GLenum mag_filter = Get_GL_Filter(params.mag_filter, false);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    
    // Wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Get_GL_Wrap(params.wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Get_GL_Wrap(params.wrap_t));
    
    // Anisotropic filtering (if supported and requested)
    if (params.min_filter == TextureFilter::ANISOTROPIC || params.mag_filter == TextureFilter::ANISOTROPIC) {
        if (Is_Anisotropic_Filtering_Supported()) {
            float max_aniso = Get_Max_Anisotropy();
            float aniso = (params.anisotropy < max_aniso) ? params.anisotropy : max_aniso;
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
            printf("TEXTURE: Applied anisotropic filtering: %.1fx (max: %.1fx)\n", aniso, max_aniso);
        } else {
            printf("TEXTURE WARNING: Anisotropic filtering requested but not supported\n");
        }
    }
}

/**
 * @brief Create OpenGL texture from DDS data
 */
GLuint Create_GL_Texture_From_DDS(const DDSData* dds, const TextureUploadParams& params) {
    if (!dds) {
        printf("TEXTURE ERROR: NULL DDSData passed to Create_GL_Texture_From_DDS\n");
        return 0;
    }
    
    // CRITICAL: Verify we have a valid OpenGL context before making GL calls
    #ifndef _WIN32
    void* current_context = SDL_GL_GetCurrentContext();
    if (!current_context) {
        printf("TEXTURE ERROR: No OpenGL context active! Cannot upload DDS texture. Deferring upload...\n");
        return 0; // Texture will be loaded on-demand when context is ready
    }
    #endif
    
    if (dds->width == 0 || dds->height == 0) {
        printf("TEXTURE ERROR: Invalid DDS dimensions %ux%u\n", dds->width, dds->height);
        return 0;
    }
    
    // Generate texture ID
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    if (texture_id == 0) {
        printf("TEXTURE ERROR: Failed to generate texture ID\n");
        return 0;
    }
    
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Get OpenGL internal format
    GLenum internal_format = GetGLInternalFormat(dds->format);
    bool is_compressed = (dds->format == DDS_FORMAT_DXT1 || 
                         dds->format == DDS_FORMAT_DXT3 || 
                         dds->format == DDS_FORMAT_DXT5);
    
    printf("TEXTURE: Uploading DDS texture (ID %u): %ux%u, format %d, %d mipmap(s), compressed=%d\n",
           texture_id, dds->width, dds->height, (int)dds->format, dds->num_mipmaps, is_compressed);
    
    // Upload all mipmap levels
    for (int mip = 0; mip < dds->num_mipmaps; ++mip) {
        int mip_width = dds->mip_widths[mip];
        int mip_height = dds->mip_heights[mip];
        size_t mip_size = dds->mip_sizes[mip];
        const uint8_t* mip_data_ptr = dds->mip_data[mip];
        
        if (!mip_data_ptr) {
            printf("TEXTURE WARNING: Mipmap level %d has null data, skipping\n", mip);
            continue;
        }
        
        if (is_compressed) {
            // Upload compressed texture
            glCompressedTexImage2D(GL_TEXTURE_2D, mip, internal_format,
                                  mip_width, mip_height, 0,
                                  mip_size, mip_data_ptr);
        } else {
            // Upload uncompressed texture
            GLenum format = (dds->format == DDS_FORMAT_RGB8) ? GL_RGB : GL_RGBA;
            GLenum type = GL_UNSIGNED_BYTE;
            glTexImage2D(GL_TEXTURE_2D, mip, internal_format,
                        mip_width, mip_height, 0,
                        format, type, mip_data_ptr);
        }
        
        // Check for errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            printf("TEXTURE ERROR: Failed to upload mipmap level %d (error 0x%04X)\n", mip, error);
            glDeleteTextures(1, &texture_id);
            return 0;
        }
    }
    
    // Apply texture parameters
    Apply_Texture_Parameters(params, dds->num_mipmaps > 1);
    
    printf("TEXTURE SUCCESS: DDS texture uploaded (ID %u)\n", texture_id);
    return texture_id;
}

/**
 * @brief Create OpenGL texture from TGA data
 */
GLuint Create_GL_Texture_From_TGA(const TGAData* tga, const TextureUploadParams& params) {
    if (!tga || !tga->pixels) {
        printf("TEXTURE ERROR: Invalid TGA data (null pointer)\n");
        return 0;
    }
    
    // CRITICAL: Verify we have a valid OpenGL context before making GL calls
    #ifndef _WIN32
    void* current_context = SDL_GL_GetCurrentContext();
    if (!current_context) {
        printf("TEXTURE ERROR: No OpenGL context active! Cannot upload texture. Deferring upload...\n");
        return 0; // Texture will be loaded on-demand when context is ready
    }
    #endif
    
    if (tga->width == 0 || tga->height == 0) {
        printf("TEXTURE ERROR: Invalid TGA dimensions %ux%u\n", tga->width, tga->height);
        return 0;
    }
    
    // Generate texture ID
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    if (texture_id == 0) {
        printf("TEXTURE ERROR: Failed to generate texture ID\n");
        return 0;
    }
    
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    printf("TEXTURE: Uploading TGA texture (ID %u): %ux%u, RGBA8, generate_mipmaps=%d\n",
           texture_id, tga->width, tga->height, params.generate_mipmaps);
    
    // Upload base level (always RGBA8 after TGA conversion)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                tga->width, tga->height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, tga->pixels);
    
    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("TEXTURE ERROR: Failed to upload TGA texture (error 0x%04X)\n", error);
        glDeleteTextures(1, &texture_id);
        return 0;
    }
    
    // Generate mipmaps if requested
    bool has_mipmaps = false;
    if (params.generate_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            printf("TEXTURE WARNING: Failed to generate mipmaps (error 0x%04X)\n", error);
        } else {
            has_mipmaps = true;
            printf("TEXTURE: Generated mipmaps for TGA texture\n");
        }
    }
    
    // Apply texture parameters
    Apply_Texture_Parameters(params, has_mipmaps);
    
    printf("TEXTURE SUCCESS: TGA texture uploaded (ID %u)\n", texture_id);
    return texture_id;
}

/**
 * @brief Delete OpenGL texture
 */
void Delete_GL_Texture(GLuint texture_id) {
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id);
        printf("TEXTURE: Deleted texture ID %u\n", texture_id);
    }
}

#endif  // !_WIN32
