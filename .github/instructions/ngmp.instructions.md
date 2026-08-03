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
