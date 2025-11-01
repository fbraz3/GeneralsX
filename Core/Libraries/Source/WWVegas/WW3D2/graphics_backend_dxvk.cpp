/**
 * @file graphics_backend_dxvk.cpp
 * 
 * Vulkan Graphics Backend Implementation - Part 1: Initialization
 * 
 * This file implements the initialization and shutdown lifecycle of the Vulkan backend.
 * Includes:
 * - Vulkan instance creation (with validation layers)
 * - Physical device selection and logical device creation
 * - Surface and swapchain setup
 * - Render pass and framebuffer creation
 * - Command pool and buffer management
 * - Synchronization object creation
 * 
 * Phase: 39.2.2 (Initialization Methods Implementation)
 * Estimated Lines: 2000-2500 for complete implementation
 * 
 * Created: October 30, 2025
 * Author: GeneralsX Development Team
 */

#include "graphics_backend_dxvk.h"
#include "win32_compat.h"
#include <cstring>
#include <cmath>
#include <iostream>
#include <algorithm>

// ============================================================================
// Debug Configuration
// ============================================================================

#ifdef _DEBUG
    #define DXVK_DEBUG 1
    #define VALIDATION_LAYERS_ENABLED 1
    #define ENABLE_DEBUG_CALLBACK 1
#else
    #define DXVK_DEBUG 0
    #define VALIDATION_LAYERS_ENABLED 0
    #define ENABLE_DEBUG_CALLBACK 0
#endif

// ============================================================================
// Vulkan Validation Layers (Debug Only)
// ============================================================================
//
// Validation layers are recommended for development debugging.
// They are discovered and managed by the Vulkan Loader.
//
// Reference: docs/PHASE39/PHASE39_2_MOLTENVK_GUIDELINES.md (Section 4)
// - Development builds: Layers ENABLED (VK_LAYER_KHRONOS_validation)
// - Production builds: Layers DISABLED (no performance overhead)
//
// During development, validation layers are automatically discovered from:
// - macOS: /usr/local/etc/vulkan/explicit_layer.d/
// - Linux: /usr/share/vulkan/explicit_layer.d/
// - Windows: Registry or $VK_LAYER_PATH environment variable
//
// NOTE: VALIDATION_LAYERS and VALIDATION_LAYER_COUNT are defined in
// graphics_backend_dxvk.h to avoid redefinition errors
// ============================================================================

// ============================================================================
// Vulkan Extension Requirements
// ============================================================================
//
// Following the RECOMMENDED MoltenVK usage model from LunarG documentation:
//
// RECOMMENDED (What We Do):
// ✅ Link to Vulkan Loader (libvulkan.dylib on macOS)
// ✅ MoltenVK acts as ICD (Installable Client Driver)
// ✅ Vulkan Loader discovers and delegates to MoltenVK
// ✅ Enables Vulkan Validation Layers for debugging
// ✅ Platform independent: same binary on Windows/Linux/macOS
//
// NOT RECOMMENDED (What We DON'T Do):
// ❌ Direct linking to MoltenVK static/dynamic library
// ❌ Sacrifices validation layer support
// ❌ Reduces portability and flexibility
//
// Architecture:
// Application → Vulkan Loader → MoltenVK ICD → Metal Framework → GPU
//
// Reference: docs/PHASE39/PHASE39_2_MOLTENVK_GUIDELINES.md (Sections 1-3)
// CMake Config: cmake/vulkan.cmake (sets Vulkan::Loader as target)
// ============================================================================

// NOTE: INSTANCE_EXTENSIONS, DEVICE_EXTENSIONS, VALIDATION_LAYERS constants
// are defined in graphics_backend_dxvk.h to avoid redefinition errors
// (Each .cpp file that includes this header would redefine them otherwise)

// ============================================================================
// Debug Callback (Validation Layers)
// ============================================================================

