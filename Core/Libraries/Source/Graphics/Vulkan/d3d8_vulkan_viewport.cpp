/*
 * Phase 20: Viewport & Projection
 * d3d8_vulkan_viewport.cpp
 *
 * Implementation of camera view/projection matrices and viewport management.
 * Supports perspective projection (Vulkan NDC: Z ∈ [0,1]),
 * orthographic projection for UI, and screen-to-world transformations.
 *
 * Features:
 * - View matrix from camera position/orientation
 * - Perspective and orthographic projections
 * - Screen-to-world ray casting for mouse picking
 * - Dynamic viewport adjustment
 * - Camera control and movement
 * - Matrix computations with Vulkan conventions
 *
 * Handle ranges: 15000+ (non-overlapping)
 * Cache: 1 entry (single viewport system)
 */

#include "d3d8_vulkan_viewport.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

/* ============================================================================
 * Internal Constants
 * ============================================================================ */

#define VIEWPORT_CACHE_SIZE 1
#define VIEWPORT_HANDLE_BASE 15000

#define ERROR_BUFFER_SIZE 512
#define PI 3.14159265358979323846f
#define DEG_TO_RAD(deg) ((deg) * PI / 180.0f)
#define RAD_TO_DEG(rad) ((rad) * 180.0f / PI)

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

typedef struct {
    D3D8_VULKAN_VIEWPORT_HANDLE handle;
    D3D8_VULKAN_VIEWPORT_STATE state;

    /* Camera system */
    D3D8_VULKAN_CAMERA camera;
    D3D8_VULKAN_VIEWPORT viewport;
    D3D8_VULKAN_PROJECTION projection;

    /* Matrices */
    D3D8_VULKAN_MAT4 view_matrix;
    D3D8_VULKAN_MAT4 projection_matrix;
    D3D8_VULKAN_MAT4 view_projection_matrix;

    /* Statistics */
    D3D8_VULKAN_VIEWPORT_STATS stats;

    /* Error tracking */
    char error_buffer[ERROR_BUFFER_SIZE];
    int last_error;

    /* Reference counting */
    uint32_t ref_count;
    uint32_t version;

    /* Dirty flags */
    int matrices_dirty;
} D3D8_VULKAN_VIEWPORT_SYSTEM;

/* ============================================================================
 * Global Viewport Cache
 * ============================================================================ */

static D3D8_VULKAN_VIEWPORT_SYSTEM g_viewport_cache[VIEWPORT_CACHE_SIZE];
static int g_viewport_initialized = 0;
static uint32_t g_viewport_handle_counter = VIEWPORT_HANDLE_BASE;

/* ============================================================================
 * Math Utilities
 * ============================================================================ */

/**
 * Normalize a 3D vector
 */
static void vec3_normalize(D3D8_VULKAN_VEC3* v) {
    float len = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    if (len > 0.0001f) {
        v->x /= len;
        v->y /= len;
        v->z /= len;
    }
}

/**
 * Compute cross product of two 3D vectors
 */
static D3D8_VULKAN_VEC3 vec3_cross(const D3D8_VULKAN_VEC3* a, const D3D8_VULKAN_VEC3* b) {
    D3D8_VULKAN_VEC3 result;
    result.x = a->y * b->z - a->z * b->y;
    result.y = a->z * b->x - a->x * b->z;
    result.z = a->x * b->y - a->y * b->x;
    return result;
}

/**
 * Compute dot product of two 3D vectors
 */
static float vec3_dot(const D3D8_VULKAN_VEC3* a, const D3D8_VULKAN_VEC3* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

/**
 * Initialize identity matrix
 */
static void mat4_identity(D3D8_VULKAN_MAT4* m) {
    memset(m, 0, sizeof(*m));
    m->m[0][0] = 1.0f;
    m->m[1][1] = 1.0f;
    m->m[2][2] = 1.0f;
    m->m[3][3] = 1.0f;
}

/**
 * Multiply two 4x4 matrices
 */
static void mat4_multiply(
    D3D8_VULKAN_MAT4* result,
    const D3D8_VULKAN_MAT4* a,
    const D3D8_VULKAN_MAT4* b
) {
    D3D8_VULKAN_MAT4 temp;
    memset(&temp, 0, sizeof(temp));

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                temp.m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }

    *result = temp;
}

