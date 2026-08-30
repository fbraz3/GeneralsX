# Web Asset Pipeline

How an operator publishes and serves the assets that the browser launcher
(`web/apps/launcher`) downloads at runtime.

> [!IMPORTANT]
> GeneralsX ships **no retail assets**. This repository contains no game
> data, no engine binaries, no manifests, and no digest lists. Everything
> below operates on a copy of the game **you already own** and are permitted
> to host, and it is your responsibility to keep that origin private to
> users who are equally authorized.

## Overview

```
your licensed install + wasm engine build
        │
        │  npm run build:manifest -w @generalsx-web/launcher
        ▼
   manifest.json  ──────────────►  authorized origin (R2 / S3 / static host)
        │                                   ▲
        │  VITE_GENERALSX_MANIFEST_URL      │  HTTPS + Range + strong ETag
        ▼                                   │
   launcher ── AssetManager ── streams, verifies, stores in OPFS ── AssetVfs ── engine
```

## 1. Stage the files

Create a staging directory **outside this repository**:

```
staging/
├── engine/
│   ├── generalsxzh.js        # Emscripten loader        (role: engine-js)
│   ├── generalsxzh.wasm      # engine module            (role: engine-wasm)
│   └── generalsxzh.data      # optional preload bundle  (role: engine-data)
├── base/*.big                # base game archives       (role: big-base)
├── expansion/*.big           # expansion archives       (role: big-expansion)
├── scripts/*.ini|*.scb       # loose scripts/config     (role: script)
└── fonts/*.ttf|*.otf         # fonts                    (role: font)
```

Roles are inferred from this layout by
`web/packages/shared/src/asset-plan.ts`. The engine JS/wasm pair comes from
the wasm build (see `wasm/`), everything else from your own installation.

## 2. Build the manifest

```bash
cd web
npm install
npm run build:manifest -w @generalsx-web/launcher -- \
  --source /path/to/staging \
  --engine-version 2026.08.30-a1b2c3d \
  --assets-revision 7 \
  --base-url https://assets.example.org/zh/r7 \
  --engine-metadata /path/to/build/emscripten-webgl2/GeneralsMD/GeneralsXZH.engine-metadata.json \
  --out /path/to/staging/manifest.json
```

`--engine-metadata` is required and must point to the post-link JSON emitted
beside the chosen CMake WebAssembly target. It records the canonical
Generals/Zero Hour and deterministic-math profile plus SHA-256 identities for
the exact JS/wasm outputs. The manifest builder verifies both staged engine
digests before copying the profile into the manifest. Missing metadata,
standalone profile JSON, noncanonical profiles, or metadata from another build
fail closed.

The tool streams every file through an incremental SHA-256 (constant
memory), records its exact size, assigns a role and a mount entry, and
validates the result before writing it. It never copies or uploads asset
bytes.

By default, roles map to the paths consumed by the engine: base archives to
`/game-base`, expansion archives to `/game`, loose scripts to
`/game/Data/Scripts`, the first font to `/fonts/default.ttf`, and engine
artifacts to `/engine`.

**Bump `--assets-revision` on every republish.** It is part of the local
storage root (`${engineVersion}-r${assetsRevision}`), so a bump makes
clients fetch the new revision and prune the old one.

Keep `manifest.json` next to the assets (or anywhere the launcher can reach
it) — **never** commit it here; the digests and file list describe retail
content.

## 3. Publish

Upload the staging tree so each asset resolves at
`<base-url>/<manifest path>`. The origin must provide:

| Requirement | Why |
|---|---|
| `Accept-Ranges: bytes`, correct `206`/`416` | resumable downloads (`Range` + `If-Range`) |
| Strong `ETag` (never `W/"…"`) | validates a resumed byte range; weak validators are rejected by manifest validation |
| `Cache-Control: public, max-age=31536000, immutable` | asset URLs are revision-scoped and content-verified |
| CORS for the launcher origin, exposing `ETag`, `Content-Range`, `Content-Length` | the client reads these to resume safely |
| HTTPS | integrity/confidentiality in transit |

Cloudflare R2 satisfies the range/ETag/immutability requirements out of the
box; publish each revision under its own key prefix and never overwrite an
existing key. `web/apps/worker/wrangler.toml` carries a commented-out
`ASSETS_BUCKET` binding for a separately authorized delivery worker — the
signaling Worker itself never serves assets.

Point the launcher at the manifest with the
`VITE_GENERALSX_MANIFEST_URL` build variable and at the room/TURN Worker
with `VITE_GENERALSX_SIGNALING_URL`
(`web/apps/launcher/src/config.ts`).

## 4. What the client does

`AssetManager.ensureAssets()` (`web/apps/launcher/src/assets/`):

1. Validates the manifest (roles, mounts, sizes, digests, ETags, ordering).
2. Estimates free storage and fails fast with `AssetStorageQuotaError`
   rather than filling the disk halfway.
3. For each asset (bounded concurrency, default 3):
   - reuses the verified local copy if the digest already exists;
   - otherwise streams the response into a temp file, hashing incrementally
     in ≤4 MiB chunks so JS heap use stays flat;
   - aborts immediately if the stream exceeds `sizeBytes`;
   - verifies exact size **and** SHA-256, then atomically promotes
     temp → final;
   - on interruption, records a resume sidecar and continues later with
     `Range`/`If-Range`, re-hashing the on-disk prefix first;
   - deletes temp + sidecar on any integrity or size failure.
4. Prunes storage roots from other revisions and any file the manifest no
   longer references.
5. Returns an `AssetVfs` that mounts everything for the engine: BIG archives
   are read by range through a 32 MiB LRU chunk cache, never buffered whole.

Storage is OPFS when available; otherwise an in-memory backend with
identical semantics keeps the launcher (and the test suite) working.

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| `AssetIntegrityError` | The published bytes differ from the manifest. Rebuild the manifest after any re-upload. |
| `AssetSizeError` | Truncated or padded object — usually a failed/partial upload. |
| `AssetDownloadError: … ETag … refresh the manifest` | The origin's ETag no longer matches the manifest; republish under a new revision and rebuild. |
| Every boot re-downloads everything | Origin is not sending strong ETags / `Accept-Ranges`, or the browser evicted non-persistent storage. The launcher requests persistent storage; the user may have to grant it. |
| `AssetStorageQuotaError` | Not enough free origin storage. Free space, or serve a smaller asset set. |
| Manifest rejected at load | Read the validation message: role counts, duplicate mount order/target, expansion ordered before base, weak ETag, or a path escaping the base URL. |

## Related

- `web/README.md` — "Asset pipeline" section (schema and internals).
- [Getting the Game Files](GETTING_THE_GAME_FILES.md) — obtaining a licensed
  copy of the game data in the first place.