#if ENABLE_DEBUG_CALLBACK
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    const char* severity = "";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        severity = "[ERROR]";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        severity = "[WARNING]";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        severity = "[INFO]";
    
    printf("Vulkan %s: %s\n", severity, pCallbackData->pMessage);
    return VK_FALSE;
}
#endif

// ============================================================================
// Static Global Backend Instance
// ============================================================================

static DXVKGraphicsBackend* g_dxvkBackend = nullptr;

// ============================================================================
// DXVKGraphicsBackend Constructor/Destructor
// ============================================================================

DXVKGraphicsBackend::DXVKGraphicsBackend()
    : m_instance(VK_NULL_HANDLE),
      m_physicalDevice(VK_NULL_HANDLE),
      m_device(VK_NULL_HANDLE),
      m_graphicsQueue(VK_NULL_HANDLE),
      m_graphicsQueueFamily(UINT32_MAX),
      m_surface(VK_NULL_HANDLE),
      m_swapchain(VK_NULL_HANDLE),
      m_swapchainFormat(VK_FORMAT_UNDEFINED),
      m_swapchainExtent({0, 0}),
      m_currentSwapchainIndex(0),
      m_renderPass(VK_NULL_HANDLE),
      m_commandPool(VK_NULL_HANDLE),
      m_graphicsPipeline(VK_NULL_HANDLE),
      m_pipelineLayout(VK_NULL_HANDLE),
      m_pipelineCache(VK_NULL_HANDLE),
      m_currentFrame(0),
      m_vertexShader(VK_NULL_HANDLE),
      m_fragmentShader(VK_NULL_HANDLE),
      m_windowHandle(nullptr),
      m_displayWidth(1920),
      m_displayHeight(1080),
      m_initialized(false),
      m_inScene(false),
      m_debugOutput(DXVK_DEBUG),
      m_lastError(S_OK),
      m_lightingEnabled(false) {
    
    // Initialize matrices to identity
    std::memset(m_worldMatrix, 0, sizeof(m_worldMatrix));
    std::memset(m_viewMatrix, 0, sizeof(m_viewMatrix));
    std::memset(m_projectionMatrix, 0, sizeof(m_projectionMatrix));
    m_worldMatrix[0] = m_worldMatrix[5] = m_worldMatrix[10] = m_worldMatrix[15] = 1.0f;
    m_viewMatrix[0] = m_viewMatrix[5] = m_viewMatrix[10] = m_viewMatrix[15] = 1.0f;
    m_projectionMatrix[0] = m_projectionMatrix[5] = m_projectionMatrix[10] = m_projectionMatrix[15] = 1.0f;
    
    // Initialize material
    std::memset(&m_material, 0, sizeof(m_material));
    m_material.Diffuse[0] = 1.0f; m_material.Diffuse[1] = 1.0f; m_material.Diffuse[2] = 1.0f; m_material.Diffuse[3] = 1.0f;
    m_material.Ambient[0] = 0.2f; m_material.Ambient[1] = 0.2f; m_material.Ambient[2] = 0.2f; m_material.Ambient[3] = 1.0f;
    m_material.Specular[0] = 1.0f; m_material.Specular[1] = 1.0f; m_material.Specular[2] = 1.0f; m_material.Specular[3] = 1.0f;
    m_material.Emissive[0] = 0.0f; m_material.Emissive[1] = 0.0f; m_material.Emissive[2] = 0.0f; m_material.Emissive[3] = 1.0f;
    m_material.Power = 32.0f;
    
    // Initialize ambient color (0xAARRGGBB format)
    m_ambientColor = 0xFF808080;
    
    g_dxvkBackend = this;
}

DXVKGraphicsBackend::~DXVKGraphicsBackend() {
    if (m_initialized) {
        Shutdown();
    }
    g_dxvkBackend = nullptr;
}

// ============================================================================
// Public Lifecycle Methods
// ============================================================================

