cmake_minimum_required(VERSION 3.25)

project(z_generals LANGUAGES C CXX)

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
    install(TARGETS z_generals RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    install(FILES $<TARGET_PDB_FILE:z_generals> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)

    if(TARGET z_worldbuilder)
        install(TARGETS z_worldbuilder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_worldbuilder> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_particleeditor)
        install(TARGETS z_particleeditor RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_particleeditor> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_debugwindow)
        install(TARGETS z_debugwindow RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_debugwindow> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_guiedit)
        install(TARGETS z_guiedit RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_guiedit> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_imagepacker)
        install(TARGETS z_imagepacker RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_imagepacker> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_mapcachebuilder)
        install(TARGETS z_mapcachebuilder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_mapcachebuilder> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_w3dview)
        install(TARGETS z_w3dview RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_w3dview> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_wdump)
        install(TARGETS z_wdump RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_wdump> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()
else()
    message(STATUS "Zero Hour install path not found. Registry keys for 'Command and Conquer Generals Zero Hour' not found. "
                   "Zero Hour targets will be built, but not installed. "
                   "You can specify the path manually by setting RTS_INSTALL_PREFIX_ZEROHOUR.")
endif()
