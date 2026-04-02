# 2026-04 Lessons Learned

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

## Session 2026-04-01 - DeepWiki MCP fallback path for large architectural research

- Problem: `mcp_cognitionai_d_ask_question` returned a runtime tool error (`Cannot read properties of undefined (reading 'invoke')`) during architecture research.
- Root cause: The question endpoint can intermittently fail in this environment, while repository wiki endpoints remain available.
- Fix: Use `mcp_cognitionai_d_read_wiki_structure` to enumerate sections, then `mcp_cognitionai_d_read_wiki_contents` to fetch the full corpus and extract only relevant build/architecture sections with targeted file reads.
- Validation: Successfully retrieved both `TheSuperHackers/GeneralsGameCode` and `electronicarts/CnC_Generals_Zero_Hour` documentation payloads and used them in Phase 6 planning.
- Prevention: For critical planning tasks, prefer a robust fallback chain: `read_wiki_structure` -> `read_wiki_contents` -> local extraction, instead of blocking on a failed question call.

## Session 2026-04-01 - Windows DXVK uses runtime replacement, not compile-time DXVK headers

- Problem: Enabling `SAGE_USE_DX8=OFF` on Windows x64 incorrectly dropped into the Linux DXVK CMake branch, expecting Linux headers/libs from `dxvk-native` tarball and risking build/runtime mismatch.
- Root cause: `cmake/dx8.cmake` only distinguished `(SAGE_USE_DX8)` vs `(APPLE + MoltenVK)` vs fallback `(Linux native DXVK)`, with no explicit modern Windows branch.
- Fix: Added a dedicated `WIN32` branch for modern presets: compile/link against Windows SDK DirectX interfaces (`d3d8`, `dxguid`) while shipping DXVK `d3d8.dll` as runtime override next to the game executable.
- Validation: CMake paths are now deterministic for `windows64-*` presets and CI bundles `d3d8.dll` with build artifacts.
- Prevention: For DXVK on Windows, separate concerns explicitly: compile-time ABI from SDK, runtime backend from local DXVK DLL; avoid reusing Linux/macOS DXVK source assumptions.
