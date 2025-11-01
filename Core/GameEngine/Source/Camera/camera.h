#pragma once

#include "Core/Math/vector.h"
#include "Core/Math/matrix.h"
#include "Core/Math/quaternion.h"

/**
 * Phase 45: Camera System & View Transformation
 * 
 * Implements complete camera system with:
 * - Position and orientation management
 * - View matrix (world to camera space)
 * - Projection matrix (camera to clip space)
 * - Support for multiple camera modes
 * - Euler angles and quaternion rotations
 * 
 * Coordinate System:
 * - X: Right (positive right)
 * - Y: Up (positive up)
 * - Z: Forward (positive away from camera, left-handed)
 * 
 * Angles:
 * - Pitch: Rotation around X axis (up/down)
 * - Yaw: Rotation around Y axis (left/right)
 * - Roll: Rotation around Z axis (camera tilt)
 */

class Camera {
public:
    // Camera modes
    enum class Mode {
        FREE,      // Free movement in all directions
        RTS,       // Isometric RTS-style camera
        CHASE,     // Follow target object
        ORBIT      // Orbit around target point
    };

public:
    /**
     * Constructor
     * Initializes camera at default position with standard perspective
     */
    Camera();

    ~Camera() = default;

    // =========================================================================
    // Position & Orientation
    // =========================================================================

    /**
     * Set camera position in world space
     * @param position Camera position (world coordinates)
     */
    void SetPosition(const Vector3& position);

    /**
     * Get camera position in world space
     * @return Camera position vector
     */
    const Vector3& GetPosition() const { return m_position; }

    /**
     * Set camera look direction (forward vector)
     * @param forward Normalized forward vector
     */
    void SetForward(const Vector3& forward);

    /**
     * Get camera forward direction
     * @return Normalized forward vector
     */
    const Vector3& GetForward() const { return m_forward; }

    /**
     * Get camera right vector (perpendicular to forward and up)
     * @return Normalized right vector
     */
    const Vector3& GetRight() const { return m_right; }

    /**
     * Get camera up vector
     * @return Normalized up vector
     */
    const Vector3& GetUp() const { return m_up; }

    /**
     * Set all three basis vectors (forward, right, up)
     * @param forward Camera look direction
     * @param right Camera right vector
     * @param up Camera up vector
     */
    void SetBasis(const Vector3& forward, const Vector3& right, const Vector3& up);

    // =========================================================================
    // Rotation & Orientation (Euler Angles)
    // =========================================================================

    /**
     * Set rotation using Euler angles (in radians)
     * Applied in order: YXZ (yaw, pitch, roll)
     * @param pitch Rotation around X axis (up/down, range: -PI/2 to PI/2)
     * @param yaw Rotation around Y axis (left/right, range: -PI to PI)
     * @param roll Rotation around Z axis (camera tilt, range: -PI to PI)
     */
    void SetEulerAngles(float pitch, float yaw, float roll);

    /**
     * Get rotation as Euler angles (in radians)
     * @param outPitch Output pitch angle
     * @param outYaw Output yaw angle
     * @param outRoll Output roll angle
     */
    void GetEulerAngles(float& outPitch, float& outYaw, float& outRoll) const;

    /**
     * Apply relative pitch rotation (up/down look)
     * @param deltaPitch Change in pitch (in radians)
     */
    void RotatePitch(float deltaPitch);

    /**
     * Apply relative yaw rotation (left/right look)
     * @param deltaYaw Change in yaw (in radians)
     */
    void RotateYaw(float deltaYaw);

    /**
     * Apply relative roll rotation (camera tilt)
     * @param deltaRoll Change in roll (in radians)
     */
    void RotateRoll(float deltaRoll);

    // =========================================================================
    // Rotation & Orientation (Quaternions)
    // =========================================================================

    /**
     * Set rotation using quaternion
     * @param orientation Rotation quaternion (should be normalized)
     */
    void SetOrientation(const Quaternion& orientation);

    /**
     * Get rotation as quaternion
     * @return Current rotation quaternion
     */
    const Quaternion& GetOrientation() const { return m_orientation; }

    /**
     * Apply relative rotation using quaternion
     * @param deltaRotation Incremental rotation (should be normalized)
     */
    void RotateByQuaternion(const Quaternion& deltaRotation);

    // =========================================================================
    // Projection Settings
    // =========================================================================

    /**
     * Set field of view (in radians)
     * @param fov Field of view angle (typical: PI/3 = 60 degrees)
     */
    void SetFieldOfView(float fov);

    /**
     * Get field of view (in radians)
     * @return FOV angle
     */
    float GetFieldOfView() const { return m_fov; }

    /**
     * Set aspect ratio (width / height)
     * @param aspectRatio Display aspect ratio
     */
    void SetAspectRatio(float aspectRatio);

    /**
     * Get aspect ratio
     * @return Width / height ratio
     */
    float GetAspectRatio() const { return m_aspectRatio; }

    /**
     * Set near clipping plane distance
     * @param nearPlane Distance to near plane (typical: 0.1 - 1.0)
     */
    void SetNearPlane(float nearPlane);

    /**
     * Get near clipping plane distance
     * @return Near plane distance
     */
    float GetNearPlane() const { return m_nearPlane; }

    /**
     * Set far clipping plane distance
     * @param farPlane Distance to far plane (typical: 100 - 10000)
     */
    void SetFarPlane(float farPlane);

    /**
     * Get far clipping plane distance
     * @return Far plane distance
     */
    float GetFarPlane() const { return m_farPlane; }

