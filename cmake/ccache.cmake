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
        # Apply time_macros sloppiness to fix 62.46% uncacheable calls issue
        # __TIME__ and __DATE__ in WinMain.cpp were breaking cache hits
        if(APPLE)
            # macOS uses both XDG and macOS-specific config paths
            execute_process(
                COMMAND ccache -o sloppiness=time_macros,locale
                RESULT_VARIABLE CCACHE_CONFIG_RESULT
            )
            if(CCACHE_CONFIG_RESULT EQUAL 0)
                message(STATUS "ccache: sloppiness configured for nondeterministic macros (time_macros,locale)")
            endif()
        endif()
    else()
        message(STATUS "ccache not found, building without compiler cache")
    endif()
else()
    message(STATUS "ccache disabled (SAGE_USE_CCACHE=OFF)")
endif()
