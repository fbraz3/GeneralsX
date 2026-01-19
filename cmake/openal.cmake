# Phase 07: OpenAL Audio Library Configuration
#
# OpenAL is a cross-platform 3D audio library used for:
# - Music playback (background tracks)
# - Sound effects (unit actions, weapons, explosions)
# - Voice acting (unit responses, campaign dialogue)
# - 3D positional audio (spatial sound)
# - Audio effects and mixing

# Windows VC6: Use vcpkg-installed OpenAL-soft
# Installation: vcpkg install openal-soft:x86-windows

# First, try to find OpenAL in standard vcpkg install location
if(WIN32)
    set(VCPKG_DEFAULT_TRIPLET "x86-windows" CACHE STRING "vcpkg target triplet")

    # Try standard vcpkg install locations (packages/ first, then installed/)
    set(VCPKG_POSSIBLE_ROOTS
        "$ENV{VCPKG_ROOT}/packages/openal-soft_${VCPKG_DEFAULT_TRIPLET}"
        "C:/vcpkg/packages/openal-soft_${VCPKG_DEFAULT_TRIPLET}"
        "$ENV{VCPKG_ROOT}/installed/${VCPKG_DEFAULT_TRIPLET}"
        "C:/vcpkg/installed/${VCPKG_DEFAULT_TRIPLET}"
        "C:/Users/$ENV{USERNAME}/vcpkg/installed/${VCPKG_DEFAULT_TRIPLET}"
    )

    foreach(VCPKG_ROOT ${VCPKG_POSSIBLE_ROOTS})
        if(EXISTS "${VCPKG_ROOT}/include/AL/al.h")
            message(STATUS "Phase 07: Found OpenAL headers at ${VCPKG_ROOT}")
            set(OPENAL_INCLUDE_DIR "${VCPKG_ROOT}/include" CACHE PATH "OpenAL include directory" FORCE)

            # Find library
            find_library(OPENAL_LIBRARY
                NAMES OpenAL32
                PATHS "${VCPKG_ROOT}/lib"
                NO_DEFAULT_PATH
            )

            if(OPENAL_LIBRARY)
                set(OPENAL_FOUND TRUE)
                message(STATUS "Phase 07: Found OpenAL library at ${OPENAL_LIBRARY}")
                break()
            endif()
        endif()
    endforeach()
endif()

# Fallback: Try system find_package
if(NOT OPENAL_FOUND)
    find_package(OpenAL CONFIG QUIET)
    if(NOT OpenAL_FOUND)
        find_package(OpenAL QUIET)
    endif()
endif()

# Report status and create target if needed
if(OPENAL_FOUND OR OpenAL_FOUND)
    message(STATUS "Phase 07: ✅ OpenAL configured successfully")
    # Normalize variable names for compatibility
    if(OpenAL_FOUND AND NOT OPENAL_FOUND)
        set(OPENAL_FOUND TRUE)
    endif()
    if(NOT OPENAL_LIBRARY AND OpenAL_LIBRARIES)
        set(OPENAL_LIBRARY ${OpenAL_LIBRARIES})
    endif()

    # Create OpenAL::OpenAL target if it doesn't exist
    if(NOT TARGET OpenAL::OpenAL)
        add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
        set_target_properties(OpenAL::OpenAL PROPERTIES
            IMPORTED_LOCATION "${OPENAL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
        )
    endif()
else()
    message(FATAL_ERROR "Phase 07: ❌ OpenAL library not found!")
    message(FATAL_ERROR "  Windows VC6: vcpkg install openal-soft:x86-windows")
endif()
