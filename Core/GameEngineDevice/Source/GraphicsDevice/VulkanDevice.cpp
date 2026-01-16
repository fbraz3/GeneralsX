/*
 * Phase 09.3: VulkanDevice Implementation
 *
 * Core graphics rendering backend using Vulkan API.
 * Implements all GraphicsDevice interface methods.
 */

#include "GraphicsDevice/VulkanDevice.h"
#include "GraphicsDevice/VulkanBuffer.h"
#include "GraphicsDevice/VulkanTexture.h"
#include "GraphicsDevice/VulkanPipeline.h"
#include "GraphicsDevice/VulkanSwapchain.h"
#include "GraphicsDevice/VulkanRenderPass.h"
#include "GraphicsDevice/VulkanMemoryAllocator.h"
#include <cstring>
#include <cstdio>

namespace GeneralsX {
namespace Graphics {

// ============================================================
// Constructor & Destructor
// ============================================================

GraphicsDeviceVulkan::GraphicsDeviceVulkan()
    : instance(VK_NULL_HANDLE),
      physicalDevice(VK_NULL_HANDLE),
      device(VK_NULL_HANDLE),
      graphicsQueue(VK_NULL_HANDLE),
      graphicsQueueFamily(0),
      commandPool(VK_NULL_HANDLE),
      currentCommandBuffer(VK_NULL_HANDLE),
      commandBuffers(nullptr),
      commandBufferCount(0),
      swapchain(nullptr),
      surface(VK_NULL_HANDLE),
      renderPass(nullptr),
      framebuffers(nullptr),
      framebufferCount(0),
      imageAvailableSemaphores(nullptr),
      renderFinishedSemaphores(nullptr),
      inFlightFences(nullptr),
      synchronizationObjectCount(0),
      currentFrameIndex(0),
      memoryAllocator(nullptr),
      nextBufferHandle(0),
      nextTextureHandle(0),
      nextPipelineHandle(0),
      isInitialized_(false),
      isFrameRecording(false),
      windowWidth(0),
      windowHeight(0),
      deltaTime(0.0f)
{
    // Initialize resource arrays
    for (int i = 0; i < MAX_BUFFERS; ++i) {
        buffers[i] = nullptr;
    }
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        textures[i] = nullptr;
    }
    for (int i = 0; i < MAX_PIPELINES; ++i) {
        pipelines[i] = nullptr;
    }

    // Initialize error message
    lastErrorMessage[0] = '\0';
}

GraphicsDeviceVulkan::~GraphicsDeviceVulkan()
{
    if (isInitialized_) {
        shutdown();
    }
}

// ============================================================
// Lifecycle Methods
// ============================================================

bool GraphicsDeviceVulkan::init(void* windowHandle, int width, int height)
{
    if (isInitialized_) {
        setError("GraphicsDeviceVulkan already initialized");
        return false;
    }

    windowWidth = width;
    windowHeight = height;

    // Step 1: Create Vulkan instance
    if (!createInstance()) {
        return false;
    }

    // Step 2: Create surface from SDL2 window
    if (!createSwapchain(windowHandle)) {
        destroyInstance();
        return false;
    }

    // Step 3: Select physical device
    if (!selectPhysicalDevice()) {
        destroySwapchain();
        destroyInstance();
        return false;
    }

    // Step 4: Create logical device
    if (!createLogicalDevice()) {
        destroySwapchain();
        destroyInstance();
        return false;
    }

    // Step 5: Create command pool and buffers
    if (!createCommandPool()) {
        destroyLogicalDevice();
        destroySwapchain();
        destroyInstance();
        return false;
    }

    // Step 6: Create render pass
    if (!createRenderPass()) {
        destroyCommandPool();
        destroyLogicalDevice();
        destroySwapchain();
        destroyInstance();
        return false;
    }

    // Step 7: Create framebuffers
    if (!createFramebuffers()) {
        destroyRenderPass();
        destroyCommandPool();
        destroyLogicalDevice();
        destroySwapchain();
        destroyInstance();
        return false;
    }

    // Step 8: Create synchronization objects
    if (!createSynchronizationObjects()) {
        destroyFramebuffers();
        destroyRenderPass();
        destroyCommandPool();
        destroyLogicalDevice();
        destroySwapchain();
        destroyInstance();
        return false;
    }

    // Step 9: Create memory allocator
    if (!createMemoryAllocator()) {
        destroySynchronizationObjects();
        destroyFramebuffers();
        destroyRenderPass();
        destroyCommandPool();
        destroyLogicalDevice();
        destroySwapchain();
        destroyInstance();
        return false;
    }

    isInitialized_ = true;
    return true;
}

void GraphicsDeviceVulkan::shutdown()
{
    if (!isInitialized_) {
        return;
    }

    // Wait for GPU to complete
    if (device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);
    }

