
add_executable(g_generals WIN32)

# GeneralsX @feature GitHub Copilot 20/05/2026 Keep executable naming consistent across Linux/macOS/Windows.
set_target_properties(g_generals PROPERTIES OUTPUT_NAME "GeneralsX")


# GeneralsX @build BenderAI 10/03/2026 Windows-specific libraries
if(WIN32)
    target_link_libraries(g_generals PRIVATE
        comctl32
        dinput8
        dxguid
        imm32
        vfw32
        winmm
    )
    if(TARGET binkstub)
        target_link_libraries(g_generals PRIVATE binkstub)
    endif()
    if(TARGET milesstub)
        target_link_libraries(g_generals PRIVATE milesstub)
    endif()
endif()


# Cross-platform core libraries
target_link_libraries(g_generals PRIVATE
    g_gameengine
    g_gameenginedevice
    gi_always
)

# Linux SDL3 windowing/input
# GeneralsX @build BenderAI 11/02/2026
if(SAGE_USE_SDL3)
    target_link_libraries(g_generals PRIVATE sdl3lib)
endif()

# TODO Originally referred to build host and user, replace with git info perhaps?
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GeneratedVersion.h
"#pragma once

#define VERSION_LOCALBUILDNUM 0
#define VERSION_BUILDUSER \"\"
#define VERSION_BUILDLOC \"\"
"
)

# Based on original binary values for these variables.
if (IS_VS6_BUILD)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 7
#define VERSION_BUILDNUM 601
"
)
else()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 8
#define VERSION_BUILDNUM 601
"
)
endif()

if(MSVC)
    target_link_options(g_generals PRIVATE "/NODEFAULTLIB:libci.lib")
endif()

target_include_directories(g_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Code/Libraries
)

# GeneralsX @bugfix GitHub Copilot 25/05/2026 Ensure g_generals also gets explicit FFmpeg include paths to keep FFmpeg source compilation consistent with ZH target behavior.
if(RTS_BUILD_OPTION_FFMPEG)
    find_package(FFmpeg QUIET)
    if(FFMPEG_FOUND)
        target_include_directories(g_generals PRIVATE ${FFMPEG_INCLUDE_DIRS})
    else()
        message(WARNING "RTS_BUILD_OPTION_FFMPEG=ON but FFmpeg include directories were not found for g_generals.")
    endif()
endif()

if(MINGW)
    # GeneralsX @bugfix GitHub Copilot 22/05/2026 Ensure MinGW SDK headers are searched before CompatLib wrappers for this Win32 entrypoint target.
    get_filename_component(_mingw_bin_dir "${CMAKE_C_COMPILER}" DIRECTORY)
    get_filename_component(_mingw_root_dir "${_mingw_bin_dir}/.." ABSOLUTE)
    set(_mingw_include_dir "${_mingw_root_dir}/include")
    if(EXISTS "${_mingw_include_dir}/windows.h")
        target_include_directories(g_generals BEFORE PRIVATE "${_mingw_include_dir}")
    endif()
endif()

target_sources(g_generals PRIVATE
    $<$<BOOL:${WIN32}>:WinMain.cpp>
    $<$<BOOL:${WIN32}>:WinMain.h>
    # Linux SDL3 entry point
    $<$<NOT:$<BOOL:${WIN32}>>:SDL3Main.cpp>
    # Linux stubs
    $<$<NOT:$<BOOL:${WIN32}>>:LinuxStubs.cpp>
)

# RC files for Windows builds (MSVC + MinGW)
# GeneralsX @feature GitHub Copilot 20/05/2026 Embed app icon and manifest on all Windows toolchains.
# TYPELIB: Broken (0 bytes) and causes windres memory exhaustion errors
if(WIN32)
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(MSVC AND NOT IS_VS6_BUILD)
        target_link_options(g_generals PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    if(MINGW)
        # GeneralsX @bugfix GitHub Copilot 22/05/2026 Use a minimal dedicated windres invocation to avoid memory exhaustion from inherited C/C++ flags.
        set(g_generals_rc_obj "${CMAKE_CURRENT_BINARY_DIR}/RTS.RC.res")
        add_custom_command(
            OUTPUT ${g_generals_rc_obj}
            COMMAND ${CMAKE_RC_COMPILER} -O coff -I "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/RTS.RC" "${g_generals_rc_obj}"
            DEPENDS RTS.RC resource.h
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Compiling RTS.RC with minimal windres flags"
            VERBATIM
        )
        target_sources(g_generals PRIVATE ${g_generals_rc_obj})
    else()
        target_sources(g_generals PRIVATE
            RTS.RC
        )
    endif()
endif()

# GeneralsX @bugfix GitHub Copilot 22/05/2026 Keep MinGW static-library resolution stable by linking dependent Windows libs/stubs after all game/static libraries.
if(WIN32)
    target_link_libraries(g_generals PRIVATE
        d3dx8
        ws2_32
    )
    if(TARGET binkstub)
        target_link_libraries(g_generals PRIVATE binkstub)
    endif()
    if(TARGET milesstub)
        target_link_libraries(g_generals PRIVATE milesstub)
    endif()
endif()

# Strip debug symbols to separate file for MinGW Release builds
# This creates generalsv${RTS_BUILD_OUTPUT_SUFFIX}.exe.debug (similar to MSVC .pdb files)
if(MINGW AND COMMAND add_debug_strip_target)
    add_debug_strip_target(g_generals)
endif()
