# Metal native backend implementation plan (macOS) — GeneralsX

## 1. Goal

Migrate GeneralsX’s graphics pipeline to use Metal natively on macOS, replacing the OpenGL→Metal translation layer to improve stability, performance, and compatibility with modern drivers.

## 2. Overall strategy

- Keep the DX8Wrapper interface for cross-platform compatibility.
- Implement a Metal abstraction in parallel to OpenGL, enabled by a platform feature macro.
- Provide a fallback to OpenGL/DXVK on Linux/Windows.

## 3. Detailed steps

### Phase 1: Initial structure and abstraction

- Create `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.cpp/h`.
- Define a MetalWrapper interface compatible with DX8Wrapper (init, draw, buffer, shader functions).
- Add a platform feature macro: `#if defined(__APPLE__) && defined(USE_METAL)`.

### Phase 2: Metal context initialization

- Integrate MetalKit/Metal.framework via CMake (add checks similar to the OpenGL block, or in a new `cmake/metal.cmake`).
- Implement device initialization (CreateMetalDevice, CreateMetalLayer) using MTLDevice, MTLCommandQueue, and CAMetalLayer.
- Adapt the SDL2 window path to attach a CAMetalLayer for presentation.

### Phase 3: Buffers and shaders

- Implement vertex/index buffer abstractions using `MTLBuffer`.
- Port shaders from GLSL to Metal Shading Language (MSL).
- Add a loader for `.metal` shader sources under `resources/shaders/`.

### Phase 4: Rendering and render states

- Implement the Metal render pipeline (`MTLRenderPipelineState`).
- Map render states (blend, depth, stencil) to Metal equivalents.
- Adapt draw calls (`DrawPrimitives`, `DrawIndexedPrimitives`) to Metal.

### Phase 5: DX8Wrapper integration

- Modify `dx8wrapper.cpp/h` to delegate calls to MetalWrapper on macOS when enabled.
- Preserve the OpenGL path (or DXVK on other platforms) as fallback.

### Phase 6: Testing and validation

- Create basic tests to validate Metal initialization and a first frame (clear/triangle).
- Validate shaders, buffers, and draw calls up to the opening screen.
- Add detailed logs for Metal error diagnostics.

### Phase 7: Documentation updates

- Update `docs/MACOS_PORT_DIARY.md` and `docs/NEXT_STEPS.md`; optionally create `docs/PHASE28/METAL_IMPLEMENTATION.md` with implementation details.
- Document functional differences and limitations between Metal and OpenGL.

## 4. Final considerations

- Keep the DX8Wrapper interface to preserve portability.
- Prioritize stability and performance on macOS.
- Plan DXVK/OpenGL backend selection for Linux/Windows in future phases.