    // Destroy all resources
    if (memoryAllocator) {
        delete memoryAllocator;
        memoryAllocator = nullptr;
    }

    destroySynchronizationObjects();
    destroyFramebuffers();
    destroyRenderPass();
    destroyCommandPool();
    destroyLogicalDevice();
    destroySwapchain();
    destroyInstance();

    isInitialized_ = false;
}

void GraphicsDeviceVulkan::update(float delta)
{
    deltaTime = delta;
    // TODO: Update memory allocator statistics, etc.
}

bool GraphicsDeviceVulkan::isInitialized() const
{
    return isInitialized_;
}

const char* GraphicsDeviceVulkan::getLastError() const
{
    return lastErrorMessage;
}

void GraphicsDeviceVulkan::handleWindowResize(int newWidth, int newHeight)
{
    windowWidth = newWidth;
    windowHeight = newHeight;

    if (swapchain) {
        swapchain->recreate(newWidth, newHeight);
    }
}

// ============================================================
// Frame Management
// ============================================================

bool GraphicsDeviceVulkan::beginFrame()
{
    if (!isInitialized_) {
        setError("GraphicsDeviceVulkan not initialized");
        return false;
    }

    // TODO: Implement frame recording setup
    // 1. Wait for previous frame
    // 2. Acquire next swapchain image
    // 3. Begin command buffer recording
    // 4. Begin render pass

    isFrameRecording = true;
    return true;
}

bool GraphicsDeviceVulkan::endFrame()
{
    if (!isFrameRecording) {
        setError("endFrame() called without beginFrame()");
        return false;
    }

    // TODO: Implement frame submission
    // 1. End render pass
    // 2. End command buffer recording
    // 3. Submit to queue
    // 4. Present to screen

    isFrameRecording = false;
    advanceFrame();
    return true;
}

void GraphicsDeviceVulkan::clear(unsigned int color, float depth, int clearMask)
{
    // TODO: Implement clear color/depth
}

void GraphicsDeviceVulkan::flushGPU()
{
    if (device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);
    }
}

// ============================================================
// Viewport & Scissor
// ============================================================

void GraphicsDeviceVulkan::setViewport(const Viewport& viewport)
{
    currentViewport = viewport;
    // TODO: Record vkCmdSetViewport
}

void GraphicsDeviceVulkan::getViewport(Viewport& outViewport) const
{
    outViewport = currentViewport;
}

void GraphicsDeviceVulkan::setScissor(int x, int y, int width, int height)
{
    // TODO: Record vkCmdSetScissor
}

// ============================================================
// Buffer Management
// ============================================================

int GraphicsDeviceVulkan::createVertexBuffer(const void* data, size_t size, int stride)
{
    if (nextBufferHandle >= MAX_BUFFERS) {
        setError("Buffer limit exceeded");
        return INVALID_GRAPHICS_HANDLE;
    }

    VulkanBuffer* buffer = new VulkanBuffer(device, BUFFER_USAGE_VERTEX);
    if (!buffer->create(data, size, stride, 0)) {
        delete buffer;
        return INVALID_GRAPHICS_HANDLE;
    }

    int handle = nextBufferHandle++;
    buffers[handle] = buffer;
    return handle;
}

int GraphicsDeviceVulkan::createIndexBuffer(const void* data, size_t size, int indexSize)
{
    if (nextBufferHandle >= MAX_BUFFERS) {
        setError("Buffer limit exceeded");
        return INVALID_GRAPHICS_HANDLE;
    }

    VulkanBuffer* buffer = new VulkanBuffer(device, BUFFER_USAGE_INDEX);
    if (!buffer->create(data, size, 0, indexSize)) {
        delete buffer;
        return INVALID_GRAPHICS_HANDLE;
    }

    int handle = nextBufferHandle++;
    buffers[handle] = buffer;
    return handle;
}

