# On 64-bit Windows, macOS, and Linux: use our d3dx8 implementation (GLI/GLM for math helpers)
# GeneralsX @build BenderAI 24/02/2026 Phase 5 - Skip d3dx8 on macOS/Linux due to GLI/GLM ambiguity
# GeneralsX @build felipebraz 20/06/2025 - Also build d3dx8 STATIC on macOS (D3DX math/texture stubs required for link)
# GeneralsX @bugfix felipebraz 02/03/2026 - Build real d3dx8 STATIC on Linux too (D3DXGetFVFVertexSize / D3DXLoadSurfaceFromSurface required for link)
# d3dx8 is a math/texture helper stub; core game code doesn't require it
if((WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8) OR APPLE OR (UNIX AND NOT APPLE))
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

# Try vcpkg first for GLM compatibility, fall back to FetchContent (already loaded by glm.cmake)
find_package(glm CONFIG QUIET)

# GeneralsX @build felipebraz 20/06/2025 GLI causes make_vec4 ambiguity with Apple Clang
# Only find/link GLI on non-Apple platforms to avoid the collision in gli::detail::convertFunc
find_package(gli CONFIG QUIET)

if(NOT APPLE)
    target_link_libraries(d3dx8 PUBLIC glm::glm gli::gli)
else()
    # macOS/Apple Clang: link GLM only (GLI omitted to prevent make_vec4 ADL ambiguity)
    target_link_libraries(d3dx8 PUBLIC glm::glm)
endif()

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

else()
# Linux only: Create empty d3dx8 stub library to satisfy d3d8lib linkage
# macOS and Win64 both build real d3dx8 STATIC above; Linux can use the stub for now
add_library(d3dx8 INTERFACE)
target_link_libraries(d3dx8 INTERFACE d3d8lib)
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
    # Non-legacy: use DXVK headers for all modern platforms
    # Windows Modern (win64-modern): DX90SDK headers (proven x64 compatible, from jmarshall reference)
    # macOS   (macos-vulkan):        DXVK built from source, Wine-style include/native layout
    # Linux   (linux64-deploy):      DXVK pre-built steamrt tarball, flat include/dxvk/
    if(WIN32)
        # Windows Modern: DXVK_D3D8_INCLUDE_DIR is pre-set by cmake/dx8.cmake to jmarshall's DX90SDK
        # Strategy: DX90SDK headers are MSVC x64-proven; DXVK native headers have MinGW types.
        if(NOT DXVK_D3D8_INCLUDE_DIR)
            message(FATAL_ERROR "D3D8 headers not found! Ensure cmake/dx8.cmake was included before CompatLib")
        endif()
        target_include_directories(d3d8lib INTERFACE ${DXVK_D3D8_INCLUDE_DIR})
        # GeneralsX @build BenderAI 01/03/2026 Add DX90SDK lib dir so linker finds d3d8.lib
        # d3d8.lib is an import stub generated from DXVK's d3d8.dll
        target_link_directories(d3d8lib INTERFACE "${DXVK_D3D8_INCLUDE_DIR}/../Lib/x64")
    elseif(APPLE)
        # macOS: DXVK built from git source -- Wine-style include/native layout
        # GeneralsX @build BenderAI 24/02/2026 Phase 5 - DXVK v2.6 uses include/native/windows + include/native/directx
        # GeneralsX @build fbraz 24/02/2026 - Also need include/native for WSI headers (wsi/native_wsi.h)
        if(NOT dxvk_SOURCE_DIR)
            message(FATAL_ERROR "DXVK not fetched! Ensure SAGE_USE_DX8=OFF and cmake/dx8.cmake was included")
        endif()
        target_include_directories(d3d8lib INTERFACE
            ${dxvk_SOURCE_DIR}/include/native
            ${dxvk_SOURCE_DIR}/include/native/windows
            ${dxvk_SOURCE_DIR}/include/native/directx
            ${dxvk_SOURCE_DIR}/include/vulkan
        )
        # GeneralsX @bugfix BenderAI 25/02/2026 On macOS, DXVK dylibs live in the Meson build dir
        # (_deps/dxvk-build-macos/src/d3d*/), not in ${dxvk_SOURCE_DIR}/lib.
        # Passing a non-existent -L path produces: ld: warning: search path not found
    else()
        # Linux: DXVK pre-built steamrt tarball -- all headers flat in include/dxvk/
        # GeneralsX @bugfix felipebraz 02/03/2026 - Restore Linux path (tarball = include/dxvk, not include/native)
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

# GeneralsX @build BenderAI 24/02/2026 Phase 5 - Require C++17 for std::filesystem in compat headers
set_property(TARGET CompatLib PROPERTY CXX_STANDARD 17)
set_property(TARGET d3dx8 PROPERTY CXX_STANDARD 17)

# GeneralsX @bugfix BenderAI 02/03/2026 - Enforce DXVK source availability before CompatLib compiles
# On macOS, DXVK is built via ExternalProject (Meson) which clones the source during the BUILD phase,
# not during configure. This means _deps/dxvk-src/include/native/windows/windows_base.h does not
# exist when d3dx8 or CompatLib start compiling unless we declare an explicit ordering dependency.
# On Linux, DXVK is a pre-built tarball fetched via FetchContent (available since configure), so
# no ordering dependency is needed there.
if(APPLE AND TARGET dxvk_macos_build)
    add_dependencies(d3dx8 dxvk_macos_build)
    add_dependencies(CompatLib dxvk_macos_build)
endif()

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