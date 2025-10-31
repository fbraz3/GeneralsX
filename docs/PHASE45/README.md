# Phase 45: Camera System & View Transformation

## Overview

Phase 45 implements the camera system that controls player viewpoint and applies view/projection transformations to all rendered geometry. This includes camera positioning, rotation, field of view, near/far clipping, and perspective calculations.

**Status**: Planning/Ready to Implement  
**Depends on**: Phase 40 (DXVK), Phase 43 (Render Loop), Phase 44 (Model Loading)  
**Estimated Duration**: 2-3 days  
**Complexity**: Medium-High (matrix math, transformations)

## Objectives

### Primary Goals

1. **Implement Camera System**
   - Position and rotation management
   - Look-at matrix calculation
   - Perspective matrix setup

2. **Implement View Transformations**
   - View matrix (world to camera space)
   - Projection matrix (camera to clip space)
   - Combined transformation pipeline

3. **Implement Camera Controls**
   - Free camera movement (for testing)
   - RTS camera (isometric view)
   - Chase camera (follow unit)

4. **Integrate with Render Pipeline**
   - Pass matrices to vertex shaders
   - Handle matrix updates per frame
   - Support camera animations

## Architecture

### Camera Components

```cpp
class Camera {
    // Position and orientation
    Vector3 position;      // World space camera position
    Vector3 forward;       // Camera look direction
    Vector3 right;         // Camera right vector
    Vector3 up;            // Camera up vector
    
    // Projection parameters
    float fov;             // Field of view (degrees)
    float aspect_ratio;    // Width / height
    float near_plane;      // Near clipping distance
    float far_plane;       // Far clipping distance
    
    // Matrices
    Matrix4x4 view_matrix;
    Matrix4x4 projection_matrix;
    
    // Methods
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
    const Matrix4x4& GetViewMatrix() const;
    const Matrix4x4& GetProjectionMatrix() const;
};
```

### Coordinate System

```
World Space (game world)
    ↓ (View Matrix)
Camera Space (camera as origin)
    ↓ (Projection Matrix)
Clip Space (-1 to +1 normalized)
    ↓ (Viewport Transform)
Screen Space (pixel coordinates)
```

### Matrix Mathematics

```
View Matrix = LookAt(position, position + forward, up)
            = [right.x    right.y    right.z    -dot(right, pos)   ]
              [up.x       up.y       up.z       -dot(up, pos)       ]
              [-forward.x -forward.y -forward.z dot(forward, pos)   ]
              [0          0          0          1                    ]

Projection Matrix = Perspective(fov, aspect, near, far)
                  = [cot(fov/2)/aspect  0              0                 0                    ]
                    [0                   cot(fov/2)     0                 0                    ]
                    [0                   0              -(far+near)/(far-near)  -2*far*near/(far-near) ]
                    [0                   0              -1                0                    ]

Combined Transform = Projection * View * World
```

## Implementation Strategy

### Stage 1: Camera Class (Day 1)

1. Create Camera class with position/orientation
2. Implement view matrix calculation
3. Implement projection matrix calculation
4. Add accessor methods

### Stage 2: Euler Angles & Quaternions (Day 1)

1. Implement rotation using Euler angles
2. Convert to quaternions if needed
3. Support camera pitch, yaw, roll
4. Handle gimbal lock avoidance

### Stage 3: Camera Modes (Day 1-2)

1. Implement free camera (all directions)
2. Implement RTS camera (fixed angle, panning)
3. Implement chase camera (follow target)
4. Smooth transitions between modes

### Stage 4: Integration with Rendering (Day 2)

1. Pass view/projection matrices to shaders
2. Update matrices in uniform buffers
3. Apply transformations in vertex shader
4. Test with rendered models

### Stage 5: Controls & Animation (Day 2-3)

1. Implement keyboard/mouse camera control
2. Add camera animation framework
3. Support zoom and orbit
4. Performance optimization

## Success Criteria

### Functional

- ✅ Camera positions correctly in world space
- ✅ View matrix transforms vertices to camera space
- ✅ Projection matrix applies perspective correctly
- ✅ Models render at correct screen positions
- ✅ Camera movement is smooth and responsive
- ✅ Near/far clipping works correctly

### Technical

- ✅ Compilation without errors
- ✅ Matrix calculations verified mathematically
- ✅ Matrices correctly passed to GPU
- ✅ No gimbal lock issues
- ✅ Proper floating-point precision

### Performance

- ✅ Camera updates < 0.1ms per frame
- ✅ Matrix calculations accurate to float precision
- ✅ No unnecessary recalculations

## Coordinate System Conventions

### Axes

- X: Right (positive right)
- Y: Up (positive up)
- Z: Forward (positive away from camera, left-handed)

### Angles

- Pitch: Rotation around X axis (up/down)
- Yaw: Rotation around Y axis (left/right)
- Roll: Rotation around Z axis (camera tilt)

