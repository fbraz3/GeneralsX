# VULKAN_INTEGRATION_SPEC.md

**Phase 09.2: Vulkan Research & Technical Specification**  
**Date**: January 16, 2026  
**Status**: Research & Planning Complete  
**Target**: VC6 32-bit Windows native (Phase 1), Wine cross-platform (Phase 3)

---

## Executive Summary

This technical specification defines the strategy for replacing DirectX 8 with Vulkan in GeneralsX. The plan balances:
- **Immediate goals**: Get Vulkan rendering on Windows VC6 32-bit
- **Platform support**: Wine/Proton compatibility for Linux/macOS
- **Code quality**: Minimal duplication, clean abstraction patterns
- **Development velocity**: Incremental implementation with early validation

Key insight: Vulkan's explicit API model maps well to DirectX 8's render state machine, making translation straightforward.

---

## 1. Vulkan Fundamentals & Architecture

### 1.1 Vulkan vs DirectX 8

| Aspect | DirectX 8 | Vulkan |
|--------|-----------|--------|
| **CPU Overhead** | High (driver does validation) | Low (explicit control) |
| **Multi-threading** | Limited (single device thread) | Full (command buffers thread-safe) |
| **State Validation** | Driver handles | Application responsibility |
| **Memory Management** | Driver hidden | Explicit allocation |
| **Pipeline States** | Loose (state machine) | Fixed (immutable pipelines) |
| **Shader Format** | DirectX bytecode | SPIR-V (intermediate) |
| **Cross-Platform** | Windows-only | All platforms |

**For GeneralsX**: Vulkan's lower CPU overhead benefits RTS gameplay with many draw calls. Explicit API matches the legacy code's direct render state manipulation.

### 1.2 Vulkan Core Objects

```
Instance
  └── PhysicalDevice (GPU)
      └── LogicalDevice (VkDevice)
          ├── Queues (graphics, transfer, compute)
          ├── CommandPool
          │   └── CommandBuffer
          ├── Memory
          │   ├── Buffers (vertex, index, uniform)
          │   └── Images (textures, render targets)
          ├── DescriptorPool
          │   └── DescriptorSet (texture/buffer bindings)
          ├── PipelineLayout
          │   └── Pipeline (complete render state)
          └── RenderPass
              └── Framebuffer
```

**For GeneralsX**: Instance = graphics subsystem, LogicalDevice = GraphicsDevice implementation.

### 1.3 Vulkan Rendering Pipeline (Simplified)

```
1. Record Command Buffer (CPU)
   - vkCmdBeginRenderPass()
   - vkCmdBindPipeline()
   - vkCmdBindDescriptorSets()
   - vkCmdDrawIndexed()
   - vkCmdEndRenderPass()
   
2. Submit to GPU Queue
   - vkQueueSubmit()
   
3. Present to Screen
   - vkQueuePresentKHR()
```

**DirectX 8 Equivalent**: Set state → Draw → Swap. Vulkan is more explicit but structurally similar.

---

## 2. DirectX 8 to Vulkan Mapping

### 2.1 Render State Mapping

| DirectX 8 | Vulkan | Implementation |
|-----------|--------|-----------------|
| SetRenderState(D3DRS_ZENABLE) | VkPipelineDepthStencilStateCreateInfo::depthTestEnable | Pipeline state |
| SetRenderState(D3DRS_CULLMODE) | VkPipelineRasterizationStateCreateInfo::cullMode | Pipeline state |
| SetRenderState(D3DRS_ALPHABLENDENABLE) | VkPipelineColorBlendStateCreateInfo | Pipeline state |
| SetTextureStageState(sampler) | VkDescriptorSet | Descriptor binding |
| DrawPrimitive() | vkCmdDrawIndexed() | Command buffer |

### 2.2 Texture Format Mapping

| DirectX 8 Format | DDS Format | Vulkan Format |
|------------------|-----------|---------------|
| D3DFMT_DXT1 | BC1 | VK_FORMAT_BC1_RGB_UNORM_BLOCK |
| D3DFMT_DXT3 | BC2 | VK_FORMAT_BC2_UNORM_BLOCK |
| D3DFMT_DXT5 | BC3 | VK_FORMAT_BC3_UNORM_BLOCK |
| D3DFMT_A8R8G8B8 | RGBA | VK_FORMAT_B8G8R8A8_UNORM |
| D3DFMT_D24S8 | D24S8 | VK_FORMAT_D24_UNORM_S8_UINT |

**For GeneralsX**: Game primarily uses DXT1/DXT3/DXT5. All convertible to DDS format (already present).

### 2.3 Shader Translation