int GraphicsDeviceVulkan::createUniformBuffer(const void* data, size_t size)
{
    if (nextBufferHandle >= MAX_BUFFERS) {
        setError("Buffer limit exceeded");
        return INVALID_GRAPHICS_HANDLE;
    }

    VulkanBuffer* buffer = new VulkanBuffer(device, BUFFER_USAGE_UNIFORM);
    if (!buffer->create(data, size, 0, 0)) {
        delete buffer;
        return INVALID_GRAPHICS_HANDLE;
    }

    int handle = nextBufferHandle++;
    buffers[handle] = buffer;
    return handle;
}

void GraphicsDeviceVulkan::updateBuffer(int bufferId, const void* data, size_t size, size_t offset)
{
    if (bufferId < 0 || bufferId >= nextBufferHandle || !buffers[bufferId]) {
        setError("Invalid buffer handle");
        return;
    }

    buffers[bufferId]->update(data, size, offset);
}

void GraphicsDeviceVulkan::destroyBuffer(int bufferId)
{
    if (bufferId < 0 || bufferId >= nextBufferHandle || !buffers[bufferId]) {
        return;
    }

    delete buffers[bufferId];
    buffers[bufferId] = nullptr;
}

void GraphicsDeviceVulkan::bindVertexBuffer(int bufferId, int stride, int offset)
{
    // TODO: Record vkCmdBindVertexBuffers
}

void GraphicsDeviceVulkan::bindIndexBuffer(int bufferId, int offset)
{
    // TODO: Record vkCmdBindIndexBuffer
}

void GraphicsDeviceVulkan::bindUniformBuffer(int bufferId, int shaderSlot)
{
    // TODO: Record descriptor set binding
}

// ============================================================
// Texture Management (Stubs)
// ============================================================

int GraphicsDeviceVulkan::createTexture(const void* data, int width, int height,
                                       TextureFormat format, int mipLevels)
{
    // TODO: Implement texture creation
    return INVALID_GRAPHICS_HANDLE;
}

int GraphicsDeviceVulkan::createRenderTargetTexture(int width, int height, TextureFormat format)
{
    // TODO: Implement render target creation
    return INVALID_GRAPHICS_HANDLE;
}

int GraphicsDeviceVulkan::createDepthTexture(int width, int height, TextureFormat format)
{
    // TODO: Implement depth texture creation
    return INVALID_GRAPHICS_HANDLE;
}

void GraphicsDeviceVulkan::updateTexture(int textureId, const void* data,
                                        int x, int y, int width, int height)
{
    // TODO: Implement texture update
}

void GraphicsDeviceVulkan::bindTexture(int textureId, int slot)
{
    // TODO: Record texture binding
}

void GraphicsDeviceVulkan::destroyTexture(int textureId)
{
    // TODO: Implement texture destruction
}

void GraphicsDeviceVulkan::getTextureSize(int textureId, int& outWidth, int& outHeight) const
{
    // TODO: Get texture dimensions
    outWidth = 0;
    outHeight = 0;
}

// ============================================================
// Shader Management (Stubs)
// ============================================================

int GraphicsDeviceVulkan::createShaderProgram(const char* vertexShaderCode,
                                             const char* fragmentShaderCode,
                                             const VertexAttribute* vertexAttributes,
                                             int attributeCount)
{
    // TODO: Implement shader program creation
    return INVALID_GRAPHICS_HANDLE;
}

int GraphicsDeviceVulkan::createComputeShader(const char* computeShaderCode)
{
    // TODO: Implement compute shader creation
    return INVALID_GRAPHICS_HANDLE;
}

void GraphicsDeviceVulkan::bindShaderProgram(int shaderId)
{
    // TODO: Record pipeline binding
}

void GraphicsDeviceVulkan::bindComputeShader(int shaderId)
{
    // TODO: Record compute pipeline binding
}

void GraphicsDeviceVulkan::dispatchCompute(int groupCountX, int groupCountY, int groupCountZ)
{
    // TODO: Record dispatch compute command
}

void GraphicsDeviceVulkan::destroyShaderProgram(int shaderId)
{
    // TODO: Implement pipeline destruction
}

