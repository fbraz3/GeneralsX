/*
 * Phase 20: Viewport & Projection
 * d3d8_vulkan_viewport.h
 *
 * Camera view/projection matrices and viewport management for Vulkan rendering.
 * Supports perspective projection (with Vulkan NDC: Z ∈ [0,1]),
 * orthographic projection for UI, and screen-to-world transformations.
 *
 * Architecture:
 * - View matrix calculation from camera position/orientation
 * - Perspective and orthographic projection matrices
 * - Screen-to-world ray casting
 * - Viewport dynamic adjustment
 * - Camera control and positioning
 * - Matrix stack for scene transformations
 *
 * Handle ranges: 15000+ (non-overlapping with other phases)
 * Cache: 1 entry (single viewport system)
 *
 * Dependencies: Phase 08 (swapchain), Phase 18 (culling)
 */

#ifndef D3D8_VULKAN_VIEWPORT_H
#define D3D8_VULKAN_VIEWPORT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Enumerations
 * ============================================================================ */

/**
 * Viewport system state machine
 */
typedef enum {
    D3D8_VULKAN_VIEWPORT_STATE_UNINITIALIZED = 0,
    D3D8_VULKAN_VIEWPORT_STATE_READY          = 1,
    D3D8_VULKAN_VIEWPORT_STATE_ACTIVE         = 2,
    D3D8_VULKAN_VIEWPORT_STATE_SHUTDOWN       = 3
} D3D8_VULKAN_VIEWPORT_STATE;

/**
 * Projection type enumeration
 */
typedef enum {
    D3D8_VULKAN_PROJECTION_PERSPECTIVE  = 0,
    D3D8_VULKAN_PROJECTION_ORTHOGRAPHIC = 1
} D3D8_VULKAN_PROJECTION_TYPE;

/**
 * Camera movement direction
 */
typedef enum {
    D3D8_VULKAN_CAMERA_FORWARD  = 0,
    D3D8_VULKAN_CAMERA_BACKWARD = 1,
    D3D8_VULKAN_CAMERA_LEFT     = 2,
    D3D8_VULKAN_CAMERA_RIGHT    = 3,
    D3D8_VULKAN_CAMERA_UP       = 4,
    D3D8_VULKAN_CAMERA_DOWN     = 5
} D3D8_VULKAN_CAMERA_DIRECTION;

/* ============================================================================
 * Structures
 * ============================================================================ */

/**
 * 2D point (screen coordinates, texture coordinates)
 */
typedef struct {
    float x;
    float y;
} D3D8_VULKAN_VEC2;

/**
 * 3D vector (position, direction)
 */
typedef struct {
    float x;
    float y;
    float z;
} D3D8_VULKAN_VEC3;

/**
 * 4D vector (homogeneous coordinates, quaternion)
 */
typedef struct {
    float x;
    float y;
    float z;
    float w;
} D3D8_VULKAN_VEC4;

/**
 * 4x4 matrix (row-major for shader compatibility)
 */
typedef struct {
    float m[4][4];
} D3D8_VULKAN_MAT4;

/**
 * Camera properties
 */
typedef struct {
    D3D8_VULKAN_VEC3 position;          /* Camera world position */
    D3D8_VULKAN_VEC3 forward;           /* Forward direction (normalized) */
    D3D8_VULKAN_VEC3 up;                /* Up direction (normalized) */
    D3D8_VULKAN_VEC3 right;             /* Right direction (normalized, computed) */
    float fov;                          /* Field of view in degrees */
    float near_plane;                   /* Near clipping plane distance */
    float far_plane;                    /* Far clipping plane distance */
    float speed;                        /* Movement speed (units per second) */
} D3D8_VULKAN_CAMERA;

/**
 * Viewport properties
 */
typedef struct {
    uint32_t width;                     /* Viewport width in pixels */
    uint32_t height;                    /* Viewport height in pixels */
    float min_depth;                    /* Min depth (Vulkan: 0.0) */
    float max_depth;                    /* Max depth (Vulkan: 1.0) */
    int32_t x;                          /* Viewport X offset */
    int32_t y;                          /* Viewport Y offset */
} D3D8_VULKAN_VIEWPORT;

