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
- `src/net/` — `SignalingClient` (WebSocket wrapper for the room protocol)
  and `fetchIceServers` (calls the Worker's `/turn-credentials`).
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

## Not included in this scaffold

- Instantiating the actual Emscripten/WebAssembly engine module.
- Any retail game asset, engine binary, or asset-hosting deployment.
- Live deployment of the Worker or Pages project (this tree is
  infrastructure-only; no `wrangler deploy` / `wrangler pages deploy`
  without a real Cloudflare account, secrets, and domain provisioning
  performed out of band by an operator).