| DirectX 8 Shader | Vulkan Equivalent |
|------------------|-------------------|
| HLSL pixel shader | GLSL fragment shader |
| HLSL vertex shader | GLSL vertex shader |
| DirectX bytecode | SPIR-V (compiled from GLSL) |

**Tools**:
- `glslc` (Google Shading Language Compiler): Compiles GLSL → SPIR-V
- Integration into CMake build system automatic

---

## 3. Implementation Architecture

### 3.1 VulkanDevice Class Hierarchy

```cpp
// Abstract base (from Phase 09.1)
class GraphicsDevice { virtual ~GraphicsDevice(); /* 70+ methods */ };

// Vulkan implementation (Phase 09.3)
class GraphicsDeviceVulkan : public GraphicsDevice {
    // Core Vulkan objects
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    
    // Swapchain for presentation
    VkSwapchainKHR swapchain;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> framebuffers;
    
    // Render pass (defines render target layout)
    VkRenderPass renderPass;
    
    // Command buffer recording
    VkCommandBuffer currentCommandBuffer;
    
    // Synchronization
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    
    // Resource tracking
    std::unordered_map<int, VulkanBuffer*> buffers;
    std::unordered_map<int, VulkanTexture*> textures;
    std::unordered_map<int, VulkanPipeline*> pipelines;
};
```

### 3.2 Supporting Classes

#### VulkanBuffer (Vertex/Index/Uniform)
```cpp
struct VulkanBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    size_t size;
    int stride;  // For vertex buffers
};
```

#### VulkanTexture (2D/RenderTarget/Depth)
```cpp
struct VulkanTexture {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
    VkSampler sampler;
    VkFormat format;
    uint32_t width, height;
    bool isRenderTarget;
};
```

#### VulkanPipeline (Graphics/Compute)
```cpp
struct VulkanPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkShaderModule> shaderModules;
};
```

### 3.3 Memory Management Strategy

**Per-Type Allocation Pools** (similar to AudioDevice pattern):

```cpp
// Vertex buffer pool (frequently created/destroyed)
VulkanMemoryPool vertexMemoryPool;

// Texture memory pool (large allocations, persistent)
VulkanMemoryPool textureMemoryPool;

// Uniform buffer pool (frequent updates)
VulkanMemoryPool uniformMemoryPool;
```

Benefits:
- Reduces fragmentation
- Faster allocation for common patterns
- Better cache locality
- Easier profiling and debugging

---

## 4. Vulkan Initialization Sequence

### 4.1 GraphicsDeviceVulkan::init() Flow

```cpp
bool GraphicsDeviceVulkan::init(void* windowHandle, int width, int height) {
    // 1. Create Vulkan Instance
    // 2. Select Physical Device (GPU)
    // 3. Create Logical Device
    // 4. Get Graphics Queue
    // 5. Create Command Pool
    // 6. Create Swapchain (via SDL2 surface)
    // 7. Create Render Pass
    // 8. Create Framebuffers
    // 9. Create Synchronization Objects
    // 10. Create Default Descriptor Pool
    return true;
}
```

### 4.2 SDL2 Surface Integration

Vulkan requires native window surface. SDL2 provides cross-platform surface creation:

```cpp
// Get SDL2 window from system
SDL_Window* window = (SDL_Window*)windowHandle;

// Get vulkan instance extensions needed for surface
unsigned int extensionCount = 0;
const char** extensionNames = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

// Create surface with platform-specific code (wrapped in abstraction)
VkSurfaceKHR surface;
SDL_Vulkan_CreateSurface(window, instance, &surface);  // SDL2 10.1+
```

This enables cross-platform Vulkan rendering on Windows/macOS/Linux.

---

## 5. Frame Rendering Cycle

### 5.1 Per-Frame Execution (GraphicsDeviceVulkan)

```cpp
bool GraphicsDeviceVulkan::beginFrame() {
    // 1. Wait for previous frame to complete
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFence);
    
    // 2. Acquire next swapchain image
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, 
                         imageAvailableSemaphore, VK_NULL_HANDLE, 
                         &imageIndex);
    
    // 3. Record command buffer for this image
    currentCommandBuffer = commandBuffers[imageIndex];
    vkResetCommandBuffer(currentCommandBuffer, 0);
    
    VkCommandBufferBeginInfo beginInfo{...};
    vkBeginCommandBuffer(currentCommandBuffer, &beginInfo);
    
    // 4. Begin render pass
    VkRenderPassBeginInfo renderPassInfo{...};
    vkCmdBeginRenderPass(currentCommandBuffer, &renderPassInfo, 
                         VK_SUBPASS_CONTENTS_INLINE);
    
    return true;
}

bool GraphicsDeviceVulkan::endFrame() {
    // 1. End render pass
    vkCmdEndRenderPass(currentCommandBuffer);
    
    // 2. End command buffer recording
    vkEndCommandBuffer(currentCommandBuffer);
    
    // 3. Submit to GPU
    VkSubmitInfo submitInfo{...};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCommandBuffer;
    
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
    
    // 4. Present to screen
    VkPresentInfoKHR presentInfo{...};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    
    vkQueuePresentKHR(graphicsQueue, &presentInfo);
    return true;
}
```

