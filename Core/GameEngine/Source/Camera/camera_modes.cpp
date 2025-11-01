#include "camera_modes.h"
#include "camera_rotation.h"
#include "Core/Math/math_utils.h"
#include <cmath>
#include <algorithm>

/**
 * Phase 45 Stage 3: Camera Modes Implementation
 * 
 * Specialized camera behaviors for different gameplay scenarios
 */

// ============================================================================
// Free Camera Mode
// ============================================================================

FreeCameraMode::FreeCameraMode()
    : m_camera(std::make_unique<Camera>())
    , m_movementSpeed(20.0f)
    , m_rotationSpeed(1.0f) {
    
    m_camera->SetMode(Camera::Mode::FREE);
    m_camera->SetPosition(Vector3(0, 5, -10));
}

void FreeCameraMode::Update(float deltaTime) {
    // Apply accumulated input to camera
    if (m_inputAccumulator.Length() > 0.001f) {
        Vector3 input = m_inputAccumulator.Normalize();
        
        // Move relative to camera orientation
        m_camera->MoveForward(input.z * m_movementSpeed * deltaTime);
        m_camera->MoveRight(input.x * m_movementSpeed * deltaTime);
        m_camera->MoveUp(input.y * m_movementSpeed * deltaTime);
    }
    
    // Decay input accumulator
    m_inputAccumulator = m_inputAccumulator * 0.95f;
}

Camera* FreeCameraMode::GetCamera() {
    return m_camera.get();
}

const Camera* FreeCameraMode::GetCamera() const {
    return m_camera.get();
}

void FreeCameraMode::Reset() {
    m_camera->SetPosition(Vector3(0, 5, -10));
    m_camera->SetEulerAngles(0, 0, 0);
    m_inputAccumulator = Vector3(0, 0, 0);
}

void FreeCameraMode::OnKeyboardInput(float moveForward, float moveRight, float moveUp) {
    m_inputAccumulator = Vector3(moveRight, moveUp, moveForward);
}

void FreeCameraMode::OnMouseMovement(float deltaMouseX, float deltaMouseY, float mouseSensitivity) {
    float yawDelta = deltaMouseX * mouseSensitivity * m_rotationSpeed;
    float pitchDelta = -deltaMouseY * mouseSensitivity * m_rotationSpeed;
    
    m_camera->RotateYaw(yawDelta);
    m_camera->RotatePitch(pitchDelta);
}

// ============================================================================
// RTS Camera Mode
// ============================================================================

RTSCameraMode::RTSCameraMode()
    : m_camera(std::make_unique<Camera>())
    , m_focusPoint(0, 0, 0)
    , m_zoom(1.0f)
    , m_baseDistance(30.0f)
    , m_viewAnglePitch(-0.785398f) {  // -45 degrees
    
    m_camera->SetMode(Camera::Mode::RTS);
    m_targetFocusPoint = m_focusPoint;
    m_targetZoom = m_zoom;
    
    // Set initial camera position for isometric view
    Vector3 cameraOffset;
    cameraOffset.x = m_baseDistance * std::sin(0);  // No yaw
    cameraOffset.y = m_baseDistance * std::sin(m_viewAnglePitch);
    cameraOffset.z = m_baseDistance * std::cos(0);  // No yaw
    
    m_camera->SetPosition(m_focusPoint + cameraOffset * m_zoom);
    m_camera->LookAt(m_focusPoint);
}

void RTSCameraMode::Update(float deltaTime) {
    // Smooth damping to target focus point
    Vector3 focusOffset = m_targetFocusPoint - m_focusPoint;
    if (focusOffset.Length() > 0.001f) {
        m_focusPoint = m_focusPoint + focusOffset * m_dampingFactor * deltaTime;
    }
    
    // Smooth damping to target zoom
    if (std::abs(m_targetZoom - m_zoom) > 0.001f) {
        m_zoom = m_zoom + (m_targetZoom - m_zoom) * m_dampingFactor * deltaTime;
    }
    
    // Calculate camera position from focus, distance, and zoom
    float distance = m_baseDistance * m_zoom;
    
    // Isometric view: camera above and behind focus point
    // Calculate position on sphere at fixed viewing angles
    Vector3 cameraOffset;
    const float yaw = 0.0f;  // Fixed horizontal angle (can be parameterized)
    
    cameraOffset.x = distance * std::sin(yaw) * std::cos(m_viewAnglePitch);
    cameraOffset.y = distance * std::sin(m_viewAnglePitch);
    cameraOffset.z = distance * std::cos(yaw) * std::cos(m_viewAnglePitch);
    
    m_camera->SetPosition(m_focusPoint + cameraOffset);
    m_camera->LookAt(m_focusPoint);
}

Camera* RTSCameraMode::GetCamera() {
    return m_camera.get();
}

const Camera* RTSCameraMode::GetCamera() const {
    return m_camera.get();
}

