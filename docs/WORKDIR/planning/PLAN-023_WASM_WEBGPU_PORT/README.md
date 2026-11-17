# PLAN-023: WebAssembly + WebGPU Unification Port (`GeneralsX` Single Codebase)

**Status**: Planned (Ready for execution on `feat/wasm-webgpu-port`)  
**Target Repository**: `GeneralsX` (`/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis`)  
**Reference Fork**: `wasm-generals` (`/Users/felipebraz/.gemini/antigravity/worktrees/wasm-generals/wasm_port_architecture_analysis`, forked from `GeneralsX` at `b791e5ff2` / `740e97ab4` on `2026-08-03`)  
**Primary Game Target**: `GeneralsXZH` (`z_generals`)

---

## Executive Summary & Architectural Decision

Following an architectural comparison between maintaining a separate downstream WebAssembly fork vs. unifying the Emscripten + WebGPU target into the main `GeneralsX` repository, **Option 1 (Single Unified Codebase)** was selected using a **surgical port** (clean integration rather than raw `git merge`):

1. **Single C++ Codebase Across All Platforms**: `macOS`, `Linux`, `Windows`, and `WebAssembly (WebGPU)` build from one tree, preventing two-level merge drift (`TheSuperHackers -> GeneralsX -> wasm-generals`) and ensuring bit-identical `WWMath` / `GameLogic` determinism across desktop and browser targets.
2. **First-Release Multiplayer Scope**: Full Singleplayer, Skirmish, Replays, and **Cross-Play LAN Multiplayer (`Browser WASM <-> Native Desktop macOS/Linux/Windows`)** through a real UDP-to-WebSocket bridge in `serve-webgpu.py` paired with `StdLANInterface` Emscripten support.
3. **Zero GameSpy, NGMP-Ready**: Legacy GameSpy (`SAGE_USE_GAMESPY`) is disabled for browser builds with zero interest in reviving GameSpy online services. Meanwhile, `cmake/ngmp.cmake` is aligned early (`if(NOT EMSCRIPTEN)` around native libcurl/GameNetworkingSockets) so future NGMP (`GeneralsOnline`) WebAssembly integration (`fetch` + WebSocket + WebRTC DataChannels) can build cleanly on top of this foundation.
4. **CI Verification & Automated Web Release Sync**: A dedicated `.github/workflows/build-wasm.yml` workflow verifies the `webgpu` target in CI on every relevant push/PR, and `.github/workflows/release.yml` prepares Stage 2 automated release packaging and web deployment synchronization.

---

## Strict Hygiene & Exclusion Rules (Blacklist)

Executors **MUST NOT** copy or merge any of the following items from `wasm-generals`:

- **MIT-PoU License / Agent Tracking Files**:
  - `AGENTS.md`, `CLAUDE.md`, `LICENSE.md`, `web/LICENSE.md`, `PROOF_OF_USAGE.md`, `proof-of-usage.json`, `web/api/proof-of-usage.mjs`, `.github/workflows/validate-proof-of-usage.yml`.
- **Prebuilt Binaries & Caches**:
  - `web/dist/GeneralsXZH.*`, `web/public/GeneralsXZH.*`, `GeneralsXZH.dxvk-cache`.
- **Benchmark / Regression CLI Pollution**:
  - `-botmatch`, `-pauseframe`, `-savestateframe`, `-loadstate`, `-captureframe*`, `-replayspeed`, `-replaycheckpointinterval`, `-maxcameraheight`, `-mincameraheight` in `CommandLine.cpp`, `HeightMap.cpp`, `W3DTreeBuffer.cpp`, `LookAtXlat.cpp`, `GameStateMap.cpp`, `InGameUI.cpp`, `W3DDisplay.cpp`, or `GameEngine.cpp`.
- **Fork-Specific Credits**:
  - `Core/GameEngine/Source/GameClient/Credits.cpp` changes.
- **Private Monorepo Web Shell**:
  - Do **NOT** copy `web/` from `wasm-generals` `HEAD` (it depends on unpublished `@wasm/runtime`, `@wasm/shell`, `@origami-ltd/ui` packages). Extract `web/` from commit `25dcc3eab` (self-contained TypeScript + Vite shell) plus the Safari `File` fix (`407c6469d`).