### 5.2 Game Loop Integration

```cpp
// In main game loop (Generals/GeneralsMD)
while (gameRunning) {
    // Game update
    TheGameLogic->update(deltaTime);
    
    // Graphics frame
    if (!TheGraphicsDevice->beginFrame()) break;
    
    // Render game (3D models, terrain, UI, etc.)
    TheGameClient->render();
    
    // Complete frame and present
    if (!TheGraphicsDevice->endFrame()) break;
    
    // Audio update
    TheAudioDevice->update(deltaTime);
}
```

---

## 6. Shader Compilation Pipeline

### 6.1 Build-Time GLSL → SPIR-V Compilation

**Tools**:
- `glslc`: Google Shading Language Compiler (part of Vulkan SDK)
- Compiles GLSL source → SPIR-V binary

**CMake Integration**:

```cmake
# Find glslc
find_program(GLSLC_EXECUTABLE glslc REQUIRED)

# Custom command to compile shaders
add_custom_command(
    OUTPUT ${SPIRV_FILE}
    COMMAND ${GLSLC_EXECUTABLE} -o ${SPIRV_FILE} ${GLSL_FILE}
    DEPENDS ${GLSL_FILE}
    COMMENT "Compiling GLSL shader ${GLSL_FILE}"
)

# Create shader library
add_library(game_shaders OBJECT ${SPIRV_FILES})
```

### 6.2 Shader Files Structure

```
shaders/
├── basic.vert              # Basic vertex shader
├── basic.frag              # Basic fragment shader
├── terrain.vert            # Terrain shader
├── terrain.frag
├── particle.vert
├── particle.frag
├── ui.vert
├── ui.frag
└── CMakeLists.txt          # Shader compilation rules
```

### 6.3 Runtime Shader Loading

```cpp
// VulkanPipeline::createShaderModule()
VkShaderModule createShaderModule(const std::vector<uint32_t>& spirvCode) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
    createInfo.pCode = spirvCode.data();
    
    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    return shaderModule;
}
```

---

## 7. Wine/Proton Compatibility Strategy

### 7.1 Vulkan on Wine

Wine can run Vulkan applications in two ways:

1. **Native Vulkan (Linux/macOS)**
   - Native Vulkan driver renders to native display
   - Performance: Good
   - Compatibility: Excellent (pure Vulkan)

2. **DXVK Layer (Alternative)**
   - DirectX 8 → Vulkan translation
   - Redundant with our GraphicsDevice abstraction
   - Not needed since we're already translating to Vulkan

### 7.2 WINEPREFIX Configuration

For testing GeneralsX on Wine:

```bash
# Create 32-bit Wine prefix
WINEPREFIX=~/.wine32 WINEARCH=win32 winecfg

# Install Vulkan support
WINEPREFIX=~/.wine32 winetricks vulkan

# Run game
WINEPREFIX=~/.wine32 wine GeneralsXZH.exe -win
```

### 7.3 Cross-Platform Testing

```
+--------+---------+--------+--------+
| Target | Primary | Fallback | Status |
+--------+---------+--------+--------+
| Windows| Vulkan  | DX8    | Phase 1|
| Wine   | Vulkan  | N/A    | Phase 3|
| macOS  | Vulkan  | N/A    | Phase 3|
| Linux  | Vulkan  | N/A    | Phase 3|
+--------+---------+--------+--------+
```

No DirectX 8 fallback needed on Wine (it has its own DX8 layer).

---

## 8. Development Timeline & Milestones

### Phase 09.2: Research Complete ✅
- [x] Vulkan fundamentals understood
- [x] DirectX 8 mapping strategy defined
- [x] Memory management approach chosen
- [x] Wine compatibility analyzed
- [x] Shader compilation pipeline planned

### Phase 09.3: Implementation (5-7 days)
- [ ] VulkanDevice core structure
- [ ] Memory allocation and pooling
- [ ] Buffer/texture/shader management
- [ ] Swapchain and render pass setup
- [ ] Frame rendering cycle
- [ ] Command buffer recording

