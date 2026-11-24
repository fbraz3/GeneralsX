#!/usr/bin/env python3
"""Serve Emscripten artifacts with the headers required by shared WebAssembly memory.

Usage:
    ./scripts/qa/smoke/serve-webgpu.py DIRECTORY [--port PORT]
"""

import argparse
import base64
import functools
import hashlib
import hmac
import json
import os
import re
import socket
import ssl
import secrets
import struct
import time
import urllib.parse
import urllib.request
import sys
import threading
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit


def detect_lan_ip(explicit_ip: str | None = None) -> tuple[str, int]:
    """Resolve the host IPv4 address advertised to browser and desktop LAN peers."""
    if explicit_ip:
        ip_str = explicit_ip
    else:
        probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            probe.connect(("192.0.2.1", 80))
            ip_str = probe.getsockname()[0]
        except OSError:
            ip_str = "127.0.0.1"
        finally:
            probe.close()
    ip_int = struct.unpack("!I", socket.inet_aton(ip_str))[0]
    return ip_str, ip_int


class LanRelay:
    """Route browser WebSocket datagrams by IPv4 address/port and bridge to physical LAN UDP."""

    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._clients: dict[tuple[int, int], "WebGPURequestHandler"] = {}
        self._udp_sockets: dict[int, socket.socket] = {}
        self._udp_workers: dict[int, tuple[threading.Event, threading.Thread]] = {}
        self._retiring_workers: dict[int, threading.Thread] = {}
        self._port_locks: dict[int, threading.Lock] = {}
        self.lan_ip_str = "127.0.0.1"
        self.lan_ip_int = 0x7F000001
        self._registrations = 0
        self._broadcasts = 0
        self._directed = 0
        self._delivered = 0

    def configure_lan_ip(self, ip_str: str, ip_int: int) -> None:
        with self._lock:
            self.lan_ip_str = ip_str
            self.lan_ip_int = ip_int

    def _port_lock(self, port: int) -> threading.Lock:
        with self._lock:
            lock = self._port_locks.get(port)
            if lock is None:
                lock = threading.Lock()
                self._port_locks[port] = lock
            return lock

    def _ensure_udp_socket(self, port: int) -> None:
        while True:
            retiring: threading.Thread | None = None
            with self._port_lock(port):
                with self._lock:
                    if port in self._udp_sockets or not any(key[1] == port for key in self._clients):
                        return
                    retiring = self._retiring_workers.get(port)
                if retiring is None or retiring is threading.current_thread() or not retiring.is_alive():
                    with self._lock:
                        if self._retiring_workers.get(port) is retiring:
                            self._retiring_workers.pop(port, None)
                        if port in self._udp_sockets or not any(key[1] == port for key in self._clients):
                            return
                        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                        if hasattr(socket, "SO_REUSEPORT"):
                            try:
                                sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
                            except OSError:
                                pass
                        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
                        sock.settimeout(0.5)
                        try:
                            sock.bind(("0.0.0.0", port))
                        except OSError as exc:
                            print(f"[LanRelay] Warning: failed to bind UDP 0.0.0.0:{port}: {exc}", file=sys.stderr)
                            sock.close()
                            return
                        stop_event = threading.Event()
                        thread = threading.Thread(
                            target=self._udp_recv_loop,
                            args=(port, sock, stop_event),
                            daemon=True,
                        )
                        self._udp_sockets[port] = sock
                        self._udp_workers[port] = (stop_event, thread)
                        thread.start()
                    return
            # Join outside _port_lock(port) and self._lock while keeping _retiring_workers[port] visible.
            retiring.join()
            with self._port_lock(port):
                with self._lock:
                    if self._retiring_workers.get(port) is retiring and not retiring.is_alive():
                        self._retiring_workers.pop(port, None)

    def _udp_recv_loop(self, port: int, sock: socket.socket, stop_event: threading.Event) -> None:
        while not stop_event.is_set():
            try:
                payload, (src_ip_str, src_port) = sock.recvfrom(65535)
            except socket.timeout:
                continue
            except OSError:
                break
            try:
                src_ip_int = struct.unpack("!I", socket.inet_aton(src_ip_str))[0]
            except OSError:
                continue
            with self._lock:
                if stop_event.is_set():
                    return
                if src_ip_int == self.lan_ip_int and src_port == port:
                    continue
                targets = [client for (_, cport), client in self._clients.items() if cport == port]
                self._delivered += len(targets)
            if not targets:
                continue
            message = bytes((2,)) + struct.pack("!IH", src_ip_int, src_port) + payload
            for target in targets:
                try:
                    target.send_websocket_frame(2, message)
                except OSError:
                    self.unregister(target)

    def register(self, key: tuple[int, int], client: "WebGPURequestHandler") -> None:
        with self._lock:
            self._clients[key] = client
            self._registrations += 1
            lan_ip_int = self.lan_ip_int
        if lan_ip_int:
            try:
                client.send_websocket_frame(2, bytes((3,)) + struct.pack("!I", lan_ip_int))
            except OSError:
                self.unregister(client)
                return
        self._ensure_udp_socket(key[1])

    def unregister(self, client: "WebGPURequestHandler") -> None:
        with self._lock:
            stale = [key for key, value in self._clients.items() if value is client]
            for key in stale:
                del self._clients[key]
            candidate_ports = sorted({key[1] for key in stale})
        for port in candidate_ports:
            with self._port_lock(port):
                with self._lock:
                    if any(key[1] == port for key in self._clients):
                        continue
                    sock = self._udp_sockets.pop(port, None)
                    stop_event, thread = self._udp_workers.pop(port, (None, None))
                if stop_event is not None:
                    stop_event.set()
                if sock is not None:
                    try:
                        sock.close()
                    except OSError:
                        pass
                if thread is not None:
                    if thread is not threading.current_thread() and thread.is_alive():
                        thread.join(timeout=1.5)
                    if thread.is_alive():
                        with self._lock:
                            self._retiring_workers[port] = thread

    def route(self, source: tuple[int, int], destination: tuple[int, int], payload: bytes) -> None:
        with self._lock:
            if destination[0] == 0xFFFFFFFF:
                self._broadcasts += 1
                targets = [
                    client
                    for key, client in self._clients.items()
                    if key[1] == destination[1] and key != source
                ]
            else:
                self._directed += 1
                target = self._clients.get(destination)
                if target is not None:
                    targets = [target]
                elif destination[0] == self.lan_ip_int:
                    targets = [
                        client
                        for key, client in self._clients.items()
                        if key[1] == destination[1] and key != source
                    ]
                else:
                    targets = []
            self._delivered += len(targets)
            udp_sock = self._udp_sockets.get(source[1])
            lan_ip_int = self.lan_ip_int

        message = bytes((2,)) + struct.pack("!IH", source[0], source[1]) + payload
        for target in targets:
            try:
                target.send_websocket_frame(2, message)
            except OSError:
                self.unregister(target)

        if udp_sock is not None:
            try:
                if destination[0] == 0xFFFFFFFF:
                    udp_sock.sendto(payload, ("255.255.255.255", destination[1]))
                elif destination[0] != lan_ip_int:
                    dst_ip = socket.inet_ntoa(struct.pack("!I", destination[0]))
                    udp_sock.sendto(payload, (dst_ip, destination[1]))
            except OSError:
                pass

    def status(self) -> dict[str, object]:
        with self._lock:
            clients = [
                {"ip": ".".join(str((ip >> shift) & 0xFF) for shift in (24, 16, 8, 0)), "port": port}
                for ip, port in sorted(self._clients)
            ]
            return {
                "clients": clients,
                "registrations": self._registrations,
                "broadcasts": self._broadcasts,
                "directed": self._directed,
                "delivered": self._delivered,
            }


