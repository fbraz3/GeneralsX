#!/usr/bin/env bash
# GeneralsX @build GitHubCopilot 09/04/2026 Flatpak launcher wrapper with host data path fallback.
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

# GeneralsX @tweak GitHubCopilot 09/04/2026 Allow explicit experimental runtime mode for bundled XCB stack.
# Usage:
#   GENERALSX_FLATPAK_RUNTIME_MODE=stock   -> Default behavior
#   GENERALSX_FLATPAK_RUNTIME_MODE=vendor-xcb -> Force /app/lib first and software Vulkan fallback knobs
RUNTIME_MODE="${GENERALSX_FLATPAK_RUNTIME_MODE:-stock}"
if [[ "${RUNTIME_MODE}" == "vendor-xcb" ]]; then
    export LD_LIBRARY_PATH="/app/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
    export DXVK_RENDERER="software"
    export DXVK_FORCESWRENDER="1"
else
    export DXVK_FORCESWRENDER="${DXVK_FORCESWRENDER:-0}"
fi

if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" ]]; then
    if [[ -d "${HOME}/GeneralsX/GeneralsZH" ]]; then
        export CNC_GENERALS_INSTALLPATH="${HOME}/GeneralsX/GeneralsZH/"
    elif [[ -d "${HOME}/GeneralsX/GeneralsMD" ]]; then
        export CNC_GENERALS_INSTALLPATH="${HOME}/GeneralsX/GeneralsMD/"
    elif [[ -d "${HOME}/.local/share/GeneralsX/GeneralsZH" ]]; then
        export CNC_GENERALS_INSTALLPATH="${HOME}/.local/share/GeneralsX/GeneralsZH/"
    elif [[ -d "${HOME}/.local/share/GeneralsX/GeneralsMD" ]]; then
        export CNC_GENERALS_INSTALLPATH="${HOME}/.local/share/GeneralsX/GeneralsMD/"
    fi
fi

if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" ]]; then
    echo "ERROR: Could not find game data directory." >&2
    echo "Expected one of these paths:" >&2
    echo "  ${HOME}/GeneralsX/GeneralsZH" >&2
    echo "  ${HOME}/GeneralsX/GeneralsMD" >&2
    echo "  ${HOME}/.local/share/GeneralsX/GeneralsZH" >&2
    echo "  ${HOME}/.local/share/GeneralsX/GeneralsMD" >&2
    exit 1
fi

exec /app/bin/GeneralsXZH "$@"
