# Windows64 Task 01 - Toolchain and Presets

## Scope

Execute the Windows x86_64 MinGW toolchain and `windows64-*` preset implementation.

## Files

- `CMakePresets.json`
- `cmake/toolchains/mingw-w64-x86_64.cmake`
- `cmake/mingw.cmake`
- `scripts/env/setup-mingw-windows.ps1`

## Deliverable

- `cmake --preset windows64-deploy` configures on Windows MSYS2.

## Implementation Reading (LLM)

- `CMakePresets.json`
- `cmake/toolchains/mingw-w64-x86_64.cmake`
- `cmake/mingw.cmake`
- `scripts/env/setup-mingw-windows.ps1`
- `references/fighter19-dxvk-port/CMakePresets.json`

## Execution Outputs

- Functional `windows64-deploy` and `windows64-debug` presets.
- Functional x86_64 MinGW toolchain file with MSYS2-aware resolution.
- Setup script capable of preparing a reproducible local Windows environment.

## Checklist

- Add `windows64-deploy`.
- Add `windows64-debug`.
- Add MinGW-w64 x86_64 toolchain support.
- Remove the hard fatal for MinGW x86_64 in `cmake/mingw.cmake`.
- Confirm the local MSYS2 bootstrap can provide the required compiler tools.

## Done Criteria

- `cmake --preset windows64-deploy` resolves compilers/toolchain correctly on MSYS2.
- `cmake --preset windows64-debug` resolves correctly with debug options.