const char* GraphicsDeviceVulkan::getShaderErrors() const
{
    return "";  // TODO: Return shader compilation errors
}

// ============================================================
// Render State Management (Stubs)
// ============================================================

void GraphicsDeviceVulkan::setRenderState(const RenderState& state)
{
    currentRenderState = state;
    // TODO: Record render state changes
}

void GraphicsDeviceVulkan::getRenderState(RenderState& outState) const
{
    outState = currentRenderState;
}

void GraphicsDeviceVulkan::setRenderStateFlag(unsigned int flag, bool enabled)
{
    // TODO: Modify render state flag
}

void GraphicsDeviceVulkan::setBlendFactor(int srcFactor, int dstFactor)
{
    // TODO: Record blend factor change
}

// ============================================================
// Drawing Operations (Stubs)
// ============================================================

void GraphicsDeviceVulkan::drawTriangleList(int vertexCount, int startVertex)
{
    // TODO: Record vkCmdDraw
}

void GraphicsDeviceVulkan::drawIndexedTriangleList(int indexCount, int startIndex, int baseVertex)
{
    // TODO: Record vkCmdDrawIndexed
}

void GraphicsDeviceVulkan::drawLineList(int lineCount, int startVertex)
{
    // TODO: Record line drawing
}

void GraphicsDeviceVulkan::drawPointList(int pointCount, int startVertex)
{
    // TODO: Record point drawing
}

// ============================================================
// Framebuffer / Render Target Management (Stubs)
// ============================================================

void GraphicsDeviceVulkan::setRenderTarget(int colorTextureId, int depthTextureId)
{
    // TODO: Implement render target binding
}

void GraphicsDeviceVulkan::setMultipleRenderTargets(const int* colorTextureIds,
                                                   int colorTextureCount,
                                                   int depthTextureId)
{
    // TODO: Implement multiple render targets
}

void GraphicsDeviceVulkan::resetRenderTarget()
{
    // TODO: Reset to backbuffer
}

void GraphicsDeviceVulkan::getRenderTargetSize(int& outWidth, int& outHeight) const
{
    outWidth = windowWidth;
    outHeight = windowHeight;
}

// ============================================================
// Feature Queries & Capabilities (Stubs)
// ============================================================

bool GraphicsDeviceVulkan::supportsFeature(const char* featureName) const
{
    // TODO: Check Vulkan extension support
    return false;
}

int GraphicsDeviceVulkan::getMaxTextureSize() const
{
    return 4096;  // TODO: Query from device
}

int GraphicsDeviceVulkan::getMaxTextureUnits() const
{
    return 16;  // TODO: Query from device
}

int GraphicsDeviceVulkan::getMaxRenderTargets() const
{
    return 8;  // TODO: Query from device
}

const char* GraphicsDeviceVulkan::getDeviceName() const
{
    return "Vulkan Device";  // TODO: Return actual device name
}

const char* GraphicsDeviceVulkan::getAPIName() const
{
    return "Vulkan";
}

size_t GraphicsDeviceVulkan::getTotalGPUMemory() const
{
    return 0;  // TODO: Query from device
}

size_t GraphicsDeviceVulkan::getAvailableGPUMemory() const
{
    return 0;  // TODO: Query from allocator
}

// ============================================================
// Debugging & Profiling (Stubs)
// ============================================================

void GraphicsDeviceVulkan::setDebugMode(bool enabled)
{
    // TODO: Enable Vulkan validation layers
}

void GraphicsDeviceVulkan::pushDebugMarker(const char* markerName)
{
    // TODO: Record debug marker
}

void GraphicsDeviceVulkan::popDebugMarker()
{
    // TODO: Pop debug marker
}

float GraphicsDeviceVulkan::getFrameTime() const
{
    return deltaTime * 1000.0f;  // Convert to milliseconds
}

float GraphicsDeviceVulkan::getGPUUtilization() const
{
    return 0.0f;  // TODO: Query GPU utilization
}

// ============================================================
// Private Initialization Methods (Stubs)
// ============================================================

