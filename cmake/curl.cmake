# GeneralsX @build BenderAI 21/04/2026 libcurl integration for update checker (SAGE_UPDATE_CHECK builds only)
# Finds libcurl via vcpkg across desktop platforms (Linux, macOS, and Windows win64).

if(SAGE_UPDATE_CHECK)
    find_package(CURL REQUIRED)
    message(STATUS "libcurl found: ${CURL_VERSION_STRING}")
endif()
