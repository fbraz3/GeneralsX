add_executable(GeneralsXZH)

# Use a binary name that matches the canonical target name
# Phase 39.5: Unified output name for all platforms
set_target_properties(GeneralsXZH PROPERTIES OUTPUT_NAME GeneralsXZH)

# Phase 43.7 FIX: Force linker to use our stubs by adding them AFTER libraries
# Normally linker resolves symbols from libz_gameengine.a first, but those symbols
# are undefined there. By using link options to specify undefined symbols,
# we force the linker to search in our object files too.
if(APPLE)
    target_link_options(GeneralsXZH PRIVATE
        -Wl,-undefined,dynamic_lookup
    )
endif()

target_link_libraries(GeneralsXZH PRIVATE
    z_gameenginedevice
    zi_always
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
    phase43_registry_stubs.cpp
)

# VC6 Windows builds still need RC file
if(IS_VS6_BUILD)
    target_sources(GeneralsXZH PRIVATE
        RTS.RC
    )
endif()

# Create a convenience copy with canonical name `GeneralsXZH` in the runtime output directory
add_custom_command(TARGET GeneralsXZH POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GeneralsXZH> $<TARGET_FILE_DIR:GeneralsXZH>/GeneralsXZH
    COMMAND chmod 0755 $<TARGET_FILE_DIR:GeneralsXZH>/GeneralsXZH
    COMMENT "Create GeneralsXZH alias for GeneralsXZH"
)
