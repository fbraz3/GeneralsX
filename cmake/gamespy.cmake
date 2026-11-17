set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheSuperHackers/GamespySDK.git
    GIT_TAG        07e3d15c500415abc281efb74322ab6d9c857eb8
)

if(SAGE_USE_GAMESPY)
    FetchContent_MakeAvailable(gamespy)
else()
    # GeneralsX @build Codex 04/08/2026 Preserve shared data declarations without linking unsupported browser sockets.
    FetchContent_GetProperties(gamespy)
    if(NOT gamespy_POPULATED)
        FetchContent_Populate(gamespy)
    endif()
    add_library(gamespy_disabled INTERFACE)
    target_include_directories(gamespy_disabled INTERFACE
        ${gamespy_SOURCE_DIR}/include
        ${gamespy_SOURCE_DIR}/include/gamespy
    )
    add_library(gamespy::gamespy ALIAS gamespy_disabled)
endif()
