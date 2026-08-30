foreach(required IN ITEMS
    OUTPUT_PATH
    ENGINE_JS_PATH
    ENGINE_WASM_PATH
    GENERALSX_COMPAT_CONTENT
    GENERALSX_COMPAT_DETERMINISTIC_MATH
    GENERALSX_COMPAT_ENGINE
    GENERALSX_COMPAT_PROTOCOL_VALUE
    GENERALSX_COMPAT_DETERMINISM)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is required")
    endif()
endforeach()

foreach(artifact IN ITEMS ENGINE_JS_PATH ENGINE_WASM_PATH)
    if(NOT EXISTS "${${artifact}}")
        message(FATAL_ERROR "Cannot bind compatibility metadata: ${${artifact}} does not exist")
    endif()
endforeach()

file(SHA256 "${ENGINE_JS_PATH}" GENERALSX_ENGINE_JS_SHA256)
file(SHA256 "${ENGINE_WASM_PATH}" GENERALSX_ENGINE_WASM_SHA256)
get_filename_component(GENERALSX_ENGINE_JS_FILE_NAME "${ENGINE_JS_PATH}" NAME)
get_filename_component(GENERALSX_ENGINE_WASM_FILE_NAME "${ENGINE_WASM_PATH}" NAME)

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../wasm/engine-metadata.json.in"
    "${OUTPUT_PATH}"
    @ONLY)
