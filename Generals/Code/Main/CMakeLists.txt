add_executable(g_generals WIN32)

# Use a binary name that doesn't conflict with original game.
if("${CMAKE_SYSTEM}" MATCHES "Windows")
    set_target_properties(g_generals PROPERTIES OUTPUT_NAME generalsv)
else()
    set_target_properties(g_generals PROPERTIES OUTPUT_NAME generalsv)
endif()

target_link_libraries(g_generals PRIVATE
    g_gameengine
    g_gameenginedevice
    gi_always
)

# Windows-only libraries
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_libraries(g_generals PRIVATE
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

# Windows-only linker options
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(g_generals PRIVATE "/NODEFAULTLIB:libci.lib")
endif()

target_include_directories(g_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Code/Libraries
)

target_sources(g_generals PRIVATE
    WinMain.h
)

# Platform-specific entry point
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(g_generals PRIVATE
        WinMain.cpp
    )
else()
    # POSIX/Unix entry point for macOS/Linux
    target_sources(g_generals PRIVATE
        main_posix.cpp
    )
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(NOT IS_VS6_BUILD)
        target_link_options(g_generals PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    target_sources(g_generals PRIVATE
        RTS.RC
    )
endif()

# Create a convenience copy with canonical name `GeneralsX` in the runtime output directory
add_custom_command(TARGET g_generals POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Generating convenience executable name: GeneralsX"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:g_generals> $<TARGET_FILE_DIR:g_generals>/GeneralsX
    COMMAND ${CMAKE_COMMAND} -E chmod $<TARGET_FILE_DIR:g_generals>/GeneralsX 0755
    COMMENT "Create GeneralsX alias for g_generals"
)
