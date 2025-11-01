#include "camera_input.h"
#include "Core/GameEngine/Source/Camera/camera_rotation.h"
#include "Core/Math/math_utils.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>

/**
 * Phase 45 Stage 5: Controls & Animation Implementation
 * 
 * Handles user input and applies it to camera system
 * Includes smooth animations and transitions
 */

// ============================================================================
// Constructor & Initialization
// ============================================================================

CameraInputController::CameraInputController() {
    std::memset(m_inputStates, 0, sizeof(m_inputStates));
    std::memset(m_debugInfo, 0, sizeof(m_debugInfo));
    ResetInputMappings();
}

bool CameraInputController::Initialize(Camera* camera, CameraModeManager* modeManager) {
    m_camera = camera;
    m_cameraModeManager = modeManager;

    if (!m_camera || !m_cameraModeManager) {
        return false;
    }

    return true;
}

void CameraInputController::Update(float deltaTime) {
    if (!m_camera || !m_cameraModeManager) {
        return;
    }

    // Clear frame-specific input states
    for (int i = 0; i < static_cast<int>(CameraInputAction::COUNT); i++) {
        m_inputStates[i].justPressed = false;
        m_inputStates[i].justReleased = false;
    }

    // Process input
    ProcessKeyboardInput(deltaTime);
    ProcessMouseInput(deltaTime);

    // Update camera mode
    m_cameraModeManager->Update(deltaTime);

    // Update animations
    UpdateAnimations(deltaTime);

    // Update smooth movement
    if (m_smoothMovement) {
        UpdateSmoothMovement(deltaTime);
    }

    // Update debug info
    UpdateDebugInfo();
}

// ============================================================================
// Input State Management
// ============================================================================

const InputState& CameraInputController::GetInputState(CameraInputAction action) const {
    static InputState nullState;
    int index = static_cast<int>(action);
    if (index >= 0 && index < static_cast<int>(CameraInputAction::COUNT)) {
        return m_inputStates[index];
    }
    return nullState;
}

void CameraInputController::SetInputState(CameraInputAction action, bool isDown, float analogValue) {
    int index = static_cast<int>(action);
    if (index < 0 || index >= static_cast<int>(CameraInputAction::COUNT)) {
        return;
    }

    InputState& state = m_inputStates[index];

    if (isDown && !state.isDown) {
        state.justPressed = true;
    } else if (!isDown && state.isDown) {
        state.justReleased = true;
    }

    state.isDown = isDown;
    state.analogValue = analogValue;
}

bool CameraInputController::IsActionDown(CameraInputAction action) const {
    return GetInputState(action).isDown;
}

bool CameraInputController::IsActionJustPressed(CameraInputAction action) const {
    return GetInputState(action).justPressed;
}

bool CameraInputController::IsActionJustReleased(CameraInputAction action) const {
    return GetInputState(action).justReleased;
}

// ============================================================================
// Mouse Input
// ============================================================================

void CameraInputController::OnMouseMove(float deltaMouseX, float deltaMouseY) {
    m_lastMouseX = m_mouseX;
    m_lastMouseY = m_mouseY;
    m_mouseX += deltaMouseX;
    m_mouseY += deltaMouseY;
}

void CameraInputController::OnMouseScroll(float scrollDelta) {
    m_mouseScrollDelta = scrollDelta;
}

void CameraInputController::OnMouseButtonDown(int buttonIndex) {
    // Can be used for special interactions
}

void CameraInputController::OnMouseButtonUp(int buttonIndex) {
    // Can be used for special interactions
}

void CameraInputController::GetMousePosition(float& outX, float& outY) const {
    outX = m_mouseX;
    outY = m_mouseY;
}

void CameraInputController::GetMouseDelta(float& outDeltaX, float& outDeltaY) const {
    outDeltaX = m_mouseX - m_lastMouseX;
    outDeltaY = m_mouseY - m_lastMouseY;
}

// ============================================================================
// Animation Framework
// ============================================================================

void CameraInputController::AnimateToPosition(const Vector3& targetPosition, float duration, int easeFunction) {
    if (!m_camera || duration <= 0.0f) {
        return;
    }

    m_isAnimating = true;
    m_animationDuration = duration;
    m_animationElapsed = 0.0f;
    m_animationProgress = 0.0f;
    m_animationType = 0;  // Position animation
    m_animationStartPos = m_camera->GetPosition();
    m_animationTargetPos = targetPosition;
}

