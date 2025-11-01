#include "camera_rotation.h"
#include "Core/Math/math_utils.h"
#include <cmath>
#include <algorithm>

/**
 * Phase 45 Stage 2: Euler & Quaternion Rotation Implementation
 * 
 * Comprehensive rotation handling with gimbal lock avoidance
 * and smooth interpolation support
 */

// ============================================================================
// Euler ↔ Quaternion Conversions
// ============================================================================

Quaternion CameraRotation::EulerToQuaternion(float pitch, float yaw, float roll) {
    // Convert Euler angles to quaternion
    // Applied in order: YXZ (yaw, pitch, roll)
    
    float halfPitch = pitch / 2.0f;
    float halfYaw = yaw / 2.0f;
    float halfRoll = roll / 2.0f;
    
    float cosPitch = std::cos(halfPitch);
    float sinPitch = std::sin(halfPitch);
    float cosYaw = std::cos(halfYaw);
    float sinYaw = std::sin(halfYaw);
    float cosRoll = std::cos(halfRoll);
    float sinRoll = std::sin(halfRoll);
    
    // YXZ order composition
    Quaternion qX(sinPitch, 0, 0, cosPitch);          // Pitch (X)
    Quaternion qY(0, sinYaw, 0, cosYaw);              // Yaw (Y)
    Quaternion qZ(0, 0, sinRoll, cosRoll);            // Roll (Z)
    
    // Combine: qY * qX * qZ
    Quaternion qYX = qY * qX;
    Quaternion result = qYX * qZ;
    
    return result.Normalize();
}

void CameraRotation::QuaternionToEuler(const Quaternion& quat, float& outPitch, float& outYaw, float& outRoll) {
    Quaternion q = quat.Normalize();
    
    // Convert quaternion to Euler angles (YXZ order)
    // Using matrix representation for numerical stability
    
    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;
    
    // Pitch (rotation around X)
    float sinP = 2.0f * (w * x - y * z);
    if (std::abs(sinP) >= 1) {
        outPitch = std::copysign(3.14159265f / 2.0f, sinP); // Use 90 degrees
    } else {
        outPitch = std::asin(sinP);
    }
    
    // Yaw (rotation around Y)
    float sinY = 2.0f * (w * y + x * z);
    float cosY = 1.0f - 2.0f * (y * y + z * z);
    outYaw = std::atan2(sinY, cosY);
    
    // Roll (rotation around Z)
    float sinR = 2.0f * (w * z - x * y);
    if (std::abs(sinR) >= 1) {
        outRoll = std::copysign(3.14159265f / 2.0f, sinR); // Use 90 degrees
    } else {
        outRoll = std::asin(sinR);
    }
    
    // Normalize angles
    NormalizeEulerAngles(outPitch, outYaw, outRoll);
}

// ============================================================================
// Gimbal Lock Detection & Avoidance
// ============================================================================

bool CameraRotation::IsInGimbalLock(float pitch) {
    float absPitch = std::abs(pitch);
    return absPitch > (GIMBAL_LOCK_THRESHOLD - 0.1f); // 0.1 rad safety margin
}

float CameraRotation::ClampPitchToAvoidGimbalLock(float pitch) {
    // Clamp to ±88 degrees (approximately GIMBAL_LOCK_THRESHOLD)
    return std::max(-GIMBAL_LOCK_THRESHOLD, std::min(GIMBAL_LOCK_THRESHOLD, pitch));
}

// ============================================================================
// Quaternion Interpolation
// ============================================================================

