# PHASE02: SDL2 App Entry + Event Pump

## Objective
Create an SDL2-driven application entry point and replace the Win32 message pump with SDL event polling on non-Windows targets.

## Key Work Items
- Add a non-Windows entry point (SDL2 `main`) and ensure build selects it.
- Create an SDL2 window and map it to the engine’s notion of “application window”.
- Replace the OS message pump (`PeekMessage/GetMessage/...`) with `SDL_PollEvent`.
- Map window lifecycle events to existing engine state:
  - close request → quit message
  - focus gained/lost → engine active state
  - minimized/restored → low-power update behavior (no busy loop)

## Checklist
- [ ] Add SDL2 initialization/shutdown in the non-Windows entry point.
- [ ] Create SDL2 window with correct flags for windowed/fullscreen.
- [ ] Implement SDL2 event pumping once per frame.
- [ ] Map `SDL_QUIT` / close window events to the game quit flow.
- [ ] Map focus and minimize/restore events to engine `setIsActive` behavior.
- [ ] Replace “alt-tabbed out” logic with SDL window minimized checks + SDL delay.
- [ ] Confirm the engine reaches “initial screen” on macOS/Linux.

## Exit Criteria
- Game starts and shows initial UI without relying on Win32 window APIs.
