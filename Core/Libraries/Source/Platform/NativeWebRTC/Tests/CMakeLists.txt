cmake_minimum_required(VERSION 3.25)

# GeneralsX @build Copilot 30/08/2026 Allow protocol tests to run standalone
# in Linux CI without configuring the full game or libdatachannel.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(generalsx_native_webrtc_tests LANGUAGES CXX)
    include(CTest)
    include(FetchContent)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    set(JSON_Install OFF CACHE INTERNAL "")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        55f93686c01528224f448c19128836e7df245f72
        GIT_SHALLOW    FALSE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

set(native_webrtc_test_sources NativeWebRTCProtocolTests.cpp)
if(NOT TARGET core_native_webrtc)
    list(APPEND native_webrtc_test_sources ../NativeWebRTCProtocol.cpp)
endif()

add_executable(core_native_webrtc_tests ${native_webrtc_test_sources})
target_include_directories(core_native_webrtc_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
if(TARGET core_native_webrtc)
    target_link_libraries(core_native_webrtc_tests PRIVATE
        core_native_webrtc
        nlohmann_json::nlohmann_json
    )
    target_compile_definitions(core_native_webrtc_tests PRIVATE NATIVE_WEBRTC_LINK_TEST)
else()
    target_link_libraries(core_native_webrtc_tests PRIVATE nlohmann_json::nlohmann_json)
endif()
target_compile_features(core_native_webrtc_tests PRIVATE cxx_std_20)
add_test(NAME core_native_webrtc_tests COMMAND core_native_webrtc_tests)

if(TARGET core_native_webrtc)
    # GeneralsX @build Copilot 30/08/2026 Build an opt-in real-network probe
    # used by the Playwright/native interoperability harness.
    add_executable(core_native_webrtc_integration_probe NativeWebRTCIntegrationProbe.cpp)
    target_include_directories(core_native_webrtc_integration_probe PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
    target_link_libraries(core_native_webrtc_integration_probe PRIVATE core_native_webrtc)
    target_compile_features(core_native_webrtc_integration_probe PRIVATE cxx_std_20)
endif()