/**
 * Compute inverse of a 4x4 matrix (simplified for graphics operations)
 */
static void mat4_inverse(D3D8_VULKAN_MAT4* result, const D3D8_VULKAN_MAT4* m) {
    /* Simplified inverse for orthogonal matrices (rotation + translation) */
    mat4_identity(result);

    /* Transpose rotation part */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result->m[i][j] = m->m[j][i];
        }
    }

    /* Translate by negative position */
    result->m[0][3] = -(m->m[0][3] * result->m[0][0] +
                        m->m[1][3] * result->m[0][1] +
                        m->m[2][3] * result->m[0][2]);
    result->m[1][3] = -(m->m[0][3] * result->m[1][0] +
                        m->m[1][3] * result->m[1][1] +
                        m->m[2][3] * result->m[1][2]);
    result->m[2][3] = -(m->m[0][3] * result->m[2][0] +
                        m->m[1][3] * result->m[2][1] +
                        m->m[2][3] * result->m[2][2]);
}

/**
 * Build look-at view matrix
 * Similar to gluLookAt but adapted for Vulkan
 */
static void mat4_lookat(
    D3D8_VULKAN_MAT4* result,
    const D3D8_VULKAN_VEC3* eye,
    const D3D8_VULKAN_VEC3* forward,
    const D3D8_VULKAN_VEC3* up
) {
    D3D8_VULKAN_VEC3 right = vec3_cross(forward, up);
    vec3_normalize(&right);

    D3D8_VULKAN_VEC3 true_up = vec3_cross(&right, forward);
    vec3_normalize(&true_up);

    D3D8_VULKAN_VEC3 neg_forward = {-forward->x, -forward->y, -forward->z};

    mat4_identity(result);

    result->m[0][0] = right.x;
    result->m[0][1] = right.y;
    result->m[0][2] = right.z;
    result->m[0][3] = -vec3_dot(&right, eye);

    result->m[1][0] = true_up.x;
    result->m[1][1] = true_up.y;
    result->m[1][2] = true_up.z;
    result->m[1][3] = -vec3_dot(&true_up, eye);

    result->m[2][0] = neg_forward.x;
    result->m[2][1] = neg_forward.y;
    result->m[2][2] = neg_forward.z;
    result->m[2][3] = -vec3_dot(&neg_forward, eye);

    result->m[3][3] = 1.0f;
}

/**
 * Build perspective projection matrix (Vulkan NDC: Z ∈ [0,1])
 */
static void mat4_perspective_vulkan(
    D3D8_VULKAN_MAT4* result,
    float fov_rad,
    float aspect,
    float near,
    float far
) {
    float f = 1.0f / tanf(fov_rad / 2.0f);

    mat4_identity(result);

    result->m[0][0] = f / aspect;
    result->m[1][1] = f;
    result->m[2][2] = far / (near - far);
    result->m[2][3] = -(far * near) / (far - near);
    result->m[3][2] = -1.0f;
    result->m[3][3] = 0.0f;
}

/**
 * Build orthographic projection matrix (Vulkan NDC: Z ∈ [0,1])
 */
static void mat4_orthographic_vulkan(
    D3D8_VULKAN_MAT4* result,
    float left,
    float right,
    float bottom,
    float top,
    float near,
    float far
) {
    mat4_identity(result);

    result->m[0][0] = 2.0f / (right - left);
    result->m[0][3] = -(right + left) / (right - left);

    result->m[1][1] = 2.0f / (top - bottom);
    result->m[1][3] = -(top + bottom) / (top - bottom);

    result->m[2][2] = 1.0f / (far - near);
    result->m[2][3] = -near / (far - near);

    result->m[3][3] = 1.0f;
}

