# GeneralsX @build BenderAI 29/05/2025
# ccache compiler cache support
#
# Significantly speeds up recompilation by caching object files.
# Auto-detects ccache installation; no-op if not found.
#
# Reference pattern from old-multiplatform-attempt branch:
#   CMAKE_C_COMPILER_LAUNCHER   = ccache
#   CMAKE_CXX_COMPILER_LAUNCHER = ccache

option(SAGE_USE_CCACHE "Use compiler cache (ccache/sccache) if available" ON)

if(SAGE_USE_CCACHE)
    # GeneralsX @feature fbraz 26/11/2026 Prefer ccache on Unix/macOS and support sccache on Windows MSVC
    if(WIN32)
        find_program(CCACHE_PROGRAM NAMES sccache ccache)
    else()
        find_program(CCACHE_PROGRAM NAMES ccache sccache)
    endif()
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}" CACHE STRING "C compiler launcher")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C++ compiler launcher")
        message(STATUS "Compiler cache enabled: ${CCACHE_PROGRAM}")
        
        # GeneralsX @build BenderAI 25/02/2026
        # Use CCACHE_SLOPPINESS env var so we don't mutate the global ccache config
        # as a side-effect of CMake configure.  The env var is inherited by compiler
        # invocations launched through CMAKE_<LANG>_COMPILER_LAUNCHER.
        if(APPLE)
            set(ENV{CCACHE_SLOPPINESS} "time_macros,locale")
            message(STATUS "ccache: CCACHE_SLOPPINESS=time_macros,locale set for this build")
        endif()
    else()
        message(STATUS "Compiler cache (ccache/sccache) not found, building without compiler cache")
    endif()
else()
    message(STATUS "Compiler cache disabled (SAGE_USE_CCACHE=OFF)")
endif()
