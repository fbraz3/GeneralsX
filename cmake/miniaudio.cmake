# GeneralsX @feature fbraz 11/06/2026 MiniAudio audio library via FetchContent
# miniaudio - Single file audio playback and capture library (miniaud.io)
#
# Replaces OpenAL + FFmpeg for audio decoding/playback with a single dependency.
# FFmpeg is still needed for video decoding (FFmpegVideoPlayer).
#
# Reference: https://miniaud.io/
# Reference: https://github.com/feliwir/CnC_Generals_Zero_Hour (MiniAudio backend by Stephan Vedder)

if(SAGE_USE_MINIAUDIO)
    message(STATUS "Configuring miniaudio via FetchContent...")

    include(FetchContent)

    FetchContent_Declare(
        miniaudio
        URL "https://github.com/mackron/miniaudio/archive/refs/heads/master.tar.gz"
    )

    # miniaudio is a single-header library; we just need the source directory
    # Use FETCHCONTENT_UPDATES_DISCONNECTED to avoid re-downloading
    FetchContent_GetProperties(miniaudio)
    if(NOT miniaudio_POPULATED)
        FetchContent_Populate(miniaudio)
    endif()

    # Create an interface target for miniaudio (single-header library)
    # We don't use miniaudio's CMakeLists.txt because it builds optional node
    # libraries with incorrect linking. Instead, we use the single-header approach.
    add_library(miniaudio_lib INTERFACE)
    target_include_directories(miniaudio_lib INTERFACE ${miniaudio_SOURCE_DIR})
    target_compile_definitions(miniaudio_lib INTERFACE SAGE_USE_MINIAUDIO)

    # Platform-specific audio backend configuration
    if(APPLE)
        # Disable CoreAudio backend on macOS to avoid Byte typedef conflict
        # between macOS MacTypes.h (UInt8) and game BaseTypeCore.h (char).
        # CoreAudio support will be re-enabled once the typedef conflict is resolved.
        target_compile_definitions(miniaudio_lib INTERFACE
            MA_NO_WASAPI MA_NO_DSOUND MA_NO_WINMM MA_NO_COREAUDIO)
    elseif(UNIX)
        # Use ALSA on Linux (PulseAudio/PipeWire via ALSA compat)
        target_compile_definitions(miniaudio_lib INTERFACE
            MA_NO_WASAPI MA_NO_DSOUND MA_NO_WINMM MA_NO_COREAUDIO)
    endif()

    message(STATUS "miniaudio configured: target miniaudio_lib available")
endif()
