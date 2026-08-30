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

set(engine_js "${OUTPUT_DIR}/GeneralsXZH.js")
set(engine_wasm "${OUTPUT_DIR}/GeneralsXZH.wasm")
file(WRITE "${engine_js}" "fixture-js")
file(WRITE "${engine_wasm}" "fixture-wasm")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DOUTPUT_PATH=${OUTPUT_DIR}/GeneralsXZH.engine-metadata.json"
        "-DENGINE_JS_PATH=${engine_js}"
        "-DENGINE_WASM_PATH=${engine_wasm}"
        "-DGENERALSX_COMPAT_CONTENT=zero-hour"
        "-DGENERALSX_COMPAT_DETERMINISTIC_MATH=true"
        "-DGENERALSX_COMPAT_ENGINE=${GENERALSX_COMPAT_ENGINE_ZERO_HOUR}"
        "-DGENERALSX_COMPAT_PROTOCOL_VALUE=${GENERALSX_COMPAT_PROTOCOL}"
        "-DGENERALSX_COMPAT_DETERMINISM=${GENERALSX_COMPAT_DETERMINISM_GAME_MATH}"
        -P "${CMAKE_CURRENT_LIST_DIR}/../../../../../../cmake/write-engine-metadata.cmake"
    RESULT_VARIABLE metadata_result)
if(NOT metadata_result EQUAL 0)
    message(FATAL_ERROR "Engine metadata generation failed")
endif()

file(SHA256 "${engine_js}" expected_js_sha256)
file(SHA256 "${engine_wasm}" expected_wasm_sha256)
file(READ "${OUTPUT_DIR}/GeneralsXZH.engine-metadata.json" metadata_json)
string(JSON metadata_engine GET "${metadata_json}" compatibility engine)
string(JSON metadata_js_sha256 GET "${metadata_json}" artifacts engine-js sha256)
string(JSON metadata_wasm_sha256 GET "${metadata_json}" artifacts engine-wasm sha256)
if(NOT metadata_engine EQUAL GENERALSX_COMPAT_ENGINE_ZERO_HOUR
    OR NOT metadata_js_sha256 STREQUAL expected_js_sha256
    OR NOT metadata_wasm_sha256 STREQUAL expected_wasm_sha256)
    message(FATAL_ERROR "Engine metadata was not bound to the fixture artifacts: ${metadata_json}")
endif()