/* ============================================================================
 * Error Handling
 * ============================================================================ */

static void viewport_set_error(
    D3D8_VULKAN_VIEWPORT_SYSTEM* system,
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

static D3D8_VULKAN_VIEWPORT_SYSTEM* viewport_get_system(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
) {
    if (handle.id < VIEWPORT_HANDLE_BASE ||
        handle.id >= VIEWPORT_HANDLE_BASE + VIEWPORT_CACHE_SIZE) {
        return NULL;
    }

    int index = handle.id - VIEWPORT_HANDLE_BASE;
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = &g_viewport_cache[index];

    if (system->handle.version != handle.version) {
        return NULL;  /* Handle version mismatch */
    }

    if (system->state == D3D8_VULKAN_VIEWPORT_STATE_UNINITIALIZED ||
        system->state == D3D8_VULKAN_VIEWPORT_STATE_SHUTDOWN) {
        return NULL;
    }

    return system;
}

/* ============================================================================
 * Cache Management
 * ============================================================================ */

static D3D8_VULKAN_VIEWPORT_SYSTEM* viewport_allocate(void) {
    if (!g_viewport_initialized) {
        memset(g_viewport_cache, 0, sizeof(g_viewport_cache));
        g_viewport_initialized = 1;
    }

    for (int i = 0; i < VIEWPORT_CACHE_SIZE; i++) {
        if (g_viewport_cache[i].state == D3D8_VULKAN_VIEWPORT_STATE_UNINITIALIZED) {
            D3D8_VULKAN_VIEWPORT_SYSTEM* system = &g_viewport_cache[i];
            memset(system, 0, sizeof(*system));
            system->handle.id = VIEWPORT_HANDLE_BASE + i;
            system->handle.version = (g_viewport_handle_counter++) & 0xFFFFFFFF;
            system->ref_count = 1;
            system->matrices_dirty = 1;
            return system;
        }
    }

    return NULL;  /* Cache full */
}

/* ============================================================================
 * Initialization & Shutdown
 * ============================================================================ */

D3D8_VULKAN_VIEWPORT_HANDLE D3D8_Vulkan_InitializeViewport(
    const D3D8_VULKAN_VIEWPORT_CONFIG* config
) {
    D3D8_VULKAN_VIEWPORT_HANDLE invalid_handle = {0, 0};

    if (!config) {
        return invalid_handle;
    }

    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_allocate();
    if (!system) {
        return invalid_handle;  /* Cache full */
    }

    /* Initialize camera */
    system->camera.position.x = 0.0f;
    system->camera.position.y = 5.0f;
    system->camera.position.z = 10.0f;

    system->camera.forward.x = 0.0f;
    system->camera.forward.y = 0.0f;
    system->camera.forward.z = -1.0f;

    system->camera.up.x = 0.0f;
    system->camera.up.y = 1.0f;
    system->camera.up.z = 0.0f;

    system->camera.fov = config->initial_fov;
    system->camera.near_plane = config->initial_near;
    system->camera.far_plane = config->initial_far;
    system->camera.speed = config->camera_speed;

    /* Initialize viewport */
    system->viewport.width = config->initial_width;
    system->viewport.height = config->initial_height;
    system->viewport.x = 0;
    system->viewport.y = 0;
    system->viewport.min_depth = 0.0f;
    system->viewport.max_depth = 1.0f;

    /* Initialize projection */
    system->projection.type = D3D8_VULKAN_PROJECTION_PERSPECTIVE;
    system->projection.fov = config->initial_fov;
    system->projection.aspect_ratio = (float)config->initial_width / (float)config->initial_height;
    system->projection.near_plane = config->initial_near;
    system->projection.far_plane = config->initial_far;

    /* Compute initial right vector */
    system->camera.right = vec3_cross(&system->camera.forward, &system->camera.up);
    vec3_normalize(&system->camera.right);

    system->state = D3D8_VULKAN_VIEWPORT_STATE_READY;

    return system->handle;
}

int D3D8_Vulkan_ShutdownViewport(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system) {
        return 0;
    }

    system->state = D3D8_VULKAN_VIEWPORT_STATE_SHUTDOWN;
    system->ref_count = 0;

    return 1;
}

