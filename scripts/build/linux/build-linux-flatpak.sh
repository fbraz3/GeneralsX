#!/usr/bin/env bash
# GeneralsX @build GitHubCopilot 13/04/2026 Build Flatpak bundles by compiling inside org.freedesktop.Sdk.
# Usage:
#   ./scripts/build/linux/build-linux-flatpak.sh [preset] [game]
#   game: GeneralsMD (default) or Generals
set -euo pipefail

# GeneralsX @build GitHubCopilot 14/04/2026 Timestamp helper for progress tracking.
ts() { date '+%H:%M:%S'; }
elapsed() {
    local start="$1"
    local end
    end=$(date +%s)
    local diff=$(( end - start ))
    printf '%dm%02ds' $(( diff / 60 )) $(( diff % 60 ))
}

PRESET="${1:-linux64-deploy}"
GAME="${2:-GeneralsMD}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
FLATPAK_DIR="${PROJECT_ROOT}/flatpak"
FLATPAK_BUILD_DIR="${PROJECT_ROOT}/build/flatpak-builddir"
FLATPAK_REPO_DIR="${PROJECT_ROOT}/build/flatpak-repo"
FLATPAK_STATE_DIR="${PROJECT_ROOT}/.flatpak-builder"
RUNTIME_REPO_URL="${RUNTIME_REPO_URL:-https://flathub.org/repo/flathub.flatpakrepo}"
# GeneralsX @build GitHubCopilot 13/04/2026 Optional hard purge for troubleshooting (drops flatpak-builder cache + workdirs).
GENERALSX_FLATPAK_PURGE_CACHE="${GENERALSX_FLATPAK_PURGE_CACHE:-0}"
# GeneralsX @build GitHubCopilot 14/04/2026 Enable flatpak-builder ccache by default for cross-session object reuse.
GENERALSX_FLATPAK_USE_CCACHE="${GENERALSX_FLATPAK_USE_CCACHE:-1}"

case "${GAME}" in
    GeneralsMD)
        MANIFEST="${FLATPAK_DIR}/com.fbraz3.GeneralsXZH.yml"
        APP_ID="com.fbraz3.GeneralsXZH"
        OUTPUT_BUNDLE="${PROJECT_ROOT}/build/GeneralsXZH-${PRESET}.flatpak"
        ;;
    Generals)
        MANIFEST="${FLATPAK_DIR}/com.fbraz3.GeneralsX.yml"
        APP_ID="com.fbraz3.GeneralsX"
        OUTPUT_BUNDLE="${PROJECT_ROOT}/build/GeneralsX-${PRESET}.flatpak"
        ;;
    *)
        echo "ERROR: Unsupported game '${GAME}'. Use GeneralsMD or Generals." >&2
        exit 1
        ;;
esac

BUILD_START=$(date +%s)
echo "[$(ts)] Building Flatpak in SDK for game ${GAME} (preset label: ${PRESET})"

if [[ ! -f "${MANIFEST}" ]]; then
    echo "ERROR: Missing Flatpak manifest ${MANIFEST}" >&2
    exit 1
fi
if ! command -v flatpak-builder >/dev/null 2>&1; then
    echo "ERROR: flatpak-builder is not installed." >&2
    echo "Install with: sudo apt-get install flatpak flatpak-builder" >&2
    exit 1
fi
if ! command -v flatpak >/dev/null 2>&1; then
    echo "ERROR: flatpak is not installed." >&2
    echo "Install with: sudo apt-get install flatpak" >&2
    exit 1
fi

if ! flatpak --user remote-list | awk '{print $1}' | grep -qx "flathub"; then
    echo "[$(ts)] Adding flathub remote for current user..."
    flatpak --user remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
fi

if ! flatpak --user info org.freedesktop.Platform//25.08 >/dev/null 2>&1 || \
   ! flatpak --user info org.freedesktop.Sdk//25.08 >/dev/null 2>&1; then
    echo "[$(ts)] Installing required Flatpak runtime and SDK (25.08) for current user..."
    flatpak --user install -y flathub org.freedesktop.Platform//25.08 org.freedesktop.Sdk//25.08
fi

if [[ "${GENERALSX_FLATPAK_PURGE_CACHE}" == "1" ]]; then
    echo "[$(ts)] Full purge requested (GENERALSX_FLATPAK_PURGE_CACHE=1): removing Flatpak build dirs and local flatpak-builder cache..."
    rm -rf "${FLATPAK_BUILD_DIR}" "${FLATPAK_REPO_DIR}"
    rm -rf "${FLATPAK_STATE_DIR}"
fi

mkdir -p "${FLATPAK_BUILD_DIR}" "${FLATPAK_REPO_DIR}" "${FLATPAK_STATE_DIR}"

echo "[$(ts)] Running flatpak-builder (build inside SDK sandbox)..."
FLATPAK_BUILDER_START=$(date +%s)
BUILDER_ARGS=(
    --verbose
    --user
    --force-clean
    --state-dir="${FLATPAK_STATE_DIR}"
    --repo="${FLATPAK_REPO_DIR}"
    --install-deps-from=flathub
)

if [[ "${GENERALSX_FLATPAK_USE_CCACHE}" == "1" ]]; then
    BUILDER_ARGS+=(--ccache)
    echo "[$(ts)] flatpak-builder ccache enabled (GENERALSX_FLATPAK_USE_CCACHE=1)."
else
    echo "[$(ts)] flatpak-builder ccache disabled (GENERALSX_FLATPAK_USE_CCACHE=0)."
fi

echo "Using flatpak-builder state/cache dir: ${FLATPAK_STATE_DIR}"
echo "Set GENERALSX_FLATPAK_PURGE_CACHE=1 for full purge."
echo "Note: flatpak-builder can stay quiet for a while during finalization/export."
echo "Do not interrupt after the last module command unless an explicit error is shown."

flatpak-builder "${BUILDER_ARGS[@]}" \
    "${FLATPAK_BUILD_DIR}" \
    "${MANIFEST}"

echo "[$(ts)] flatpak-builder completed in $(elapsed ${FLATPAK_BUILDER_START}). Building final .flatpak bundle..."
BUNDLE_START=$(date +%s)
flatpak build-bundle --runtime-repo="${RUNTIME_REPO_URL}" "${FLATPAK_REPO_DIR}" "${OUTPUT_BUNDLE}" "${APP_ID}"

echo "[$(ts)] .flatpak bundle created in $(elapsed ${BUNDLE_START}) (total: $(elapsed ${BUILD_START})). Output: ${OUTPUT_BUNDLE}"
echo "Install example:"
echo "  flatpak --user install -y \"${OUTPUT_BUNDLE}\""
