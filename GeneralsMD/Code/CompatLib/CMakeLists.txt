# On 64-bit Windows or Unix, use our d3dx8 implementation
if (UNIX OR WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
list(APPEND SOURCE_D3DX_COMPAT
    d3dx8_compat.cpp
    d3dx8math.cpp
)

list(TRANSFORM SOURCE_D3DX_COMPAT PREPEND "Source/")

add_library(d3dx8 STATIC ${SOURCE_D3DX_COMPAT})

# GLM requires GLM_ENABLE_EXPERIMENTAL for internal extensions (like gli/core/coord.hpp)
target_compile_definitions(d3dx8 PRIVATE GLM_ENABLE_EXPERIMENTAL)

# TheSuperHackers @build 09/02/2025 Bender
# Use FetchContent for GLM (header-only library, no compilation needed)
# This avoids vcpkg compilation issues with MSVC runtime library
if(NOT SAGE_USE_GLM)
    message(FATAL_ERROR "GLM is required for d3dx8 compatibility layer")
endif()

include(${CMAKE_SOURCE_DIR}/cmake/glm.cmake)
find_package(glm CONFIG QUIET)  # Try vcpkg first for compatibility
if(NOT glm_FOUND)
    # Fall back to our FetchContent GLM from glm.cmake
endif()
target_link_libraries(d3dx8 PUBLIC glm::glm)

find_package(gli CONFIG REQUIRED)
target_link_libraries(d3dx8 PUBLIC gli)

# d3dx8 needs bittype.h from WWLib for Win32 types (DWORD, BYTE, UINT, etc.)
# Add only WWLib directory (not full core_wwcommon to avoid d3d8.h conflicts from WWAudio)
target_include_directories(d3dx8 PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWLib
)

# Link d3dx8 to d3d8lib to get DirectX8 headers (min-dx8-sdk or DXVK)
target_link_libraries(d3dx8 PUBLIC d3d8lib)

endif()

# GeneralsX @build BenderAI 10/02/2026
# Configure d3d8lib target globally (created early in Core/CMakeLists.txt)
# Fighter19's approach: Windows uses min-dx8-sdk, Linux uses ONLY DXVK headers
if(SAGE_USE_DX8)
    # Windows native: Use min-dx8-sdk headers and libs
    if(NOT dx8_SOURCE_DIR)
        message(FATAL_ERROR "DirectX 8 SDK (min-dx8-sdk) not fetched! Ensure cmake/dx8.cmake was included")
    endif()
    target_include_directories(d3d8lib INTERFACE
        ${dx8_SOURCE_DIR}/include
    )
    target_link_directories(d3d8lib INTERFACE
        ${dx8_SOURCE_DIR}/lib
    )
else()
    # Linux: Use ONLY DXVK headers (not min-dx8-sdk) to avoid conflicts
    # DXVK provides complete Wine-compatible DirectX 8 headers
    if(NOT dxvk_SOURCE_DIR)
        message(FATAL_ERROR "DXVK not fetched! Ensure SAGE_USE_DX8=OFF and cmake/dx8.cmake was included")
    endif()
    target_include_directories(d3d8lib INTERFACE
        ${dxvk_SOURCE_DIR}/include/dxvk
    )
    target_link_directories(d3d8lib INTERFACE
        ${dxvk_SOURCE_DIR}/lib
    )
endif()

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

# GeneralsX @build BenderAI 10/02/2026
# Fighter19 pattern: d3d8lib (INTERFACE) links TO CompatLib
# This makes CompatLib includes transitively available to all targets linking d3d8lib
# REVERSED from previous (CompatLib linking TO d3d8lib)
target_link_libraries(d3d8lib INTERFACE CompatLib)

# CompatLib needs d3dx8 for D3DX types
# SDL3 headers available via vcpkg but linking happens in GameEngineDevice/Main
target_link_libraries(CompatLib PUBLIC d3dx8)

endif()