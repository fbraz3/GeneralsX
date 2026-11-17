# Phase 05: LAN Cross-Play (Browser ↔ Desktop) & GameSpy Isolation

## Objective
1. Enable **true LAN cross-play between the WebAssembly browser client and native desktop builds (`macOS`, `Linux`, `Windows`)** by ensuring the WASM client advertises its real LAN bridge IP (instead of a hardcoded `10.0.0.x` virtual IP) and integrating `__EMSCRIPTEN__` into `StdLANInterface`.
2. Cleanly gate legacy GameSpy (`SAGE_USE_GAMESPY=OFF`) while **preserving 100% of NGMP (`SAGE_USE_NGMP`)** for future online integration.

---

## 1. Why `wasm-generals` Networking Needed Upgrading for Cross-Play

In `wasm-generals`:
- `udp.cpp` assigned each browser tab an arbitrary `10.0.0.x` virtual IP, and `serve-webgpu.py` only routed WebSocket frames between browser tabs in memory (it never opened real POSIX UDP sockets on ports `8086` or `8088`).
- Meanwhile, `GeneralsX`'s `LANAPIhandlers.cpp` checks exact IP matching on join:
  - Line 333 (`handleRequestJoin`): `if (msg->GameToJoin.gameIP != m_localIP && !LANInterfaceDevice::isLocalHostAddress(msg->GameToJoin.gameIP)) return;`
  - Line 533 (`handleJoinAccept`): `if (msg->GameJoined.playerIP == m_localIP || LANInterfaceDevice::isLocalHostAddress(msg->GameJoined.playerIP))`
- Therefore, for a desktop client and browser client to join each other's lobby and play in lockstep, the browser's `GetWebLanVirtualIP()` must match the host interface (or alias IP) used by the `serve-webgpu.py` UDP bridge.

---

## 2. Browser LAN IP & UDP-over-WebSocket (`udp.h`, `udp.cpp`, `StdLANInterface.cpp`)

1. **`Core/GameEngine/Include/GameNetwork/udp.h`** (around line 120):
   - Add:
     ```cpp
     #if defined(__EMSCRIPTEN__)
     UnsignedInt GetWebLanVirtualIP();
     #endif
     ```

2. **`Core/GameEngine/Source/GameNetwork/udp.cpp`**:
   - Port the `#if defined(__EMSCRIPTEN__)` `EM_JS` bridge (`GeneralsXWebLanVirtualIP`, `GeneralsXWebLanBind`, `GeneralsXWebLanClose`, `GeneralsXWebLanWrite`, `GeneralsXWebLanRead`, `GetWebLanVirtualIP`) and the `#if defined(__EMSCRIPTEN__)` paths inside `UDP::~UDP`, `UDP::Bind`, `UDP::SetBlocking`, `UDP::Write`, `UDP::Read`, `UDP::SetInputBuffer`, `UDP::SetOutputBuffer`, `UDP::GetInputBuffer`, `UDP::GetOutputBuffer`, and `UDP::AllowBroadcasts` from `wasm-generals/Core/GameEngine/Source/GameNetwork/udp.cpp`.
   - **Critical Cross-Play Upgrade in `GeneralsXWebLanVirtualIP`**:
     - First check `globalThis.generalsXLanHostIP` (populated before module init by `web/src/main.ts` from `GET /GeneralsXLanConfig`) before falling back to `?lanClient=` / `sessionStorage` `10.0.0.x`.
     - In `GeneralsXWebLanBind` (`onRelayMessage`), also handle control frame opcode `3` (`bytes.length === 5 && bytes[0] === 3`) to update `globalThis.generalsXLanVirtualIP = view.getUint32(1, false) >>> 0;` if pushed by the relay.
   - **Important `GeneralsXWebLanBind` URL resolution**:
     - Use `(globalThis.location.protocol === "https:" ? "wss://" : "ws://") + globalThis.location.host + "/GeneralsXLan"` so it works on any configured port (`8765`, custom `--port`, or reverse proxy) rather than hardcoding `:8765` as primary.

