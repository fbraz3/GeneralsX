# Phase 41: Graphics Driver Framework

# Find Vulkan SDK
find_package(Vulkan REQUIRED)

add_library(graphics_drivers STATIC
    # Graphics Driver Interface
    IGraphicsDriver.h
    GraphicsDriverFactory.h
    GraphicsDriverFactory.cpp
    DRIVER_ARCHITECTURE.md
    
    # DirectX 8 Compatibility Layer (Phase 41)
    dx8buffer_compat.h
    dx8buffer_compat.cpp
    
    # Phase 41: Stub implementations for missing methods
    phase41_missing_implementations.cpp
    phase41_global_stubs.cpp
    phase41_cpp_stubs.cpp
    phase41_final_stubs.cpp
    
    # Vulkan Backend (Primary Implementation)
    Vulkan/vulkan_graphics_driver.h
    Vulkan/vulkan_graphics_driver.cpp
    Vulkan/vulkan_globals.cpp
    Vulkan/vulkan_ww3d_wrapper.cpp
    
    # D3D8 to Vulkan Mappings (22 files)
    Vulkan/d3d8_vulkan_buffer.h
    Vulkan/d3d8_vulkan_buffer.cpp
    Vulkan/d3d8_vulkan_command_buffer.h
    Vulkan/d3d8_vulkan_command_buffer.cpp
    Vulkan/d3d8_vulkan_culling.h
    Vulkan/d3d8_vulkan_culling.cpp
    Vulkan/d3d8_vulkan_descriptor.h
    Vulkan/d3d8_vulkan_descriptor.cpp
    Vulkan/d3d8_vulkan_drawing.h
    Vulkan/d3d8_vulkan_drawing.cpp
    Vulkan/d3d8_vulkan_enums_compat.h
    Vulkan/d3d8_vulkan_graphics_compat.h
    Vulkan/d3d8_vulkan_interfaces_compat.h
    Vulkan/d3d8_vulkan_lighting.h
    Vulkan/d3d8_vulkan_lighting.cpp
    Vulkan/d3d8_vulkan_material.h
    Vulkan/d3d8_vulkan_material.cpp
    Vulkan/d3d8_vulkan_render_pass.h
    Vulkan/d3d8_vulkan_render_pass.cpp
    Vulkan/d3d8_vulkan_renderloop.h
    Vulkan/d3d8_vulkan_renderloop.cpp
    Vulkan/d3d8_vulkan_rendertarget.h
    Vulkan/d3d8_vulkan_rendertarget.cpp
    Vulkan/d3d8_vulkan_shader.h
    Vulkan/d3d8_vulkan_shader.cpp
    Vulkan/d3d8_vulkan_texture.h
    Vulkan/d3d8_vulkan_texture.cpp
    Vulkan/d3d8_vulkan_types_compat.h
    Vulkan/d3d8_vulkan_viewport.h
    Vulkan/d3d8_vulkan_viewport.cpp
    
    # Future Backend Stubs (Prepared for Phase 50+)
    Future/opengl_graphics_driver.h
    Future/directx12_graphics_driver.h
    Future/metal_graphics_driver.h
    Future/graphics_driver_stubs.cpp
)

# Include paths for Graphics Framework
target_include_directories(graphics_drivers 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Link Vulkan SDK
target_link_libraries(graphics_drivers 
    PRIVATE 
        Vulkan::Vulkan
)

# Compile definitions
if(MSVC)
    target_compile_options(graphics_drivers PRIVATE /W4)
else()
    target_compile_options(graphics_drivers PRIVATE -Wall -Wextra -Wpedantic)
endif()

# C++ standard
set_target_properties(graphics_drivers PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Add graphics library to core libraries
add_library(graphics_drivers_interface INTERFACE)
target_link_libraries(graphics_drivers_interface 
    INTERFACE 
        graphics_drivers
)
