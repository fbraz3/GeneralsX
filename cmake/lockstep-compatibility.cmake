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

macro(_generalsx_select_compatibility content)
    if(content STREQUAL "GENERALS")
        set(GENERALSX_COMPAT_ENGINE "${GENERALSX_COMPAT_ENGINE_GENERALS}")
        set(GENERALSX_COMPAT_CONTENT "generals")
    elseif(content STREQUAL "ZERO_HOUR")
        set(GENERALSX_COMPAT_ENGINE "${GENERALSX_COMPAT_ENGINE_ZERO_HOUR}")
        set(GENERALSX_COMPAT_CONTENT "zero-hour")
    else()
        message(FATAL_ERROR "Unknown GeneralsX compatibility content identity: ${content}")
    endif()

    if(SAGE_USE_DETERMINISTIC_MATH)
        set(GENERALSX_COMPAT_DETERMINISM "${GENERALSX_COMPAT_DETERMINISM_GAME_MATH}")
        set(GENERALSX_COMPAT_DETERMINISTIC_MATH "true")
    else()
        set(GENERALSX_COMPAT_DETERMINISM "${GENERALSX_COMPAT_DETERMINISM_PLATFORM_MATH}")
        set(GENERALSX_COMPAT_DETERMINISTIC_MATH "false")
    endif()
    set(GENERALSX_COMPAT_PROTOCOL_VALUE "${GENERALSX_COMPAT_PROTOCOL}")
endmacro()

function(generalsx_generate_compatibility_artifacts output_prefix content)
    _generalsx_select_compatibility("${content}")

    configure_file(
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../wasm/compatibility.js.in"
        "${output_prefix}.compatibility.js"
        @ONLY)
    if(NOT EMSCRIPTEN)
        configure_file(
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../wasm/compatibility.json.in"
            "${output_prefix}.compatibility.json"
            @ONLY)
    endif()
endfunction()

function(generalsx_bind_wasm_engine_metadata target output_prefix content)
    if(NOT EMSCRIPTEN)
        return()
    endif()
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "Unknown WebAssembly engine target: ${target}")
    endif()

    _generalsx_select_compatibility("${content}")
    set(metadata_path "${output_prefix}.engine-metadata.json")
    set_property(TARGET "${target}" APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/write-engine-metadata.cmake"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../wasm/engine-metadata.json.in")
    add_custom_command(
        TARGET "${target}"
        POST_BUILD
        BYPRODUCTS "${metadata_path}"
        COMMAND "${CMAKE_COMMAND}"
            "-DOUTPUT_PATH=${metadata_path}"
            "-DENGINE_JS_PATH=$<TARGET_FILE:${target}>"
            "-DENGINE_WASM_PATH=$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.wasm"
            "-DGENERALSX_COMPAT_CONTENT=${GENERALSX_COMPAT_CONTENT}"
            "-DGENERALSX_COMPAT_DETERMINISTIC_MATH=${GENERALSX_COMPAT_DETERMINISTIC_MATH}"
            "-DGENERALSX_COMPAT_ENGINE=${GENERALSX_COMPAT_ENGINE}"
            "-DGENERALSX_COMPAT_PROTOCOL_VALUE=${GENERALSX_COMPAT_PROTOCOL_VALUE}"
            "-DGENERALSX_COMPAT_DETERMINISM=${GENERALSX_COMPAT_DETERMINISM}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/write-engine-metadata.cmake"
        VERBATIM)
endfunction()
