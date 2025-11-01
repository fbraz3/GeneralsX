#pragma once

#include "camera.h"
#include "Core/Math/vector.h"
#include <memory>

/**
 * Phase 45 Stage 3: Camera Modes
 * 
 * Provides specialized camera behaviors for different gameplay scenarios:
 * - Free Camera: Unrestricted movement for debugging/testing
 * - RTS Camera: Isometric view with restricted angles (Command & Conquer style)
 * - Chase Camera: Follow target object with smooth damping
 * - Orbit Camera: Rotate around a fixed point
 * 
 * Each mode encapsulates specific control schemes and constraints
 */

/**
 * Base class for camera mode implementations
 */
class CameraMode {
public:
    virtual ~CameraMode() = default;

    /**
     * Update camera based on mode-specific logic
     * @param deltaTime Time elapsed since last update (seconds)
     */
    virtual void Update(float deltaTime) = 0;

    /**
     * Get the underlying camera object
     * @return Pointer to camera
     */
    virtual Camera* GetCamera() = 0;
    virtual const Camera* GetCamera() const = 0;

    /**
     * Reset camera to default position/orientation for this mode
     */
    virtual void Reset() = 0;

    /**
     * Get mode type identifier
     * @return Mode type
     */
    virtual Camera::Mode GetModeType() const = 0;
};

// ============================================================================
// Free Camera Mode
// ============================================================================

/**
 * Unrestricted camera movement in all directions
 * Useful for debugging, testing, and exploring scenes
 */
class FreeCameraMode : public CameraMode {
public:
    FreeCameraMode();
    ~FreeCameraMode() = default;

    void Update(float deltaTime) override;
    Camera* GetCamera() override;
    const Camera* GetCamera() const override;
    void Reset() override;
    Camera::Mode GetModeType() const override { return Camera::Mode::FREE; }

    /**
     * Set camera movement speed (world units per second)
     * @param speed Movement speed
     */
    void SetMovementSpeed(float speed) { m_movementSpeed = speed; }

    /**
     * Get camera movement speed
     * @return Speed value
     */
    float GetMovementSpeed() const { return m_movementSpeed; }

    /**
     * Set camera rotation speed (radians per second for mouse input)
     * @param speed Rotation speed
     */
    void SetRotationSpeed(float speed) { m_rotationSpeed = speed; }

    /**
     * Get camera rotation speed
     * @return Speed value
     */
    float GetRotationSpeed() const { return m_rotationSpeed; }

    /**
     * Apply keyboard input for movement
     * @param moveForward Forward movement input (-1 to 1)
     * @param moveRight Right movement input (-1 to 1)
     * @param moveUp Up movement input (-1 to 1)
     */
    void OnKeyboardInput(float moveForward, float moveRight, float moveUp);

    /**
     * Apply mouse movement for rotation
     * @param deltaMouseX Mouse X movement (pixels or raw input)
     * @param deltaMouseY Mouse Y movement (pixels or raw input)
     * @param mouseSensitivity Sensitivity multiplier
     */
    void OnMouseMovement(float deltaMouseX, float deltaMouseY, float mouseSensitivity = 0.01f);

private:
    std::unique_ptr<Camera> m_camera;
    float m_movementSpeed = 20.0f;  // Units per second
    float m_rotationSpeed = 1.0f;   // Radians per unit input
    Vector3 m_inputAccumulator;      // Accumulated keyboard input
    float m_accumulatorTimeout = 0.0f;
};

// ============================================================================
// RTS Camera Mode
// ============================================================================

/**
 * Isometric-style camera for real-time strategy games
 * Fixed viewing angle, panning and zooming support
 * Simulates Command & Conquer perspective
 */
class RTSCameraMode : public CameraMode {
public:
    RTSCameraMode();
    ~RTSCameraMode() = default;

    void Update(float deltaTime) override;
    Camera* GetCamera() override;
    const Camera* GetCamera() const override;
    void Reset() override;
    Camera::Mode GetModeType() const override { return Camera::Mode::RTS; }

    /**
     * Set camera focus point (what the camera looks at)
     * @param position World position to focus on
     */
    void SetFocusPoint(const Vector3& position);

    /**
     * Get camera focus point
     * @return Current focus point
     */
    const Vector3& GetFocusPoint() const { return m_focusPoint; }

    /**
     * Set camera zoom level (affects distance from focus point)
     * @param zoom Zoom factor (1.0 = default, < 1.0 = closer, > 1.0 = farther)
     */
    void SetZoom(float zoom);

    /**
     * Get current zoom level
     * @return Zoom factor
     */
    float GetZoom() const { return m_zoom; }

    /**
     * Pan camera (move focus point)
     * @param panDelta Panning direction and distance
     */
    void Pan(const Vector3& panDelta);

