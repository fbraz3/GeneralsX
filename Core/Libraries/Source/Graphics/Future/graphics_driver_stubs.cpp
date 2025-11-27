/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * Graphics Driver Factory Stubs for Future Backends
 * 
 * These stub implementations allow the factory pattern to gracefully handle
 * backend requests that are not yet implemented. When a future backend is
 * requested (OpenGL, Metal, DirectX12), these stubs return nullptr with
 * appropriate logging.
 * 
 * These stubs allow:
 * - Graceful degradation when backends aren't available
 * - Structure for future backend implementation
 * - Proper error reporting to game code
 * 
 * NOTE: extern "C" functions MUST be outside namespace for proper linkage!
 */

#include "../IGraphicsDriver.h"
#include <cstdio>

// ============================================================================
// OpenGL Graphics Driver Factory Stub (Phase 50+)
// ============================================================================

/**
 * Factory function for creating OpenGL graphics driver instances.
 * 
 * @param windowHandle Platform-specific window handle
 * @param width Display width in pixels
 * @param height Display height in pixels
 * @param fullscreen Full-screen mode flag
 * @return New IGraphicsDriver instance (OpenGL implementation), or nullptr
 * @note NOT YET IMPLEMENTED - returns nullptr
 */
extern "C" Graphics::IGraphicsDriver* CreateOpenGLGraphicsDriver(void* windowHandle, uint32_t width,
                                                                uint32_t height, bool fullscreen)
{
    printf("[CreateOpenGLGraphicsDriver] OpenGL backend NOT YET IMPLEMENTED (Phase 50+)\n");
    printf("[CreateOpenGLGraphicsDriver] Falling back to default backend\n");
    return nullptr;  // Signals factory to use fallback
}

// ============================================================================
// Metal Graphics Driver Factory Stub (Phase 50+)
// ============================================================================

/**
 * Factory function for creating Metal graphics driver instances.
 * 
 * @param windowHandle Platform-specific window handle (NSView*)
 * @param width Display width in pixels
 * @param height Display height in pixels
 * @param fullscreen Full-screen mode flag
 * @return New IGraphicsDriver instance (Metal implementation), or nullptr
 * @note NOT YET IMPLEMENTED - returns nullptr
 */
extern "C" Graphics::IGraphicsDriver* CreateMetalGraphicsDriver(void* windowHandle, uint32_t width,
                                                               uint32_t height, bool fullscreen)
{
    printf("[CreateMetalGraphicsDriver] Metal backend NOT YET IMPLEMENTED (Phase 50+)\n");
    printf("[CreateMetalGraphicsDriver] Metal is macOS/iOS only, requires platform-specific setup\n");
    printf("[CreateMetalGraphicsDriver] Falling back to default backend\n");
    return nullptr;  // Signals factory to use fallback
}

// ============================================================================
// DirectX 12 Graphics Driver Factory Stub (Phase 50+)
// ============================================================================

/**
 * Factory function for creating DirectX 12 graphics driver instances.
 * 
 * @param windowHandle Platform-specific window handle (HWND)
 * @param width Display width in pixels
 * @param height Display height in pixels
 * @param fullscreen Full-screen mode flag
 * @return New IGraphicsDriver instance (DirectX 12 implementation), or nullptr
 * @note NOT YET IMPLEMENTED - returns nullptr
 */
extern "C" Graphics::IGraphicsDriver* CreateDirectX12GraphicsDriver(void* windowHandle, uint32_t width,
                                                                   uint32_t height, bool fullscreen)
{
    printf("[CreateDirectX12GraphicsDriver] DirectX 12 backend NOT YET IMPLEMENTED (Phase 50+)\n");
    printf("[CreateDirectX12GraphicsDriver] DirectX 12 is Windows only\n");
    printf("[CreateDirectX12GraphicsDriver] Falling back to default backend\n");
    return nullptr;  // Signals factory to use fallback
}

// ============================================================================
// Software Graphics Driver Factory Stub (Future - Fallback)
// ============================================================================

/**
 * Factory function for creating Software graphics driver instances.
 * 
 * Pure CPU rendering - used as absolute fallback when no GPU backend
 * is available. Extremely slow but allows basic testing.
 * 
 * @param windowHandle Platform-specific window handle
 * @param width Display width in pixels
 * @param height Display height in pixels
 * @param fullscreen Full-screen mode flag
 * @return New IGraphicsDriver instance (Software implementation), or nullptr
 * @note NOT YET IMPLEMENTED - returns nullptr
 */
extern "C" Graphics::IGraphicsDriver* CreateSoftwareGraphicsDriver(void* windowHandle, uint32_t width,
                                                                  uint32_t height, bool fullscreen)
{
    printf("[CreateSoftwareGraphicsDriver] Software backend NOT YET IMPLEMENTED\n");
    printf("[CreateSoftwareGraphicsDriver] Software rendering is CPU-only, not recommended\n");
    printf("[CreateSoftwareGraphicsDriver] Falling back to default backend\n");
    return nullptr;  // Signals factory to use fallback
}
