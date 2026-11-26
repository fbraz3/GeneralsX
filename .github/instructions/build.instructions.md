---
applyTo: 'cmake/**,CMakeLists.txt,CMakePresets.json'
---

## Build Presets

### Cross-Platform Active Targets
- **`win64`** — MSVC x64 (`RelWithDebInfo`), SDL3 + MiniAudio + FFmpeg + NGMP + DXVK 2.6 x64 / static `d3d8to9` fallback — **PRIMARY WINDOWS**
- **`win64-debug`** — MSVC x64 (`Debug`) variant
- **`linux64-deploy`** — GCC/Clang x86_64, Release — **PRIMARY LINUX**
- **`linux64-testing`** — Linux debug variant
- **`macos-vulkan`** — macOS ARM64, RelWithDebInfo — **PRIMARY MACOS**
- **`webgpu`** — Emscripten 32-bit WebAssembly + WebGPU (`z_generals` / `GeneralsXZH`, exception to DXVK — uses `WebGPUDevice` + `web/` frontend)
- **`mingw-w64-i686`** — exploratory MinGW-w64 cross-compile for Windows

### Legacy / Superseded
- **`vc6`** — Visual Studio 6 (C++98), 32-bit, DirectX 8 + Miles (replaced by `win64`)
- **`win32`** — MSVC 2022 32-bit, experimental upstream path (superseded by `win64`)

## Build Workflow

```bash
# Windows x64 (PowerShell + VS 2022 Build Tools + vcpkg)
./scripts/build/windows/build-windows-zh.ps1
./scripts/build/windows/build-windows-generals.ps1

# Linux (Docker)
./scripts/build/linux/docker-configure-linux.sh linux64-deploy
./scripts/build/linux/docker-build-linux-zh.sh linux64-deploy

# Linux (native)
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals

# macOS
cmake --preset macos-vulkan
cmake --build build/macos-vulkan --target z_generals

# WebAssembly + WebGPU (Emscripten)
cmake --preset webgpu
cmake --build build/webgpu --target z_generals

# Windows cross-build (exploratory)
cmake --preset mingw-w64-i686
cmake --build build/mingw-w64-i686 --target z_generals
```

## DXVK Source of Truth (macOS)

- DXVK fixes must live in `generalsx-dxvk` and be pushed to the fork branch `generalsx-macos-v2.6`.
- macOS build tracks that branch via CMake FetchContent (`UPDATE_DISCONNECTED FALSE`).
- Local mode: `-DSAGE_DXVK_USE_LOCAL_FORK=ON` (disables update/fetch).

**Rules**:
1. Never patch DXVK files inside `build/_deps/...`.
2. Do not rely on transient patch scripts.
3. Keep Linux/macOS aligned on DXVK 2.6 branch unless explicitly changed.
4. Validate fix locally → commit/push to fork → CMake consumes from tracked branch.

## Testing Strategy

1. Per-platform smoke tests: launch game, reach main menu, load skirmish map.
2. Replay compatibility: VC6 optimized builds with `RTS_BUILD_OPTION_DEBUG=OFF`.
3. Cross-platform validation: same replays valid across platforms.
