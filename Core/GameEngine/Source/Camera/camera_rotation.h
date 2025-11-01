#pragma once

#include "Core/Math/quaternion.h"
#include "Core/Math/vector.h"

/**
 * Phase 45 Stage 2: Euler Angles & Quaternion Rotation Utilities
 * 
 * Provides comprehensive support for:
 * - Euler angle to quaternion conversion
 * - Quaternion to Euler angle conversion
 * - Gimbal lock detection and avoidance
 * - Smooth rotation interpolation (SLERP)
 * - Rotation matrix generation from quaternions
 */

class CameraRotation {
public:
    /**
     * Convert Euler angles to quaternion
     * Applied in order: YXZ (yaw, pitch, roll)
     * @param pitch Rotation around X axis (radians, -PI/2 to PI/2)
     * @param yaw Rotation around Y axis (radians, -PI to PI)
     * @param roll Rotation around Z axis (radians, -PI to PI)
     * @return Normalized quaternion representing rotation
     */
    static Quaternion EulerToQuaternion(float pitch, float yaw, float roll);

    /**
     * Convert quaternion to Euler angles
     * @param quat Quaternion to convert
     * @param outPitch Output pitch angle (radians)
     * @param outYaw Output yaw angle (radians)
     * @param outRoll Output roll angle (radians)
     */
    static void QuaternionToEuler(const Quaternion& quat, float& outPitch, float& outYaw, float& outRoll);

    /**
     * Check if camera is in gimbal lock state
     * Gimbal lock occurs when pitch is near ±90 degrees
     * @param pitch Current pitch angle (radians)
     * @return true if in gimbal lock, false otherwise
     */
    static bool IsInGimbalLock(float pitch);

    /**
     * Clamp pitch to avoid gimbal lock
     * Limits pitch to ±88 degrees (1.535889 radians)
     * @param pitch Input pitch angle (radians)
     * @return Clamped pitch angle
     */
    static float ClampPitchToAvoidGimbalLock(float pitch);

    /**
     * Spherical linear interpolation between two quaternions
     * Used for smooth camera rotation transitions
     * @param q1 Start quaternion
     * @param q2 End quaternion
     * @param t Interpolation parameter (0 to 1)
     * @return Interpolated quaternion
     */
    static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);

    /**
     * Linear interpolation between two quaternions (faster, less smooth)
     * @param q1 Start quaternion
     * @param q2 End quaternion
     * @param t Interpolation parameter (0 to 1)
     * @return Interpolated quaternion
     */
    static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t);

    /**
     * Create rotation quaternion from axis and angle
     * @param axis Rotation axis (should be normalized)
     * @param angle Rotation angle in radians
     * @return Quaternion representing rotation around axis
     */
    static Quaternion FromAxisAngle(const Vector3& axis, float angle);

    /**
     * Extract rotation axis and angle from quaternion
     * @param quat Quaternion to decompose
     * @param outAxis Output rotation axis (normalized)
     * @param outAngle Output rotation angle (radians)
     */
    static void ToAxisAngle(const Quaternion& quat, Vector3& outAxis, float& outAngle);

    /**
     * Create rotation quaternion from forward and up vectors
     * Useful for LookAt-style camera control
     * @param forward Camera forward direction (should be normalized)
     * @param up Camera up direction (should be normalized)
     * @return Quaternion representing camera orientation
     */
    static Quaternion FromForwardAndUp(const Vector3& forward, const Vector3& up);

    /**
     * Extract forward and up vectors from quaternion
     * @param quat Quaternion representing camera orientation
     * @param outForward Output camera forward direction
     * @param outUp Output camera up direction
     */
    static void ToForwardAndUp(const Quaternion& quat, Vector3& outForward, Vector3& outUp);

    /**
     * Create rotation quaternion that rotates from one vector to another
     * @param from Source vector (should be normalized)
     * @param to Target vector (should be normalized)
     * @return Quaternion rotation from 'from' to 'to'
     */
    static Quaternion FromToRotation(const Vector3& from, const Vector3& to);

    /**
     * Apply incremental rotation using Euler angle deltas
     * Useful for continuous camera control (mouse movement)
     * @param current Current orientation quaternion
     * @param deltaPitch Change in pitch (radians)
     * @param deltaYaw Change in yaw (radians)
     * @param deltaRoll Change in roll (radians)
     * @return New orientation quaternion
     */
    static Quaternion ApplyEulerDelta(const Quaternion& current, float deltaPitch, float deltaYaw, float deltaRoll);

    /**
     * Normalize Euler angle to standard ranges
     * Pitch: ±PI/2, Yaw: -PI to PI, Roll: -PI to PI
     * @param pitch Input pitch
     * @param yaw Input yaw
     * @param roll Input roll
     */
    static void NormalizeEulerAngles(float& pitch, float& yaw, float& roll);

    /**
     * Convert angle in degrees to radians
     * @param degrees Angle in degrees
     * @return Angle in radians
     */
    static constexpr float DegreesToRadians(float degrees) {
        return degrees * 3.14159265f / 180.0f;
    }

    /**
     * Convert angle in radians to degrees
     * @param radians Angle in radians
     * @return Angle in degrees
     */
    static constexpr float RadiansToDegrees(float radians) {
        return radians * 180.0f / 3.14159265f;
    }

private:
    // Private constructor - utility class only
    CameraRotation() = delete;

    // Gimbal lock threshold (in radians)
    static constexpr float GIMBAL_LOCK_THRESHOLD = 1.535889f; // ~88 degrees
};
