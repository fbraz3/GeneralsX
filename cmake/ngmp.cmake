# GeneralsX @feature GeneralsOnline NGMP protocol dependency setup
# Integrates nlohmann_json, libcurl, and cross-platform networking primitives.

if(SAGE_USE_NGMP)
    find_package(nlohmann_json QUIET)
    if(NOT nlohmann_json_FOUND AND NOT FETCHCONTENT_FULLY_DISCONNECTED)
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
    if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/ngmp_env.cmake")
        include("${CMAKE_SOURCE_DIR}/cmake/ngmp_env.cmake")
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/.ngmp-config.cmake")
        include("${CMAKE_SOURCE_DIR}/.ngmp-config.cmake")
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/.ngmp-server-host")
        file(READ "${CMAKE_SOURCE_DIR}/.ngmp-server-host" NGMP_SERVER_HOST)
        string(STRIP "${NGMP_SERVER_HOST}" NGMP_SERVER_HOST)
        set(NGMP_SERVER_HOST "${NGMP_SERVER_HOST}" CACHE STRING "NGMP Server Host" FORCE)
    elseif(DEFINED ENV{NGMP_SERVER_HOST} AND NOT "$ENV{NGMP_SERVER_HOST}" STREQUAL "")
        set(NGMP_SERVER_HOST "$ENV{NGMP_SERVER_HOST}" CACHE STRING "NGMP Server Host" FORCE)
    elseif(DEFINED ENV{NGMP_DEFAULT_HOST} AND NOT "$ENV{NGMP_DEFAULT_HOST}" STREQUAL "")
        set(NGMP_SERVER_HOST "$ENV{NGMP_DEFAULT_HOST}" CACHE STRING "NGMP Server Host" FORCE)
    elseif(DEFINED NGMP_SERVER_HOST AND NOT "${NGMP_SERVER_HOST}" STREQUAL "")
        set(NGMP_SERVER_HOST "${NGMP_SERVER_HOST}" CACHE STRING "NGMP Server Host" FORCE)
    elseif(DEFINED NGMP_DEFAULT_HOST AND NOT "${NGMP_DEFAULT_HOST}" STREQUAL "")
        set(NGMP_SERVER_HOST "${NGMP_DEFAULT_HOST}" CACHE STRING "NGMP Server Host" FORCE)
    else()
        set(NGMP_SERVER_HOST "localhost" CACHE STRING "NGMP Server Host")
    endif()

    if(DEFINED ENV{NGMP_SERVER_PORT} AND NOT "$ENV{NGMP_SERVER_PORT}" STREQUAL "")
        set(NGMP_SERVER_PORT "$ENV{NGMP_SERVER_PORT}" CACHE STRING "NGMP Server Port" FORCE)
    elseif(NOT DEFINED NGMP_SERVER_PORT OR "${NGMP_SERVER_PORT}" STREQUAL "")
        set(NGMP_SERVER_PORT "9001" CACHE STRING "NGMP Server Port")
    endif()

    if(DEFINED ENV{NGMP_USE_SSL} AND NOT "$ENV{NGMP_USE_SSL}" STREQUAL "")
        if("$ENV{NGMP_USE_SSL}" MATCHES "^(1|ON|YES|TRUE|Y|on|yes|true|y)$")
            option(NGMP_USE_SSL "Enable SSL for NGMP protocol" ON)
        else()
            option(NGMP_USE_SSL "Enable SSL for NGMP protocol" OFF)
        endif()
    elseif(NOT DEFINED NGMP_USE_SSL)
        option(NGMP_USE_SSL "Enable SSL for NGMP protocol" OFF)
    endif()

    target_compile_definitions(core_config INTERFACE
        NGMP_DEFAULT_HOST="${NGMP_SERVER_HOST}"
        NGMP_DEFAULT_PORT="${NGMP_SERVER_PORT}"
    )

    if(NGMP_USE_SSL)
        target_compile_definitions(core_config INTERFACE NGMP_USE_SSL=1)
        message(STATUS "NGMP: Server target configured to wss://${NGMP_SERVER_HOST}:${NGMP_SERVER_PORT}")
    else()
        target_compile_definitions(core_config INTERFACE NGMP_USE_SSL=0)
        message(STATUS "NGMP: Server target configured to ws://${NGMP_SERVER_HOST}:${NGMP_SERVER_PORT}")
    endif()

    message(STATUS "NGMP: nlohmann_json and libcurl configured successfully")
endif()
