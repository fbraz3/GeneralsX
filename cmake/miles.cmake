# GeneralsX @build GitHub Copilot 18/05/2026 Phase 2: Gate Miles for legacy-only
# Miles Sound System is legacy audio backend (Windows only)
# Modern Windows64 path uses OpenAL instead

if(NOT DEFINED MILES_ENABLED)
    # Gate Miles for legacy-only: disable when using modern stack (SDL3 + OpenAL)
    if(SAGE_USE_SDL3 AND SAGE_USE_OPENAL)
        # Modern Windows64 path: disable Miles
        set(MILES_ENABLED OFF)
        message(STATUS "Miles Sound System disabled (modern Windows64 path uses OpenAL)")
    else()
        # Legacy path: enable Miles
        set(MILES_ENABLED ON)
        message(STATUS "Miles Sound System enabled (legacy path)")
    endif()
endif()

if(MILES_ENABLED)
    FetchContent_Declare(
        miles
        GIT_REPOSITORY https://github.com/TheSuperHackers/miles-sdk-stub.git
        GIT_TAG        6e32700d7ba4b4713a03bf1f5ffc3b0ac8d17264
    )

    FetchContent_MakeAvailable(miles)
endif()
