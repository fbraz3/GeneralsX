# Phase 08: SDL2 Cross-Platform Input and Windowing Library Configuration
#
# SDL2 is a cross-platform windowing and input library used for:
# - Window creation and management
# - Keyboard input handling
# - Mouse input handling
# - Event loop integration
# - Text input and IME support

# Windows VC6: Use vcpkg-installed SDL2
# Installation: vcpkg install sdl2:x86-windows

# First, try to find SDL2 in standard vcpkg install location
if(WIN32)
    set(VCPKG_DEFAULT_TRIPLET "x86-windows" CACHE STRING "vcpkg target triplet")

    # Try standard vcpkg install locations (new and old layout)
    set(VCPKG_POSSIBLE_ROOTS
        "$ENV{VCPKG_ROOT}/packages/sdl2_${VCPKG_DEFAULT_TRIPLET}"
        "$ENV{VCPKG_ROOT}/installed/${VCPKG_DEFAULT_TRIPLET}"
        "C:/vcpkg/packages/sdl2_${VCPKG_DEFAULT_TRIPLET}"
        "C:/vcpkg/installed/${VCPKG_DEFAULT_TRIPLET}"
        "C:/Users/$ENV{USERNAME}/vcpkg/packages/sdl2_${VCPKG_DEFAULT_TRIPLET}"
        "C:/Users/$ENV{USERNAME}/vcpkg/installed/${VCPKG_DEFAULT_TRIPLET}"
    )

    foreach(VCPKG_ROOT ${VCPKG_POSSIBLE_ROOTS})
        if(EXISTS "${VCPKG_ROOT}/include/SDL2/SDL.h")
            message(STATUS "Phase 08: Found SDL2 headers at ${VCPKG_ROOT}")
            set(SDL2_INCLUDE_DIR "${VCPKG_ROOT}/include" CACHE PATH "SDL2 include directory")

            # Find library
            find_library(SDL2_LIBRARY
                NAMES SDL2
                PATHS "${VCPKG_ROOT}/lib"
                NO_DEFAULT_PATH
            )

            if(SDL2_LIBRARY)
                set(SDL2_FOUND TRUE)
                message(STATUS "Phase 08: Found SDL2 library at ${SDL2_LIBRARY}")
                break()
            endif()
        endif()
    endforeach()
endif()

# Fallback: Try system find_package
if(NOT SDL2_FOUND)
    find_package(SDL2 CONFIG QUIET)
    if(NOT SDL2_FOUND)
        find_package(SDL2 QUIET)
    endif()
endif()

# Report status and create target if needed
if(SDL2_FOUND)
    message(STATUS "Phase 08: ✅ SDL2 configured successfully")

    # Create SDL2::SDL2 target if it doesn't exist
    if(NOT TARGET SDL2::SDL2)
        if(WIN32)
            # On Windows, SDL2 from vcpkg is static, but we need to link it dynamically
            # Create an interface library that adds SDL2.lib with proper DLL linking
            add_library(SDL2::SDL2 INTERFACE IMPORTED)

            set_target_properties(SDL2::SDL2 PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARY}"
            )

            # Get the SDL2 DLL directory for adding to DELAYLOAD
            get_filename_component(SDL2_LIB_DIR "${SDL2_LIBRARY}" DIRECTORY)
            set(SDL2_BIN_DIR "${SDL2_LIB_DIR}/../bin")

            find_file(SDL2_DLL
                NAMES SDL2.dll
                PATHS "${SDL2_BIN_DIR}"
                NO_DEFAULT_PATH
            )

            if(SDL2_DLL)
                message(STATUS "Phase 08: Found SDL2 DLL at ${SDL2_DLL}")
                # Mark SDL2.dll as a delayload to ensure it appears in dependencies
                set_target_properties(SDL2::SDL2 PROPERTIES
                    INTERFACE_LINK_OPTIONS "/DELAYLOAD:SDL2.dll"
                )
            endif()
        endif()
    endif()
else()
    message(FATAL_ERROR "Phase 08: ❌ SDL2 library not found!")
    message(FATAL_ERROR "  Windows VC6: vcpkg install sdl2:x86-windows")
endif()
