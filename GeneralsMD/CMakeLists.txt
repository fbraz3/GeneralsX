cmake_minimum_required(VERSION 3.25)

project(GeneralsXZH LANGUAGES C CXX)

# Cross-platform debug file installation macro
macro(install_debug_files TARGET DEST)
    if(WIN32)
        install(FILES $<TARGET_PDB_FILE:${TARGET}> DESTINATION "${DEST}" OPTIONAL)
    endif()
endmacro()

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
    install_debug_files(GeneralsXZH "${RTS_INSTALL_PREFIX_ZEROHOUR}")

    if(TARGET zhworld_builder)
        install(TARGETS zhworld_builder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install_debug_files(zhworld_builder "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    endif()

    if(TARGET zhparticle_editor)
        install(TARGETS zhparticle_editor RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install_debug_files(zhparticle_editor "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    endif()

    if(TARGET zhdebug_window)
        install(TARGETS zhdebug_window RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install_debug_files(zhdebug_window "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    endif()

    if(TARGET zhgui_edit)
        install(TARGETS zhgui_edit RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install_debug_files(zhgui_edit "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    endif()

    if(TARGET zhimage_packer)
        install(TARGETS zhimage_packer RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install_debug_files(zhimage_packer "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    endif()

    if(TARGET zhmap_cache_builder)
        install(TARGETS zhmap_cache_builder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install_debug_files(zhmap_cache_builder "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    endif()

    if(TARGET zhw3d_view)
        install(TARGETS zhw3d_view RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install_debug_files(zhw3d_view "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    endif()

    if(TARGET zhwdump)
        install(TARGETS zhwdump RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install_debug_files(zhwdump "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    endif()
else()
    message(STATUS "Zero Hour install path not found. Registry keys for 'Command and Conquer Generals Zero Hour' not found. "
                   "Zero Hour targets will be built, but not installed. "
                   "You can specify the path manually by setting RTS_INSTALL_PREFIX_ZEROHOUR.")
endif()
