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

## Session 2026-04-02 - SaveLoad crashes can be environment-specific (M1 vs M3, local state)

- Problem: `Menus/SaveLoad.wnd` opened through main menu could exit immediately on one macOS machine while another user (M3) could not reproduce.
- Root cause hypothesis: Post-layout init path (`SaveLoadMenuFullScreenInit` -> save-file enumeration) was under-instrumented, and non-Windows save directory traversal could throw before/around iteration in environment-specific setups (missing/invalid save directory, stale local runtime state).
- Fix: Added Apple-only diagnostics in main-menu click path, fullscreen SaveLoad init, and save-file iteration; hardened non-Windows `GameState::iterateSaveFiles()` to wrap directory switch + iteration + restore in guarded exception handling.
- Validation: Static diagnostics (`get_errors`) reported no issues in edited files; macOS build task completed with warnings only in unrelated code.
- Prevention: For macOS-only menu/open flows, instrument both UI transition boundaries and filesystem entry points, and never call `std::filesystem::current_path(target)` outside a guarded path when target can depend on local user state.

## Session 2026-04-02 - Save/load data paths must preserve separator and case semantics

- Problem: Save files were listed and selected correctly, but loading failed with `SC_INVALID_DATA` during snapshot transfer on macOS.
- Root cause: The portable-to-real map path conversion in save-load logic assumed Windows-style path semantics (`\\` separator and forced lowercase output), which can corrupt valid paths on case-sensitive platforms.
- Fix: Updated map path helpers to accept both `\\` and `/` separators and preserved real path casing on non-Windows builds (keep lowercase normalization only on Windows).
- Backport: Applied the same path-semantics fix to both Zero Hour and Generals base game codepaths.
- Validation: Manual macOS Zero Hour flow completed (create save + load save) and process exited cleanly with code 0.
