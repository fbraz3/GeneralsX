################################################################################
# Vulkan Graphics Backend Support for GeneralsX
#
# This CMake module configures Vulkan support following MoltenVK best practices:
#
# Recommended Usage Model (from MoltenVK Documentation):
# =====================================================
# For desktop, iOS, and iPadOS applications, use MoltenVK dynamic library in
# conjunction with the Vulkan Loader (NOT direct linking to MoltenVK):
#
#   ✅ RECOMMENDED:
#      - Link to Vulkan Loader (find via Vulkan::Loader)
#      - MoltenVK acts as ICD (Installable Client Driver)
#      - Enables Vulkan Validation Layers
#      - Better debugging and portability
#
#   ❌ NOT RECOMMENDED:
#      - Direct linking to MoltenVK static library
#      - Sacrifices validation layer support
#      - Reduces portability across platforms
#
# Runtime Configuration:
# ======================
# macOS:  Vulkan Loader + MoltenVK distributed in app bundle (or from SDK)
# Linux:  Vulkan Loader + native Vulkan ICD (Mesa/AMD/NVIDIA)
# Windows: Vulkan Loader + native Vulkan ICD (AMD/NVIDIA/Intel)
#
# Phase: 39.2.2 (CMake Integration)
# Created: October 30, 2025
################################################################################

# ============================================================================
# Enable Vulkan Support Option
# ============================================================================

option(USE_DXVK "Enable Vulkan graphics backend (DXVKGraphicsBackend)" ON)
message(STATUS "Vulkan support: ${USE_DXVK}")

if(NOT USE_DXVK)
    return()  # Exit early if Vulkan support disabled
endif()

# ============================================================================
# Find Vulkan SDK
# ============================================================================

# Vulkan SDK detection order:
# 1. VULKAN_SDK environment variable (explicit user override)
# 2. Standard system paths (macOS: /usr/local/share/vulkan, Linux: /usr, Windows: registry)
# 3. CMake's built-in Vulkan module (VulkanHeaders, Vulkan-Loader)

if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
    message(STATUS "Using Vulkan SDK from environment: ${VULKAN_SDK_PATH}")
else()
    # Try to find from system
    if(APPLE)
        # macOS: Vulkan SDK typically installed to /usr/local/share/vulkan
        set(VULKAN_SDK_PATH "/usr/local/share/vulkan")
    endif()
endif()

# Find Vulkan package (provides Vulkan::Vulkan and Vulkan::Loader)
find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK from https://vulkan.lunarg.com/")
endif()

message(STATUS "Vulkan SDK found:")
message(STATUS "  - Version: ${Vulkan_VERSION}")
message(STATUS "  - Headers: ${Vulkan_INCLUDE_DIR}")
message(STATUS "  - Loader: ${Vulkan_LOADER_LIBRARY}")

# ============================================================================
# Platform-Specific Vulkan Configuration
# ============================================================================

