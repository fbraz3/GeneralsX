# GeneralsX WebAssembly Build Instructions

The WebAssembly target is experimental and currently builds Zero Hour only.
It uses Emscripten, SDL3, MiniAudio, and the d8web Direct3D 8 to WebGL2
translation layer.

## Prerequisites

- CMake 3.25 or newer
- Ninja
- Emscripten 6

On macOS:

```bash
brew install cmake ninja emscripten
```

## Configure and build

From the repository root:

```bash
emcmake cmake --preset emscripten-webgl2
cmake --build build/emscripten-webgl2 --target z_generals
```

The generated browser artifacts are:

```text
build/emscripten-webgl2/GeneralsMD/GeneralsXZH.js
build/emscripten-webgl2/GeneralsMD/GeneralsXZH.wasm
```

The build downloads a pinned revision of d8web automatically. It does not
require DXVK, Vulkan, vcpkg, or a sibling source checkout.

## Game data

Retail game assets are never stored in this repository or normal CI
artifacts. The launcher under `web/` consumes a schema-v2 manifest, verifies
downloads while streaming them into OPFS, and stages the authorized files at
the paths required by the engine. See
[`WEB_ASSET_PIPELINE.md`](../HOWTO/WEB_ASSET_PIPELINE.md).

## Current limitations

- Zero Hour is the only configured game target.
- `wasm/boot.html` is a development harness and requires manually staged,
  legally obtained game archives.
- The production launcher copies verified BIG archives from OPFS into MEMFS
  in 4 MiB chunks because the current engine filesystem calls are synchronous.
  Archives therefore remain subject to the wasm32 4 GiB memory ceiling.
- WebGL2 rendering and user-gesture WebAudio startup work in the launcher.
- Browser and opt-in native WebRTC transports are implemented, but a
  cross-platform release still requires real-match soak and determinism
  validation plus deployed signaling/TURN infrastructure.
