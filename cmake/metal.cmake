# Metal configuration for GeneralsGameCode project (macOS only)

option(ENABLE_METAL "Enable Metal renderer support (macOS only)" ON)
option(DEFAULT_TO_METAL "Use Metal as default renderer on macOS" OFF)

if(APPLE AND ENABLE_METAL)
    find_library(METAL_FRAMEWORK Metal)
    find_library(METALKIT_FRAMEWORK MetalKit)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore)
    find_library(COCOA_FRAMEWORK Cocoa)

    if(NOT METAL_FRAMEWORK)
        message(FATAL_ERROR "Metal framework not found on macOS")
    endif()
    if(NOT METALKIT_FRAMEWORK)
        message(FATAL_ERROR "MetalKit framework not found on macOS")
    endif()
    if(NOT QUARTZCORE_FRAMEWORK)
        message(FATAL_ERROR "QuartzCore framework not found on macOS")
    endif()
    if(NOT COCOA_FRAMEWORK)
        message(FATAL_ERROR "Cocoa framework not found on macOS")
    endif()

    set(METAL_PLATFORM_LIBS
        ${METAL_FRAMEWORK}
        ${METALKIT_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
        ${COCOA_FRAMEWORK}
    )

    add_compile_definitions(ENABLE_METAL=1)

    if(DEFAULT_TO_METAL)
        add_compile_definitions(DEFAULT_GRAPHICS_API=METAL)
    endif()

    message(STATUS "Metal renderer enabled on macOS")
else()
    set(ENABLE_METAL OFF)
endif()

# Helper to link Metal frameworks to a target
function(add_metal_support target_name)
    if(APPLE AND ENABLE_METAL)
        # Link frameworks appropriately based on target type
        get_target_property(_t ${target_name} TYPE)
        if(_t STREQUAL "INTERFACE_LIBRARY")
            target_link_libraries(${target_name} INTERFACE ${METAL_PLATFORM_LIBS})
        else()
            target_link_libraries(${target_name} PRIVATE ${METAL_PLATFORM_LIBS})
        endif()
    endif()
endfunction()
