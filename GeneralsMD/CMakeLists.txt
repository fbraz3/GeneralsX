cmake_minimum_required(VERSION 3.25)

project(GeneralsXZH LANGUAGES C CXX)

if (RTS_BUILD_ZEROHOUR_DOCS)
    find_package(Doxygen REQUIRED)
    doxygen_add_docs(z_docs Code)
endif()

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Add main build targets
add_subdirectory(Code)

# Phase 39.5: Windows Registry queries removed - using cross-platform configuration instead
# Install path is now determined at runtime via environment or configuration files
# See: docs/PHASE39/39.5_INDEX.md for configuration strategy

if(RTS_INSTALL_PREFIX_ZEROHOUR AND NOT "${RTS_INSTALL_PREFIX_ZEROHOUR}" STREQUAL "/registry")
    install(TARGETS GeneralsXZH RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    install(FILES $<TARGET_PDB_FILE:GeneralsXZH> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)

    if(TARGET zhworld_builder)
        install(TARGETS zhworld_builder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:zhworld_builder> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET zhparticle_editor)
        install(TARGETS zhparticle_editor RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:zhparticle_editor> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET zhdebug_window)
        install(TARGETS zhdebug_window RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:zhdebug_window> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET zhgui_edit)
        install(TARGETS zhgui_edit RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:zhgui_edit> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET zhimage_packer)
        install(TARGETS zhimage_packer RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:zhimage_packer> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET zhmap_cache_builder)
        install(TARGETS zhmap_cache_builder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:zhmap_cache_builder> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET zhw3d_view)
        install(TARGETS zhw3d_view RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:zhw3d_view> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET zhwdump)
        install(TARGETS zhwdump RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:zhwdump> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()
else()
    message(STATUS "Zero Hour install path not found. Registry keys for 'Command and Conquer Generals Zero Hour' not found. "
                   "Zero Hour targets will be built, but not installed. "
                   "You can specify the path manually by setting RTS_INSTALL_PREFIX_ZEROHOUR.")
endif()
