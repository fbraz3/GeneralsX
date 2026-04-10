# Flatpak libxcb PoC Quickstart (April 2026)

## Goal

Provide a short-term validation path for Vulkan WSI loader issues by injecting a newer `libxcb` userspace stack into the Flatpak app runtime.

## What Was Added

The Linux Flatpak build script now supports:

- `LIBXCB_POC_DIR=/path/to/libs`

When set, the script copies `libxcb*`, `libX11*`, and related companion libs from that directory into app runtime staging and skips the host fallback XCB copy path.

## Expected Directory Contents

At minimum, the PoC directory should include version-compatible files such as:

- `libxcb.so*`
- `libxcb-present.so*`
- `libxcb-dri3.so*`
- `libxcb-sync.so*`
- `libxcb-randr.so*`
- `libX11.so*`
- `libX11-xcb.so*`
- `libxshmfence.so*`

Prefer preserving symlinks (`cp -a` style layout) from source distro/runtime.

## Build Example

```bash
cd ~/Projects/GeneralsX
export LIBXCB_POC_DIR="$PWD/flatpak/poc-libxcb"
./scripts/build/linux/build-linux-flatpak.sh linux64-deploy GeneralsMD
```

## Runtime Example

Use the experimental wrapper mode to prioritize `/app/lib`:

```bash
GENERALSX_FLATPAK_RUNTIME_MODE=vendor-xcb flatpak run com.generals.GeneralsXZH -win -noshellmap
```

## Diagnostics

Capture a reproducible report after each PoC run:

```bash
./scripts/qa/smoke/collect-flatpak-vulkan-wsi-report.sh com.generals.GeneralsXZH
```

Attach the resulting log from `logs/` when comparing PoC variants.
