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

    message(STATUS "NGMP: nlohmann_json and libcurl configured successfully")
endif()
