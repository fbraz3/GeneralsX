/*
 * Phase 19: Lighting System
 * d3d8_vulkan_lighting.h
 *
 * Lighting calculations and shader integration for Vulkan rendering pipeline.
 * Supports multiple light types: directional, point, spot, and ambient.
 * Provides per-object light contribution calculations and dynamic light updates.
 *
 * Architecture:
 * - Light data structures for all light types
 * - Shader uniform buffer objects (UBOs)
 * - Per-object light contribution calculations
 * - Dynamic light updates during render loop
 * - Light attenuation and falloff
 * - Shadow mapping infrastructure (placeholder for Phase 39+)
 *
 * Handle ranges: 14000+ (non-overlapping with other phases)
 * Cache: 2 entries
 *
 * Dependencies: Phase 13 (descriptors), Phase 14 (shaders)
 */

#ifndef D3D8_VULKAN_LIGHTING_H
#define D3D8_VULKAN_LIGHTING_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Enumerations
 * ============================================================================ */

/**
 * Lighting system state machine
 */
typedef enum {
    D3D8_VULKAN_LIGHTING_STATE_UNINITIALIZED = 0,
    D3D8_VULKAN_LIGHTING_STATE_READY         = 1,
    D3D8_VULKAN_LIGHTING_STATE_ACTIVE        = 2,
    D3D8_VULKAN_LIGHTING_STATE_SHUTDOWN      = 3
} D3D8_VULKAN_LIGHTING_STATE;

/**
 * Light type enumeration
 */
typedef enum {
    D3D8_VULKAN_LIGHT_TYPE_DIRECTIONAL = 0,
    D3D8_VULKAN_LIGHT_TYPE_POINT       = 1,
    D3D8_VULKAN_LIGHT_TYPE_SPOT        = 2,
    D3D8_VULKAN_LIGHT_TYPE_AMBIENT     = 3
} D3D8_VULKAN_LIGHT_TYPE;

/**
 * Attenuation model enumeration
 */
typedef enum {
    D3D8_VULKAN_ATTENUATION_NONE         = 0,
    D3D8_VULKAN_ATTENUATION_LINEAR       = 1,
    D3D8_VULKAN_ATTENUATION_QUADRATIC    = 2,
    D3D8_VULKAN_ATTENUATION_EXPONENTIAL  = 3
} D3D8_VULKAN_ATTENUATION_MODEL;

/* ============================================================================
 * Structures
 * ============================================================================ */

/**
 * RGBA color representation
 * Used for light diffuse, specular, and ambient colors
 */
typedef struct {
    float r;
    float g;
    float b;
    float a;
} D3D8_VULKAN_COLOR_RGBA;

/**
 * 3D vector (position, direction, etc.)
 */
typedef struct {
    float x;
    float y;
    float z;
    float _padding;  /* Alignment for shader UBOs */
} D3D8_VULKAN_VEC3;

/**
 * Directional light (sun, moon)
 * Used for global illumination like sun light
 */
typedef struct {
    D3D8_VULKAN_VEC3 direction;         /* Light direction (normalized) */
    D3D8_VULKAN_COLOR_RGBA diffuse;     /* Diffuse color */
    D3D8_VULKAN_COLOR_RGBA specular;    /* Specular color */
    D3D8_VULKAN_COLOR_RGBA ambient;     /* Ambient color */
    float intensity;                     /* Light intensity [0.0, 1.0] */
    float _padding[3];                   /* Alignment */
} D3D8_VULKAN_DIRECTIONAL_LIGHT;

/**
 * Point light (lamp, explosion, etc.)
 * Used for localized illumination
 */
typedef struct {
    D3D8_VULKAN_VEC3 position;          /* Light position (world space) */
    D3D8_VULKAN_COLOR_RGBA diffuse;     /* Diffuse color */
    D3D8_VULKAN_COLOR_RGBA specular;    /* Specular color */
    float range;                         /* Light range (falloff distance) */
    float intensity;                     /* Light intensity [0.0, 1.0] */
    float constant;                      /* Attenuation: constant factor */
    float linear;                        /* Attenuation: linear factor */
    float quadratic;                     /* Attenuation: quadratic factor */
    D3D8_VULKAN_ATTENUATION_MODEL attenuation; /* Attenuation model */
    float _padding[2];                   /* Alignment */
} D3D8_VULKAN_POINT_LIGHT;

