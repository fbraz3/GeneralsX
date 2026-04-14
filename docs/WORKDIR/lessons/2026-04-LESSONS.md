# 2026-04 Lessons Learned

## Session 2026-04-13 - Replay header parsing must not depend on host wchar size

- Problem: Headless replay simulation on macOS crashed or rejected valid replay files in mixed real-world datasets.
- Evidence: Stack protector aborts and malformed replay metadata (invalid/empty GameInfo) occurred while reading replay headers on non-Windows.
- Root cause: Replay parsing paths implicitly depended on platform `wchar_t` size via wide-char file reads. Windows replay layout stores Unicode fields as UTF-16 (2-byte units), while macOS/Linux `wchar_t` is typically 4 bytes.
- Fix: Parse replay Unicode fields as explicit 2-byte UTF-16 units, support absolute replay paths, and harden string parsing against overflow/truncation edge cases.
- Validation: macOS replay suite now processes real-world filenames (including spaces/symbols) without parser crashes; remaining failures are CRC mismatches/determinism, not filename parsing.
- Prevention: For binary compatibility formats shared with Windows retail assets, never bind on-disk structure parsing to native type widths (`wchar_t`, struct packing, etc.); use explicit fixed-width serialization rules.

## Session 2026-04-12 - LAN lobby visibility must be traced at render and prune points, not only at announce receipt

- Problem: Logs could show `handleGameAnnounce` success while the user still reported that the LAN lobby list did not visibly retain the remote game.
- Code finding: The LAN lobby callback does not apply extra compatibility filtering; it forwards `m_games` directly into `LANDisplayGameList`.
- Remaining suspects: a parsed game can still fail to appear usefully if its row content is unexpected, or it can disappear shortly after due to `lastHeard`-based pruning.
- Process improvement: For LAN lobby regressions, instrument both row rendering and timeout-driven removal in addition to message receive/parse logs.

## Session 2026-04-11 - LAN auto-IP and global broadcast can hide hosts cross-platform

- Problem: Linux and macOS builds failed to discover each other in the LAN lobby, and same-platform behavior remained uncertain without multiple test machines per OS.
- Code finding: LAN discovery sends to a fixed global broadcast target (`INADDR_BROADCAST` / `255.255.255.255`) and LAN menu startup can auto-pick the first enumerated non-loopback IP.
- Risk: On macOS/Linux systems with VPN/Docker/virtual NICs, first-IP selection can bind sockets to a non-game interface, and global broadcast may not reach peers as expected for the active subnet.
- Process improvement: For LAN regressions, always log selected bind IP, broadcast destination, bind/send error codes, and incoming source addresses before judging protocol-level compatibility.
- Prevention: Prefer interface-aware LAN discovery (default-route NIC + subnet broadcast calculation) and keep manual IP override visible/easy in options.

## Session 2026-04-12 - LAN join accept must stay unicast to avoid host-only false positives

- Problem: In direct-connect tests, the host sometimes showed the remote player in the game lobby while the joining machine still timed out and never entered the lobby.
- Evidence: Fresh `[LAN86]` logs showed the host receiving the join request and then emitting `MSG_JOIN_ACCEPT` as broadcast (`255.255.255.255`) instead of sending it back directly to the joining client IP.
- Root cause: `LANAPI::handleRequestJoin()` zeroed the response target after adding the player locally, so `sendMessage()` fell through to the broadcast path for the join-accept packet.
- Fix: Keep the requester IP as the response target for join accept/deny packets; only local game-state updates should be host-local.
- Prevention: For request/response handshake packets (`REQUEST_JOIN`, `JOIN_ACCEPT`, `JOIN_DENY`), never repurpose the destination selection based on local slot mutation; log both sender and final response target during debugging.

## Session 2026-04-12 - In-game LAN control packets need directed delivery on mixed OS networks

- Problem: Even after direct-connect join success, clients could miss host-driven updates (game options, game start, and host leave effects), producing partial desync UX.
- Evidence: Join handshake traffic succeeded with unicast, but follow-up control behavior matched packet classes still emitted as broadcast without explicit destination.
- Fix: Prefer directed fan-out to known human slots for in-game control/state packets, keeping broadcast only as fallback when no slot target exists.
- Prevention: In cross-platform LAN paths, treat broadcast as discovery-only by default; use explicit per-slot delivery for game/session control events.

## Session 2026-04-12 - LAN discovery should prefer subnet broadcast over global broadcast

- Problem: LAN lobby discovery remained unreliable across macOS/Linux even when direct-connect worked.
- Evidence: Discovery still depended on global broadcast `255.255.255.255`, which may not be forwarded/handled consistently on mixed-network setups.
- Fix: In POSIX builds, collect broadcast addresses from active IPv4 interfaces matching the selected local LAN IP and send broadcast packets to those subnet addresses first; keep global broadcast as fallback.
- Prevention: For multi-platform LAN discovery, avoid single global broadcast as the only path; use interface-scoped subnet broadcast to reduce network-policy sensitivity.

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
