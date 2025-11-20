/*
 * Phase 19: Lighting System
 * d3d8_vulkan_lighting.cpp
 *
 * Implementation of lighting calculations and shader integration.
 * Manages multiple light types with per-object contributions and dynamic updates.
 *
 * Features:
 * - 4 light types: directional, point, spot, ambient
 * - Attenuation models: none, linear, quadratic, exponential
 * - Per-object material properties
 * - Shader uniform buffer objects
 * - Dynamic light updates
 * - Reference counting and validation
 *
 * Handle ranges: 14000+ (non-overlapping)
 * Cache: 2 entries
 */

#include "d3d8_vulkan_lighting.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

/* ============================================================================
 * Internal Constants
 * ============================================================================ */

#define LIGHTING_CACHE_SIZE 2
#define LIGHTING_HANDLE_BASE 14000

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 8
#define MAX_MATERIALS 512
#define MAX_LIGHT_UPDATES_PER_FRAME 1000

#define ERROR_BUFFER_SIZE 512

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

typedef struct {
    D3D8_VULKAN_LIGHTING_HANDLE handle;
    D3D8_VULKAN_LIGHTING_STATE state;
    D3D8_VULKAN_LIGHTING_CONFIG config;
    D3D8_VULKAN_LIGHTING_STATS stats;

    /* Light storage */
    D3D8_VULKAN_DIRECTIONAL_LIGHT directional_lights[MAX_DIRECTIONAL_LIGHTS];
    D3D8_VULKAN_POINT_LIGHT point_lights[MAX_POINT_LIGHTS];
    D3D8_VULKAN_SPOT_LIGHT spot_lights[MAX_SPOT_LIGHTS];
    D3D8_VULKAN_AMBIENT_LIGHT ambient_light;

    /* Material storage */
    D3D8_VULKAN_MATERIAL materials[MAX_MATERIALS];

    /* Error tracking */
    char error_buffer[ERROR_BUFFER_SIZE];
    int last_error;

    /* Reference counting */
    uint32_t ref_count;
    uint32_t version;
} D3D8_VULKAN_LIGHTING_SYSTEM;

/* ============================================================================
 * Global Lighting Cache
 * ============================================================================ */

static D3D8_VULKAN_LIGHTING_SYSTEM g_lighting_cache[LIGHTING_CACHE_SIZE];
static int g_lighting_initialized = 0;
static uint32_t g_lighting_handle_counter = LIGHTING_HANDLE_BASE;

/* ============================================================================
 * Error Handling
 * ============================================================================ */

static void lighting_set_error(
    D3D8_VULKAN_LIGHTING_SYSTEM* system,
    int error_code,
    const char* format,
    ...
) {
    if (!system) return;

    system->last_error = error_code;

    va_list args;
    va_start(args, format);
    vsnprintf(system->error_buffer, ERROR_BUFFER_SIZE, format, args);
    va_end(args);
}

/* ============================================================================
 * Handle Validation
 * ============================================================================ */

static D3D8_VULKAN_LIGHTING_SYSTEM* lighting_get_system(
    D3D8_VULKAN_LIGHTING_HANDLE handle
) {
    if (handle.id < LIGHTING_HANDLE_BASE ||
        handle.id >= LIGHTING_HANDLE_BASE + LIGHTING_CACHE_SIZE) {
        return NULL;
    }

    int index = handle.id - LIGHTING_HANDLE_BASE;
    D3D8_VULKAN_LIGHTING_SYSTEM* system = &g_lighting_cache[index];

    if (system->handle.version != handle.version) {
        return NULL;  /* Handle version mismatch */
    }

    if (system->state == D3D8_VULKAN_LIGHTING_STATE_UNINITIALIZED ||
        system->state == D3D8_VULKAN_LIGHTING_STATE_SHUTDOWN) {
        return NULL;
    }

    return system;
}

/* ============================================================================
 * Cache Management
 * ============================================================================ */

static D3D8_VULKAN_LIGHTING_SYSTEM* lighting_allocate(void) {
    if (!g_lighting_initialized) {
        memset(g_lighting_cache, 0, sizeof(g_lighting_cache));
        g_lighting_initialized = 1;
    }

    for (int i = 0; i < LIGHTING_CACHE_SIZE; i++) {
        if (g_lighting_cache[i].state == D3D8_VULKAN_LIGHTING_STATE_UNINITIALIZED) {
            D3D8_VULKAN_LIGHTING_SYSTEM* system = &g_lighting_cache[i];
            memset(system, 0, sizeof(*system));
            system->handle.id = LIGHTING_HANDLE_BASE + i;
            system->handle.version = (g_lighting_handle_counter++) & 0xFFFFFFFF;
            system->ref_count = 1;
            return system;
        }
    }

    return NULL;  /* Cache full */
}

/* ============================================================================
 * Initialization & Shutdown
 * ============================================================================ */

