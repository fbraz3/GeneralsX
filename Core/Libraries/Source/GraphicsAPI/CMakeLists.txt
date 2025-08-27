cmake_minimum_required(VERSION 3.25)

# Graphics API abstraction library
# Provides OpenGL and DirectX 8 support through a unified interface

project(GraphicsAPI LANGUAGES CXX)

# Source files
set(GRAPHICS_API_SOURCES
    GraphicsRenderer.cpp
    W3DRendererAdapter.cpp
)

# Add OpenGL sources if enabled
if(ENABLE_OPENGL)
    list(APPEND GRAPHICS_API_SOURCES OpenGLRenderer.cpp)
endif()

# Create the library
add_library(GraphicsAPI STATIC ${GRAPHICS_API_SOURCES})

# Include directories
target_include_directories(GraphicsAPI PUBLIC
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Include
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Include/GraphicsAPI
)

# Link with graphics libraries
add_graphics_support(GraphicsAPI)

# Set C++ standard
target_compile_features(GraphicsAPI PUBLIC cxx_std_17)

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(GraphicsAPI PRIVATE 
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
elseif(UNIX AND NOT APPLE)
    # Linux specific settings
    target_compile_definitions(GraphicsAPI PRIVATE 
        LINUX
    )
elseif(APPLE)
    # macOS specific settings
    target_compile_definitions(GraphicsAPI PRIVATE 
        MACOS
    )
endif()

# Export include directories for other targets
target_include_directories(GraphicsAPI INTERFACE
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Include/GraphicsAPI
)

# Make sure always.h and other core headers are available
target_link_libraries(GraphicsAPI PRIVATE 
    z_always
    core_utility
)

message(STATUS "Graphics API library configured with:")
if(ENABLE_OPENGL)
    message(STATUS "  - OpenGL support: YES")
else()
    message(STATUS "  - OpenGL support: NO")
endif()

if(ENABLE_DIRECTX AND WIN32)
    message(STATUS "  - DirectX 8 support: YES")
else()
    message(STATUS "  - DirectX 8 support: NO")
endif()