3. **`Core/GameEngineDevice/Source/StdDevice/Common/StdLANInterface.cpp`** *(Note: `GeneralsX` HEAD moved interface enumeration here from `IPEnumeration.cpp`)*:
   - Include `"GameNetwork/udp.h"` under `#if defined(__EMSCRIPTEN__)`.
   - In `StdLANInterface::getLocalHostAddresses(UnsignedInt *outAddrs, Int maxAddrs)`:
     ```cpp
     #if defined(__EMSCRIPTEN__)
         const UnsignedInt webIP = GetWebLanVirtualIP();
         if (webIP != 0 && maxAddrs > 0)
         {
             outAddrs[0] = webIP;
             return 1;
         }
         return 0;
     #elif !defined(_WIN32)
         // ... existing getifaddrs implementation ...
     ```
   - In `StdLANInterface::isRealLocalInterfaceAddress(UnsignedInt ip)`:
     ```cpp
     #if defined(__EMSCRIPTEN__)
         return (ip != 0 && ip == GetWebLanVirtualIP()) ? TRUE : FALSE;
     #elif !defined(_WIN32)
         // ... existing implementation ...
     ```
   - In `StdLANInterface::isLocalHostAddress(UnsignedInt ip)`:
     ```cpp
     #if defined(__EMSCRIPTEN__)
         return (ip != 0 && ip == GetWebLanVirtualIP()) ? TRUE : FALSE;
     #else
         // ... existing implementation ...
     ```
   - In `StdLANInterface::getSubnetBroadcastAddresses(UnsignedInt localIP, UnsignedInt *outAddrs, Int maxAddrs)`:
     - Under `#if defined(__EMSCRIPTEN__)`, return `0` (`LANAPI` already falls back to `m_broadcastAddr = INADDR_BROADCAST = 0xFFFFFFFF`, which `serve-webgpu.py` fans out to both WebSocket peers and physical subnet broadcast addresses).

4. **`Core/GameEngine/Source/GameNetwork/LANAPIhandlers.cpp`** (line 149, `LANAPI::handleGameAnnounce`):
   - Change:
     ```cpp
     if (senderIP == m_localIP || LANInterfaceDevice::isLocalHostAddress(senderIP))
     ```
     to:
     ```cpp
     if (senderIP == m_localIP || (AmIHost() && LANInterfaceDevice::isLocalHostAddress(senderIP)))
     ```
     *Why*: When running a native desktop host and a browser client on the **same machine** (e.g. via `serve-webgpu.py` bound to a secondary local IP or loopback alias), the client must not discard `MSG_GAME_ANNOUNCE` packets coming from another local interface on the same host when `!AmIHost()`.

5. **`Core/GameEngine/Source/GameNetwork/LANAPI.cpp`** (around line 496):
   - Optional diagnostic log under `#if defined(__EMSCRIPTEN__)` for `GENERALSX_LAN_PEER_DISCOVERED` (non-blocking).

---

## 3. Shared `LobbyTooltips` & `SAGE_USE_GAMESPY=OFF` Guards

1. **Copy new shared tooltip files from `wasm-generals`**:
   ```bash
   SRC=/Users/felipebraz/.gemini/antigravity/worktrees/wasm-generals/wasm_port_architecture_analysis
   DST=/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis

   cp "$SRC/Core/GameEngine/Include/GameNetwork/LobbyTooltips.h" "$DST/Core/GameEngine/Include/GameNetwork/LobbyTooltips.h"
   cp "$SRC/Core/GameEngine/Source/GameNetwork/LobbyTooltips.cpp" "$DST/Core/GameEngine/Source/GameNetwork/LobbyTooltips.cpp"
   ```
   - Update `Core/GameEngine/Include/GameNetwork/GameSpy/LobbyUtils.h`: include `"GameNetwork/LobbyTooltips.h"` and remove duplicate declarations of `playerTemplateComboBoxTooltip` and `playerTemplateListBoxTooltip`.
   - Update `Core/GameEngine/Source/GameNetwork/GameSpy/LobbyUtils.cpp` (lines 902–957 in `GeneralsX` HEAD): remove `playerTemplateComboBoxTooltip` and `playerTemplateListBoxTooltip` definitions (now in `LobbyTooltips.cpp`).
   - Update `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/WOLGameSetupMenu.cpp` (lines 510–526 in `GeneralsX` HEAD): remove `gameAcceptTooltip` definition (now in `LobbyTooltips.cpp`).
   - Update `LanGameOptionsMenu.cpp` and `SkirmishGameOptionsMenu.cpp` in `GeneralsMD/` to include `"GameNetwork/LobbyTooltips.h"` instead of `"GameNetwork/GameSpy/LobbyUtils.h"` (and remove `extern void gameAcceptTooltip(...)` from `LanGameOptionsMenu.cpp`).

