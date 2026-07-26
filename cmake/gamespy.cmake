set(GS_OPENSSL FALSE)

if(DEFINED ENV{GAMESPY_SERVER_NAME})
    set(GAMESPY_SERVER_NAME "$ENV{GAMESPY_SERVER_NAME}" CACHE STRING "Gamespy Server Name" FORCE)
else()
    set(GAMESPY_SERVER_NAME "gamespy.local" CACHE STRING "Gamespy Server Name")
endif()

add_compile_definitions(GSI_DOMAIN_NAME="${GAMESPY_SERVER_NAME}")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/fbraz3/GamespySDK.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(gamespy)
