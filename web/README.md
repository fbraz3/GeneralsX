# GeneralsX Web Infrastructure

Independent web infrastructure for the GeneralsX WebAssembly browser
prototype: a static launcher shell (`apps/launcher`, deployed to Cloudflare
Pages as `play.generalsx.org`) and a Cloudflare Worker + Durable Object
backend (`apps/worker`) that relays WebRTC room signaling and issues
short-lived TURN credentials.

**This tree never stores, embeds, or proxies retail game assets or engine
binaries.** The launcher only downloads assets at runtime from an
operator-configured, integrity-verified origin (see
`apps/launcher/src/config.ts` and `apps/launcher/src/assets/asset-manager.ts`).

## Layout

```
web/
├── packages/shared/     Protocol, manifest, and security-header types/logic
│                        shared by the launcher and the Worker.
└── apps/
    ├── launcher/        Static Vite/TypeScript site for Cloudflare Pages.
    └── worker/          Cloudflare Worker + Durable Object for signaling
                         and TURN credential issuance.
```

Cross-package imports use the `@generalsx-web/shared` workspace package
(subpath exports like `@generalsx-web/shared/protocol`); there is no build
step for `packages/shared` — Vite, Vitest, and Wrangler all consume its
TypeScript sources directly via esbuild.

## Stack

- **TypeScript** (strict, `noUncheckedIndexedAccess`, `exactOptionalPropertyTypes`)
- **Vite** for the static launcher build
- **Wrangler** for the Worker (Durable Objects, dev server, deploy)
- **Vitest** for unit tests in every package
- **ESLint** (flat config, `typescript-eslint`) for linting

All dependencies are pinned to exact versions in each `package.json`.

## Setup

```bash
cd web
npm install
```

## Common commands (run from `web/`)

```bash
npm run lint        # ESLint across every workspace
npm run typecheck   # tsc --noEmit in every workspace
npm run test        # Vitest in every workspace
npm run build       # Vite build (launcher) + wrangler dry-run bundle (worker)
```

Per-package dev servers:

```bash
npm run dev -w @generalsx-web/launcher   # Vite dev server on :5173
npm run dev -w @generalsx-web/worker     # wrangler dev on :8787
```

## Launcher (`apps/launcher`)

- `src/config.ts` — frozen, build-time `LauncherConfig` (manifest URL,
  signaling Worker URL, default room capacity). Immutable by construction
  (`Object.freeze`) so no runtime code path can redirect asset/engine
  fetches to an unexpected origin.
- `src/engine-manifest.ts` — fetches the `EngineManifest` JSON document and
  validates it with `@generalsx-web/shared/manifest` before trusting it.
- `src/assets/asset-manager.ts` — integrity-aware downloader/cache: fetches
  each manifest asset strictly from `manifest.assetBaseUrl`, verifies its
  SHA-256 digest with `crypto.subtle`, and only writes verified bytes to the
  Cache Storage API (`caches`). A digest mismatch throws
  `AssetIntegrityError` and the bytes are never cached.
- `src/ui/` — canvas, loading overlay, error overlay, settings panel, and
  room create/join panel, wired together in `src/main.ts`. Instantiating the
  actual Emscripten engine module is out of scope for this scaffold; the
  seam is `startEngineBoot()` in `src/main.ts`.
- The generated Emscripten module exposes `Module.generalsxAudio`. Launcher
  integration should call `bindUserGesture(canvas)` or invoke `unlock()`
  directly from a click/pointer handler, then use `mountArchives()` to
  attach the base-game and Zero Hour audio BIG files after engine boot.
  The bridge resumes interrupted WebAudio contexts and requests MiniAudio
  device recovery after browser lifecycle or output-device changes.
- `src/net/` — `SignalingClient` (WebSocket wrapper for the room protocol;
  see **Room join lifecycle** below for its connect/leave guarantees),
  `fetchIceServers` (calls the Worker's `/turn-credentials`; invoked by
  the bridge itself, per join), and `WebRtcUdpBridge` (see below).
- `scripts/write-headers.ts` — post-build step that renders
  `dist/_headers` (Cloudflare Pages header file) from the same
  `renderPagesHeadersFile` policy the Worker uses, so COOP/COEP/CORP/CSP
  never drift between the static site and the Worker.
