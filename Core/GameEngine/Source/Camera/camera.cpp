#include "camera.h"
#include "Core/Math/math_utils.h"
#include <cmath>
#include <algorithm>

/**
 * Phase 45: Camera System & View Transformation - Implementation
 * 
 * Implements core camera functionality including:
 * - Position and orientation management
 * - View matrix calculation (LookAt)
 * - Projection matrix calculation (Perspective)
 * - Euler angle and quaternion rotations
 * - Matrix caching for performance
 */

// ============================================================================
// Constructor & Initialization
// ============================================================================

Camera::Camera()
    : m_position(0.0f, 5.0f, -10.0f)    // Default: above and behind world origin
    , m_forward(0.0f, 0.0f, 1.0f)       // Looking forward
    , m_right(1.0f, 0.0f, 0.0f)         // Right vector
    , m_up(0.0f, 1.0f, 0.0f)            // Up vector
    , m_orientation()                    // Identity quaternion
    , m_pitch(0.0f)                     // No pitch
    , m_yaw(0.0f)                       // No yaw
    , m_roll(0.0f)                      // No roll
    , m_fov(3.14159265f / 3.0f)         // 60 degrees (PI/3)
    , m_aspectRatio(16.0f / 9.0f)       // 16:9 HD
    , m_nearPlane(0.1f)                 // Standard near
    , m_farPlane(1000.0f)               // Standard far
    , m_viewDirty(true)
    , m_projectionDirty(true)
    , m_mode(Mode::FREE) {
    
    // Force matrix recalculation
    UpdateMatrices();
}

// ============================================================================
// Position & Orientation
// ============================================================================

void Camera::SetPosition(const Vector3& position) {
    if (m_position != position) {
        m_position = position;
        m_viewDirty = true;
    }
}

void Camera::SetForward(const Vector3& forward) {
    Vector3 normalized = forward.Normalize();
    if (m_forward != normalized) {
        m_forward = normalized;
        
        // Reconstruct right vector
        Vector3 worldUp(0.0f, 1.0f, 0.0f);
        m_right = Vector3::Cross(worldUp, m_forward).Normalize();
        
        // Reconstruct up vector (should be perpendicular to forward and right)
        m_up = Vector3::Cross(m_forward, m_right).Normalize();
        
        m_viewDirty = true;
        UpdateBasisFromEuler(); // Update Euler angles to match new basis
    }
}

void Camera::SetBasis(const Vector3& forward, const Vector3& right, const Vector3& up) {
    m_forward = forward.Normalize();
    m_right = right.Normalize();
    m_up = up.Normalize();
    m_viewDirty = true;
    
    // Update quaternion and Euler angles from basis
    // Convert basis to quaternion
    float trace = m_right.x + m_up.y + m_forward.z;
    
    if (trace > 0.0f) {
        float s = 0.5f / std::sqrt(trace + 1.0f);
        m_orientation.w = 0.25f / s;
        m_orientation.x = (m_up.z - m_forward.y) * s;
        m_orientation.y = (m_forward.x - m_right.z) * s;
        m_orientation.z = (m_right.y - m_up.x) * s;
    } else if ((m_right.x > m_up.y) && (m_right.x > m_forward.z)) {
        float s = 2.0f * std::sqrt(1.0f + m_right.x - m_up.y - m_forward.z);
        m_orientation.w = (m_up.z - m_forward.y) / s;
        m_orientation.x = 0.25f * s;
        m_orientation.y = (m_up.x + m_right.y) / s;
        m_orientation.z = (m_forward.x + m_right.z) / s;
    } else if (m_up.y > m_forward.z) {
        float s = 2.0f * std::sqrt(1.0f + m_up.y - m_right.x - m_forward.z);
        m_orientation.w = (m_forward.x - m_right.z) / s;
        m_orientation.x = (m_up.x + m_right.y) / s;
        m_orientation.y = 0.25f * s;
        m_orientation.z = (m_forward.y + m_up.z) / s;
    } else {
        float s = 2.0f * std::sqrt(1.0f + m_forward.z - m_right.x - m_up.y);
        m_orientation.w = (m_right.y - m_up.x) / s;
        m_orientation.x = (m_forward.x + m_right.z) / s;
        m_orientation.y = (m_forward.y + m_up.z) / s;
        m_orientation.z = 0.25f * s;
    }
}

