cmake_minimum_required(VERSION 3.25)

project(GeneralsX LANGUAGES C CXX)

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
    # Check the CD registry path
    if(NOT RTS_INSTALL_PREFIX_GENERALS)
        get_filename_component(RTS_INSTALL_PREFIX_GENERALS  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Generals;InstallPath]" ABSOLUTE CACHE)
    endif()

    # Check the "First Decade" registry path
    if(NOT RTS_INSTALL_PREFIX_GENERALS OR "${RTS_INSTALL_PREFIX_GENERALS}" STREQUAL "/registry")
        get_filename_component(RTS_INSTALL_PREFIX_GENERALS  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer The First Decade;gr_folder]" ABSOLUTE CACHE)
    endif()

    # Check the Steam registry path
    if(NOT RTS_INSTALL_PREFIX_GENERALS OR "${RTS_INSTALL_PREFIX_GENERALS}" STREQUAL "/registry")
        get_filename_component(RTS_INSTALL_PREFIX_GENERALS  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Generals;installPath]" ABSOLUTE CACHE)
    endif()
endif()

if(RTS_INSTALL_PREFIX_GENERALS AND NOT "${RTS_INSTALL_PREFIX_GENERALS}" STREQUAL "/registry")
    install(TARGETS GeneralsX RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
    install(FILES $<TARGET_PDB_FILE:GeneralsX> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)

    if(TARGET gworld_builder)
        install(TARGETS gworld_builder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:gworld_builder> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET gparticle_editor)
        install(TARGETS gparticle_editor RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:gparticle_editor> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET gdebug_window)
        install(TARGETS gdebug_window RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:gdebug_window> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET ggui_edit)
        install(TARGETS ggui_edit RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:ggui_edit> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET gimage_packer)
        install(TARGETS gimage_packer RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:gimage_packer> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET gmap_cache_builder)
        install(TARGETS gmap_cache_builder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:gmap_cache_builder> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET gw3d_view)
        install(TARGETS gw3d_view RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:gw3d_view> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()
else()
    message(STATUS "Generals install path not found. Registry keys for 'Generals' not found. "
                   "Generals targets will be built, but not installed. "
                   "You can specify the path manually by setting RTS_INSTALL_PREFIX_GENERALS.")
endif()
