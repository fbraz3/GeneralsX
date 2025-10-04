# Interface libraries to set common defines and includes, avoid duplication later
add_library(core_wwcommon INTERFACE)

target_compile_definitions(core_wwcommon INTERFACE
    #NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# SDL2 and OpenGL for non-Windows platforms (Phase 27.1.2 - Graphics Implementation)
if(NOT WIN32)
    # Find SDL2 for window and OpenGL context management
    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        message(STATUS "Phase 27.1.2: SDL2 found - ${SDL2_LIBRARIES}")
    endif()
    
    # OpenGL framework already configured in cmake/opengl.cmake
    if(ENABLE_OPENGL)
        message(STATUS "Phase 27.1.2: OpenGL enabled for graphics rendering")
    endif()
    
    # Create GLAD static library for OpenGL 3.3 Core function loading
    add_library(glad STATIC 
        ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/glad/glad.c
    )
    target_include_directories(glad PUBLIC
        ${CMAKE_SOURCE_DIR}/Core/Libraries/Include
    )
    
    message(STATUS "Phase 27.1.2: GLAD OpenGL loader integrated")
endif()

target_link_libraries(core_wwcommon INTERFACE
    core_config
    core_utility
    # Windows: DirectX 8 and Miles Sound System
    $<$<PLATFORM_ID:Windows>:d3d8lib>
    $<$<PLATFORM_ID:Windows>:milesstub>
    # Non-Windows: SDL2, GLAD, and OpenGL framework (Phase 27.1.2)
    $<$<NOT:$<PLATFORM_ID:Windows>>:SDL2::SDL2>
    $<$<NOT:$<PLATFORM_ID:Windows>>:glad>
    $<$<NOT:$<PLATFORM_ID:Windows>>:${OPENGL_PLATFORM_LIBS}>
    stlport
)

target_include_directories(core_wwcommon INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    WW3D2
    WWAudio
    WWDebug
    # WWDownload # is not added because headers are included as "WWDownload/file.h"
    WWLib
    WWMath
    WWSaveLoad
    # Add GLAD headers for OpenGL function loading (Phase 27.1.2)
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_SOURCE_DIR}/Core/Libraries/Include>
)

add_subdirectory(WW3D2)
add_subdirectory(WWAudio)
add_subdirectory(WWDebug)
add_subdirectory(WWDownload)
add_subdirectory(WWLib)
add_subdirectory(WWMath)
add_subdirectory(WWSaveLoad)
add_subdirectory(WWStub)

# Helpful interface to bundle the ww modules together.
add_library(core_wwvegas INTERFACE)

target_include_directories(core_wwvegas INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    WW3D2
    WWAudio
    WWDebug
    # WWDownload # is not added because headers are included as "WWDownload/file.h"
    WWLib
    WWMath
    WWSaveLoad
)

target_link_libraries(core_wwvegas INTERFACE
    # core_ww3d2
    # core_wwaudio
    core_wwdebug
    # core_wwdownload
    core_wwlib
    core_wwmath
    core_wwsaveload
)
