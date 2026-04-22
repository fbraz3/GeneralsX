# GeneralsX @build BenderAI 21/04/2026 libcurl integration for update checker (SAGE_UPDATE_CHECK builds only)
# Finds libcurl via vcpkg on Linux/macOS. Windows builds do not use this module.

if(SAGE_UPDATE_CHECK)
    find_package(CURL CONFIG REQUIRED)
    message(STATUS "libcurl found: ${CURL_VERSION_STRING}")
endif()
