#include "GraphicsDriverFactory.h"
#include "IGraphicsDriver.h"
#include "dx8buffer_compat.h"  // For SetGraphicsDriver

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace Graphics {

// ============================================================================
// FORWARD DECLARATIONS - Backend Factory Functions
// ============================================================================
// These are defined in respective backend .cpp files

extern "C" {
    IGraphicsDriver* CreateVulkanGraphicsDriver(void* windowHandle, uint32_t width,
                                              uint32_t height, bool fullscreen);
    
    IGraphicsDriver* CreateOpenGLGraphicsDriver(void* windowHandle, uint32_t width,
                                              uint32_t height, bool fullscreen);
    
    IGraphicsDriver* CreateDirectX12GraphicsDriver(void* windowHandle, uint32_t width,
                                                  uint32_t height, bool fullscreen);
    
    IGraphicsDriver* CreateMetalGraphicsDriver(void* windowHandle, uint32_t width,
                                             uint32_t height, bool fullscreen);
    
    IGraphicsDriver* CreateSoftwareGraphicsDriver(void* windowHandle, uint32_t width,
                                                uint32_t height, bool fullscreen);
}

// ============================================================================
// IMPLEMENTATION - Factory Methods
// ============================================================================

IGraphicsDriver* GraphicsDriverFactory::CreateDriver(BackendType backendType, void* windowHandle,
                                                    uint32_t width, uint32_t height, bool fullscreen) {
    // Determine which backend to use
    BackendType selectedBackend = backendType;
    
    if (selectedBackend == BackendType::Unknown) {
        // Priority 1: GRAPHICS_DRIVER environment variable
        selectedBackend = GetBackendFromEnvironment();
        
        // Priority 2: ~/.generalsX/graphics.ini configuration
        if (selectedBackend == BackendType::Unknown) {
            selectedBackend = GetBackendFromConfig();
        }
        
        // Priority 3: Platform default (Vulkan for Phase 41)
        if (selectedBackend == BackendType::Unknown) {
            selectedBackend = GetPlatformDefault();
        }
    }
    
    // Validate that requested backend is supported
    if (!IsSupportedBackend(selectedBackend)) {
        // Fallback to platform default
        selectedBackend = GetPlatformDefault();
        
        // If even default fails, try to find ANY supported backend
        if (!IsSupportedBackend(selectedBackend)) {
            std::vector<BackendType> supported = GetSupportedBackends();
            if (!supported.empty()) {
                selectedBackend = supported[0];
            } else {
                return nullptr;  // No backends available
            }
        }
    }
    
    // Create backend-specific driver instance
    IGraphicsDriver* driver = nullptr;
    
    switch (selectedBackend) {
        case BackendType::Vulkan:
            driver = CreateVulkanGraphicsDriver(windowHandle, width, height, fullscreen);
            break;
            
        case BackendType::OpenGL:
            driver = CreateOpenGLGraphicsDriver(windowHandle, width, height, fullscreen);
            break;
            
        case BackendType::DirectX12:
            driver = CreateDirectX12GraphicsDriver(windowHandle, width, height, fullscreen);
            break;
            
        case BackendType::Metal:
            driver = CreateMetalGraphicsDriver(windowHandle, width, height, fullscreen);
            break;
            
        case BackendType::Software:
            driver = CreateSoftwareGraphicsDriver(windowHandle, width, height, fullscreen);
            break;
            
        default:
            return nullptr;
    }
    
    // Register driver globally for legacy DX8 compatibility layer access
    if (driver) {
        SetGraphicsDriver(driver);
    }
    
    return driver;
}

void GraphicsDriverFactory::DestroyDriver(IGraphicsDriver* driver) {
    if (driver) {
        driver->Shutdown();
        delete driver;
    }
}

bool GraphicsDriverFactory::IsSupportedBackend(BackendType backendType) {
    switch (backendType) {
        case BackendType::Vulkan:
            return IsVulkanAvailable();
        case BackendType::OpenGL:
            return IsOpenGLAvailable();
        case BackendType::DirectX12:
            return IsDirectX12Available();
        case BackendType::Metal:
            return IsMetalAvailable();
        case BackendType::Software:
            return true;  // Software backend always available
        default:
            return false;
    }
}

std::vector<BackendType> GraphicsDriverFactory::GetSupportedBackends() {
    std::vector<BackendType> supported;
    
    if (IsVulkanAvailable()) {
        supported.push_back(BackendType::Vulkan);
    }
    if (IsOpenGLAvailable()) {
        supported.push_back(BackendType::OpenGL);
    }
    if (IsDirectX12Available()) {
        supported.push_back(BackendType::DirectX12);
    }
    if (IsMetalAvailable()) {
        supported.push_back(BackendType::Metal);
    }
    // Software is always available
    supported.push_back(BackendType::Software);
    
    return supported;
}

std::string GraphicsDriverFactory::GetBackendDisplayName(BackendType backendType) {
    switch (backendType) {
        case BackendType::Vulkan:
            return "Vulkan";
        case BackendType::OpenGL:
            return "OpenGL";
        case BackendType::DirectX12:
            return "DirectX 12";
        case BackendType::Metal:
            return "Metal";
        case BackendType::Software:
            return "Software";
        case BackendType::Unknown:
        default:
            return "Unknown";
    }
}

std::string GraphicsDriverFactory::GetSelectedBackendName() {
    BackendType selected = GetBackendFromEnvironment();
    
    if (selected == BackendType::Unknown) {
        selected = GetBackendFromConfig();
    }
    
    if (selected == BackendType::Unknown) {
        selected = GetPlatformDefault();
    }
    
    return BackendTypeToString(selected);
}

std::string GraphicsDriverFactory::GetDefaultBackendName() {
    BackendType backend = GetBackendFromEnvironment();
    
    if (backend == BackendType::Unknown) {
        backend = GetBackendFromConfig();
    }
    
    if (backend == BackendType::Unknown) {
        backend = GetPlatformDefault();
    }
    
    return BackendTypeToString(backend);
}

bool GraphicsDriverFactory::IsVulkanAvailable() {
#ifdef GRAPHICS_BACKEND_VULKAN
    return true;
#else
    return false;
#endif
}

bool GraphicsDriverFactory::IsOpenGLAvailable() {
#ifdef GRAPHICS_BACKEND_OPENGL
    return true;
#else
    return false;
#endif
}

bool GraphicsDriverFactory::IsDirectX12Available() {
#ifdef GRAPHICS_BACKEND_DIRECTX12
    return true;
#else
    return false;
#endif
}

bool GraphicsDriverFactory::IsMetalAvailable() {
#ifdef GRAPHICS_BACKEND_METAL
    return true;
#else
    return false;
#endif
}

// ============================================================================
// HELPER METHODS
// ============================================================================

BackendType GraphicsDriverFactory::GetBackendFromEnvironment() {
    const char* env = std::getenv("GRAPHICS_DRIVER");
    if (env) {
        return NormalizeBackendName(std::string(env));
    }
    return BackendType::Unknown;
}

BackendType GraphicsDriverFactory::GetBackendFromConfig() {
    // Try to read ~/.generalsX/graphics.ini
    const char* homeDir = std::getenv("HOME");
    if (!homeDir) {
        return BackendType::Unknown;
    }
    
    std::string configPath = std::string(homeDir) + "/.generalsX/graphics.ini";
    std::ifstream configFile(configPath);
    
    if (!configFile.is_open()) {
        return BackendType::Unknown;
    }
    
    std::string line;
    bool inGraphicsSection = false;
    
    while (std::getline(configFile, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Check for section header
        if (line.find("[Graphics]") == 0) {
            inGraphicsSection = true;
            continue;
        }
        
        // Check for next section (end of Graphics section)
        if (line.length() > 0 && line[0] == '[') {
            inGraphicsSection = false;
            continue;
        }
        
        // Parse key=value in Graphics section
        if (inGraphicsSection) {
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);
                
                // Trim key and value
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                if (key == "Driver") {
                    return NormalizeBackendName(value);
                }
            }
        }
    }
    
    return BackendType::Unknown;
}