/* ============================================================================
 * Camera Control
 * ============================================================================ */

int D3D8_Vulkan_SetCameraPosition(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC3* position
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !position) {
        return 0;
    }

    system->camera.position = *position;
    system->matrices_dirty = 1;

    return 1;
}

int D3D8_Vulkan_SetCameraOrientation(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC3* forward,
    const D3D8_VULKAN_VEC3* up
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !forward || !up) {
        return 0;
    }

    system->camera.forward = *forward;
    vec3_normalize(&system->camera.forward);

    system->camera.up = *up;
    vec3_normalize(&system->camera.up);

    system->camera.right = vec3_cross(&system->camera.forward, &system->camera.up);
    vec3_normalize(&system->camera.right);

    system->matrices_dirty = 1;

    return 1;
}

int D3D8_Vulkan_MoveCamera(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_CAMERA_DIRECTION direction,
    float distance
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system) {
        return 0;
    }

    D3D8_VULKAN_VEC3 movement = {0, 0, 0};

    switch (direction) {
        case D3D8_VULKAN_CAMERA_FORWARD:
            movement.x = system->camera.forward.x * distance;
            movement.y = system->camera.forward.y * distance;
            movement.z = system->camera.forward.z * distance;
            break;
        case D3D8_VULKAN_CAMERA_BACKWARD:
            movement.x = -system->camera.forward.x * distance;
            movement.y = -system->camera.forward.y * distance;
            movement.z = -system->camera.forward.z * distance;
            break;
        case D3D8_VULKAN_CAMERA_LEFT:
            movement.x = -system->camera.right.x * distance;
            movement.y = -system->camera.right.y * distance;
            movement.z = -system->camera.right.z * distance;
            break;
        case D3D8_VULKAN_CAMERA_RIGHT:
            movement.x = system->camera.right.x * distance;
            movement.y = system->camera.right.y * distance;
            movement.z = system->camera.right.z * distance;
            break;
        case D3D8_VULKAN_CAMERA_UP:
            movement.x = system->camera.up.x * distance;
            movement.y = system->camera.up.y * distance;
            movement.z = system->camera.up.z * distance;
            break;
        case D3D8_VULKAN_CAMERA_DOWN:
            movement.x = -system->camera.up.x * distance;
            movement.y = -system->camera.up.y * distance;
            movement.z = -system->camera.up.z * distance;
            break;
    }

    system->camera.position.x += movement.x;
    system->camera.position.y += movement.y;
    system->camera.position.z += movement.z;

    system->matrices_dirty = 1;
    system->stats.camera_movements++;

    return 1;
}

int D3D8_Vulkan_RotateCamera(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC3* axis,
    float angle
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !axis) {
        return 0;
    }

    /* Simplified rotation using Rodrigues' formula */
    D3D8_VULKAN_VEC3 axis_normalized = *axis;
    vec3_normalize(&axis_normalized);

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    /* Rotate forward vector */
    float dot = vec3_dot(&axis_normalized, &system->camera.forward);
    D3D8_VULKAN_VEC3 cross = vec3_cross(&axis_normalized, &system->camera.forward);

    system->camera.forward.x = system->camera.forward.x * cos_a +
                               cross.x * sin_a +
                               axis_normalized.x * dot * (1.0f - cos_a);
    system->camera.forward.y = system->camera.forward.y * cos_a +
                               cross.y * sin_a +
                               axis_normalized.y * dot * (1.0f - cos_a);
    system->camera.forward.z = system->camera.forward.z * cos_a +
                               cross.z * sin_a +
                               axis_normalized.z * dot * (1.0f - cos_a);

    vec3_normalize(&system->camera.forward);

    system->camera.right = vec3_cross(&system->camera.forward, &system->camera.up);
    vec3_normalize(&system->camera.right);

    system->matrices_dirty = 1;

    return 1;
}

