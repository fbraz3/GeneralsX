# Phase 47: OpenAL Audio Library Configuration
#
# OpenAL is a cross-platform 3D audio library used for:
# - Music playback (background tracks)
# - Sound effects (unit actions, weapons, explosions)
# - Voice acting (unit responses, campaign dialogue)
# - 3D positional audio (spatial sound)
# - Audio effects and mixing

# On macOS, use system OpenAL.framework which supports ARM64
# Homebrew openal-soft only ships x86_64, so we use the native framework instead
if(APPLE)
    # Use macOS system OpenAL.framework for ARM64 support
    find_library(OPENAL_LIBRARY OpenAL)
    if(NOT OPENAL_LIBRARY)
        message(FATAL_ERROR "OpenAL.framework not found on macOS")
    endif()
    # OpenAL.framework headers are in the framework bundle, not /usr/include
    set(OPENAL_INCLUDE_DIR "${OPENAL_LIBRARY}/Headers" CACHE PATH "OpenAL include directory")
else()
    # For Linux/Windows, try standard OpenAL-soft
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/local/opt/openal-soft")
    list(INSERT CMAKE_FIND_ROOT_PATH 0 "/usr/local/opt/openal-soft")
    set(OPENAL_INCLUDE_DIR "/usr/local/opt/openal-soft/include" CACHE PATH "OpenAL include directory")
    set(OPENAL_LIBRARY "/usr/local/opt/openal-soft/lib/libopenal.dylib" CACHE FILEPATH "OpenAL library")
endif()

# On macOS with OpenAL.framework, directly set paths without find_package
if(APPLE)
    # Verify the framework and header path exist
    if(NOT OPENAL_LIBRARY)
        message(FATAL_ERROR "OpenAL.framework not found on macOS")
    endif()
    if(NOT EXISTS "${OPENAL_INCLUDE_DIR}")
        message(FATAL_ERROR "OpenAL headers not found at ${OPENAL_INCLUDE_DIR}")
    endif()

    # Create imported library for OpenAL.framework
    if(NOT TARGET OpenAL::OpenAL)
        add_library(OpenAL::OpenAL INTERFACE IMPORTED)
        set_target_properties(OpenAL::OpenAL PROPERTIES
            INTERFACE_LINK_LIBRARIES "${OPENAL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
        )
    endif()
    set(OPENAL_LIBRARIES OpenAL::OpenAL CACHE INTERNAL "OpenAL libraries")
    set(OPENAL_FOUND TRUE CACHE INTERNAL "OpenAL found")
else()
    # For Linux/Windows, try standard find_package
    find_package(OpenAL REQUIRED)

    if(NOT OPENAL_FOUND)
        message(FATAL_ERROR "OpenAL not found. Please install OpenAL development files:")
        if(UNIX)
            message(FATAL_ERROR "  Linux: apt-get install libopenal-dev")
        elseif(WIN32)
            message(FATAL_ERROR "  Windows: Download from https://www.openal.org/")
        endif()
    endif()

    if(UNIX AND NOT APPLE)
        # Create an imported library target if needed to avoid file dependency issues
        if(NOT TARGET OpenAL::OpenAL)
            add_library(OpenAL::OpenAL SHARED IMPORTED)
            set_target_properties(OpenAL::OpenAL PROPERTIES
                IMPORTED_LOCATION "${OPENAL_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
            )
        endif()
        set(OPENAL_LIBRARIES OpenAL::OpenAL CACHE INTERNAL "OpenAL libraries")
    else()
        set(OPENAL_LIBRARIES ${OPENAL_LIBRARY} CACHE INTERNAL "OpenAL libraries")
    endif()
endif()

# Add OpenAL to include directories globally
include_directories(${OPENAL_INCLUDE_DIR})

# Export OpenAL include directory
set(OPENAL_INCLUDE_DIR ${OPENAL_INCLUDE_DIR} CACHE INTERNAL "OpenAL include directory")

message(STATUS "Phase 47: OpenAL configured")
message(STATUS "  OpenAL Include: ${OPENAL_INCLUDE_DIR}")
message(STATUS "  OpenAL Library: ${OPENAL_LIBRARY}")
