#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace Graphics {

// Forward declarations - NO backend-specific includes in this header
class IGraphicsDriver;
class VulkanGraphicsDriver;
class OpenGLGraphicsDriver;
class DirectX12GraphicsDriver;
class MetalGraphicsDriver;
class SoftwareGraphicsDriver;

enum class BackendType;

// ============================================================================
// GRAPHICS DRIVER FACTORY
// ============================================================================

class GraphicsDriverFactory {
public:
    // Deleted copy/move constructors - static factory only
    GraphicsDriverFactory() = delete;
    GraphicsDriverFactory(const GraphicsDriverFactory&) = delete;
    GraphicsDriverFactory& operator=(const GraphicsDriverFactory&) = delete;
    GraphicsDriverFactory(GraphicsDriverFactory&&) = delete;
    GraphicsDriverFactory& operator=(GraphicsDriverFactory&&) = delete;

    // ========================================================================
    // MAIN FACTORY INTERFACE
    // ========================================================================

    /**
     * Create a graphics driver instance
     * 
     * Backend selection priority:
     * 1. Explicit parameter (if not unknown)
     * 2. GRAPHICS_DRIVER environment variable
     * 3. ~/.generalsX/graphics.ini [Graphics] Driver= key
     * 4. Platform default (Vulkan on all platforms for Phase 41)
     * 
     * @param backendType Explicitly requested backend (BackendType::Unknown for auto-select)
     * @param windowHandle Platform-specific window handle (HWND on Windows, NSWindow* on macOS, etc)
     * @param width Display width in pixels
     * @param height Display height in pixels
     * @param fullscreen Full-screen mode flag
     * @return IGraphicsDriver* Allocated driver instance, or nullptr on failure
     *         Caller owns the returned pointer - use DestroyDriver() to free
     */
    static IGraphicsDriver* CreateDriver(BackendType backendType, void* windowHandle,
                                        uint32_t width, uint32_t height, bool fullscreen);

    /**
     * Destroy a graphics driver instance
     * 
     * @param driver Driver pointer returned from CreateDriver()
     */
    static void DestroyDriver(IGraphicsDriver* driver);

    // ========================================================================
    // BACKEND INFORMATION QUERIES
    // ========================================================================

    /**
     * Check if a backend is supported on this platform
     * 
     * @param backendType Backend to check
     * @return true if backend is available/compiled in, false otherwise
     */
    static bool IsSupportedBackend(BackendType backendType);

    /**
     * Get list of all supported backends for this platform
     * 
     * @return Vector of BackendType values representing supported backends
     */
    static std::vector<BackendType> GetSupportedBackends();

    /**
     * Get human-readable display name for a backend
     * 
     * @param backendType Backend type
     * @return String like "Vulkan", "OpenGL", "Metal", etc
     */
    static std::string GetBackendDisplayName(BackendType backendType);

    /**
     * Get the name of the currently selected/active backend
     * 
     * @return String like "vulkan", "opengl", "metal", "directx12"
     */
    static std::string GetSelectedBackendName();

    /**
     * Get the default backend name for this platform
     * 
     * Default priority:
     * 1. GRAPHICS_DRIVER environment variable
     * 2. ~/.generalsX/graphics.ini [Graphics] Driver= key
     * 3. Platform default (Vulkan on all platforms for Phase 41)
     * 
     * @return String like "vulkan", "opengl", etc
     */
    static std::string GetDefaultBackendName();

    // ========================================================================
    // BACKEND-SPECIFIC QUERIES
    // ========================================================================

    /**
     * Check if Vulkan is available
     * @return true if Vulkan backend is compiled and supported
     */
    static bool IsVulkanAvailable();

    /**
     * Check if OpenGL is available
     * @return true if OpenGL backend is compiled and supported
     */
    static bool IsOpenGLAvailable();

    /**
     * Check if DirectX 12 is available
     * @return true if DirectX 12 backend is compiled and supported (Windows only)
     */
    static bool IsDirectX12Available();

    /**
     * Check if Metal is available
     * @return true if Metal backend is compiled and supported (macOS/iOS only)
     */
    static bool IsMetalAvailable();

    // ========================================================================
    // HELPER METHODS (Internal Use)
    // ========================================================================

private:
    /**
     * Get backend type from GRAPHICS_DRIVER environment variable
     * 
     * @return BackendType from env var, or BackendType::Unknown if not set
     */
    static BackendType GetBackendFromEnvironment();

    /**
     * Get backend type from ~/.generalsX/graphics.ini configuration file
     * 
     * @return BackendType from config, or BackendType::Unknown if not found/invalid
     */
    static BackendType GetBackendFromConfig();

    /**
     * Normalize backend name string to BackendType enum
     * 
     * Accepts: "vulkan", "opengl", "directx12", "dx12", "metal", "software"
     * Case-insensitive, handles variations
     * 
     * @param backendName String name to normalize
     * @return Corresponding BackendType, or BackendType::Unknown if unrecognized
     */
    static BackendType NormalizeBackendName(const std::string& backendName);

    /**
     * Convert BackendType enum to normalized lowercase string
     * 
     * @param backendType Backend type to convert
     * @return String like "vulkan", "opengl", "metal", "directx12", "software"
     */
    static std::string BackendTypeToString(BackendType backendType);

    /**
     * Get the default backend for current platform
     * 
     * Phase 41 default: Vulkan on all platforms
     * 
     * @return BackendType::Vulkan (current standard)
     */
    static BackendType GetPlatformDefault();
};

} // namespace Graphics