BackendType GraphicsDriverFactory::NormalizeBackendName(const std::string& backendName) {
    std::string normalized = backendName;
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    
    // Handle variations
    if (normalized == "vulkan" || normalized == "vk") {
        return BackendType::Vulkan;
    }
    if (normalized == "opengl" || normalized == "gl" || normalized == "gles") {
        return BackendType::OpenGL;
    }
    if (normalized == "directx12" || normalized == "directx" || normalized == "dx12" || 
        normalized == "d3d12" || normalized == "d3d") {
        return BackendType::DirectX12;
    }
    if (normalized == "metal" || normalized == "mtl") {
        return BackendType::Metal;
    }
    if (normalized == "software" || normalized == "cpu" || normalized == "sw") {
        return BackendType::Software;
    }
    
    return BackendType::Unknown;
}

std::string GraphicsDriverFactory::BackendTypeToString(BackendType backendType) {
    switch (backendType) {
        case BackendType::Vulkan:
            return "vulkan";
        case BackendType::OpenGL:
            return "opengl";
        case BackendType::DirectX12:
            return "directx12";
        case BackendType::Metal:
            return "metal";
        case BackendType::Software:
            return "software";
        case BackendType::Unknown:
        default:
            return "unknown";
    }
}

BackendType GraphicsDriverFactory::GetPlatformDefault() {
    // Phase 41: Vulkan is the default on all platforms
    // This can be overridden by environment or config
    return BackendType::Vulkan;
}

} // namespace Graphics
