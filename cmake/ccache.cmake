# GeneralsX @build BenderAI 29/05/2025
# ccache compiler cache support
#
# Significantly speeds up recompilation by caching object files.
# Auto-detects ccache installation; no-op if not found.
#
# Reference pattern from old-multiplatform-attempt branch:
#   CMAKE_C_COMPILER_LAUNCHER   = ccache
#   CMAKE_CXX_COMPILER_LAUNCHER = ccache

option(SAGE_USE_CCACHE "Use ccache compiler cache if available" ON)

if(SAGE_USE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}" CACHE STRING "C compiler launcher")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C++ compiler launcher")
        message(STATUS "ccache enabled: ${CCACHE_PROGRAM}")
        
        # GeneralsX @build BenderAI 25/02/2026
        # Use CCACHE_SLOPPINESS env var so we don't mutate the global ccache config
        # as a side-effect of CMake configure.  The env var is inherited by compiler
        # invocations launched through CMAKE_<LANG>_COMPILER_LAUNCHER.
        if(APPLE)
            set(ENV{CCACHE_SLOPPINESS} "time_macros,locale")
            message(STATUS "ccache: CCACHE_SLOPPINESS=time_macros,locale set for this build")
        endif()
    else()
        message(STATUS "ccache not found, building without compiler cache")
    endif()
else()
    message(STATUS "ccache disabled (SAGE_USE_CCACHE=OFF)")
endif()
