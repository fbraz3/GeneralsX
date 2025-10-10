add_executable(GeneralsXZH WIN32)

# Use a binary name that matches our branding
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(GeneralsXZH PROPERTIES OUTPUT_NAME GeneralsXZH)
else()
    set_target_properties(GeneralsXZH PROPERTIES OUTPUT_NAME GeneralsXZH)
endif()

target_link_libraries(GeneralsXZH PRIVATE
    $<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/libbinkstub.a>
    $<$<PLATFORM_ID:Windows>:comctl32>
    core_debug
    core_profile
    $<$<PLATFORM_ID:Windows>:d3d8>
    $<$<PLATFORM_ID:Windows>:d3dx8>
    $<$<PLATFORM_ID:Windows>:dbghelplib>
    $<$<PLATFORM_ID:Windows>:dinput8>
    $<$<PLATFORM_ID:Windows>:dxguid>
    $<$<PLATFORM_ID:Windows>:imm32>
    $<$<PLATFORM_ID:Windows>:milesstub>
    $<$<PLATFORM_ID:Windows>:vfw32>
    $<$<PLATFORM_ID:Windows>:winmm>
    $<$<NOT:$<PLATFORM_ID:Windows>>:SDL2::SDL2>
    z_gameengine
    z_gameenginedevice
    zi_always
)

# Set build information variables with default values that can be overridden
if(NOT DEFINED VERSION_BUILDUSER)
    set(VERSION_BUILDUSER "\"The Super Hackers\"")
endif()

if(NOT DEFINED VERSION_BUILDLOC)
    set(VERSION_BUILDLOC "\"https://github.com/TheSuperHackers/GeneralsGameCode\"")
endif()

# Normalize to ensure they are C string literals even when provided via -D on the CLI
set(_VERSION_BUILDUSER ${VERSION_BUILDUSER})
if(NOT _VERSION_BUILDUSER MATCHES "^\".*\"$")
    set(_VERSION_BUILDUSER "\"${VERSION_BUILDUSER}\"")
endif()

set(_VERSION_BUILDLOC ${VERSION_BUILDLOC})
if(NOT _VERSION_BUILDLOC MATCHES "^\".*\"$")
    set(_VERSION_BUILDLOC "\"${VERSION_BUILDLOC}\"")
endif()

# Use git revision count as build number if available, otherwise use default or override
if(NOT DEFINED VERSION_BUILDNUM)
    if(DEFINED GIT_REV_LIST_COUNT)
        set(VERSION_BUILDNUM ${GIT_REV_LIST_COUNT})
    else()
        set(VERSION_BUILDNUM 601)
    endif()
endif()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#ifndef VERSION_BUILDNUM
#define VERSION_BUILDNUM ${VERSION_BUILDNUM}
#endif

#ifndef VERSION_LOCALBUILDNUM
#define VERSION_LOCALBUILDNUM 0
#endif

#ifndef VERSION_BUILDUSER
#define VERSION_BUILDUSER ${_VERSION_BUILDUSER}
#endif

#ifndef VERSION_BUILDLOC
#define VERSION_BUILDLOC ${_VERSION_BUILDLOC}
#endif
"
)

# Based on original binary values for these variables.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 4
"
)

if(WIN32)
    target_link_options(GeneralsXZH PRIVATE "/NODEFAULTLIB:libci.lib")
endif()

target_include_directories(GeneralsXZH PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(GeneralsXZH PRIVATE
    WinMain.cpp
    WinMain.h
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(NOT IS_VS6_BUILD)
        target_link_options(GeneralsXZH PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    target_sources(GeneralsXZH PRIVATE
        RTS.RC
    )
endif()
