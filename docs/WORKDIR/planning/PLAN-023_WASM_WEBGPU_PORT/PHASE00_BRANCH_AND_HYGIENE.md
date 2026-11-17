# Phase 00: Feature Branch & Hygiene Rules

## Objective
Create an isolated feature branch (`feat/wasm-webgpu-port`) in the `GeneralsX` worktree and establish strict exclusion rules so that only clean engine, build, web-shell, and CI changes are ported from `wasm-generals`.

---

## 1. Repository Locations

| Role | Path |
| :--- | :--- |
| **Target Worktree (Edit Here)** | `/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis` |
| **Reference Worktree (Read-Only)** | `/Users/felipebraz/.gemini/antigravity/worktrees/wasm-generals/wasm_port_architecture_analysis` |

---

## 2. Create Feature Branch

Run from `/Users/felipebraz/.gemini/antigravity/worktrees/GeneralsX/wasm_port_architecture_analysis`:

```bash
git status -s
git checkout -b feat/wasm-webgpu-port
```

If the working tree already has uncommitted planning docs from `PLAN-023_WASM_WEBGPU_PORT`, keep or commit them cleanly on `feat/wasm-webgpu-port` before making C++/CMake edits.

---

## 3. Divergence Baseline

- `wasm-generals` forked from `GeneralsX` on **2026-08-03** at `Merge pull request #241 from fbraz3/thesuperhackers-sync-08-03-2026` (`b791e5ff2` in `GeneralsX`, `740e97ab4` in `wasm-generals`).
- Since the fork, `GeneralsX` has advanced **578+ commits**, including multiple `TheSuperHackers` upstream syncs that moved files out of `Generals/` and `GeneralsMD/` into `Core/`.
- Specifically, these files modified by `wasm-generals` moved in `GeneralsX` `HEAD`:
  - `Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp` -> `Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`
  - `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp` -> `Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`
  - `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Diplomacy.cpp` -> `Core/GameEngine/Source/GameClient/GUI/GUICallbacks/Diplomacy.cpp`
  - `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/InGameChat.cpp` -> `Core/GameEngine/Source/GameClient/GUI/GUICallbacks/InGameChat.cpp`
  - `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp` -> `Core/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp`
- Never copy whole `.cpp`/`.h` files from `wasm-generals` over existing `GeneralsX` files — always apply surgical edits to the current `GeneralsX` files. Only brand-new files (`WebGPUDevice/`, `NullAudioManager.h`, `LobbyTooltips.h/.cpp`, `cmake/webgpu.cmake`, `serve-webgpu.py`) are copied directly.

---

## 4. Strict Blacklist (Do NOT Port)

1. **MIT-PoU License & Agent Tracking Files** (commits `6120298c3..HEAD` in `wasm-generals`):
   - `AGENTS.md`
   - `CLAUDE.md`
   - `LICENSE.md`
   - `LICENSE-ENGINE-GPL-3.0.md`
   - `PROOF_OF_USAGE.md`
   - `proof-of-usage.json`
   - `web/LICENSE.md`
   - `web/api/proof-of-usage.mjs`
   - `web/public/.well-known/proof-of-usage.json`
   - `.github/workflows/validate-proof-of-usage.yml`
2. **Prebuilt Binaries & Local Caches**:
   - `web/dist/GeneralsXZH.*`
   - `web/public/GeneralsXZH.*`
   - `GeneralsXZH.dxvk-cache`
3. **Private Monorepo Web Shell (`wasm-generals` `HEAD` `web/`)**:
   - Commits `b2ea6f411` and `9c4a34afa` replaced `web/src/streaming.ts` and `web/src/ui.ts` with unpublished `@wasm/runtime`, `@wasm/shell`, and `@origami-ltd/ui` workspace packages. Extract `web/` from commit `25dcc3eab` instead (see [PHASE06](./PHASE06_SELF_CONTAINED_WEB_AND_UDP_BRIDGE.md)).
4. **Benchmark / Visual-Regression CLI Pollution**:
   - Omit `-botmatch`, `-pauseframe`, `-savestateframe`, `-loadstate`, `-captureframe*`, `-replayspeed`, `-replaycheckpointinterval`, `-maxcameraheight`, `-mincameraheight` in `CommandLine.h/.cpp`, `HeightMap.cpp`, `W3DTreeBuffer.h/.cpp`, `LookAtXlat.cpp`, `GameStateMap.cpp`, `InGameUI.cpp`, `W3DDisplay.cpp`, `GameEngine.h/.cpp`, and `SDL3GameEngine.cpp`.
5. **Credits Override**:
   - Do not port `Core/GameEngine/Source/GameClient/Credits.cpp`.
6. **Existing NGMP Guards**:
   - Preserve every `#if defined(SAGE_USE_NGMP)` / `#ifdef SAGE_USE_NGMP` block across `Core/`, `Generals/`, and `GeneralsMD/`.

---

## Checklist

- [x] Verify clean starting working tree in `GeneralsX`.
- [x] Create and switch to branch `feat/wasm-webgpu-port`.
- [x] Confirm read access to `/Users/felipebraz/.gemini/antigravity/worktrees/wasm-generals/wasm_port_architecture_analysis`.
