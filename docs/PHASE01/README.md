# PHASE01: Geometry Rendering

## Phase Title
**Geometry Rendering - Draw First Triangle**

## Objective
Establish foundational graphics rendering by drawing a simple colored triangle to screen using Vulkan/Metal backend. This phase validates that the graphics pipeline is correctly initialized and can output pixels.

## Dependencies
- PHASE00 (Project Planning & Architecture) - must be complete
- PHASE00.5 (Build Targets Configuration) - must be complete
- CMake build system configured and working
- Vulkan/Metal SDK installed and linked
- GLFW or SDL2 window system operational

## Key Deliverables
1. Window initialization with graphics context
2. Vulkan/Metal rendering pipeline setup
3. Render loop that outputs a single colored triangle
4. Frame timing validation (FPS tracking)
5. Basic validation that graphics device is operational

## Acceptance Criteria
- [ ] Window displays on screen
- [ ] Triangle renders in center of window
- [ ] Triangle has correct RGB color (e.g., red)
- [ ] Render loop runs at stable FPS (target 60)
- [ ] No GPU errors or validation warnings
- [ ] Code compiles without warnings on macOS/Linux

## Technical Details

### Components to Implement
1. **Graphics Device Abstraction**
   - Vulkan device initialization (logical/physical)
   - Metal device setup
   - Surface/drawable creation

2. **Vertex Buffer Setup**
   - Create vertex data structure (position, color)
   - Allocate GPU vertex buffer
   - Set up vertex input descriptors

3. **Render Pipeline**
   - Compile vertex shader (simple passthrough)
   - Compile fragment shader (output vertex color)
   - Create graphics pipeline object

4. **Command Recording**
   - Allocate command buffers
   - Record clear/draw commands
   - Submit to GPU queue

### Code Location
```
Core/GameEngineDevice/Source/MetalWrapper/   # Metal implementation
Core/GameEngineDevice/Source/VulkanWrapper/  # Vulkan implementation
GeneralsMD/Code/Graphics/                    # Unified graphics interface
```

### Example Shader (Vertex)
```glsl
#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(position, 1.0);
    fragColor = color;
}
```

### Example Shader (Fragment)
```glsl
#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

### Triangle Vertex Data
```cpp
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

Vertex vertices[] = {
    {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Top - Red
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-left - Green
    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Bottom-right - Blue
};
```

## Estimated Scope
**MEDIUM** (3-5 days for experienced graphics developer)

- GPU initialization: 1-2 days
- Shader setup: 0.5-1 day
- Command recording: 1 day
- Debugging/validation: 1-2 days

## Status
**not-started**

## Testing Strategy
```bash
# Build and run PHASE01
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | grep "Triangle\|PHASE01"
```

## Success Criteria
- Window opens without crash
- Triangle visible and properly colored
- Frame rate stable at 60 FPS
- No validation errors in graphics API

## Reference Documentation
- See `docs/PHASE00/README.md` for architecture decisions
- See `docs/PHASE00_5/README.md` for build target configuration
- See `docs/PHASE00/BUILD_TARGETS.md` for executable naming and build patterns
- See `docs/PHASE00/SETUP_ENVIRONMENT.md` for development environment setup
- Vulkan spec: https://www.khronos.org/registry/vulkan/
- Metal documentation: Apple Developer
