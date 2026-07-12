# GeneralsX @build AI 11/07/2026 Unit Tests setup

project(GeneralsXTests)

# Add the test executable
add_executable(GeneralsXTests
    PathUtilTests.cpp
    VectorTests.cpp
    MatrixTests.cpp
    QuatTests.cpp
)

set_target_properties(GeneralsXTests PROPERTIES 
    CXX_STANDARD 17 
    CXX_STANDARD_REQUIRED YES
)
target_compile_features(GeneralsXTests PRIVATE cxx_std_17)

if(NOT IS_VS6_BUILD)
    target_compile_options(GeneralsXTests PRIVATE -include Utility/CppMacros.h)
endif()

# Link against GTest (which is fetched in gtest.cmake and provides gtest_main target globally)
# and the core libraries.
target_link_libraries(GeneralsXTests
    gtest_main
    core_wwcommon
    core_wwvegas
    gamemath
)

# Add include directories for tests
target_include_directories(GeneralsXTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Include
    ${CMAKE_SOURCE_DIR}/Dependencies/Utility
)

# Add the test to CTest
include(GoogleTest)
gtest_discover_tests(GeneralsXTests)
