# OpenGL configuration for GeneralsGameCode project
# This file adds OpenGL support while maintaining DirectX compatibility

# Build options
option(ENABLE_OPENGL "Enable OpenGL renderer support" ON)
option(ENABLE_DIRECTX "Enable DirectX 8 renderer support" ON)
option(DEFAULT_TO_OPENGL "Use OpenGL as default renderer" OFF)

# SDL2 is required for cross-platform window/input management
if(NOT WIN32)
    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        message(STATUS "Phase 27.1.2: SDL2 found - ${SDL2_LIBRARIES}")
        # Create SDL2::SDL2 imported target if not already created
        if(NOT TARGET SDL2::SDL2)
            add_library(SDL2::SDL2 UNKNOWN IMPORTED)
            set_target_properties(SDL2::SDL2 PROPERTIES
                IMPORTED_LOCATION "${SDL2_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
            )
        endif()
        message(STATUS "    ${SDL2_LIBRARIES}")
    else()
        message(FATAL_ERROR "SDL2 not found - required for non-Windows platforms")
    endif()
endif()

# Detect platform and configure appropriate OpenGL
if(ENABLE_OPENGL)
    if(WIN32)
        # Windows - OpenGL + WGL extensions
        find_package(OpenGL REQUIRED)
        if(NOT OpenGL_FOUND)
            message(FATAL_ERROR "OpenGL not found on Windows")
        endif()
        
        # Add Windows-specific libraries
        set(OPENGL_PLATFORM_LIBS opengl32 gdi32 user32)
        
    elseif(UNIX AND NOT APPLE)
        # Linux - OpenGL + GLX
        find_package(OpenGL REQUIRED)
        find_package(X11 REQUIRED)
        
        if(NOT OPENGL_FOUND)
            message(FATAL_ERROR "OpenGL not found on Linux")
        endif()
        if(NOT X11_FOUND)
            message(FATAL_ERROR "X11 not found on Linux")
        endif()
        
        set(OPENGL_PLATFORM_LIBS ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
        
    elseif(APPLE)
        # macOS - OpenGL framework
        find_library(OPENGL_FRAMEWORK OpenGL)
        find_library(COCOA_FRAMEWORK Cocoa)
        
        if(NOT OPENGL_FRAMEWORK)
            message(FATAL_ERROR "OpenGL framework not found on macOS")
        endif()
        
        set(OPENGL_PLATFORM_LIBS ${OPENGL_FRAMEWORK} ${COCOA_FRAMEWORK})
        
    endif()
    
    # Define macro for compilation
    add_compile_definitions(ENABLE_OPENGL=1)
    
    if(DEFAULT_TO_OPENGL)
        add_compile_definitions(DEFAULT_GRAPHICS_API=OPENGL)
    endif()
    
    message(STATUS "OpenGL renderer enabled for platform: ${CMAKE_SYSTEM_NAME}")
endif()

# DirectX 8 (somente Windows)
if(ENABLE_DIRECTX AND WIN32)
    add_compile_definitions(ENABLE_DIRECTX8=1)
    
    if(NOT DEFAULT_TO_OPENGL)
        add_compile_definitions(DEFAULT_GRAPHICS_API=DIRECTX8)
    endif()
    
    message(STATUS "DirectX 8 renderer enabled")
elseif(ENABLE_DIRECTX AND NOT WIN32)
    message(WARNING "DirectX 8 requested but not available on non-Windows platforms")
    set(ENABLE_DIRECTX OFF)
endif()

# Check if at least one API is enabled
if(NOT ENABLE_OPENGL AND NOT ENABLE_DIRECTX)
    message(FATAL_ERROR "At least one graphics API must be enabled")
endif()

# Function to add graphics support to a target
function(add_graphics_support target_name)
    if(ENABLE_OPENGL)
        target_link_libraries(${target_name} PRIVATE ${OPENGL_PLATFORM_LIBS})
        
        # Include platform-specific headers
        if(WIN32)
            target_include_directories(${target_name} PRIVATE ${OPENGL_INCLUDE_DIR})
        elseif(UNIX AND NOT APPLE)
            target_include_directories(${target_name} PRIVATE 
                ${OPENGL_INCLUDE_DIR} 
                ${X11_INCLUDE_DIR}
            )
        endif()
    endif()
    
    if(ENABLE_DIRECTX AND WIN32)
        # DirectX 8 is already configured via FetchContent in main CMakeLists
        target_link_libraries(${target_name} PRIVATE dx8)
    endif()
    
    # Add the new graphics abstraction code
    target_sources(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/GraphicsAPI/GraphicsRenderer.cpp
        ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/GraphicsAPI/W3DRendererAdapter.cpp
    )
    
    if(ENABLE_OPENGL)
        target_sources(${target_name} PRIVATE
            ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/GraphicsAPI/OpenGLRenderer.cpp
        )
    endif()
    
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/Core/Libraries/Include/GraphicsAPI
    )
endfunction()

# Show final configuration
message(STATUS "Graphics Configuration:")
message(STATUS "  OpenGL: ${ENABLE_OPENGL}")
message(STATUS "  DirectX 8: ${ENABLE_DIRECTX}")
message(STATUS "  Default API: ${DEFAULT_TO_OPENGL}")
