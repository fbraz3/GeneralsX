add_executable(GeneralsX)

# Use a binary name that matches the canonical target name
# Phase 39.5: Unified output name for all platforms
set_target_properties(GeneralsX PROPERTIES OUTPUT_NAME GeneralsX)

# Phase 43.8 FIX: Force linker to use our stubs by adding them AFTER libraries
# Apply same linker strategy as GeneralsXZH to ensure Generals reaches feature parity
if(APPLE)
    target_link_options(GeneralsX PRIVATE
        -Wl,-undefined,dynamic_lookup
    )
endif()

target_link_libraries(GeneralsX PRIVATE
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

target_include_directories(GeneralsX PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Code/Libraries
)

target_sources(GeneralsX PRIVATE
    WinMain.h
)

# Unified entry point: WinMain.cpp for all platforms
# Phase 39.5: Platform #ifdef blocks removed - all platforms use same SDL2 code path
target_sources(GeneralsX PRIVATE
    WinMain.cpp
    phase43_registry_stubs.cpp
)

# Phase 54: POSIX command line compatibility for non-Windows platforms
if(NOT WIN32)
    target_sources(GeneralsX PRIVATE
        ${CMAKE_SOURCE_DIR}/Dependencies/Utility/Compat/msvc_posix_compat.cpp
    )
endif()

# VC6 Windows builds still need RC file
if(IS_VS6_BUILD)
    target_sources(GeneralsX PRIVATE
        RTS.RC
    )
endif()

# Create a convenience copy with canonical name `GeneralsX` in the runtime output directory
add_custom_command(TARGET GeneralsX POST_BUILD
    COMMAND cp $<TARGET_FILE:GeneralsX> $<TARGET_FILE_DIR:GeneralsX>/GeneralsX
    COMMAND chmod 0755 $<TARGET_FILE_DIR:GeneralsX>/GeneralsX
    COMMENT "Create GeneralsX alias for GeneralsX"
)