- **Existing NGMP Code**:
  - Never delete or break `#if defined(SAGE_USE_NGMP)` blocks anywhere in `GeneralsX`.

---

## Phase Index & Progress Checklist

| Phase | Document | Scope | Status |
| :--- | :--- | :--- | :---: |
| **Phase 00** | [PHASE00_BRANCH_AND_HYGIENE.md](./PHASE00_BRANCH_AND_HYGIENE.md) | Feature branch setup (`feat/wasm-webgpu-port`) and strict exclusion rules | `[x]` |
| **Phase 01** | [PHASE01_BUILD_SYSTEM_AND_NGMP_PREP.md](./PHASE01_BUILD_SYSTEM_AND_NGMP_PREP.md) | `vcpkg.json`, `cmake/webgpu.cmake`, `cmake/ngmp.cmake` prep, CMake modules, `CMakePresets.json` | `[x]` |
| **Phase 02** | [PHASE02_PLATFORM_COMPAT_AND_FONTS.md](./PHASE02_PLATFORM_COMPAT_AND_FONTS.md) | 32-bit Emscripten types (`WWLib`, `CompatLib`), `WW3D2` FreeType without Fontconfig, audio stubs | `[x]` |
| **Phase 03** | [PHASE03_WEBGPU_D3D8_BACKEND.md](./PHASE03_WEBGPU_D3D8_BACKEND.md) | `WebGPUDevice` (`WebGPUContext` + `WebGPUD3D8`), `NullAudioManager`, `dx8wrapper.cpp` hook | `[x]` |
| **Phase 04** | [PHASE04_ASYNC_LOOP_SDL3_AND_AUDIO.md](./PHASE04_ASYNC_LOOP_SDL3_AND_AUDIO.md) | `GameEngine::executeFrame()`, `SDL3GameEngine::browserFrame()`, `ASYNCIFY` loading, threadless `MiniAudio` | `[x]` |
| **Phase 05** | [PHASE05_LAN_CROSSPLAY_AND_GAMESPY_REMOVAL.md](./PHASE05_LAN_CROSSPLAY_AND_GAMESPY_REMOVAL.md) | Browser ↔ Desktop LAN cross-play (`udp.cpp`, `StdLANInterface`, `LANAPIhandlers`), `SAGE_USE_GAMESPY=OFF` guards | `[x]` |
| **Phase 06** | [PHASE06_SELF_CONTAINED_WEB_AND_UDP_BRIDGE.md](./PHASE06_SELF_CONTAINED_WEB_AND_UDP_BRIDGE.md) | Self-contained `web/` app (`25dcc3eab` + Safari `File` fix), real POSIX UDP bridge in `serve-webgpu.py` | `[x]` |
| **Phase 07** | [PHASE07_LOCAL_VERIFICATION_AND_LAN_SMOKE.md](./PHASE07_LOCAL_VERIFICATION_AND_LAN_SMOKE.md) | Local WASM build, Vite build, macOS desktop non-regression build, Browser ↔ Desktop LAN smoke test | `[x]` |
| **Phase 08** | [PHASE08_CI_WASM_VERIFICATION.md](./PHASE08_CI_WASM_VERIFICATION.md) | `.github/workflows/build-wasm.yml` and integration into `.github/workflows/ci.yml` | `[x]` |
| **Phase 09** | [PHASE09_WEB_RELEASE_SYNC_PIPELINE.md](./PHASE09_WEB_RELEASE_SYNC_PIPELINE.md) | Stage 2 `.github/workflows/release.yml` artifact attachment & `nginx:alpine` GHCR container publish on release | `[x]` |

---

## Stop & Re-Plan Triggers

Stop execution and report back if any of the following conditions occur:
- `emscripten` (`emcc`) or `vcpkg` is missing or broken in the local build environment during Phase 07.
- Upstream-unified files in `Core/` introduce GameSpy or platform dependencies not listed in Phases 02–05 that require an architectural choice involving `SAGE_USE_NGMP`.
- Local Emscripten's `emdawnwebgpu` headers (`webgpu/webgpu.h`) have breaking API signature changes relative to Emscripten `6.0.2` used in `WebGPUContext.cpp` / `WebGPUD3D8.cpp`.
