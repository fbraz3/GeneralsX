# OpenAL audio library configuration
# GeneralsX @build BenderAI 26/02/2026 - Phase 6 (Windows) + fbraz 24/02/2026 - Phase 5 (macOS)
# GeneralsX @bugfix BenderAI 04/03/2026 - Use FetchContent for ALL platforms (Linux, macOS, Windows).
#   Homebrew-based detection was unreliable on Apple Silicon M1/M2/M3: Intel Homebrew (/usr/local)
#   installs x86_64-only binaries which fail to link against native arm64 builds. Using FetchContent
#   guarantees a native-architecture openal-soft compiled for the target (arm64 on Apple Silicon).
#   Apple's system OpenAL.framework is explicitly avoided: it uses <OpenAL/al.h> which is
#   incompatible with the standard <AL/al.h> used throughout jmarshall's OpenAL implementation.
#
# Platform strategy (all via FetchContent):
# - macOS:   CoreAudio backend. Compiled native arm64. No Homebrew dependency.
# - Linux:   ALSA/PipeWire backend.
# - Windows: WASAPI backend (modern, low-latency).

if(SAGE_USE_OPENAL)
    message(STATUS "Configuring OpenAL Soft (v1.24.2) with FetchContent...")

    include(FetchContent)

    FetchContent_Declare(
        openal_soft
        GIT_REPOSITORY https://github.com/kcat/openal-soft.git
        GIT_TAG        1.24.2
    )

    # Disable unnecessary build components to speed up compilation
    set(ALSOFT_INSTALL_RUNTIME_LIBS  ON  CACHE BOOL "Install runtime libs" FORCE)
    set(ALSOFT_EXAMPLES              OFF CACHE BOOL "Build examples"       FORCE)
    set(ALSOFT_TESTS                 OFF CACHE BOOL "Build tests"          FORCE)
    set(ALSOFT_UTILS                 OFF CACHE BOOL "Build utils"          FORCE)
    set(ALSOFT_NO_CONFIG_UTIL        ON  CACHE BOOL "Disable config util"  FORCE)

    # Platform-specific backend preferences
    if(WIN32)
        # Windows: WASAPI is the modern low-latency audio API
        set(ALSOFT_REQUIRE_WASAPI ON CACHE BOOL "Require WASAPI backend on Windows" FORCE)
    endif()

    FetchContent_MakeAvailable(openal_soft)

    # openal-soft FetchContent creates the OpenAL::OpenAL imported target
    message(STATUS "OpenAL Soft configured: target OpenAL::OpenAL available")
endif()
