# OpenAL Audio Backend Configuration
# Configures OpenAL framework/library for cross-platform audio support

message(STATUS "Configuring OpenAL audio backend...")

if(APPLE)
    # macOS has OpenAL framework built-in (deprecated but still available)
    find_library(OPENAL_LIBRARY OpenAL REQUIRED)
    
    if(OPENAL_LIBRARY)
        message(STATUS "Found OpenAL framework: ${OPENAL_LIBRARY}")
        
        # Create OpenAL interface target only if it doesn't exist
        if(NOT TARGET OpenAL::OpenAL)
            add_library(OpenAL::OpenAL INTERFACE IMPORTED)
            set_target_properties(OpenAL::OpenAL PROPERTIES
                INTERFACE_LINK_LIBRARIES "${OPENAL_LIBRARY}"
            )
            
            # Set include directories (OpenAL headers are in OpenAL/ subdirectory)
            set(OPENAL_INCLUDE_DIR "/System/Library/Frameworks/OpenAL.framework/Headers")
            set_target_properties(OpenAL::OpenAL PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
            )
        endif()
    else()
        message(FATAL_ERROR "OpenAL framework not found on macOS!")
    endif()
    
elseif(UNIX)
    # Linux requires OpenAL Soft library
    find_package(OpenAL)
    
    if(OPENAL_FOUND)
        message(STATUS "Found OpenAL library: ${OPENAL_LIBRARY}")
        message(STATUS "OpenAL include dir: ${OPENAL_INCLUDE_DIR}")
        
        # Create OpenAL interface target if not already created by find_package
        if(NOT TARGET OpenAL::OpenAL)
            add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
            set_target_properties(OpenAL::OpenAL PROPERTIES
                IMPORTED_LOCATION "${OPENAL_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
            )
        endif()
    else()
        message(WARNING "OpenAL not found on Linux!")
        message(STATUS "Install with: sudo apt-get install libopenal-dev")
        message(STATUS "Or on Fedora: sudo dnf install openal-soft-devel")
    endif()
    
elseif(WIN32)
    # Windows requires OpenAL Soft library (must be installed separately)
    find_package(OpenAL)
    
    if(OPENAL_FOUND)
        message(STATUS "Found OpenAL library: ${OPENAL_LIBRARY}")
        
        if(NOT TARGET OpenAL::OpenAL)
            add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
            set_target_properties(OpenAL::OpenAL PROPERTIES
                IMPORTED_LOCATION "${OPENAL_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
            )
        endif()
    else()
        message(WARNING "OpenAL not found on Windows!")
        message(STATUS "Download OpenAL Soft from: https://www.openal-soft.org/")
    endif()
    
else()
    message(WARNING "Unsupported platform for OpenAL")
endif()

message(STATUS "OpenAL configured successfully")
