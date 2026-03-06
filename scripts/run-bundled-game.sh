#!/bin/bash
# GeneralsX @build felipebraz 28/02/2026 - Runtime wrapper for bundled game executable
# 
# This script is deployed alongside the game binary and libraries in GitHub Actions bundles.
# It sets up the environment (LD_LIBRARY_PATH, DYLD_LIBRARY_PATH, DXVK vars) and launches the game.
#
# Usage: ./run.sh [game-args...]
# Example: ./run.sh -win
#          ./run.sh -noshellmap
#

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# GeneralsX @bugfix felipebraz 05/03/2026 Remove macOS quarantine and ad-hoc sign all binaries on first run.
# macOS adds com.apple.quarantine to every file extracted from a downloaded archive.
# This causes Gatekeeper to require one-by-one approval for the binary AND each dylib.
# Stripping quarantine + ad-hoc codesigning resolves this without a paid Apple Developer ID.
if [[ "$OSTYPE" == "darwin"* ]]; then
    # Strip quarantine attribute from all bundle files (silent if already clear)
    xattr -rd com.apple.quarantine "${SCRIPT_DIR}" 2>/dev/null || true

    # Ad-hoc sign: dylibs first, then the main executable (order matters for --verify)
    if command -v codesign &>/dev/null; then
        find "${SCRIPT_DIR}/lib" -name "*.dylib" 2>/dev/null | while IFS= read -r lib; do
            codesign --force --sign - "$lib" 2>/dev/null || true
        done
        for exe in "${SCRIPT_DIR}/GeneralsXZH" "${SCRIPT_DIR}/GeneralsX"; do
            [ -f "$exe" ] && codesign --force --sign - "$exe" 2>/dev/null || true
        done
    fi
fi

# Determine OS and set library path variable
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    export DYLD_LIBRARY_PATH="${SCRIPT_DIR}/lib:${DYLD_LIBRARY_PATH:-}"
    export DYLD_FALLBACK_LIBRARY_PATH="${SCRIPT_DIR}/lib:${DYLD_FALLBACK_LIBRARY_PATH:-}"
    # GeneralsX @bugfix felipebraz 04/03/2026 Ensure Vulkan loader uses bundled MoltenVK ICD manifest.
    if [[ -f "${SCRIPT_DIR}/lib/MoltenVK_icd.json" ]]; then
        export VK_ICD_FILENAMES="${SCRIPT_DIR}/lib/MoltenVK_icd.json"
    fi
else
    # Linux
    export LD_LIBRARY_PATH="${SCRIPT_DIR}/lib:${LD_LIBRARY_PATH:-}"
fi

# Set DXVK environment variables
export DXVK_WSI_DRIVER="SDL3"
export DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-info}"
export DXVK_HUD="${DXVK_HUD:-0}"

# Find the game executable in this directory
# Try both common names
EXECUTABLE=""
if [ -f "${SCRIPT_DIR}/GeneralsXZH" ]; then
    EXECUTABLE="${SCRIPT_DIR}/GeneralsXZH"
elif [ -f "${SCRIPT_DIR}/GeneralsX" ]; then
    EXECUTABLE="${SCRIPT_DIR}/GeneralsX"
else
    echo "ERROR: Game executable not found in ${SCRIPT_DIR}"
    echo "Expected: GeneralsXZH or GeneralsX"
    exit 1
fi

# Launch the game with all passed arguments
exec "${EXECUTABLE}" "$@"
