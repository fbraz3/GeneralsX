# 2026-04 Lessons Learned

## Session 2026-04-20 - Overlord portable riders must not go through fire-point redeploy on turret rotation

- Problem: Overlord portable upgrades (Gatling Cannon, Propaganda Tower, BattleBunker) could disappear intermittently during force-attack / heavy turret turning on macOS.
- Root cause:
	- `OverlordContain::containReactToTransformChange()` executes on turret orientation changes.
	- Base `OpenContain::redeployOccupants()` moved portable riders through fire points (`putObjAtNextFirePoint`).
	- Portable `setTransformMatrix` triggered rider `reactToTransformChange`, which could route into `GameLogic::destroyObject`.
	- BattleBunker loss cascaded to its contained infantry, making the issue appear broader than only one upgrade type.
- Fix:
	- Added `OverlordContain::redeployOccupants()` override to skip fire-point redeploy for `KINDOF_PORTABLE_STRUCTURE` riders.
	- Kept portable transform sync in `syncPortablePosition()` after transform updates.
	- Preserved base redeploy behavior for non-portable occupants.
- Validation insight: Removal traces observed during score/reset were expected cleanup (`GameLogic::reset`/`clearGameData`) and should not be treated as gameplay regression.
- Prevention: For Overlord portable upgrades, never route rider positioning through generic occupant fire-point redeploy; keep portable movement tied to host transform synchronization only.

## Session 2026-04-19 - Wide printf `%s`/`%S` mismatch causes one-character UI and replay metadata truncation on POSIX

- Problem: Multiple UI texts on macOS/Linux showed only the first character (construction requirements, under-construction/completed labels, replay list fields such as date/time/version).
- Root cause:
	- Legacy code used Windows/MSVC wide `printf` semantics (`%s` for wide string and `%S` for narrow string).
	- On POSIX libc (`vswprintf`), `%s` expects narrow strings unless `l` modifier is present (`%ls`), so many `UnicodeString::format` callsites truncated output.
	- Replay headers also wrote wide strings through `LocalFile::writeFormat(L"%s", ...)`, which could persist truncated metadata in newly recorded `.rep` files.
- Fix:
	- Added POSIX format normalization in `UnicodeString::format_va` to map MSVC-style wide specifiers safely (`%s` -> `%ls`, `%S` -> `%hs` when no explicit length modifier exists).
	- Updated `LocalFile::writeFormat(const WideChar*)` to route formatting via `UnicodeString::format_va`, ensuring identical normalization for replay/header serialization paths.
- Validation: Static diagnostics (`get_errors`) reported no issues in modified files after patching.
- Prevention: For cross-platform wide formatting, avoid relying on CRT-specific `%s`/`%S` behavior; normalize format strings or use explicit `%ls` / `%hs` in new code.

## Session 2026-04-16 - A single issue can hide multiple EXC_BAD_ACCESS root causes

- Problem: A macOS crash issue initially looked like one intermittent defect but later reports in the same thread showed a second stack signature.
- Root causes:
	- AI path: `AITNGuardOuterState::update()` could dereference `m_attackState` when it had not been initialized on certain state-entry paths.
	- Audio path: `OpenALAudioFileCache::freeEnoughSpaceForSample()` could dereference `m_eventInfo` during priority eviction for filename-only cache loads.
- Fix:
	- Added lazy attack-state reconstruction plus safer team/prototype checks in tunnel-network guard update logic.
	- Added null-safe priority selection for OpenAL cache eviction (`AudioEventInfo` optional entries now handled explicitly).
- Validation: Static diagnostics reported no errors in modified files; issue thread updated with code anchors and retest request.
- Prevention: Split triage by crash signature (top frame + fault address + subsystem) before assuming a shared cause across reports in the same issue.

## Session 2026-04-13 - Flatpak must build inside SDK, not package host binaries/libraries

