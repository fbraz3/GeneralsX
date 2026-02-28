# z stands for Zero Hour, i stands for Interface
add_library(zi_gameengine_include INTERFACE)
add_library(zi_libraries_source_wwvegas INTERFACE)
add_library(zi_main INTERFACE)
add_library(zi_always INTERFACE)
add_library(zi_always_no_pch INTERFACE) # Use this for Shared Libs with MFC AFX

target_include_directories(zi_gameengine_include INTERFACE "GameEngine/Include")
target_include_directories(zi_libraries_source_wwvegas INTERFACE "Libraries/Source/WWVegas")
target_include_directories(zi_main INTERFACE "Main")
# GeneralsX @build BenderAI 28/02/2026 - CompatLib/Include contains Linux-only headers
# (windows.h shim, types_compat.h, pthread.h, etc.) that break MSVC compilation.
# On Linux: Must be included so D3DX8 stubs, Windows compat layer, etc. are found.
# On WIN32 (all variants): D3DX8 headers come via d3d8lib INTERFACE (DXVK native or DX8 SDK).
# TheSuperHackers @build 08/02/2026 Add DirectX 8 compat headers
if(NOT WIN32)
    target_include_directories(zi_always INTERFACE "CompatLib/Include")
endif()

target_compile_definitions(zi_always INTERFACE
    RTS_ZEROHOUR=1
)
target_compile_definitions(zi_always_no_pch INTERFACE
    RTS_ZEROHOUR=1
)
target_link_libraries(zi_gameengine_include INTERFACE
    corei_gameengine_include
)
target_link_libraries(zi_always INTERFACE
    corei_always # Must stay below so headers from game are included first
)
target_link_libraries(zi_always_no_pch INTERFACE
    corei_always_no_pch # Must stay below so headers from game are included first
)

# CompatLib: Platform compatibility layer (Linux headers, d3d8lib config)
add_subdirectory(CompatLib)

# Contains internal libraries
add_subdirectory(Libraries)

# Base GameEngine library
add_subdirectory(GameEngine)

# Platform specific GameEngine code
add_subdirectory(GameEngineDevice)

# Contains the main game exe target.
add_subdirectory(Main)

if (RTS_BUILD_ZEROHOUR_TOOLS OR RTS_BUILD_ZEROHOUR_EXTRAS)
    add_subdirectory(Tools)
endif()
