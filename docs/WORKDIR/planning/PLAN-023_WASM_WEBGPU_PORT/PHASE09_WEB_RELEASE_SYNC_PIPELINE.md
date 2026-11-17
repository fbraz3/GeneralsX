# Phase 09: Web Release Synchronization Pipeline (Stage 2)

## Objective
Keep the published web build automatically synchronized with every official `GeneralsX` release tag by:
1. Attaching `Web-WebGPU-GeneralsXZH.zip` to the GitHub Release (including **Draft** releases) during `.github/workflows/release.yml`.
2. Triggering `.github/workflows/publish-web-container.yml` **only when a release is actually published** (`on: release: types: [published]`, never on drafts) to build and push a multi-arch `nginx:alpine` container image to `ghcr.io/generalsx-project/generalsx-web`.

> **Note**: This phase is designed as Stage 2 (following Phases 00–08 on `feat/wasm-webgpu-port`).

---

## 1. Stage 1 — Attach `Web-WebGPU-GeneralsXZH.zip` in `.github/workflows/release.yml`

1. Add a `build-wasm` job alongside `build-linux-flatpak`, `build-macos`, and `build-windows`:
   ```yaml
   build-wasm:
     needs: [create-tag]
     uses: ./.github/workflows/build-wasm.yml
     with:
       checkout_ref: ${{ inputs.release_mode == 'Dry Run' && github.sha || format('refs/tags/{0}', needs.create-tag.outputs.release_version) }}
     secrets: inherit
   ```
   Because `build-wasm` runs **after** `create-tag`, `GeneratedVersion.h` / `git_watcher.cmake` embeds the exact release tag into `GeneralsXZH.wasm` (matching desktop builds).
2. In `prepare-release`:
   - Add `build-wasm` to `needs: [...]`.
   - Download `wasm-webgpu-GeneralsXZH.zip` into `artifacts/wasm`.
   - Normalize the asset filename to `release-assets/Web-WebGPU-GeneralsXZH.zip` and attach it in `gh release create "$VERSION" ...`.
   - Update `cleanup-tag-on-failure` to also roll back the tag if `needs.build-wasm.result` is `failure` or `cancelled`.
   - Include `WebAssembly (WebGPU) asset: Web-WebGPU-GeneralsXZH.zip` in the workflow summary.

---

## 2. Container Runtime (`web/Dockerfile` & `web/nginx.conf`)

`SharedArrayBuffer` (used by `web/src/streaming.ts` for synchronous `.big` reads) and `WebGPU` require a **cross-origin isolated** context with these exact HTTP headers on all responses:

```text
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

- `web/nginx.conf` configures `nginx:1.27-alpine` on port `80` with:
  - `COOP: same-origin` and `COEP: require-corp` (`always`) on `/`, `/assets/`, `\.(wasm|data|big)$`, and `/healthz`.
  - `application/wasm` MIME type and `gzip` compression.
  - `Cache-Control: public, max-age=31536000, immutable` for `.wasm`, `.data`, `.big`, and Vite-hashed `/assets/*`, with `no-cache` on `index.html`.
- `web/Dockerfile` packages the extracted `Web-WebGPU-GeneralsXZH.zip` payload (`ARG BUNDLE_DIR=dist-wasm-bundle`, stripping `serve-webgpu.py`) with a `/healthz` `HEALTHCHECK`.

---

## 3. Stage 2 — `.github/workflows/publish-web-container.yml` (Draft → Published Trigger)

- **Trigger**:
  ```yaml
  on:
    release:
      types: [published]
    workflow_dispatch:
      inputs:
        tag: ...
  ```
  - **Draft releases do NOT trigger this workflow.** You can run `release.yml` in `Draft` mode, validate all desktop and WASM artifacts, and only trigger container publication when clicking **Publish release** in GitHub.
- **No WASM recompilation**: Downloads `Web-WebGPU-GeneralsXZH.zip` directly from the published release (`gh release download`), extracts it into `dist-wasm-bundle/`, and builds/pushes `linux/amd64,linux/arm64` images via Docker Buildx to:
  - `ghcr.io/generalsx-project/generalsx-web:<tag>`
  - `ghcr.io/generalsx-project/generalsx-web:latest` (when `prerelease != true`)
- **Authentication**: Uses `secrets.GHCR_TOKEN || secrets.GITHUB_TOKEN` with `packages: write`. (If the repository is outside the `generalsx-project` org or org-level package permissions require it, set `GHCR_TOKEN` as a repository secret with `write:packages` scope.)

---

## 4. Future Stage 3 Outlook

1. **Automated Server Deploy**: Extend `publish-web-container.yml` (or trigger a follow-up CD webhook / SSH / Watchtower roll-out) to pull and restart `ghcr.io/generalsx-project/generalsx-web:<tag>` on the production host once pushed.
2. **NGMP Online in WebAssembly**: Once `SAGE_USE_NGMP` is extended to Emscripten in a follow-up plan:
   - Rest API + JWT auth (`OnlineServices_Auth.cpp`) -> `emscripten_fetch`
   - Lobby & Presence WebSocket (`NGMPWebSocket.h/.cpp`) -> browser `Emscripten` WebSocket API (`/ws`)
   - Match P2P (`NetworkMesh.cpp` / `NextGenTransport.cpp`) -> browser `RTCPeerConnection` WebRTC DataChannels signaling over existing NGMP WebSocket (`EWebSocketMessageID::NETWORK_SIGNAL`) against desktop `libdatachannel` / `GameNetworkingSockets` ICE + Cloudflare Calls TURN.

---

## Checklist

- [x] Wire `build-wasm` into `.github/workflows/release.yml` (`prepare-release` and `cleanup-tag-on-failure`).
- [x] Attach `Web-WebGPU-GeneralsXZH.zip` to GitHub Releases.
- [x] Add `web/Dockerfile` and `web/nginx.conf` (`nginx:1.27-alpine` with `COOP: same-origin`, `COEP: require-corp`, `application/wasm`, `/healthz`).
- [x] Add `.github/workflows/publish-web-container.yml` triggered on `release: types: [published]` publishing `ghcr.io/generalsx-project/generalsx-web`.
