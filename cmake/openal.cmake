# GeneralsX @build fbraz 24/02/2026
# OpenAL audio library detection
#
# On Linux, openal-soft is managed via vcpkg (see vcpkg.json). The vcpkg build compiles
# openal-soft with ALSA-only backend (no PipeWire, no PulseAudio), which avoids a SIGSEGV
# crash in the system libopenal1 1.25.1 Debian package. find_package(OpenAL) picks up the
# vcpkg-installed version automatically when the vcpkg toolchain is active.
#
# On macOS, CMake's FindOpenAL prefers Apple's deprecated OpenAL.framework which uses
# <OpenAL/al.h> instead of the standard <AL/al.h> expected by the Linux-compatible code.
# Prefer openal-soft (brew install openal-soft) which matches the Linux layout.
#
# Reference: jmarshall OpenAL implementation uses <AL/al.h> throughout.

if(APPLE)
    # Prefer openal-soft from Homebrew (Apple Silicon arm64: /opt/homebrew, Intel: /usr/local)
    # over the deprecated Apple OpenAL.framework
    foreach(_prefix /opt/homebrew/opt/openal-soft /usr/local/opt/openal-soft)
        if(EXISTS "${_prefix}/include/AL/al.h")
            set(OPENAL_INCLUDE_DIR "${_prefix}/include" CACHE PATH "OpenAL include dir" FORCE)
            find_library(OPENAL_LIBRARY
                NAMES openal OpenAL
                HINTS "${_prefix}/lib"
                NO_DEFAULT_PATH
            )
            if(OPENAL_LIBRARY)
                message(STATUS "Using openal-soft: ${_prefix}")
                break()
            endif()
        endif()
    endforeach()
endif()