- Problem: Flatpak packaging mixed host-built binaries with manually copied shared libraries, causing fragile runtime behavior and Wayland/Vulkan instability risks.
- Root cause: Packaging flow treated Flatpak as a generic container bundle instead of a runtime+SDK build pipeline.
- Fix:
	- Migrated manifests to compile the game inside `org.freedesktop.Sdk` (25.08) with `flatpak-builder`.
	- Declared dependency sources in manifest (`SDL3`, `SDL3_image`, `openal-soft`, `dxvk-native`) and wired them through `FETCHCONTENT_SOURCE_DIR_*` to avoid in-build network fetches.
	- Removed host library staging logic from the build script and switched to manifest-driven build/export only.
	- Kept runtime environment/path logic centralized in `/app/bin/run.sh`, with wrappers acting as pass-through entrypoints.
- Validation:
	- `flatpak-builder --show-manifest` succeeds for both `com.fbraz3.GeneralsX.yml` and `com.fbraz3.GeneralsXZH.yml`.
	- `flatpak-builder --user --build-only` now advances to SDK update/install phase (instead of failing on missing system remote refs).
- Prevention: For Flatpak work, never copy `.so` files from host system paths into app runtime. Build inside SDK and ship only artifacts produced by the manifest modules.

## Session 2026-04-09 - libxcb Flatpak PoC needs newer source libs, not host baseline copy

## Session 2026-04-11 - 8-player macOS crash points to AI guard-state null dereference path

- Problem: A user reported an intermittent crash during 8-player skirmish on macOS (Apple Silicon), while the same broad scenario could not be reproduced on another Apple Silicon machine.
- Evidence: The crash report pinned thread 0 to `AITNGuardOuterState::update()` with `EXC_BAD_ACCESS` at `0x0000000000000038`, indicating a likely null-pointer dereference with field offset access.
- Code finding: `AITNGuardOuterState::update()` dereferences `m_attackState` without a null guard, but `AITNGuardOuterState::onEnter()` has early-success paths that can leave `m_attackState` unset.
- Scope insight: The crash location is in gameplay AI logic (tunnel-network guard state), not in rendering/audio backends, so hardware generation differences (M1 vs newer Apple Silicon) likely change trigger timing rather than root cause.
- Prevention: For legacy state-machine code, treat early-success state transitions and lazily initialized sub-states as high-risk paths and add explicit invariant checks before dereferencing nested state objects.

## Session 2026-04-11 - Dynamic shadow draws need explicit stream rebinding

- Problem: Animated object shadows (flags, rotor parts) in Zero Hour rendered incorrectly, while static shadows looked fine and the issue disappeared when `3D shadows` was disabled.
- Root cause: `W3DVolumetricShadow::RenderDynamicMeshVolume` in Zero Hour missed the `SetStreamSource` rebind, so dynamic volume draws could use stale vertex stream state.
- Fix: Restored vertex stream rebinding before dynamic shadow draw calls when the active buffer differs from `lastActiveVertexBuffer`.
- Validation: Static diagnostics reported no errors in the edited file; local non-docker build path was not usable due existing cache path mismatch, so full runtime verification is pending smoke test.
- Prevention: Keep dynamic and static shadow render paths behaviorally aligned between `Generals` and `GeneralsMD`, and audit render-state rebinding when cleaning refactor artifacts.

## Session 2026-04-09 - AppImage can bypass current Flatpak Vulkan/XCB coupling blockers

- Problem: Flatpak remained blocked by Vulkan ICD/XCB symbol incompatibilities despite multiple runtime workarounds.
- Action: Implemented an AppImage packaging PoC for `GeneralsXZH` bundling userspace runtime libs (DXVK, SDL3, SDL3_image, OpenAL, GameSpy) with a dedicated launcher.
- Result: AppImage launched successfully and progressed beyond Vulkan window creation and early engine initialization, where Flatpak path previously failed.
- Insight: For short-term Linux distribution, AppImage is currently lower-risk and faster to stabilize than Flatpak in this codebase state.
- Prevention: Keep Flatpak as a parallel track for longer-term sandbox goals, but prioritize AppImage for immediate user-facing releases.

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

- Problem: `flatpak run com.fbraz3.GeneralsXZH` failed with missing `libSDL3_image.so.0` and later codec libraries such as `libavcodec.so.60` / `libx264.so.164`.
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
