# GeneralsX @build GitHub Copilot 18/05/2026 Phase 2: Gate Bink for legacy-only
# Bink video is legacy video backend (Windows only)
# Modern Windows64 path uses FFmpeg instead

if(NOT DEFINED BINK_ENABLED)
    # Gate Bink for legacy-only: disable when using modern stack (SDL3 + OpenAL + FFmpeg)
    if(SAGE_USE_SDL3 AND SAGE_USE_OPENAL AND RTS_BUILD_OPTION_FFMPEG)
        # Modern Windows64 path: disable Bink
        set(BINK_ENABLED OFF)
        message(STATUS "Bink video disabled (modern Windows64 path uses FFmpeg)")
    else()
        # Legacy path: enable Bink
        set(BINK_ENABLED ON)
        message(STATUS "Bink video enabled (legacy path)")
    endif()
endif()

if(BINK_ENABLED)
    FetchContent_Declare(
        bink
        GIT_REPOSITORY https://github.com/TheSuperHackers/bink-sdk-stub.git
        GIT_TAG        180fc4620ed72fd700347ab837a5271fd0259901
    )

    FetchContent_MakeAvailable(bink)
endif()
