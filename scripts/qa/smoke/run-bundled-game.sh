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

# Determine OS and set library path variable
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    export DYLD_LIBRARY_PATH="${SCRIPT_DIR}/lib:${DYLD_LIBRARY_PATH:-}"
    export DYLD_FALLBACK_LIBRARY_PATH="${SCRIPT_DIR}/lib:${DYLD_FALLBACK_LIBRARY_PATH:-}"
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
