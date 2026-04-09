# GeneralsX - macOS Build Instructions (Apple Silicon)

## Prerequisites

### System Requirements

- **macOS 15 (Sequoia) or later** on Apple Silicon (M1/M2/M3/M4)
- **Xcode Command Line Tools** 14+
- ~10 GB free disk space (build artifacts + DXVK Meson build)

### 1. Xcode Command Line Tools

```bash
xcode-select --install
```

### 2. Homebrew

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. Build Tools

```bash
brew install cmake ninja meson python3 pkgconf ffmpeg glm
```

> **Note on `meson`**: The DXVK sub-project requires Meson >= 1.0. The Homebrew arm64
> bottle is sufficient. CMake overrides the build arches via `CFLAGS/CXXFLAGS=-arch arm64`.

### 4. Vulkan SDK (REQUIRED — NOT from Homebrew)

Download the **macOS Vulkan SDK** from LunarG. **Do not use the Homebrew `vulkan-headers` package**
— it lacks the MoltenVK ICD JSON that routes Vulkan calls to Metal.

1. Go to <https://vulkan.lunarg.com/sdk/home#mac>
2. Download the latest SDK installer (`.dmg`)
3. Run the installer — it installs to `~/VulkanSDK/<version>/macOS/`

After installation, verify:

```bash
ls ~/VulkanSDK/*/macOS/lib/libvulkan.dylib   # should list one file
ls ~/VulkanSDK/*/macOS/lib/libMoltenVK.dylib # should list one file
```

### 5. Game Files

Copy your retail Command & Conquer: Generals Zero Hour installation to:

```
~/GeneralsX/GeneralsZH/
```

Legacy fallback during migration is still supported:

```
~/GeneralsX/GeneralsMD/
```

Required files from the retail install:

- `generalszh.big`, `W3DZH.big`, `MapsZH.big` (and other `.big` archives)
- `AudioZH.big` (even though audio is not yet functional)

---

## Building

### Clone the Repository

```bash
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
```

### Configure and Build

```bash
./scripts/build-macos-zh.sh
```

This does:

1. Checks all prerequisites (cmake, ninja, meson, Vulkan SDK)
2. Runs `cmake --preset macos-vulkan` (fetches pinned DXVK fork commit and builds via Meson)
3. Builds `z_generals` target (Zero Hour executable)
4. Prints the binary path on success

**First run takes 5-10 minutes** because DXVK is fetched from git and compiled
via Meson. Subsequent builds reuse the Meson cache and finish in under a minute.

> **`--build-only` flag**: If you have already configured (cmake cache exists),
> skip configuration:
> ```bash
> ./scripts/build-macos-zh.sh --build-only
> ```

### Manual cmake commands (equivalent)

```bash
cmake --preset macos-vulkan
cmake --build build/macos-vulkan --target z_generals -j$(sysctl -n hw.logicalcpu)
```

---

## Deploying

After a successful build, deploy the binary and Vulkan runtime to the game directory:

```bash
./scripts/deploy-macos-zh.sh
```

This script:

- Copies `build/macos-vulkan/GeneralsMD/GeneralsXZH` to `~/GeneralsX/GeneralsZH/` (or `~/GeneralsX/GeneralsMD/` when legacy assets are detected)
- Detects the Vulkan SDK in `~/VulkanSDK/` and copies:
  - `libvulkan.dylib`, `libvulkan.1.dylib`
  - `libMoltenVK.dylib`
- Writes the `MoltenVK_icd.json` ICD manifest
- Generates a `run.sh` wrapper that sets `VK_ICD_FILENAMES` before launching

---

## Running

```bash
./scripts/run-macos-zh.sh -win
```

Or use the generated wrapper in the deploy directory:

```bash
~/GeneralsX/GeneralsZH/run.sh -win -noshellmap
```

Legacy fallback path also works:

```bash
~/GeneralsX/GeneralsMD/run.sh -win -noshellmap
```

Common flags:

| Flag | Effect |
|------|--------|
| `-win` | Windowed mode (recommended for debugging) |
| `-fullscreen` | Fullscreen mode |
| `-noshellmap` | Skip the animated main menu shell map |
| `-xres 1280 -yres 720` | Set resolution |