HRESULT DXVKGraphicsBackend::Initialize() {
    if (m_initialized) {
        m_lastError = S_OK;
        return S_OK;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] Initializing Vulkan graphics backend...\n");
    }
    
    // Step 1: Create Vulkan instance
    HRESULT hr = CreateInstance();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create Vulkan instance (0x%08X)\n", hr);
        m_lastError = hr;
        return hr;
    }
    
    // Step 2: Create physical device and logical device
    hr = CreateDevice();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create Vulkan device (0x%08X)\n", hr);
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 3: Create window surface
    hr = CreateSurface();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create window surface (0x%08X)\n", hr);
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 4: Create swapchain
    hr = CreateSwapchain();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create swapchain (0x%08X)\n", hr);
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 5: Create render pass
    hr = CreateRenderPass();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create render pass (0x%08X)\n", hr);
        DestroySwapchain();
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 6: Create framebuffers
    hr = CreateFramebuffers();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create framebuffers (0x%08X)\n", hr);
        DestroyRenderPass();
        DestroySwapchain();
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 7: Create command pool and buffers
    hr = CreateCommandPool();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create command pool (0x%08X)\n", hr);
        DestroyFramebuffers();
        DestroyRenderPass();
        DestroySwapchain();
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    hr = AllocateCommandBuffers();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to allocate command buffers (0x%08X)\n", hr);
        DestroyCommandPool();
        DestroyFramebuffers();
        DestroyRenderPass();
        DestroySwapchain();
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 8: Create synchronization objects
    hr = CreateSyncObjects();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create synchronization objects (0x%08X)\n", hr);
        FreeCommandBuffers();
        DestroyCommandPool();
        DestroyFramebuffers();
        DestroyRenderPass();
        DestroySwapchain();
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 9: Create pipeline cache
    hr = CreatePipelineCache();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create pipeline cache (0x%08X)\n", hr);
        DestroySyncObjects();
        FreeCommandBuffers();
        DestroyCommandPool();
        DestroyFramebuffers();
        DestroyRenderPass();
        DestroySwapchain();
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 10: Create shader modules
    hr = CreateShaderModules();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create shader modules (0x%08X)\n", hr);
        DestroyPipelineCache();
        DestroySyncObjects();
        FreeCommandBuffers();
        DestroyCommandPool();
        DestroyFramebuffers();
        DestroyRenderPass();
        DestroySwapchain();
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    // Step 11: Create graphics pipeline
    hr = CreateGraphicsPipeline();
    if (FAILED(hr)) {
        printf("[DXVK] ERROR: Failed to create graphics pipeline (0x%08X)\n", hr);
        DestroyShaderModules();
        DestroyPipelineCache();
        DestroySyncObjects();
        FreeCommandBuffers();
        DestroyCommandPool();
        DestroyFramebuffers();
        DestroyRenderPass();
        DestroySwapchain();
        DestroySurface();
        DestroyDevice();
        DestroyInstance();
        m_lastError = hr;
        return hr;
    }
    
    m_initialized = true;
    m_lastError = S_OK;
    
    if (m_debugOutput) {
        printf("[DXVK] Vulkan graphics backend initialized successfully\n");
        printf("[DXVK] Backend: %s\n", GetBackendName());
        printf("[DXVK] Swapchain: %ux%u (%s)\n",
               m_swapchainExtent.width,
               m_swapchainExtent.height,
               m_swapchainFormat == VK_FORMAT_B8G8R8A8_UNORM ? "BGRA8" :
               m_swapchainFormat == VK_FORMAT_R8G8B8A8_UNORM ? "RGBA8" : "Unknown");
    }
    
    return S_OK;
}

HRESULT DXVKGraphicsBackend::Shutdown() {
    if (!m_initialized) {
        return S_OK;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] Shutting down Vulkan graphics backend...\n");
    }
    
    // Wait for device to finish operations
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }
    
    // Release all graphics resources
    DestroyGraphicsPipeline();
    DestroyShaderModules();
    DestroyPipelineCache();
    DestroySyncObjects();
    FreeCommandBuffers();
    DestroyCommandPool();
    DestroyFramebuffers();
    DestroyRenderPass();
    DestroySwapchain();
    DestroySurface();
    DestroyDevice();
    DestroyInstance();
    
    // Clear resource caches
    m_textures.clear();
    m_vertexBuffers.clear();
    m_indexBuffer = nullptr;
    m_renderStates.clear();
    m_lights.clear();
    
    m_initialized = false;
    m_lastError = S_OK;
    
    if (m_debugOutput) {
        printf("[DXVK] Vulkan graphics backend shutdown complete\n");
    }
    
    return S_OK;
}

