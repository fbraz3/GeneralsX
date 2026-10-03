# Windows64 Task 06 - Legacy Windows Cull

**Status**: IN PROGRESS

## Scope

Execute the removal/isolation of legacy Windows-only code that collides with the modern path.

## Files

- `Core/GameEngineDevice/CMakeLists.txt`
- `cmake/reactos-atl.cmake`
- `cmake/widl.cmake`
- Win32 display/input sources
- Legacy stubs and compatibility shims

## Deliverable

- Modern Windows64 build path is free of unnecessary legacy dependencies, with guards/isolation where removal is not yet possible.

## Implementation Reading (LLM)

- `Core/GameEngineDevice/CMakeLists.txt`
- `cmake/reactos-atl.cmake`
- `cmake/widl.cmake`
- `GeneralsMD/Code/Main/WinMain.cpp`
- `Generals/Code/Main/WinMain.cpp`
- `references/thesuperhackers-main/`

## Execution Outputs

- Keep/Isolate/Remove table committed with file-level decisions.
- CMake and source changes that enforce those decisions in build/runtime paths.
- Validation notes proving the modern path no longer drags legacy-only components by default.

## Current Implementation Progress

- Added build option `RTS_BUILD_OPTION_ISOLATE_LEGACY_WININPUT` in `cmake/config-build.cmake`.
- Enabled isolation by default in `windows64-deploy` preset.
- Added conditional source inclusion in `Core/GameEngineDevice/CMakeLists.txt`:
	- when isolation is ON: legacy Win32 input/engine sources remain excluded from modern path
	- when isolation is OFF: legacy Win32 input/engine sources are added back explicitly
- Kept `Win32BIGFile.cpp` and `Win32BIGFileSystem.cpp` in modern path as required filesystem components.

## Checklist

- Inventory legacy candidates (Win32 display/input, ATL/COM, WIDL, stubs) with owners.
- Implement `remove` decisions directly in CMake/source where safe.
- Implement `isolate` decisions behind explicit legacy guards or separate targets.
- Keep only components proven necessary for modern runtime functionality.
- Validate `windows64-deploy` configure/build after each cull batch.
- Record what remains and why, with a concrete follow-up issue/task when not removed.

## Done Criteria

- Modern Windows64 build graph is free of removable legacy-only dependencies.
- Remaining legacy pieces are explicitly isolated and documented with rationale.