if(APPLE)
    # macOS: MoltenVK support
    # ========================
    # MoltenVK is the Installable Client Driver (ICD) for Vulkan on macOS
    # It translates Vulkan calls to Metal API calls
    
    message(STATUS "Configuring Vulkan for macOS (via MoltenVK)")
    
    # Find MoltenVK (included with Vulkan SDK on macOS)
    find_package(MoltenVK QUIET)
    
    if(MoltenVK_FOUND)
        message(STATUS "  ✅ MoltenVK found: ${MoltenVK_LIBRARY}")
        set(VULKAN_FRAMEWORKS
            ${MoltenVK_LIBRARY}
            "-framework Metal"
            "-framework IOKit"
            "-framework IOSurface"
            "-framework QuartzCore"
        )
        message(STATUS "  - Metal framework linking enabled")
    else()
        message(STATUS "  ⚠️  MoltenVK not found in SDK, relying on system installation")
        message(STATUS "  - Ensure MoltenVK is in your DYLD_LIBRARY_PATH or app bundle")
    endif()
    
    # Additional macOS-specific Vulkan extensions
    set(VULKAN_PLATFORM_EXTENSIONS
        VK_EXT_METAL_SURFACE_EXTENSION_NAME      # Create Metal surfaces
        VK_EXT_METAL_OBJECTS_EXTENSION_NAME      # Mixed Vulkan/Metal (optional)
    )
    
    # macOS architecture support (Apple Silicon and Intel)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        message(STATUS "  - ARM64 (Apple Silicon) detected")
        set(VULKAN_ARCH "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        message(STATUS "  - x86_64 (Intel) detected")
        set(VULKAN_ARCH "x86_64")
    endif()

elseif(UNIX AND NOT APPLE)
    # Linux: Native Vulkan support
    # =============================
    message(STATUS "Configuring Vulkan for Linux")
    
    # Linux uses native Vulkan ICDs (Mesa, AMD, NVIDIA)
    # No additional framework linking needed
    
    set(VULKAN_PLATFORM_EXTENSIONS
        VK_KHR_XCB_SURFACE_EXTENSION_NAME        # X11 (XCB)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME       # X11 (Xlib)
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME    # Wayland
    )
    
    # Detect available display servers
    if(WAYLAND_FOUND)
        message(STATUS "  - Wayland support detected")
    endif()
    if(X11_FOUND)
        message(STATUS "  - X11 (XCB/Xlib) support detected")
    endif()

elseif(WIN32)
    # Windows: Native Vulkan support
    # ================================
    message(STATUS "Configuring Vulkan for Windows")
    
    # Windows uses native Vulkan ICDs (AMD, NVIDIA, Intel)
    # No additional linking needed
    
    set(VULKAN_PLATFORM_EXTENSIONS
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME      # Win32 surface creation
    )

endif()

# ============================================================================
# Vulkan Loader Configuration (Recommended Model)
# ============================================================================
#
# The Vulkan Loader is the recommended way to use Vulkan on all platforms.
# It provides:
# - ICD (Installable Client Driver) discovery and management
# - Validation layer support (critical for debugging)
# - Platform independence
#
# Target: Vulkan::Loader
# This is what we link to, NOT MoltenVK directly
# ============================================================================

# Verify Vulkan Loader is available
if(NOT TARGET Vulkan::Loader)
    message(WARNING "Vulkan::Loader target not found. Falling back to Vulkan::Vulkan")
    # Fallback: Use Vulkan::Vulkan which includes both loader and headers
    set(VULKAN_LOADER_LIBRARY Vulkan::Vulkan)
else()
    set(VULKAN_LOADER_LIBRARY Vulkan::Loader)
endif()

# ============================================================================
# Validation Layers Support
# ============================================================================
#
# For development and debugging, we use Vulkan Validation Layers
# These are NOT included in production builds
#
# During development:
# - Vulkan SDK provides validation layers in standard locations
# - Automatically discovered by Vulkan Loader
# - Controlled by environment variables and config files
#
# Production:
# - Validation layers can be stripped
# - Shader compilation time is reduced
# - Performance is improved
# ============================================================================

option(VULKAN_VALIDATION_LAYERS "Enable Vulkan Validation Layers (debug builds only)" ON)

if(VULKAN_VALIDATION_LAYERS AND (CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo"))
    message(STATUS "Vulkan Validation Layers: ENABLED (debug build)")
    add_compile_definitions(VULKAN_VALIDATION_ENABLED=1)
else()
    message(STATUS "Vulkan Validation Layers: DISABLED (release build)")
    add_compile_definitions(VULKAN_VALIDATION_ENABLED=0)
endif()

# ============================================================================
# Vulkan SDK Installation Paths
# ============================================================================
#
# These paths are used for shader compilation and validation layer discovery
# ============================================================================

if(APPLE)
    # macOS Vulkan SDK path
    if(NOT DEFINED VULKAN_SDK_PATH)
        set(VULKAN_SDK_PATH "/usr/local/share/vulkan")
    endif()
    
    set(VULKAN_GLSLC "${VULKAN_SDK_PATH}/bin/glslc")
    set(VULKAN_LAYER_PATH "${VULKAN_SDK_PATH}/etc/vulkan/explicit_layer.d")
    set(VULKAN_ICD_PATH "${VULKAN_SDK_PATH}/etc/vulkan/icd.d")
    
elseif(UNIX AND NOT APPLE)
    # Linux Vulkan SDK path
    if(NOT DEFINED VULKAN_SDK_PATH)
        set(VULKAN_SDK_PATH "/usr")
    endif()
    
    set(VULKAN_GLSLC "${VULKAN_SDK_PATH}/bin/glslc")
    set(VULKAN_LAYER_PATH "${VULKAN_SDK_PATH}/share/vulkan/explicit_layer.d")
    set(VULKAN_ICD_PATH "${VULKAN_SDK_PATH}/share/vulkan/icd.d")
    
elseif(WIN32)
    # Windows Vulkan SDK path
    if(DEFINED ENV{VULKAN_SDK})
        set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
    else()
        # Default installation paths
        set(VULKAN_SDK_PATH "C:/VulkanSDK")
    endif()
    
    set(VULKAN_GLSLC "${VULKAN_SDK_PATH}/Bin/glslc.exe")
    set(VULKAN_LAYER_PATH "${VULKAN_SDK_PATH}/etc/vulkan/explicit_layer.d")
    set(VULKAN_ICD_PATH "${VULKAN_SDK_PATH}/etc/vulkan/icd.d")
    
endif()

# ============================================================================
# Shader Compilation Support
# ============================================================================
#
# Shaders need to be compiled from GLSL to SPIR-V bytecode
# glslc is provided by Vulkan SDK
# ============================================================================

if(EXISTS "${VULKAN_GLSLC}")
    message(STATUS "GLSL Shader Compiler found: ${VULKAN_GLSLC}")
    set(VULKAN_SHADER_COMPILER_AVAILABLE TRUE)
    
    # Helper function to compile shaders
    function(vulkan_compile_shader shader_source shader_spirv)
        add_custom_command(
            OUTPUT "${shader_spirv}"
            COMMAND "${VULKAN_GLSLC}" -o "${shader_spirv}" "${shader_source}"
            DEPENDS "${shader_source}"
            COMMENT "Compiling shader: ${shader_source}"
            VERBATIM
        )
    endfunction()
    
else()
    message(WARNING "GLSL Shader Compiler (glslc) not found at ${VULKAN_GLSLC}")
    message(WARNING "Shader compilation will fail. Ensure Vulkan SDK is installed:")
    message(WARNING "  macOS:   https://vulkan.lunarg.com/doc/sdk/1.4.328.1/mac/")
    message(WARNING "  Linux:   https://vulkan.lunarg.com/doc/sdk/1.4.328.1/linux/")
    message(WARNING "  Windows: https://vulkan.lunarg.com/doc/sdk/1.4.328.1/windows/")
    set(VULKAN_SHADER_COMPILER_AVAILABLE FALSE)
endif()

# ============================================================================
# MoltenVK-Specific Recommendations
# ============================================================================
#
# For macOS deployment:
#
# Development (Recommended):
# --------------------------
# 1. Link to Vulkan::Loader only (NOT MoltenVK)
# 2. System-wide Vulkan SDK provides MoltenVK ICD
# 3. Enable Validation Layers for debugging
# 4. Use vkconfig to manage validation layer settings
#
# Production (Shipping):
# ----------------------
# 1. Embed Vulkan Loader + MoltenVK in app bundle
# 2. Set up proper rpath for embedded libraries
# 3. Disable validation layers for performance
# 4. Optionally mix Vulkan + Metal using VK_EXT_metal_objects
#
# ============================================================================

message(STATUS "")
message(STATUS "Vulkan Configuration Summary:")
message(STATUS "  - Backend: DXVKGraphicsBackend (Vulkan wrapper)")
message(STATUS "  - Loader: Vulkan::Loader (recommended model)")
if(APPLE)
    message(STATUS "  - Platform: macOS (MoltenVK)")
    message(STATUS "  - Architecture: ${VULKAN_ARCH}")
elseif(UNIX)
    message(STATUS "  - Platform: Linux (native Vulkan)")
elseif(WIN32)
    message(STATUS "  - Platform: Windows (native Vulkan)")
endif()
message(STATUS "  - Validation: ${VULKAN_VALIDATION_LAYERS}")
message(STATUS "  - Shader Compiler: ${VULKAN_SHADER_COMPILER_AVAILABLE}")
message(STATUS "")

# ============================================================================
# Export Variables for Target Configuration
# ============================================================================

# These variables can be used by other CMake files to configure targets

set(VULKAN_AVAILABLE TRUE CACHE BOOL "Vulkan support available")
set(VULKAN_INCLUDE_DIRS ${Vulkan_INCLUDE_DIR})
# VULKAN_LOADER_LIBRARY already set above (either Vulkan::Loader or Vulkan::Vulkan)
set(VULKAN_LIBRARIES Vulkan::Vulkan)
set(VULKAN_COMPILE_DEFINITIONS
    -DVULKAN_ENABLED=1
    -DVK_PLATFORM_SUPPORTS_MOLTENVK=0$<$<PLATFORM_ID:Darwin>:1>
)

# Export for use in other CMakeLists.txt files
mark_as_advanced(VULKAN_AVAILABLE)
mark_as_advanced(VULKAN_SDK_PATH)
