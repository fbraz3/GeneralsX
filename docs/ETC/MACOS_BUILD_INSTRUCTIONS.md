# GeneralsX - macOS Build Instructions (Apple Silicon)

> **Status**: 🔄 **IN PROGRESS** — Native ARM64 builds working.
>
> Graphics rendering (DXVK → MoltenVK → Metal) is functional. Audio and video playback
> are not yet implemented (Phase 2/3 targets), so the game runs silently.
>
> The entire DXVK patch series (9 patches for macOS compatibility) is scripted and
> applied automatically by CMake — no manual intervention needed.

---

## Prerequisites

### System Requirements

- **macOS 13 (Ventura) or later** on Apple Silicon (M1/M2/M3/M4)
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
brew install cmake ninja meson python3
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
2. Runs `cmake --preset macos-vulkan` (fetches + patches + builds DXVK via Meson)
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

- Copies `build/macos-vulkan/GeneralsMD/GeneralsXZH` to `~/GeneralsX/GeneralsMD/`
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

## How DXVK Patches Work

The macOS DXVK port requires 9 patches to DXVK 2.6 source code. These are fully
automated — **no manual patching is required**.

`cmake/dx8.cmake` uses CMake's `ExternalProject_Add` with a `PATCH_COMMAND` that
invokes `cmake/dxvk-macos-patches.py` immediately after fetching DXVK from git:

```cmake
ExternalProject_Add(dxvk
    ...
    PATCH_COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/cmake/dxvk-macos-patches.py
        ${DXVK_SOURCE_DIR}
    ...
)
```

### Patch Summary

| # | File | Problem | Fix |
|---|------|---------|-----|
| 1 | `util_win32_compat.h` | `__unix__` macro undefined on macOS | Add `|| defined(__APPLE__)` |
| 2 | `util_env.cpp` | `pthread_setname_np` Linux 2-arg form | Use 1-arg macOS form with `#ifdef __APPLE__` |
| 3 | `util_small_vector.h` | `lzcnt(size_t)` ambiguous on arm64 | Cast to `uint64_t` |
| 4 | `util_bit.h` | `uintptr_t` overload ambiguity | Add explicit `uintptr_t` overloads |
| 5 | `meson.build` x5 | `--version-script` is GNU ld only | Guard with `if platform != 'darwin'` |
| 6 | `util_env.cpp` | `getExePath()` returns empty on macOS | Use `_NSGetExecutablePath()` |
| 7 | `vulkan_loader.cpp` | Tries to dlopen `libvulkan.so` (Linux name) | Add `#elif defined(__APPLE__)` block with `.dylib` names |
| 8 | `dxvk_extensions.h` + `dxvk_instance.cpp` | `vkEnumeratePhysicalDevices` returns `VK_ERROR_INCOMPATIBLE_DRIVER` | Enable `VK_KHR_portability_enumeration` + `ENUMERATE_PORTABILITY` flag |
| 9 | `dxvk_adapter.cpp` | `vkCreateDevice` returns `VK_ERROR_FEATURE_NOT_PRESENT` (tessellationShader, shaderFloat64, robustness2) | Enable `VK_KHR_portability_subset`, mask core features against device caps, inject pNext chain |

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

If you see `--version-script` linker errors, it means Patch 5 did not apply.
Clean the DXVK build cache and reconfigure:

```bash
rm -rf build/macos-vulkan/_deps/dxvk-src build/macos-vulkan/_deps/dxvk-build
cmake --preset macos-vulkan
```

### `VK_ERROR_INCOMPATIBLE_DRIVER` in logs

This was fixed by Patch 8 (portability enumeration). If you see it:

1. Ensure the Vulkan SDK is installed via LunarG installer (not Homebrew)
2. Ensure `deploy-macos-zh.sh` was run (MoltenVK ICD JSON must be present)
3. Verify `VK_ICD_FILENAMES` points to the correct JSON in the runtime dir

### Game crashes at startup (SIGSEGV)

Run with verbose MoltenVK output:

```bash
cd ~/GeneralsX/GeneralsMD
VK_ICD_FILENAMES=./MoltenVK_icd.json MVK_CONFIG_LOG_LEVEL=4 ./GeneralsXZH -win
```

### "Feature not present" Vulkan validation error

Patch 9 masks core features against what the physical device actually supports.
If you still see this, MoltenVK may need an update. Re-running `deploy-macos-zh.sh`
after updating the Vulkan SDK copies the new `libMoltenVK.dylib` to the runtime dir.

---

## Current Status

| Feature | Status |
|---------|--------|
| CMake configure | Working |
| DXVK compile via Meson | Working (9-patch series auto-applied) |
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
| `cmake/dx8.cmake` | DXVK ExternalProject build (includes PATCH_COMMAND) |
| `cmake/dxvk-macos-patches.py` | All 9 DXVK patches (auto-applied by cmake) |
| `CMakePresets.json` (`macos-vulkan`) | Build preset (arm64, MoltenVK, SDL3, OpenAL, ffmpeg) |

---

*See the [Dev Blog](../../DEV_BLOG/) for detailed session-by-session technical notes.*
