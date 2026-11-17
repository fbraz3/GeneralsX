# Phase 06: Self-Contained Web Shell (`web/`) & Real UDP Bridge (`serve-webgpu.py`)

## Objective
1. Extract the self-contained Vite + TypeScript + Tailwind frontend from `wasm-generals` commit `25dcc3eab` (before private `@wasm/*` monorepo coupling and `MIT-PoU` licensing), add the Safari `File` worker fix (`407c6469d`), and query `/GeneralsXLanConfig` at startup.
2. Upgrade `scripts/qa/smoke/serve-webgpu.py` so `LanRelay` opens real POSIX `SOCK_DGRAM` sockets on ports `8086` and `8088` and bridges LAN discovery & game packets between WebAssembly browsers and native desktop clients (`macOS`, `Linux`, `Windows`).

---

## 1. Extract Self-Contained `web/` App (No External `@wasm/*` Monorepo)

Run from `/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis`:

```bash
git -C /Users/felipebraz/.gemini/antigravity/worktrees/wasm-generals/wasm_port_architecture_analysis \
  archive 25dcc3eab -- web/package.json web/tsconfig.json web/vite.config.ts web/index.html web/src/ \
  | tar -x -C /Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis
```

Verify that `web/package.json` contains **NO** `@wasm/runtime`, `@wasm/shell`, `@wasm/proof-of-usage`, or `@origami-ltd/ui` dependencies, and that `web/src/streaming.ts` and `web/src/ui.ts` are present on disk.

> **Do NOT extract** `web/LICENSE.md`, `web/api/`, `web/dist/`, or `web/public/`.

---

## 2. Apply Safari `File` Fix & LAN Config Fetch in `web/src/`

1. **`web/src/streaming.ts`** (from `wasm-generals` commit `407c6469d` / `web/src/archives.ts`):
   - In `ArchiveEntry`: change/add `file?: File;` alongside `handle?: FileSystemFileHandle;`.
   - In the inline worker (`workerSource`): prefer `file` (`bytes = new Uint8Array(await file.slice(start, end + 1).arrayBuffer());`) before `handle` (`await handle.getFile()`).
   - In `readLocalArchives()` (or `localArchives()`): resolve `const file = await (handle as FileSystemFileHandle).getFile();` on the main thread before posting entries to the reader worker (`File` clones across workers in Safari, Chromium, and Firefox, whereas `FileSystemFileHandle` fails in Safari).
   - Also check `generalsX` IndexedDB reads: ensure `hasSavedFolders()` and `readLocalArchives()` never hang if IndexedDB throws or lacks permission.

2. **`web/src/main.ts`**:
   - Before instantiating the Emscripten WASM module (`factory(config)`), fetch `/GeneralsXLanConfig` (ignoring failures gracefully when served on pure static hosting without relay):
     ```ts
     try {
       const lanConfig = await fetch("/GeneralsXLanConfig").then((r) => (r.ok ? r.json() : null));
       if (lanConfig && typeof lanConfig.ipInt === "number" && lanConfig.ipInt > 0) {
         (globalThis as Record<string, unknown>).generalsXLanHostIP = lanConfig.ipInt >>> 0;
       }
     } catch {
       // Static hosting without local UDP bridge falls back to virtual IP.
     }
     ```
   - Ensure `GeneralsXZH.js` import does not break `vite dev` / `vite build` (keep dynamic import or `public/` runtime URL pattern `/GeneralsXZH.js`).

---

## 3. Implement Real POSIX UDP Bridge in `scripts/qa/smoke/serve-webgpu.py`

Copy `wasm-generals/scripts/qa/smoke/serve-webgpu.py` to `scripts/qa/smoke/serve-webgpu.py` and extend `LanRelay` with real LAN UDP sockets:

1. **CLI Argument `--lan-ip`**:
   - Add `parser.add_argument("--lan-ip", default=None, help="Host IPv4 address to advertise and bind for real LAN UDP bridging")` (auto-detected via outbound UDP probe `192.0.2.1` when omitted).
