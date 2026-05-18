# Windows64 Task 06 - Legacy Windows Cull

## Scope

Audit and classify legacy Windows-only code that collides with the modern path.

## Files

- `Core/GameEngineDevice/CMakeLists.txt`
- `cmake/reactos-atl.cmake`
- `cmake/widl.cmake`
- Win32 display/input sources
- Legacy stubs and compatibility shims

## Deliverable

- A keep/isolate/remove table with implementation actions.

## Checklist

- Review Win32 display and input remnants.
- Review ATL/COM and widl assumptions for Windows64 MSYS2.
- Decide what remains necessary for the modern runtime.