add_executable(g_generals)

# Use a binary name that doesn't conflict with original game.
# Phase 39.5: Unified output name for all platforms
set_target_properties(g_generals PROPERTIES OUTPUT_NAME generalsv)

target_link_libraries(g_generals PRIVATE
    g_gameengine
    g_gameenginedevice
    gi_always
)

# Phase 39.5: Windows-specific libraries removed
# Graphics now unified to Vulkan, input unified to SDL2
# Complete cross-platform unification achieved

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

# Phase 39.5: Platform-specific linker options removed - unified for all platforms

target_include_directories(g_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Code/Libraries
)

target_sources(g_generals PRIVATE
    WinMain.h
)

# Unified entry point: WinMain.cpp for all platforms
# Phase 39.5: Platform #ifdef blocks removed - all platforms use same SDL2 code path
target_sources(g_generals PRIVATE
    WinMain.cpp
)

# VC6 Windows builds still need RC file
if(IS_VS6_BUILD)
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