- `wrangler.toml` — documents the Cloudflare Pages build settings and
  custom-domain wiring for `play.generalsx.org` (Pages projects are
  typically configured via the dashboard or `wrangler pages deploy`; this
  file is reference documentation, not an active binding config).

## Worker (`apps/worker`)

- `src/durable-objects/room-logic.ts` — pure, runtime-agnostic room state
  machine (stable slot assignment, roster, capacity, host bookkeeping).
  Fully unit tested without any Cloudflare runtime dependency.
- `src/durable-objects/room-do.ts` — thin `RoomDurableObject` adapter that
  wires `room-logic.ts` to the Workers WebSocket API: validates every
  inbound message with `@generalsx-web/shared/protocol`, relays
  offer/answer/ICE only to the addressed slot, broadcasts roster updates,
  and cleans up on disconnect.
- `src/turn/turn-credentials.ts` — requests short-lived Cloudflare Realtime
  TURN credentials using `env.TURN_KEY_ID` / `env.TURN_KEY_API_TOKEN`
  secrets. The long-lived API token is only ever sent in the
  `Authorization` header to `rtc.live.cloudflare.com`; only the resulting
  short-lived `iceServers` payload is returned to the client.
- `src/index.ts` — routes `/room` (WebSocket upgrade to the Durable
  Object), `/turn-credentials`, and applies COOP/COEP/CORP/CSP/CORS headers
  (via `@generalsx-web/shared/security-headers`) to every response.
- `wrangler.toml` — Durable Object binding + SQLite migration, non-secret
  `vars` (allowed/signaling/asset origins), a commented-out R2 binding
  placeholder for a *future* authorized asset-delivery worker, and
  custom-domain notes for `signaling.generalsx.org`.

### Secrets (never committed)

```bash
wrangler secret put TURN_KEY_ID
wrangler secret put TURN_KEY_API_TOKEN
```

## Security headers

`packages/shared/src/security-headers.ts` is the single source of truth for
COOP/COEP/CORP/CSP/CORS, consumed by:

- the Worker, for every dynamic response (`apps/worker/src/index.ts`), and
- the static launcher build, via the generated `dist/_headers` file
  (`apps/launcher/scripts/write-headers.ts`).

CORS only reflects an `Origin` that exactly matches the configured
allowlist (`ALLOWED_ORIGINS` var) — no wildcards, no suffix matching.

## Signaling protocol

Defined in `packages/shared/src/protocol.ts`:

- `join` → server replies `welcome` (assigned slot, capacity, roster) and
  broadcasts `roster` to existing peers.
- `offer` / `answer` / `ice` → relayed only to the addressed slot (`to`),
  never broadcast; the server rewrites the sender field to `from` so
  clients cannot spoof their slot.
- `leave` / disconnect → frees the slot, broadcasts `peer-left` and an
  updated `roster`.
- Every message is validated (size limits, room id / name character sets,
  slot bounds, SDP length) before it is trusted; invalid input yields a
  typed `error` message instead of a crash or silent drop.

## WebRTC UDP bridge (`apps/launcher/src/net/webrtc-udp-bridge.ts`)

Emulates a small UDP-like transport over WebRTC unordered DataChannels, so
the existing GeneralsX engine networking code (written for real UDP
sockets) can run unmodified in the browser. `main.ts` constructs a
`WebRtcUdpBridge` from the same `SignalingClient` instance used by the room
UI, plus the signaling Worker's base URL (for on-demand TURN credential
fetches — see **Room join lifecycle** below), and publishes it as
`window.GeneralsXUdp` *before* any (future) engine module instantiation.

The bridge mirrors the wire format and ABI of the engine repository's
development-harness reference implementation
(`wasm/webrtc_udp.js`) bit-for-bit, so either transport talks to the
native engine module unmodified. **Every address is a host-order
`uint32`, never a string** — `bind()`/`localIP()`/`hostIP()` return
numbers (`0` means "unassigned"), `send()` takes a numeric `destIP` and
returns the integer count of peers the datagram was handed to a
DataChannel for, and `recv()` resolves `{ ip, port, data }` (matching the
field names the engine's Emscripten glue reads directly off the packet
object). Display-only strings (dotted-quad IPs) are confined to
`status()`, which the launcher UI reads for its room panel.