bool GraphicsDeviceVulkan::createInstance()
{
    // Vulkan API Version
    const uint32_t vulkanVersion = VK_API_VERSION_1_2;

    // Application info
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "GeneralsX";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "GeneralsX Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = vulkanVersion;

    // Required extensions for SDL2 surface creation
    uint32_t extensionCount = 0;
    const char** extensionNames = nullptr;
    
    // SDL_Vulkan_GetInstanceExtensions requires SDL window to be created first
    // Extensions will be added by SDL2 integration layer

    // Instance creation info
    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensionNames;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    // Create Vulkan instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        setError("Failed to create Vulkan instance: error code %d", result);
        return false;
    }

    printf("[Vulkan] Instance created successfully (API version: 1.2)\n");
    return true;
}

bool GraphicsDeviceVulkan::selectPhysicalDevice()
{
    // Enumerate all available physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        setError("No Vulkan-capable devices found");
        return false;
    }

    // Get list of physical devices
    VkPhysicalDevice* devices = new VkPhysicalDevice[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    printf("[Vulkan] Found %d physical device(s)\n", deviceCount);

    // Select first suitable device
    bool foundDevice = false;
    for (uint32_t i = 0; i < deviceCount; ++i) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);

        printf("[Vulkan] Device %d: %s (Type: %d)\n", 
               i, props.deviceName, props.deviceType);

        if (isDeviceSuitable(devices[i])) {
            physicalDevice = devices[i];
            foundDevice = true;

            // Log selected device info
            VkPhysicalDeviceMemoryProperties memProps;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

            printf("[Vulkan] Selected: %s\n", props.deviceName);
            printf("[Vulkan]   Max memory: %lu MB\n", 
                   memProps.memoryHeaps[0].size / (1024 * 1024));
            break;
        }
    }

    delete[] devices;

    if (!foundDevice) {
        setError("No suitable Vulkan device found");
        return false;
    }

    // Find graphics queue family
    if (!findQueueFamilies(physicalDevice)) {
        setError("Could not find graphics queue family");
        return false;
    }

    return true;
}

bool GraphicsDeviceVulkan::createLogicalDevice()
{
    // Queue creation info
    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Device creation info
    VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;

    // Device features (keep minimal for VC6 compatibility)
    VkPhysicalDeviceFeatures deviceFeatures;
    memset(&deviceFeatures, 0, sizeof(deviceFeatures));
    // Enable features as needed:
    // deviceFeatures.samplerAnisotropy = VK_TRUE;
    // deviceFeatures.fillModeNonSolid = VK_TRUE;

    createInfo.pEnabledFeatures = &deviceFeatures;

    // Create logical device
    VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
        setError("Failed to create logical Vulkan device: error code %d", result);
        return false;
    }

    // Retrieve graphics queue
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);

    printf("[Vulkan] Logical device created (Queue family: %d)\n", graphicsQueueFamily);
    return true;
}

bool GraphicsDeviceVulkan::createCommandPool()
{
    // Command pool creation info
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = graphicsQueueFamily;

    // Create command pool
    VkResult result = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) {
        setError("Failed to create command pool: error code %d", result);
        return false;
    }

    // Allocate command buffers (one per frame in flight)
    commandBufferCount = 3;  // Triple buffering
    commandBuffers = new VkCommandBuffer[commandBufferCount];

    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBufferCount;

    result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers);
    if (result != VK_SUCCESS) {
        setError("Failed to allocate command buffers: error code %d", result);
        delete[] commandBuffers;
        commandBuffers = nullptr;
        return false;
    }

    printf("[Vulkan] Command pool created with %d command buffers\n", commandBufferCount);
    return true;
}

bool GraphicsDeviceVulkan::createSwapchain(void* windowHandle)
{
    // TODO: Implement swapchain creation
    return false;  // Placeholder
}

bool GraphicsDeviceVulkan::createRenderPass()
{
    // TODO: Implement render pass creation
    return false;  // Placeholder
}

bool GraphicsDeviceVulkan::createFramebuffers()
{
    // TODO: Implement framebuffer creation
    return false;  // Placeholder
}

bool GraphicsDeviceVulkan::createSynchronizationObjects()
{
    // TODO: Implement synchronization object creation
    return false;  // Placeholder
}

bool GraphicsDeviceVulkan::createMemoryAllocator()
{
    // TODO: Implement memory allocator creation
    return false;  // Placeholder
}

