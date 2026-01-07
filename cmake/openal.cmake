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
    set(OPENAL_INCLUDE_DIR "/usr/include" CACHE PATH "OpenAL include directory")
    if(NOT OPENAL_LIBRARY)
        message(FATAL_ERROR "OpenAL.framework not found on macOS")
    endif()
else()
    # For Linux/Windows, try standard OpenAL-soft
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/local/opt/openal-soft")
    list(INSERT CMAKE_FIND_ROOT_PATH 0 "/usr/local/opt/openal-soft")
    set(OPENAL_INCLUDE_DIR "/usr/local/opt/openal-soft/include" CACHE PATH "OpenAL include directory")
    set(OPENAL_LIBRARY "/usr/local/opt/openal-soft/lib/libopenal.dylib" CACHE FILEPATH "OpenAL library")
endif()

# Find OpenAL package
find_package(OpenAL REQUIRED)

if(NOT OPENAL_FOUND)
    message(FATAL_ERROR "OpenAL not found. Please install OpenAL development files:")
    if(APPLE)
        message(FATAL_ERROR "  macOS: brew install openal-soft")
    elseif(UNIX)
        message(FATAL_ERROR "  Linux: apt-get install libopenal-dev")
    elseif(WIN32)
        message(FATAL_ERROR "  Windows: Download from https://www.openal.org/")
    endif()
endif()

# On Linux, create an imported library target if needed to avoid file dependency issues
if(UNIX AND NOT APPLE)
    # Create an imported library target that Ninja won't try to build
    if(NOT TARGET OpenAL::OpenAL)
        add_library(OpenAL::OpenAL SHARED IMPORTED)
        set_target_properties(OpenAL::OpenAL PROPERTIES
            IMPORTED_LOCATION "${OPENAL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
        )
    endif()
    # Use the imported target for linking
    set(OPENAL_LIBRARIES OpenAL::OpenAL CACHE INTERNAL "OpenAL libraries")
else()
    # For non-Linux platforms, use the standard approach
    set(OPENAL_LIBRARIES ${OPENAL_LIBRARY} CACHE INTERNAL "OpenAL libraries")
endif()

# Add OpenAL to include directories
include_directories(${OPENAL_INCLUDE_DIR})

# Export OpenAL include directory
set(OPENAL_INCLUDE_DIR ${OPENAL_INCLUDE_DIR} CACHE INTERNAL "OpenAL include directory")

message(STATUS "Phase 47: OpenAL configured")
message(STATUS "  OpenAL Include: ${OPENAL_INCLUDE_DIR}")
message(STATUS "  OpenAL Library: ${OPENAL_LIBRARY}")
