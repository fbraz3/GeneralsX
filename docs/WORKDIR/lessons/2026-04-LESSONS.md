# 2026-04 Lessons Learned

## Session 2026-04-09 - Flatpak packaging needs explicit runtime source destination and compliant metadata/icon rules

- Problem: Local Flatpak packaging failed first with missing `runtime/.` during module build, then with AppStream metadata validation, and finally with icon export rejection.
- Root cause:
	- The `type: dir` source in manifest did not guarantee the expected `runtime/` folder name for the build command.
	- Metainfo files missed required AppStream `metadata_license`.
	- Source icon assets were 650x650, while Flatpak export enforces max 512x512 icon size.
- Fix:
	- Set `dest: runtime` for staging runtime dir source in both manifests.
	- Added `<metadata_license>CC0-1.0</metadata_license>` to both metainfo files.
	- Generated 512x512 icons for Flatpak packaging and switched manifests to install them under `hicolor/512x512`.
	- Added local script preflight to install Flatpak SDK/runtime (`org.freedesktop.Platform//23.08`, `org.freedesktop.Sdk//23.08`) under `--user` when missing.
- Validation: Local builds completed for both targets and produced:
	- `build/GeneralsX-linux64-deploy.flatpak`
	- `build/GeneralsXZH-linux64-deploy.flatpak`
- Prevention: For every new Flatpak app module, validate early that sources map to expected in-build paths, metainfo passes `appstreamcli compose`, and icon dimensions satisfy export limits.

## Session 2026-04-09 - Flatpak runtime library bundling must preserve SONAME links and avoid self-symlink overwrite

- Problem: `flatpak run com.generals.GeneralsXZH` failed with missing `libSDL3_image.so.0` and later codec libraries such as `libavcodec.so.60` / `libx264.so.164`.
- Root cause:
	- Runtime staging copied only regular files, dropping symlink chains required by SONAME resolution.
	- Manifest fallback symlink logic could overwrite already-correct `lib*.so.<major>` files into self-symlinks.
	- FFmpeg-linked binary required a larger codec dependency set than the initial package list.
- Fix:
	- Staging now copies runtime libs with `cp -a` globs to preserve symlinks.
	- Manifest symlink fallback now creates `lib*.so.<major>` links only when source has a minor suffix.
	- Flatpak build script now includes FFmpeg codec libs and uses `ldd`-based dependency closure copy for FFmpeg roots.
- Validation: Dynamic loader errors for SDL3_image/FFmpeg libs were eliminated; runtime progressed to Vulkan initialization stage.
- Prevention: For Flatpak packaging of dynamically linked binaries, validate `NEEDED`/SONAME closure and inspect `/app/lib` for accidental self-symlink artifacts.

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
- Root cause: Non-Windows save-file enumeration and load flow depended on local filesystem/user state, so missing or invalid save directories could fail during iteration and load could proceed without a valid selection.
- Fix: Hardened non-Windows `GameState::iterateSaveFiles()` to guard directory switch + iteration + restore, and added a load selection guard so the load path bails out safely when no valid save entry is selected.
- Validation: Static diagnostics (`get_errors`) reported no issues in edited files; macOS build task completed with warnings only in unrelated code.
- Prevention: For cross-platform save/load flows, guard filesystem-dependent save enumeration and validate selected save entries before starting load operations when local user state may differ between machines.

## Session 2026-04-02 - Save/load data paths must preserve separator and case semantics

- Problem: Save files were listed and selected correctly, but loading failed with `SC_INVALID_DATA` during snapshot transfer on macOS.
- Root cause: The portable-to-real map path conversion in save-load logic assumed Windows-style path semantics (`\\` separator and forced lowercase output), which can corrupt valid paths on case-sensitive platforms.
- Fix: Updated map path helpers to accept both `\\` and `/` separators and preserved real path casing on non-Windows builds (keep lowercase normalization only on Windows).
- Backport: Applied the same path-semantics fix to both Zero Hour and Generals base game codepaths.
- Validation: Manual macOS Zero Hour flow completed (create save + load save) and process exited cleanly with code 0.
