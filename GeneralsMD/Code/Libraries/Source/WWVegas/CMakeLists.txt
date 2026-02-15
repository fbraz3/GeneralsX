# Interface libraries to set common defines and includes, avoid duplication later
add_library(z_wwcommon INTERFACE)

# GeneralsX @build BenderAI 13/02/2026 d3d8lib is INTERFACE (headers only) - safe for all
target_link_libraries(z_wwcommon INTERFACE
    core_wwcommon
    d3d8lib     # INTERFACE target: provides DXVK/DirectX types (DWORD, BOOL, etc.)
    stlport
)

# GeneralsX @build BenderAI 13/02/2026 Platform-specific multimedia stubs
if(WIN32)
    target_link_libraries(z_wwcommon INTERFACE
        milesstub
    )
endif()

target_include_directories(z_wwcommon INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    WW3D2
)

add_subdirectory(WWAudio)
add_subdirectory(WW3D2)
add_subdirectory(WWDownload)

# Helpful interface to bundle the ww modules together.
add_library(z_wwvegas INTERFACE)

target_include_directories(z_wwvegas INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    WW3D2
)

target_link_libraries(z_wwvegas INTERFACE
    core_wwvegas
    z_ww3d2
    z_wwdownload
)
