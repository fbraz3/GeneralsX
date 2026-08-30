set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheSuperHackers/GamespySDK.git
    GIT_TAG        07e3d15c500415abc281efb74322ab6d9c857eb8
)

FetchContent_MakeAvailable(gamespy)

# Igroteka @build 05/07/2026 - Emscripten: GameSpy's platform detection knows
# __linux__/__APPLE__ but not Emscripten. Its Linux/POSIX paths compile cleanly
# against musl headers. PUBLIC so engine translation units including GameSpy
# headers (GameNetwork) agree on the platform view; the only engine _UNIX guards
# reached this way are POSIX socket/registry paths, correct on wasm.
if(EMSCRIPTEN AND TARGET gamespy)
    # PUBLIC for consumers (engine GameNetwork translation units)...
    target_compile_definitions(gamespy PUBLIC _LINUX _UNIX)
    # ...and directory-recursive for GameSpy's own module targets (gschat, gsqr2,
    # ...), which are PRIVATE deps of gamespy and don't inherit its PUBLIC defs.
    function(igroteka_gs_defs_recursive dir)
        # __linux__ too: gsplatformsocket.c dispatches on the raw compiler macro.
        set_property(DIRECTORY ${dir} APPEND PROPERTY COMPILE_DEFINITIONS _LINUX _UNIX __linux__)
        get_property(_subs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
        foreach(_s ${_subs})
            igroteka_gs_defs_recursive(${_s})
        endforeach()
    endfunction()
    igroteka_gs_defs_recursive(${gamespy_SOURCE_DIR})
endif()
