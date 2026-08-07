---
applyTo: '**/GeneralsOnline/**,**/NextGenMP/**'
---

# NGMP Subsystem Implementation Instructions

These instructions govern the Next-Gen Multiplayer (NGMP) client protocol integration into **GeneralsX**. All changes in `GeneralsOnline` and `NextGenMP` components must adhere strictly to these guidelines.

---

## Golden Constraints

1. **Strict Prohibition of `<windows.h>` & Win32 APIs**:
   - Never include `<windows.h>`, `<winsock2.h>`, `<ws2tcpip.h>`, `<wincred.h>`, or `<shellapi.h>` in NGMP sources.
   - Use standard C++ (`<chrono>`, `<thread>`, `<mutex>`), POSIX networking (`<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`), or SDL3 primitives (`SDL_GetTicks()`, `SDL_Delay()`, `SDL_GetPrefPath()`).

2. **Platform Layer Isolation**:
   - Low-level network socket and OS calls must reside exclusively in `Core/GameEngineDevice/`.
   - Higher-level network logic under `GeneralsMD/Code/GameEngine/Source/GameNetwork/GeneralsOnline/` must use pure abstract interfaces and cross-platform abstractions.

3. **Thread Safety & UI Main Thread Separation**:
   - Network callbacks from HTTP/WebSocket/P2P threads **MUST NOT** directly access or mutate UI controls (`GameWindow`, `WOLLobbyMenu`, etc.).
   - Network events must be posted to a thread-safe thread/FIFO queue, consumed strictly on the main render thread during `Shell::update()`.

4. **Credential & Token Storage**:
   - Do NOT use Windows Credential Manager.
   - Save session JWT tokens in an obfuscated local configuration file under the directory provided by `SDL_GetPrefPath("GeneralsX", "GeneralsOnline")`.

5. **Server Endpoint Environment**:
   - Development backend server target: `ws://192.168.1.120:9001/ws` (WebSocket) and `http://192.168.1.120:9001/api` (REST).
   - Server URLs must be configurable via INI/JSON config rather than hardcoded string literals.

6. **Cross-Platform Math & Endianness**:
   - Ensure network packet data serialization handles network byte order (`htons`/`ntohs`, `htonl`/`ntohl`) explicitly to support cross-play between x86_64 Linux and ARM64 macOS.

7. **Reference repositories**:
   - There are two reference repositories for NGMP, take a look on these to understand the protocol and implementation details:
     1. `references/GameClient`: The client-side implementation.
     2. `references/GameServer`: The server-side implementation.

8. **REST API & Routing Conventions (Lessons Learned)**:
   - **Environment/Contract Prefix**: The C# Kestrel backend strictly requires environment and contract version routing. Do **NOT** use `GetServerRESTEndpoint()` directly to build URLs. Always use `NGMP::GetAPIEndpoint("EndpointName")` (e.g. `NGMP::GetAPIEndpoint("Lobbies")`), which appends the required `/env/dev/contract/1/` prefix automatically.
   - **Authorization**: Almost all endpoints require authorization. You must inject the `Authorization: Bearer <token>` HTTP header in all requests (GET, POST, PUT, DELETE). Missing this will result in a `401 Unauthorized` response.
   - **HTTP Methods & Payloads**: Backend endpoints have strict method bindings (`[HttpGet]`, `[HttpPut]`, `[HttpPost]`). For example, creating a lobby requires `PUT /Lobbies`, while joining a lobby requires `PUT /Lobby/{id}`. The JSON payloads must also contain all expected fields (even if default or empty), or the server will reject the request with `400 Bad Request`.

9. **WebSocket Protocol Conventions (Lessons Learned)**:
   - **Message ID Routing**: The backend `WebSocketController` strictly uses integer `msg_id` values to route actions, NOT string action types (e.g., `msg_id: 1` for sending chat, `msg_id: 2` for receiving chat, `msg_id: 4` for member list updates). Always include the correct `msg_id` in sent JSON payloads and use it to parse incoming messages.
   - **Network Rooms (Lobby Registration)**: The backend requires the client to explicitly register into a network room to receive lobby and player updates. When initializing the global lobby chat, you must send `{"msg_id": 3, "room": 0}` (NETWORK_ROOM_CHANGE_ROOM). If you fail to do this, the server considers you in room `-1`, which causes the HTTP `GET /Lobbies` endpoint to return `0 lobbies` (as it filters by your current room) and prevents you from receiving player list updates (`msg_id: 4`).
   - **Heartbeats**: The WebSocket server automatically drops connections if a PING is not received within a timeout window (often 20s). The client must periodically send `{"msg_id": 8}` (ping) at least every 10 seconds to keep the socket alive.
