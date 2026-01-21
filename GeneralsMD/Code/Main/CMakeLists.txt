add_executable(z_generals WIN32)

# Use a binary name that doesn't conflict with original game.
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME GeneralsXZH)
else()
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME GeneralsXZH)
endif()

target_link_libraries(z_generals PRIVATE
    binkstub
    comctl32
    core_debug
    core_profile
    d3d8
    d3dx8
    dinput8
    dxguid
    imm32
    milesstub
    vfw32
    winmm
    z_gameengine
    z_gameenginedevice
    zi_always
    SDL2::SDL2
)

# Add delay load import library for Windows builds (needed for /DELAYLOAD)
if(WIN32)
    target_link_libraries(z_generals PRIVATE delayimp)
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

target_link_options(z_generals PRIVATE "/NODEFAULTLIB:libci.lib")

target_include_directories(z_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Platform-specific entry point
# VS6 build keeps the original Win32 entrypoint.
# All other builds use the SDL2 entrypoint.
if(IS_VS6_BUILD)
    set(ENTRY_POINT_SRC WinMain.cpp WinMain.h)
    message(STATUS "Main: Using WinMain.cpp (VS6 legacy entry)")
else()
    set(ENTRY_POINT_SRC SDL2Main.cpp SDL2Main.h)
    message(STATUS "Main: Using SDL2Main.cpp (SDL2 entry)")
endif()

target_sources(z_generals PRIVATE
    ${ENTRY_POINT_SRC}
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
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
