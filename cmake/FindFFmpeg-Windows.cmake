# GeneralsX @feature fbraz 18/05/2026
# FFmpeg via FetchContent for Windows64 (static build, app-local)
#
# Reference: openal-soft v1.24.2 fetch pattern
# Build: FFmpeg 6.1 static Windows kit (MinGW-w64 cross-compile or prebuilt)
# URL: https://github.com/Bzztock/FFmpeg.ninja or official git
#
# Strategy:
# - Windows64 (MSYS2/MinGW): FetchContent static build (no external pkg-config)
# - Linux/macOS: pkg-config via vcpkg/Homebrew (existing FindFFmpeg.cmake)
# - Components: AVFORMAT, AVCODEC, AVUTIL (intro video playback)
# - Optional: AVDEVICE, SWSCALE, SWRESAMPLE, POSTPROC, LIBASS (subtitles)
#
# Bundle location: build/windows64-deploy/ThirdParty/ffmpeg/
# DLLs: libavcodec-*.dll, libavformat-*.dll, libavutil-*.dll (and dependencies)

if(WIN32 AND RTS_BUILD_OPTION_FFMPEG)
    message(STATUS "Configuring FFmpeg for Windows64 (static build, app-local)...")

    include(FetchContent)

    # FFmpeg 6.1 static kit via git (MinGW-w64 cross-compile path)
    # Alternative: use prebuilt static from https://www.gyan.dev/ffmpeg/builds/
    FetchContent_Declare(
        ffmpeg_static
        URL "https://github.com/Bzztock/FFmpeg.ninja/archive/refs/tags/v6.1.tar.gz"
        URL_HASH "SHA256=PENDING_HASH_CHECK"
    )

    # Build: static toolchain, MinGW-w64 x86_64
    set(FFmpeg_BUILD_STATIC TRUE CACHE BOOL "Build static FFmpeg" FORCE)
    set(FFmpeg_BUILD_SHARED_LIBS FALSE CACHE BOOL "Build shared FFmpeg libs" FORCE)
    set(FFmpeg_ENABLE_PIC FALSE CACHE BOOL "Enable position-independent code" FORCE)
    set(FFmpeg_ENABLE_PROGRAMS FALSE CACHE BOOL "Build ffmpeg utility" FORCE)
    set(FFmpeg_ENABLE_TESTS FALSE CACHE BOOL "Build tests" FORCE)
    set(FFmpeg_ENABLE_PROG_TESTS FALSE CACHE BOOL "Build prog tests" FORCE)
    set(FFmpeg_ENABLE_DEveloper_PROGRAMS FALSE CACHE BOOL "Build developer tools" FORCE)
    set(FFmpeg_ENABLE_PNG FALSE CACHE BOOL "Enable PNG support" FORCE)
    set(FFmpeg_ENABLE_FREETYPE FALSE CACHE BOOL "Enable FreeType" FORCE)
    set(FFmpeg_ENABLE_FONTCONFIG FALSE CACHE BOOL "Enable fontconfig" FORCE)
    set(FFmpeg_ENABLE_FREETYPE FALSE CACHE BOOL "Enable FreeType" FORCE)
    set(FFmpeg_ENABLE_X11 FALSE CACHE BOOL "Enable X11" FORCE)
    set(FFmpeg_ENABLE_AVFILTER FALSE CACHE BOOL "Enable libavfilter" FORCE)
    
    # Note: Full FFmpeg.ninja build requires MSYS2 environment setup
    # Alternative: use prebuilt static from https://www.gyan.dev/ffmpeg/builds/
    # and copy to build/windows64-deploy/ThirdParty/ffmpeg/

    # For now: gate behind prebuilt static kit (simpler bootstrap)
    find_path(FFMPEG_INCLUDE_DIRS
        NAMES libavformat/avformat.h
        PATHS
            ${CMAKE_SOURCE_DIR}/build/windows64-deploy/ThirdParty/ffmpeg/include
            $ENV{FFMPEG_HOME}/include
            "C:/ffmpeg/include"
            "C:/msys64/usr/include"
            C:/MinGW/w64/include
            ${CMAKE_BINARY_DIR}/ThirdParty/ffmpeg/include
        PATH_SUFFIXES ffmpeg
    )

    find_library(FFMPEG_LIBRARIES
        NAMES avcodec avformat avutil
        PATHS
            ${CMAKE_SOURCE_DIR}/build/windows64-deploy/ThirdParty/ffmpeg/lib
            $ENV{FFMPEG_HOME}/lib
            "C:/ffmpeg/lib"
            ${CMAKE_BINARY_DIR}/ThirdParty/ffmpeg/lib
        PATH_SUFFIXES mingw w64
    )

    if(FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)
        # Component detection
        find_library(AVUTIL_LIBRARIES NAMES avutil PATHS ${FFMPEG_INCLUDE_DIRS}/.. PATH_SUFFIXES mingw w64)
        find_library(AVCODEC_LIBRARIES NAMES avcodec PATHS ${FFMPEG_INCLUDE_DIRS}/.. PATH_SUFFIXES mingw w64)
        find_library(AVFORMAT_LIBRARIES NAMES avformat PATHS ${FFMPEG_INCLUDE_DIRS}/.. PATH_SUFFIXES mingw w64)
        
        if(AVUTIL_LIBRARIES AND AVCODEC_LIBRARIES AND AVFORMAT_LIBRARIES)
            set(FFMPEG_FOUND TRUE)
            set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} CACHE STRING "FFmpeg include dirs" FORCE)
            set(FFMPEG_LIBRARIES ${AVUTIL_LIBRARIES};${AVCODEC_LIBRARIES};${AVFORMAT_LIBRARIES} CACHE STRING "FFmpeg libs" FORCE)

            # Load FFmpeg headers
            FetchContent_MakeAvailable(ffmpeg_static)

            message(STATUS "FFmpeg configured for Windows64: include=${FFMPEG_INCLUDE_DIRS} libs=${FFMPEG_LIBRARIES}")
        else()
            message(WARNING "FFmpeg libraries not found in expected paths. Ensure FFmpeg is installed at one of the configured paths.")
            set(FFMPEG_FOUND FALSE)
        endif()
    else()
        message(WARNING "FFmpeg includes or libs not found. Please install FFmpeg static kit or set $ENV{FFMPEG_HOME}")
        set(FFMPEG_FOUND FALSE)
    endif()

    mark_as_advanced(
        FFMPEG_INCLUDE_DIRS
        FFMPEG_LIBRARIES
        AVUTIL_LIBRARIES
        AVCODEC_LIBRARIES
        AVFORMAT_LIBRARIES
    )
else()
    message(STATUS "FFmpeg disabled on non-Windows or RTS_BUILD_OPTION_FFMPEG=OFF")
endif()