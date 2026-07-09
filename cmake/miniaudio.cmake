# GeneralsX @feature fbraz 11/06/2026 MiniAudio audio library via FetchContent
# miniaudio - Single file audio playback and capture library (miniaud.io)
#
# Replaces OpenAL + FFmpeg for audio decoding/playback with a single dependency.
# FFmpeg is still needed for video decoding (FFmpegVideoPlayer).
#
# Reference: https://miniaud.io/
# Reference: https://github.com/feliwir/CnC_Generals_Zero_Hour (MiniAudio backend by Stephan Vedder)

if(SAGE_USE_MINIAUDIO)
    find_path(MINIAUDIO_INCLUDE_DIR miniaudio.h)
    if(MINIAUDIO_INCLUDE_DIR)
        message(STATUS "✓ Found system miniaudio header at ${MINIAUDIO_INCLUDE_DIR}")
        add_library(miniaudio_lib INTERFACE)
        target_include_directories(miniaudio_lib INTERFACE ${MINIAUDIO_INCLUDE_DIR})
        target_compile_definitions(miniaudio_lib INTERFACE SAGE_USE_MINIAUDIO)
        
        # Platform-specific audio backend configuration
        if(APPLE)
            target_compile_definitions(miniaudio_lib INTERFACE
                MA_NO_WASAPI MA_NO_DSOUND MA_NO_WINMM)
        elseif(UNIX)
            target_compile_definitions(miniaudio_lib INTERFACE
                MA_NO_WASAPI MA_NO_DSOUND MA_NO_WINMM MA_NO_COREAUDIO)
        endif()
    else()
        message(STATUS "Configuring miniaudio via FetchContent...")

        include(FetchContent)

        FetchContent_Declare(
            miniaudio
            URL "https://github.com/mackron/miniaudio/archive/refs/heads/master.tar.gz"
        )

        FetchContent_GetProperties(miniaudio)
        if(NOT miniaudio_POPULATED)
            FetchContent_Populate(miniaudio)
        endif()

        add_library(miniaudio_lib INTERFACE)
        target_include_directories(miniaudio_lib INTERFACE ${miniaudio_SOURCE_DIR})
        target_compile_definitions(miniaudio_lib INTERFACE SAGE_USE_MINIAUDIO)

        # Platform-specific audio backend configuration
        if(APPLE)
            # Use CoreAudio on macOS
            target_compile_definitions(miniaudio_lib INTERFACE
                MA_NO_WASAPI MA_NO_DSOUND MA_NO_WINMM)
        elseif(UNIX)
            # Use ALSA on Linux (PulseAudio/PipeWire via ALSA compat)
            target_compile_definitions(miniaudio_lib INTERFACE
                MA_NO_WASAPI MA_NO_DSOUND MA_NO_WINMM MA_NO_COREAUDIO)
        endif()

        message(STATUS "miniaudio configured: target miniaudio_lib available")
    endif()
endif()
