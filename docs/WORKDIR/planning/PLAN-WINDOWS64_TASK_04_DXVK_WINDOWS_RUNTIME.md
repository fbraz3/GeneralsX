# Windows64 Task 04 - DXVK Windows Runtime

## Scope

Define and implement how DXVK is used by the Windows64 modern path.

## Files

- `cmake/dx8.cmake`
- DX8/DXVK wrapper files under `Core/Libraries/Source/WWVegas/WW3D2`
- Windows runtime bundle layout files

## Deliverable

- The Windows64 modern build uses DXVK runtime assets instead of the native DX8 legacy path.

## Checklist

- Define the `d3d8.dll` bundle policy.
- Confirm load order and runtime search rules.
- Avoid any Linux/macOS regressions in the shared wrapper logic.