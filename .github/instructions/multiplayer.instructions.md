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