    /**
     * Apply keyboard input for panning/zooming
     * @param panX Horizontal panning input (-1 to 1)
     * @param panZ Vertical panning input (-1 to 1, forward/backward)
     * @param zoomDelta Zoom change input (-1 to 1)
     */
    void OnKeyboardInput(float panX, float panZ, float zoomDelta = 0.0f);

    /**
     * Apply mouse scroll for zooming
     * @param scrollDelta Mouse wheel scroll amount
     * @param zoomSensitivity Sensitivity multiplier
     */
    void OnMouseScroll(float scrollDelta, float zoomSensitivity = 0.1f);

    /**
     * Set panning speed (units per second)
     * @param speed Pan speed
     */
    void SetPanSpeed(float speed) { m_panSpeed = speed; }

    /**
     * Set zoom speed (zoom factor change per second)
     * @param speed Zoom speed
     */
    void SetZoomSpeed(float speed) { m_zoomSpeed = speed; }

    /**
     * Set zoom limits
     * @param minZoom Minimum zoom factor
     * @param maxZoom Maximum zoom factor
     */
    void SetZoomLimits(float minZoom, float maxZoom) {
        m_minZoom = minZoom;
        m_maxZoom = maxZoom;
    }

private:
    std::unique_ptr<Camera> m_camera;
    Vector3 m_focusPoint = Vector3(0, 0, 0);    // Point camera looks at
    float m_zoom = 1.0f;                        // Zoom factor
    float m_baseDistance = 30.0f;               // Distance at zoom=1.0
    float m_viewAnglePitch = -0.785398f;        // -45 degrees (isometric)
    float m_panSpeed = 30.0f;                   // Units per second
    float m_zoomSpeed = 2.0f;                   // Zoom factor change per second
    float m_minZoom = 0.5f;                     // Minimum zoom
    float m_maxZoom = 3.0f;                     // Maximum zoom
    Vector3 m_targetFocusPoint;                 // Smooth damping target
    float m_targetZoom = 1.0f;                  // Smooth damping target
    float m_dampingFactor = 5.0f;               // Smooth damping speed
};

// ============================================================================
// Chase Camera Mode
// ============================================================================

/**
 * Camera that follows a target object
 * Maintains smooth distance and prevents clipping
 */
class ChaseCameraMode : public CameraMode {
public:
    ChaseCameraMode();
    ~ChaseCameraMode() = default;

    void Update(float deltaTime) override;
    Camera* GetCamera() override;
    const Camera* GetCamera() const override;
    void Reset() override;
    Camera::Mode GetModeType() const override { return Camera::Mode::CHASE; }

    /**
     * Set target to follow
     * @param targetPosition Position of target object
     */
    void SetTarget(const Vector3& targetPosition);

    /**
     * Get current target position
     * @return Target position
     */
    const Vector3& GetTarget() const { return m_targetPosition; }

    /**
     * Set camera offset relative to target
     * @param offset Position offset (camera relative to target)
     */
    void SetOffset(const Vector3& offset);

    /**
     * Get camera offset
     * @return Current offset
     */
    const Vector3& GetOffset() const { return m_offset; }

    /**
     * Set look-ahead point ahead of target
     * Useful for dynamic camera in fast-moving scenes
     * @param lookAhead Amount to look ahead (0 = at target, > 0 = ahead)
     */
    void SetLookAhead(float lookAhead) { m_lookAhead = lookAhead; }

    /**
     * Get look-ahead distance
     * @return Look-ahead value
     */
    float GetLookAhead() const { return m_lookAhead; }

    /**
     * Set distance from target
     * @param distance Distance (will be clamped to valid range)
     */
    void SetDistance(float distance);

    /**
     * Get distance from target
     * @return Current distance
     */
    float GetDistance() const { return m_distance; }

    /**
     * Set camera damping (smoothness of following)
     * @param damping Damping factor (0 = instant, > 0 = smooth)
     */
    void SetDamping(float damping) { m_damping = damping; }

    /**
     * Get camera damping value
     * @return Damping factor
     */
    float GetDamping() const { return m_damping; }

    /**
     * Set min/max distance bounds
     * @param minDist Minimum follow distance
     * @param maxDist Maximum follow distance
     */
    void SetDistanceBounds(float minDist, float maxDist) {
        m_minDistance = minDist;
        m_maxDistance = maxDist;
    }

    /**
     * Enable/disable collision avoidance
     * Camera will move closer if terrain blocks view
     * @param enabled Enable collision checks
     */
    void SetCollisionAvoidance(bool enabled) { m_collisionAvoidance = enabled; }