// ============================================================================
// Rotation - Euler Angles
// ============================================================================

void Camera::SetEulerAngles(float pitch, float yaw, float roll) {
    m_pitch = pitch;
    m_yaw = yaw;
    m_roll = roll;
    
    ClampPitch();
    NormalizeYaw();
    NormalizeRoll();
    
    UpdateBasisFromEuler();
    m_viewDirty = true;
}

void Camera::GetEulerAngles(float& outPitch, float& outYaw, float& outRoll) const {
    outPitch = m_pitch;
    outYaw = m_yaw;
    outRoll = m_roll;
}

void Camera::RotatePitch(float deltaPitch) {
    m_pitch += deltaPitch;
    ClampPitch();
    UpdateBasisFromEuler();
    m_viewDirty = true;
}

void Camera::RotateYaw(float deltaYaw) {
    m_yaw += deltaYaw;
    NormalizeYaw();
    UpdateBasisFromEuler();
    m_viewDirty = true;
}

void Camera::RotateRoll(float deltaRoll) {
    m_roll += deltaRoll;
    NormalizeRoll();
    UpdateBasisFromEuler();
    m_viewDirty = true;
}

// ============================================================================
// Rotation - Quaternions
// ============================================================================

void Camera::SetOrientation(const Quaternion& orientation) {
    m_orientation = orientation.Normalize();
    UpdateBasisFromQuaternion();
    m_viewDirty = true;
}

void Camera::RotateByQuaternion(const Quaternion& deltaRotation) {
    m_orientation = (deltaRotation * m_orientation).Normalize();
    UpdateBasisFromQuaternion();
    m_viewDirty = true;
}

// ============================================================================
// Projection Settings
// ============================================================================

void Camera::SetFieldOfView(float fov) {
    if (m_fov != fov) {
        m_fov = fov;
        m_projectionDirty = true;
    }
}

void Camera::SetAspectRatio(float aspectRatio) {
    if (m_aspectRatio != aspectRatio) {
        m_aspectRatio = aspectRatio;
        m_projectionDirty = true;
    }
}

void Camera::SetNearPlane(float nearPlane) {
    if (m_nearPlane != nearPlane) {
        m_nearPlane = nearPlane;
        m_projectionDirty = true;
    }
}

void Camera::SetFarPlane(float farPlane) {
    if (m_farPlane != farPlane) {
        m_farPlane = farPlane;
        m_projectionDirty = true;
    }
}

void Camera::SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane) {
    m_fov = fov;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionDirty = true;
}

// ============================================================================
// Matrix Calculations
// ============================================================================

Matrix4x4 Camera::GetViewProjectionMatrix() const {
    UpdateMatrices();
    return m_projectionMatrix * m_viewMatrix;
}

void Camera::UpdateMatrices() {
    if (m_viewDirty) {
        RecalculateViewMatrix();
        m_viewDirty = false;
    }
    if (m_projectionDirty) {
        RecalculateProjectionMatrix();
        m_projectionDirty = false;
    }
}

void Camera::RecalculateViewMatrix() const {
    // View matrix = LookAt(position, position + forward, up)
    // Build orthonormal basis from forward vector
    
    // Ensure all vectors are normalized
    Vector3 forward = m_forward.Normalize();
    Vector3 right = m_right.Normalize();
    Vector3 up = m_up.Normalize();
    
    // View matrix in row-major order (for Vulkan/column-major storage)
    // V = | right.x        up.x        -forward.x       0 |
    //     | right.y        up.y        -forward.y       0 |
    //     | right.z        up.z        -forward.z       0 |
    //     | -dot(right,p)  -dot(up,p)  dot(forward,p)   1 |
    
    m_viewMatrix.m[0][0] = right.x;
    m_viewMatrix.m[0][1] = up.x;
    m_viewMatrix.m[0][2] = -forward.x;
    m_viewMatrix.m[0][3] = 0.0f;
    
    m_viewMatrix.m[1][0] = right.y;
    m_viewMatrix.m[1][1] = up.y;
    m_viewMatrix.m[1][2] = -forward.y;
    m_viewMatrix.m[1][3] = 0.0f;
    
    m_viewMatrix.m[2][0] = right.z;
    m_viewMatrix.m[2][1] = up.z;
    m_viewMatrix.m[2][2] = -forward.z;
    m_viewMatrix.m[2][3] = 0.0f;
    
    m_viewMatrix.m[3][0] = -Vector3::Dot(right, m_position);
    m_viewMatrix.m[3][1] = -Vector3::Dot(up, m_position);
    m_viewMatrix.m[3][2] = Vector3::Dot(forward, m_position);
    m_viewMatrix.m[3][3] = 1.0f;
}

