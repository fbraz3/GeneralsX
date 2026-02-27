# GLI - Generic Image Library
# Header-only library for texture manipulation
set(GLI_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/gli-src)

FetchContent_Populate(gli DOWNLOAD_EXTRACT_TIMESTAMP
    GIT_REPOSITORY https://github.com/g-truc/gli.git
    GIT_TAG        0.8.2.0
    SOURCE_DIR     ${GLI_DIR}
)

# GLI is header-only, create INTERFACE library
add_library(gli INTERFACE)
target_include_directories(gli INTERFACE ${GLI_DIR})
target_compile_features(gli INTERFACE cxx_std_17)

# GLI depends on GLM
if(TARGET glm::glm)
    target_link_libraries(gli INTERFACE glm::glm)
endif()

# Add alias for find_package compatibility
add_library(gli::gli ALIAS gli)
