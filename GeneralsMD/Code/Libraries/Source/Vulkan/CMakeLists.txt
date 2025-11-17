###############################################################################
# Phase 39.3: Vulkan Graphics Backend
###############################################################################

# Find Vulkan SDK
find_package(Vulkan REQUIRED)

set(VULKAN_GRAPHICS_BACKEND_SRC
    vulkan_graphics_backend.h
    vulkan_graphics_backend.cpp
)

add_library(z_vulkan_graphics_backend STATIC)
set_target_properties(z_vulkan_graphics_backend PROPERTIES OUTPUT_NAME vulkan_graphics_backend)

target_sources(z_vulkan_graphics_backend PRIVATE ${VULKAN_GRAPHICS_BACKEND_SRC})

# Link against Vulkan SDK
target_link_libraries(z_vulkan_graphics_backend PRIVATE
    Vulkan::Vulkan
)

# Include the Vulkan SDK headers
target_include_directories(z_vulkan_graphics_backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Export public headers
target_include_directories(z_vulkan_graphics_backend INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set C++ standard to C++20 (required for std::make_unique and other features)
target_compile_features(z_vulkan_graphics_backend PRIVATE cxx_std_20)

# Add compile options
if(APPLE)
    # macOS: Use MoltenVK for Vulkan→Metal translation
    target_compile_definitions(z_vulkan_graphics_backend PRIVATE
        VK_USE_PLATFORM_MACOS_MVK
    )
elseif(UNIX)
    # Linux: Use native Vulkan
    target_compile_definitions(z_vulkan_graphics_backend PRIVATE
        VK_USE_PLATFORM_XCB_KHR
    )
elseif(WIN32)
    # Windows: Use native Vulkan
    target_compile_definitions(z_vulkan_graphics_backend PRIVATE
        VK_USE_PLATFORM_WIN32_KHR
    )
endif()

# Link SDL2 for cross-platform window/surface creation (Phase 39.2)
target_link_libraries(z_vulkan_graphics_backend PRIVATE
    SDL2::SDL2
)