2. **CMake Source Filtering when Both GameSpy and NGMP are Disabled**:
   - In **`Core/GameEngine/CMakeLists.txt`**:
     - Add `Include/GameNetwork/LobbyTooltips.h` and `Source/GameNetwork/LobbyTooltips.cpp` to `GAMEENGINE_SRC`.
     - Add:
       ```cmake
       if(NOT SAGE_USE_GAMESPY AND NOT SAGE_USE_NGMP)
           list(FILTER GAMEENGINE_SRC EXCLUDE REGEX "GameNetwork/GameSpy")
           list(REMOVE_ITEM GAMEENGINE_SRC
               Source/GameNetwork/FirewallHelper.cpp
               Source/GameNetwork/NAT.cpp
           )
           list(APPEND GAMEENGINE_SRC Source/GameNetwork/GameSpy/Thread/ThreadUtils.cpp)
       endif()
       ```
     - Also keep `if(SAGE_USE_SDL3 AND RTS_BUILD_OPTION_DEEP_CRC)` linking `sdl3lib` to `corei_gameengine_private` if needed on `wasm32`.
   - In **`GeneralsMD/Code/GameEngine/CMakeLists.txt`**:
     - Guard the WOL/GameSpy UI callback sources (lines ~1060–1112) when `if(NOT SAGE_USE_GAMESPY AND NOT SAGE_USE_NGMP)`:
       ```cmake
       if(NOT SAGE_USE_GAMESPY AND NOT SAGE_USE_NGMP)
           list(FILTER GAMEENGINE_SRC EXCLUDE REGEX "/Menus/WOL")
           list(FILTER GAMEENGINE_SRC EXCLUDE REGEX "Source/GameNetwork/GameSpy")
           list(FILTER GAMEENGINE_SRC EXCLUDE REGEX "/Menus/NetworkDirectConnect.cpp$")
           list(FILTER GAMEENGINE_SRC EXCLUDE REGEX "/Menus/PopupHostGame.cpp$")
           list(FILTER GAMEENGINE_SRC EXCLUDE REGEX "/Menus/PopupJoinGame.cpp$")
           list(FILTER GAMEENGINE_SRC EXCLUDE REGEX "/Menus/PopupLadderSelect.cpp$")
       endif()
       ```

