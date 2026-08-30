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

Retail game assets are never stored in this repository or its CI artifacts.
The browser packaging and authorized asset-delivery pipeline are still under
development. Building the engine alone does not provide game data.

## Current limitations

- Zero Hour is the only configured game target.
- The browser launcher and persistent asset cache are not yet included.
- Rendering support is incomplete and remains experimental.
- Browser audio and multiplayer are not yet release-ready.
