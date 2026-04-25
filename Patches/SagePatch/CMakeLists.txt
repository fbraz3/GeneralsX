# SagePatch - QoL features for GeneralsX (macOS)
# Built as a DYLD_INSERT_LIBRARIES dylib that hooks SDL3 + adds screenshot/cursor/brightness.
# Engine-side overrides (camera height, scroll speed) are shipped as Data/INI overlays.

if(NOT APPLE)
    message(STATUS "SagePatch: Currently macOS-only; skipping on this platform.")
    return()
endif()

if(NOT SAGE_USE_SDL3)
    message(FATAL_ERROR "SagePatch requires SAGE_USE_SDL3=ON.")
endif()

add_library(sage_patch SHARED
    src/Init.cpp
    src/interposers.cpp
    src/features/KeyHandler.cpp
    src/features/Screenshot.cpp
    src/features/CursorLock.cpp
    src/features/Brightness.cpp
    src/util/Logger.cpp
    src/util/Config.cpp
)

target_include_directories(sage_patch PRIVATE
    include
    ${CMAKE_BINARY_DIR}/_deps/sdl3-src/include
)

# SDL3 is interposed at runtime — link only as a build-time symbol resolver,
# do NOT make sage_patch depend on the SDL3 dylib explicitly. Resolution happens
# at load time through the host process's existing libSDL3.0.dylib.
target_link_libraries(sage_patch PRIVATE
    "-undefined dynamic_lookup"
    "-framework CoreGraphics"
    "-framework ApplicationServices"
    "-framework Foundation"
)

set_target_properties(sage_patch PROPERTIES
    OUTPUT_NAME "sage_patch"
    PREFIX "lib"
    SUFFIX ".dylib"
    INSTALL_NAME_DIR "@rpath"
    BUILD_WITH_INSTALL_RPATH TRUE
)

target_compile_options(sage_patch PRIVATE
    -fvisibility=hidden
    -fno-exceptions
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_compile_definitions(sage_patch PRIVATE
    SAGE_PATCH_VERSION="0.1.0-alpha"
)

# Resources (Override.ini for engine-side INI overrides) are picked up by deploy script
add_feature_info(SagePatch ON "QoL patch for GeneralsX (screenshot, cursor lock, camera ext., brightness)")