void RTSCameraMode::Reset() {
    m_focusPoint = Vector3(0, 0, 0);
    m_targetFocusPoint = m_focusPoint;
    m_zoom = 1.0f;
    m_targetZoom = m_zoom;
}

void RTSCameraMode::SetFocusPoint(const Vector3& position) {
    m_targetFocusPoint = position;
}

void RTSCameraMode::SetZoom(float zoom) {
    m_targetZoom = std::max(m_minZoom, std::min(m_maxZoom, zoom));
}

void RTSCameraMode::Pan(const Vector3& panDelta) {
    m_targetFocusPoint = m_targetFocusPoint + panDelta;
}

void RTSCameraMode::OnKeyboardInput(float panX, float panZ, float zoomDelta) {
    Vector3 panVector = Vector3(panX, 0, panZ) * m_panSpeed * 0.016f;  // Assume ~60fps
    Pan(panVector);
    
    if (std::abs(zoomDelta) > 0.001f) {
        SetZoom(m_targetZoom + zoomDelta * m_zoomSpeed * 0.016f);
    }
}

void RTSCameraMode::OnMouseScroll(float scrollDelta, float zoomSensitivity) {
    float zoomChange = -scrollDelta * zoomSensitivity;
    SetZoom(m_targetZoom + zoomChange);
}

// ============================================================================
// Chase Camera Mode
// ============================================================================

ChaseCameraMode::ChaseCameraMode()
    : m_camera(std::make_unique<Camera>())
    , m_targetPosition(0, 0, 0)
    , m_currentPosition(0, 5, -10)
    , m_offset(0, 5, -10)
    , m_distance(15.0f) {
    
    m_camera->SetMode(Camera::Mode::CHASE);
    m_camera->SetPosition(m_currentPosition);
}

void ChaseCameraMode::Update(float deltaTime) {
    // Calculate desired camera position (target + offset)
    Vector3 desiredPosition = m_targetPosition + m_offset;
    
    // Apply look-ahead if camera is following moving target
    if (m_lookAhead > 0.001f) {
        // Look-ahead offset would be added based on target velocity
        // For now, just calculate based on direction
    }
    
    // Smooth damping to desired position
    Vector3 positionDelta = desiredPosition - m_currentPosition;
    if (positionDelta.Length() > 0.001f) {
        m_currentPosition = m_currentPosition + positionDelta * m_damping * deltaTime;
    }
    
    // Update camera position and look at target
    m_camera->SetPosition(m_currentPosition);
    m_camera->LookAt(m_targetPosition);
}

Camera* ChaseCameraMode::GetCamera() {
    return m_camera.get();
}

const Camera* ChaseCameraMode::GetCamera() const {
    return m_camera.get();
}

void ChaseCameraMode::Reset() {
    m_targetPosition = Vector3(0, 0, 0);
    m_offset = Vector3(0, 5, -10);
    m_distance = 15.0f;
    m_currentPosition = m_targetPosition + m_offset;
    m_camera->SetPosition(m_currentPosition);
}

void ChaseCameraMode::SetTarget(const Vector3& targetPosition) {
    m_targetPosition = targetPosition;
}

void ChaseCameraMode::SetOffset(const Vector3& offset) {
    m_offset = offset;
    m_distance = offset.Length();
}

void ChaseCameraMode::SetDistance(float distance) {
    m_distance = std::max(m_minDistance, std::min(m_maxDistance, distance));
    
    // Adjust offset to maintain direction but change magnitude
    if (m_offset.Length() > 0.001f) {
        m_offset = m_offset.Normalize() * m_distance;
    } else {
        m_offset = Vector3(0, m_distance / 2.0f, -m_distance);
    }
}

void ChaseCameraMode::OnMouseScroll(float scrollDelta, float sensitivity) {
    float newDistance = m_distance - scrollDelta * sensitivity;
    SetDistance(newDistance);
}

// ============================================================================
// Orbit Camera Mode
// ============================================================================

OrbitCameraMode::OrbitCameraMode()
    : m_camera(std::make_unique<Camera>())
    , m_orbitCenter(0, 0, 0)
    , m_distance(20.0f)
    , m_pitch(-0.785398f)
    , m_yaw(0.0f) {
    
    m_camera->SetMode(Camera::Mode::ORBIT);
}

void OrbitCameraMode::Update(float deltaTime) {
    // Apply auto-rotation if enabled
    if (m_autoRotateEnabled && std::abs(m_autoRotationSpeed) > 0.001f) {
        float radiansPerSecond = CameraRotation::DegreesToRadians(m_autoRotationSpeed);
        m_yaw += radiansPerSecond * deltaTime;
        
        // Normalize yaw
        while (m_yaw > 3.14159265f) m_yaw -= 6.28318530f;
        while (m_yaw < -3.14159265f) m_yaw += 6.28318530f;
    }
    
    // Calculate camera position on orbit sphere
    float cosYaw = std::cos(m_yaw);
    float sinYaw = std::sin(m_yaw);
    float cosPitch = std::cos(m_pitch);
    float sinPitch = std::sin(m_pitch);
    
    Vector3 cameraPos;
    cameraPos.x = m_orbitCenter.x + m_distance * cosYaw * cosPitch;
    cameraPos.y = m_orbitCenter.y + m_distance * sinPitch;
    cameraPos.z = m_orbitCenter.z + m_distance * sinYaw * cosPitch;
    
    m_camera->SetPosition(cameraPos);
    m_camera->LookAt(m_orbitCenter);
}

