# Phase 43: Render Loop & Frame Presentation

## Overview

Phase 43 implements the complete render loop that ties together device initialization, drawing operations, and presentation. This includes BeginScene/EndScene lifecycle, command buffer recording, GPU submission, and frame presentation.

**Status**: Planning/Ready to Implement  
**Depends on**: Phase 40 (DXVK Backend), Phase 41 (Drawing), Phase 42 (Textures)  
**Estimated Duration**: 2-3 days  
**Complexity**: High (GPU synchronization, timing)

## Objectives

### Primary Goals

1. **Implement Scene Rendering Cycle**
   - `BeginScene()` - Begin frame recording
   - Record draw commands to command buffer
   - `EndScene()` - Complete recording
   - `Present()` - Submit and present to screen

2. **Implement Frame Synchronization**
   - Image acquisition from swapchain
   - GPU-CPU synchronization with fences
   - Frame rate limiting and timing

3. **Implement Command Recording**
   - Record render pass commands
   - Transition image layouts
   - Handle swapchain recreation

4. **Implement Performance Monitoring**
   - Frame time tracking
   - GPU timing queries
   - Profiling hooks

## Architecture

### Render Loop Flow

```
Frame Start
├── BeginScene()
│   ├── Acquire swapchain image
│   ├── Wait for in-flight fence
│   ├── Begin command buffer recording
│   └── Begin render pass
├── Draw Commands (from game logic)
│   └── DrawPrimitive/DrawIndexedPrimitive
├── EndScene()
│   ├── End render pass
│   ├── End command buffer recording
│   └── Submit to GPU queue
├── Present()
│   ├── Wait for render finished semaphore
│   └── Submit present request
└── Frame Timing
    └── Update FPS counter
```

### Key Vulkan Operations

```cpp
vkWaitForFences()           // Wait for GPU work completion
vkResetFences()             // Reset fences for next frame
vkAcquireNextImageKHR()     // Get swapchain image
vkResetCommandBuffer()      // Prepare for new recording
vkBeginCommandBuffer()      // Start recording
vkCmdBeginRenderPass()      // Begin render pass
// ... draw commands ...
vkCmdEndRenderPass()        // End render pass
vkEndCommandBuffer()        // Complete recording
vkQueueSubmit()             // Submit to GPU
vkQueuePresentKHR()         // Present to screen
```

## Implementation Strategy

### Stage 1: Basic Render Loop (Day 1)

1. Implement `BeginScene()` with image acquisition
2. Implement `EndScene()` with basic submission
3. Implement `Present()` with swapchain presentation
4. Test with simple frame rendering

### Stage 2: Frame Synchronization (Day 1)

1. Implement fence/semaphore synchronization
2. Handle wait-before-overwrite pattern
3. Implement frame pacing (60 FPS limiting)

### Stage 3: Command Recording (Day 1-2)

1. Implement render pass command recording
2. Handle dynamic state (viewport, scissors)
3. Implement clear color and depth buffer

### Stage 4: Error Handling & Robustness (Day 2)

1. Handle swapchain out-of-date (window resize)
2. Implement swapchain recreation
3. Error recovery and logging

### Stage 5: Performance Monitoring (Day 2-3)

1. Implement frame time tracking
2. Add GPU timestamp queries
3. Generate performance reports

## Success Criteria

### Functional

- ✅ BeginScene() successfully acquires swapchain image
- ✅ EndScene() submits commands without errors
- ✅ Present() displays rendered frame on screen
- ✅ Frame synchronization works correctly (no tearing)
- ✅ Window resize triggers swapchain recreation
- ✅ 60 FPS frame pacing maintained

### Technical

- ✅ All operations compile without errors
- ✅ Vulkan validation layers pass (zero errors/warnings)
- ✅ No deadlocks or synchronization issues
- ✅ Frame time consistently < 16.6ms (60 FPS)
- ✅ GPU memory properly managed

## Integration Points

### With Phase 41 & 42
- Draw commands recorded between BeginScene/EndScene
- Render pass compatible with texture binding

### With Game Engine
- Game calls BeginScene before draw commands
- Game calls Present to display frame

### With Phase 45 (Camera)
- Camera matrices set before BeginScene
- Viewport set in BeginScene

## Testing Strategy

### Unit Tests

```cpp
// tests/test_render_loop.cpp
- Test BeginScene/EndScene cycle
- Test synchronization primitives
- Test frame timing
- Test swapchain recreation
```

### Integration Tests

```bash
# Run game with render loop test
./GeneralsXZH --render-loop-test
# Verify: Frames render at 60 FPS without tearing
```

### Performance Tests

```bash
# Monitor frame times and GPU utilization
./GeneralsXZH --perf-monitor
```

## Frame Time Budget (60 FPS target)

```
Total: 16.67 ms
├── Acquire image: 0.1 ms
├── Record commands: 2.0 ms
├── GPU rendering: 10.0 ms
├── Present: 0.5 ms
└── CPU idle: 4.0 ms
```

## Next Phase

**Phase 44**: Model Loading - Load and render 3D models  
**Parallel**: Phase 45 - Camera System

## References

- Vulkan Presentation: Vulkan Specification Chapter 29
- Vulkan Synchronization: Vulkan Specification Chapter 7
- Frame Pacing: GDC talks on game loop design
- `docs/PHASE39/` - Backend architecture
- `docs/PHASE40/` - Vulkan initialization