Quaternion CameraRotation::Slerp(const Quaternion& q1, const Quaternion& q2, float t) {
    // Spherical Linear Interpolation
    // Provides smooth, constant-speed rotation interpolation
    
    Quaternion qa = q1.Normalize();
    Quaternion qb = q2.Normalize();
    
    // Calculate dot product
    float dotProduct = (qa.x * qb.x) + (qa.y * qb.y) + (qa.z * qb.z) + (qa.w * qb.w);
    
    // If dot product is negative, negate one quaternion to take shorter path
    if (dotProduct < 0.0f) {
        qb.x = -qb.x;
        qb.y = -qb.y;
        qb.z = -qb.z;
        qb.w = -qb.w;
        dotProduct = -dotProduct;
    }
    
    // Clamp dot product to prevent numerical issues with acos
    dotProduct = std::max(-1.0f, std::min(1.0f, dotProduct));
    
    // Calculate interpolation angle
    float theta = std::acos(dotProduct);
    float sinTheta = std::sin(theta);
    
    // Handle edge case: parallel quaternions
    if (std::abs(sinTheta) < 0.001f) {
        return Lerp(qa, qb, t); // Fall back to linear interpolation
    }
    
    // Calculate coefficients
    float w1 = std::sin((1.0f - t) * theta) / sinTheta;
    float w2 = std::sin(t * theta) / sinTheta;
    
    // Interpolate
    Quaternion result;
    result.x = w1 * qa.x + w2 * qb.x;
    result.y = w1 * qa.y + w2 * qb.y;
    result.z = w1 * qa.z + w2 * qb.z;
    result.w = w1 * qa.w + w2 * qb.w;
    
    return result.Normalize();
}

Quaternion CameraRotation::Lerp(const Quaternion& q1, const Quaternion& q2, float t) {
    // Linear Interpolation (faster but less smooth than Slerp)
    
    Quaternion qa = q1.Normalize();
    Quaternion qb = q2.Normalize();
    
    // Ensure shortest path
    float dotProduct = (qa.x * qb.x) + (qa.y * qb.y) + (qa.z * qb.z) + (qa.w * qb.w);
    if (dotProduct < 0.0f) {
        qb.x = -qb.x;
        qb.y = -qb.y;
        qb.z = -qb.z;
        qb.w = -qb.w;
    }
    
    // Linear interpolation
    Quaternion result;
    result.x = qa.x + t * (qb.x - qa.x);
    result.y = qa.y + t * (qb.y - qa.y);
    result.z = qa.z + t * (qb.z - qa.z);
    result.w = qa.w + t * (qb.w - qa.w);
    
    return result.Normalize();
}

// ============================================================================
// Axis-Angle Representations
// ============================================================================

Quaternion CameraRotation::FromAxisAngle(const Vector3& axis, float angle) {
    Vector3 normalizedAxis = axis.Normalize();
    
    float halfAngle = angle / 2.0f;
    float sinHalf = std::sin(halfAngle);
    
    Quaternion result;
    result.x = normalizedAxis.x * sinHalf;
    result.y = normalizedAxis.y * sinHalf;
    result.z = normalizedAxis.z * sinHalf;
    result.w = std::cos(halfAngle);
    
    return result.Normalize();
}

void CameraRotation::ToAxisAngle(const Quaternion& quat, Vector3& outAxis, float& outAngle) {
    Quaternion q = quat.Normalize();
    
    // Handle edge case: identity quaternion
    if (std::abs(q.w) >= 1.0f) {
        outAxis = Vector3(0.0f, 1.0f, 0.0f); // Default axis
        outAngle = 0.0f;
        return;
    }
    
    // Calculate angle
    outAngle = 2.0f * std::acos(std::max(-1.0f, std::min(1.0f, q.w)));
    
    // Calculate axis
    float sinHalfAngle = std::sin(outAngle / 2.0f);
    
    if (std::abs(sinHalfAngle) < 0.001f) {
        outAxis = Vector3(0.0f, 1.0f, 0.0f); // Default axis for small angle
    } else {
        outAxis = Vector3(q.x, q.y, q.z) / sinHalfAngle;
        outAxis = outAxis.Normalize();
    }
}

// ============================================================================
// Vector-Based Rotations
// ============================================================================

