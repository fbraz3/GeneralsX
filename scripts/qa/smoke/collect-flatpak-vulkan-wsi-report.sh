#!/usr/bin/env bash
# Collect a reproducible Vulkan WSI diagnostics report from Flatpak sandbox.
# GeneralsX @build GitHubCopilot 09/04/2026 Add reproducible Flatpak Vulkan WSI diagnostics collector for upstream runtime issues.
#
# Usage:
#   ./scripts/qa/smoke/collect-flatpak-vulkan-wsi-report.sh [app-id]
#
# Example:
#   ./scripts/qa/smoke/collect-flatpak-vulkan-wsi-report.sh com.generals.GeneralsXZH
set -euo pipefail

APP_ID="${1:-com.generals.GeneralsXZH}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
LOG_DIR="${PROJECT_ROOT}/logs"
TS="$(date +%Y%m%d_%H%M%S)"
OUT_FILE="${LOG_DIR}/flatpak-vulkan-wsi-${APP_ID//./_}-${TS}.log"

mkdir -p "${LOG_DIR}"

{
    echo "== GeneralsX Flatpak Vulkan WSI Diagnostics =="
    echo "Date: $(date -Iseconds)"
    echo "App ID: ${APP_ID}"
    echo

    echo "== Host: vulkaninfo summary =="
    if command -v vulkaninfo >/dev/null 2>&1; then
        vulkaninfo --summary || true
    else
        echo "vulkaninfo not found on host"
    fi
    echo

    echo "== Host: mesa packages =="
    if command -v dpkg >/dev/null 2>&1; then
        dpkg -l | grep -E 'mesa|vulkan|libxcb' || true
    else
        echo "dpkg not available"
    fi
    echo

    echo "== Flatpak: runtime info =="
    flatpak --user info org.freedesktop.Platform//25.08 || true
    flatpak --user info org.freedesktop.Sdk//25.08 || true
    echo

    echo "== Flatpak: app info =="
    flatpak --user info "${APP_ID}" || true
    echo

    echo "== Flatpak sandbox: Vulkan loader debug =="
    flatpak run --command=bash "${APP_ID}" -c '
        set -e
        export VK_LOADER_DEBUG=all
        echo "WAYLAND_DISPLAY=${WAYLAND_DISPLAY-}"
        echo "DISPLAY=${DISPLAY-}"
        ls -la /usr/share/vulkan/icd.d || true
        for icd in /usr/share/vulkan/icd.d/*.json; do
            echo "-- Testing ICD: ${icd}"
            VK_ICD_FILENAMES="${icd}" /app/bin/'"$(basename "${APP_ID}" | sed 's/com.generals.//')"' -win -noshellmap || true
        done
    ' || true
    echo

    echo "== Flatpak sandbox: symbol check for xcb_dri3_import_syncobj_checked =="
    flatpak run --command=bash "${APP_ID}" -c '
        set -e
        target="/app/lib/libxcb-dri3.so.0.1.0"
        if [[ -f "${target}" ]]; then
            nm "${target}" 2>/dev/null | grep xcb_dri3_import_syncobj_checked || echo "Symbol not found in ${target}"
        else
            echo "Missing ${target}"
        fi
    ' || true
} >"${OUT_FILE}" 2>&1

echo "Diagnostics report generated: ${OUT_FILE}"