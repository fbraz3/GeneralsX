# On 64-bit Windows, use our d3dx8 implementation (GLI/GLM for math helpers)
# GeneralsX @build BenderAI 24/02/2026 Phase 5 - Skip d3dx8 on macOS/Linux due to GLI/GLM ambiguity
# GeneralsX @build felipebraz 20/06/2025 - Also build d3dx8 STATIC on macOS (D3DX math/texture stubs required for link)
# d3dx8 is a math/texture helper stub; core game code doesn't require it
if ((WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8) OR APPLE)
list(APPEND SOURCE_D3DX_COMPAT
    d3dx8_compat.cpp
    d3dx8math.cpp
)

list(TRANSFORM SOURCE_D3DX_COMPAT PREPEND "Source/")

add_library(d3dx8 STATIC ${SOURCE_D3DX_COMPAT})

# GLI requires GLM_ENABLE_EXPERIMENTAL for internal extensions (like gli/core/coord.hpp)
target_compile_definitions(d3dx8 PRIVATE GLM_ENABLE_EXPERIMENTAL)

# TheSuperHackers @build 09/02/2025 Bender
# Use vcpkg find_package (like fighter19) instead of FetchContent
# This ensures compatible GLM/GLI versions via vcpkg baseline
if(NOT SAGE_USE_GLM)
    message(FATAL_ERROR "GLM is required for d3dx8 compatibility layer")
endif()

find_package(glm CONFIG REQUIRED)
target_link_libraries(d3dx8 PUBLIC glm::glm)

# GeneralsX @build felipebraz 20/06/2025 GLI causes make_vec4 ambiguity with Apple Clang - only link on Windows
if(NOT APPLE)
    find_package(gli CONFIG REQUIRED)
    target_link_libraries(d3dx8 PUBLIC gli)
endif()

# d3dx8 needs bittype.h from WWLib for Win32 types (DWORD, BYTE, UINT, etc.)
# Add only WWLib directory (not full core_wwcommon to avoid d3d8.h conflicts from WWAudio)
target_include_directories(d3dx8 PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWLib
)

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
    # Linux + macOS: Use ONLY DXVK headers (not min-dx8-sdk) to avoid conflicts
    # DXVK provides complete Wine-compatible DirectX 8 headers
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
    # (_deps/dxvk-build-macos/src/d3d*/), not in ${dxvk_SOURCE_DIR}/lib — that path does not exist.
    # Linux prebuilt DXVK native package does have a lib/ dir, so the guard is macOS-only.
    # Passing a non-existent -L path produces: ld: warning: search path '.../dxvk-src/lib' not found
    if(NOT APPLE)
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