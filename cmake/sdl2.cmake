# SDL2 Configuration for GeneralsX
# Phase 01: SDL2 Window & Event Loop

# Try to find SDL2 - it's more reliable than SDL3 at this point
find_package(SDL2 QUIET CONFIG)

if(SDL2_FOUND)
    message(STATUS "Phase 01: SDL2 found - using SDL2 for cross-platform window support")
    
    # Create SDL2 interface library for easy linking
    add_library(sdl2_compat INTERFACE)
    
    target_link_libraries(sdl2_compat INTERFACE SDL2::SDL2)
    target_include_directories(sdl2_compat INTERFACE ${SDL2_INCLUDE_DIRS})
    
else()
    message(WARNING "Phase 01: SDL2 not found - trying to install via brew or system package manager")
    message(WARNING "Phase 01: To install SDL2 on macOS, run: brew install sdl2")
    
    # Create dummy library to prevent cmake errors
    add_library(sdl2_compat INTERFACE)
endif()

# Set compile definitions
target_compile_definitions(sdl2_compat INTERFACE
    -DENABLE_SDL2=1
)

message(STATUS "Phase 01: SDL2 configuration completed")



