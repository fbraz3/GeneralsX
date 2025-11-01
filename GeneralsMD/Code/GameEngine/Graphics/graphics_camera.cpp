#include "graphics_camera.h"
#include "Core/Math/math_utils.h"
#include <cstring>
#include <cstdio>

/**
 * Phase 45 Stage 4: Render Integration Implementation
 * 
 * Integrates camera with Vulkan render pipeline
 * Manages uniform buffers and frustum culling
 */

// ============================================================================
// Constructor & Initialization
// ============================================================================

GraphicsCamera::GraphicsCamera() {
    std::memset(&m_uniformBuffer, 0, sizeof(CameraUniformBuffer));
    std::memset(m_debugInfo, 0, sizeof(m_debugInfo));
    std::memset(m_frustumPlanes, 0, sizeof(m_frustumPlanes));
}

bool GraphicsCamera::Initialize() {
    if (m_initialized) {
        return true;
    }

    // Create default camera if not set
    if (!m_camera) {
        // Note: In real implementation, would use global camera
        // For now, just mark as requiring external camera
        return false;
    }

    // Create GPU uniform buffer
    if (!CreateGPUUniformBuffer()) {
        return false;
    }

    // Initial update
    if (!UpdateUniformBuffer()) {
        return false;
    }

    m_initialized = true;
    return true;
}

void GraphicsCamera::Shutdown() {
    if (!m_initialized) {
        return;
    }

    DestroyGPUUniformBuffer();
    m_initialized = false;
    m_camera = nullptr;
    m_cameraModeManager = nullptr;
}

// ============================================================================
// Camera Management
// ============================================================================

void GraphicsCamera::SetCamera(Camera* camera) {
    m_camera = camera;
}

void GraphicsCamera::SetCameraModeManager(CameraModeManager* modeManager) {
    m_cameraModeManager = modeManager;
}

// ============================================================================
// Uniform Buffer Management
// ============================================================================

bool GraphicsCamera::UpdateUniformBuffer() {
    if (!m_initialized || !m_camera) {
        return false;
    }

    // Update camera matrices
    m_camera->UpdateMatrices();

    // Copy matrices to uniform buffer
    m_uniformBuffer.view = m_camera->GetViewMatrix();
    m_uniformBuffer.projection = m_camera->GetProjectionMatrix();
    m_uniformBuffer.cameraPosition = m_camera->GetPosition();
    m_uniformBuffer.cameraDirection = m_camera->GetForward();

    // Upload to GPU
    if (!UpdateGPUUniformBuffer()) {
        return false;
    }

    // Recalculate frustum planes for culling
    RecalculateFrustumPlanes();

    return true;
}

// ============================================================================
// Frustum Culling
// ============================================================================

bool GraphicsCamera::IsSphereInFrustum(const Vector3& worldPoint, float radius) const {
    if (!m_camera) {
        return false;
    }

    return m_camera->IsPointInFrustum(worldPoint, radius);
}

void GraphicsCamera::GetFrustumPlanes(Camera::FrustumPlane outPlanes[6]) const {
    std::memcpy(outPlanes, m_frustumPlanes, sizeof(m_frustumPlanes));
}

bool GraphicsCamera::IsBoxInFrustum(const Vector3& minCorner, const Vector3& maxCorner) const {
    if (!m_camera) {
        return false;
    }

    // Check if any corner of the box is in frustum
    Vector3 corners[8] = {
        Vector3(minCorner.x, minCorner.y, minCorner.z),
        Vector3(maxCorner.x, minCorner.y, minCorner.z),
        Vector3(minCorner.x, maxCorner.y, minCorner.z),
        Vector3(maxCorner.x, maxCorner.y, minCorner.z),
        Vector3(minCorner.x, minCorner.y, maxCorner.z),
        Vector3(maxCorner.x, minCorner.y, maxCorner.z),
        Vector3(minCorner.x, maxCorner.y, maxCorner.z),
        Vector3(maxCorner.x, maxCorner.y, maxCorner.z),
    };

    // Check each corner
    for (int i = 0; i < 8; i++) {
        if (m_camera->IsPointInFrustum(corners[i], 0.0f)) {
            return true;
        }
    }

    // No corner in frustum - check if any plane intersects
    Vector3 boxCenter = (minCorner + maxCorner) * 0.5f;
    Vector3 boxExtent = (maxCorner - minCorner) * 0.5f;

    for (int i = 0; i < 6; i++) {
        float distance = Vector3::Dot(m_frustumPlanes[i].normal, boxCenter) - m_frustumPlanes[i].distance;
        float maxExtent = std::abs(boxExtent.x * m_frustumPlanes[i].normal.x) +
                         std::abs(boxExtent.y * m_frustumPlanes[i].normal.y) +
                         std::abs(boxExtent.z * m_frustumPlanes[i].normal.z);

        if (distance < -maxExtent) {
            return false; // Box is completely outside this plane
        }
    }

    return true; // Box might be partially visible
}

// ============================================================================
// Projection Management
// ============================================================================

void GraphicsCamera::SetViewportSize(uint32_t width, uint32_t height) {
    if (width > 0 && height > 0) {
        m_viewportWidth = width;
        m_viewportHeight = height;
        UpdateAspectRatio();
    }
}