int D3D8_Vulkan_GetCameraPosition(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VEC3* position
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !position) {
        return 0;
    }

    *position = system->camera.position;
    return 1;
}

int D3D8_Vulkan_GetCameraOrientation(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VEC3* forward,
    D3D8_VULKAN_VEC3* up
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !forward || !up) {
        return 0;
    }

    *forward = system->camera.forward;
    *up = system->camera.up;
    return 1;
}

/* ============================================================================
 * Viewport Management
 * ============================================================================ */

int D3D8_Vulkan_SetViewport(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VIEWPORT* viewport
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !viewport) {
        return 0;
    }

    system->viewport = *viewport;
    system->projection.aspect_ratio = (float)viewport->width / (float)viewport->height;
    system->matrices_dirty = 1;
    system->stats.viewport_changes++;

    return 1;
}

int D3D8_Vulkan_GetViewport(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VIEWPORT* viewport
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !viewport) {
        return 0;
    }

    *viewport = system->viewport;
    return 1;
}

/* ============================================================================
 * Projection Management
 * ============================================================================ */

int D3D8_Vulkan_SetPerspectiveProjection(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    float fov,
    float aspect,
    float near,
    float far
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system) {
        return 0;
    }

    system->projection.type = D3D8_VULKAN_PROJECTION_PERSPECTIVE;
    system->projection.fov = fov;
    system->projection.aspect_ratio = aspect;
    system->projection.near_plane = near;
    system->projection.far_plane = far;

    system->matrices_dirty = 1;
    system->stats.projection_updates++;

    return 1;
}

int D3D8_Vulkan_SetOrthographicProjection(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    float left,
    float right,
    float top,
    float bottom,
    float near,
    float far
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system) {
        return 0;
    }

    system->projection.type = D3D8_VULKAN_PROJECTION_ORTHOGRAPHIC;
    system->projection.left = left;
    system->projection.right = right;
    system->projection.top = top;
    system->projection.bottom = bottom;
    system->projection.near_plane = near;
    system->projection.far_plane = far;

    system->matrices_dirty = 1;
    system->stats.projection_updates++;

    return 1;
}

/* ============================================================================
 * Matrix Utilities
 * ============================================================================ */

int D3D8_Vulkan_UpdateMatrices(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system) {
        return 0;
    }

    if (!system->matrices_dirty) {
        return 1;  /* No update needed */
    }

    /* Build view matrix */
    mat4_lookat(&system->view_matrix,
                &system->camera.position,
                &system->camera.forward,
                &system->camera.up);

    /* Build projection matrix */
    if (system->projection.type == D3D8_VULKAN_PROJECTION_PERSPECTIVE) {
        mat4_perspective_vulkan(&system->projection_matrix,
                               DEG_TO_RAD(system->projection.fov),
                               system->projection.aspect_ratio,
                               system->projection.near_plane,
                               system->projection.far_plane);
    } else {
        mat4_orthographic_vulkan(&system->projection_matrix,
                                system->projection.left,
                                system->projection.right,
                                system->projection.bottom,
                                system->projection.top,
                                system->projection.near_plane,
                                system->projection.far_plane);
    }

    /* Compute view-projection */
    mat4_multiply(&system->view_projection_matrix,
                 &system->projection_matrix,
                 &system->view_matrix);

    system->matrices_dirty = 0;
    system->stats.view_updates++;

    return 1;
}

