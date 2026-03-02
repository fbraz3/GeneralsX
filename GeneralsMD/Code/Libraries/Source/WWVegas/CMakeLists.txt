# Interface libraries to set common defines and includes, avoid duplication later
add_library(z_wwcommon INTERFACE)

# GeneralsX @build BenderAI 13/02/2026 d3d8lib is INTERFACE (headers only) - safe for all
target_link_libraries(z_wwcommon INTERFACE
    core_wwcommon
    d3d8lib     # INTERFACE target: provides DXVK/DirectX types (DWORD, BOOL, etc.)
    stlport
)

# GeneralsX @build BenderAI 13/02/2026 Platform-specific multimedia stubs
if(WIN32 AND NOT SAGE_USE_SDL3)
    # Legacy 32-bit Windows: Miles audio stub (not needed for win64-modern with OpenAL)
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
# GeneralsX @bugfix BenderAI 02/03/2026 WWDownload supports both UNIX and Windows.
# registry.cpp and FTP.cpp have #ifdef _WIN32 blocks with Win32 socket/registry implementations.
# On UNIX, BSD socket networking is used; on Windows, Winsock is used.
# Both platforms need the registry functions (std::string overloads used by game UI code).
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
)
# GeneralsX @bugfix BenderAI 02/03/2026 WWDownload now built on all platforms (Windows+UNIX)
target_link_libraries(z_wwvegas INTERFACE z_wwdownload)
