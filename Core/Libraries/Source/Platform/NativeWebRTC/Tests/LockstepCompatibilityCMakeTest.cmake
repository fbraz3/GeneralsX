if(NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "OUTPUT_DIR is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../../../../../../cmake/lockstep-compatibility.cmake")
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

function(assert_profile path expected_engine expected_determinism)
    file(READ "${path}" profile_json)
    string(JSON engine GET "${profile_json}" engine)
    string(JSON protocol GET "${profile_json}" protocol)
    string(JSON determinism GET "${profile_json}" determinism)
    if(NOT engine EQUAL expected_engine
        OR NOT protocol EQUAL GENERALSX_COMPAT_PROTOCOL
        OR NOT determinism EQUAL expected_determinism)
        message(FATAL_ERROR "Unexpected compatibility profile in ${path}: ${profile_json}")
    endif()
endfunction()

set(SAGE_USE_DETERMINISTIC_MATH ON)
generalsx_generate_compatibility_artifacts("${OUTPUT_DIR}/GeneralsX" GENERALS)
generalsx_generate_compatibility_artifacts("${OUTPUT_DIR}/GeneralsXZH" ZERO_HOUR)
assert_profile(
    "${OUTPUT_DIR}/GeneralsX.compatibility.json"
    "${GENERALSX_COMPAT_ENGINE_GENERALS}"
    "${GENERALSX_COMPAT_DETERMINISM_GAME_MATH}")
assert_profile(
    "${OUTPUT_DIR}/GeneralsXZH.compatibility.json"
    "${GENERALSX_COMPAT_ENGINE_ZERO_HOUR}"
    "${GENERALSX_COMPAT_DETERMINISM_GAME_MATH}")

set(SAGE_USE_DETERMINISTIC_MATH OFF)
generalsx_generate_compatibility_artifacts("${OUTPUT_DIR}/GeneralsXZH-platform-math" ZERO_HOUR)
assert_profile(
    "${OUTPUT_DIR}/GeneralsXZH-platform-math.compatibility.json"
    "${GENERALSX_COMPAT_ENGINE_ZERO_HOUR}"
    "${GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH}")
