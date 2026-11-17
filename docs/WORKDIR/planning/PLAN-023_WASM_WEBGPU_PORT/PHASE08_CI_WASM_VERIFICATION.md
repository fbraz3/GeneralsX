# Phase 08: CI Verification for the WebAssembly Target

## Objective
Add a dedicated GitHub Actions reusable workflow (`.github/workflows/build-wasm.yml`) and wire it into `.github/workflows/ci.yml` so every PR/push touching the engine, CMake, or `web/` automatically compiles and verifies the `webgpu` WASM binary and Vite frontend.

---

## 1. Create `.github/workflows/build-wasm.yml`

Design specifications matching existing `GeneralsX` workflows (`actions/checkout@v6`, `lukka/run-vcpkg@v11` pinned to `ffc071e0c08432c60c9b64f00334c0227667931b`, `actions/cache@v4`/`@v5`, `actions/upload-artifact@v7`):

- **Triggers**: `workflow_call` (with optional `checkout_ref` input for `release.yml` reuse) and `workflow_dispatch`.
- **Runner**: `ubuntu-latest`, `timeout-minutes: 45`.
- **Steps**:
  1. **Checkout Code**: `actions/checkout@v6` (`ref: ${{ inputs.checkout_ref || github.ref }}`, `fetch-depth: 0`, `fetch-tags: true`).
  2. **Populate Git Info for Version Headers**: export `GENERALS_GIT_OVERRIDE_TSTAMP` and `GENERALS_GIT_OVERRIDE_TAG` (matching `build-linux-flatpak.yml`).
  3. **Install Build Tools**: `ninja-build`, `pkg-config`, `python3`, `ccache`, `zip`.
  4. **Setup Emscripten SDK**: `mymindstorm/setup-emsdk@v14` with `version: "6.0.2"` (matches tested `emdawnwebgpu` API; enable `actions-cache-folder` caching so emsdk download/activation is fast).
     - Export `EMSCRIPTEN_ROOT="${EMSDK}/upstream/emscripten"`, `EM_LLVM_ROOT="${EMSDK}/upstream/bin"`, `EM_BINARYEN_ROOT="${EMSDK}/upstream/bin"`.
  5. **Setup vcpkg + Binary Cache**:
     - `lukka/run-vcpkg@v11` (`vcpkgGitCommitId: 'ffc071e0c08432c60c9b64f00334c0227667931b'`, `runVcpkgInstall: false`, `doNotCache: true`).
     - Restore/save `actions/cache@v4` for `${{ github.workspace }}/vcpkg-bincache` keyed on `vcpkg-bincache-v1-Linux-wasm32-emscripten-${{ hashFiles('vcpkg.json', 'vcpkg-lock.json', 'cmake/ports/**', 'cmake/triplets/**', '.github/workflows/build-wasm.yml') }}`.
     - Configure `VCPKG_BINARY_SOURCES="clear;files,$cacheDir,readwrite"`.
  6. **Cache CMake `_deps` & `ccache`**:
     - `actions/cache@v5` for `build/webgpu/_deps` (DXVK headers, SDL3, SDL3_image, MiniAudio, GameMath, GameSpy headers).
     - `hendrikmuhs/ccache-action@v1.2` (`key: ccache-wasm-webgpu-${{ github.ref_name }}`).
  7. **Configure & Build WASM Engine**:
     ```bash
     cmake --preset webgpu \
       -DCMAKE_C_COMPILER_LAUNCHER=ccache \
       -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
     cmake --build build/webgpu --target z_generals
     ```
  8. **Verify Generated WASM Outputs**:
     - Assert non-empty existence of `build/webgpu/GeneralsMD/GeneralsXZH.js`, `GeneralsXZH.wasm`, `GeneralsXZH.data`, and `build/webgpu/generalsx.big`.
  9. **Setup Node.js & Build `web/`**:
     - `actions/setup-node@v6` (`node-version: "22"`, `cache: "npm"`, `cache-dependency-path: "web/package-lock.json"` if present).
     - Run `cd web && npm install && npm run build` (and `npx tsc --noEmit` to typecheck TypeScript strictly).
  10. **Package Bundle Artifact**:
      - Copy `web/dist/*`, `build/webgpu/GeneralsMD/GeneralsXZH.{js,wasm,data}`, `build/webgpu/generalsx.big`, and `scripts/qa/smoke/serve-webgpu.py` into `dist-wasm-bundle/`.
      - Zip into `wasm-webgpu-GeneralsXZH.zip` and upload via `actions/upload-artifact@v7` (`name: wasm-webgpu-GeneralsXZH.zip`).

---

## 2. Wire `build-wasm` into `.github/workflows/ci.yml`

Edit `.github/workflows/ci.yml`:

1. **`detect-changes` job**:
   - Add a `wasm-should-build` filter output covering:
     - `GeneralsMD/**`
     - `Core/**`
     - `Dependencies/**`
     - `assets/fonts/**`
     - `cmake/**`
     - `CMakeLists.txt`
     - `CMakePresets.json`
     - `vcpkg.json`
     - `web/**`
     - `.github/workflows/build-wasm.yml`
     - `.github/workflows/ci.yml`
   - Export `wasm-should-build: ${{ steps.filter.outputs.wasm-should-build }}` and add a row to the `Build Trigger Summary` table.
2. **`build-wasm` job**:
   ```yaml
   build-wasm:
     name: Build WebAssembly (WebGPU)
     needs: detect-changes
     if: needs.detect-changes.outputs.wasm-should-build == 'true' || github.event_name == 'workflow_dispatch'
     uses: ./.github/workflows/build-wasm.yml
     secrets: inherit
   ```
3. **`ci-summary` job**:
   - Add `build-wasm` to `needs: [...]`.
   - Add `| WebAssembly (WebGPU) | ... |` to the summary markdown table.
   - Add `needs.build-wasm.result == 'failure'` to the `Fail if Any Build or Replay Test Failed` condition.

---

## Checklist

- [x] Create `.github/workflows/build-wasm.yml` with emsdk `6.0.2`, vcpkg binary cache, CMake `webgpu` build, `tsc --noEmit` + Vite build, and artifact upload.
- [x] Add `wasm-should-build` filter, `build-wasm` job, and `ci-summary` status gate to `.github/workflows/ci.yml`.
- [ ] Verify CI workflow syntax and run `build-wasm` via GitHub Actions on the `feat/wasm-webgpu-port` PR. *(Local `npx tsc --noEmit` passed; GitHub Actions run pending push/PR.)*
