
add_executable(z_generals WIN32)

# GeneralsX @feature GitHub Copilot 20/05/2026 Keep executable naming consistent across Linux/macOS/Windows.
set_target_properties(z_generals PROPERTIES OUTPUT_NAME "GeneralsXZH")


# GeneralsX @build BenderAI 13/02/2026 Windows-specific libraries
if(WIN32)
    target_link_libraries(z_generals PRIVATE
        comctl32
        core_debug
        dinput8
        dxguid
        imm32
        vfw32
        winmm
    )
    if(TARGET binkstub)
        target_link_libraries(z_generals PRIVATE binkstub)
    endif()
    if(TARGET milesstub)
        target_link_libraries(z_generals PRIVATE milesstub)
    endif()
endif()


# Cross-platform core libraries
target_link_libraries(z_generals PRIVATE
    core_profile_legacy
    z_gameengine
    z_gameenginedevice
    zi_always
    d3dx8
    ws2_32
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

target_sources(z_generals PRIVATE
    $<$<BOOL:${WIN32}>:WinMain.cpp>
    $<$<BOOL:${WIN32}>:WinMain.h>
    # Linux SDL3 entry point (Phase 1.5 - TheSuperHackers @build 10/02/2026)
    $<$<NOT:$<BOOL:${WIN32}>>:SDL3Main.cpp>
    # Linux stubs for P2-P3 platform functions (P2-P3 - TheSuperHackers @build 13/02/2026 BenderAI Session 37)
    $<$<NOT:$<BOOL:${WIN32}>>:LinuxStubs.cpp>
)

# RC files for Windows builds (MSVC + MinGW)
# GeneralsX @feature GitHub Copilot 20/05/2026 Embed app icon and manifest on all Windows toolchains.
if(WIN32)
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(MSVC AND NOT IS_VS6_BUILD)
        target_link_options(z_generals PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    target_sources(z_generals PRIVATE
        RTS.RC
    )
endif()

# Strip debug symbols to separate file for MinGW Release builds
# This creates generalszh${RTS_BUILD_OUTPUT_SUFFIX}.exe.debug (similar to MSVC .pdb files)
if(MINGW AND COMMAND add_debug_strip_target)
    add_debug_strip_target(z_generals)
endif()