// ============================================================
// Private Cleanup Methods (Stubs)
// ============================================================

void GraphicsDeviceVulkan::destroySwapchain()
{
    // TODO: Implement swapchain destruction
}

void GraphicsDeviceVulkan::destroySynchronizationObjects()
{
    // TODO: Implement synchronization object destruction
}

void GraphicsDeviceVulkan::destroyFramebuffers()
{
    // TODO: Implement framebuffer destruction
}

void GraphicsDeviceVulkan::destroyRenderPass()
{
    // TODO: Implement render pass destruction
}

void GraphicsDeviceVulkan::destroyCommandPool()
{
    if (commandBuffers != nullptr) {
        if (commandPool != VK_NULL_HANDLE && device != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(device, commandPool, commandBufferCount, commandBuffers);
        }
        delete[] commandBuffers;
        commandBuffers = nullptr;
    }

    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        printf("[Vulkan] Command pool destroyed\n");
    }
}

void GraphicsDeviceVulkan::destroyLogicalDevice()
{
    if (device != VK_NULL_HANDLE) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
        graphicsQueue = VK_NULL_HANDLE;
        printf("[Vulkan] Logical device destroyed\n");
    }
}

void GraphicsDeviceVulkan::destroyInstance()
{
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
        printf("[Vulkan] Instance destroyed\n");
    }
}

// ============================================================
// Private Utility Methods
// ============================================================

VkFormat GraphicsDeviceVulkan::textureFormatToVulkan(TextureFormat format) const
{
    switch (format) {
    case TEX_FORMAT_RGBA8:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case TEX_FORMAT_BGRA8:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case TEX_FORMAT_BC1:
        return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case TEX_FORMAT_BC3:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case TEX_FORMAT_DEPTH24:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case TEX_FORMAT_DEPTH32F:
        return VK_FORMAT_D32_SFLOAT;
    default:
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

bool GraphicsDeviceVulkan::findQueueFamilies(VkPhysicalDevice device)
{
    // Get available queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    if (queueFamilyCount == 0) {
        setError("Device has no queue families");
        return false;
    }

    VkQueueFamilyProperties* queueFamilies = 
        new VkQueueFamilyProperties[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    // Find queue family with graphics support
    bool foundGraphicsQueue = false;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamily = i;
            foundGraphicsQueue = true;
            printf("[Vulkan] Found graphics queue family at index %d\n", i);
            break;
        }
    }

    delete[] queueFamilies;

    return foundGraphicsQueue;
}

bool GraphicsDeviceVulkan::isDeviceSuitable(VkPhysicalDevice device) const
{
    // Check device type (prefer discrete GPUs)
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);

    // Check for graphics support via queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    if (queueFamilyCount == 0) {
        return false;
    }

    VkQueueFamilyProperties* queueFamilies = 
        new VkQueueFamilyProperties[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    bool hasGraphics = false;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            hasGraphics = true;
            break;
        }
    }

    delete[] queueFamilies;

    // Check minimum memory requirement (512 MB)
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(device, &memProps);

    bool hasEnoughMemory = false;
    for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
        if (memProps.memoryHeaps[i].size >= (512 * 1024 * 1024)) {
            hasEnoughMemory = true;
            break;
        }
    }

    // Accept device if it has graphics queue and minimum memory
    bool suitable = hasGraphics && hasEnoughMemory;

    if (suitable && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        printf("[Vulkan] Device '%s' is discrete GPU - preferred\n", props.deviceName);
    } else if (suitable) {
        printf("[Vulkan] Device '%s' is suitable\n", props.deviceName);
    }

    return suitable;
}

void GraphicsDeviceVulkan::setError(const char* format, ...)
{
    // Simple string formatting (VC6 compatible)
    if (!format) return;

    va_list args;
    va_start(args, format);
    vsnprintf(lastErrorMessage, sizeof(lastErrorMessage) - 1, format, args);
    va_end(args);

    lastErrorMessage[sizeof(lastErrorMessage) - 1] = '\0';
}

void GraphicsDeviceVulkan::advanceFrame()
{
    currentFrameIndex = (currentFrameIndex + 1) % (synchronizationObjectCount > 0 ? synchronizationObjectCount : 1);
}

} // namespace Graphics
} // namespace GeneralsX