---

## DXVK macOS Source Model

DXVK for macOS is consumed from the project fork as a **pinned commit** configured in
`cmake/dx8.cmake` (`DXVK_REMOTE_REF`).

- No local `PATCH_COMMAND` is executed in the current workflow.
- macOS fixes are expected to exist in the fork commit itself.
- For local DXVK development, use `-DSAGE_DXVK_USE_LOCAL_FORK=ON`.

---

## Troubleshooting

### "Vulkan SDK not found"

```
ERROR: Vulkan SDK not found at ~/VulkanSDK/
```

Install from <https://vulkan.lunarg.com/sdk/home#mac>. The SDK must be in
`~/VulkanSDK/<version>/macOS/lib/libvulkan.dylib`.

### "meson: command not found"

```bash
brew install meson
```

### DXVK Meson build fails with linker error

If you see `--version-script` linker errors, the DXVK source being built likely
does not include the darwin linker guard fix in its commit history.
Clean the DXVK build cache and reconfigure:

```bash
rm -rf build/macos-vulkan/_deps/dxvk-src build/macos-vulkan/_deps/dxvk-build
cmake --preset macos-vulkan
```

### `VK_ERROR_INCOMPATIBLE_DRIVER` in logs

This is addressed by the portability-enumeration fix included in the pinned fork
commit. If you see it:

1. Ensure the Vulkan SDK is installed via LunarG installer (not Homebrew)
2. Ensure `deploy-macos-zh.sh` was run (MoltenVK ICD JSON must be present)
3. Verify `VK_ICD_FILENAMES` points to the correct JSON in the runtime dir

### `VK_ERROR_FEATURE_NOT_PRESENT` — robustBufferAccess2 / nullDescriptor

```
[mvk-error] VK_ERROR_FEATURE_NOT_PRESENT: vkCreateDevice(): Requested physical
device feature specified by the 1st flag in VkPhysicalDeviceRobustness2FeaturesKHR
is not available on this device.
```

This is addressed in the pinned fork commit. If you see this, the DXVK dylib in
the game directory is stale or from a different DXVK source revision. Rebuild and
redeploy:

```bash
./scripts/build-macos-zh.sh --build-only
./scripts/deploy-macos-zh.sh
```

### Game crashes at startup (SIGSEGV)

Run with verbose MoltenVK output:

```bash
cd ~/GeneralsX/GeneralsZH
VK_ICD_FILENAMES=./MoltenVK_icd.json MVK_CONFIG_LOG_LEVEL=4 ./GeneralsXZH -win
```

### "Feature not present" Vulkan validation error

The pinned DXVK commit masks core features against what the physical device
actually supports.
If you still see this, MoltenVK may need an update. Re-running `deploy-macos-zh.sh`
after updating the Vulkan SDK copies the new `libMoltenVK.dylib` to the runtime dir.

---

## Current Status

| Feature | Status |
|---------|--------|
| CMake configure | Working |
| DXVK compile via Meson | Working (fork-pinned source model) |
| GeneralsXZH binary | Builds successfully |
| Vulkan device init | Working (MoltenVK -> Metal) |
| 3D rendering | Under active testing |
| Audio (OpenAL) | Not yet implemented (Phase 2) |
| Video (Bink) | Not yet implemented (Phase 3) |

---

## Related Scripts

| Script | Purpose |
|--------|---------|
| `scripts/build-macos-zh.sh` | Configure + build `GeneralsXZH` |
| `scripts/deploy-macos-zh.sh` | Deploy binary + Vulkan runtime to game dir |
| `scripts/run-macos-zh.sh` | Launch with correct environment |
| `cmake/dx8.cmake` | DXVK ExternalProject build (pinned fork commit) |
| `cmake/dxvk-macos-patches.py` | Deprecated legacy helper (not used by current build flow) |
| `CMakePresets.json` (`macos-vulkan`) | Build preset (arm64, MoltenVK, SDL3, OpenAL, ffmpeg) |

---

*See the [Dev Blog](../../DEV_BLOG/) for detailed session-by-session technical notes.*