void Camera::RecalculateProjectionMatrix() const {
    // Perspective projection matrix
    float tanHalfFov = std::tan(m_fov / 2.0f);
    
    float f = 1.0f / tanHalfFov;
    float a = f / m_aspectRatio;
    
    float zFarPlusNear = m_farPlane + m_nearPlane;
    float zFarMinusNear = m_farPlane - m_nearPlane;
    
    // Vulkan clip space: Z ranges from 0 (near) to 1 (far)
    // P = | a    0    0                              0       |
    //     | 0    f    0                              0       |
    //     | 0    0    -farPlane/(far-near)          -1      |
    //     | 0    0    -(2*far*near)/(far-near)      0       |
    
    m_projectionMatrix.m[0][0] = a;
    m_projectionMatrix.m[0][1] = 0.0f;
    m_projectionMatrix.m[0][2] = 0.0f;
    m_projectionMatrix.m[0][3] = 0.0f;
    
    m_projectionMatrix.m[1][0] = 0.0f;
    m_projectionMatrix.m[1][1] = f;
    m_projectionMatrix.m[1][2] = 0.0f;
    m_projectionMatrix.m[1][3] = 0.0f;
    
    m_projectionMatrix.m[2][0] = 0.0f;
    m_projectionMatrix.m[2][1] = 0.0f;
    m_projectionMatrix.m[2][2] = -m_farPlane / zFarMinusNear;
    m_projectionMatrix.m[2][3] = -1.0f;
    
    m_projectionMatrix.m[3][0] = 0.0f;
    m_projectionMatrix.m[3][1] = 0.0f;
    m_projectionMatrix.m[3][2] = -(2.0f * m_farPlane * m_nearPlane) / zFarMinusNear;
    m_projectionMatrix.m[3][3] = 0.0f;
}

// ============================================================================
// Movement
// ============================================================================

void Camera::MoveForward(float distance) {
    m_position = m_position + (m_forward * distance);
    m_viewDirty = true;
}

void Camera::MoveRight(float distance) {
    m_position = m_position + (m_right * distance);
    m_viewDirty = true;
}

void Camera::MoveUp(float distance) {
    m_position = m_position + (m_up * distance);
    m_viewDirty = true;
}

void Camera::Translate(const Vector3& movement) {
    m_position = m_position + movement;
    m_viewDirty = true;
}

// ============================================================================
// Utility Methods
// ============================================================================

Camera::Ray Camera::CastRay(float screenX, float screenY) const {
    UpdateMatrices();
    
    // Transform screen coordinates to NDC space
    Vector3 ndc(screenX, screenY, -1.0f); // -1 for near plane
    
    // Unproject from NDC to world space
    // First unproject to camera space using inverse projection
    Matrix4x4 invProj = m_projectionMatrix.Inverse();
    Vector3 camera_space = invProj * ndc;
    
    // Normalize to get direction
    Vector3 direction = camera_space.Normalize();
    
    // Transform direction to world space using inverse view
    Matrix4x4 invView = m_viewMatrix.Inverse();
    Vector3 world_direction = (invView * Vector4(direction, 0.0f)).xyz;
    
    return {m_position, world_direction.Normalize()};
}

