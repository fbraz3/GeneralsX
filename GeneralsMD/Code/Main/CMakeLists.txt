add_executable(z_generals WIN32)

# GeneralsX @bugfix BenderAI 02/03/2026 - OUTPUT_NAME was inverted (Windows got lowercase, Linux got branding).
# Branding: executable name must be GeneralsXZH on all platforms.
set_target_properties(z_generals PROPERTIES OUTPUT_NAME GeneralsXZH)

# GeneralsX @build BenderAI 13/02/2026 Windows-specific libraries
if(WIN32)
    target_link_libraries(z_generals PRIVATE
        comctl32
        core_debug
        d3d8
        dinput8
        dxguid
        imm32
        vfw32
        winmm
    )
    if(NOT SAGE_USE_SDL3)
        # GeneralsX @build BenderAI 01/03/2026 Bink/Miles stubs for legacy 32-bit builds only
        # win64-modern uses DXVK (no Bink) and OpenAL (no Miles); no 64-bit stubs available
        target_link_libraries(z_generals PRIVATE
            binkstub
            milesstub
        )
    endif()
endif()

# Cross-platform core libraries
target_link_libraries(z_generals PRIVATE
    core_profile
    d3dx8
    z_gameengine
    z_gameenginedevice
    zi_always
)

# Linux SDL3 windowing/input (Phase 1)
# GeneralsX @build BenderAI 11/02/2026
if(SAGE_USE_SDL3)
    target_link_libraries(z_generals PRIVATE sdl3lib)
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
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define VERSION_BUILDNUM 601
"
)

if(MSVC)
    target_link_options(z_generals PRIVATE "/NODEFAULTLIB:libci.lib")
endif()

target_include_directories(z_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# GeneralsX @bugfix BenderAI 01/03/2026 SDL3Main.cpp includes <wsi/native_wsi.h> (DXVK WSI for SDL3).
# The DXVK native WSI headers are in ${DXVK_INCLUDE_DIR}/native/ (Windows Modern).
# On Linux, the DXVK native path is in dxvk_SOURCE_DIR which is already added by z_gameengine.
if(SAGE_USE_SDL3 AND DEFINED DXVK_INCLUDE_DIR)
    target_include_directories(z_generals PRIVATE "${DXVK_INCLUDE_DIR}/native")
elseif(SAGE_USE_SDL3 AND DEFINED dxvk_SOURCE_DIR)
    target_include_directories(z_generals PRIVATE "${dxvk_SOURCE_DIR}/usr/include/dxvk/native")
endif()

target_sources(z_generals PRIVATE
    # Entry point selection based on windowing backend
    # GeneralsX @build BenderAI 26/02/2026 - Phase 6
    # If SAGE_USE_SDL3=ON: SDL3 entry point (cross-platform: Linux and Windows Modern)
    # If SAGE_USE_SDL3=OFF and WIN32: Win32 entry point (Windows Legacy: vc6/win32 presets)
    $<$<AND:$<BOOL:${WIN32}>,$<NOT:$<BOOL:${SAGE_USE_SDL3}>>>:WinMain.cpp>
    $<$<AND:$<BOOL:${WIN32}>,$<NOT:$<BOOL:${SAGE_USE_SDL3}>>>:WinMain.h>
    # SDL3 entry point (Phase 1.5 - TheSuperHackers @build 10/02/2026)
    # Now also used on Windows Modern (Phase 6 - BenderAI 26/02/2026)
    $<$<OR:$<NOT:$<BOOL:${WIN32}>>,$<BOOL:${SAGE_USE_SDL3}>>:SDL3Main.cpp>
    # Linux stubs for P2-P3 platform functions (P2-P3 - TheSuperHackers @build 13/02/2026 BenderAI Session 37)
    $<$<NOT:$<BOOL:${WIN32}>>:LinuxStubs.cpp>
)

# RC files optional for MinGW builds
# Icon: LoadIcon() handles missing resource gracefully (uses default system icon)
# Manifest: DPI awareness metadata (nice to have but not essential)
if(MSVC)
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(NOT IS_VS6_BUILD)
        target_link_options(z_generals PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    target_sources(z_generals PRIVATE
        RTS.RC
    )
endif()

# Strip debug symbols to separate file for MinGW Release builds
# This creates generalszh.exe.debug (similar to MSVC .pdb files)
if(MINGW AND COMMAND add_debug_strip_target)
    add_debug_strip_target(z_generals)
endif()
