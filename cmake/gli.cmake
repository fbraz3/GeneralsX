# GLI - Generic Image Library
# Header-only library for texture manipulation
set(GLI_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/gli-src)

FetchContent_Populate(gli DOWNLOAD_EXTRACT_TIMESTAMP
    GIT_REPOSITORY https://github.com/g-truc/gli.git
    # GeneralsX @build BenderAI 26/05/2026 - Upgrade from 0.8.2.0 to master (Feb 2026 commit)
    # 0.8.2.0 (2016) breaks with GLM 1.0.3 in C++17/20 mode on MSVC due to aggregate
    # initialization changes in gli::texture constructors. Master has these fixes.
    GIT_TAG        7efdcf87dd51cfe828b0da1068f2072542b5631a
    SOURCE_DIR     ${GLI_DIR}
)

# GeneralsX @build BenderAI 26/05/2026 - Patch GLI convert_func.hpp for GLM 1.0+ compatibility.
# GLM 1.0.3 added glm::make_vec4 with the same signature as gli::make_vec4 (defined in
# gli/type.hpp). Inside the gli::detail namespace, unqualified make_vec4 calls are ambiguous
# via ADL (argument type is glm::vec<L,...>) and MSVC raises C2668.
# Fix: qualify all make_vec4 calls in convert_func.hpp as gli::make_vec4 so ADL is bypassed.
set(_gli_convert_func "${GLI_DIR}/gli/core/convert_func.hpp")
if(EXISTS "${_gli_convert_func}")
    file(READ "${_gli_convert_func}" _gli_convert_content)
    string(REPLACE
        "return make_vec4<retType, P>"
        "return gli::make_vec4<retType, P>"
        _gli_convert_patched "${_gli_convert_content}"
    )
    if(NOT _gli_convert_patched STREQUAL _gli_convert_content)
        file(WRITE "${_gli_convert_func}" "${_gli_convert_patched}")
        message(STATUS "GeneralsX: Patched gli/core/convert_func.hpp for GLM 1.0+ make_vec4 ADL conflict")
    endif()
    unset(_gli_convert_content)
    unset(_gli_convert_patched)
endif()
unset(_gli_convert_func)

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
