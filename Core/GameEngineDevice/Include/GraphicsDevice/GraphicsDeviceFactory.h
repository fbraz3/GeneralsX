/*
 * Phase 09: GraphicsDeviceFactory - Factory for creating graphics device instances
 * 
 * Provides abstraction for device creation, enabling runtime selection of
 * graphics backend (Vulkan primary, DirectX 8 fallback for Windows).
 */

#ifndef GRAPHICSDEVICEFACTORY_H
#define GRAPHICSDEVICEFACTORY_H

namespace GeneralsX {
namespace Graphics {

// Forward declaration
class GraphicsDevice;

/**
 * @brief Graphics device backend type enumeration
 */
enum GraphicsBackend {
    GRAPHICS_BACKEND_AUTO,          // Auto-detect best backend
    GRAPHICS_BACKEND_VULKAN,        // Vulkan (primary, cross-platform)
    GRAPHICS_BACKEND_DX8,           // DirectX 8 (Windows only, fallback)
};

/**
 * @brief Factory for creating graphics device instances
 * 
 * Provides static methods to instantiate graphics devices of various types.
 * Encapsulates backend-specific device creation logic.
 */
class GraphicsDeviceFactory {
public:
    /**
     * Create graphics device with specified backend
     * @param backend Backend type to create
     * @return Pointer to GraphicsDevice instance, nullptr if creation failed
     * 
     * Caller is responsible for deleting returned pointer
     * Use:
     *   GraphicsDevice* device = GraphicsDeviceFactory::create(GRAPHICS_BACKEND_VULKAN);
     *   delete device;
     */
    static GraphicsDevice* create(GraphicsBackend backend = GRAPHICS_BACKEND_AUTO);

    /**
     * Get recommended backend for current system
     * @return Recommended GraphicsBackend for platform
     * 
     * Returns best-supported backend:
     * - Windows with Vulkan: GRAPHICS_BACKEND_VULKAN
     * - Windows without Vulkan: GRAPHICS_BACKEND_DX8
     * - Wine/Linux: GRAPHICS_BACKEND_VULKAN
     * - macOS: GRAPHICS_BACKEND_VULKAN
     */
    static GraphicsBackend getRecommendedBackend();

    /**
     * Check if backend is supported on current system
     * @param backend Backend type to check
     * @return true if backend is available
     */
    static bool isBackendSupported(GraphicsBackend backend);

    /**
     * Get human-readable backend name
     * @param backend Backend type
     * @return Backend name string (e.g., "Vulkan", "DirectX 8")
     */
    static const char* getBackendName(GraphicsBackend backend);

private:
    // Private constructor - factory only
    GraphicsDeviceFactory() {}
};

} // namespace Graphics
} // namespace GeneralsX

#endif // GRAPHICSDEVICEFACTORY_H
