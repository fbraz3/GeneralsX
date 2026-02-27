# OpenAL Soft audio library
# GeneralsX @build BenderAI 26/02/2026 - Phase 06
# Cross-platform OpenAL implementation via kcat/openal-soft.
# Used on Linux and Windows Modern builds (SAGE_USE_OPENAL=ON).
# On Linux: WASAPI/ALSA/PipeWire backends.
# On Windows: WASAPI backend (primary), WinMM fallback.

if(SAGE_USE_OPENAL)
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