/**
 * Projection parameters
 */
typedef struct {
    D3D8_VULKAN_PROJECTION_TYPE type;   /* Projection type */
    float fov;                          /* FOV for perspective (degrees) */
    float aspect_ratio;                 /* Aspect ratio (width/height) */
    float near_plane;                   /* Near clipping plane */
    float far_plane;                    /* Far clipping plane */
    float left;                         /* Left for orthographic */
    float right;                        /* Right for orthographic */
    float top;                          /* Top for orthographic */
    float bottom;                       /* Bottom for orthographic */
} D3D8_VULKAN_PROJECTION;

/**
 * Ray for screen-to-world casting
 */
typedef struct {
    D3D8_VULKAN_VEC3 origin;            /* Ray origin (world space) */
    D3D8_VULKAN_VEC3 direction;         /* Ray direction (normalized) */
} D3D8_VULKAN_RAY;

/**
 * Intersection result
 */
typedef struct {
    int hit;                            /* Did we hit (1) or miss (0)? */
    float distance;                     /* Distance along ray to hit point */
    D3D8_VULKAN_VEC3 point;             /* World space hit point */
    D3D8_VULKAN_VEC3 normal;            /* Surface normal at hit point */
} D3D8_VULKAN_INTERSECTION;

/**
 * Viewport system statistics
 */
typedef struct {
    uint32_t view_updates;              /* Total view matrix updates */
    uint32_t projection_updates;        /* Total projection matrix updates */
    uint32_t viewport_changes;          /* Total viewport changes */
    uint32_t camera_movements;          /* Total camera movements */
    uint32_t ray_casts;                 /* Total ray casts performed */
} D3D8_VULKAN_VIEWPORT_STATS;

/**
 * Viewport configuration
 */
typedef struct {
    uint32_t initial_width;             /* Initial viewport width */
    uint32_t initial_height;            /* Initial viewport height */
    float initial_fov;                  /* Initial field of view (degrees) */
    float initial_near;                 /* Initial near plane distance */
    float initial_far;                  /* Initial far plane distance */
    float camera_speed;                 /* Camera movement speed */
} D3D8_VULKAN_VIEWPORT_CONFIG;

/**
 * Viewport handle (opaque)
 */
typedef struct {
    uint32_t id;
    uint32_t version;
} D3D8_VULKAN_VIEWPORT_HANDLE;

/**
 * Viewport system information
 */
typedef struct {
    D3D8_VULKAN_VIEWPORT_STATE state;
    D3D8_VULKAN_CAMERA camera;
    D3D8_VULKAN_VIEWPORT viewport;
    D3D8_VULKAN_PROJECTION projection;
    D3D8_VULKAN_MAT4 view_matrix;
    D3D8_VULKAN_MAT4 projection_matrix;
    D3D8_VULKAN_MAT4 view_projection_matrix;
    D3D8_VULKAN_VIEWPORT_STATS stats;
} D3D8_VULKAN_VIEWPORT_INFO;

/* ============================================================================
 * API Functions
 * ============================================================================ */

/**
 * Initialize the viewport system
 * Must be called before any other viewport functions
 *
 * @param config Configuration parameters
 * @return Handle to viewport system (version check required)
 */
D3D8_VULKAN_VIEWPORT_HANDLE D3D8_Vulkan_InitializeViewport(
    const D3D8_VULKAN_VIEWPORT_CONFIG* config
);

