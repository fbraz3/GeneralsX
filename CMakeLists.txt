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

include(FeatureSummary)
include(CMakeDependentOption)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

# Top level project, doesn't really affect anything.
project(genzh LANGUAGES C CXX)

# Phase 54.2: Linux static library circular dependency fix
# On Linux with static libraries, the linker processes libraries in order and only
# resolves symbols needed at that moment. This causes issues when library A needs
# symbols from library B but A is processed before B. The solution is to wrap all
# static libraries in --start-group / --end-group to allow multiple resolution passes.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Define a custom link library type that wraps libraries in a group
    set(CMAKE_C_LINK_LIBRARY_USING_RESCAN_SUPPORTED TRUE)
    set(CMAKE_CXX_LINK_LIBRARY_USING_RESCAN_SUPPORTED TRUE)
    set(CMAKE_C_LINK_LIBRARY_USING_RESCAN "-Wl,--start-group" "<LINK_ITEM>" "-Wl,--end-group")
    set(CMAKE_CXX_LINK_LIBRARY_USING_RESCAN "-Wl,--start-group" "<LINK_ITEM>" "-Wl,--end-group")
    
    # Override the default executable link command to wrap all libraries in a group
    # This ensures proper resolution of circular dependencies between static libraries
    set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> -Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group")
endif()

# This file handles extra settings wanted/needed for different compilers.
include(cmake/compilers.cmake)

include(FetchContent)

# Build dependencies (miles/bink only needed for VC6 Windows builds)
# Phase 39.5: Platform detection removed - codebase unified for all platforms
if (IS_VS6_BUILD AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    include(cmake/miles.cmake)
    include(cmake/bink.cmake)
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
include(cmake/sdl2.cmake)
include(cmake/openal.cmake)

if (IS_VS6_BUILD)
    # The original max sdk does not compile against a modern compiler.
    # If there is a desire to make this work, then a fixed max sdk needs to be created.
    add_subdirectory(Dependencies/MaxSDK)
endif()
add_subdirectory(Dependencies/Utility)
add_subdirectory(resources)

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
