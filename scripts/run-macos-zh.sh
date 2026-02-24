#!/bin/bash
# GeneralsX @build BenderAI 24/02/2026 Launch script for macOS (MoltenVK + SDL3)

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/macos-vulkan"
GAME_DIR="${HOME}/GeneralsX/GeneralsMD"
GAME_BINARY="${GAME_DIR}/GeneralsXZH"
LOG_FILE="${PROJECT_ROOT}/logs/run_zh_macos.log"

if [[ ! -f "${GAME_BINARY}" ]]; then
    echo "ERROR: Game binary not found at ${GAME_BINARY}"
    echo "Run deploy first: ./scripts/deploy-macos-zh.sh"
    exit 1
fi

if [[ ! -f "${GAME_DIR}/libSDL3.0.dylib" ]]; then
    echo "ERROR: SDL3 library not found in ${GAME_DIR}"
    echo "Run deploy first: ./scripts/deploy-macos-zh.sh"
    exit 1
fi

# Dylibs in game dir + Vulkan SDK for MoltenVK
export DYLD_LIBRARY_PATH="${GAME_DIR}:${DYLD_LIBRARY_PATH:-}"

# MoltenVK ICD so Vulkan loader finds MoltenVK
MVK_ICD="$(find "${HOME}" /Users -maxdepth 6 -name "MoltenVK_icd.json" 2>/dev/null | head -1 || true)"
if [[ -n "${MVK_ICD}" ]]; then
    export VK_ICD_FILENAMES="${MVK_ICD}"
fi

# Disable validation layers in production runs (remove for debug)
export VK_INSTANCE_LAYERS=""

mkdir -p "${PROJECT_ROOT}/logs"

echo "Launching GeneralsXZH (macOS)"
echo "   Game:  ${GAME_BINARY}"
echo "   SDL3:  ${GAME_DIR}/libSDL3.0.dylib"
echo "   Vulkan ICD: ${VK_ICD_FILENAMES:-system default}"
echo ""

cd "${GAME_DIR}"
exec "${GAME_BINARY}" "$@" 2>&1 | tee "${LOG_FILE}"
