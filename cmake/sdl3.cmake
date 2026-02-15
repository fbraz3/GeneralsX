# SDL3 windowing/input library for Linux builds
# GeneralsX @build BenderAI 11/02/2026 - Session 26
# SDL3 provides cross-platform windowing, input events, and OS integration
# Used by SDL3GameEngine (replaces Win32 window management on Linux)
#
# Fighter19 pattern: Use find_package (expects system install)
# Our approach: Use FetchContent to download and compile SDL3 directly
# This avoids vcpkg issues with libsystemd and complex dependencies

if(SAGE_USE_SDL3)
    message(STATUS "Fetching SDL3 from GitHub...")
    
    # Download SDL3 release from official repository
    # Using preview release 3.1.6 (same as vcpkg baseline)
    FetchContent_Declare(
        sdl3
        URL https://github.com/libsdl-org/SDL/releases/download/preview-3.1.6/SDL3-3.1.6.tar.gz
        URL_HASH SHA256=d177d6727669609c92069b377281c7d5eec587f45b3893bbd0a35b2388dc42ad
    )
    
    # Configure SDL3 options (minimal feature set for our needs)
    # Disable unnecessary features to speed up compilation
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    
    # GeneralsX @build BenderAI 13/02/2026 - Enable VIDEO and VULKAN for graphics support
    # VIDEO must be ON to use Vulkan (SDL_Vulkan_LoadLibrary requires video driver)
    set(SDL_VIDEO ON CACHE BOOL "" FORCE)
    set(SDL_VULKAN ON CACHE BOOL "" FORCE)
    
    # Graphics driver selection (Vulkan is primary, fallback to software)
    set(SDL_VIDEO_VULKAN ON CACHE BOOL "" FORCE)
    set(SDL_VIDEO_KMSDRM ON CACHE BOOL "" FORCE)  # Linux KMS/DRM driver (headless support)
    
    # Disable legacy/unused drivers for faster compilation
    set(SDL_X11 OFF CACHE BOOL "" FORCE)
    set(SDL_WAYLAND OFF CACHE BOOL "" FORCE)
    set(SDL_UNIX_CONSOLE_BUILD ON CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(sdl3)
    
    message(STATUS "SDL3 fetched and configured (${${sdl3_SOURCE_DIR}})")
    
    # Create interface library to wrap SDL3
    # Makes linking easier for targets that need SDL3
    add_library(sdl3lib INTERFACE)
    
    # Link SDL3::SDL3-shared for runtime dynamically linked library
    # Fighter19 uses shared libs to avoid static linking bloat
    target_link_libraries(sdl3lib INTERFACE SDL3::SDL3-shared)
endif()
