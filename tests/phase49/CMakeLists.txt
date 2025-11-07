# Phase 49 Test Suite - Vulkan Graphics Pipeline Tests
cmake_minimum_required(VERSION 3.20)

# Colored Quad Rendering Test
add_executable(phase49_colored_quad_test
    phase49_colored_quad_test.cpp
)

target_link_libraries(phase49_colored_quad_test
    PRIVATE
    Vulkan::Vulkan
    ww3d2
)

target_compile_features(phase49_colored_quad_test PRIVATE cxx_std_20)

# Set output directory
set_target_properties(phase49_colored_quad_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Add to test suite
add_test(
    NAME phase49_colored_quad
    COMMAND phase49_colored_quad_test 30
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)
