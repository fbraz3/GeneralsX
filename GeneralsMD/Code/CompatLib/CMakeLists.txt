# On 64-bit Windows or Unix, use our d3dx8 implementation
if (UNIX OR WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
list(APPEND SOURCE_D3DX_COMPAT
    d3dx8_compat.cpp
    d3dx8math.cpp
)

list(TRANSFORM SOURCE_D3DX_COMPAT PREPEND "Source/")

add_library(d3dx8 STATIC ${SOURCE_D3DX_COMPAT})

if(NOT SAGE_USE_GLM)
    message(FATAL_ERROR "GLM is required for d3dx8 compatibility layer")
endif()

find_package(glm REQUIRED)
target_link_libraries(d3dx8 PUBLIC glm::glm)

find_package(gli REQUIRED)
target_link_libraries(d3dx8 PUBLIC gli)

# d3dx8 is a pure math library, no platform-specific headers needed
target_include_directories(d3dx8 PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

endif()

# Configure d3d8lib target globally (created early in Core/CMakeLists.txt)
# This MUST be done for both Windows and Linux, not just UNIX
if(SAGE_USE_DX8)
    # Windows: Use min-dx8-sdk headers
    target_include_directories(d3d8lib INTERFACE
        ${dx8_SOURCE_DIR}/include
    )
    target_link_directories(d3d8lib INTERFACE
        ${dx8_SOURCE_DIR}/lib
    )
else()
    # Linux: Use DXVK headers for DirectX→Vulkan translation
    if(NOT dxvk_SOURCE_DIR)
        message(FATAL_ERROR "DXVK not fetched! Ensure SAGE_USE_DX8=OFF and cmake/dx8.cmake was included")
    endif()
    target_include_directories(d3d8lib INTERFACE
        ${dxvk_SOURCE_DIR}/include
        ${dxvk_SOURCE_DIR}/include/dxvk
    )
    target_link_directories(d3d8lib INTERFACE
        ${dxvk_SOURCE_DIR}/lib
    )
endif()

# Link d3d8lib to our d3dx8 math library
target_link_libraries(d3d8lib INTERFACE d3dx8)

if (UNIX)
set(SOURCE_COMPAT
    module_compat.cpp
    string_compat.cpp
    thread_compat.cpp
    time_compat.cpp
    wchar_compat.cpp
    wnd_compat.cpp
)

list(TRANSFORM SOURCE_COMPAT PREPEND "Source/")

add_library(CompatLib STATIC ${SOURCE_COMPAT})

target_include_directories(CompatLib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# Link CompatLib to d3d8lib (already configured globally above)
target_link_libraries(CompatLib PUBLIC d3d8lib)

endif()