/*
 * Phase 09: GraphicsDeviceFactory Implementation
 */

#include "GraphicsDevice/GraphicsDeviceFactory.h"
#include "GraphicsDevice/GraphicsDevice.h"

namespace GeneralsX {
namespace Graphics {

// Forward declarations
class GraphicsDeviceVulkan;
class GraphicsDeviceDX8;

GraphicsDevice* GraphicsDeviceFactory::create(GraphicsBackend backend) {
    // Auto-detect best backend
    if (backend == GRAPHICS_BACKEND_AUTO) {
        backend = getRecommendedBackend();
    }

    GraphicsDevice* device = nullptr;

    switch (backend) {
    case GRAPHICS_BACKEND_VULKAN:
        // Create Vulkan implementation
        // TODO: Implement GraphicsDeviceVulkan in Phase 09.3
        // device = new GraphicsDeviceVulkan();
        break;

    case GRAPHICS_BACKEND_DX8:
        // Create DirectX 8 fallback
        // TODO: Implement GraphicsDeviceDX8 wrapper in Phase 09.3
        // device = new GraphicsDeviceDX8();
        break;

    default:
        break;
    }

    return device;
}

GraphicsBackend GraphicsDeviceFactory::getRecommendedBackend() {
    // For Windows with proper Vulkan drivers, prefer Vulkan
    // Otherwise fall back to DirectX 8 for compatibility
#ifdef _WIN32
    // Check if Vulkan is available
    if (isBackendSupported(GRAPHICS_BACKEND_VULKAN)) {
        return GRAPHICS_BACKEND_VULKAN;
    }
    return GRAPHICS_BACKEND_DX8;
#else
    // On other platforms (Wine, macOS, Linux), use Vulkan
    return GRAPHICS_BACKEND_VULKAN;
#endif
}

bool GraphicsDeviceFactory::isBackendSupported(GraphicsBackend backend) {
    switch (backend) {
    case GRAPHICS_BACKEND_VULKAN:
        // TODO: Check Vulkan driver availability
        return true;  // Assume available for now

    case GRAPHICS_BACKEND_DX8:
#ifdef _WIN32
        return true;  // Always available on Windows
#else
        return false; // DirectX not available on non-Windows
#endif

    default:
        return false;
    }
}

const char* GraphicsDeviceFactory::getBackendName(GraphicsBackend backend) {
    switch (backend) {
    case GRAPHICS_BACKEND_VULKAN:
        return "Vulkan";
    case GRAPHICS_BACKEND_DX8:
        return "DirectX 8";
    case GRAPHICS_BACKEND_AUTO:
        return "Auto-detect";
    default:
        return "Unknown";
    }
}

} // namespace Graphics
} // namespace GeneralsX