Quaternion CameraRotation::FromForwardAndUp(const Vector3& forward, const Vector3& up) {
    Vector3 f = forward.Normalize();
    Vector3 u = up.Normalize();
    
    // Calculate right vector
    Vector3 r = Vector3::Cross(u, f).Normalize();
    
    // Recalculate up to ensure orthogonality
    Vector3 u2 = Vector3::Cross(f, r).Normalize();
    
    // Convert rotation matrix to quaternion
    float trace = r.x + u2.y - f.z;
    
    Quaternion result;
    
    if (trace > 0.0f) {
        float s = 0.5f / std::sqrt(trace + 1.0f);
        result.w = 0.25f / s;
        result.x = (u2.z - f.y) * s;
        result.y = (f.x - r.z) * s;
        result.z = (r.y - u2.x) * s;
    } else if ((r.x > u2.y) && (r.x > -f.z)) {
        float s = 2.0f * std::sqrt(1.0f + r.x - u2.y + f.z);
        result.w = (u2.z - f.y) / s;
        result.x = 0.25f * s;
        result.y = (r.y + u2.x) / s;
        result.z = (f.x + r.z) / s;
    } else if (u2.y > -f.z) {
        float s = 2.0f * std::sqrt(1.0f + u2.y - r.x - f.z);
        result.w = (f.x - r.z) / s;
        result.x = (r.y + u2.x) / s;
        result.y = 0.25f * s;
        result.z = (u2.z + f.y) / s;
    } else {
        float s = 2.0f * std::sqrt(1.0f - f.z - r.x + u2.y);
        result.w = (r.y - u2.x) / s;
        result.x = (f.x + r.z) / s;
        result.y = (u2.z + f.y) / s;
        result.z = 0.25f * s;
    }
    
    return result.Normalize();
}

void CameraRotation::ToForwardAndUp(const Quaternion& quat, Vector3& outForward, Vector3& outUp) {
    Quaternion q = quat.Normalize();
    
    // Forward vector (Z-axis of rotation)
    outForward.x = 2.0f * (q.x * q.z + q.w * q.y);
    outForward.y = 2.0f * (q.y * q.z - q.w * q.x);
    outForward.z = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    outForward = outForward.Normalize();
    
    // Up vector (Y-axis of rotation)
    outUp.x = 2.0f * (q.x * q.y - q.w * q.z);
    outUp.y = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
    outUp.z = 2.0f * (q.y * q.z + q.w * q.x);
    outUp = outUp.Normalize();
}

Quaternion CameraRotation::FromToRotation(const Vector3& from, const Vector3& to) {
    Vector3 f = from.Normalize();
    Vector3 t = to.Normalize();
    
    // Handle parallel vectors
    float dot = Vector3::Dot(f, t);
    
    if (dot > 0.9995f) {
        // Vectors are nearly parallel
        return Quaternion(0, 0, 0, 1); // Identity
    } else if (dot < -0.9995f) {
        // Vectors are nearly opposite
        // Find perpendicular axis
        Vector3 perp;
        if (std::abs(f.x) < 0.9f) {
            perp = Vector3(1, 0, 0);
        } else {
            perp = Vector3(0, 1, 0);
        }
        
        Vector3 axis = Vector3::Cross(f, perp).Normalize();
        return FromAxisAngle(axis, 3.14159265f); // 180 degree rotation
    }
    
    // General case
    Vector3 axis = Vector3::Cross(f, t).Normalize();
    float angle = std::acos(dot);
    
    return FromAxisAngle(axis, angle);
}

// ============================================================================
// Incremental Rotations
// ============================================================================

Quaternion CameraRotation::ApplyEulerDelta(const Quaternion& current, float deltaPitch, float deltaYaw, float deltaRoll) {
    // Create delta quaternion from Euler deltas
    Quaternion qDelta = EulerToQuaternion(deltaPitch, deltaYaw, deltaRoll);
    
    // Apply delta rotation to current orientation
    // For camera control: new = qDelta * current (rotate first, then current orientation)
    Quaternion result = (qDelta * current).Normalize();
    
    return result;
}

// ============================================================================
// Angle Normalization
// ============================================================================

void CameraRotation::NormalizeEulerAngles(float& pitch, float& yaw, float& roll) {
    const float PI = 3.14159265f;
    const float TWO_PI = 6.28318530f;
    
    // Normalize pitch to [-PI/2, PI/2]
    pitch = ClampPitchToAvoidGimbalLock(pitch);
    
    // Normalize yaw to [-PI, PI]
    while (yaw > PI) yaw -= TWO_PI;
    while (yaw < -PI) yaw += TWO_PI;
    
    // Normalize roll to [-PI, PI]
    while (roll > PI) roll -= TWO_PI;
    while (roll < -PI) roll += TWO_PI;
}
