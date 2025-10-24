cmake_minimum_required(VERSION 3.25)

# Use packagename_ROOT for FindPackage.
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

# Disable default MSVC setting CRT type so we can set it ourselves.
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# Disable default MSVC warning level so we can set it ourselves.
if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

# Allow specifying MSVC debug configurations.
#if(POLICY CMP0141)
#    cmake_policy(SET CMP0141 NEW)
#endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# ============================================================================
# Compilation Cache Configuration (ccache/sccache)
# ============================================================================
# Dramatically reduces rebuild times by caching compiled object files
# - First build: ~3-4 minutes (no cache)
# - Rebuild with changes: ~30-60 seconds (cache hit)
# - Full rebuild from cache: ~1 minute
#
# Installation:
#   macOS:   brew install ccache
#   Linux:   apt-get install ccache / yum install ccache
#   Windows: cargo install sccache (cross-platform Rust implementation)
#
# Configuration:
#   ccache -M 10G  (set max cache size to 10GB)
#   ccache -s      (show cache statistics)
#   ccache -C      (clear cache if needed)
# ============================================================================

option(USE_CCACHE "Use ccache/sccache to speed up compilation" ON)

if(USE_CCACHE)
    # Try to find ccache first (native Unix tool)
    find_program(CCACHE_PROGRAM ccache)
    
    # Fallback to sccache (cross-platform Rust implementation)
    if(NOT CCACHE_PROGRAM)
        find_program(CCACHE_PROGRAM sccache)
    endif()
    
    if(CCACHE_PROGRAM)
        message(STATUS "Found compilation cache: ${CCACHE_PROGRAM}")
        
        # Set compiler launchers for both C and C++
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        
        # Get cache program name for statistics
        get_filename_component(CACHE_NAME "${CCACHE_PROGRAM}" NAME)
        
        message(STATUS "✅ Compilation cache enabled: ${CACHE_NAME}")
        message(STATUS "   - First build will populate cache (~3-4 min)")
        message(STATUS "   - Subsequent rebuilds will be much faster (~30-60 sec)")
        message(STATUS "   - Run '${CACHE_NAME} -s' to view cache statistics")
        message(STATUS "   - Run '${CACHE_NAME} -M 10G' to set max cache size")
        
        # Recommended ccache/sccache configuration
        if(CACHE_NAME STREQUAL "ccache")
            message(STATUS "   💡 Recommended: export CCACHE_DIR=$HOME/.ccache (default)")
            message(STATUS "   💡 Recommended: export CCACHE_MAXSIZE=10G")
        elseif(CACHE_NAME STREQUAL "sccache")
            message(STATUS "   💡 Recommended: export SCCACHE_DIR=$HOME/.sccache")
            message(STATUS "   💡 Recommended: export SCCACHE_CACHE_SIZE=10G")
        endif()
    else()
        message(STATUS "⚠️  Compilation cache not found (ccache/sccache)")
        message(STATUS "   Install ccache for faster rebuilds:")
        message(STATUS "   - macOS:   brew install ccache")
        message(STATUS "   - Linux:   apt-get install ccache")
        message(STATUS "   - Windows: cargo install sccache")
        message(STATUS "   Continuing without cache (rebuilds will be slower)")
    endif()
else()
    message(STATUS "ℹ️  Compilation cache disabled (USE_CCACHE=OFF)")
    message(STATUS "   Enable with: cmake -DUSE_CCACHE=ON")
endif()

# ============================================================================

include(FeatureSummary)
include(CMakeDependentOption)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

# Top level project, doesn't really affect anything.
project(genzh LANGUAGES C CXX)

# This file handles extra settings wanted/needed for different compilers.
include(cmake/compilers.cmake)

include(FetchContent)

# Add OpenGL support
include(cmake/opengl.cmake)

# Add Metal support (macOS)
include(cmake/metal.cmake)

# Add OpenAL support for cross-platform audio (Phase 5)
if(NOT WIN32)
    find_package(OpenAL REQUIRED)
    if(OpenAL_FOUND)
        message(STATUS "OpenAL found: ${OPENAL_LIBRARY}")
        # Only create the imported target if it doesn't exist
        if(NOT TARGET OpenAL::OpenAL)
            add_library(OpenAL::OpenAL INTERFACE IMPORTED)
            target_link_libraries(OpenAL::OpenAL INTERFACE ${OPENAL_LIBRARY})
            target_include_directories(OpenAL::OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
        endif()
    endif()
endif()

# Find/Add build dependencies and stubs shared by all projects
# TheSuperHackers stubs are cross-platform, include always
include(cmake/miles.cmake)
include(cmake/bink.cmake)
include(cmake/openal.cmake)

if((WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows") AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    include(cmake/dx8.cmake)
    include(cmake/dbghelp.cmake)
endif()

# Define a dummy stlport target when not on VC6.
if (IS_VS6_BUILD)
    include(cmake/stlport.cmake)
else()
    add_library(stlport INTERFACE)
endif()

include(cmake/config.cmake)
include(cmake/gamespy.cmake)
include(cmake/lzhl.cmake)

if (IS_VS6_BUILD)
    # The original max sdk does not compile against a modern compiler.
    # If there is a desire to make this work, then a fixed max sdk needs to be created.
    add_subdirectory(Dependencies/MaxSDK)
endif()
add_subdirectory(Dependencies/Utility)
add_subdirectory(resources)

# Find FFmpeg globally if enabled (before adding subdirectories)
if(RTS_BUILD_OPTION_FFMPEG)
    find_package(FFMPEG REQUIRED)
    if(FFMPEG_FOUND)
        message(STATUS "✅ FFmpeg found globally: ${FFMPEG_LIBRARIES}")
        # Make variables available to all subdirectories
        set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} CACHE INTERNAL "")
        set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} CACHE INTERNAL "")
    endif()
endif()

add_subdirectory(Core)

# Add main build targets
if(RTS_BUILD_ZEROHOUR)
    add_subdirectory(GeneralsMD)
endif()

if(RTS_BUILD_GENERALS)
    add_subdirectory(Generals)
endif()

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
