add_executable(z_generals WIN32)

# Use a binary name that doesn't conflict with original game.
# GeneralsX @build BenderAI 15/02/2026 Linux branding update
# GeneralsX @build fbraz3 19/08/2026 Standardize executable name across all platforms
set_target_properties(z_generals PROPERTIES OUTPUT_NAME GeneralsXZH)

# GeneralsX @build BenderAI 13/02/2026 Windows-specific libraries
if(WIN32)
    target_link_libraries(z_generals PRIVATE
        deps_bink
        comctl32
        core_debug
        d3d8
        dinput8
        dxguid
        imm32
        deps_miles
        vfw32
        winmm
    )
endif()

# Cross-platform core libraries
target_link_libraries(z_generals PRIVATE
    core_profile_legacy
    d3dx8
    z_gameengine
    z_gameenginedevice
    zi_always
)

# Linux SDL3 windowing/input (Phase 1)
# GeneralsX @build BenderAI 11/02/2026
if(SAGE_USE_SDL3)
    target_link_libraries(z_generals PRIVATE sdl3lib)
endif()

# GeneralsX @feature Codex 04/08/2026 Link browser runtime contracts into GeneralsXZH.
if(SAGE_USE_WEBGPU)
    target_link_libraries(z_generals PRIVATE webgpu_context)
    target_link_options(z_generals PRIVATE
        "LINKER:--error-limit=0"
        "-sSUPPORT_LONGJMP=emscripten"
        # Emit an ES module only: the page around the game is a TypeScript + Tailwind app in web/.
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sENVIRONMENT=web,worker,node"
        "-lnodefs.js"
        "-sEXPORT_NAME=createGeneralsX"
        "-sEXPORTED_RUNTIME_METHODS=FS,IDBFS,NODEFS,addRunDependency,removeRunDependency,callMain,ccall"
    )
    set_target_properties(z_generals PROPERTIES SUFFIX ".js")

    set(GENERALSX_WEB_FONT_FILE "" CACHE FILEPATH "Font file preloaded by the browser build")
    if(GENERALSX_WEB_FONT_FILE)
        if(NOT EXISTS "${GENERALSX_WEB_FONT_FILE}")
            message(FATAL_ERROR "GENERALSX_WEB_FONT_FILE does not exist: ${GENERALSX_WEB_FONT_FILE}")
        endif()
        target_link_options(z_generals PRIVATE
            "SHELL:--preload-file \"${GENERALSX_WEB_FONT_FILE}@/fonts/GeneralsX.ttf\""
        )
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/assets/fonts")
        target_link_options(z_generals PRIVATE
            "SHELL:--preload-file \"${CMAKE_SOURCE_DIR}/assets/fonts@/fonts\""
        )
    endif()

    set(GENERALSX_WEB_REPLAY_FILE "" CACHE FILEPATH "Replay preloaded for deterministic browser rendering comparison")
    if(GENERALSX_WEB_REPLAY_FILE)
        if(NOT EXISTS "${GENERALSX_WEB_REPLAY_FILE}")
            message(FATAL_ERROR "GENERALSX_WEB_REPLAY_FILE does not exist: ${GENERALSX_WEB_REPLAY_FILE}")
        endif()
        target_link_options(z_generals PRIVATE
            "SHELL:--preload-file \"${GENERALSX_WEB_REPLAY_FILE}@/GeneralsReplays/visual-reference.rep\""
        )
    endif()

    set(GENERALSX_WEB_ASSET_DIR "" CACHE PATH "Directory containing legally owned Zero Hour .big archives")
    set(GENERALSX_WEB_ASSET_DIRS "" CACHE STRING "Semicolon-separated directories containing legally owned game archives")
    set(GENERALSX_WEB_ASSET_FILES "" CACHE STRING "Semicolon-separated .big archives to package for browser runtime")
    set(GENERALSX_WEB_PRELOAD_DIRS "" CACHE STRING "Semicolon-separated relative directories to package for browser runtime")
    set(web_asset_dirs "")
    if(GENERALSX_WEB_ASSET_DIR)
        list(APPEND web_asset_dirs "${GENERALSX_WEB_ASSET_DIR}")
    endif()
    list(APPEND web_asset_dirs ${GENERALSX_WEB_ASSET_DIRS})
    if(GENERALSX_WEB_ASSET_FILES AND NOT web_asset_dirs)
        message(FATAL_ERROR "GENERALSX_WEB_ASSET_FILES requires GENERALSX_WEB_ASSET_DIR or GENERALSX_WEB_ASSET_DIRS")
    endif()
    foreach(asset_file IN LISTS GENERALSX_WEB_ASSET_FILES)
        set(asset_found FALSE)
        foreach(asset_dir IN LISTS web_asset_dirs)
            if(EXISTS "${asset_dir}/${asset_file}")
                if(asset_dir STREQUAL GENERALSX_WEB_ASSET_DIR)
                    set(asset_mount_root "/GeneralsZH")
                else()
                    set(asset_mount_root "/Generals")
                endif()
                target_link_options(z_generals PRIVATE
                    "SHELL:--preload-file \"${asset_dir}/${asset_file}@${asset_mount_root}/${asset_file}\""
                )
                set(asset_found TRUE)
            endif()
        endforeach()
        if(NOT asset_found)
            message(FATAL_ERROR "Browser asset not found in configured directories: ${asset_file}")
        endif()
    endforeach()
    foreach(asset_directory IN LISTS GENERALSX_WEB_PRELOAD_DIRS)
        if(IS_ABSOLUTE "${asset_directory}" OR asset_directory MATCHES "(^|/)\\.\\.(/|$)")
            message(FATAL_ERROR "Browser preload directory must be a relative path: ${asset_directory}")
        endif()
        if(NOT IS_DIRECTORY "${GENERALSX_WEB_ASSET_DIR}/${asset_directory}")
            message(FATAL_ERROR "Browser preload directory not found in primary asset directory: ${asset_directory}")
        endif()
        target_link_options(z_generals PRIVATE
            "SHELL:--preload-file \"${GENERALSX_WEB_ASSET_DIR}/${asset_directory}@/GeneralsZH/${asset_directory}\""
        )
    endforeach()
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
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BuildVersion.h
"#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define VERSION_BUILDNUM 601
"
)