void GraphicsCamera::GetViewportSize(uint32_t& outWidth, uint32_t& outHeight) const {
    outWidth = m_viewportWidth;
    outHeight = m_viewportHeight;
}

void GraphicsCamera::SetFieldOfView(float fov) {
    if (m_camera) {
        m_camera->SetFieldOfView(fov);
    }
}

float GraphicsCamera::GetFieldOfView() const {
    return m_camera ? m_camera->GetFieldOfView() : 0.0f;
}

void GraphicsCamera::SetNearPlane(float nearPlane) {
    if (m_camera) {
        m_camera->SetNearPlane(nearPlane);
    }
}

void GraphicsCamera::SetFarPlane(float farPlane) {
    if (m_camera) {
        m_camera->SetFarPlane(farPlane);
    }
}

void GraphicsCamera::GetClippingPlanes(float& outNear, float& outFar) const {
    if (m_camera) {
        outNear = m_camera->GetNearPlane();
        outFar = m_camera->GetFarPlane();
    }
}

// ============================================================================
// Screen Projection
// ============================================================================

bool GraphicsCamera::ProjectToScreen(const Vector3& worldPoint, float& outScreenX, float& outScreenY) const {
    if (!m_camera) {
        return false;
    }

    return m_camera->ProjectToScreen(worldPoint, outScreenX, outScreenY);
}

Camera::Ray GraphicsCamera::CastRay(float screenX, float screenY) const {
    if (!m_camera) {
        return {{0, 0, 0}, {0, 0, 1}};
    }

    return m_camera->CastRay(screenX, screenY);
}

void GraphicsCamera::PixelToScreenCoordinates(float pixelX, float pixelY, float& outScreenX, float& outScreenY) const {
    // Convert from pixel coordinates (0 to width/height) to normalized screen coordinates (-1 to +1)
    outScreenX = (pixelX / m_viewportWidth) * 2.0f - 1.0f;
    outScreenY = 1.0f - (pixelY / m_viewportHeight) * 2.0f;  // Flip Y for screen coordinates
}

void GraphicsCamera::ScreenToPixelCoordinates(float screenX, float screenY, float& outPixelX, float& outPixelY) const {
    // Convert from normalized screen coordinates (-1 to +1) to pixel coordinates
    outPixelX = (screenX + 1.0f) * 0.5f * m_viewportWidth;
    outPixelY = (1.0f - screenY) * 0.5f * m_viewportHeight;  // Flip Y back
}

// ============================================================================
// Debugging & Statistics
// ============================================================================

void GraphicsCamera::UpdateDebugInfo() {
    if (!m_camera) {
        return;
    }

    Vector3 pos = m_camera->GetPosition();
    float pitch, yaw, roll;
    m_camera->GetEulerAngles(pitch, yaw, roll);

    // Convert radians to degrees for readability
    float pitchDeg = pitch * 180.0f / 3.14159265f;
    float yawDeg = yaw * 180.0f / 3.14159265f;
    float rollDeg = roll * 180.0f / 3.14159265f;

    std::snprintf(m_debugInfo, sizeof(m_debugInfo),
        "Camera: Pos(%.2f,%.2f,%.2f) Rot(%.1f,%.1f,%.1f)° FOV%.1f° Near%.2f Far%.1f VP%ux%u",
        pos.x, pos.y, pos.z,
        pitchDeg, yawDeg, rollDeg,
        m_camera->GetFieldOfView() * 180.0f / 3.14159265f,
        m_camera->GetNearPlane(),
        m_camera->GetFarPlane(),
        m_viewportWidth, m_viewportHeight);
}

// ============================================================================
// Internal GPU Buffer Management
// ============================================================================

bool GraphicsCamera::CreateGPUUniformBuffer() {
    // Phase 44 integration: Create VkBuffer through graphics backend
    // For now, reserve handle space
    // In actual implementation, would call graphics backend like:
    // m_gpuBufferHandle = GraphicsBackend::CreateBuffer(
    //     sizeof(CameraUniformBuffer),
    //     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    // );

    m_gpuBufferHandle = 1; // Placeholder
    m_gpuBufferOffset = 0;
    return true;
}

void GraphicsCamera::DestroyGPUUniformBuffer() {
    // Phase 44 integration: Destroy VkBuffer
    // In actual implementation:
    // if (m_gpuBufferHandle) {
    //     GraphicsBackend::DestroyBuffer(m_gpuBufferHandle);
    // }

    m_gpuBufferHandle = 0;
}

bool GraphicsCamera::UpdateGPUUniformBuffer() {
    // Phase 44 integration: Update buffer data on GPU
    // In actual implementation:
    // if (m_gpuBufferHandle) {
    //     return GraphicsBackend::UpdateBuffer(
    //         m_gpuBufferHandle,
    //         0,
    //         sizeof(CameraUniformBuffer),
    //         &m_uniformBuffer
    //     );
    // }

    return true; // Placeholder success
}

void GraphicsCamera::RecalculateFrustumPlanes() {
    if (!m_camera) {
        return;
    }

    m_camera->GetFrustumPlanes(m_frustumPlanes);
}

void GraphicsCamera::UpdateAspectRatio() {
    if (m_camera && m_viewportHeight > 0) {
        float aspectRatio = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
        m_camera->SetAspectRatio(aspectRatio);
    }
}
