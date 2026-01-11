# SDL2 Event + Window Notes (for Win32 → SDL2 port)

## Event pump
- SDL2 event processing is typically done once per frame:
  - `SDL_Event event; while (SDL_PollEvent(&event)) { ... }`
- `SDL_PollEvent` may implicitly call `SDL_PumpEvents`, and must be called from the thread that initialized the video subsystem.

## Window state (minimize / focus)
- Use `SDL_GetWindowFlags(window)` to query window state.
- Use `SDL_WINDOWEVENT` + `SDL_WindowEventID` events to react to:
  - focus gained/lost
  - minimized/restored
  - size changes

## Mapping ideas
- Win32 `IsIconic(hwnd)` (minimized) → SDL2 `SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED`.
- Win32 `Sleep(ms)` in minimized loop → SDL2 `SDL_Delay(ms)`.

## References
- SDL_PollEvent: https://wiki.libsdl.org/SDL2/SDL_PollEvent
- SDL_GetWindowFlags: https://wiki.libsdl.org/SDL2/SDL_GetWindowFlags
- SDL_WINDOWEVENT: https://wiki.libsdl.org/SDL2/SDL_WINDOWEVENT
