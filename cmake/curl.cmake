# GeneralsX @build BenderAI 21/04/2026 libcurl integration for update checker (SAGE_UPDATE_CHECK builds only)
# Finds libcurl via vcpkg on Linux/macOS. Windows builds do not use this module.

# GeneralsX @build Copilot 30/08/2026 Reuse libcurl for short-lived TURN credential retrieval.
if(SAGE_UPDATE_CHECK OR SAGE_USE_NATIVE_WEBRTC)
    find_package(CURL REQUIRED)
    message(STATUS "libcurl found: ${CURL_VERSION_STRING}")
endif()