/**
 * Shutdown the viewport system
 * Releases all resources
 *
 * @param handle Viewport system handle
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_ShutdownViewport(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
);

/**
 * Set camera position in world space
 *
 * @param handle Viewport system handle
 * @param position New camera position
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_SetCameraPosition(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC3* position
);

/**
 * Set camera orientation (forward and up vectors)
 *
 * @param handle Viewport system handle
 * @param forward Forward direction (will be normalized)
 * @param up Up direction (will be normalized)
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_SetCameraOrientation(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC3* forward,
    const D3D8_VULKAN_VEC3* up
);

/**
 * Move camera in specified direction
 *
 * @param handle Viewport system handle
 * @param direction Movement direction
 * @param distance Distance to move (world units)
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_MoveCamera(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_CAMERA_DIRECTION direction,
    float distance
);

/**
 * Rotate camera around specified axis
 *
 * @param handle Viewport system handle
 * @param axis Rotation axis (must be normalized)
 * @param angle Rotation angle in radians
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_RotateCamera(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC3* axis,
    float angle
);

/**
 * Get camera position
 *
 * @param handle Viewport system handle
 * @param position Output camera position
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetCameraPosition(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VEC3* position
);

/**
 * Get camera orientation
 *
 * @param handle Viewport system handle
 * @param forward Output forward direction
 * @param up Output up direction
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetCameraOrientation(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VEC3* forward,
    D3D8_VULKAN_VEC3* up
);

/**
 * Set viewport size and position
 * Call after window resize
 *
 * @param handle Viewport system handle
 * @param viewport New viewport parameters
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_SetViewport(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VIEWPORT* viewport
);

/**
 * Get current viewport
 *
 * @param handle Viewport system handle
 * @param viewport Output viewport parameters
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetViewport(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VIEWPORT* viewport
);

/**
 * Set perspective projection
 *
 * @param handle Viewport system handle
 * @param fov Field of view in degrees
 * @param aspect Aspect ratio (width/height)
 * @param near Near clipping plane distance
 * @param far Far clipping plane distance
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_SetPerspectiveProjection(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    float fov,
    float aspect,
    float near,
    float far
);

/**
 * Set orthographic projection (for UI)
 *
 * @param handle Viewport system handle
 * @param left Left coordinate in world space
 * @param right Right coordinate in world space
 * @param top Top coordinate in world space
 * @param bottom Bottom coordinate in world space
 * @param near Near clipping plane distance
 * @param far Far clipping plane distance
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_SetOrthographicProjection(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    float left,
    float right,
    float top,
    float bottom,
    float near,
    float far
);

/**
 * Get view matrix
 *
 * @param handle Viewport system handle
 * @param matrix Output view matrix
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetViewMatrix(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_MAT4* matrix
);

/**
 * Get projection matrix
 *
 * @param handle Viewport system handle
 * @param matrix Output projection matrix
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetProjectionMatrix(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_MAT4* matrix
);

/**
 * Get combined view-projection matrix
 *
 * @param handle Viewport system handle
 * @param matrix Output view-projection matrix
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetViewProjectionMatrix(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_MAT4* matrix
);

/**
 * Cast a ray from screen coordinates to world space
 * Useful for mouse picking and click detection
 *
 * @param handle Viewport system handle
 * @param screen_pos Screen coordinates (0,0 at top-left)
 * @param ray Output ray in world space
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetScreenToWorldRay(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC2* screen_pos,
    D3D8_VULKAN_RAY* ray
);

/**
 * Transform a world space point to screen coordinates
 *
 * @param handle Viewport system handle
 * @param world_pos World space position
 * @param screen_pos Output screen coordinates
 * @return Non-zero on success, 0 on failure (e.g., point behind camera)
 */
int D3D8_Vulkan_GetWorldToScreenPoint(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    const D3D8_VULKAN_VEC3* world_pos,
    D3D8_VULKAN_VEC2* screen_pos
);

/**
 * Update all matrices (should be called once per frame)
 *
 * @param handle Viewport system handle
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_UpdateMatrices(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
);

/**
 * Get viewport system statistics
 *
 * @param handle Viewport system handle
 * @param stats Output statistics
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetViewportStats(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VIEWPORT_STATS* stats
);

/**
 * Get viewport system information
 *
 * @param handle Viewport system handle
 * @param info Output information
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_GetViewportInfo(
    D3D8_VULKAN_VIEWPORT_HANDLE handle,
    D3D8_VULKAN_VIEWPORT_INFO* info
);

/**
 * Reset viewport statistics
 *
 * @param handle Viewport system handle
 * @return Non-zero on success, 0 on failure
 */
int D3D8_Vulkan_ResetViewportStats(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
);

/**
 * Get error message from viewport system
 * Provides detailed information about last error
 *
 * @param handle Viewport system handle
 * @return Error message string (static, do not free)
 */
const char* D3D8_Vulkan_GetViewportError(
    D3D8_VULKAN_VIEWPORT_HANDLE handle
);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_VIEWPORT_H */