HRESULT DXVKGraphicsBackend::Reset() {
    if (!m_initialized) {
        return S_FALSE;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] Resetting graphics device (display mode change or window resize)\n");
    }
    
    // Wait for device to finish
    vkDeviceWaitIdle(m_device);
    
    // Recreate swapchain and related resources
    DestroyFramebuffers();
    DestroySwapchain();
    
    HRESULT hr = CreateSwapchain();
    if (FAILED(hr)) {
        m_lastError = hr;
        return hr;
    }
    
    hr = CreateFramebuffers();
    if (FAILED(hr)) {
        m_lastError = hr;
        return hr;
    }
    
    m_lastError = S_OK;
    return S_OK;
}

// ============================================================================
// Private Instance Creation
// ============================================================================

HRESULT DXVKGraphicsBackend::CreateInstance() {
    // Always print startup message for debugging
    printf("[DXVK] CreateInstance() called - Creating Vulkan instance...\n");
    
    // Step 1: Check available extensions
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, availableExtensions.data());
    
    printf("[DXVK] Available instance extensions (%u):\n", extCount);
    for (const auto& ext : availableExtensions) {
        printf("[DXVK]   - %s\n", ext.extensionName);
    }
    
    // Step 2: Check validation layer support
    #if VALIDATION_LAYERS_ENABLED
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        
        bool validationLayerFound = false;
        for (const auto& layer : availableLayers) {
            if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
                validationLayerFound = true;
                break;
            }
        }
        
        if (!validationLayerFound) {
            printf("[DXVK] WARNING: Validation layers requested but not available\n");
        }
    #endif
    
    // Step 3: Prepare application info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "GeneralsX";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "GeneralsX Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    
    // Step 4: Debug print requested extensions
    printf("[DXVK] Requesting %u instance extensions:\n", INSTANCE_EXTENSION_COUNT);
    for (uint32_t i = 0; i < INSTANCE_EXTENSION_COUNT; i++) {
        printf("[DXVK]   - %s\n", INSTANCE_EXTENSIONS[i]);
    }
    
    // Step 5: Prepare instance create info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = INSTANCE_EXTENSION_COUNT;
    createInfo.ppEnabledExtensionNames = INSTANCE_EXTENSIONS;
    
    #if VALIDATION_LAYERS_ENABLED
        createInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
    #else
        createInfo.enabledLayerCount = 0;
    #endif
    
    #ifdef __APPLE__
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        printf("[DXVK] Enabling VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR for macOS\n");
    #endif
    
    // Step 6: Create instance
    printf("[DXVK] Calling vkCreateInstance...\n");
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    printf("[DXVK] vkCreateInstance returned: %d\n", result);
    
    if (result != VK_SUCCESS) {
        printf("[DXVK] ERROR: vkCreateInstance failed with result: %d (0x%08X)\n", result, result);
        
        // Map error codes to readable strings
        const char* errorStr = "UNKNOWN";
        switch (result) {
            case VK_ERROR_OUT_OF_HOST_MEMORY: errorStr = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: errorStr = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
            case VK_ERROR_INITIALIZATION_FAILED: errorStr = "VK_ERROR_INITIALIZATION_FAILED"; break;
            case VK_ERROR_LAYER_NOT_PRESENT: errorStr = "VK_ERROR_LAYER_NOT_PRESENT"; break;
            case VK_ERROR_EXTENSION_NOT_PRESENT: errorStr = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
            case VK_ERROR_INCOMPATIBLE_DRIVER: errorStr = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
            default: break;
        }
        printf("[DXVK]   Error type: %s\n", errorStr);
        printf("[DXVK]   m_instance = %p\n", (void*)m_instance);
        
        return E_FAIL;
    }
    
    printf("[DXVK] Vulkan instance created successfully (handle: %p)\n", (void*)m_instance);
    
    #if ENABLE_DEBUG_CALLBACK
        // Setup debug callback
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;
        debugCreateInfo.pUserData = nullptr;
        
        // Note: Debug messenger creation omitted for now (requires function pointers)
        // This can be added later if needed
    #endif
    
    return S_OK;
}