D3D8_VULKAN_LIGHTING_HANDLE D3D8_Vulkan_InitializeLighting(
    const D3D8_VULKAN_LIGHTING_CONFIG* config
) {
    D3D8_VULKAN_LIGHTING_HANDLE invalid_handle = {0, 0};

    if (!config) {
        return invalid_handle;
    }

    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_allocate();
    if (!system) {
        return invalid_handle;  /* Cache full */
    }

    /* Validate configuration */
    if (config->max_directional_lights > MAX_DIRECTIONAL_LIGHTS ||
        config->max_point_lights > MAX_POINT_LIGHTS ||
        config->max_spot_lights > MAX_SPOT_LIGHTS) {
        system->state = D3D8_VULKAN_LIGHTING_STATE_UNINITIALIZED;
        return invalid_handle;
    }

    /* Initialize system */
    system->config = *config;
    system->state = D3D8_VULKAN_LIGHTING_STATE_READY;

    /* Initialize ambient light to neutral */
    system->ambient_light.color.r = 0.2f;
    system->ambient_light.color.g = 0.2f;
    system->ambient_light.color.b = 0.2f;
    system->ambient_light.color.a = 1.0f;
    system->ambient_light.intensity = 1.0f;

    /* Initialize materials with default values */
    for (int i = 0; i < MAX_MATERIALS; i++) {
        system->materials[i].diffuse.r = 0.8f;
        system->materials[i].diffuse.g = 0.8f;
        system->materials[i].diffuse.b = 0.8f;
        system->materials[i].diffuse.a = 1.0f;
        system->materials[i].specular.r = 0.5f;
        system->materials[i].specular.g = 0.5f;
        system->materials[i].specular.b = 0.5f;
        system->materials[i].specular.a = 1.0f;
        system->materials[i].shininess = 32.0f;
    }

    return system->handle;
}

int D3D8_Vulkan_ShutdownLighting(
    D3D8_VULKAN_LIGHTING_HANDLE handle
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system) {
        return 0;
    }

    system->state = D3D8_VULKAN_LIGHTING_STATE_SHUTDOWN;
    system->ref_count = 0;

    return 1;
}

/* ============================================================================
 * Directional Light Management
 * ============================================================================ */

int D3D8_Vulkan_AddDirectionalLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    const D3D8_VULKAN_DIRECTIONAL_LIGHT* light
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !light) {
        return -1;
    }

    if (system->stats.directional_count >= system->config.max_directional_lights) {
        lighting_set_error(system, 1, "Directional light limit reached");
        return -1;
    }

    int index = system->stats.directional_count;
    system->directional_lights[index] = *light;
    system->stats.directional_count++;
    system->stats.total_light_updates++;

    return index;
}

int D3D8_Vulkan_UpdateDirectionalLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index,
    const D3D8_VULKAN_DIRECTIONAL_LIGHT* light
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !light || index < 0) {
        return 0;
    }

    if (index >= system->stats.directional_count) {
        lighting_set_error(system, 2, "Directional light index out of range");
        return 0;
    }

    system->directional_lights[index] = *light;
    system->stats.total_light_updates++;

    return 1;
}

int D3D8_Vulkan_RemoveDirectionalLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || index < 0) {
        return 0;
    }

    if (index >= system->stats.directional_count) {
        lighting_set_error(system, 3, "Directional light index out of range");
        return 0;
    }

    /* Swap with last light and decrement count */
    if (index < system->stats.directional_count - 1) {
        system->directional_lights[index] =
            system->directional_lights[system->stats.directional_count - 1];
    }
    system->stats.directional_count--;

    return 1;
}

/* ============================================================================
 * Point Light Management
 * ============================================================================ */

int D3D8_Vulkan_AddPointLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    const D3D8_VULKAN_POINT_LIGHT* light
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !light) {
        return -1;
    }

    if (system->stats.point_count >= system->config.max_point_lights) {
        lighting_set_error(system, 4, "Point light limit reached");
        return -1;
    }

    int index = system->stats.point_count;
    system->point_lights[index] = *light;
    system->stats.point_count++;
    system->stats.total_light_updates++;

    return index;
}

int D3D8_Vulkan_UpdatePointLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index,
    const D3D8_VULKAN_POINT_LIGHT* light
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !light || index < 0) {
        return 0;
    }

    if (index >= system->stats.point_count) {
        lighting_set_error(system, 5, "Point light index out of range");
        return 0;
    }

    system->point_lights[index] = *light;
    system->stats.total_light_updates++;

    return 1;
}

int D3D8_Vulkan_RemovePointLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || index < 0) {
        return 0;
    }

    if (index >= system->stats.point_count) {
        lighting_set_error(system, 6, "Point light index out of range");
        return 0;
    }

    /* Swap with last light and decrement count */
    if (index < system->stats.point_count - 1) {
        system->point_lights[index] =
            system->point_lights[system->stats.point_count - 1];
    }
    system->stats.point_count--;

    return 1;
}

/* ============================================================================
 * Spot Light Management
 * ============================================================================ */

