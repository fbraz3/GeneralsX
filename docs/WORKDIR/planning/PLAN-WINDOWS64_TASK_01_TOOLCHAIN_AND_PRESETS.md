# Windows64 Task 01 - Toolchain and Presets

## Scope

Create the Windows x86_64 MinGW toolchain and the new `windows64-*` presets.

## Files

- `CMakePresets.json`
- `cmake/toolchains/mingw-w64-x86_64.cmake`
- `cmake/mingw.cmake`
- `scripts/env/setup-mingw-windows.ps1`

## Deliverable

- `cmake --preset windows64-deploy` configures on Windows MSYS2.

## Checklist

- Add `windows64-deploy`.
- Add `windows64-debug`.
- Add MinGW-w64 x86_64 toolchain support.
- Remove the hard fatal for MinGW x86_64 in `cmake/mingw.cmake`.
- Confirm the local MSYS2 bootstrap can provide the required compiler tools.