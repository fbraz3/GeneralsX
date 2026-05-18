# Windows64 Task 04 - DXVK Windows Runtime

## Scope

Define and implement how DXVK is used by the Windows64 modern path.

## Files

- `cmake/dx8.cmake`
- DX8/DXVK wrapper files under `Core/Libraries/Source/WWVegas/WW3D2`
- Windows runtime bundle layout files

## Deliverable

- The Windows64 modern build uses DXVK runtime assets instead of the native DX8 legacy path.

## Implementation Reading (LLM)

- `cmake/dx8.cmake`
- `GeneralsMD/Code/CompatLib/CMakeLists.txt`
- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`
- `references/fighter19-dxvk-port/GeneralsMD/Code/CompatLib/CMakeLists.txt`

## Runtime Policy (Windows64)

- Keep compile-time DirectX8 headers from `min-dx8-sdk` for stable type/import definitions.
- Stage DXVK runtime DLLs from the DXVK release bundle `x64/` directory into the build/runtime directory:
	- `d3d8.dll`
	- `dxgi.dll`
	- `d3d11.dll`
- Keep load behavior based on `LoadLibrary("D3D8.DLL")` from the engine wrapper; Windows DLL search order resolves the application-local DXVK `d3d8.dll` first.
- Keep Linux/macOS DXVK behavior unchanged.

## Checklist

- Define the `d3d8.dll` bundle policy. (✅ DONE)
- Confirm load order and runtime search rules. (✅ DONE)
- Avoid any Linux/macOS regressions in the shared wrapper logic. (Validation tracked in Phase 8 gates)

## Done Criteria

- Windows64 modern build stages DXVK runtime DLLs for local app-first loading.
- Shared DX8 wrapper path remains compatible with Linux/macOS behavior.