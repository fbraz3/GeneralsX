# AppImage PoC Plan (April 2026)

## Why AppImage now

Flatpak currently requires heavy runtime workarounds for Vulkan WSI/XCB compatibility.
A short-term AppImage path reduces distro ABI friction while keeping distribution simple for end users.

## PoC Scope

- Target: Zero Hour runtime (`GeneralsXZH`)
- Output: single portable file under `build/`
- Tooling: `scripts/build/linux/build-linux-appimage-zh.sh`

## Included runtime artifacts

- Game binary (`GeneralsXZH`)
- DXVK userspace libs (`libdxvk_d3d8.so*`, optional d3d9)
- SDL3 + SDL3_image
- OpenAL
- GameSpy
- Optional `dxvk.conf`

## Launcher behavior

- Uses bundled libs via `LD_LIBRARY_PATH`
- Exposes DXVK defaults (`DXVK_WSI_DRIVER=SDL3`, logging/HUD knobs)
- Auto-detects base Generals path from common directories
- Keeps OpenAL workaround env for known alignment/backend issues

## Build command

Example:

- `./scripts/build/linux/build-linux-appimage-zh.sh linux64-deploy`

## Validation checklist

- AppImage file created under `build/`
- Binary starts from AppImage launcher
- Intro/menu reached with expected Vulkan + SDL3 path
- Smoke test on at least two distro families

## Risks

- Host driver stack (Vulkan ICD) still remains a runtime dependency
- Some environments may require additional policy tweaks (for example FUSE or sandbox constraints)
- FFmpeg/video libraries are not yet bundled in this initial PoC
