# Interface libraries to set common defines and includes, avoid duplication later
add_library(z_wwcommon INTERFACE)

target_link_libraries(z_wwcommon INTERFACE
    core_config
    core_utility
    core_wwcommon
    stlport
)

# Link Windows-only stubs (only on Windows)
if(WIN32 OR MSVC OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(z_wwcommon INTERFACE
        d3d8lib
        milesstub
    )
endif()

target_include_directories(z_wwcommon INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    WW3D2
)

add_subdirectory(WWAudio)
add_subdirectory(WW3D2)
#add_subdirectory(WWDownload)  # TODO: Fix Windows winsock.h and process.h dependencies

# Phase 39.3: Vulkan Graphics Backend (replaces DirectX8Wrapper)
# Only include Vulkan backend when USE_VULKAN is enabled
if(USE_VULKAN)
    add_subdirectory(../Vulkan Vulkan)
endif()

# Helpful interface to bundle the ww modules together.
add_library(z_wwvegas INTERFACE)

target_include_directories(z_wwvegas INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    WW3D2
)

target_link_libraries(z_wwvegas INTERFACE
    core_wwvegas
    z_ww3d2
    #z_wwdownload  # TODO: Fix Windows winsock.h and process.h dependencies
)

# Phase 39.3: Link Vulkan backend when enabled
if(USE_VULKAN)
    target_link_libraries(z_wwvegas INTERFACE
        z_vulkan_graphics_backend
    )
endif()