LAN_RELAY = LanRelay()

TLS_PORT = 8765  # the listener started with --cert; the plain one is only for local convenience
CONFIG_DIR = Path.home() / "Library" / "Application Support" / "GeneralsX"

# GeneralsX @build Codex 04/08/2026 Provide deterministic local browser isolation.
class WebGPURequestHandler(SimpleHTTPRequestHandler):
    # HTTP/1.0 closes per response; parallel range reads then raced the close and the browser
    # reported ERR_CONTENT_LENGTH_MISMATCH.
    protocol_version = "HTTP/1.1"
    extensions_map = {
        **SimpleHTTPRequestHandler.extensions_map,
        ".wasm": "application/wasm",
    }

    def setup(self) -> None:
        super().setup()
        self._websocket_send_lock = threading.Lock()

    # GeneralsX @feature Codex 05/08/2026 Persist browser runtime logs so tab crashes leave evidence.
    def do_POST(self) -> None:
        path = urlsplit(self.path).path
        if path != "/GeneralsXLog":
            self.send_error(404)
            return
        length = min(int(self.headers.get("Content-Length", 0) or 0), 4 * 1024 * 1024)
        body = self.rfile.read(length)
        log_dir = Path.home() / "Library" / "Logs" / "GeneralsX"
        log_dir.mkdir(parents=True, exist_ok=True)
        client = self.client_address[0].replace(":", "_")
        with open(log_dir / f"{client}.log", "ab") as handle:
            handle.write(body)
        self.send_response(204)
        self.send_header("Content-Length", "0")
        self.end_headers()

    CRASH_PAGE = b"""<!doctype html><meta charset="utf-8"><title>GeneralsX crashed</title>
<body style="background:#111315;color:#f4f1e8;font-family:system-ui;padding:40px;max-width:900px;margin:auto">
<h1 style="color:#e1a94b">GeneralsX hit the memory guard</h1>
<p>The runtime log below was saved before the page was torn down. A copy is also on the
server at <code>~/Library/Logs/GeneralsX/</code>.</p>
<p><a style="color:#e1a94b" href="/GeneralsXZH.html">Reload the game</a></p>
<pre id="log" style="background:#000;padding:16px;overflow:auto;max-height:70vh;white-space:pre-wrap"></pre>
<script>
document.getElementById("log").textContent =
    (localStorage.getItem("generalsX.lastCrashAt") || "") + "\\n" +
    (localStorage.getItem("generalsX.lastCrashLog") || "No saved log found.");
</script>"""

    def do_GET(self) -> None:
        path = urlsplit(self.path).path
        if path == "/GeneralsXCrash":
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(self.CRASH_PAGE)))
            self.end_headers()
            self.wfile.write(self.CRASH_PAGE)
            return
        if path == "/GeneralsXLan" and self.headers.get("Upgrade", "").lower() == "websocket":
            self.handle_lan_websocket()
            return
        if path == "/GeneralsXLanConfig":
            payload = json.dumps(
                {"ip": LAN_RELAY.lan_ip_str, "ipInt": LAN_RELAY.lan_ip_int},
                separators=(",", ":"),
            ).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)
            return
        if path == "/GeneralsXShare":
            # LAN invite: the address other machines on this network use to reach this host.
            probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            try:
                probe.connect(("192.0.2.1", 80))  # never sent; just picks the outbound interface
                address = probe.getsockname()[0]
            except OSError:
                address = "127.0.0.1"
            finally:
                probe.close()
            # Advertise the active listener scheme and port; remote guests require HTTPS for SharedArrayBuffer.
            scheme = "https" if getattr(self.server, "is_tls", False) else "http"
            port = self.server.server_port
            body = json.dumps({"url": f"{scheme}://{address}:{port}/"}).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/GeneralsXAssets":
            self.send_asset_manifest()
            return
        if path.endswith(".big"):
            self._cacheable = True
        range_header = self.headers.get("Range")
        if range_header and self.send_range(range_header):
            return
        if path == "/GeneralsXLanStatus":
            payload = json.dumps(LAN_RELAY.status(), separators=(",", ":")).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)
            return
        super().do_GET()

    def end_headers(self) -> None:
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Accept-Ranges", "bytes")
        # GeneralsX @feature Codex 05/08/2026 Game archives are streamed in ranges and never change during a
        # session, so they must stay cacheable; no-store here would refetch every chunk the engine reads.
        if getattr(self, "_cacheable", False):
            self.send_header("Cache-Control", "public, max-age=3600")
        else:
            self.send_header("Cache-Control", "no-store")
        super().end_headers()

    # GeneralsX @feature Codex 06/08/2026 First-run: game archives come from the player's own install.
    # Default location is ~/GeneralsX/{Generals,GeneralsZH}; custom paths (e.g. a Steam install) go in
    # game_paths.txt next to the server data, one KEY=path per line: GENERALS=... / ZEROHOUR=...
    GAME_PATHS_FILE = CONFIG_DIR / "game_paths.txt"

    def resolve_game_dirs(self) -> None:
        root = Path(self.directory)
        configured = {}
        try:
            for line in self.GAME_PATHS_FILE.read_text().splitlines():
                key, _, value = line.partition("=")
                if key.strip() in ("GENERALS", "ZEROHOUR") and value.strip():
                    configured[key.strip()] = Path(value.strip()).expanduser()
        except OSError:
            pass
        env_map = {
            "GENERALS": os.environ.get("CNC_GENERALS_PATH"),
            "ZEROHOUR": os.environ.get("CNC_GENERALS_ZH_PATH"),
        }
        for key, raw in env_map.items():
            if raw and raw.strip():
                configured[key] = Path(raw.strip()).expanduser()
        candidates = {
            "GENERALS": [
                configured.get("GENERALS"),
                Path.home() / "GeneralsX" / "Generals",
                Path.home() / "GeneralsX" / "data" / "Generals",
            ],
            "ZEROHOUR": [
                configured.get("ZEROHOUR"),
                Path.home() / "GeneralsX" / "GeneralsZH",
                Path.home() / "GeneralsX" / "data" / "GeneralsZH",
            ],
        }
        has_retail_big = lambda d: d is not None and d.is_dir() and any(
            p.name.lower() != "generalsx.big" for p in d.glob("*.big")
        )
        mounts = {"GENERALS": "Generals", "ZEROHOUR": "GeneralsZH"}
        for key, mount in mounts.items():
            link = root / mount
            if has_retail_big(link):
                continue
            target = next((c for c in candidates[key] if has_retail_big(c)), None)
            if target is not None:
                if link.is_symlink() or link.exists():
                    try:
                        link.unlink()
                    except OSError:
                        continue
                link.symlink_to(target)

    def send_asset_manifest(self) -> None:
        self.resolve_game_dirs()
        root = Path(self.directory)
        entries = []
        for mount in ("GeneralsZH", "Generals"):
            directory = root / mount
            if not directory.is_dir():
                continue
            for archive in sorted(directory.glob("*.big")):
                entries.append({
                    "mount": f"/{mount}",
                    "name": archive.name,
                    "url": f"/{mount}/{archive.name}",
                    "size": archive.stat().st_size,
                })
            # Also expose loose Data/Cursors and Data/Scripts files required by SDL3Mouse and Skirmish AI.
            for sub in ("Cursors", "Scripts"):
                subdir = directory / "Data" / sub
                if not subdir.is_dir():
                    continue
                for item in sorted(subdir.iterdir()):
                    if item.is_file():
                        entries.append({
                            "mount": f"/{mount}/Data/{sub}",
                            "name": item.name,
                            "url": f"/{mount}/Data/{sub}/{item.name}",
                            "size": item.stat().st_size,
                        })
        payload = json.dumps({
            "entries": entries,
            "missing": not any(
                e["mount"] == "/GeneralsZH" and e["name"].lower() != "generalsx.big"
                for e in entries
            ),
            "defaultPath": str(Path.home() / "GeneralsX"),
            "configPath": str(self.GAME_PATHS_FILE),
        }, separators=(",", ":")).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)

    # GeneralsX @feature Codex 05/08/2026 Range support lets the browser stream .big archives on demand
    # instead of the build packaging every byte into one preloaded blob held in memory.
    def send_range(self, range_header: str) -> bool:
        fs_path = Path(self.translate_path(self.path))
        if not fs_path.is_file():
            return False
        match = re.fullmatch(r"bytes=(\d*)-(\d*)", range_header.strip())
        if not match:
            return False

        size = fs_path.stat().st_size
        start_text, end_text = match.groups()
        if start_text:
            start = int(start_text)
            end = int(end_text) if end_text else size - 1
        elif end_text:  # suffix range: last N bytes
            start = max(0, size - int(end_text))
            end = size - 1
        else:
            return False

        if start >= size or start > end:
            self.send_response(416)
            self.send_header("Content-Range", f"bytes */{size}")
            self.send_header("Content-Length", "0")
            self.end_headers()
            return True

        end = min(end, size - 1)
        length = end - start + 1
        self._cacheable = True
        self.send_response(206)
        self.send_header("Content-Type", self.guess_type(str(fs_path)))
        self.send_header("Content-Range", f"bytes {start}-{end}/{size}")
        self.send_header("Content-Length", str(length))
        self.end_headers()
        with open(fs_path, "rb") as handle:
            handle.seek(start)
            remaining = length
            while remaining > 0:
                chunk = handle.read(min(1 << 20, remaining))
                if not chunk:
                    break
                self.wfile.write(chunk)
                remaining -= len(chunk)
        return True

    def handle_lan_websocket(self) -> None:
        key = self.headers.get("Sec-WebSocket-Key")
        if not key:
            self.send_error(400, "Missing Sec-WebSocket-Key")
            return

        accept = base64.b64encode(
            hashlib.sha1((key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode("ascii")).digest()
        ).decode("ascii")
        self.send_response(101, "Switching Protocols")
        self.send_header("Upgrade", "websocket")
        self.send_header("Connection", "Upgrade")
        self.send_header("Sec-WebSocket-Accept", accept)
        self.end_headers()
        self.wfile.flush()
        # Set a fixed 1.0s socket timeout once before concurrent reads/writes; _read_exact()
        # retries on timeout so idle clients stay connected while stalled sends fail within 1.0s.
        self.connection.settimeout(1.0)

        source: tuple[int, int] | None = None
        try:
            while True:
                opcode, payload = self.read_websocket_frame()
                if opcode == 8:
                    break
                if opcode == 9:
                    self.send_websocket_frame(10, payload)
                    continue
                if opcode != 2 or not payload:
                    continue
                if payload[0] == 1 and len(payload) == 7:
                    source = struct.unpack("!IH", payload[1:])
                    LAN_RELAY.register(source, self)
                elif payload[0] == 2 and source is not None and len(payload) >= 7:
                    destination = struct.unpack("!IH", payload[1:7])
                    LAN_RELAY.route(source, destination, payload[7:])
        except (ConnectionError, OSError, ValueError):
            pass
        finally:
            LAN_RELAY.unregister(self)

    def read_websocket_frame(self) -> tuple[int, bytes]:
        header = self._read_exact(2)
        opcode = header[0] & 0x0F
        masked = bool(header[1] & 0x80)
        length = header[1] & 0x7F
        if length == 126:
            length = struct.unpack("!H", self._read_exact(2))[0]
        elif length == 127:
            length = struct.unpack("!Q", self._read_exact(8))[0]
        if length > 65535:
            raise ValueError("WebSocket frame too large")
        mask = self._read_exact(4) if masked else b""
        payload = self._read_exact(length)
        if masked:
            payload = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
        return opcode, payload

    def send_websocket_frame(self, opcode: int, payload: bytes) -> None:
        if len(payload) < 126:
            header = bytes((0x80 | opcode, len(payload)))
        else:
            header = bytes((0x80 | opcode, 126)) + struct.pack("!H", len(payload))
        with self._websocket_send_lock:
            try:
                self.connection.sendall(header + payload)
            except OSError:
                try:
                    self.connection.shutdown(socket.SHUT_RDWR)
                except OSError:
                    pass
                raise

    def _read_exact(self, length: int) -> bytes:
        data = bytearray()
        while len(data) < length:
            try:
                chunk = self.connection.recv(length - len(data))
            except (socket.timeout, TimeoutError):
                continue
            if not chunk:
                raise ConnectionError("WebSocket closed")
            data.extend(chunk)
        return bytes(data)


def main() -> int:
    """Serve one artifact directory."""
    parser = argparse.ArgumentParser()
    parser.add_argument("directory", type=Path)
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--bind", default="127.0.0.1")
    parser.add_argument("--lan-ip", default=None, help="Host IPv4 address to advertise and bind for real LAN UDP bridging")
    # GeneralsX @feature Codex 05/08/2026 TLS keeps WebGPU/SharedArrayBuffer secure-context over LAN IPs.
    parser.add_argument("--cert", type=Path)
    parser.add_argument("--key", type=Path)
    args = parser.parse_args()

    lan_ip_str, lan_ip_int = detect_lan_ip(args.lan_ip)
    LAN_RELAY.configure_lan_ip(lan_ip_str, lan_ip_int)

    directory = args.directory.resolve()
    if not directory.is_dir():
        parser.error(f"directory does not exist: {directory}")

    handler = functools.partial(WebGPURequestHandler, directory=str(directory))
    server = ThreadingHTTPServer((args.bind, args.port), handler)
    # A browser opening many parallel range reads must not out-run the accept queue.
    server.daemon_threads = True
    server.request_queue_size = 128
    server.is_tls = bool(args.cert)  # type: ignore[attr-defined]
    scheme = "http"
    if args.cert:
        import ssl

        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(args.cert, args.key)
        # Handshake lazily in the worker thread: wrapping the listening socket eagerly makes accept()
        # perform the handshake, so one slow client blocks every new connection.
        server.socket = context.wrap_socket(server.socket, server_side=True,
                                            do_handshake_on_connect=False)
        scheme = "https"
    print(f"Serving {directory} at {scheme}://{args.bind}:{args.port}/")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
