# SDL3 windowing/input library for Linux builds
# GeneralsX @build BenderAI 11/02/2026 - Session 26
# SDL3 provides cross-platform windowing, input events, and OS integration
# Used by SDL3GameEngine (replaces Win32 window management on Linux)
#
# Fighter19 pattern: Use find_package (expects system install)
# Our approach: Use FetchContent to download and compile SDL3 directly
# This avoids vcpkg issues with libsystemd and complex dependencies

if(SAGE_USE_SDL3)
    # GeneralsX @build BenderAI 22/02/2026 (updated)
    # Strategy: FetchContent to compile SDL3 + SDL3_image from source
    # Docker environment (ubuntu:24.04) has build dependencies pre-installed
    # This ensures local build compatibility (same glibc, same distro as developer machine)
    # Reference: https://github.com/libsdl-org/SDL/releases/download/release-3.4.2/SDL3-3.4.2.tar.gz
    
    message(STATUS "Configuring SDL3 (v3.4.2) with FetchContent (native build)...")
    
    include(FetchContent)
    
    # SDL3 - Core graphics, input, events, filesystem
    set(SDL3_VERSION "3.4.2")
    set(SDL3_URL "https://github.com/libsdl-org/SDL/releases/download/release-${SDL3_VERSION}/SDL3-${SDL3_VERSION}.tar.gz")
    set(SDL3_URL_HASH "SHA256=ef39a2e3f9a8a78296c40da701967dd1b0d0d6e267e483863ce70f8a03b4050c")
    
    FetchContent_Declare(
        SDL3
        URL ${SDL3_URL}
        URL_HASH ${SDL3_URL_HASH}
    )
    
    # Configure SDL3 build options
    set(SDL_SHARED ON CACHE BOOL "Build SDL3 as shared library" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "Don't build static library" FORCE)
    set(SDL_AUDIO ON CACHE BOOL "Enable audio subsystem" FORCE)
    set(SDL_TIMERS ON CACHE BOOL "Enable timers" FORCE)
    set(SDL_EVENTS ON CACHE BOOL "Enable events" FORCE)
    set(SDL_FILESYSTEM ON CACHE BOOL "Enable filesystem" FORCE)
    set(SDL_RENDER ON CACHE BOOL "Enable render subsystem" FORCE)
    set(SDL_VIDEO ON CACHE BOOL "Enable video subsystem" FORCE)
    
    # Platform support
    if(NOT WIN32)
        # Linux: Wayland and X11 support
        set(SDL_WAYLAND ON CACHE BOOL "Enable Wayland support (Linux)" FORCE)
        set(SDL_X11 ON CACHE BOOL "Enable X11 support (Linux)" FORCE)
    else()
        # Windows: DirectX and WASAPI support
        set(SDL_DIRECTX ON CACHE BOOL "Enable DirectX support (Windows)" FORCE)
        set(SDL_WASAPI ON CACHE BOOL "Enable WASAPI audio (Windows)" FORCE)
        set(SDL_WAYLAND OFF CACHE BOOL "Disable Wayland (Windows)" FORCE)
        set(SDL_X11 OFF CACHE BOOL "Disable X11 (Windows)" FORCE)
        
        # GeneralsX @build BenderAI 27/02/2026
        # WASAPI requires WinRT headers (roapi.h) which are in Windows SDK winrt\ folder
        # Add winrt include path for SDL3 compilation
        if(MSVC)
            # Find Windows SDK version and add winrt headers
            get_filename_component(WINDOWS_SDK_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
            if(WINDOWS_SDK_ROOT)
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I\"${WINDOWS_SDK_ROOT}/Include/10.0.26100.0/winrt\"")
                message(STATUS "Added Windows SDK WinRT headers: ${WINDOWS_SDK_ROOT}/Include/10.0.26100.0/winrt")
            endif()
        endif()
    endif()
    
    set(SDL_CAMERA OFF CACHE BOOL "Disable camera (unused)" FORCE)
    set(SDL_QSPI OFF CACHE BOOL "Disable QSPI (unused)" FORCE)
    
    FetchContent_MakeAvailable(SDL3)
    
    # GeneralsX @build BenderAI 27/02/2026 (updated for Windows compat)
    # SDL3_image is only needed on Linux (PNG/JPG cursor loading, X cursor support)
    # On Windows: DirectX handles image loading; skip SDL3_image to avoid libpng dependency
    if(NOT WIN32)
        # Linux: Use system libpng-dev
        set(PNG_SHARED ON CACHE BOOL "Require PNG as shared library" FORCE)
        set(PNG_INCLUDE_DIR "/usr/include" CACHE PATH "PNG include dir (system)" FORCE)
        set(PNG_LIBRARY "/usr/lib/x86_64-linux-gnu/libpng16.so.16" CACHE FILEPATH "PNG library (system .so)" FORCE)
        set(PNG_LIBRARY_DEBUG "/usr/lib/x86_64-linux-gnu/libpng16.so.16" CACHE FILEPATH "PNG debug library" FORCE)
        set(PNG_LIBRARY_RELEASE "/usr/lib/x86_64-linux-gnu/libpng16.so.16" CACHE FILEPATH "PNG release library" FORCE)
        set(ENV{PKG_CONFIG_PATH} "/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
        find_package(PNG REQUIRED MODULE)

        # SDL3_image - Image format support (PNG, JPG for cursor ANI loading, Linux only)
        message(STATUS "Configuring SDL3_image (v3.4.0) with FetchContent (Linux build)...")

        set(SDL3_IMAGE_VERSION "3.4.0")
        set(SDL3_IMAGE_URL "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL3_IMAGE_VERSION}/SDL3_image-${SDL3_IMAGE_VERSION}.tar.gz")
        set(SDL3_IMAGE_URL_HASH "SHA256=2ceb75eab4235c2c7e93dafc3ef3268ad368ca5de40892bf8cffdd510f29d9d8")

        FetchContent_Declare(
            SDL3_image
            URL ${SDL3_IMAGE_URL}
            URL_HASH ${SDL3_IMAGE_URL_HASH}
        )

        set(SDL3IMAGE_INSTALL ON CACHE BOOL "Install SDL3_image" FORCE)
        set(SDL3IMAGE_DEPS_SHARED ON CACHE BOOL "Use system shared dependencies" FORCE)
        set(SDL3IMAGE_JPG ON CACHE BOOL "Enable JPG support" FORCE)
        set(SDL3IMAGE_PNG ON CACHE BOOL "Enable PNG support (ANI cursor loading)" FORCE)
        set(SDL3IMAGE_TIF ON CACHE BOOL "Enable TIF support" FORCE)
        set(SDL3IMAGE_WEBP ON CACHE BOOL "Enable WebP support" FORCE)
        set(SDL3IMAGE_AVIF OFF CACHE BOOL "Disable AVIF (optional)" FORCE)
        set(SDL3IMAGE_XCUR ON CACHE BOOL "Enable X cursor support" FORCE)

        FetchContent_MakeAvailable(SDL3_image)
    endif()

    # Create unified interface library for linking
    add_library(sdl3lib INTERFACE)
    if(NOT WIN32)
        target_link_libraries(sdl3lib INTERFACE SDL3::SDL3 SDL3_image::SDL3_image)
        target_include_directories(sdl3lib INTERFACE
            "${SDL3_SOURCE_DIR}/include"
            "${sdl3_image_SOURCE_DIR}/include"
        )
        message(STATUS "SDL3 (${SDL3_VERSION}) + SDL3_image (${SDL3_IMAGE_VERSION}) configured (Linux)")
    else()
        target_link_libraries(sdl3lib INTERFACE SDL3::SDL3)
        target_include_directories(sdl3lib INTERFACE
            "${SDL3_SOURCE_DIR}/include"
        )
        message(STATUS "SDL3 (${SDL3_VERSION}) configured (Windows, no SDL3_image)")
    endif()
    message(STATUS "  Build approach: Native FetchContent compilation")
    
endif()
