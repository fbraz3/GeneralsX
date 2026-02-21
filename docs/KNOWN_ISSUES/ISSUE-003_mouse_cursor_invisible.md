# ISSUE-003: Mouse Cursor Not Visible In-Game

**Status**: OPEN  
**Session Discovered**: 53 (2026-02-21)  
**Severity**: Medium  
**Component**: Platform / Graphics  
**Reproducibility**: 100%  

## Symptom

The mouse cursor is invisible. The cursor position works correctly (clicks register on menus after Session 52–53 fixes), but the visual cursor sprite is never drawn.

## Investigation Summary

No deep investigation yet.

### Hypotheses

1. **`SDL3Mouse::initCursorResources()` is a stub** — Our current implementation is empty (deferred to Phase 2). The game relies on `initCursorResources()` to load `.ani` cursor files and set SDL cursors. Without it, no cursor surface is created and SDL falls back to displaying nothing (or the system cursor is hidden by `SDL_HideCursor()` called elsewhere).

2. **`SDL_HideCursor()` called during init without matching `SDL_ShowCursor()`** — A code path may call `SDL_HideCursor()` early and our `SDL_ShowCursor()` in `init()` may not be reached, or may be called before the window is fully created.

3. **SDL3 hardware cursor not set** — `SDL3Mouse::setCursor()` currently creates a `SDL_SYSTEM_CURSOR_DEFAULT` each frame but may fail silently if the window surface or renderer is not ready yet.

4. **Game-side cursor rendering** — The engine may render the cursor via W3D (software sprite drawn over the frame), not via the OS cursor. If W3D cursor rendering is not wired up, the OS cursor is hidden and nothing replaces it.

## Code Audit Results

`SDL3Mouse::initCursorResources()` in `GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/GameClient/SDL3Mouse.cpp` is currently a stub:

```cpp
void SDL3Mouse::initCursorResources(void)
{
    // TODO: Phase 2 - Load SDL3 cursor images from files
}
```

Fighter19's reference has a full implementation that reads `.ani` cursor files via RIFF parsing and sets SDL cursors per `MouseCursor` enum value.

## Next Steps (for Future Session)

1. Port `SDL3Mouse::initCursorResources()` from fighter19 reference (full ANI cursor loader).
2. Port `SDL3Mouse::setCursor()` to map `MouseCursor` enum → `cursorResources[cursor][direction]` → `SDL_SetCursor()`.
3. Alternatively: enable the OS system cursor as a temporary visible fallback (`SDL_ShowCursor()` + `SDL_SYSTEM_CURSOR_DEFAULT` permanently set) to unblock testing.

## Workaround

The system cursor may be visible if `SDL_CaptureMouse` / `SDL_SetWindowMouseGrab` are not in use. No functional workaround currently.

## Impact

- **Gameplay**: Major (usability severely affected — users cannot see where they are clicking)
- **Stability**: None
- **Determinism**: N/A
- **Release blocker**: Yes

## Reference

- Session 53 discovery (2026-02-21)
- Fighter19 ANI cursor loader: `references/fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/GameClient/SDL3Mouse.cpp` (lines 107–230)