## Camera Matrices Implementation

```cpp
Matrix4x4 Camera::GetViewMatrix() const {
    // Create orthonormal basis from forward vector
    Vector3 x_axis = Cross(forward, up).Normalize();
    Vector3 y_axis = Cross(x_axis, forward).Normalize();
    Vector3 z_axis = -forward;
    
    // Build view matrix (world to camera)
    return Matrix4x4(
        x_axis.x,      y_axis.x,      z_axis.x,      0,
        x_axis.y,      y_axis.y,      z_axis.y,      0,
        x_axis.z,      y_axis.z,      z_axis.z,      0,
        -Dot(x_axis, position),
        -Dot(y_axis, position),
        -Dot(z_axis, position),
        1
    );
}

Matrix4x4 Camera::GetProjectionMatrix() const {
    float tan_half_fov = tan(fov / 2.0f);
    
    float f = 1.0f / tan_half_fov;
    float a = f / aspect_ratio;
    
    float z_far_plus_near = far_plane + near_plane;
    float z_far_minus_near = far_plane - near_plane;
    
    return Matrix4x4(
        a,  0,  0,  0,
        0,  f,  0,  0,
        0,  0,  -z_far_plus_near/z_far_minus_near,  -1,
        0,  0,  -2*far_plane*near_plane/z_far_minus_near,  0
    );
}
```

## Camera Modes

### Free Camera (Testing)

```cpp
void FreeCameraUpdate(float dt) {
    // Movement: WASD keys
    // Rotation: Mouse drag
    // Speed: Adjustable
}
```

### RTS Camera (Isometric)

```cpp
void RTSCameraUpdate(float dt) {
    // Fixed viewing angle (45 degrees)
    // Pan: WASD or arrow keys
    // Zoom: Mouse wheel
    // Rotation: Limited (0-360 degrees)
}
```

### Chase Camera (Following Unit)

```cpp
void ChaseCameraUpdate(float dt) {
    // Follow target position with offset
    // Smooth damping
    // Collision avoidance
}
```

## Uniform Buffer Layout

```cpp
struct CameraUniforms {
    glm::mat4 view;          // offset 0, 64 bytes
    glm::mat4 projection;    // offset 64, 64 bytes
    glm::vec3 camera_pos;    // offset 128, 12 bytes
    float _padding1;         // offset 140, 4 bytes
    glm::vec3 camera_dir;    // offset 144, 12 bytes
    float _padding2;         // offset 156, 4 bytes
    // Total: 160 bytes
};
```

## Vertex Shader Integration

```glsl
#version 450

layout(set = 0, binding = 0) uniform CameraBlock {
    mat4 view;
    mat4 projection;
    vec3 camera_pos;
    vec3 camera_dir;
} camera;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

layout(location = 0) out vec3 vs_normal;
layout(location = 1) out vec2 vs_texcoord;
layout(location = 2) out vec3 vs_world_pos;

void main() {
    // Transform to world space (using identity for now)
    vec4 world_pos = vec4(position, 1.0);
    vs_world_pos = world_pos.xyz;
    
    // Transform to clip space
    gl_Position = camera.projection * camera.view * world_pos;
    
    // Pass data to fragment shader
    vs_normal = normal;
    vs_texcoord = texcoord;
}
```

## Integration Points

### With Phase 43 (Render Loop)

- Camera matrices updated before BeginScene
- Matrices passed to GPU before drawing

### With Phase 44 (Model Loading)

- World transform combined with camera matrices
- Object positioning in world space

### With Phase 46 (Game Logic)

- Camera follows game objects
- Game updates camera target position

## Testing Strategy

### Unit Tests

```cpp
// tests/test_camera.cpp
- Test matrix calculations
- Test camera movement
- Test view/projection transforms
- Test coordinate space conversions
```

### Integration Tests

```bash
# Render simple scene with camera
./GeneralsXZH --camera-test
# Verify: Models render with correct perspective
```

### Visual Tests

1. Create simple cube in world space
2. Verify perspective: far edge smaller than near edge
3. Test camera rotation: geometry rotates correctly
4. Test camera panning: world space coordinates correct

## Clipping Plane Values

```
Default RTS Camera:
- Near plane: 1.0 (units)
- Far plane: 1000.0 (units)
- Field of view: 60 degrees
- Aspect ratio: 1920/1080 = 1.777
```

## Next Phases

**Phase 46**: Game Logic Integration - Connect camera to game objects  
**Phase 47**: Testing & Stabilization - Comprehensive testing

## References

- `docs/PHASE43/` - Render loop integration
- `docs/PHASE44/` - Model positioning in world space
- Vulkan Specification Chapter 26 (Shaders)
- 3D Graphics Mathematics (matrix transformations)
- Game Engine Architecture (camera systems)