/**
 * Spot light (flashlight, spotlight, etc.)
 * Used for cone-shaped illumination
 */
typedef struct {
    D3D8_VULKAN_VEC3 position;          /* Light position (world space) */
    D3D8_VULKAN_VEC3 direction;         /* Light direction (normalized) */
    D3D8_VULKAN_COLOR_RGBA diffuse;     /* Diffuse color */
    D3D8_VULKAN_COLOR_RGBA specular;    /* Specular color */
    float range;                         /* Light range (falloff distance) */
    float intensity;                     /* Light intensity [0.0, 1.0] */
    float inner_cutoff;                 /* Inner cone angle (cos) */
    float outer_cutoff;                 /* Outer cone angle (cos) */
    float constant;                      /* Attenuation: constant factor */
    float linear;                        /* Attenuation: linear factor */
    float quadratic;                     /* Attenuation: quadratic factor */
    D3D8_VULKAN_ATTENUATION_MODEL attenuation; /* Attenuation model */
    float _padding;                      /* Alignment */
} D3D8_VULKAN_SPOT_LIGHT;

/**
 * Ambient light (global illumination base)
 */
typedef struct {
    D3D8_VULKAN_COLOR_RGBA color;       /* Ambient color */
    float intensity;                     /* Ambient intensity [0.0, 1.0] */
    float _padding[3];                   /* Alignment */
} D3D8_VULKAN_AMBIENT_LIGHT;

/**
 * Per-object material properties
 * Used for light calculations (diffuse/specular responses)
 */
typedef struct {
    D3D8_VULKAN_COLOR_RGBA diffuse;     /* Diffuse color */
    D3D8_VULKAN_COLOR_RGBA specular;    /* Specular color */
    D3D8_VULKAN_COLOR_RGBA emissive;    /* Emissive color (self-illumination) */
    float shininess;                     /* Specular shininess exponent */
    float _padding[3];                   /* Alignment */
} D3D8_VULKAN_MATERIAL;

/**
 * Light configuration
 */
typedef struct {
    uint32_t max_directional_lights;    /* Max directional lights (typically 2-4) */
    uint32_t max_point_lights;          /* Max point lights (typically 8-16) */
    uint32_t max_spot_lights;           /* Max spot lights (typically 4-8) */
    uint32_t enable_shadows;            /* Shadow mapping enabled (0=no, 1=yes) */
} D3D8_VULKAN_LIGHTING_CONFIG;

/**
 * Lighting system statistics
 */
typedef struct {
    uint32_t directional_count;         /* Active directional lights */
    uint32_t point_count;               /* Active point lights */
    uint32_t spot_count;                /* Active spot lights */
    uint32_t total_light_updates;       /* Total light updates performed */
    uint32_t material_updates;          /* Total material updates */
    uint32_t ubo_uploads;               /* Total UBO uploads */
} D3D8_VULKAN_LIGHTING_STATS;

/**
 * Light handle (opaque)
 */
typedef struct {
    uint32_t id;
    uint32_t version;
} D3D8_VULKAN_LIGHTING_HANDLE;

/**
 * Lighting system information
 */
typedef struct {
    D3D8_VULKAN_LIGHTING_STATE state;
    D3D8_VULKAN_LIGHTING_CONFIG config;
    D3D8_VULKAN_LIGHTING_STATS stats;
    uint32_t directional_buffer_offset;
    uint32_t point_buffer_offset;
    uint32_t spot_buffer_offset;
    uint32_t material_buffer_offset;
} D3D8_VULKAN_LIGHTING_INFO;

/* ============================================================================
 * API Functions
 * ============================================================================ */

/**
 * Initialize the lighting system
 * Must be called before any other lighting functions
 *
 * @param config Configuration parameters
 * @return Handle to lighting system (version check required)
 */
D3D8_VULKAN_LIGHTING_HANDLE D3D8_Vulkan_InitializeLighting(
    const D3D8_VULKAN_LIGHTING_CONFIG* config
);