- **Addressing** — every stable room slot (0..capacity-1) maps to a
  synthetic IPv4 address `10.0.0.(slot+1)`, encoded as `uint32`; the
  reserved broadcast address is `0xffffffff` (not `10.0.0.255`), which
  fans a `send()` out to every connected peer.
- **Framing** — each DataChannel message is a 4-byte header (2-byte
  little-endian source port, 2-byte little-endian destination port)
  followed by the raw payload. Anything shorter than the header, or with
  an oversized payload, is dropped as malformed, never queued; incoming
  messages are also validated to be an `ArrayBuffer` (not a `Blob` or
  `string`) before being parsed.
- **Negotiation** — "perfect negotiation" with a deterministic
  polite/impolite role per peer pair (the lower slot is always impolite,
  creates the `generalsx-udp` DataChannel, and wins glare; the higher
  slot is polite and yields), so either side can (re)negotiate without a
  signaling-layer lock. Works with both direct/STUN and TURN-relayed ICE
  candidates.
- **Reliability** — DataChannels are `{ ordered: false, maxRetransmits: 5
  }`: GeneralsX's own engine netcode already implements application-level
  reliability (acks/resends) on top of UDP, so the transport stays close
  to fire-and-forget semantics — with a small bounded retransmit count
  rather than unlimited, so a lossy link cannot grow the send buffer
  without bound — instead of stacking a second, redundant retry/ordering
  layer on top.
- **Safety bounds** — each bound port's inbox evicts its oldest queued
  datagram once it hits `maxInboxPacketsPerPort` (default 256), and
  outgoing sends are dropped (not queued or blocked) once a channel's
  `bufferedAmount` exceeds `maxBufferedAmountBytes` (default 64 KiB),
  mirroring how a real UDP socket drops under congestion.
- **Explicit errors** — invalid ports/addresses/payload types and using an
  unbound port throw a typed `UdpBridgeError`; network-level conditions
  (unknown/disconnected peer, backpressure) instead return `0`/`null`,
  matching best-effort UDP semantics.
- Peer connections are torn down cleanly on roster departure, an explicit
  `peer-left` signal, or the signaling socket closing.

### Room join lifecycle

`joinRoom()`/`leaveRoom()` (used both by the engine ABI and the launcher's
room panel) and `SignalingClient.connect()`/`leave()` are all designed so
that a rejoin, a room switch, or a rapid double-click can never leave a
duplicate room membership or stale connection behind:

- `SignalingClient.connect()` always supersedes any socket it already
  owns first — the old socket's listeners are detached before it is
  closed, so its (possibly asynchronous) close event can never fire
  after the new connection has already started emitting its own events.
  `leave()` sends a `leave` request (if still open) and then always
  closes the socket locally, so the same `SignalingClient` instance can
  safely be reused for a later `connect()` (rejoin).
- `WebRtcUdpBridge.joinRoom()` tears down any existing peers/room state
  immediately, then fetches TURN credentials **fresh for this join**
  (never once at launcher startup, where the ~10-minute credential TTL
  could expire long before a match starts) before ever opening the
  signaling connection — so peer connections are always created with
  credentials that were just issued. A bridge instance constructed
  without a `turnWorkerBaseUrl` (e.g. a future single-player/offline boot
  path that never joins a room) never calls TURN at all.
  A generation counter guards every step of this sequence, so a slower,
  superseded `joinRoom()`/`leaveRoom()` call can never race ahead of a
  newer one and resurrect torn-down state.
- TURN fetch failures are **non-fatal**: the bridge falls back to
  direct/STUN-only ICE and reports it via the `onJoinIssue` callback
  (`{ kind: "turn-unavailable" }`) so the launcher can show a **visible
  warning** in the room panel — direct-ICE fallback is explicit, never
  silent. A signaling socket closing before a room join ever completes is
  reported as `{ kind: "join-failed" }`, which the launcher routes to its
  blocking error overlay.

## Not included in this scaffold

- Instantiating the actual Emscripten/WebAssembly engine module. The
  bridge is published at `window.GeneralsXUdp` in anticipation of that
  integration, but nothing yet calls into it from engine code.
- Any retail game asset, engine binary, or asset-hosting deployment.
- Live deployment of the Worker or Pages project (this tree is
  infrastructure-only; no `wrangler deploy` / `wrangler pages deploy`
  without a real Cloudflare account, secrets, and domain provisioning
  performed out of band by an operator).
