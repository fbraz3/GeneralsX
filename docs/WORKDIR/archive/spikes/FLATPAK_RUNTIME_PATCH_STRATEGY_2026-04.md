# Flatpak Runtime Patch Strategy (April 2026)

## Problem Statement

GeneralsX Flatpak builds currently fail during Vulkan WSI initialization with:

- `VK_KHR_surface not implemented`
- Driver load error with missing symbol: `xcb_dri3_import_syncobj_checked`

The failure was reproduced on Freedesktop runtime `25.08` and appears to come from runtime-provided `libxcb-dri3` being older than what modern Mesa Vulkan drivers expect.

## Decision

Do not fork Flatpak itself. Use a staged strategy focused on runtime compatibility:

1. Track A (fast): app-level experimental mode with bundled user-space libs for validation.
2. Track B (robust): maintain a custom runtime branch if upstream latency is high.
3. Upstream-first: prepare and submit reproducible issue/PR to Freedesktop SDK.

## Track A: App-Level Validation (Fast)

Goal: validate whether newer XCB stack in app runtime path can unblock driver load.

- Keep default behavior unchanged.
- Add opt-in mode controlled by environment variable:
  - `GENERALSX_FLATPAK_RUNTIME_MODE=stock` (default)
  - `GENERALSX_FLATPAK_RUNTIME_MODE=vendor-xcb` (experimental)
- In experimental mode:
  - prepend `/app/lib` in `LD_LIBRARY_PATH`
  - enable software rendering knobs for early smoke tests

Acceptance criteria:

- Game binary starts further than current Vulkan loader failure, or
- diagnostics prove symbol issue still unresolved with app-level override.

## Track B: Custom Runtime (Robust)

Goal: guarantee cross-distro compatibility independent from host package set.

- Base on Freedesktop `25.08` branch.
- Patch/bump `libxcb` stack in runtime build.
- Publish runtime as `org.generalsx.Platform//25.08-gx1` from project OSTree remote.
- Point app manifests to custom runtime only if Track A and upstream path are insufficient.

Acceptance criteria:

- Reproducible launch on Ubuntu and Fedora-family hosts with same runtime revision.
- No regression in existing app startup flow.

## Upstream Contribution Path

- Open issue in Freedesktop SDK with full diagnostics log.
- Provide exact missing symbol and Vulkan loader evidence.
- If maintainers confirm, submit PR with minimally scoped runtime package bump.

## Tooling Added

Use this report collector for reproducible evidence:

- `scripts/qa/smoke/collect-flatpak-vulkan-wsi-report.sh`

The generated log can be attached directly to an upstream issue.