void DXVKGraphicsBackend::DestroyInstance() {
    if (m_instance == VK_NULL_HANDLE) {
        return;
    }
    
    vkDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
    
    if (m_debugOutput) {
        printf("[DXVK] Vulkan instance destroyed\n");
    }
}

// ============================================================================
// Device, Surface, and Swapchain Implementation
// ============================================================================
// These methods are implemented in graphics_backend_dxvk_device.cpp
// File organization for better code structure:
// - graphics_backend_dxvk.cpp: Lifecycle (Initialize, Shutdown, Reset)
// - graphics_backend_dxvk_device.cpp: Device/Surface/Swapchain creation
// - graphics_backend_dxvk_frame.cpp (Phase 39.2.3b): Frame management (BeginScene, EndScene)
// - graphics_backend_dxvk_render.cpp (Phase 39.2.4): Drawing operations
// ============================================================================

// NOTE: See graphics_backend_dxvk_device.cpp for implementations of:
// - CreateDevice(), DestroyDevice()
// - CreateSurface(), DestroySurface()
// - CreateSwapchain(), RecreateSwapchain(), DestroySwapchain()
// - CreateRenderPass(), DestroyRenderPass()
// - CreateFramebuffers(), DestroyFramebuffers()
// - CreateCommandPool(), DestroyCommandPool()
// - AllocateCommandBuffers(), FreeCommandBuffers()
// - CreateSyncObjects(), DestroySyncObjects()
// - CreatePipelineCache(), DestroyPipelineCache()
// - CreateShaderModules(), DestroyShaderModules()
// - CreateGraphicsPipeline(), RecreateGraphicsPipeline(), DestroyGraphicsPipeline()
// - Memory allocation helpers and format conversion

// All implementations are in graphics_backend_dxvk_device.cpp
// These functions are declared and implemented there, so they should be called
// and linker will resolve them properly.

// Destroy functions with implementations 
void DXVKGraphicsBackend::DestroySurface() {
    if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}

