add_executable(z_generals WIN32)

# Use a binary name that doesn't conflict with original game.
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME generalszh)
else()
    set_target_properties(z_generals PROPERTIES OUTPUT_NAME generalszh)
endif()

target_link_libraries(z_generals PRIVATE
    z_gameengine
    z_gameenginedevice
    zi_always
    core_debug
    core_profile
)

# Windows-only libraries
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_libraries(z_generals PRIVATE
        binkstub
        comctl32
        d3d8
        d3dx8
        dinput8
        dxguid
        imm32
        milesstub
        vfw32
        winmm
    )
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

target_link_options(z_generals PRIVATE
    $<$<OR:$<BOOL:${WIN32}>,$<PLATFORM_ID:Windows>>:/NODEFAULTLIB:libci.lib>
)

target_include_directories(z_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(z_generals PRIVATE
    WinMain.h
)

# Platform-specific entry point
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(z_generals PRIVATE
        WinMain.cpp
    )
else()
    # POSIX/Unix entry point for macOS/Linux
    target_sources(z_generals PRIVATE
        main_posix.cpp
    )
endif()

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

# Create a convenience copy with canonical name `GeneralsXZH` in the runtime output directory
add_custom_command(TARGET z_generals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Generating convenience executable name: GeneralsXZH"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:z_generals> $<TARGET_FILE_DIR:z_generals>/GeneralsXZH
    COMMAND ${CMAKE_COMMAND} -E chmod $<TARGET_FILE_DIR:z_generals>/GeneralsXZH 0755
    COMMENT "Create GeneralsXZH alias for z_generals"
)