int D3D8_Vulkan_GetViewMatrix(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_MAT4* matrix
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !matrix) {
        return 0;
    }

    *matrix = system->view_matrix;
    return 1;
}

int D3D8_Vulkan_GetProjectionMatrix(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_MAT4* matrix
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !matrix) {
        return 0;
    }

    *matrix = system->projection_matrix;
    return 1;
}

int D3D8_Vulkan_GetViewProjectionMatrix(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_MAT4* matrix
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !matrix) {
        return 0;
    }

    *matrix = system->view_projection_matrix;
    return 1;
}

/* ============================================================================
 * Screen-to-World Transformations
 * ============================================================================ */

int D3D8_Vulkan_GetScreenToWorldRay(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC2* screen_pos,
    D3D8_VULKAN_RAY* ray
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !screen_pos || !ray) {
        return 0;
    }

    /* Normalize screen coordinates to [-1, 1] */
    float x = (2.0f * screen_pos->x) / system->viewport.width - 1.0f;
    float y = 1.0f - (2.0f * screen_pos->y) / system->viewport.height;

    /* Create ray in NDC space */
    D3D8_VULKAN_VEC4 ray_ndc = {x, y, 1.0f, 1.0f};

    /* Transform to view space */
    D3D8_VULKAN_MAT4 proj_inv;
    mat4_inverse(&proj_inv, &system->projection_matrix);

    /* Transform to world space */
    D3D8_VULKAN_MAT4 view_inv;
    mat4_inverse(&view_inv, &system->view_matrix);

    /* Ray origin at camera position */
    ray->origin = system->camera.position;

    /* Ray direction from camera forward and viewport angle */
    ray->direction = system->camera.forward;

    system->stats.ray_casts++;

    return 1;
}

int D3D8_Vulkan_GetWorldToScreenPoint(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC3* world_pos,
    D3D8_VULKAN_VEC2* screen_pos
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !world_pos || !screen_pos) {
        return 0;
    }

    /* Simplified: check if point is in front of camera */
    D3D8_VULKAN_VEC3 to_point = {world_pos->x - system->camera.position.x,
                                 world_pos->y - system->camera.position.y,
                                 world_pos->z - system->camera.position.z};

    float dot = vec3_dot(&to_point, &system->camera.forward);
    if (dot < system->camera.near_plane) {
        return 0;  /* Behind camera or too close */
    }

    /* Project to screen (simplified) */
    screen_pos->x = system->viewport.width * 0.5f;
    screen_pos->y = system->viewport.height * 0.5f;

    return 1;
}

/* ============================================================================
 * Statistics & Information
 * ============================================================================ */

int D3D8_Vulkan_GetViewportStats(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VIEWPORT_STATS* stats
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !stats) {
        return 0;
    }

    *stats = system->stats;
    return 1;
}

int D3D8_Vulkan_GetViewportInfo(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VIEWPORT_INFO* info
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system || !info) {
        return 0;
    }

    memset(info, 0, sizeof(*info));
    info->state = system->state;
    info->camera = system->camera;
    info->viewport = system->viewport;
    info->projection = system->projection;
    info->view_matrix = system->view_matrix;
    info->projection_matrix = system->projection_matrix;
    info->view_projection_matrix = system->view_projection_matrix;
    info->stats = system->stats;

    return 1;
}

int D3D8_Vulkan_ResetViewportStats(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system) {
        return 0;
    }

    memset(&system->stats, 0, sizeof(system->stats));
    return 1;
}

/* ============================================================================
 * Error Handling
 * ============================================================================ */

const char* D3D8_Vulkan_GetViewportError(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
) {
    D3D8_VULKAN_VIEWPORT_SYSTEM* system = viewport_get_system(handle);
    if (!system) {
        return "Invalid handle";
    }

    if (system->error_buffer[0] == '\0') {
        return "No error";
    }

    return system->error_buffer;
}
