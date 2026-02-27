# GLM - Header-Only Math Library for C++
# No compilation needed, just make headers available via CMake
set(GLM_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/glm-src)

FetchContent_Populate(glm DOWNLOAD_EXTRACT_TIMESTAMP
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
    SOURCE_DIR     ${GLM_DIR}
)

# GLM is header-only, create INTERFACE library
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${GLM_DIR})
target_compile_features(glm INTERFACE cxx_std_17)

# Add alias for find_package compatibility
add_library(glm::glm ALIAS glm)
