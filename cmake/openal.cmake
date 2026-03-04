# OpenAL audio library configuration
# GeneralsX @build BenderAI 26/02/2026 - Phase 6 (Windows) + fbraz 24/02/2026 - Phase 5 (macOS)
#
# Platform strategy:
# - macOS:   Prefer openal-soft from Homebrew (brew install openal-soft) over deprecated
#            Apple OpenAL.framework (which uses <OpenAL/al.h> instead of standard <AL/al.h>).
#            jmarshall's implementation uses <AL/al.h> throughout — must match.
# - Windows: FetchContent kcat/openal-soft v1.24.2 (WASAPI backend).
# - Linux:   FetchContent kcat/openal-soft v1.24.2 (ALSA/PipeWire/WASAPI backends).

if(APPLE)
    # macOS: Prefer openal-soft from Homebrew (Apple Silicon: /opt/homebrew, Intel: /usr/local)
    foreach(_prefix /opt/homebrew/opt/openal-soft /usr/local/opt/openal-soft)
        if(EXISTS "${_prefix}/include/AL/al.h")
            set(OPENAL_INCLUDE_DIR "${_prefix}/include" CACHE PATH "OpenAL include dir" FORCE)
            find_library(OPENAL_LIBRARY
                NAMES openal OpenAL
                HINTS "${_prefix}/lib"
                NO_DEFAULT_PATH
            )
            if(OPENAL_LIBRARY)
                message(STATUS "Using openal-soft from Homebrew: ${_prefix}")
                break()
            endif()
        endif()
    endforeach()
endif()

if(SAGE_USE_OPENAL AND NOT APPLE)
    # Windows + Linux: FetchContent openal-soft v1.24.2
    # macOS is handled above via Homebrew detection
    message(STATUS "Configuring OpenAL Soft (v1.24.2) with FetchContent...")

    include(FetchContent)

    FetchContent_Declare(
        openal_soft
        GIT_REPOSITORY https://github.com/kcat/openal-soft.git
        GIT_TAG        1.24.2
    )

    # Disable unnecessary build components
    set(ALSOFT_INSTALL_RUNTIME_LIBS  ON  CACHE BOOL "Install runtime libs" FORCE)
    set(ALSOFT_EXAMPLES              OFF CACHE BOOL "Build examples"       FORCE)
    set(ALSOFT_TESTS                 OFF CACHE BOOL "Build tests"          FORCE)
    set(ALSOFT_UTILS                 OFF CACHE BOOL "Build utils"          FORCE)
    set(ALSOFT_NO_CONFIG_UTIL        ON  CACHE BOOL "Disable config util"  FORCE)

    # Windows-specific: prefer WASAPI (modern, low-latency)
    if(WIN32)
        set(ALSOFT_REQUIRE_WASAPI ON CACHE BOOL "Require WASAPI backend on Windows" FORCE)
    endif()

    FetchContent_MakeAvailable(openal_soft)

    # openal-soft creates the OpenAL::OpenAL target
    message(STATUS "OpenAL Soft configured: target OpenAL::OpenAL available")
endif()
