# GeneralsX @feature fbraz 03/05/2026
# Deterministic cross-platform math library integration (Phase 4)
# GameMath: fdlibm-based deterministic math functions for bit-exact replay validation
#
# Strategy:
# - Enable deterministic math via FetchContent (not on VC6, which uses x87 asm)
# - Define USE_DETERMINISTIC_MATH compile flag when enabled
# - Wrappers in wwmath.h conditionally dispatch to GameMath (deterministic) or CRT (fast fallback)
#
# Reference: Okladnoj et al., PR #2670, TheSuperHackers/GeneralsGameCode
# https://github.com/TheSuperHackers/GeneralsGameCode/pull/2670
#
# Note: GameMath source location is configurable via SAGE_GAMEMATH_GIT_REPO
# Default: TheSuperHackers fork with deterministic math integration
#
# Upstream reference: fdlibm (Berkeley math library) provides platform-independent
# implementations of standard math functions (sin, cos, sqrt, atan2, etc.) that produce
# identical results on all architectures when compiled with the same precision flags.

# Enable deterministic math only for non-VC6 builds (VC6 uses native x87 asm)
# Note: Currently defaults to OFF until GameMath is available as a proper library/submodule
if(NOT IS_VS6_BUILD)
    option(SAGE_USE_DETERMINISTIC_MATH "Use fdlibm-based deterministic math for cross-platform replay validation" OFF)
else()
    # VC6 uses native x87 inline asm; deterministic mode not applicable
    set(SAGE_USE_DETERMINISTIC_MATH OFF)
endif()

if(SAGE_USE_DETERMINISTIC_MATH)
    message(STATUS "Configuring GameMath (fdlibm-based deterministic math)...")

    include(FetchContent)

    # FetchContent declaration for GameMath library
    # Source: TheSuperHackers fork with deterministic math support
    # Can be overridden via cmake -DSAGE_GAMEMATH_GIT_REPO=<url> -DSAGE_GAMEMATH_GIT_TAG=<tag>
    if(NOT SAGE_GAMEMATH_GIT_REPO)
        set(SAGE_GAMEMATH_GIT_REPO "https://github.com/TheSuperHackers/GeneralsGameCode.git")
    endif()
    
    if(NOT SAGE_GAMEMATH_GIT_TAG)
        set(SAGE_GAMEMATH_GIT_TAG "main")
    endif()

    FetchContent_Declare(
        gamemath
        GIT_REPOSITORY ${SAGE_GAMEMATH_GIT_REPO}
        GIT_TAG ${SAGE_GAMEMATH_GIT_TAG}
        SOURCE_SUBDIR "Core/GameMath"  # Adjust if GameMath moves
    )

    # Minimal GameMath configuration
    set(GAMEMATH_ENABLE_TESTS OFF CACHE BOOL "Disable GameMath tests" FORCE)
    set(GAMEMATH_ENABLE_EXAMPLES OFF CACHE BOOL "Disable GameMath examples" FORCE)

    # Make GameMath available (FetchContent_MakeAvailable is idempotent)
    FetchContent_MakeAvailable(gamemath)

    # Add USE_DETERMINISTIC_MATH to all compile definitions for this project
    # This enables conditional compilation in wwmath.h and trig wrappers
    add_compile_definitions(USE_DETERMINISTIC_MATH)

    message(STATUS "GameMath deterministic math enabled (fdlibm backend)")
    message(STATUS "  Math operations will be bit-exact across platforms")
    message(STATUS "  Performance: Slightly slower than CRT but guarantees replay determinism")

else()
    message(STATUS "Deterministic math disabled (SAGE_USE_DETERMINISTIC_MATH=OFF)")
    message(STATUS "  Math operations will use platform-native CRT/x87")
    message(STATUS "  Note: Replays may differ between platforms due to FMA/rounding differences")
endif()
