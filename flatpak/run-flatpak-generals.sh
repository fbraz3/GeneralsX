#!/usr/bin/env bash
# GeneralsX @build GitHubCopilot 09/04/2026 Flatpak launcher wrapper for Generals with host data path fallback.
set -euo pipefail

if [[ -n "${LD_LIBRARY_PATH:-}" ]]; then
    export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/app/lib"
else
    export LD_LIBRARY_PATH="/app/lib"
fi
export DXVK_WSI_DRIVER="SDL3"
export DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-info}"
export DXVK_HUD="${DXVK_HUD:-0}"

# GeneralsX @bugfix GitHubCopilot 09/04/2026 Fix Vulkan WSI(VK_KHR_surface) in Flatpak sandbox with Wayland/software fallback.
# Issue: Flatpak sandbox has DISPLAY empty but WAYLAND_DISPLAY=wayland-0; Vulkan WSI fails without proper config.
# Solution: 1) Force Wayland-only mode 2) Prioritize Wayland provider 3) Enable software rendering fallback
# Note: Intel driver tries X11 first (even though DISPLAY=:0 doesn't work in sandbox), then Wayland.
# We UNSET DISPLAY to force Wayland-only path in the Vulkan driver.
unset DISPLAY  # Force Wayland WSI path
export WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-0}"

# Prioritize Wayland-capable ICD
if [[ -z "${VK_ICD_FILENAMES:-}" ]]; then
    if [[ -f "/usr/share/vulkan/icd.d/intel_hasvk_icd.x86_64.json" ]]; then
        export VK_ICD_FILENAMES="/usr/share/vulkan/icd.d/intel_hasvk_icd.x86_64.json"
    elif [[ -f "/usr/share/vulkan/icd.d/intel_icd.x86_64.json" ]]; then
        export VK_ICD_FILENAMES="/usr/share/vulkan/icd.d/intel_icd.x86_64.json"
    elif [[ -f "/usr/share/vulkan/icd.d/lvp_icd.x86_64.json" ]]; then
        export VK_ICD_FILENAMES="/usr/share/vulkan/icd.d/lvp_icd.x86_64.json"
    fi
fi

# Enable DXVK software rendering as fallback when Vulkan WSI fails
export DXVK_FORCESWRENDER="${DXVK_FORCESWRENDER:-0}"

if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" ]]; then
    if [[ -d "${HOME}/GeneralsX/Generals" ]]; then
        export CNC_GENERALS_INSTALLPATH="${HOME}/GeneralsX/Generals/"
    elif [[ -d "${HOME}/.local/share/GeneralsX/Generals" ]]; then
        export CNC_GENERALS_INSTALLPATH="${HOME}/.local/share/GeneralsX/Generals/"
    fi
fi

if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" ]]; then
    echo "ERROR: Could not find game data directory." >&2
    echo "Expected one of these paths:" >&2
    echo "  ${HOME}/GeneralsX/Generals" >&2
    echo "  ${HOME}/.local/share/GeneralsX/Generals" >&2
    exit 1
fi

exec /app/bin/GeneralsX "$@"

# GeneralsX @bugfix GitHubCopilot 09/04/2026 Force DXVK software rendering in Flatpak due to Vulkan WSI unavailability.
# Issue: Vulkan Radeon driver fails to load due to missing xcb_dri3_import_syncobj_checked symbol
# This is a Freedesktop SDK 25.08 libxcb version incompatibility issue.
# Workaround: Force DXVK to software-render instead of trying Vulkan.
export DXVK_RENDERER="software"
export DXVK_FORCESWRENDER="1"
