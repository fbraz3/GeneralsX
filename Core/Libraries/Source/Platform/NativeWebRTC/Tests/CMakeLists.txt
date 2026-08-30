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

if(NOT DEFINED GENERALSX_COMPAT_PROTOCOL)
    include("${CMAKE_CURRENT_LIST_DIR}/../../../../../../cmake/lockstep-compatibility.cmake")
endif()

set(native_webrtc_test_sources NativeWebRTCProtocolTests.cpp)
if(NOT TARGET core_native_webrtc)
    list(APPEND native_webrtc_test_sources ../NativeWebRTCProtocol.cpp)
endif()

add_executable(core_native_webrtc_tests ${native_webrtc_test_sources})
target_include_directories(core_native_webrtc_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_compile_definitions(core_native_webrtc_tests PRIVATE
    GENERALSX_COMPAT_ENGINE_GENERALS=${GENERALSX_COMPAT_ENGINE_GENERALS}
    GENERALSX_COMPAT_ENGINE_ZERO_HOUR=${GENERALSX_COMPAT_ENGINE_ZERO_HOUR}
    GENERALSX_COMPAT_PROTOCOL=${GENERALSX_COMPAT_PROTOCOL}
    GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH=${GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH}
    GENERALSX_COMPAT_DETERMINISM_GAME_MATH=${GENERALSX_COMPAT_DETERMINISM_GAME_MATH}
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
add_test(
    NAME lockstep_compatibility_artifacts
    COMMAND ${CMAKE_COMMAND}
        -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/compatibility-artifacts
        -P ${CMAKE_CURRENT_SOURCE_DIR}/LockstepCompatibilityCMakeTest.cmake)

foreach(profile IN ITEMS generals_game_math zero_hour_platform_math)
    add_executable(core_native_webrtc_profile_${profile} NativeWebRTCBuildProfileTests.cpp)
    target_include_directories(core_native_webrtc_profile_${profile} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
    target_compile_definitions(core_native_webrtc_profile_${profile} PRIVATE
        GENERALSX_COMPAT_ENGINE_GENERALS=${GENERALSX_COMPAT_ENGINE_GENERALS}
        GENERALSX_COMPAT_ENGINE_ZERO_HOUR=${GENERALSX_COMPAT_ENGINE_ZERO_HOUR}
        GENERALSX_COMPAT_PROTOCOL=${GENERALSX_COMPAT_PROTOCOL}
        GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH=${GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH}
        GENERALSX_COMPAT_DETERMINISM_GAME_MATH=${GENERALSX_COMPAT_DETERMINISM_GAME_MATH}
    )
    target_compile_features(core_native_webrtc_profile_${profile} PRIVATE cxx_std_20)
    add_test(
        NAME core_native_webrtc_profile_${profile}
        COMMAND core_native_webrtc_profile_${profile})
endforeach()

target_compile_definitions(core_native_webrtc_profile_generals_game_math PRIVATE
    RTS_GENERALS=1
    GENERALSX_DETERMINISTIC_MATH_ENABLED=1
    EXPECTED_ENGINE_COMPATIBILITY=${GENERALSX_COMPAT_ENGINE_GENERALS}
    EXPECTED_DETERMINISM_COMPATIBILITY=${GENERALSX_COMPAT_DETERMINISM_GAME_MATH}
)
target_compile_definitions(core_native_webrtc_profile_zero_hour_platform_math PRIVATE
    RTS_ZEROHOUR=1
    GENERALSX_DETERMINISTIC_MATH_ENABLED=0
    EXPECTED_ENGINE_COMPATIBILITY=${GENERALSX_COMPAT_ENGINE_ZERO_HOUR}
    EXPECTED_DETERMINISM_COMPATIBILITY=${GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH}
)

if(TARGET core_native_webrtc)
    # GeneralsX @build Copilot 30/08/2026 Build an opt-in real-network probe
    # used by the Playwright/native interoperability harness.
    add_executable(core_native_webrtc_integration_probe NativeWebRTCIntegrationProbe.cpp)
    target_include_directories(core_native_webrtc_integration_probe PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
    target_link_libraries(core_native_webrtc_integration_probe PRIVATE core_native_webrtc)
    target_compile_definitions(core_native_webrtc_integration_probe PRIVATE
        RTS_ZEROHOUR=1
        GENERALSX_DETERMINISTIC_MATH_ENABLED=$<BOOL:${SAGE_USE_DETERMINISTIC_MATH}>
    )
    target_compile_features(core_native_webrtc_integration_probe PRIVATE cxx_std_20)
    generalsx_generate_compatibility_artifacts(
        "${CMAKE_BINARY_DIR}/GeneralsXZH"
        ZERO_HOUR)
endif()
