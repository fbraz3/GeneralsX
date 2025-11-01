#pragma once

#include "Core/GameEngine/Source/Camera/camera.h"
#include "Core/GameEngine/Source/Camera/camera_modes.h"
#include "Core/Math/matrix.h"
#include <cstdint>
#include <memory>

/**
 * Phase 45 Stage 4: Render Integration
 * 
 * Integrates camera system with Vulkan render pipeline:
 * - Creates and manages camera uniform buffers
 * - Updates matrices for each frame
 * - Passes matrices to vertex shaders
 * - Handles camera frustum culling
 * - Integrates with Phase 44 material system
 */

/**
 * Camera uniform buffer structure
 * Matches vertex shader layout for GPU memory alignment
 * Total size: 160 bytes (16-byte alignment)
 */
struct CameraUniformBuffer {
    Matrix4x4 view;          // offset 0, 64 bytes
    Matrix4x4 projection;    // offset 64, 64 bytes
    Vector3 cameraPosition;  // offset 128, 12 bytes
    float _padding1;         // offset 140, 4 bytes (alignment)
    Vector3 cameraDirection; // offset 144, 12 bytes
    float _padding2;         // offset 156, 4 bytes (alignment)
    
    // Total: 160 bytes
};

/**
 * Manages camera rendering integration with Vulkan pipeline
 * Responsible for:
 * - Creating/managing camera uniform buffers
 * - Updating matrices each frame
 * - Culling and frustum management
 * - Integration with material system
 */
class GraphicsCamera {
public:
    GraphicsCamera();
    ~GraphicsCamera() = default;

    // =========================================================================
    // Initialization
    // =========================================================================

    /**
     * Initialize graphics camera for rendering
     * Should be called after Vulkan device is created
     * @return true if initialization successful
     */
    bool Initialize();

    /**
     * Shutdown graphics camera
     * Should be called during cleanup phase
     */
    void Shutdown();

    /**
     * Check if graphics camera is initialized
     * @return true if ready for rendering
     */
    bool IsInitialized() const { return m_initialized; }

    // =========================================================================
    // Camera Management
    // =========================================================================

    /**
     * Set camera to use for rendering
     * @param camera Pointer to camera object
     */
    void SetCamera(Camera* camera);

    /**
     * Get current rendering camera
     * @return Pointer to camera
     */
    Camera* GetCamera() { return m_camera; }
    const Camera* GetCamera() const { return m_camera; }

    /**
     * Set camera mode manager
     * For switching between different camera behaviors
     * @param modeManager Pointer to mode manager
     */
    void SetCameraModeManager(CameraModeManager* modeManager);

    /**
     * Get camera mode manager
     * @return Pointer to mode manager
     */
    CameraModeManager* GetCameraModeManager() { return m_cameraModeManager; }
    const CameraModeManager* GetCameraModeManager() const { return m_cameraModeManager; }

    // =========================================================================
    // Uniform Buffer Management
    // =========================================================================

    /**
     * Update camera matrices and uniform buffer
     * Should be called once per frame before rendering
     * @return true if update successful
     */
    bool UpdateUniformBuffer();

    /**
     * Get camera uniform buffer
     * Used for shader binding
     * @return Pointer to uniform buffer structure
     */
    const CameraUniformBuffer& GetUniformBuffer() const { return m_uniformBuffer; }

    /**
     * Get GPU buffer handle for uniform buffer
     * Integration with Phase 44 graphics backend
     * @return VkBuffer handle (opaque pointer)
     */
    uint64_t GetGPUBufferHandle() const { return m_gpuBufferHandle; }

    /**
     * Get GPU buffer memory offset
     * @return Offset in GPU memory
     */
    uint64_t GetGPUBufferOffset() const { return m_gpuBufferOffset; }

    /**
     * Get GPU buffer size
     * @return Size in bytes (always 160)
     */
    uint64_t GetGPUBufferSize() const { return sizeof(CameraUniformBuffer); }

    // =========================================================================
    // Frustum Culling
    // =========================================================================

    /**
     * Check if world point is visible from camera
     * @param worldPoint Point to test in world space
     * @param radius Optional radius for conservative culling
     * @return true if visible
     */
    bool IsSphereInFrustum(const Vector3& worldPoint, float radius = 0.0f) const;

    /**
     * Get camera frustum planes
     * Useful for advanced culling operations
     * @param outPlanes Output array of 6 frustum planes
     */
    void GetFrustumPlanes(Camera::FrustumPlane outPlanes[6]) const;

    /**
     * Check if bounding box is visible
     * @param minCorner Minimum corner of bounding box
     * @param maxCorner Maximum corner of bounding box
     * @return true if any part of box is visible
     */
    bool IsBoxInFrustum(const Vector3& minCorner, const Vector3& maxCorner) const;

    // =========================================================================
    // Projection Management
    // =========================================================================