void CameraInputController::AnimateLookAt(const Vector3& targetPoint, float duration) {
    if (!m_camera || duration <= 0.0f) {
        return;
    }

    m_isAnimating = true;
    m_animationDuration = duration;
    m_animationElapsed = 0.0f;
    m_animationProgress = 0.0f;
    m_animationType = 1;  // Look-at animation
    m_animationStartLookAt = m_camera->GetPosition() + m_camera->GetForward();
    m_animationTargetLookAt = targetPoint;
}

void CameraInputController::AnimateFocusOn(const Vector3& targetPosition, const Vector3& lookAtPoint, float distance, float duration) {
    if (!m_camera || duration <= 0.0f) {
        return;
    }

    // Calculate camera position at desired distance
    Vector3 direction = (targetPosition - lookAtPoint).Normalize();
    Vector3 cameraPos = targetPosition + direction * distance;

    m_isAnimating = true;
    m_animationDuration = duration;
    m_animationElapsed = 0.0f;
    m_animationProgress = 0.0f;
    m_animationType = 2;  // Focus animation (combined)
    m_animationStartPos = m_camera->GetPosition();
    m_animationTargetPos = cameraPos;
    m_animationTargetLookAt = lookAtPoint;
}

void CameraInputController::AnimateOrbit(const Vector3& centerPoint, float radius, float degreesPerSecond) {
    m_isAnimating = true;
    m_animationType = 3;  // Orbit animation
    m_animationElapsed = 0.0f;
    m_orbitRadius = radius;
    m_orbitSpeed = CameraRotation::DegreesToRadians(degreesPerSecond);
}

void CameraInputController::StopAnimations() {
    m_isAnimating = false;
    m_animationType = -1;
}

// ============================================================================
// Input Processing
// ============================================================================

void CameraInputController::ProcessKeyboardInput(float deltaTime) {
    Camera* camera = m_cameraModeManager->GetCamera();
    if (!camera) {
        return;
    }

    // Get current mode-specific camera
    FreeCameraMode* freeMode = m_cameraModeManager->GetFreeMode();
    RTSCameraMode* rtsMode = m_cameraModeManager->GetRTSMode();
    ChaseCameraMode* chaseMode = m_cameraModeManager->GetChaseMode();

    // Switch camera modes
    if (IsActionJustPressed(CameraInputAction::SWITCH_FREE_CAMERA)) {
        m_cameraModeManager->SetMode(Camera::Mode::FREE);
    }
    if (IsActionJustPressed(CameraInputAction::SWITCH_RTS_CAMERA)) {
        m_cameraModeManager->SetMode(Camera::Mode::RTS);
    }
    if (IsActionJustPressed(CameraInputAction::SWITCH_CHASE_CAMERA)) {
        m_cameraModeManager->SetMode(Camera::Mode::CHASE);
    }
    if (IsActionJustPressed(CameraInputAction::SWITCH_ORBIT_CAMERA)) {
        m_cameraModeManager->SetMode(Camera::Mode::ORBIT);
    }

    // Reset camera
    if (IsActionJustPressed(CameraInputAction::RESET_CAMERA)) {
        CameraMode* mode = m_cameraModeManager->GetMode(m_cameraModeManager->GetCurrentMode());
        if (mode) {
            mode->Reset();
        }
    }

    // Mode-specific input
    switch (m_cameraModeManager->GetCurrentMode()) {
        case Camera::Mode::FREE: {
            if (freeMode) {
                float moveForward = 0.0f;
                float moveRight = 0.0f;
                float moveUp = 0.0f;

                if (IsActionDown(CameraInputAction::MOVE_FORWARD)) moveForward = 1.0f;
                if (IsActionDown(CameraInputAction::MOVE_BACKWARD)) moveForward = -1.0f;
                if (IsActionDown(CameraInputAction::MOVE_RIGHT)) moveRight = 1.0f;
                if (IsActionDown(CameraInputAction::MOVE_LEFT)) moveRight = -1.0f;
                if (IsActionDown(CameraInputAction::MOVE_UP)) moveUp = 1.0f;
                if (IsActionDown(CameraInputAction::MOVE_DOWN)) moveUp = -1.0f;

                freeMode->OnKeyboardInput(moveForward, moveRight, moveUp);

                // Handle rotation input via mouse (will be applied in ProcessMouseInput)
            }
            break;
        }

        case Camera::Mode::RTS: {
            if (rtsMode) {
                float panX = 0.0f;
                float panZ = 0.0f;
                float zoomDelta = 0.0f;

                if (IsActionDown(CameraInputAction::MOVE_RIGHT)) panX = 1.0f;
                if (IsActionDown(CameraInputAction::MOVE_LEFT)) panX = -1.0f;
                if (IsActionDown(CameraInputAction::MOVE_FORWARD)) panZ = 1.0f;
                if (IsActionDown(CameraInputAction::MOVE_BACKWARD)) panZ = -1.0f;
                if (IsActionDown(CameraInputAction::ZOOM_IN)) zoomDelta = 1.0f;
                if (IsActionDown(CameraInputAction::ZOOM_OUT)) zoomDelta = -1.0f;

                rtsMode->OnKeyboardInput(panX, panZ, zoomDelta);
            }
            break;
        }

        case Camera::Mode::CHASE: {
            if (chaseMode) {
                if (IsActionDown(CameraInputAction::ZOOM_IN)) {
                    chaseMode->SetDistance(chaseMode->GetDistance() - 1.0f);
                }
                if (IsActionDown(CameraInputAction::ZOOM_OUT)) {
                    chaseMode->SetDistance(chaseMode->GetDistance() + 1.0f);
                }
            }
            break;
        }

        case Camera::Mode::ORBIT: {
            // Orbit rotation handled by mouse
            break;
        }
    }
}

