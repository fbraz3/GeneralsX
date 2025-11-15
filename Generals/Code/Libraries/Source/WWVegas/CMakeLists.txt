# Interface libraries to set common defines and includes, avoid duplication later
add_library(g_wwcommon INTERFACE)

target_link_libraries(g_wwcommon INTERFACE
    core_config
    core_utility
    core_wwcommon
    stlport
)

# Link Windows-only stubs (only on Windows)
if(WIN32 OR MSVC OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(g_wwcommon INTERFACE
        d3d8lib
        milesstub
    )
endif()

target_include_directories(g_wwcommon INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    WW3D2
)

add_subdirectory(WWAudio)
add_subdirectory(WW3D2)
#add_subdirectory(WWDownload)  # TODO: Fix Windows winsock.h and process.h dependencies

# Helpful interface to bundle the ww modules together.
add_library(g_wwvegas INTERFACE)

target_include_directories(g_wwvegas INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    WW3D2
)

target_link_libraries(g_wwvegas INTERFACE
    core_wwvegas
    g_ww3d2
    #g_wwdownload  # TODO: Fix Windows winsock.h and process.h dependencies
)
