set(gamemath_SHARED_LIBS OFF CACHE BOOL "Force GameMath as static lib" FORCE)
set(GM_ENABLE_INTRINSICS OFF CACHE BOOL "Disable intrinsics for cross-arch determinism" FORCE)
set(GM_ENABLE_TESTS OFF CACHE BOOL "Disable GameMath tests" FORCE)

FetchContent_Declare(
    gamemath
    GIT_REPOSITORY https://github.com/TheSuperHackers/GameMath.git
    GIT_TAG        59f7ccd494f7e7c916a784ac26ef266f9f09d78d
)

FetchContent_MakeAvailable(gamemath)

# Ensure gamemath has position-independent code for 64-bit / PIE targets
if(TARGET gamemath)
    set_target_properties(gamemath PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Ensure GameMath includes are available to ALL targets
# to prevent one-definition-rule violations and ensure USE_DETERMINISTIC_MATH activates consistently.
include_directories(${gamemath_SOURCE_DIR}/include)