3. **Apply `#if defined(SAGE_USE_GAMESPY)` Guards (Preserving ALL `SAGE_USE_NGMP` Blocks)**:
   Carefully port the GameSpy compile-time guards without touching any `#if defined(SAGE_USE_NGMP)` paths in:
   - **`Core/GameEngine/Source/Common/INI/INIMultiplayer.cpp`**: fallback `INI::parseOnlineChatColorDefinition` when `!defined(SAGE_USE_GAMESPY) && !defined(SAGE_USE_NGMP)`.
   - **`Core/GameEngine/Source/GameClient/GUI/LoadScreen.cpp`**: wrap `GameSpyLoadScreen` class implementation in `#if defined(SAGE_USE_GAMESPY) || defined(SAGE_USE_NGMP)`.
   - **`Core/GameEngine/Source/GameClient/MessageStream/CommandXlat.cpp`**: wrap `GameSpyToggleOverlay(GSOVERLAY_BUDDY)` in `#if defined(SAGE_USE_GAMESPY)`.
   - **`Core/GameEngine/Source/GameNetwork/DisconnectManager.cpp`**: wrap `pingThread` logic in `#if defined(SAGE_USE_GAMESPY)`.
   - **`Core/GameEngine/Source/GameNetwork/GameInfo.cpp`**: wrap `TheGameSpyGame` reference in `#if defined(SAGE_USE_GAMESPY)`.
   - **`Core/GameEngine/Source/GameClient/GUI/GUICallbacks/Diplomacy.cpp`** *(moved to `Core/` in HEAD)*: wrap GameSpy Buddy headers/functions (`BuddyDefs.h`, `PeerDefs.h`, `BuddyControlSystem`, `InitBuddyControls`, `PopulateOldBuddyMessages`, `updateBuddyInfo`) in `#if defined(SAGE_USE_GAMESPY)`.
   - **`Core/GameEngine/Source/GameClient/GUI/GUICallbacks/InGameChat.cpp`** *(moved to `Core/` in HEAD)*: wrap `getQR2HostingStatus` / `isThreadHosting` / `"host"` slash command in `#if defined(SAGE_USE_GAMESPY)`.
   - **`Core/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp`** *(moved to `Core/` in HEAD)*: wrap `if(TheGameSpyInfo) GameSpyCloseAllOverlays();` calls in `#if defined(SAGE_USE_GAMESPY)` (and ensure `if(!TheGlobalData->m_initialFile.isEmpty() || !TheGlobalData->m_simulateReplays.empty())` stays clean).
   - **`GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`**: wrap `#include "GameNetwork/GameSpy/GameResultsThread.h"`, `TheGameResultsQueue->endThreads()`, and `initSubsystem(TheGameResultsQueue, ...)` in `#if defined(SAGE_USE_GAMESPY)` — keep lines 92–94, 285–287, 718–723, and 1050–1055 (`#ifdef SAGE_USE_NGMP`) completely untouched!
   - **`GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp`**: wrap `TheGameSpyGame` / `TheGameSpyInfo` / `PeerDefs.h` calls in `#if defined(SAGE_USE_GAMESPY)` — keep all `#if defined(SAGE_USE_NGMP)` blocks intact!
   - **`GeneralsMD/Code/GameEngine/Source/Common/System/FunctionLexicon.cpp`**: wrap `WOL*`, `NetworkDirectConnect*`, `PopupHostGame*`, `PopupJoinGame*`, `PopupLadderSelect*`, and `PopupBuddyNotificationSystem` entries in `#if defined(SAGE_USE_GAMESPY) || defined(SAGE_USE_NGMP)`.
   - **`GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/MainMenu.cpp`**:
     - Hide `exitButton` under `#if defined(__EMSCRIPTEN__)` in `MainMenuInit`.
     - Guard `ghttp.h`, `GameSpyOverlay.h`, `PeerDefs.h`, `PeerThread.h`, `BuddyThread.h`, `MainMenuUtils.h`, `RaiseGSMessageBox`, `HTTPThinkWrapper`, `GameSpyUpdateOverlays`, `ghttpStartup`, `ghttpCleanup`, `StopAsyncDNSCheck`, `CancelPatchCheckCallback` with `#if defined(SAGE_USE_GAMESPY)` (keeping all `SAGE_USE_NGMP` code intact!).
     - On `onlineID` button: when `#if !defined(SAGE_USE_GAMESPY) && !defined(SAGE_USE_NGMP)`, disable `buttonOnline->winEnable(FALSE)`.
     - Define `Bool DontShowMainMenu = FALSE;` when `!defined(SAGE_USE_GAMESPY) && !defined(SAGE_USE_NGMP)` (since `WOLLobbyMenu.cpp` is excluded).
   - **`GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/OptionsMenu.cpp`**, **`PopupPlayerInfo.cpp`**, **`ScoreScreen.cpp`**, **`WOLLobbyMenu.cpp`**, **`GameLogic.cpp`**: apply `#if defined(SAGE_USE_GAMESPY)` / `#if defined(SAGE_USE_GAMESPY) || defined(SAGE_USE_NGMP)` guards matching `wasm-generals` while leaving every `#if defined(SAGE_USE_NGMP)` block intact.

---

## Checklist

- [x] Update `udp.h` and `udp.cpp` (`GeneralsXWebLanVirtualIP` reading `globalThis.generalsXLanHostIP` + opcode `3`, `wss://` / `ws://` from `location.host`).
- [x] Implement `#if defined(__EMSCRIPTEN__)` support in `StdLANInterface.cpp`.
- [x] Guard `handleGameAnnounce` self-ignore in `LANAPIhandlers.cpp` with `AmIHost()`.
- [x] Copy `LobbyTooltips.h/.cpp` and remove duplicate functions from `LobbyUtils.cpp` and `WOLGameSetupMenu.cpp`.
- [x] Add `if(NOT SAGE_USE_GAMESPY AND NOT SAGE_USE_NGMP)` source filters in `Core/GameEngine/CMakeLists.txt` and `GeneralsMD/Code/GameEngine/CMakeLists.txt`.
- [x] Apply `#if defined(SAGE_USE_GAMESPY)` guards across `Core/` and `GeneralsMD/` without altering any `SAGE_USE_NGMP` blocks.
