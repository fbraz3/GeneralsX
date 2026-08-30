# GeneralsX Web Infrastructure

Independent web infrastructure for the GeneralsX WebAssembly browser
prototype: a static launcher shell (`apps/launcher`, deployed to Cloudflare
Pages as `play.generalsx.org`) and a Cloudflare Worker + Durable Object
backend (`apps/worker`) that relays WebRTC room signaling and issues
short-lived TURN credentials.

**This tree never stores, embeds, or proxies retail game assets or engine
binaries.** The launcher only downloads assets at runtime from an
operator-configured, integrity-verified origin that the player is
authorized to use (see `apps/launcher/src/config.ts` and
`apps/launcher/src/assets/`). Manifests are produced by the operator from
their own legally obtained installation; no manifest, digest list, or asset
byte belongs in this repository.

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
- `src/assets/` — the authorized asset pipeline (see
  [Asset pipeline](#asset-pipeline) below): `manifest`-driven streaming
  downloader (`asset-manager.ts`), OPFS/in-memory file storage
  (`storage.ts`), bounded-memory virtual file system (`vfs.ts`), and the
  typed error surface (`errors.ts`).
- `src/ui/` — canvas, loading overlay, error overlay, settings panel, and
  room create/join panel, wired together in `src/main.ts`.
- `src/engine/emscripten-loader.ts` — launches the verified engine JS/wasm,
  stages manifest-mounted files into Emscripten's filesystem in bounded
  chunks, applies preferences, and reports startup failures to the launcher.
- The generated Emscripten module exposes `Module.generalsxAudio`. Launcher
  integration calls `bindUserGesture(canvas)` after runtime initialization.
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
- `scripts/build-asset-manifest.ts` — operator-side CLI
  (`npm run build:manifest -w @generalsx-web/launcher`) that streams a local,
  legally obtained install directory, hashes every file, infers its role,
  and prints a schema v2 manifest. It never copies asset bytes anywhere.
- `wrangler.toml` — documents the Cloudflare Pages build settings and
  custom-domain wiring for `play.generalsx.org` (Pages projects are
  typically configured via the dashboard or `wrangler pages deploy`; this
  file is reference documentation, not an active binding config).

## Asset pipeline

The launcher never ships assets; it downloads what an authorized origin
serves and verifies every byte. Full operator guide:
[`docs/HOWTO/WEB_ASSET_PIPELINE.md`](../docs/HOWTO/WEB_ASSET_PIPELINE.md).

### Manifest (schema v2)

`packages/shared/src/manifest.ts` defines and validates the document:

```jsonc
{
  "schemaVersion": 2,
  "engineVersion": "zh-wasm-2026.08.30",
  "assetsRevision": 7,                        // bumped on every republish
  "assetBaseUrl": "https://assets.example.org/zh/r7",
  "assets": [
    {
      "path": "engine/generalsx.wasm",
      "role": "engine-wasm",                  // strict role vocabulary
      "sizeBytes": 41582592,
      "sha256": "…64 lowercase hex chars…",
      "etag": "\"a1b2c3\"",                   // strong ETag, optional
      "mount": { "target": "/engine/generalsx.wasm",
                 "order": 1, "streaming": false }
    }
  ]
}
```

Roles: `engine-js`, `engine-wasm`, `engine-data`, `big-base`,
`big-expansion`, `script`, `font`. Validation rejects a manifest unless it
has exactly one `engine-js` and one `engine-wasm`, at most one
`engine-data`, at least one `big-base`, unique `path`/`mount.target`/
`mount.order`, expansion orders strictly above every base order,
`streaming` matching the role (BIG archives stream, everything else does
not), absolute mount targets, traversal-free relative paths, strong-only
ETags, per-asset sizes in `1 B … 8 GiB`, and a total under 32 GiB.

### Download and verification

`AssetManager.ensureAssets()` resolves each URL strictly under
`assetBaseUrl`, then for every asset:

1. **Reuses** an already-verified file (content-addressed by digest) with no
   network request at all — immutable cache keys mean a re-download only
   happens when the bytes actually change.
2. **Streams** the response body straight into a temp file, hashing
   incrementally with `@generalsx-web/shared/sha256`. Chunks are split to
   4 MiB, so JS heap use stays bounded no matter how large the archive is
   (`crypto.subtle.digest` is deliberately *not* used: it is one-shot and
   would require buffering the whole file).
3. **Verifies** the exact `sizeBytes` and `sha256` before the file is
   visible. Too many bytes aborts the stream mid-flight; too few, or a wrong
   digest, deletes the temp file and its resume record.
4. **Promotes** temp → final atomically (`FileSystemFileHandle.move()` where
   available, otherwise a chunked copy plus a post-copy size check).
5. **Resumes** an interrupted download with `Range: bytes=N-` +
   `If-Range: <etag>`. The resume record must match digest, size, URL,
   ETag, `engineVersion`, and `assetsRevision`; the bytes already on disk
   are re-hashed before anything is appended. A `200`, a `416`, or a
   contradictory `Content-Range` restarts cleanly from zero.

Storage is rooted at `${engineVersion}-r${assetsRevision}`; publishing a new
revision creates a new root and prunes the old ones, and unreferenced files
inside the live root are garbage-collected. Before downloading, the manager
checks `navigator.storage.estimate()` and raises `AssetStorageQuotaError`
(with required/available bytes) rather than dying halfway through.

Downloads run with bounded concurrency (default 3). Cancellation, quota
exhaustion, and dropped connections keep the partial file plus its resume
record; integrity failures never do.

### Storage backends

`openAssetStorage()` picks OPFS
(`navigator.storage.getDirectory()`) when the browser supports it and falls
back to `MemoryAssetStorage` otherwise. Both implement the same
`AssetStorage`/`AssetFileStore`/`AssetWriteStream` interfaces with separate
`files/`, `tmp/`, and `meta/` namespaces, so every code path — including
resume and atomic promotion — is exercised by the fallback in tests.

### Virtual file system

`AssetVfs` maps mount targets to stored files for the engine. Streaming BIG
archives are read through `read(target, offset, length)`, served from 1 MiB
chunks behind a 32 MiB LRU budget, so a multi-gigabyte archive never lands
in the JS heap. `readAll()` is reserved for small non-streaming assets and
refuses streaming roles and files over 64 MiB.

### Origin requirements

An authorized asset origin (R2, S3, or any static host) must send
`Accept-Ranges: bytes`, strong `ETag`s, long-lived
`Cache-Control: public, max-age=31536000, immutable`, and CORS headers that
allow the launcher origin to read `ETag`, `Content-Range`, and
`Content-Length`.



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
`window.GeneralsXUdp` before engine module instantiation.

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
  without a `turnWorkerBaseUrl` (e.g. a single-player/offline boot
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

## Runtime staging

Verified files remain in OPFS between sessions. Engine JS and wasm are
executed from verified Blob URLs. The current legacy Emscripten filesystem
API is synchronous, so BIG archives must still be copied from OPFS into
MEMFS before `main()` runs; the loader performs that copy in 4 MiB chunks
to avoid a second whole-archive JavaScript buffer. The wasm build therefore
retains its 4 GiB memory ceiling. Replacing MEMFS residency entirely requires
a worker-hosted synchronous OPFS backend and is a later optimization.

## Not included

- Instantiating the actual Emscripten/WebAssembly engine module. The
  bridge is published at `window.GeneralsXUdp` in anticipation of that
  integration, but nothing yet calls into it from engine code.
- Any retail game asset, published engine binary, or asset-hosting deployment.
- Live deployment of the Worker or Pages project (this tree is
  infrastructure-only; no `wrangler deploy` / `wrangler pages deploy`
  without a real Cloudflare account, secrets, and domain provisioning
  performed out of band by an operator).