    /**
     * Set viewport size
     * Updates camera aspect ratio based on viewport
     * @param width Viewport width in pixels
     * @param height Viewport height in pixels
     */
    void SetViewportSize(uint32_t width, uint32_t height);

    /**
     * Get viewport dimensions
     * @param outWidth Output width
     * @param outHeight Output height
     */
    void GetViewportSize(uint32_t& outWidth, uint32_t& outHeight) const;

    /**
     * Set field of view
     * @param fov Field of view in radians (typical: PI/3 = 60 degrees)
     */
    void SetFieldOfView(float fov);

    /**
     * Get field of view
     * @return FOV in radians
     */
    float GetFieldOfView() const;

    /**
     * Set near clipping plane
     * @param nearPlane Distance to near plane
     */
    void SetNearPlane(float nearPlane);

    /**
     * Set far clipping plane
     * @param farPlane Distance to far plane
     */
    void SetFarPlane(float farPlane);

    /**
     * Get near/far plane distances
     * @param outNear Output near plane
     * @param outFar Output far plane
     */
    void GetClippingPlanes(float& outNear, float& outFar) const;

    // =========================================================================
    // Screen Projection
    // =========================================================================

    /**
     * Project world point to screen coordinates
     * Used for UI rendering, picking, etc.
     * @param worldPoint Point in world space
     * @param outScreenX Output screen X (-1 to +1)
     * @param outScreenY Output screen Y (-1 to +1)
     * @return true if point is in front of camera
     */
    bool ProjectToScreen(const Vector3& worldPoint, float& outScreenX, float& outScreenY) const;

    /**
     * Cast ray from camera through screen coordinates
     * Used for mouse picking, raycasting
     * @param screenX Screen X coordinate (-1 to +1)
     * @param screenY Screen Y coordinate (-1 to +1)
     * @return Ray from camera through screen point
     */
    Camera::Ray CastRay(float screenX, float screenY) const;

    /**
     * Convert pixel coordinates to normalized screen coordinates
     * @param pixelX Pixel X coordinate (0 to width)
     * @param pixelY Pixel Y coordinate (0 to height)
     * @param outScreenX Output normalized X (-1 to +1)
     * @param outScreenY Output normalized Y (-1 to +1)
     */
    void PixelToScreenCoordinates(float pixelX, float pixelY, float& outScreenX, float& outScreenY) const;

    /**
     * Convert normalized screen coordinates to pixel coordinates
     * @param screenX Normalized X (-1 to +1)
     * @param screenY Normalized Y (-1 to +1)
     * @param outPixelX Output pixel X
     * @param outPixelY Output pixel Y
     */
    void ScreenToPixelCoordinates(float screenX, float screenY, float& outPixelX, float& outPixelY) const;

    // =========================================================================
    // Debugging & Statistics
    // =========================================================================

    /**
     * Get debug information string
     * Camera position, rotation, FOV, etc.
     * @return Debug information
     */
    const char* GetDebugInfo() const { return m_debugInfo; }

    /**
     * Update debug information
     * Call periodically for performance monitoring
     */
    void UpdateDebugInfo();

    /**
     * Get number of objects culled last frame
     * @return Culled object count
     */
    uint32_t GetCulledObjectCount() const { return m_culledObjectCount; }

    /**
     * Get number of visible objects last frame
     * @return Visible object count
     */
    uint32_t GetVisibleObjectCount() const { return m_visibleObjectCount; }

private:
    // =========================================================================
    // Internal State
    // =========================================================================

    bool m_initialized = false;
    Camera* m_camera = nullptr;                          // Current camera
    CameraModeManager* m_cameraModeManager = nullptr;    // Camera mode switcher

    // Viewport
    uint32_t m_viewportWidth = 1920;
    uint32_t m_viewportHeight = 1080;

    // Uniform buffer
    CameraUniformBuffer m_uniformBuffer;                 // CPU-side buffer
    uint64_t m_gpuBufferHandle = 0;                      // GPU buffer pointer
    uint64_t m_gpuBufferOffset = 0;                      // GPU buffer offset

    // Frustum culling
    Camera::FrustumPlane m_frustumPlanes[6];             // 6 view frustum planes
    uint32_t m_culledObjectCount = 0;
    uint32_t m_visibleObjectCount = 0;

    // Debug
    char m_debugInfo[512];

    // =========================================================================
    // Internal Methods
    // =========================================================================

    /**
     * Create GPU uniform buffer
     * @return true if successful
     */
    bool CreateGPUUniformBuffer();

    /**
     * Destroy GPU uniform buffer
     */
    void DestroyGPUUniformBuffer();

    /**
     * Update GPU uniform buffer data
     * @return true if successful
     */
    bool UpdateGPUUniformBuffer();

    /**
     * Recalculate frustum planes from matrices
     */
    void RecalculateFrustumPlanes();

    /**
     * Update aspect ratio from viewport
     */
    void UpdateAspectRatio();
};