Camera* OrbitCameraMode::GetCamera() {
    return m_camera.get();
}

const Camera* OrbitCameraMode::GetCamera() const {
    return m_camera.get();
}

void OrbitCameraMode::Reset() {
    m_orbitCenter = Vector3(0, 0, 0);
    m_distance = 20.0f;
    m_pitch = -0.785398f;
    m_yaw = 0.0f;
}

void OrbitCameraMode::SetOrbitCenter(const Vector3& center) {
    m_orbitCenter = center;
}

void OrbitCameraMode::SetDistance(float distance) {
    if (distance > 0.1f) {
        m_distance = distance;
    }
}

void OrbitCameraMode::SetOrbitAngles(float pitch, float yaw) {
    m_pitch = CameraRotation::ClampPitchToAvoidGimbalLock(pitch);
    m_yaw = yaw;
    
    // Normalize yaw
    while (m_yaw > 3.14159265f) m_yaw -= 6.28318530f;
    while (m_yaw < -3.14159265f) m_yaw += 6.28318530f;
}

void OrbitCameraMode::RotateOrbit(float deltaPitch, float deltaYaw) {
    m_pitch = CameraRotation::ClampPitchToAvoidGimbalLock(m_pitch + deltaPitch);
    m_yaw += deltaYaw;
    
    // Normalize yaw
    while (m_yaw > 3.14159265f) m_yaw -= 6.28318530f;
    while (m_yaw < -3.14159265f) m_yaw += 6.28318530f;
}

// ============================================================================
// Camera Mode Manager
// ============================================================================

CameraModeManager::CameraModeManager()
    : m_freeMode(std::make_unique<FreeCameraMode>())
    , m_rtsMode(std::make_unique<RTSCameraMode>())
    , m_chaseMode(std::make_unique<ChaseCameraMode>())
    , m_orbitMode(std::make_unique<OrbitCameraMode>())
    , m_currentMode(Camera::Mode::FREE) {
}

void CameraModeManager::Update(float deltaTime) {
    switch (m_currentMode) {
        case Camera::Mode::FREE:
            m_freeMode->Update(deltaTime);
            break;
        case Camera::Mode::RTS:
            m_rtsMode->Update(deltaTime);
            break;
        case Camera::Mode::CHASE:
            m_chaseMode->Update(deltaTime);
            break;
        case Camera::Mode::ORBIT:
            m_orbitMode->Update(deltaTime);
            break;
    }
}

void CameraModeManager::SetMode(Camera::Mode mode) {
    m_currentMode = mode;
}

Camera::Mode CameraModeManager::GetCurrentMode() const {
    return m_currentMode;
}

Camera* CameraModeManager::GetCamera() {
    switch (m_currentMode) {
        case Camera::Mode::FREE:
            return m_freeMode->GetCamera();
        case Camera::Mode::RTS:
            return m_rtsMode->GetCamera();
        case Camera::Mode::CHASE:
            return m_chaseMode->GetCamera();
        case Camera::Mode::ORBIT:
            return m_orbitMode->GetCamera();
        default:
            return nullptr;
    }
}

const Camera* CameraModeManager::GetCamera() const {
    switch (m_currentMode) {
        case Camera::Mode::FREE:
            return m_freeMode->GetCamera();
        case Camera::Mode::RTS:
            return m_rtsMode->GetCamera();
        case Camera::Mode::CHASE:
            return m_chaseMode->GetCamera();
        case Camera::Mode::ORBIT:
            return m_orbitMode->GetCamera();
        default:
            return nullptr;
    }
}

CameraMode* CameraModeManager::GetMode(Camera::Mode mode) {
    switch (mode) {
        case Camera::Mode::FREE:
            return m_freeMode.get();
        case Camera::Mode::RTS:
            return m_rtsMode.get();
        case Camera::Mode::CHASE:
            return m_chaseMode.get();
        case Camera::Mode::ORBIT:
            return m_orbitMode.get();
        default:
            return nullptr;
    }
}

const CameraMode* CameraModeManager::GetMode(Camera::Mode mode) const {
    switch (mode) {
        case Camera::Mode::FREE:
            return m_freeMode.get();
        case Camera::Mode::RTS:
            return m_rtsMode.get();
        case Camera::Mode::CHASE:
            return m_chaseMode.get();
        case Camera::Mode::ORBIT:
            return m_orbitMode.get();
        default:
            return nullptr;
    }
}
