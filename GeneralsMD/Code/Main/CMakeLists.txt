add_executable(GeneralsXZH)

# Use a binary name that matches the canonical target name
# Phase 39.5: Unified output name for all platforms
set_target_properties(GeneralsXZH PROPERTIES OUTPUT_NAME GeneralsXZH)

# Phase 43.7 FIX (revised): Fail fast on missing symbols on macOS.
#
# Historically we used `-Wl,-undefined,dynamic_lookup`, which defers undefined
# symbol failures to runtime (dyld flat-namespace errors). That makes iteration
# slow and hides the full set of missing objects.
#
# Use `-Wl,-undefined,error` so the link step reports all missing symbols.
if(APPLE)
    target_link_options(GeneralsXZH PRIVATE
        -Wl,-undefined,error
    )
    # Phase 43.8: Link macOS system frameworks needed by game code
    # GlobalLanguage.cpp uses CoreFoundation and CoreText for font management
    target_link_libraries(GeneralsXZH PRIVATE
        "-framework CoreFoundation"
        "-framework CoreText"
    )
endif()

target_link_libraries(GeneralsXZH PRIVATE
    z_gameenginedevice
    zi_always
)

# Phase 54.1: Repeat core libraries at end of link for Linux circular dependencies
# Static library link order matters - symbols must be resolved when first needed
# By adding core libraries again at the end, we ensure symbols that ww3d2 needs
# are available since libww3d2.a comes after libwwlib.a/libwwmath.a
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(GeneralsXZH PRIVATE
        core_wwlib
        core_wwmath
        core_wwsaveload
        graphics_drivers
        # Second pass - repeat because ww3d2 needs symbols from these
        core_wwlib
        core_wwmath
    )
endif()

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

# Phase 54: POSIX command line compatibility for non-Windows platforms
if(NOT WIN32)
    target_sources(GeneralsXZH PRIVATE
        ${CMAKE_SOURCE_DIR}/Dependencies/Utility/Compat/msvc_posix_compat.cpp
    )
endif()

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
