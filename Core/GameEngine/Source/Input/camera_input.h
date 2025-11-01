#pragma once

#include "Core/GameEngine/Source/Camera/camera.h"
#include "Core/GameEngine/Source/Camera/camera_modes.h"
#include "Core/Math/vector.h"
#include <cstdint>
#include <memory>

/**
 * Phase 45 Stage 5: Controls & Animation
 * 
 * Implements keyboard and mouse input handling for camera control
 * Includes:
 * - Keyboard input mapping (WASD, arrow keys, etc.)
 * - Mouse movement and scroll handling
 * - Smooth camera animations and transitions
 * - Configurable input sensitivity and speed
 * - Animation framework for cinematic cameras
 */

/**
 * Enumeration for supported input actions
 */
enum class CameraInputAction {
    // Movement
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,

    // Rotation
    ROTATE_UP,
    ROTATE_DOWN,
    ROTATE_LEFT,
    ROTATE_RIGHT,
    ROLL_LEFT,
    ROLL_RIGHT,

    // Zoom (RTS/Chase camera)
    ZOOM_IN,
    ZOOM_OUT,

    // Camera mode switching
    SWITCH_FREE_CAMERA,
    SWITCH_RTS_CAMERA,
    SWITCH_CHASE_CAMERA,
    SWITCH_ORBIT_CAMERA,

    // Special
    RESET_CAMERA,
    TOGGLE_DEBUG,

    COUNT
};

/**
 * Input state for each action (down, just pressed, just released)
 */
struct InputState {
    bool isDown = false;           // Button currently pressed
    bool justPressed = false;      // Button pressed this frame
    bool justReleased = false;     // Button released this frame
    float analogValue = 0.0f;      // Analog axis value (0-1)
};

/**
 * Manages camera input and animations
 * Handles all user input and applies it to camera
 */
class CameraInputController {
public:
    CameraInputController();
    ~CameraInputController() = default;

    // =========================================================================
    // Initialization
    // =========================================================================

    /**
     * Initialize input controller
     * @param camera Camera to control
     * @param modeManager Camera mode manager
     * @return true if initialization successful
     */
    bool Initialize(Camera* camera, CameraModeManager* modeManager);

    /**
     * Update input and apply to camera
     * Should be called once per frame
     * @param deltaTime Time elapsed since last update (seconds)
     */
    void Update(float deltaTime);

    // =========================================================================
    // Input State Management
    // =========================================================================

    /**
     * Get input state for specific action
     * @param action Input action to query
     * @return Input state for that action
     */
    const InputState& GetInputState(CameraInputAction action) const;

    /**
     * Set input state for action (called by input system)
     * @param action Input action
     * @param isDown Whether action is currently active
     * @param analogValue Optional analog value (0-1)
     */
    void SetInputState(CameraInputAction action, bool isDown, float analogValue = 0.0f);

    /**
     * Check if action is currently held
     * @param action Input action
     * @return true if action is down
     */
    bool IsActionDown(CameraInputAction action) const;

    /**
     * Check if action was just pressed this frame
     * @param action Input action
     * @return true if just pressed
     */
    bool IsActionJustPressed(CameraInputAction action) const;

    /**
     * Check if action was just released this frame
     * @param action Input action
     * @return true if just released
     */
    bool IsActionJustReleased(CameraInputAction action) const;

    // =========================================================================
    // Sensitivity & Speed Configuration
    // =========================================================================

    /**
     * Set mouse look sensitivity
     * @param sensitivity Sensitivity multiplier (default: 1.0)
     */
    void SetMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

    /**
     * Get mouse look sensitivity
     * @return Current sensitivity
     */
    float GetMouseSensitivity() const { return m_mouseSensitivity; }

    /**
     * Set camera movement speed
     * @param speed Speed in units per second
     */
    void SetMovementSpeed(float speed) { m_movementSpeed = speed; }

    /**
     * Get camera movement speed
     * @return Current speed
     */
    float GetMovementSpeed() const { return m_movementSpeed; }

    /**
     * Set camera rotation speed
     * @param speed Rotation speed (radians per second)
     */
    void SetRotationSpeed(float speed) { m_rotationSpeed = speed; }

    /**
     * Get camera rotation speed
     * @return Current speed
     */
    float GetRotationSpeed() const { return m_rotationSpeed; }

    /**
     * Enable/disable inverted mouse Y-axis
     * @param inverted true for inverted Y-axis
     */
    void SetInvertedYAxis(bool inverted) { m_invertedYAxis = inverted; }

    /**
     * Check if Y-axis is inverted
     * @return true if inverted
     */
    bool IsYAxisInverted() const { return m_invertedYAxis; }

    // =========================================================================
    // Mouse Input
    // =========================================================================

    /**
     * Apply mouse movement input
     * @param deltaMouseX Change in mouse X (pixels or raw input)
     * @param deltaMouseY Change in mouse Y (pixels or raw input)
     */
    void OnMouseMove(float deltaMouseX, float deltaMouseY);

    /**
     * Apply mouse wheel scroll
     * @param scrollDelta Mouse wheel delta (-1 = down, +1 = up)
     */
    void OnMouseScroll(float scrollDelta);

    /**
     * Apply mouse button press
     * @param buttonIndex Button index (0 = left, 1 = middle, 2 = right)
     */
    void OnMouseButtonDown(int buttonIndex);

    /**
     * Apply mouse button release
     * @param buttonIndex Button index
     */
    void OnMouseButtonUp(int buttonIndex);

