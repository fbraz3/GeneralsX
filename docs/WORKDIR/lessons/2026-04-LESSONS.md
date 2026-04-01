# 2026-04 Lessons Learned

## Session 2026-04-01 - User-facing path migrations need runtime fallback, not just docs updates

- Problem: Zero Hour user-facing scripts and docs exposed the internal `GeneralsMD` path, which leaks implementation details and conflicts with product naming (`GeneralsZH`).
- Root cause: Runtime/deploy/bundle scripts hardcoded `~/GeneralsX/GeneralsMD` as the only default path.
- Fix: Switched user-facing defaults to `~/GeneralsX/GeneralsZH` and added automatic fallback to `~/GeneralsX/GeneralsMD` when legacy installs are detected.
- Validation: Shell syntax checks (`bash -n`) and diagnostics validation (`get_errors`) passed for all modified scripts/docs.
- Prevention: For user-visible path renames, implement resolution order (`new path` then `legacy path`) directly in launch/deploy logic instead of relying on migration notes alone.

## Session 2026-04-01 - SDL3 key events are not enough for GUI text entry

- Problem: Text-entry gadgets on SDL3 builds (Linux/macOS) could gain focus but did not insert typed characters.
- Root cause: The SDL3 backend forwarded only key up/down events (`GWM_CHAR`) while the text-entry pipeline inserts printable characters through `GWM_IME_CHAR`.
- Fix: Added an SDL text-input bridge in `SDL3GameEngine` (both Zero Hour and Generals) that enables `SDL_StartTextInput` only when a `GWS_ENTRY_FIELD` has focus and forwards `SDL_EVENT_TEXT_INPUT` UTF-8 codepoints as `GWM_IME_CHAR`.
- Validation: macOS `GeneralsXZH` build completed successfully and the mirrored `g_generals` target built with `EXIT:0`.
- Prevention: On SDL platforms, treat physical key events and text composition as separate channels; always wire `SDL_EVENT_TEXT_INPUT` for editable widgets.

## Session 2026-04-01 - SDL text bridge still needs editing key scan-code coverage

- Problem: After wiring `SDL_EVENT_TEXT_INPUT`, printable characters worked but `Backspace` and navigation/editing keys did not trigger expected GUI behavior.
- Root cause: `SDL3Keyboard::translateScanCodeToKeyVal` was missing mappings for editing/navigation keys (notably `SDL_SCANCODE_BACKSPACE`, plus `Delete/Home/End/PageUp/PageDown/KP Enter`).
- Fix: Added the missing SDL scancode mappings to both Zero Hour and Generals SDL3 keyboard backends.
- Validation: Incremental macOS build (`z_generals`) finished with `EXIT:0`; static diagnostics reported no errors in edited files.
- Prevention: When introducing IME/text-input paths, verify scancode coverage for non-printable editing/navigation keys to preserve legacy `GWM_CHAR` behavior.

## Session 2026-04-01 - Double Backspace from mixed repeat sources

- Problem: Pressing Backspace once deleted two characters in SDL3 builds.
- Root cause: Keyboard repeat happened in two places at once: SDL repeated `KEY_DOWN` events and the engine also generated autorepeat in `Keyboard::checkKeyRepeat()`.
- Fix: In SDL3 keyboard backends (Zero Hour and Generals), ignore SDL repeated keydown events (`event->key.repeat`) and set `keyDownTimeMsec` using `timeGetTime()` so repeat timing uses the same clock domain as engine logic.
- Validation: Incremental macOS `z_generals` build completed with `EXIT:0`.
- Prevention: Keep a single repeat authority (engine side) and avoid mixing timestamp domains when feeding input timing into legacy repeat logic.
