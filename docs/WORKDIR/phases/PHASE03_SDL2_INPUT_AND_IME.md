# PHASE03: SDL2 Input + IME

## Objective
Replace runtime mouse/keyboard/IME handling with SDL2 equivalents while preserving gameplay behavior.

## Checklist
- [ ] Implement SDL2 mouse event translation (motion, buttons, wheel).
- [ ] Implement SDL2 cursor capture/relative mouse mode behavior.
- [ ] Implement SDL2 keyboard translation (key down/up, modifiers).
- [ ] Replace any runtime `GetAsyncKeyState`-style polling with SDL state where required.
- [ ] Implement SDL2 text input and IME composition integration compatible with `IMEManager`.
- [ ] Verify menu navigation and in-game controls on macOS/Linux.

## Exit Criteria
- Menus are fully navigable and core gameplay input works (camera, selection, commands).
