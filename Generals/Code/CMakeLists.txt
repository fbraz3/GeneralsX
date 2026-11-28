# g stands for Generals, i stands for Interface
add_library(gi_gameengine_include INTERFACE)
add_library(gi_libraries_source_wwvegas INTERFACE)
add_library(gi_main INTERFACE)
add_library(gi_always INTERFACE)
add_library(gi_always_no_pch INTERFACE) # Use this for Shared Libs with MFC AFX

target_include_directories(gi_gameengine_include INTERFACE "GameEngine/Include")
target_include_directories(gi_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(gi_main INTERFACE "Main")
# GeneralsX @build Copilot 25/11/2026 On Windows expose only the four d3dx8 headers so CompatLib's POSIX windows.h never shadows the Windows SDK.
if(WIN32)
    set(_compatlib_win_inc "${CMAKE_CURRENT_BINARY_DIR}/compatlib-d3dx8-only")
    file(MAKE_DIRECTORY "${_compatlib_win_inc}")
    foreach(_d3dx_hdr IN ITEMS d3dx8.h d3dx8core.h d3dx8math.h d3dx8tex.h)
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/CompatLib/Include/${_d3dx_hdr}"
            "${_compatlib_win_inc}/${_d3dx_hdr}"
            COPYONLY)
    endforeach()
    target_include_directories(gi_always INTERFACE "${_compatlib_win_inc}")
elseif(UNIX)
    target_include_directories(gi_always INTERFACE "CompatLib/Include")
endif()

target_compile_definitions(gi_always INTERFACE
    RTS_GENERALS=1
)
target_compile_definitions(gi_always_no_pch INTERFACE
    RTS_GENERALS=1
)
target_link_libraries(gi_gameengine_include INTERFACE
    corei_gameengine_include
)
target_link_libraries(gi_always INTERFACE
    corei_always # Must stay below so headers from game are included first
)
target_link_libraries(gi_always_no_pch INTERFACE
    corei_always_no_pch # Must stay below so headers from game are included first
)

# Contains internal libraries
add_subdirectory(Libraries)

# Base GameEngine library
add_subdirectory(GameEngine)

# Platform specific GameEngine code
add_subdirectory(GameEngineDevice)

# Contains the main game exe target.
add_subdirectory(Main)

if (RTS_BUILD_GENERALS_TOOLS OR RTS_BUILD_GENERALS_EXTRAS)
    add_subdirectory(Tools)
endif()