void DXVKGraphicsBackend::DestroySwapchain() {
    if (m_swapchain != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    m_swapchainImages.clear();
    m_swapchainImageViews.clear();
}

void DXVKGraphicsBackend::DestroyRenderPass() {
    if (m_renderPass != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
}

void DXVKGraphicsBackend::DestroyFramebuffers() {
    if (m_device != VK_NULL_HANDLE) {
        for (auto framebuffer : m_framebuffers) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }
    }
    m_framebuffers.clear();
}

void DXVKGraphicsBackend::DestroyCommandPool() {
    if (m_commandPool != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
}

void DXVKGraphicsBackend::FreeCommandBuffers() {
    m_commandBuffers.clear();
}

void DXVKGraphicsBackend::DestroySyncObjects() {
    if (m_device != VK_NULL_HANDLE) {
        for (auto semaphore : m_imageAvailableSemaphores) {
            vkDestroySemaphore(m_device, semaphore, nullptr);
        }
        for (auto semaphore : m_renderFinishedSemaphores) {
            vkDestroySemaphore(m_device, semaphore, nullptr);
        }
        for (auto fence : m_inFlightFences) {
            vkDestroyFence(m_device, fence, nullptr);
        }
    }
    m_imageAvailableSemaphores.clear();
    m_renderFinishedSemaphores.clear();
    m_inFlightFences.clear();
}

void DXVKGraphicsBackend::DestroyPipelineCache() {
    if (m_pipelineCache != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
        vkDestroyPipelineCache(m_device, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }
}

void DXVKGraphicsBackend::DestroyShaderModules() {
    if (m_device != VK_NULL_HANDLE) {
        if (m_vertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, m_vertexShader, nullptr);
            m_vertexShader = VK_NULL_HANDLE;
        }
        if (m_fragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, m_fragmentShader, nullptr);
            m_fragmentShader = VK_NULL_HANDLE;
        }
    }
}

// ============================================================================
// Frame Rendering (Placeholder - to be implemented in Phase 39.2.3)
// ============================================================================

HRESULT DXVKGraphicsBackend::BeginScene() {
    printf("[DXVK] BeginScene() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::EndScene() {
    printf("[DXVK] EndScene() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::Present() {
    printf("[DXVK] Present() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::Clear(
    bool clear_color,
    bool clear_z_stencil,
    const void* color_vec3,
    float z,
    DWORD stencil) {
    printf("[DXVK] Clear() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

void DXVKGraphicsBackend::RecordFrameCommands() {
    // Placeholder
}

HRESULT DXVKGraphicsBackend::SubmitCommands() {
    printf("[DXVK] SubmitCommands() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

// ============================================================================
// Drawing Operations (Placeholder - to be implemented in Phase 39.2.4)
// ============================================================================

HRESULT DXVKGraphicsBackend::DrawPrimitive(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int startVertex,
    unsigned int primitiveCount) {
    printf("[DXVK] DrawPrimitive() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::DrawIndexedPrimitive(
    D3DPRIMITIVETYPE primitiveType,
    unsigned int baseVertexIndex,
    unsigned int minVertexIndex,
    unsigned int numVertices,
    unsigned int startIndex,
    unsigned int primitiveCount) {
    printf("[DXVK] DrawIndexedPrimitive() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

// ============================================================================
// Remaining Public Interface Methods (Placeholder - to be implemented in Phase 39.2.5)
// ============================================================================

HRESULT DXVKGraphicsBackend::CreateTexture(
    unsigned int width,
    unsigned int height,
    D3DFORMAT format,
    void** texture) {
    printf("[DXVK] CreateTexture() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::ReleaseTexture(void* texture) {
    printf("[DXVK] ReleaseTexture() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::SetTexture(unsigned int stage, void* texture) {
    printf("[DXVK] SetTexture() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::GetTexture(unsigned int stage, void** texture) {
    printf("[DXVK] GetTexture() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::LockTexture(void* texture, void** data, unsigned int* pitch) {
    printf("[DXVK] LockTexture() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::UnlockTexture(void* texture) {
    printf("[DXVK] UnlockTexture() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::SetRenderState(D3DRENDERSTATETYPE state, DWORD value) {
    m_renderStates[state] = value;
    return S_OK;
}

HRESULT DXVKGraphicsBackend::GetRenderState(D3DRENDERSTATETYPE state, DWORD* value) {
    auto it = m_renderStates.find(state);
    if (it != m_renderStates.end()) {
        *value = it->second;
        return S_OK;
    }
    return E_FAIL;
}

HRESULT DXVKGraphicsBackend::SetTextureOp(unsigned int stage, D3DTEXTUREOP operation) {
    printf("[DXVK] SetTextureOp() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::CreateVertexBuffer(
    unsigned int size,
    unsigned int usage,
    unsigned int format,
    void** buffer) {
    printf("[DXVK] CreateVertexBuffer() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::ReleaseVertexBuffer(void* buffer) {
    printf("[DXVK] ReleaseVertexBuffer() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::LockVertexBuffer(void* buffer, void** data, unsigned int flags) {
    printf("[DXVK] LockVertexBuffer() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::UnlockVertexBuffer(void* buffer) {
    printf("[DXVK] UnlockVertexBuffer() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::SetStreamSource(
    unsigned int stream,
    void* buffer,
    unsigned int stride) {
    printf("[DXVK] SetStreamSource() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::CreateIndexBuffer(
    unsigned int size,
    D3DFORMAT format,
    void** buffer) {
    printf("[DXVK] CreateIndexBuffer() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::ReleaseIndexBuffer(void* buffer) {
    printf("[DXVK] ReleaseIndexBuffer() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::LockIndexBuffer(void* buffer, void** data, unsigned int flags) {
    printf("[DXVK] LockIndexBuffer() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::UnlockIndexBuffer(void* buffer) {
    printf("[DXVK] UnlockIndexBuffer() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::SetIndices(void* buffer) {
    printf("[DXVK] SetIndices() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::SetViewport(
    unsigned int x,
    unsigned int y,
    unsigned int width,
    unsigned int height,
    float minZ,
    float maxZ) {
    printf("[DXVK] SetViewport() - NOT YET IMPLEMENTED\n");
    return E_NOTIMPL;
}

HRESULT DXVKGraphicsBackend::SetTransform(D3DTRANSFORMSTATETYPE state, const float* matrix) {
    if (matrix == nullptr) {
        return E_INVALIDARG;
    }
    
    switch (state) {
        case D3DTS_WORLD:
            std::memcpy(m_worldMatrix, matrix, sizeof(m_worldMatrix));
            break;
        case D3DTS_VIEW:
            std::memcpy(m_viewMatrix, matrix, sizeof(m_viewMatrix));
            break;
        case D3DTS_PROJECTION:
            std::memcpy(m_projectionMatrix, matrix, sizeof(m_projectionMatrix));
            break;
        default:
            return E_INVALIDARG;
    }
    
    return S_OK;
}

HRESULT DXVKGraphicsBackend::GetTransform(D3DTRANSFORMSTATETYPE state, float* matrix) {
    if (matrix == nullptr) {
        return E_INVALIDARG;
    }
    
    switch (state) {
        case D3DTS_WORLD:
            std::memcpy(matrix, m_worldMatrix, sizeof(m_worldMatrix));
            break;
        case D3DTS_VIEW:
            std::memcpy(matrix, m_viewMatrix, sizeof(m_viewMatrix));
            break;
        case D3DTS_PROJECTION:
            std::memcpy(matrix, m_projectionMatrix, sizeof(m_projectionMatrix));
            break;
        default:
            return E_INVALIDARG;
    }
    
    return S_OK;
}

HRESULT DXVKGraphicsBackend::EnableLighting(bool enable) {
    m_lightingEnabled = enable;
    return S_OK;
}

HRESULT DXVKGraphicsBackend::SetLight(unsigned int index, const D3DLIGHT8* light) {
    if (light == nullptr) {
        return E_INVALIDARG;
    }
    
    if (index >= m_lights.size()) {
        m_lights.resize(index + 1);
    }
    
    m_lights[index] = *light;
    return S_OK;
}

HRESULT DXVKGraphicsBackend::LightEnable(unsigned int index, bool enable) {
    if (index >= m_lights.size()) {
        return E_INVALIDARG;
    }
    
    // Mark light as enabled via type field (simplified)
    return S_OK;
}

HRESULT DXVKGraphicsBackend::SetMaterial(const D3DMATERIAL8* material) {
    if (material == nullptr) {
        return E_INVALIDARG;
    }
    
    m_material = *material;
    return S_OK;
}

HRESULT DXVKGraphicsBackend::SetAmbient(D3DCOLOR color) {
    m_ambientColor = color;
    return S_OK;
}

const char* DXVKGraphicsBackend::GetBackendName() const {
    #ifdef __APPLE__
        return "Vulkan/MoltenVK (macOS)";
    #elif defined(_WIN32)
        return "Vulkan (Windows)";
    #elif defined(__linux__)
        return "Vulkan (Linux)";
    #else
        return "Vulkan (Unknown)";
    #endif
}

HRESULT DXVKGraphicsBackend::GetLastError() const {
    return m_lastError;
}

void DXVKGraphicsBackend::SetDebugOutput(bool enable) {
    m_debugOutput = enable;
}