int D3D8_Vulkan_AddSpotLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    const D3D8_VULKAN_SPOT_LIGHT* light
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !light) {
        return -1;
    }

    if (system->stats.spot_count >= system->config.max_spot_lights) {
        lighting_set_error(system, 7, "Spot light limit reached");
        return -1;
    }

    int index = system->stats.spot_count;
    system->spot_lights[index] = *light;
    system->stats.spot_count++;
    system->stats.total_light_updates++;

    return index;
}

int D3D8_Vulkan_UpdateSpotLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index,
    const D3D8_VULKAN_SPOT_LIGHT* light
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !light || index < 0) {
        return 0;
    }

    if (index >= system->stats.spot_count) {
        lighting_set_error(system, 8, "Spot light index out of range");
        return 0;
    }

    system->spot_lights[index] = *light;
    system->stats.total_light_updates++;

    return 1;
}

int D3D8_Vulkan_RemoveSpotLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    int index
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || index < 0) {
        return 0;
    }

    if (index >= system->stats.spot_count) {
        lighting_set_error(system, 9, "Spot light index out of range");
        return 0;
    }

    /* Swap with last light and decrement count */
    if (index < system->stats.spot_count - 1) {
        system->spot_lights[index] =
            system->spot_lights[system->stats.spot_count - 1];
    }
    system->stats.spot_count--;

    return 1;
}

/* ============================================================================
 * Ambient Light & Material Management
 * ============================================================================ */

int D3D8_Vulkan_SetAmbientLight(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    const D3D8_VULKAN_AMBIENT_LIGHT* ambient
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !ambient) {
        return 0;
    }

    system->ambient_light = *ambient;
    system->stats.total_light_updates++;

    return 1;
}

int D3D8_Vulkan_SetMaterial(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    uint32_t object_id,
    const D3D8_VULKAN_MATERIAL* material
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !material) {
        return 0;
    }

    if (object_id >= MAX_MATERIALS) {
        lighting_set_error(system, 10, "Object ID out of range");
        return 0;
    }

    system->materials[object_id] = *material;
    system->stats.material_updates++;

    return 1;
}

int D3D8_Vulkan_GetMaterial(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    uint32_t object_id,
    D3D8_VULKAN_MATERIAL* material
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !material) {
        return 0;
    }

    if (object_id >= MAX_MATERIALS) {
        lighting_set_error(system, 11, "Object ID out of range");
        return 0;
    }

    *material = system->materials[object_id];

    return 1;
}

/* ============================================================================
 * Shader Integration
 * ============================================================================ */

int D3D8_Vulkan_UpdateLightingUBO(
    D3D8_VULKAN_LIGHTING_HANDLE handle
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system) {
        return 0;
    }

    /* In a real implementation, this would:
     * 1. Map the UBO buffer
     * 2. Copy light data to GPU
     * 3. Unmap the buffer
     *
     * For now, we just track that the upload was requested
     */
    system->stats.ubo_uploads++;

    return 1;
}

/* ============================================================================
 * Statistics & Information
 * ============================================================================ */

int D3D8_Vulkan_GetLightingStats(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    D3D8_VULKAN_LIGHTING_STATS* stats
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !stats) {
        return 0;
    }

    *stats = system->stats;

    return 1;
}

int D3D8_Vulkan_GetLightingInfo(
    D3D8_VULKAN_LIGHTING_HANDLE handle,
    D3D8_VULKAN_LIGHTING_INFO* info
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system || !info) {
        return 0;
    }

    memset(info, 0, sizeof(*info));
    info->state = system->state;
    info->config = system->config;
    info->stats = system->stats;
    info->directional_buffer_offset = 0;
    info->point_buffer_offset = system->config.max_directional_lights *
                                sizeof(D3D8_VULKAN_DIRECTIONAL_LIGHT);
    info->spot_buffer_offset = info->point_buffer_offset +
                              system->config.max_point_lights *
                              sizeof(D3D8_VULKAN_POINT_LIGHT);
    info->material_buffer_offset = info->spot_buffer_offset +
                                   system->config.max_spot_lights *
                                   sizeof(D3D8_VULKAN_SPOT_LIGHT);

    return 1;
}

int D3D8_Vulkan_ResetLightingStats(
    D3D8_VULKAN_LIGHTING_HANDLE handle
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system) {
        return 0;
    }

    memset(&system->stats, 0, sizeof(system->stats));
    system->stats.directional_count = 0;
    system->stats.point_count = 0;
    system->stats.spot_count = 0;

    return 1;
}

/* ============================================================================
 * Error Handling
 * ============================================================================ */

const char* D3D8_Vulkan_GetLightingError(
    D3D8_VULKAN_LIGHTING_HANDLE handle
) {
    D3D8_VULKAN_LIGHTING_SYSTEM* system = lighting_get_system(handle);
    if (!system) {
        return "Invalid handle";
    }

    if (system->error_buffer[0] == '\0') {
        return "No error";
    }

    return system->error_buffer;
}
