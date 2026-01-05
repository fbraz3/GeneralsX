cmake_minimum_required(VERSION 3.25)

project(GeneralsX LANGUAGES C CXX)

# Cross-platform debug file installation macro
macro(install_debug_files TARGET DEST)
    if(WIN32)
        install(FILES $<TARGET_PDB_FILE:${TARGET}> DESTINATION "${DEST}" OPTIONAL)
    endif()
endmacro()

if (RTS_BUILD_GENERALS_DOCS)
    find_package(Doxygen REQUIRED)
    doxygen_add_docs(g_docs Code)
endif()

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Add main build targets
add_subdirectory(Code)

# If we are building on windows for windows, try and get the game install path from the registry.
if("${CMAKE_HOST_SYSTEM}" MATCHES "Windows" AND "${CMAKE_SYSTEM}" MATCHES "Windows")
    include(${CMAKE_SOURCE_DIR}/cmake/registry.cmake)

    # Check the EA App/CD registry path
    fetch_registry_value(
        "HKEY_LOCAL_MACHINE/SOFTWARE/Electronic Arts/EA Games/Generals"
        "InstallPath"
        RTS_INSTALL_PREFIX_GENERALS
        "Generals install path from registry")

    # Check the "First Decade" registry path
    fetch_registry_value(
        "HKEY_LOCAL_MACHINE/SOFTWARE/Electronic Arts/EA Games/Command and Conquer The First Decade"
        "gr_folder"
        RTS_INSTALL_PREFIX_GENERALS
        "Generals install path from registry")

    # Check the Steam registry path
    fetch_registry_value(
        "HKEY_LOCAL_MACHINE/SOFTWARE/Electronic Arts/EA Games/Generals"
        "installPath"
        RTS_INSTALL_PREFIX_GENERALS
        "Generals install path from registry")
endif()

if(RTS_INSTALL_PREFIX_GENERALS AND NOT "${RTS_INSTALL_PREFIX_GENERALS}" STREQUAL "/registry")
    install(TARGETS GeneralsX RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
    install_debug_files(GeneralsX "${RTS_INSTALL_PREFIX_GENERALS}")

    if(TARGET gworld_builder)
        install(TARGETS gworld_builder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install_debug_files(gworld_builder "${RTS_INSTALL_PREFIX_GENERALS}")
    endif()

    if(TARGET gparticle_editor)
        install(TARGETS gparticle_editor RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install_debug_files(gparticle_editor "${RTS_INSTALL_PREFIX_GENERALS}")
    endif()

    if(TARGET gdebug_window)
        install(TARGETS gdebug_window RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install_debug_files(gdebug_window "${RTS_INSTALL_PREFIX_GENERALS}")
    endif()

    if(TARGET ggui_edit)
        install(TARGETS ggui_edit RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install_debug_files(ggui_edit "${RTS_INSTALL_PREFIX_GENERALS}")
    endif()

    if(TARGET gimage_packer)
        install(TARGETS gimage_packer RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install_debug_files(gimage_packer "${RTS_INSTALL_PREFIX_GENERALS}")
    endif()

    if(TARGET gmap_cache_builder)
        install(TARGETS gmap_cache_builder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install_debug_files(gmap_cache_builder "${RTS_INSTALL_PREFIX_GENERALS}")
    endif()

    if(TARGET gw3d_view)
        install(TARGETS gw3d_view RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install_debug_files(gw3d_view "${RTS_INSTALL_PREFIX_GENERALS}")
    endif()
else()
    message(STATUS "Generals install path not found. Registry keys for 'Generals' not found. "
                   "Generals targets will be built, but not installed. "
                   "You can specify the path manually by setting RTS_INSTALL_PREFIX_GENERALS.")
endif()
