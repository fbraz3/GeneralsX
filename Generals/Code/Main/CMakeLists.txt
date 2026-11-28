add_executable(g_generals WIN32)

# Use a binary name that doesn't conflict with original game.
# GeneralsX @build BenderAI 15/02/2026 Linux branding update
# GeneralsX @build fbraz3 19/08/2026 Standardize executable name across all platforms
set_target_properties(g_generals PROPERTIES OUTPUT_NAME GeneralsX)

# GeneralsX @build BenderAI 10/03/2026 Windows-specific libraries
if(WIN32)
    target_link_libraries(g_generals PRIVATE
        deps_bink
        comctl32
        dxguid
        imm32
        vfw32
        winmm
    )
    if(NOT SAGE_USE_SDL3)
        target_link_libraries(g_generals PRIVATE dinput8)
    endif()
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_libraries(g_generals PRIVATE d3d8)
    endif()
    if(NOT SAGE_USE_MINIAUDIO AND NOT SAGE_USE_OPENAL)
        target_link_libraries(g_generals PRIVATE deps_miles)
    endif()
endif()

# GeneralsX @feature Copilot 25/11/2026 Static d3d8to9 (Direct3D 9) graphics backend for Windows x64
if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(g_generals PRIVATE gx_d3d8to9)
    target_compile_definitions(g_generals PRIVATE GX_COMPAT_D3D9=1)
endif()

# Cross-platform core libraries
target_link_libraries(g_generals PRIVATE
    d3dx8
    g_gameengine
    g_gameenginedevice
    gi_always
)

# Linux SDL3 windowing/input
# GeneralsX @build BenderAI 11/02/2026
if(SAGE_USE_SDL3)
    target_link_libraries(g_generals PRIVATE sdl3lib)
endif()

# TODO Originally referred to build host and user, replace with git info perhaps?
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#define VERSION_LOCALBUILDNUM 0
#define VERSION_BUILDUSER \"\"
#define VERSION_BUILDLOC \"\"
"
)

# Based on original binary values for these variables.
if (IS_VS6_BUILD)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 7
#define VERSION_BUILDNUM 601
"
)
else()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 8
#define VERSION_BUILDNUM 601
"
)
endif()

if(MSVC)
    target_link_options(g_generals PRIVATE
        "/NODEFAULTLIB:libci.lib"
        "$<$<CONFIG:RelWithDebInfo>:/OPT:REF>"
        "$<$<CONFIG:RelWithDebInfo>:/OPT:NOICF>"
    )
endif()

target_include_directories(g_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Code/Libraries
)

target_sources(g_generals PRIVATE
    $<$<BOOL:${WIN32}>:WinMain.cpp>
    $<$<BOOL:${WIN32}>:WinMain.h>
    # Linux SDL3 entry point
    $<$<NOT:$<BOOL:${WIN32}>>:SDL3Main.cpp>
    # Linux stubs
    $<$<NOT:$<BOOL:${WIN32}>>:LinuxStubs.cpp>
)

# RC files are optional for MinGW builds
# Icon: LoadIcon() handles missing resource gracefully (uses default system icon)
# Manifest: DPI awareness metadata (nice to have but not essential)
# TYPELIB: Broken (0 bytes) and causes windres memory exhaustion errors
if(MSVC)
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(NOT IS_VS6_BUILD)
        target_link_options(g_generals PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    target_sources(g_generals PRIVATE
        RTS.RC
    )
    set_source_files_properties(RTS.RC PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/app.manifest;${CMAKE_CURRENT_SOURCE_DIR}/Generals.ico")
endif()

# Strip debug symbols to separate file for MinGW Release builds
# This creates generalsv${RTS_BUILD_OUTPUT_SUFFIX}.exe.debug (similar to MSVC .pdb files)
if(MINGW AND COMMAND add_debug_strip_target)
    add_debug_strip_target(g_generals)
endif()
