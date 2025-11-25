add_executable(GeneralsXZH)

# Use a binary name that matches the canonical target name
# Phase 39.5: Unified output name for all platforms
set_target_properties(GeneralsXZH PROPERTIES OUTPUT_NAME GeneralsXZH)

target_link_libraries(GeneralsXZH PRIVATE
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

target_include_directories(GeneralsXZH PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(GeneralsXZH PRIVATE
    WinMain.h
)

# Unified entry point: WinMain.cpp for all platforms
# Phase 39.5: Platform #ifdef blocks removed - all platforms use same SDL2 code path
target_sources(GeneralsXZH PRIVATE
    WinMain.cpp
)

# VC6 Windows builds still need RC file
if(IS_VS6_BUILD)
    target_sources(GeneralsXZH PRIVATE
        RTS.RC
    )
endif()
