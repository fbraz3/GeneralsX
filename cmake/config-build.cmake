# Do we want to build extra SDK stuff or just the game binary?
option(RTS_BUILD_CORE_TOOLS "Build core tools" ON)
option(RTS_BUILD_CORE_EXTRAS "Build core extra tools/tests" OFF)
option(RTS_BUILD_ZEROHOUR "Build Zero Hour code." ON)
option(RTS_BUILD_GENERALS "Build Generals code." ON)
option(RTS_BUILD_OPTION_PROFILE "Build code with the \"Profile\" configuration." OFF)
option(RTS_BUILD_OPTION_DEBUG "Build code with the \"Debug\" configuration." OFF)
option(RTS_BUILD_OPTION_ASAN "Build code with Address Sanitizer." OFF)
option(RTS_BUILD_OPTION_VC6_FULL_DEBUG "Build VC6 with full debug info." OFF)
option(RTS_BUILD_OPTION_FFMPEG "Enable FFmpeg support" ON)

if(NOT RTS_BUILD_ZEROHOUR AND NOT RTS_BUILD_GENERALS)
    set(RTS_BUILD_ZEROHOUR TRUE)
    message("You must select one project to build, building Zero Hour by default.")
endif()

add_feature_info(CoreTools RTS_BUILD_CORE_TOOLS "Build Core Mod Tools")
add_feature_info(CoreExtras RTS_BUILD_CORE_EXTRAS "Build Core Extra Tools/Tests")
add_feature_info(ZeroHourStuff RTS_BUILD_ZEROHOUR "Build Zero Hour code")
add_feature_info(GeneralsStuff RTS_BUILD_GENERALS "Build Generals code")
add_feature_info(ProfileBuild RTS_BUILD_OPTION_PROFILE "Building as a \"Profile\" build")
add_feature_info(DebugBuild RTS_BUILD_OPTION_DEBUG "Building as a \"Debug\" build")
add_feature_info(AddressSanitizer RTS_BUILD_OPTION_ASAN "Building with address sanitizer")
add_feature_info(Vc6FullDebug RTS_BUILD_OPTION_VC6_FULL_DEBUG "Building VC6 with full debug info")
add_feature_info(FFmpegSupport RTS_BUILD_OPTION_FFMPEG "Building with FFmpeg support")

# New options that can be controlled via CMake presets
option(USE_VULKAN "Enable Vulkan backend (when available)" OFF)
option(USE_CCACHE "Enable use of ccache as compiler launcher" ON)
add_feature_info(UsingVulkan USE_VULKAN "Using Vulkan backend for rendering")
add_feature_info(UsingCcache USE_CCACHE "Using ccache for compiler launcher")

if(RTS_BUILD_ZEROHOUR)
    option(RTS_BUILD_ZEROHOUR_TOOLS "Build tools for Zero Hour" ON)
    option(RTS_BUILD_ZEROHOUR_EXTRAS "Build extra tools/tests for Zero Hour" OFF)
    option(RTS_BUILD_ZEROHOUR_DOCS "Build documentation for Zero Hour" OFF)

    add_feature_info(ZeroHourTools RTS_BUILD_ZEROHOUR_TOOLS "Build Zero Hour Mod Tools")
    add_feature_info(ZeroHourExtras RTS_BUILD_ZEROHOUR_EXTRAS "Build Zero Hour Extra Tools/Tests")
    add_feature_info(ZeroHourDocs RTS_BUILD_ZEROHOUR_DOCS "Build Zero Hour Documentation")
endif()

if(RTS_BUILD_GENERALS)
    option(RTS_BUILD_GENERALS_TOOLS "Build tools for Generals" ON)
    option(RTS_BUILD_GENERALS_EXTRAS "Build extra tools/tests for Generals" OFF)
    option(RTS_BUILD_GENERALS_DOCS "Build documentation for Generals" OFF)

    add_feature_info(GeneralsTools RTS_BUILD_GENERALS_TOOLS "Build Generals Mod Tools")
    add_feature_info(GeneralsExtras RTS_BUILD_GENERALS_EXTRAS "Build Generals Extra Tools/Tests")
    add_feature_info(GeneralsDocs RTS_BUILD_GENERALS_DOCS "Build Generals Documentation")
endif()

if(NOT IS_VS6_BUILD)
    # Because we set CMAKE_CXX_STANDARD_REQUIRED and CMAKE_CXX_EXTENSIONS in the compilers.cmake this should be enforced.
    target_compile_features(core_config INTERFACE cxx_std_20)
endif()

# Provide a compatibility include directory with minimal headers for non-Windows platforms
target_include_directories(core_config INTERFACE ${CMAKE_SOURCE_DIR}/Dependencies/Utility/Compat)

if(IS_VS6_BUILD AND RTS_BUILD_OPTION_VC6_FULL_DEBUG)
    target_compile_options(core_config INTERFACE ${RTS_FLAGS} /Zi)
else()
    target_compile_options(core_config INTERFACE ${RTS_FLAGS})
endif()

# This disables a lot of warnings steering developers to use windows only functions/function names.
if(MSVC)
    target_compile_definitions(core_config INTERFACE _CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS $<$<CONFIG:DEBUG>:_DEBUG_CRT>)
endif()

if(UNIX)
    target_compile_definitions(core_config INTERFACE _UNIX=1)
endif()

if(RTS_BUILD_OPTION_DEBUG)
    target_compile_definitions(core_config INTERFACE RTS_DEBUG WWDEBUG DEBUG)
else()
    target_compile_definitions(core_config INTERFACE RTS_RELEASE NDEBUG)
endif()

if(RTS_BUILD_OPTION_PROFILE)
    target_compile_definitions(core_config INTERFACE RTS_PROFILE)
endif()

# Honor USE_CCACHE preset variable and configure compiler launchers when requested.
if(USE_CCACHE)
    if(NOT CMAKE_C_COMPILER_LAUNCHER)
        set(CMAKE_C_COMPILER_LAUNCHER "ccache" CACHE STRING "C compiler launcher" FORCE)
    endif()
    if(NOT CMAKE_CXX_COMPILER_LAUNCHER)
        set(CMAKE_CXX_COMPILER_LAUNCHER "ccache" CACHE STRING "C++ compiler launcher" FORCE)
    endif()
    message(STATUS "Using ccache as compiler launcher (USE_CCACHE=ON)")
endif()

# Honor USE_VULKAN preset variable so top-level CMake consumes it and exposes
# a preprocessor definition to code that wants to conditionally compile Vulkan paths.
if(USE_VULKAN)
    target_compile_definitions(core_config INTERFACE USE_VULKAN=1)
    message(STATUS "Vulkan backend enabled (USE_VULKAN=ON)")
else()
    target_compile_definitions(core_config INTERFACE USE_VULKAN=0)
endif()