2. **Helper IP Conversion & Subnet Broadcast**:
   - Compute `LAN_IP_STR`, `LAN_IP_INT` (`struct.unpack("!I", socket.inet_aton(LAN_IP_STR))[0]`), and broadcast destination `(255.255.255.255)` + directed subnet broadcast if determinable.
   - Serve `GET /GeneralsXLanConfig` returning `{"ip": LAN_IP_STR, "ipInt": LAN_IP_INT}`.
3. **Real UDP Socket Lifecycle per Port (`8086`, `8088`, etc.) in `LanRelay`**:
   - Maintain `self._udp_sockets: dict[int, socket.socket] = {}`.
   - On `register((ip, port), client)`:
     - Also send control frame `bytes((3,)) + struct.pack("!I", LAN_IP_INT)` back to the WebSocket client so JS/WASM synchronizes with the relay's real IP.
     - If `port` has no active UDP socket in `_udp_sockets`, create `sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)`:
       - `sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)`
       - `if hasattr(socket, "SO_REUSEPORT"): sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)`
       - `sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)`
       - `sock.bind(("0.0.0.0", port))` (note: binding `0.0.0.0:port` allows receiving LAN broadcasts such as `255.255.255.255:8086` and subnet broadcasts on macOS/Linux!).
       - Spawn daemon thread `_udp_recv_loop(port, sock)`.
   - **Outgoing (`WASM -> WebSocket -> Physical LAN UDP`)** in `LanRelay.route(source, destination, payload)`:
     - Forward to any other registered WebSocket clients (multi-tab support).
     - Also forward over `sock = self._udp_sockets.get(source[1])`:
       - If `destination[0] == 0xFFFFFFFF`: `sock.sendto(payload, ("255.255.255.255", destination[1]))` (catch `OSError` gracefully).
       - Else if `destination[0] != LAN_IP_INT`: `dst_ip = socket.inet_ntoa(struct.pack("!I", destination[0]))` -> `sock.sendto(payload, (dst_ip, destination[1]))`.
   - **Incoming (`Physical LAN UDP -> WebSocket -> WASM`)** in `_udp_recv_loop(port, sock)`:
     - Read `(payload, (src_ip_str, src_port)) = sock.recvfrom(65535)`.
     - Convert `src_ip_int = struct.unpack("!I", socket.inet_aton(src_ip_str))[0]`.
     - Loop prevention: skip packets whose `src_ip_int == LAN_IP_INT` and `src_port == port` **only if** they are exact self-echoes from the local machine's own socket; if running Desktop + Browser on the same machine via a secondary alias IP (`--lan-ip`), allow forwarding when `src_ip_int != LAN_IP_INT`.
     - Pack `frame = bytes((2,)) + struct.pack("!IH", src_ip_int, src_port) + payload` and send to registered WebSocket client(s) on `port`.
4. **Update `resolve_game_dirs` / headers**:
   - Preserve `Cross-Origin-Opener-Policy: same-origin` and `Cross-Origin-Embedder-Policy: require-corp` required by `SharedArrayBuffer` and `WebGPU`.
   - Make sure `web/` built files (`index.html`, `assets/*`) and `GeneralsXZH.{js,wasm,data}` + `generalsx.big` can be served together cleanly from one directory.

---

## 4. Update `.gitignore`

Add the following entries to `/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis/.gitignore`:

```gitignore
# WebAssembly / Vite shell build outputs
web/node_modules/
web/dist/
web/public/GeneralsXZH.*
```

---

## Checklist

- [x] Extract self-contained `web/` files from `wasm-generals` commit `25dcc3eab`.
- [x] Apply Safari `File` cloning fix in `web/src/streaming.ts`.
- [x] Fetch `/GeneralsXLanConfig` in `web/src/main.ts` before starting WASM.
- [x] Create `scripts/qa/smoke/serve-webgpu.py` with real POSIX UDP bridge (`8086` & `8088`) and `--lan-ip` option.
- [x] Add `web/node_modules/`, `web/dist/`, and `web/public/GeneralsXZH.*` to `.gitignore`.
