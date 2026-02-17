#!/bin/bash
# GeneralsX @build felipebraz 16/02/2026 Launch script for Linux with DXVK

set -e

LOG_FILE="logs/run_zh.log"

# Runtime directory
GAME_DIR="${HOME}/GeneralsX/GeneralsMD"
GAME_BINARY="${GAME_DIR}/GeneralsXZH"

# Check if binary exists
if [[ ! -f "${GAME_BINARY}" ]]; then
    echo "❌ ERROR: Game binary not found at ${GAME_BINARY}"
    echo "Run deploy first: ./scripts/deploy-linux-zh.sh"
    exit 1
fi

# Check if DXVK library is deployed
if [[ ! -f "${GAME_DIR}/libdxvk_d3d8.so" ]]; then
    echo "❌ ERROR: DXVK library not found in ${GAME_DIR}"
    echo "Run deploy first: ./scripts/deploy-linux-zh.sh"
    exit 1
fi

# Set LD_LIBRARY_PATH so dlopen() can find DXVK dependencies
export LD_LIBRARY_PATH="${GAME_DIR}:${LD_LIBRARY_PATH:-}"

# Set up environment variables for DXVK
export DXVK_WSI_DRIVER="SDL3"
export DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-info}"  # Override with 'debug' if needed
export DXVK_HUD="${DXVK_HUD:-0}"  # Set to '1' for FPS overlay

echo "🚀 Launching GeneralsXZH (Linux)"
echo "   Game: ${GAME_BINARY}"
echo "   DXVK: libdxvk_*.so (local + LD_LIBRARY_PATH)"
echo ""

# Change to game directory (needs game data files)
cd "${GAME_DIR}"

# Launch with arguments (pass all script args to game)
exec "${GAME_BINARY}" "$@" 2>&1 |tee "$LOG_FILE"

echo 
echo "------------------------"
echo "✅ Run complete:"
echo "Exit code: $?"
echo "Log file: ${LOG_FILE}"