void CameraInputController::ProcessMouseInput(float deltaTime) {
    float deltaX, deltaY;
    GetMouseDelta(deltaX, deltaY);

    // Apply mouse look to free camera
    if (m_cameraModeManager->GetCurrentMode() == Camera::Mode::FREE) {
        FreeCameraMode* freeMode = m_cameraModeManager->GetFreeMode();
        if (freeMode && (std::abs(deltaX) > 0.001f || std::abs(deltaY) > 0.001f)) {
            float yDelta = m_invertedYAxis ? deltaY : -deltaY;
            freeMode->OnMouseMovement(deltaX, yDelta, m_mouseSensitivity * 0.01f);
        }
    }

    // Apply mouse scroll
    if (std::abs(m_mouseScrollDelta) > 0.001f) {
        RTSCameraMode* rtsMode = m_cameraModeManager->GetRTSMode();
        if (rtsMode) {
            rtsMode->OnMouseScroll(m_mouseScrollDelta, 0.1f);
        }

        ChaseCameraMode* chaseMode = m_cameraModeManager->GetChaseMode();
        if (chaseMode) {
            chaseMode->OnMouseScroll(m_mouseScrollDelta, 0.5f);
        }

        m_mouseScrollDelta = 0.0f;
    }
}

void CameraInputController::UpdateAnimations(float deltaTime) {
    if (!m_isAnimating || !m_camera) {
        return;
    }

    m_animationElapsed += deltaTime;
    m_animationProgress = std::min(1.0f, m_animationElapsed / m_animationDuration);

    // Apply easing
    float easedProgress = ApplyEasingFunction(m_animationProgress, 0);

    switch (m_animationType) {
        case 0: {  // Position animation
            Vector3 newPos = m_animationStartPos + (m_animationTargetPos - m_animationStartPos) * easedProgress;
            m_camera->SetPosition(newPos);
            break;
        }

        case 1: {  // Look-at animation
            Vector3 newLookAt = m_animationStartLookAt + (m_animationTargetLookAt - m_animationStartLookAt) * easedProgress;
            m_camera->LookAt(newLookAt);
            break;
        }

        case 2: {  // Focus animation
            Vector3 newPos = m_animationStartPos + (m_animationTargetPos - m_animationStartPos) * easedProgress;
            m_camera->SetPosition(newPos);
            m_camera->LookAt(m_animationTargetLookAt);
            break;
        }

        case 3: {  // Orbit animation
            // Continuously orbit
            m_animationElapsed += deltaTime;
            break;
        }
    }

    // Stop animation if complete
    if (m_animationProgress >= 1.0f) {
        m_isAnimating = false;
        m_animationType = -1;
    }
}

