# GeneralsX @feature GeneralsOnline NGMP protocol dependency setup
# Integrates nlohmann_json, libcurl, and cross-platform networking primitives.

if(SAGE_USE_NGMP)
    find_package(nlohmann_json QUIET)
    if(NOT nlohmann_json_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            json
            URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        )
        FetchContent_MakeAvailable(json)
    endif()

    find_package(CURL REQUIRED)

    if(TARGET nlohmann_json::nlohmann_json)
        target_link_libraries(core_config INTERFACE nlohmann_json::nlohmann_json)
    elseif(TARGET nlohmann_json)
        target_link_libraries(core_config INTERFACE nlohmann_json)
    endif()

    target_link_libraries(core_config INTERFACE ${CURL_LIBRARIES})
    target_include_directories(core_config INTERFACE ${CURL_INCLUDE_DIRS})

    # NGMP Server Host & Port configuration (File > Environment > Default)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.ngmp-server-host")
        file(READ "${CMAKE_SOURCE_DIR}/.ngmp-server-host" NGMP_SERVER_HOST)
        string(STRIP "${NGMP_SERVER_HOST}" NGMP_SERVER_HOST)
        set(NGMP_SERVER_HOST "${NGMP_SERVER_HOST}" CACHE STRING "NGMP Server Host" FORCE)
    elseif(DEFINED ENV{NGMP_SERVER_HOST})
        set(NGMP_SERVER_HOST "$ENV{NGMP_SERVER_HOST}" CACHE STRING "NGMP Server Host" FORCE)
    elseif(DEFINED ENV{NGMP_DEFAULT_HOST})
        set(NGMP_SERVER_HOST "$ENV{NGMP_DEFAULT_HOST}" CACHE STRING "NGMP Server Host" FORCE)
    else()
        set(NGMP_SERVER_HOST "localhost" CACHE STRING "NGMP Server Host")
    endif()

    if(DEFINED ENV{NGMP_SERVER_PORT})
        set(NGMP_SERVER_PORT "$ENV{NGMP_SERVER_PORT}" CACHE STRING "NGMP Server Port" FORCE)
    else()
        set(NGMP_SERVER_PORT "9001" CACHE STRING "NGMP Server Port")
    endif()

    target_compile_definitions(core_config INTERFACE
        NGMP_DEFAULT_HOST="${NGMP_SERVER_HOST}"
        NGMP_DEFAULT_PORT="${NGMP_SERVER_PORT}"
    )

    message(STATUS "NGMP: Server target configured to ${NGMP_SERVER_HOST}:${NGMP_SERVER_PORT}")
    message(STATUS "NGMP: nlohmann_json and libcurl configured successfully")
endif()