    /**
     * Set all projection parameters at once
     * @param fov Field of view (radians)
     * @param aspectRatio Width / height
     * @param nearPlane Near clipping distance
     * @param farPlane Far clipping distance
     */
    void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane);

    // =========================================================================
    // Matrix Calculations
    // =========================================================================

    /**
     * Get view matrix (world to camera space)
     * Calculated from position and orientation basis vectors
     * @return 4x4 view matrix
     */
    const Matrix4x4& GetViewMatrix() const { return m_viewMatrix; }

    /**
     * Get projection matrix (camera to clip space)
     * Calculated from FOV, aspect ratio, near/far planes
     * @return 4x4 projection matrix
     */
    const Matrix4x4& GetProjectionMatrix() const { return m_projectionMatrix; }

    /**
     * Get combined view-projection matrix
     * @return Combined matrix (projection * view)
     */
    Matrix4x4 GetViewProjectionMatrix() const;

    /**
     * Check if matrices need recalculation
     * @return true if any parameter changed since last update
     */
    bool IsDirty() const { return m_viewDirty || m_projectionDirty; }

    /**
     * Force recalculation of matrices
     * Useful after external modifications to internal state
     */
    void UpdateMatrices();

    // =========================================================================
    // Movement
    // =========================================================================

    /**
     * Move camera forward relative to current orientation
     * @param distance Distance to move (positive = forward, negative = backward)
     */
    void MoveForward(float distance);

    /**
     * Move camera right relative to current orientation
     * @param distance Distance to move (positive = right, negative = left)
     */
    void MoveRight(float distance);

    /**
     * Move camera up in world space
     * @param distance Distance to move (positive = up, negative = down)
     */
    void MoveUp(float distance);

    /**
     * Move camera in world space
     * @param movement Movement vector in world coordinates
     */
    void Translate(const Vector3& movement);

    /**
     * Set camera mode
     * @param mode Camera mode (FREE, RTS, CHASE, ORBIT)
     */
    void SetMode(Mode mode) { m_mode = mode; }

    /**
     * Get current camera mode
     * @return Current mode
     */
    Mode GetMode() const { return m_mode; }

    // =========================================================================
    // Utility
    // =========================================================================

    /**
     * Get a ray from camera through a screen point
     * Useful for picking/casting rays
     * @param screenX Normalized screen X (-1 to +1)
     * @param screenY Normalized screen Y (-1 to +1)
     * @return Ray origin and direction
     */
    struct Ray {
        Vector3 origin;
        Vector3 direction;
    };

    /**
     * Cast a ray from camera through screen coordinates
     * @param screenX Normalized screen X coordinate (-1 to +1)
     * @param screenY Normalized screen Y coordinate (-1 to +1)
     * @return Ray from camera through screen point
     */
    Ray CastRay(float screenX, float screenY) const;

    /**
     * Project world point to screen coordinates
     * @param worldPoint Point in world space
     * @param outScreenX Output screen X (-1 to +1)
     * @param outScreenY Output screen Y (-1 to +1)
     * @return true if point is in front of camera, false if behind
     */
    bool ProjectToScreen(const Vector3& worldPoint, float& outScreenX, float& outScreenY) const;

    /**
     * Check if world point is within view frustum
     * @param worldPoint Point to test
     * @param radius Sphere radius around point (for conservative culling)
     * @return true if visible, false if outside frustum
     */
    bool IsPointInFrustum(const Vector3& worldPoint, float radius = 0.0f) const;

    /**
     * Look at target point
     * @param target Target point in world space
     * @param upVector World up direction (for roll)
     */
    void LookAt(const Vector3& target, const Vector3& upVector = Vector3(0, 1, 0));

    /**
     * Get camera frustum planes (for culling)
     * Returns 6 planes: near, far, left, right, top, bottom
     * Each plane is (normal, -distance)
     */
    struct FrustumPlane {
        Vector3 normal;
        float distance;
    };

    /**
     * Get all frustum planes
     * @param outPlanes Output array of 6 planes
     */
    void GetFrustumPlanes(FrustumPlane outPlanes[6]) const;

private:
    // =========================================================================
    // Internal State
    // =========================================================================

    // Position and orientation in world space
    Vector3 m_position;        // Camera position
    Vector3 m_forward;         // Look direction (normalized)
    Vector3 m_right;           // Right vector (normalized)
    Vector3 m_up;              // Up vector (normalized)
    Quaternion m_orientation;  // Rotation as quaternion

    // Euler angles for debugging/serialization
    float m_pitch;             // Rotation around X axis (radians)
    float m_yaw;               // Rotation around Y axis (radians)
    float m_roll;              // Rotation around Z axis (radians)

    // Projection parameters
    float m_fov;               // Field of view (radians)
    float m_aspectRatio;       // Width / height
    float m_nearPlane;         // Near clipping distance
    float m_farPlane;          // Far clipping distance

    // Cached matrices
    mutable Matrix4x4 m_viewMatrix;
    mutable Matrix4x4 m_projectionMatrix;
    mutable bool m_viewDirty;
    mutable bool m_projectionDirty;

    // Camera mode
    Mode m_mode;

    // =========================================================================
    // Internal Methods
    // =========================================================================

    /**
     * Recalculate view matrix from position and basis vectors
     */
    void RecalculateViewMatrix() const;

    /**
     * Recalculate projection matrix from FOV and clipping planes
     */
    void RecalculateProjectionMatrix() const;

    /**
     * Update basis vectors from Euler angles
     */
    void UpdateBasisFromEuler();

    /**
     * Update basis vectors from quaternion
     */
    void UpdateBasisFromQuaternion();

    /**
     * Clamp pitch to avoid gimbal lock
     * Ensures pitch stays in valid range: -PI/2 to PI/2
     */
    void ClampPitch();

    /**
     * Normalize yaw to range -PI to PI
     */
    void NormalizeYaw();

    /**
     * Normalize roll to range -PI to PI
     */
    void NormalizeRoll();
};
