# SDL2 Configuration for GeneralsX
# Phase 01: SDL2 Window & Event Loop

# Try to find SDL2 - prioritize ARM64 Homebrew installation on Apple Silicon
find_package(SDL2 QUIET CONFIG 
    PATHS /opt/homebrew/lib/cmake
    NO_DEFAULT_PATH
)

# If not found in Homebrew ARM64, try system paths
if(NOT SDL2_FOUND)
    find_package(SDL2 QUIET CONFIG)
endif()

if(SDL2_FOUND)
    message(STATUS "Phase 01: SDL2 found - using SDL2 for cross-platform window support")
    message(STATUS "Phase 01: SDL2 location: ${SDL2_DIR}")
    
    # Create SDL2 interface library for easy linking
    add_library(sdl2_compat INTERFACE)
    
    target_link_libraries(sdl2_compat INTERFACE SDL2::SDL2)
    target_include_directories(sdl2_compat INTERFACE ${SDL2_INCLUDE_DIRS})
    
else()
    message(WARNING "Phase 01: SDL2 not found - trying Homebrew ARM64 location")
    
    # Try ARM64 Homebrew installation manually
    if(EXISTS /opt/homebrew/lib/libSDL2.dylib)
        message(STATUS "Phase 01: Found SDL2 at /opt/homebrew - using manual configuration")
        
        add_library(sdl2_compat INTERFACE)
        target_link_libraries(sdl2_compat INTERFACE /opt/homebrew/lib/libSDL2.dylib)
        target_include_directories(sdl2_compat INTERFACE /opt/homebrew/include)
        
    else()
        message(WARNING "Phase 01: SDL2 not found - trying to install via brew or system package manager")
        message(WARNING "Phase 01: To install SDL2 on macOS, run: brew install sdl2")
        
        # Create dummy library to prevent cmake errors
        add_library(sdl2_compat INTERFACE)
    endif()
endif()

# Set compile definitions
target_compile_definitions(sdl2_compat INTERFACE
    -DENABLE_SDL2=1
)

message(STATUS "Phase 01: SDL2 configuration completed")