/**
 * Shutdown the lighting system
 * Releases all resources
 *
 * @param handle Lighting system handle
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_ShutdownLighting(
    D3D8_VULKAN_LIGHTING_HANDLE handle
);

/**
 * Add a directional light
 * Returns light index for later updates
 *
 * @param handle Lighting system handle
 * @param light Directional light parameters
 * @return Light index (< max_directional_lights), or -1 on failure
 */
int D3D8_Vulkan_AddDirectionalLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    const D3D8_VULKAN_DIRECTIONAL_LIGHT* light
);

/**
 * Add a point light
 * Returns light index for later updates
 *
 * @param handle Lighting system handle
 * @param light Point light parameters
 * @return Light index (< max_point_lights), or -1 on failure
 */
int D3D8_Vulkan_AddPointLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    const D3D8_VULKAN_POINT_LIGHT* light
);

/**
 * Add a spot light
 * Returns light index for later updates
 *
 * @param handle Lighting system handle
 * @param light Spot light parameters
 * @return Light index (< max_spot_lights), or -1 on failure
 */
int D3D8_Vulkan_AddSpotLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    const D3D8_VULKAN_SPOT_LIGHT* light
);

/**
 * Update a directional light
 * Can be called each frame for dynamic lighting
 *
 * @param handle Lighting system handle
 * @param index Light index (from AddDirectionalLight)
 * @param light Updated light parameters
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_UpdateDirectionalLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index,
    const D3D8_VULKAN_DIRECTIONAL_LIGHT* light
);

/**
 * Update a point light
 * Can be called each frame for dynamic lighting
 *
 * @param handle Lighting system handle
 * @param index Light index (from AddPointLight)
 * @param light Updated light parameters
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_UpdatePointLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index,
    const D3D8_VULKAN_POINT_LIGHT* light
);

/**
 * Update a spot light
 * Can be called each frame for dynamic lighting
 *
 * @param handle Lighting system handle
 * @param index Light index (from AddSpotLight)
 * @param light Updated light parameters
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_UpdateSpotLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index,
    const D3D8_VULKAN_SPOT_LIGHT* light
);

/**
 * Remove a directional light
 *
 * @param handle Lighting system handle
 * @param index Light index
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_RemoveDirectionalLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index
);

/**
 * Remove a point light
 *
 * @param handle Lighting system handle
 * @param index Light index
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_RemovePointLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index
);

/**
 * Remove a spot light
 *
 * @param handle Lighting system handle
 * @param index Light index
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_RemoveSpotLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index
);

/**
 * Set ambient light
 *
 * @param handle Lighting system handle
 * @param ambient Ambient light parameters
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_SetAmbientLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    const D3D8_VULKAN_AMBIENT_LIGHT* ambient
);

/**
 * Set material properties for an object
 * Affects how the object responds to lighting
 *
 * @param handle Lighting system handle
 * @param object_id Object identifier (for material tracking)
 * @param material Material properties
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_SetMaterial(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    uint32_t object_id,
    const D3D8_VULKAN_MATERIAL* material
);

/**
 * Get material properties for an object
 *
 * @param handle Lighting system handle
 * @param object_id Object identifier
 * @param material Output material properties
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetMaterial(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    uint32_t object_id,
    D3D8_VULKAN_MATERIAL* material
);

/**
 * Update shader uniform buffers
 * Must be called before rendering to ensure lights are available to shaders
 *
 * @param handle Lighting system handle
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_UpdateLightingUBO(
    D3D8_VULKAN_LIGHTING_HANDLE handle
);

/**
 * Get lighting system statistics
 *
 * @param handle Lighting system handle
 * @param stats Output statistics
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetLightingStats(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    D3D8_VULKAN_LIGHTING_STATS* stats
);

/**
 * Get lighting system information
 *
 * @param handle Lighting system handle
 * @param info Output information
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetLightingInfo(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    D3D8_VULKAN_LIGHTING_INFO* info
);

/**
 * Reset lighting system statistics
 *
 * @param handle Lighting system handle
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_ResetLightingStats(
    D3D8_VULKAN_LIGHTING_HANDLE handle
);

/**
 * Get error message from lighting system
 * Provides detailed information about last error
 *
 * @param handle Lighting system handle
 * @return Error message string (static, do not free)
 */
const char* D3D8_Vulkan_GetLightingError(
    D3D8_VULKAN_LIGHTING_HANDLE handle
);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_LIGHTING_H */
