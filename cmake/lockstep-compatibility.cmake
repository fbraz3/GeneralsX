include_guard(GLOBAL)

set(GENERALSX_LOCKSTEP_COMPATIBILITY_PATH
    "${CMAKE_CURRENT_LIST_DIR}/../web/packages/shared/src/lockstep-compatibility.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${GENERALSX_LOCKSTEP_COMPATIBILITY_PATH}")
file(READ "${GENERALSX_LOCKSTEP_COMPATIBILITY_PATH}"
    GENERALSX_LOCKSTEP_COMPATIBILITY_JSON)
string(JSON GENERALSX_COMPAT_ENGINE_GENERALS
    GET "${GENERALSX_LOCKSTEP_COMPATIBILITY_JSON}" engine generals)
string(JSON GENERALSX_COMPAT_ENGINE_ZERO_HOUR
    GET "${GENERALSX_LOCKSTEP_COMPATIBILITY_JSON}" engine zeroHour)
string(JSON GENERALSX_COMPAT_PROTOCOL
    GET "${GENERALSX_LOCKSTEP_COMPATIBILITY_JSON}" protocol)
string(JSON GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH
    GET "${GENERALSX_LOCKSTEP_COMPATIBILITY_JSON}" determinism platformMath)
string(JSON GENERALSX_COMPAT_DETERMINISM_GAME_MATH
    GET "${GENERALSX_LOCKSTEP_COMPATIBILITY_JSON}" determinism gameMath)

function(generalsx_generate_compatibility_artifacts output_prefix content)
    if(content STREQUAL "GENERALS")
        set(GENERALSX_COMPAT_ENGINE "${GENERALSX_COMPAT_ENGINE_GENERALS}")
    elseif(content STREQUAL "ZERO_HOUR")
        set(GENERALSX_COMPAT_ENGINE "${GENERALSX_COMPAT_ENGINE_ZERO_HOUR}")
    else()
        message(FATAL_ERROR "Unknown GeneralsX compatibility content identity: ${content}")
    endif()

    if(SAGE_USE_DETERMINISTIC_MATH)
        set(GENERALSX_COMPAT_DETERMINISM "${GENERALSX_COMPAT_DETERMINISM_GAME_MATH}")
    else()
        set(GENERALSX_COMPAT_DETERMINISM "${GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH}")
    endif()
    set(GENERALSX_COMPAT_PROTOCOL_VALUE "${GENERALSX_COMPAT_PROTOCOL}")

    configure_file(
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../wasm/compatibility.js.in"
        "${output_prefix}.compatibility.js"
        @ONLY)
    configure_file(
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../wasm/compatibility.json.in"
        "${output_prefix}.compatibility.json"
        @ONLY)
endfunction()
