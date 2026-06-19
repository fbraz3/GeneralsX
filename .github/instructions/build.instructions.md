---
applyTo: 'cmake/**,CMakeLists.txt,CMakePresets.json'
---

## Build Presets

### Legacy (Maintenance Only)
- **`vc6`** — Visual Studio 6 (C++98), 32-bit, DirectX 8 + Miles
- **`win32`** — MSVC 2022 (C++20), experimental upstream path

### Cross-Platform (SDL3 + DXVK + MiniAudio) — Active Targets
- **`linux64-deploy`** — GCC/Clang x86_64, Release — **PRIMARY LINUX**
- **`linux64-testing`** — Linux debug variant
- **`macos-vulkan`** — macOS ARM64, RelWithDebInfo — **PRIMARY MACOS**
- **`mingw-w64-i686`** — exploratory MinGW-w64 cross-compile for Windows
- **`windows64-deploy`** — planned MinGW-w64 x86_64 (issue #29, not active)

## Build Workflow

```bash
# Linux (Docker)
./scripts/build/linux/docker-configure-linux.sh linux64-deploy
./scripts/build/linux/docker-build-linux-zh.sh linux64-deploy

# Linux (native)
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals

# macOS
cmake --preset macos-vulkan
cmake --build build/macos-vulkan --target z_generals

# Windows cross-build (exploratory)
cmake --preset mingw-w64-i686
cmake --build build/mingw-w64-i686 --target z_generals
```

## DXVK Source of Truth (macOS)

- DXVK fixes must live in `references/fbraz3-dxvk` and be pushed to the fork branch `generalsx-macos-v2.6`.
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
