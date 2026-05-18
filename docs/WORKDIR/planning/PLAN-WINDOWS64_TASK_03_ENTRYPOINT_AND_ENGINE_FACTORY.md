# Windows64 Task 03 - Entry Point and Engine Factory

## Scope

Execute Windows64 startup and engine selection changes to use the SDL3-based modern path.

## Files

- `GeneralsMD/Code/Main/WinMain.cpp`
- `GeneralsMD/Code/Main/SDL3Main.cpp`
- `Generals/Code/Main/WinMain.cpp`
- `Core/GameEngineDevice/Include/SDL3GameEngine.h`
- Related engine factory code in main/device layers

## Deliverable

- Windows64 modern builds choose the SDL3-based engine path without breaking Linux or macOS.

## Implementation Reading (LLM)

- `GeneralsMD/Code/Main/CMakeLists.txt`
- `GeneralsMD/Code/Main/WinMain.cpp`
- `GeneralsMD/Code/Main/SDL3Main.cpp`
- `Generals/Code/Main/CMakeLists.txt`
- `Generals/Code/Main/WinMain.cpp`
- `Core/GameEngineDevice/Include/SDL3GameEngine.h`
- `references/fighter19-dxvk-port/GeneralsMD/Code/Main/SDL3Main.cpp`

## Execution Outputs

- WinMain factory selects SDL3 engine when modern flags are enabled.
- Legacy Win32 engine path remains available behind explicit conditions.

## Checklist

- Identify all Windows-only entry guards.
- Extract common startup flow where necessary.
- Keep any legacy WinMain path isolated instead of shared.

## Done Criteria

- Windows64 modern build instantiates `SDL3GameEngine` path under `SAGE_USE_SDL3=ON`.
- Legacy behavior remains intact when modern flags are disabled.