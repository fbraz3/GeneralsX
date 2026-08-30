cmake_minimum_required(VERSION 3.25)

# GeneralsX @build Copilot 30/08/2026 Build the opt-in native WebRTC transport
# as a shared Core platform service for Generals and Zero Hour.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(generalsx_native_webrtc LANGUAGES C CXX)
    include(CTest)
    include(FetchContent)
    find_package(CURL REQUIRED)

    set(NO_MEDIA ON CACHE BOOL "Disable libdatachannel media support" FORCE)
    set(NO_EXAMPLES ON CACHE BOOL "Disable libdatachannel examples" FORCE)
    set(NO_TESTS ON CACHE BOOL "Disable libdatachannel tests" FORCE)
    set(NO_WEBSOCKET OFF CACHE BOOL "Keep libdatachannel WebSocket signaling" FORCE)
    set(WARNINGS_AS_ERRORS OFF CACHE BOOL "Do not promote dependency warnings" FORCE)
    set(RTC_UPDATE_VERSION_HEADER OFF CACHE BOOL "Do not modify fetched sources" FORCE)
    FetchContent_Declare(
        libdatachannel
        GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
        GIT_TAG        443f6934d9007eb7076ab7825ba330f355fcbead
        GIT_SHALLOW    FALSE
        GIT_SUBMODULES_RECURSE TRUE
    )
    FetchContent_MakeAvailable(libdatachannel)

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

add_library(core_native_webrtc STATIC
    NativeWebRTCProtocol.cpp
    NativeWebRTCProtocol.h
    NativeWebRTCTransport.cpp
    NativeWebRTCTransport.h
)

target_include_directories(core_native_webrtc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_native_webrtc
    PRIVATE
        LibDataChannel::LibDataChannelStatic
        nlohmann_json::nlohmann_json
        CURL::libcurl
)

target_compile_features(core_native_webrtc PRIVATE cxx_std_20)

if(BUILD_TESTING)
    add_subdirectory(Tests)
endif()