    /**
     * Apply mouse scroll for distance adjustment
     * @param scrollDelta Mouse wheel scroll
     * @param sensitivity Distance change per scroll
     */
    void OnMouseScroll(float scrollDelta, float sensitivity = 0.5f);

private:
    std::unique_ptr<Camera> m_camera;
    Vector3 m_targetPosition = Vector3(0, 0, 0);   // Target to follow
    Vector3 m_currentPosition = Vector3(0, 0, 0);  // Smooth camera position
    Vector3 m_offset = Vector3(0, 5, -10);         // Camera offset from target
    float m_distance = 15.0f;                       // Distance from target
    float m_lookAhead = 0.0f;                       // Look-ahead distance
    float m_damping = 5.0f;                         // Smoothing speed
    float m_minDistance = 5.0f;                     // Minimum distance
    float m_maxDistance = 50.0f;                    // Maximum distance
    bool m_collisionAvoidance = true;               // Enable collision checks
};

// ============================================================================
// Orbit Camera Mode
// ============================================================================

/**
 * Camera that orbits around a fixed point
 * Useful for object inspection or cinematic views
 */
class OrbitCameraMode : public CameraMode {
public:
    OrbitCameraMode();
    ~OrbitCameraMode() = default;

    void Update(float deltaTime) override;
    Camera* GetCamera() override;
    const Camera* GetCamera() const override;
    void Reset() override;
    Camera::Mode GetModeType() const override { return Camera::Mode::ORBIT; }

    /**
     * Set orbit center point
     * @param center Point to orbit around
     */
    void SetOrbitCenter(const Vector3& center);

    /**
     * Get orbit center
     * @return Center point
     */
    const Vector3& GetOrbitCenter() const { return m_orbitCenter; }

    /**
     * Set orbit distance (radius)
     * @param distance Distance from center
     */
    void SetDistance(float distance);

    /**
     * Get orbit distance
     * @return Distance from center
     */
    float GetDistance() const { return m_distance; }

    /**
     * Set orbit angles
     * @param pitch Vertical angle (radians)
     * @param yaw Horizontal angle (radians)
     */
    void SetOrbitAngles(float pitch, float yaw);

    /**
     * Rotate orbit view
     * @param deltaPitch Vertical rotation change
     * @param deltaYaw Horizontal rotation change
     */
    void RotateOrbit(float deltaPitch, float deltaYaw);

    /**
     * Set orbit speed for automatic rotation
     * @param degreesPerSecond Automatic rotation speed (0 = manual only)
     */
    void SetAutoRotationSpeed(float degreesPerSecond) { m_autoRotationSpeed = degreesPerSecond; }

    /**
     * Enable/disable automatic rotation
     * @param enabled Enable auto-rotation
     */
    void SetAutoRotation(bool enabled) { m_autoRotateEnabled = enabled; }

private:
    std::unique_ptr<Camera> m_camera;
    Vector3 m_orbitCenter = Vector3(0, 0, 0);   // Center of orbit
    float m_distance = 20.0f;                    // Orbit radius
    float m_pitch = -0.785398f;                 // Vertical angle (~-45°)
    float m_yaw = 0.0f;                         // Horizontal angle
    float m_autoRotationSpeed = 0.0f;           // Degrees per second
    bool m_autoRotateEnabled = false;           // Enable auto-rotation
};

// ============================================================================
// Camera Mode Manager
// ============================================================================

/**
 * Manages multiple camera modes and transitions between them
 */
class CameraModeManager {
public:
    CameraModeManager();
    ~CameraModeManager() = default;

    /**
     * Update active camera mode
     * @param deltaTime Elapsed time (seconds)
     */
    void Update(float deltaTime);

    /**
     * Switch to camera mode
     * @param mode Camera mode to activate
     */
    void SetMode(Camera::Mode mode);

    /**
     * Get current active mode
     * @return Current mode
     */
    Camera::Mode GetCurrentMode() const;

    /**
     * Get camera from current mode
     * @return Pointer to camera
     */
    Camera* GetCamera();
    const Camera* GetCamera() const;

    /**
     * Get specific camera mode
     * @param mode Mode type to retrieve
     * @return Pointer to mode object
     */
    CameraMode* GetMode(Camera::Mode mode);
    const CameraMode* GetMode(Camera::Mode mode) const;

    /**
     * Get free camera mode
     * @return Pointer to free camera mode
     */
    FreeCameraMode* GetFreeMode() { return m_freeMode.get(); }

    /**
     * Get RTS camera mode
     * @return Pointer to RTS camera mode
     */
    RTSCameraMode* GetRTSMode() { return m_rtsMode.get(); }

    /**
     * Get chase camera mode
     * @return Pointer to chase camera mode
     */
    ChaseCameraMode* GetChaseMode() { return m_chaseMode.get(); }

    /**
     * Get orbit camera mode
     * @return Pointer to orbit camera mode
     */
    OrbitCameraMode* GetOrbitMode() { return m_orbitMode.get(); }

private:
    std::unique_ptr<FreeCameraMode> m_freeMode;
    std::unique_ptr<RTSCameraMode> m_rtsMode;
    std::unique_ptr<ChaseCameraMode> m_chaseMode;
    std::unique_ptr<OrbitCameraMode> m_orbitMode;
    Camera::Mode m_currentMode = Camera::Mode::FREE;
};
