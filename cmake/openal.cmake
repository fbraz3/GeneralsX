# Phase 47: OpenAL Audio Library Configuration
# 
# OpenAL is a cross-platform 3D audio library used for:
# - Music playback (background tracks)
# - Sound effects (unit actions, weapons, explosions)
# - Voice acting (unit responses, campaign dialogue)
# - 3D positional audio (spatial sound)
# - Audio effects and mixing

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

# Add OpenAL to include directories
include_directories(${OPENAL_INCLUDE_DIR})

# Export OpenAL for use in other CMakeLists.txt files
set(OPENAL_LIBRARIES ${OPENAL_LIBRARY} CACHE INTERNAL "OpenAL libraries")
set(OPENAL_INCLUDE_DIR ${OPENAL_INCLUDE_DIR} CACHE INTERNAL "OpenAL include directory")

message(STATUS "Phase 47: OpenAL configured")
message(STATUS "  OpenAL Include: ${OPENAL_INCLUDE_DIR}")
message(STATUS "  OpenAL Library: ${OPENAL_LIBRARY}")