bool Camera::ProjectToScreen(const Vector3& worldPoint, float& outScreenX, float& outScreenY) const {
    UpdateMatrices();
    
    // Transform world point to clip space
    Vector4 clipPos = m_projectionMatrix * (m_viewMatrix * Vector4(worldPoint, 1.0f));
    
    // Perspective divide
    if (std::abs(clipPos.w) < 0.001f) {
        return false; // Point is too close to camera plane
    }
    
    Vector3 ndc = Vector3(clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w);
    
    // Check if behind camera
    if (ndc.z < 0.0f || ndc.z > 1.0f) {
        return false; // Point is outside view frustum depth
    }
    
    outScreenX = ndc.x;
    outScreenY = ndc.y;
    return true;
}

bool Camera::IsPointInFrustum(const Vector3& worldPoint, float radius) const {
    UpdateMatrices();
    
    // Transform point to clip space
    Vector4 clipPos = m_projectionMatrix * (m_viewMatrix * Vector4(worldPoint, 1.0f));
    
    if (std::abs(clipPos.w) < 0.001f) {
        return false;
    }
    
    Vector3 ndc = Vector3(clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w);
    
    // Add radius bias for conservative culling
    float bias = radius / (clipPos.w > 0 ? clipPos.w : 1.0f);
    
    return (ndc.x > -1.0f - bias && ndc.x < 1.0f + bias &&
            ndc.y > -1.0f - bias && ndc.y < 1.0f + bias &&
            ndc.z >= 0.0f - bias && ndc.z <= 1.0f + bias);
}

void Camera::LookAt(const Vector3& target, const Vector3& upVector) {
    Vector3 forward = (target - m_position).Normalize();
    Vector3 right = Vector3::Cross(upVector, forward).Normalize();
    Vector3 up = Vector3::Cross(forward, right).Normalize();
    
    SetBasis(forward, right, up);
}

void Camera::GetFrustumPlanes(FrustumPlane outPlanes[6]) const {
    UpdateMatrices();
    
    // Get combined view-projection matrix
    Matrix4x4 vp = m_projectionMatrix * m_viewMatrix;
    
    // Extract frustum planes from VP matrix
    // Left plane: M[3] + M[0]
    // Right plane: M[3] - M[0]
    // Bottom plane: M[3] + M[1]
    // Top plane: M[3] - M[1]
    // Near plane: M[3] + M[2]
    // Far plane: M[3] - M[2]
    
    // Near plane
    outPlanes[0].normal = Vector3(vp.m[0][2], vp.m[1][2], vp.m[2][2]);
    outPlanes[0].distance = -vp.m[3][2];
    outPlanes[0].normal = outPlanes[0].normal.Normalize();
    
    // Far plane
    outPlanes[1].normal = Vector3(-vp.m[0][2], -vp.m[1][2], -vp.m[2][2]);
    outPlanes[1].distance = vp.m[3][2];
    outPlanes[1].normal = outPlanes[1].normal.Normalize();
    
    // Left plane
    outPlanes[2].normal = Vector3(vp.m[0][3] + vp.m[0][0], vp.m[1][3] + vp.m[1][0], vp.m[2][3] + vp.m[2][0]);
    outPlanes[2].distance = vp.m[3][3] + vp.m[3][0];
    outPlanes[2].normal = outPlanes[2].normal.Normalize();
    
    // Right plane
    outPlanes[3].normal = Vector3(vp.m[0][3] - vp.m[0][0], vp.m[1][3] - vp.m[1][0], vp.m[2][3] - vp.m[2][0]);
    outPlanes[3].distance = vp.m[3][3] - vp.m[3][0];
    outPlanes[3].normal = outPlanes[3].normal.Normalize();
    
    // Bottom plane
    outPlanes[4].normal = Vector3(vp.m[0][3] + vp.m[0][1], vp.m[1][3] + vp.m[1][1], vp.m[2][3] + vp.m[2][1]);
    outPlanes[4].distance = vp.m[3][3] + vp.m[3][1];
    outPlanes[4].normal = outPlanes[4].normal.Normalize();
    
    // Top plane
    outPlanes[5].normal = Vector3(vp.m[0][3] - vp.m[0][1], vp.m[1][3] - vp.m[1][1], vp.m[2][3] - vp.m[2][1]);
    outPlanes[5].distance = vp.m[3][3] - vp.m[3][1];
    outPlanes[5].normal = outPlanes[5].normal.Normalize();
}

