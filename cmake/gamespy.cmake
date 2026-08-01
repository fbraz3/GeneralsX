set(GS_OPENSSL FALSE)

if(EXISTS "${CMAKE_SOURCE_DIR}/.gamespy-server-name")
    file(READ "${CMAKE_SOURCE_DIR}/.gamespy-server-name" GAMESPY_SERVER_NAME)
    string(STRIP "${GAMESPY_SERVER_NAME}" GAMESPY_SERVER_NAME)
    set(GAMESPY_SERVER_NAME "${GAMESPY_SERVER_NAME}" CACHE STRING "Gamespy Server Name" FORCE)
elseif(DEFINED ENV{GAMESPY_SERVER_NAME})
    set(GAMESPY_SERVER_NAME "$ENV{GAMESPY_SERVER_NAME}" CACHE STRING "Gamespy Server Name" FORCE)
else()
    set(GAMESPY_SERVER_NAME "gamespy.local" CACHE STRING "Gamespy Server Name")
endif()

add_compile_definitions(GSI_DOMAIN_NAME="${GAMESPY_SERVER_NAME}")

option(SAGE_GAMESPY_USE_LOCAL_FORK "Use local GamespySDK in references/GamespySDK" ON)

if(SAGE_GAMESPY_USE_LOCAL_FORK AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/references/GamespySDK/CMakeLists.txt")
    message(STATUS "GamespySDK: Using local repository in references/GamespySDK")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/references/GamespySDK" "${CMAKE_BINARY_DIR}/_deps/gamespy-build")
else()
    FetchContent_Declare(
        gamespy
        GIT_REPOSITORY https://github.com/fbraz3/GamespySDK.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(gamespy)
endif()
