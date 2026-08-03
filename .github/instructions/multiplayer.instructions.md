# Multiplayer & GameSpy Integration (GeneralsX)

This document contains rules and patterns for the multiplayer subsystem (WOL/GameSpy) in GeneralsX.

## 1. UI Flow & Tick Updates
- **Menu Ticks**: Menus like `WOLLoginMenu` and `WOLWelcomeMenu` process logic in their tick functions (`WOLLoginMenuUpdate`, `WOLWelcomeMenuUpdate`).
- **Animation Sync**: Always check `TheShell->isAnimFinished()` before triggering network requests in menus to prevent race conditions during UI transitions.
- **Defensive UI Enabling**: If returning from a lobby or QuickMatch (e.g., `WOLWelcomeMenu`), verify if the UI controls are enabled (`BitIsSet(button->winGetStatus(), WIN_STATUS_ENABLED)`). Re-enable them via `winEnable(TRUE)` once the group room list is fetched (`TheGameSpyInfo->gotGroupRoomList()`).

## 2. String Handling (Ascii vs Unicode)
- **Database Literals**: Names coming from the GameSpy backend (like room names or player nicks) are **literal strings**, not `GameText` lookup keys. Do not pass them through `TheGameText->fetch()` unless they match a specific GUI token.
- **Unicode Translation**: Never convert `AsciiString` to `UnicodeString` using assignment or constructor like `UnicodeString(s.str())` as it will fail to compile or result in "Missing GUI:" text errors. 
  - **CORRECT**: `UnicodeString dest; dest.translate(asciiSource);`

## 3. Server Configuration & Macros
- Always use the `GSI_DOMAIN_NAME` macro (defined in `cmake/gamespy.cmake`) instead of hardcoding `gamespy.com` or `peerchat.gamespy.com`. This ensures the game can connect to alternative master servers (e.g., C&C:Online or OpenSpy).

## 4. Passwords and Storage
- Passwords and sensitive data stored in local `Options.ini` (via `PersistentStorageThread` or `WOLLoginMenu`) must be **obfuscated** before saving to prevent plaintext leaks in configuration files.

## 5. Subsystem Synchronization
- Both `Generals` and `GeneralsMD` share the same core GameSpy logic (`Core/GameEngine/Source/GameNetwork/`). UI logic is somewhat duplicated between the two games (`GameClient/GUI/GUICallbacks/Menus/`). 
- When fixing network or lobby UI bugs in Zero Hour (`GeneralsMD`), you **MUST** port those same fixes to the Base Game (`Generals`) to keep parity.

## 6. Cross-Platform Nuances
- **Thread Cancellation (macOS vs Linux)**: The network threads (`PeerThread`, etc.) use `pthread_cancel`. On Linux (GNU `libstdc++`), this relies on `abi::__forced_unwind`. macOS (`libc++`) does NOT have this type. Guard catch blocks with `#if defined(_UNIX) && !defined(__APPLE__)` to prevent compile errors.
- **Socket Polling (EWOULDBLOCK)**: The original GameSpy SDK was built for Windows and often checked `rcode <= 0` to assume a disconnect. On UNIX, non-blocking sockets can return `-1` with `EWOULDBLOCK` (or `EAGAIN`). You must check `gsiSocketIsError()` and `GOAGetLastError()` to avoid silent chat disconnects.
- **CRC Verification**: Cross-platform play requires CRCs to match. Because macOS uses Mach-O and Linux uses ELF binaries, reading the executable for CRC generation will always result in a mismatch. We bypass executable reading on UNIX (`#elif defined(_UNIX)`) and rely solely on version numbering and script checksums to ensure cross-play determinism.
- **Main Thread Message Queuing**: GameSpy SDK network callbacks (e.g., `roomMessageCallback`) execute on the network thread. They MUST explicitly add their payload to the main thread's queue (e.g., `TheGameSpyPeerMessageQueue->addResponse(resp)`) otherwise the GUI will silently ignore the network event (breaking chat, auto-refresh, etc).
