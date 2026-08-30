# GeneralsX @build Copilot 30/08/2026 Fetch pinned native WebRTC dependencies
# only for explicit macOS/Linux WebRTC builds.

if(SAGE_USE_NATIVE_WEBRTC)
    if(NOT UNIX OR EMSCRIPTEN)
        message(FATAL_ERROR "SAGE_USE_NATIVE_WEBRTC is supported only by native macOS/Linux builds")
    endif()

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
