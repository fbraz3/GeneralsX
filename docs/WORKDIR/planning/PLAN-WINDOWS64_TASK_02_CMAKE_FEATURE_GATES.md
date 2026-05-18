# Windows64 Task 02 - CMake Feature Gates

## Scope

Move the Windows64 modern path away from legacy 32-bit and VC6 gating.

## Files

- `CMakeLists.txt`
- `cmake/dx8.cmake`
- `cmake/openal.cmake`
- `cmake/miles.cmake`
- `cmake/bink.cmake`
- `cmake/config-build.cmake`

## Deliverable

- The modern Windows64 path is controlled by feature flags rather than pointer-size or VC6 assumptions.

## Checklist

- Audit every `CMAKE_SIZEOF_VOID_P == 4` gate.
- Isolate Miles and Bink to the legacy-only path.
- Ensure SDL3/OpenAL/FFmpeg gating does not regress Linux or macOS.