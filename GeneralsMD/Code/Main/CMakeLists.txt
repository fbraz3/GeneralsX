add_executable(z_generals)

# Use a binary name that doesn't conflict with original game.
# Phase 39.5: Unified output name for all platforms
set_target_properties(z_generals PROPERTIES OUTPUT_NAME generalszh)

target_link_libraries(z_generals PRIVATE
    z_gameengine
    z_gameenginedevice
    zi_always
    core_debug
    core_profile
)

# Phase 39.5: Windows-specific libraries removed
# Graphics now unified to Vulkan, input unified to SDL2, audio to OpenAL
# No Windows API libraries needed - complete cross-platform unification

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

# Phase 39.5: Platform-specific linker options removed - unified for all platforms

target_include_directories(z_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(z_generals PRIVATE
    WinMain.h
)

# Unified entry point: WinMain.cpp for all platforms
# Phase 39.5: Platform #ifdef blocks removed - all platforms use same SDL2 code path
target_sources(z_generals PRIVATE
    WinMain.cpp
)

# VC6 Windows builds still need RC file
if(IS_VS6_BUILD)
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
