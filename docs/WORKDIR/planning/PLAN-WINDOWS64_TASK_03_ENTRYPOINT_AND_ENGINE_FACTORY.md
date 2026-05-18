# Windows64 Task 03 - Entry Point and Engine Factory

## Scope

Make Windows64 use the SDL3-based startup and engine path.

## Files

- `GeneralsMD/Code/Main/WinMain.cpp`
- `GeneralsMD/Code/Main/SDL3Main.cpp`
- `Generals/Code/Main/WinMain.cpp`
- `Core/GameEngineDevice/Include/SDL3GameEngine.h`
- Related engine factory code in main/device layers

## Deliverable

- Windows64 modern builds choose the SDL3-based engine path without breaking Linux or macOS.

## Checklist

- Identify all Windows-only entry guards.
- Extract common startup flow where necessary.
- Keep any legacy WinMain path isolated instead of shared.