if(MSVC)
    target_link_options(z_generals PRIVATE "/NODEFAULTLIB:libci.lib")
endif()

target_include_directories(z_generals PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(z_generals PRIVATE
    $<$<BOOL:${WIN32}>:WinMain.cpp>
    $<$<BOOL:${WIN32}>:WinMain.h>
    # Linux SDL3 entry point (Phase 1.5 - TheSuperHackers @build 10/02/2026)
    $<$<NOT:$<BOOL:${WIN32}>>:SDL3Main.cpp>
    # Linux stubs for P2-P3 platform functions (P2-P3 - TheSuperHackers @build 13/02/2026 BenderAI Session 37)
    $<$<NOT:$<BOOL:${WIN32}>>:LinuxStubs.cpp>
)

# RC files optional for MinGW builds
# Icon: LoadIcon() handles missing resource gracefully (uses default system icon)
# Manifest: DPI awareness metadata (nice to have but not essential)
if(MSVC)
    # VS2005 and later adds default manifest, we need to turn it off to prevent conflict with custom manifest
    if(NOT IS_VS6_BUILD)
        target_link_options(z_generals PRIVATE
            "/MANIFEST:NO"
        )
    endif()

    target_sources(z_generals PRIVATE
        RTS.RC
    )
endif()

# Strip debug symbols to separate file for MinGW Release builds
# This creates generalszh${RTS_BUILD_OUTPUT_SUFFIX}.exe.debug (similar to MSVC .pdb files)
if(MINGW AND COMMAND add_debug_strip_target)
    add_debug_strip_target(z_generals)
endif()
