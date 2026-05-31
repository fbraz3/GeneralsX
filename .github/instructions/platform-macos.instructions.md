---
applyTo: 'scripts/build/macos/**,references/fbraz3-dxvk/**'
---

## macOS Architecture

- SDL3 for windowing/input.
- DXVK + MoltenVK: DX8 → Vulkan → Metal chain.
- OpenAL for audio.
- Target: **ARM64 (Apple Silicon)**, macOS 15.0+.
- Universal binary (arm64 + x86_64) planned.

## Key Considerations

- DXVK is built via Meson as ExternalProject — must pass `-arch arm64` via `cmake/meson-arm64-native.ini` to avoid Rosetta2 confusion.
- DXVK source of truth: fork branch `generalsx-macos-v2.6`; CMake tracks remote by default.
- Local fork mode: `-DSAGE_DXVK_USE_LOCAL_FORK=ON` (disables update/fetch, uses `references/fbraz3-dxvk`).
- Vulkan SDK **must** be from LunarG — provides MoltenVK ICD JSON. Not from Homebrew.
- Vulkan SDK path: `~/VulkanSDK/<version>/macOS/` — must contain `libvulkan.dylib` and `libMoltenVK.dylib`.

## Build Workflow

```bash
# Prerequisites (once)
brew install cmake ninja meson
# + LunarG Vulkan SDK: https://vulkan.lunarg.com/sdk/home#mac

./scripts/build/macos/build-macos-zh.sh      # configure + build
./scripts/build/macos/deploy-macos-zh.sh     # copy to ~/GeneralsX/GeneralsMD/
./scripts/build/macos/run-macos-zh.sh -win   # launch windowed
```

## macOS-Specific Notes

- **Rosetta2 + Meson**: always use `cmake/meson-arm64-native.ini` to force `-arch arm64`.
- **SDL3**: fetched via CMake FetchContent — no system package needed.
- **No Cocoa/Metal calls in game code**: all platform access through SDL3 + DXVK layers.
- **DXVK fixes**: commit/push to `references/fbraz3-dxvk` first; never edit `build/_deps/...`.