// ============================================================================
// Internal Helper Methods
// ============================================================================

void Camera::UpdateBasisFromEuler() {
    // Convert Euler angles (pitch, yaw, roll) to basis vectors
    // Applied in order: YXZ (yaw, pitch, roll)
    
    float cosPitch = std::cos(m_pitch);
    float sinPitch = std::sin(m_pitch);
    float cosYaw = std::cos(m_yaw);
    float sinYaw = std::sin(m_yaw);
    float cosRoll = std::cos(m_roll);
    float sinRoll = std::sin(m_roll);
    
    // Forward vector (from yaw and pitch)
    m_forward.x = sinYaw * cosPitch;
    m_forward.y = sinPitch;
    m_forward.z = cosYaw * cosPitch;
    m_forward = m_forward.Normalize();
    
    // Right vector (perpendicular to forward and world up)
    Vector3 worldUp(0.0f, 1.0f, 0.0f);
    m_right = Vector3::Cross(worldUp, m_forward).Normalize();
    
    // Up vector (perpendicular to forward and right, with roll applied)
    m_up = Vector3::Cross(m_forward, m_right).Normalize();
    
    // Apply roll rotation if needed (rotate up around forward axis)
    if (std::abs(m_roll) > 0.001f) {
        float cosR = std::cos(m_roll);
        float sinR = std::sin(m_roll);
        
        Vector3 tempUp = m_up;
        m_up = m_up * cosR + m_right * sinR;
        m_right = m_right * cosR - tempUp * sinR;
    }
    
    // Update quaternion
    m_orientation = Quaternion::FromEulerAngles(m_pitch, m_yaw, m_roll);
}

void Camera::UpdateBasisFromQuaternion() {
    // Convert quaternion to basis vectors
    Vector3 forward, right, up;
    
    // Forward vector: Z-axis of quaternion rotation
    forward.x = 2.0f * (m_orientation.x * m_orientation.z + m_orientation.w * m_orientation.y);
    forward.y = 2.0f * (m_orientation.y * m_orientation.z - m_orientation.w * m_orientation.x);
    forward.z = 1.0f - 2.0f * (m_orientation.x * m_orientation.x + m_orientation.y * m_orientation.y);
    m_forward = forward.Normalize();
    
    // Right vector: X-axis of quaternion rotation
    right.x = 1.0f - 2.0f * (m_orientation.y * m_orientation.y + m_orientation.z * m_orientation.z);
    right.y = 2.0f * (m_orientation.x * m_orientation.y - m_orientation.w * m_orientation.z);
    right.z = 2.0f * (m_orientation.x * m_orientation.z + m_orientation.w * m_orientation.y);
    m_right = right.Normalize();
    
    // Up vector: Y-axis of quaternion rotation
    up.x = 2.0f * (m_orientation.x * m_orientation.y + m_orientation.w * m_orientation.z);
    up.y = 1.0f - 2.0f * (m_orientation.x * m_orientation.x + m_orientation.z * m_orientation.z);
    up.z = 2.0f * (m_orientation.y * m_orientation.z - m_orientation.w * m_orientation.x);
    m_up = up.Normalize();
    
    // Update Euler angles from quaternion
    m_orientation.ToEulerAngles(m_pitch, m_yaw, m_roll);
}

void Camera::ClampPitch() {
    // Prevent gimbal lock: clamp pitch to ±88 degrees
    const float maxPitch = 1.535889f; // ~88 degrees
    m_pitch = std::max(-maxPitch, std::min(maxPitch, m_pitch));
}

void Camera::NormalizeYaw() {
    // Keep yaw in range -PI to PI
    const float twoPi = 6.28318530f;
    while (m_yaw > 3.14159265f) m_yaw -= twoPi;
    while (m_yaw < -3.14159265f) m_yaw += twoPi;
}

void Camera::NormalizeRoll() {
    // Keep roll in range -PI to PI
    const float twoPi = 6.28318530f;
    while (m_roll > 3.14159265f) m_roll -= twoPi;
    while (m_roll < -3.14159265f) m_roll += twoPi;
}