### Phase 09.4-09.8: Integration & Testing
- [ ] Hook game rendering calls
- [ ] Asset audit and conversion
- [ ] Shader development
- [ ] Build and test
- [ ] Documentation

---

## 9. Known Limitations & Workarounds

### 9.1 VC6 Compiler Limitations

**Issue**: VC6 (1998-era) doesn't support modern C++ (no STL, templates limited)

**Workaround**:
- Use manual memory management (new/delete)
- Use arrays instead of std::vector where needed
- Vulkan C API is C-compatible, no C++ features required
- STL containers might work for internal bookkeeping

### 9.2 Vulkan API Complexity

**Issue**: Vulkan has ~400 functions vs DirectX 8's ~100

**Workaround**:
- Create helper functions to abstract complexity
- Vulkan wrapper classes (VulkanBuffer, VulkanTexture)
- Focus on essential features first (not all extensions)

### 9.3 SPIR-V Intermediate Format

**Issue**: Can't debug SPIR-V directly, need source maps

**Workaround**:
- Develop and debug GLSL shaders first
- Keep original GLSL source alongside SPIR-V
- Use glslang debug extensions in builds

### 9.4 Swapchain Complexity

**Issue**: Swapchain recreation on window resize can be fragile

**Workaround**:
- Implement `handleWindowResize()` carefully
- Test with multiple monitor configurations
- Support fullscreen/windowed transitions

---

## 10. Testing & Validation Strategy

### 10.1 Unit Tests (Per Component)

```cpp
// VulkanBuffer creation
TEST(VulkanBuffer, CreateVertexBuffer) {
    GraphicsDeviceVulkan device;
    device.init(window, 1024, 768);
    
    float vertices[12] = {...};
    int handle = device.createVertexBuffer(vertices, 12 * sizeof(float), 12);
    
    EXPECT_NE(handle, INVALID_GRAPHICS_HANDLE);
    device.destroyBuffer(handle);
}

// Texture loading
TEST(VulkanTexture, LoadDXTTexture) {
    // Load DXT1 compressed texture
    // Verify GPU memory allocation
    // Verify texture binding works
}

// Shader compilation
TEST(VulkanShader, CompileShaderProgram) {
    // Compile simple vertex/fragment pair
    // Verify pipeline creation
    // Verify shader binding works
}
```

### 10.2 Integration Tests

```cpp
// Render simple triangle
// Verify pixel colors match expected
// Test all render states (depth, blend, culling)
// Verify texture binding and filtering
```

### 10.3 Game Tests

```cpp
// Load actual game level
// Verify terrain renders
// Verify units render with textures
// Check particle effects
// Validate UI rendering
// Performance metrics (FPS, memory)
```

---

## 11. Performance Considerations

### 11.1 Expected Performance

**Target Metrics**:
- Main menu: ≥ 60 FPS
- Skirmish game: ≥ 30 FPS
- GPU memory: < 256 MB (DirectX 8 baseline)

**Vulkan Advantages**:
- Lower CPU overhead (fewer driver calls)
- Better multithreading (parallelizable)
- Explicit memory management (predictable allocation)

### 11.2 Optimization Opportunities (Phase 2+)

- **Command buffer recording reuse**: Record once, replay
- **Descriptor set pooling**: Reduce descriptor allocation overhead
- **Pipeline caching**: Save compiled pipelines to disk
- **Texture compression**: DXT formats reduce bandwidth
- **Instancing**: Render multiple objects in single draw call

---

## 12. References & Documentation

### Vulkan Specifications
- Vulkan 1.2 Specification (latest stable)
- Khronos Vulkan SDK documentation
- Local copy: `docs/Vulkan/` (to be downloaded)

### Example Implementations
- Vulkan Samples (github.com/KhronosGroup/Vulkan-Samples)
- DXVK (github.com/doitsujin/dxvk) - DirectX to Vulkan
- Wine Vulkan support (wine.readthedocs.io)

### Reference Repositories
- `references/fighter19-dxvk-port/` - Linux port with Vulkan/DXVK
- `references/jmarshall-win64-modern/` - Windows 64-bit modernization

---

## 13. Success Criteria (Phase 09.2)

- [x] Vulkan fundamentals documented
- [x] DirectX 8 to Vulkan mapping complete
- [x] Implementation architecture defined
- [x] Memory management strategy chosen
- [x] Shader compilation pipeline planned
- [x] Wine compatibility analyzed
- [x] Development plan created
- [x] Testing strategy documented

**Status**: ✅ Complete - Ready for Phase 09.3 Implementation

---

**Prepared by**: GeneralsX Development Team  
**Date**: January 16, 2026  
**Next Phase**: 09.3 - VulkanDevice Core Implementation