float CameraInputController::ApplyEasingFunction(float t, int easeFunction) const {
    switch (easeFunction) {
        case 0:  // Linear
            return t;
        case 1:  // Ease-in (quadratic)
            return t * t;
        case 2:  // Ease-out (quadratic)
            return t * (2.0f - t);
        case 3:  // Ease-in-out
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        default:
            return t;
    }
}

void CameraInputController::UpdateSmoothMovement(float deltaTime) {
    // Decay velocity over time
    m_velocityAccumulator = m_velocityAccumulator * (1.0f - m_smoothDamping);
}

// ============================================================================
// Input Mapping
// ============================================================================

void CameraInputController::MapKeyToAction(uint32_t keyCode, CameraInputAction action) {
    int index = static_cast<int>(action);
    if (index >= 0 && index < static_cast<int>(CameraInputAction::COUNT)) {
        m_keyBindings[index] = keyCode;
    }
}

CameraInputAction CameraInputController::GetActionForKey(uint32_t keyCode) const {
    for (int i = 0; i < static_cast<int>(CameraInputAction::COUNT); i++) {
        if (m_keyBindings[i] == keyCode) {
            return static_cast<CameraInputAction>(i);
        }
    }
    return CameraInputAction::COUNT;
}

uint32_t CameraInputController::GetKeyForAction(CameraInputAction action) const {
    int index = static_cast<int>(action);
    if (index >= 0 && index < static_cast<int>(CameraInputAction::COUNT)) {
        return m_keyBindings[index];
    }
    return 0;
}

void CameraInputController::ResetInputMappings() {
    // Default WASD + arrow keys mapping
    MapKeyToAction('W', CameraInputAction::MOVE_FORWARD);
    MapKeyToAction('S', CameraInputAction::MOVE_BACKWARD);
    MapKeyToAction('A', CameraInputAction::MOVE_LEFT);
    MapKeyToAction('D', CameraInputAction::MOVE_RIGHT);
    MapKeyToAction('Q', CameraInputAction::MOVE_DOWN);
    MapKeyToAction('E', CameraInputAction::MOVE_UP);

    // Arrow keys as alternates
    MapKeyToAction(265, CameraInputAction::ROTATE_UP);      // Up arrow
    MapKeyToAction(264, CameraInputAction::ROTATE_DOWN);    // Down arrow
    MapKeyToAction(263, CameraInputAction::ROTATE_LEFT);    // Left arrow
    MapKeyToAction(262, CameraInputAction::ROTATE_RIGHT);   // Right arrow

    // Mode switching
    MapKeyToAction('1', CameraInputAction::SWITCH_FREE_CAMERA);
    MapKeyToAction('2', CameraInputAction::SWITCH_RTS_CAMERA);
    MapKeyToAction('3', CameraInputAction::SWITCH_CHASE_CAMERA);
    MapKeyToAction('4', CameraInputAction::SWITCH_ORBIT_CAMERA);

    // Special
    MapKeyToAction('R', CameraInputAction::RESET_CAMERA);
    MapKeyToAction('`', CameraInputAction::TOGGLE_DEBUG);

    // Zoom
    MapKeyToAction('Z', CameraInputAction::ZOOM_IN);
    MapKeyToAction('X', CameraInputAction::ZOOM_OUT);
}

// ============================================================================
// Debugging
// ============================================================================

void CameraInputController::UpdateDebugInfo() {
    const char* modeName = "Unknown";
    switch (m_cameraModeManager->GetCurrentMode()) {
        case Camera::Mode::FREE:
            modeName = "Free";
            break;
        case Camera::Mode::RTS:
            modeName = "RTS";
            break;
        case Camera::Mode::CHASE:
            modeName = "Chase";
            break;
        case Camera::Mode::ORBIT:
            modeName = "Orbit";
            break;
    }

    std::snprintf(m_debugInfo, sizeof(m_debugInfo),
        "Camera Control: Mode=%s Smooth=%s Sensitivity=%.2f Speed=%.1f %s",
        modeName,
        m_smoothMovement ? "ON" : "OFF",
        m_mouseSensitivity,
        m_movementSpeed,
        m_isAnimating ? "[ANIMATING]" : "");
}
