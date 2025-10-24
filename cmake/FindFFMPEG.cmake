# FindFFMPEG.cmake - Find FFmpeg libraries and headers
# Sets FFMPEG_FOUND, FFMPEG_LIBRARIES, FFMPEG_INCLUDE_DIRS

# Try using pkg-config first (macOS/Linux)
# On Apple Silicon, prioritize /opt/homebrew (ARM64 native)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    # Set PKG_CONFIG_PATH to prefer ARM64 homebrew on Apple Silicon
    if(APPLE AND CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    endif()
    pkg_check_modules(PC_FFMPEG QUIET libavcodec libavformat libavutil libswscale libswresample)
endif()

# Find include directories
find_path(FFMPEG_AVCODEC_INCLUDE_DIR
    NAMES libavcodec/avcodec.h
    HINTS ${PC_FFMPEG_INCLUDE_DIRS}
    PATH_SUFFIXES ffmpeg
)

find_path(FFMPEG_AVFORMAT_INCLUDE_DIR
    NAMES libavformat/avformat.h
    HINTS ${PC_FFMPEG_INCLUDE_DIRS}
    PATH_SUFFIXES ffmpeg
)

find_path(FFMPEG_AVUTIL_INCLUDE_DIR
    NAMES libavutil/avutil.h
    HINTS ${PC_FFMPEG_INCLUDE_DIRS}
    PATH_SUFFIXES ffmpeg
)

find_path(FFMPEG_SWSCALE_INCLUDE_DIR
    NAMES libswscale/swscale.h
    HINTS ${PC_FFMPEG_INCLUDE_DIRS}
    PATH_SUFFIXES ffmpeg
)

find_path(FFMPEG_SWRESAMPLE_INCLUDE_DIR
    NAMES libswresample/swresample.h
    HINTS ${PC_FFMPEG_INCLUDE_DIRS}
    PATH_SUFFIXES ffmpeg
)

# Find libraries
find_library(FFMPEG_AVCODEC_LIBRARY
    NAMES avcodec
    HINTS ${PC_FFMPEG_LIBRARY_DIRS}
)

find_library(FFMPEG_AVFORMAT_LIBRARY
    NAMES avformat
    HINTS ${PC_FFMPEG_LIBRARY_DIRS}
)

find_library(FFMPEG_AVUTIL_LIBRARY
    NAMES avutil
    HINTS ${PC_FFMPEG_LIBRARY_DIRS}
)

find_library(FFMPEG_SWSCALE_LIBRARY
    NAMES swscale
    HINTS ${PC_FFMPEG_LIBRARY_DIRS}
)

find_library(FFMPEG_SWRESAMPLE_LIBRARY
    NAMES swresample
    HINTS ${PC_FFMPEG_LIBRARY_DIRS}
)

# Set output variables
set(FFMPEG_INCLUDE_DIRS
    ${FFMPEG_AVCODEC_INCLUDE_DIR}
    ${FFMPEG_AVFORMAT_INCLUDE_DIR}
    ${FFMPEG_AVUTIL_INCLUDE_DIR}
    ${FFMPEG_SWSCALE_INCLUDE_DIR}
    ${FFMPEG_SWRESAMPLE_INCLUDE_DIR}
)

set(FFMPEG_LIBRARIES
    ${FFMPEG_AVCODEC_LIBRARY}
    ${FFMPEG_AVFORMAT_LIBRARY}
    ${FFMPEG_AVUTIL_LIBRARY}
    ${FFMPEG_SWSCALE_LIBRARY}
    ${FFMPEG_SWRESAMPLE_LIBRARY}
)

if(PC_FFMPEG_LIBRARY_DIRS)
    set(FFMPEG_LIBRARY_DIRS ${PC_FFMPEG_LIBRARY_DIRS})
else()
    get_filename_component(FFMPEG_LIBRARY_DIRS "${FFMPEG_AVCODEC_LIBRARY}" DIRECTORY)
endif()

list(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS)

# Handle standard find_package arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFMPEG
    REQUIRED_VARS
        FFMPEG_AVCODEC_LIBRARY
        FFMPEG_AVFORMAT_LIBRARY
        FFMPEG_AVUTIL_LIBRARY
        FFMPEG_SWSCALE_LIBRARY
        FFMPEG_SWRESAMPLE_LIBRARY
        FFMPEG_AVCODEC_INCLUDE_DIR
    VERSION_VAR PC_FFMPEG_VERSION
)

mark_as_advanced(
    FFMPEG_AVCODEC_INCLUDE_DIR
    FFMPEG_AVFORMAT_INCLUDE_DIR
    FFMPEG_AVUTIL_INCLUDE_DIR
    FFMPEG_SWSCALE_INCLUDE_DIR
    FFMPEG_SWRESAMPLE_INCLUDE_DIR
    FFMPEG_AVCODEC_LIBRARY
    FFMPEG_AVFORMAT_LIBRARY
    FFMPEG_AVUTIL_LIBRARY
    FFMPEG_SWSCALE_LIBRARY
    FFMPEG_SWRESAMPLE_LIBRARY
)
