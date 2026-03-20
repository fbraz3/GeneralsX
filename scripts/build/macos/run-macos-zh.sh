#!/bin/bash
# GeneralsX @build BenderAI 24/02/2026 Launch script for macOS (MoltenVK + SDL3)

set -e

# GeneralsX @bugfix BenderAI 09/03/2026 Resolve repository root correctly from scripts/build/macos.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/macos-vulkan"
GAME_DIR="${HOME}/GeneralsX/GeneralsMD"
GAME_BINARY="${GAME_DIR}/GeneralsXZH"
LOG_FILE="${PROJECT_ROOT}/logs/run_zh_macos.log"

if [[ ! -f "${GAME_BINARY}" ]]; then
    echo "ERROR: Game binary not found at ${GAME_BINARY}"
    echo "Run deploy first: ./scripts/build/macos/deploy-macos-zh.sh"
    exit 1
fi

if [[ ! -f "${GAME_DIR}/libSDL3.0.dylib" ]]; then
    echo "ERROR: SDL3 library not found in ${GAME_DIR}"
    echo "Run deploy first: ./scripts/build/macos/deploy-macos-zh.sh"
    exit 1
fi

# Dylibs in game dir + Vulkan SDK for MoltenVK
export DYLD_LIBRARY_PATH="${GAME_DIR}:${DYLD_LIBRARY_PATH:-}"

# MoltenVK ICD so Vulkan loader finds MoltenVK.
# deploy-macos-zh.sh copies MoltenVK_icd.json into the game runtime dir;
# fall back to the Vulkan SDK path if it is not there yet.
MVK_ICD="${GAME_DIR}/MoltenVK_icd.json"
if [[ ! -f "${MVK_ICD}" ]]; then
    # Try the Vulkan SDK installation path (LunarG installer layout)
    for sdk_candidate in "${HOME}/VulkanSDK"/*/macOS; do
        if [[ -f "${sdk_candidate}/share/vulkan/icd.d/MoltenVK_icd.json" ]]; then
            MVK_ICD="${sdk_candidate}/share/vulkan/icd.d/MoltenVK_icd.json"
            break
        fi
    done
fi
if [[ -f "${MVK_ICD}" ]]; then
    export VK_ICD_FILENAMES="${MVK_ICD}"
fi

# Disable validation layers in production runs (remove for debug)
export VK_INSTANCE_LAYERS=""

# GeneralsX @bugfix fbraz3 20/03/2026 DXVK requires DXVK_WSI_DRIVER on non-Win32; must match game windowing (SDL3)
export DXVK_WSI_DRIVER="SDL3"

# GeneralsX @bugfix BenderAI 13/03/2026 Explicitly point DXVK to runtime config file.
# Relying only on CWD can hide config/deploy issues and mislead terrain shader diagnostics.
if [[ -f "${GAME_DIR}/dxvk.conf" ]]; then
    export DXVK_CONFIG_FILE="${GAME_DIR}/dxvk.conf"
else
    echo "WARNING: ${GAME_DIR}/dxvk.conf not found; DXVK will use defaults."
fi

# GeneralsX @tweak BenderAI 13/03/2026 Optional shader-cache reset for terrain debugging.
# Use GX_CLEAR_DXVK_SHADER_CACHE=1 to force fresh shader compilation.
if [[ "${GX_CLEAR_DXVK_SHADER_CACHE:-0}" == "1" ]]; then
    DXVK_CACHE_DIR="${XDG_CACHE_HOME:-${HOME}/.cache}/dxvk"
    if [[ -d "${DXVK_CACHE_DIR}" ]]; then
        echo "Clearing DXVK shader cache in ${DXVK_CACHE_DIR}"
        rm -f "${DXVK_CACHE_DIR}"/*.dxvk.bin "${DXVK_CACHE_DIR}"/*.dxvk.lut 2>/dev/null || true
    fi
fi

mkdir -p "${PROJECT_ROOT}/logs"

echo "Launching GeneralsXZH (macOS)"
echo "   Game:  ${GAME_BINARY}"
echo "   SDL3:  ${GAME_DIR}/libSDL3.0.dylib"
echo "   Vulkan ICD: ${VK_ICD_FILENAMES:-system default}"
echo "   DXVK config: ${DXVK_CONFIG_FILE:-cwd/dxvk.conf (implicit)}"
echo ""

cd "${GAME_DIR}"
exec "${GAME_BINARY}" "$@" 2>&1 | tee "${LOG_FILE}"