    /**
     * Get current mouse position (last frame)
     * @param outX Output mouse X
     * @param outY Output mouse Y
     */
    void GetMousePosition(float& outX, float& outY) const;

    /**
     * Get mouse delta from last frame
     * @param outDeltaX Output mouse X delta
     * @param outDeltaY Output mouse Y delta
     */
    void GetMouseDelta(float& outDeltaX, float& outDeltaY) const;

    // =========================================================================
    // Animation Framework
    // =========================================================================

    /**
     * Start camera animation to target position
     * @param targetPosition Target world position
     * @param duration Animation duration in seconds
     * @param easeFunction Easing function type (0 = linear, 1 = ease-in, 2 = ease-out)
     */
    void AnimateToPosition(const Vector3& targetPosition, float duration, int easeFunction = 0);

    /**
     * Start camera animation to look at target
     * @param targetPoint World point to look at
     * @param duration Animation duration in seconds
     */
    void AnimateLookAt(const Vector3& targetPoint, float duration);

    /**
     * Start camera animation to focus on object
     * Combines position and look-at animation
     * @param targetPosition Target position
     * @param lookAtPoint Point to look at
     * @param distance Distance from target
     * @param duration Animation duration in seconds
     */
    void AnimateFocusOn(const Vector3& targetPosition, const Vector3& lookAtPoint, float distance, float duration);

    /**
     * Start orbit animation around point
     * @param centerPoint Center of orbit
     * @param radius Orbit radius
     * @param degreesPerSecond Rotation speed (360 = full circle per second)
     */
    void AnimateOrbit(const Vector3& centerPoint, float radius, float degreesPerSecond);

    /**
     * Stop all active animations
     */
    void StopAnimations();

    /**
     * Check if any animation is active
     * @return true if animating
     */
    bool IsAnimating() const { return m_isAnimating; }

    /**
     * Get current animation progress (0 to 1)
     * @return Progress percentage
     */
    float GetAnimationProgress() const { return m_animationProgress; }

    // =========================================================================
    // Smooth Movement
    // =========================================================================

    /**
     * Enable/disable smooth movement damping
     * Creates smooth acceleration/deceleration
     * @param enabled Enable smooth movement
     */
    void SetSmoothMovement(bool enabled) { m_smoothMovement = enabled; }

    /**
     * Check if smooth movement is enabled
     * @return true if enabled
     */
    bool IsSmoothMovementEnabled() const { return m_smoothMovement; }

    /**
     * Set smooth movement damping factor
     * @param damping Damping factor (0-1, higher = more damping)
     */
    void SetSmoothDamping(float damping) { m_smoothDamping = damping; }

    // =========================================================================
    // Input Mapping
    // =========================================================================

    /**
     * Map keyboard key to input action
     * @param keyCode Key code
     * @param action Input action
     */
    void MapKeyToAction(uint32_t keyCode, CameraInputAction action);

    /**
     * Get action mapped to keyboard key
     * @param keyCode Key code
     * @return Mapped action (COUNT if not mapped)
     */
    CameraInputAction GetActionForKey(uint32_t keyCode) const;

    /**
     * Get key code mapped to action
     * @param action Input action
     * @return Key code (0 if not mapped)
     */
    uint32_t GetKeyForAction(CameraInputAction action) const;

    /**
     * Reset all input mappings to defaults
     */
    void ResetInputMappings();

    // =========================================================================
    // Debugging
    // =========================================================================

    /**
     * Get debug input info string
     * @return Debug information
     */
    const char* GetDebugInfo() const { return m_debugInfo; }

    /**
     * Update debug information
     */
    void UpdateDebugInfo();

private:
    // =========================================================================
    // Internal State
    // =========================================================================

    Camera* m_camera = nullptr;
    CameraModeManager* m_cameraModeManager = nullptr;

    // Input state
    InputState m_inputStates[static_cast<int>(CameraInputAction::COUNT)];
    uint32_t m_keyBindings[static_cast<int>(CameraInputAction::COUNT)];

    // Mouse state
    float m_mouseX = 0.0f;
    float m_mouseY = 0.0f;
    float m_lastMouseX = 0.0f;
    float m_lastMouseY = 0.0f;
    float m_mouseScrollDelta = 0.0f;

    // Movement
    float m_mouseSensitivity = 1.0f;
    float m_movementSpeed = 20.0f;
    float m_rotationSpeed = 1.0f;
    bool m_invertedYAxis = false;
    bool m_smoothMovement = true;
    float m_smoothDamping = 0.15f;
    Vector3 m_velocityAccumulator;

    // Animation
    bool m_isAnimating = false;
    float m_animationDuration = 0.0f;
    float m_animationElapsed = 0.0f;
    float m_animationProgress = 0.0f;
    int m_animationType = -1;  // Type of active animation
    Vector3 m_animationStartPos;
    Vector3 m_animationTargetPos;
    Vector3 m_animationStartLookAt;
    Vector3 m_animationTargetLookAt;
    float m_orbitRadius = 0.0f;
    float m_orbitSpeed = 0.0f;

    // Debug
    char m_debugInfo[512];

    // =========================================================================
    // Internal Methods
    // =========================================================================

    /**
     * Process keyboard input
     */
    void ProcessKeyboardInput(float deltaTime);

    /**
     * Process mouse input
     */
    void ProcessMouseInput(float deltaTime);

    /**
     * Update animations
     */
    void UpdateAnimations(float deltaTime);

    /**
     * Apply easing function to animation progress
     */
    float ApplyEasingFunction(float t, int easeFunction) const;

    /**
     * Update smooth movement velocity
     */
    void UpdateSmoothMovement(float deltaTime);
};
