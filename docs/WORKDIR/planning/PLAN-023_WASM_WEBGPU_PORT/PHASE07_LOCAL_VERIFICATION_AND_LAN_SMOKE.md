# Phase 07: Local Build Verification & Browser ↔ Desktop LAN Smoke Test

## Objective
Verify that the unified branch compiles cleanly for both WebAssembly (`webgpu`) and native Desktop (`macos-vulkan`), that the self-contained Vite frontend builds cleanly, and that a Browser WASM client can discover, join, and play a LAN match against a native Desktop `GeneralsXZH` client.

---

## 1. Verify Toolchain Prerequisites

Check that Emscripten, `vcpkg`, `cmake`, `ninja`, and `node` are available locally:

```bash
which emcc cmake ninja node npm
echo "EMSCRIPTEN_ROOT=${EMSCRIPTEN_ROOT:-$(brew --prefix emscripten 2>/dev/null)/libexec}"
echo "VCPKG_ROOT=${VCPKG_ROOT:-$HOME/vcpkg}"
```

*Stop and report back if `emcc` or `vcpkg` is missing.*

---

## 2. Build WebAssembly Engine (`webgpu` Preset)

```bash
export EMSCRIPTEN_ROOT="${EMSCRIPTEN_ROOT:-$(brew --prefix emscripten)/libexec}"
export EM_LLVM_ROOT="${EMSCRIPTEN_ROOT}/llvm/bin"
export EM_BINARYEN_ROOT="${EMSCRIPTEN_ROOT}/binaryen"
export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"

cmake --preset webgpu
cmake --build build/webgpu --target z_generals
```

**Expected Artifacts**:
- `build/webgpu/GeneralsMD/GeneralsXZH.js`
- `build/webgpu/GeneralsMD/GeneralsXZH.wasm`
- `build/webgpu/GeneralsMD/GeneralsXZH.data` (containing bundled `/fonts`)
- `build/webgpu/generalsx.big`

---

## 3. Build Self-Contained Web Frontend (`web/`)

```bash
cd web
npm install
npm run build
cd ..
```

**Expected Output**:
- `web/dist/index.html` and `web/dist/assets/*` build with exit code `0` and zero references to `@wasm/runtime`, `@wasm/shell`, or `@origami-ltd/ui`.

---

## 4. Verify Native Desktop Non-Regression (`macos-vulkan`)

Ensure all `#if defined(__EMSCRIPTEN__)`, `#if defined(SAGE_USE_WEBGPU)`, and `#if defined(SAGE_USE_GAMESPY)` edits preserved native Desktop compilation and `SAGE_USE_NGMP=ON`:

```bash
cmake --preset macos-vulkan
cmake --build build/macos-vulkan --target z_generals
```

**Expected Output**:
- `build/macos-vulkan/GeneralsMD/GeneralsXZH` builds cleanly without errors.

---

## 5. Browser WASM ↔ Desktop Native LAN Cross-Play Smoke Test

1. Stage the WASM artifacts alongside `web/dist/` (e.g., into a temporary staging folder or `web/dist/`):
   ```bash
   cp build/webgpu/GeneralsMD/GeneralsXZH.js \
      build/webgpu/GeneralsMD/GeneralsXZH.wasm \
      build/webgpu/GeneralsMD/GeneralsXZH.data \
      build/webgpu/generalsx.big \
      web/dist/
   ```
2. **Two-Machine Test (Recommended)**:
   - Machine A (macOS/Linux/Windows): launch native `GeneralsXZH`, go to **Multiplayer -> Network**.
   - Machine B: run `./scripts/qa/smoke/serve-webgpu.py web/dist --bind 0.0.0.0 --port 8765`, open Chrome with WebGPU (`https://<machine-b-ip>:8765` with TLS or `http://localhost:8765` if browser is on Machine B), select your local Generals/Zero Hour install folder, and go to **Multiplayer -> Network**.
3. **Single-Machine Test (Alias IP on macOS)**:
   - Note that binding both native `GeneralsXZH` and `serve-webgpu.py` to `0.0.0.0:8086` on the same OS kernel requires `SO_REUSEPORT` (enabled in `serve-webgpu.py`) plus assigning a distinct local alias IP (e.g. `sudo ifconfig en0 alias 192.168.X.250 netmask 255.255.255.0 up`) and passing `--lan-ip 192.168.X.250` to `serve-webgpu.py` so the two instances have distinct IPv4 identities in `LANAPI`.
4. **Acceptance Criteria**:
   - Both Browser and Desktop see each other in the LAN Lobby player list (`port 8086`).
   - Either side can host a game, and the other joins, selects faction/color, clicks Accept, and starts the match (`port 8088`).
   - Match runs without `SyncCrash` / CRC mismatch (verifying `GameMath` + `WWMath` parity across `wasm32` and `arm64`/`x86_64`).

---

## Checklist

- [x] `cmake --preset webgpu && cmake --build build/webgpu --target z_generals` passes.
- [x] `cd web && npm install && npm run build` passes.
- [x] `cmake --preset macos-vulkan && cmake --build build/macos-vulkan --target z_generals` and `--target g_generals` pass.
- [ ] Singleplayer Skirmish boots in Chrome with WebGPU + fonts + MiniAudio. *(HTTP/COOP/COEP + `serve-webgpu.py` smoke test passed; interactive Chrome WebGPU boot ready for manual run.)*
- [ ] Browser WASM ↔ Desktop Native LAN lobby discovery and match start succeed without CRC desync. *(Ready for manual two-machine or alias-IP LAN test.)*
