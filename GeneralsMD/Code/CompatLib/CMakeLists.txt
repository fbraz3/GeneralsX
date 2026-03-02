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
# Use FetchContent for GLM and GLI (both header-only libraries)
# This avoids vcpkg compilation issues with MSVC runtime library
if(NOT SAGE_USE_GLM)
    message(FATAL_ERROR "GLM is required for d3dx8 compatibility layer")
endif()

include(${CMAKE_SOURCE_DIR}/cmake/glm.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/gli.cmake)

# Try vcpkg first for compatibility, fall back to FetchContent
find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
    # FetchContent is already loaded via include(glm.cmake)
endif()

find_package(gli CONFIG QUIET)
if(NOT gli_FOUND)
    # FetchContent is already loaded via include(gli.cmake)
endif()

target_link_libraries(d3dx8 PUBLIC glm::glm gli::gli)

# GeneralsX @build BenderAI 26/05/2026 - Disable MSVC two-phase name lookup for d3dx8.
# GLM 1.0.3 added glm::make_vec4 which conflicts with gli::make_vec4 via ADL inside
# gli::detail::convertFunc templates. /Zc:twoPhase- prevents ADL from finding glm::make_vec4
# when called within the gli namespace, resolving the "ambiguous call" C2668 errors.
if(MSVC)
    target_compile_options(d3dx8 PRIVATE /Zc:twoPhase-)
endif()

# GeneralsX @build BenderAI 26/05/2026 - CompatLib/Include is Linux-only for d3dx8:
# On Linux: CompatLib shim headers (windows.h → windows_compat.h) intercept <windows.h>
# On Windows: src files use relative paths (../Include/xxx.h). Real SDK windows.h is found
#             via INCLUDE env var (no CompatLib/Include to shadow it).
target_include_directories(d3dx8 PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWLib
)
if(UNIX)
    target_include_directories(d3dx8 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
    )
endif()


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
    # Non-legacy: use DXVK headers
    # On Windows Modern (win64-modern): dxvk_headers_SOURCE_DIR (DXVK headers only, no lib)
    # On Linux (linux64-deploy):        dxvk_SOURCE_DIR  (DXVK native with lib)
    if(WIN32)
        # Windows Modern: DXVK_D3D8_INCLUDE_DIR is pre-set in cmake/dx8.cmake
        # Strategy: Use jmarshall DX90SDK headers (proven x64 compatible) instead of DXVK native headers (MinGW types)
        if(NOT DXVK_D3D8_INCLUDE_DIR)
            message(FATAL_ERROR "D3D8 headers not found! Ensure cmake/dx8.cmake was included before CompatLib")
        endif()
        target_include_directories(d3d8lib INTERFACE ${DXVK_D3D8_INCLUDE_DIR})
        # GeneralsX @build BenderAI 01/03/2026 Add DX90SDK lib dir so linker can find d3d8.lib
        # d3d8.lib is a generated import stub from DXVK's d3d8.dll (see Core/Libraries/DX90SDK/Lib/x64/d3d8.lib)
        target_link_directories(d3d8lib INTERFACE "${DXVK_D3D8_INCLUDE_DIR}/../Lib/x64")
    else()
        # Linux: DXVK native provides both headers and lib
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