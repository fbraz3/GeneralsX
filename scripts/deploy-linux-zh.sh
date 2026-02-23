#!/bin/bash
# GeneralsX @build felipebraz 16/02/2026 Deploy Linux build to runtime directory
# GeneralsX @bugfix BenderAI 19/02/2026 Add non-empty check to guard against stale CMake placeholder files

set -e

# Directories
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/linux64-deploy"
DXVK_LIB_DIR="${BUILD_DIR}/_deps/dxvk-src/lib"
SDL3_LIB_DIR="${BUILD_DIR}/_deps/sdl3-build"
SDL3_IMAGE_LIB_DIR="${BUILD_DIR}/_deps/sdl3_image-build"
GAMESPY_LIB="${BUILD_DIR}/libgamespy.so"
RUNTIME_DIR="${HOME}/GeneralsX/GeneralsMD"
# Note: CMakeLists.txt uses OUTPUT_NAME GeneralsXZH on Linux (see GeneralsMD/Code/Main/CMakeLists.txt)
BINARY_SRC="${BUILD_DIR}/GeneralsMD/GeneralsXZH"

echo "Deploying GeneralsXZH (Linux) to ${RUNTIME_DIR}"

# Check if binary exists and is non-zero (CMake creates a 0-byte placeholder before the link step)
if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary not found at ${BINARY_SRC}"
    echo "Build first: ./scripts/docker-build-linux-zh.sh linux64-deploy"
    exit 1
fi
if [[ ! -s "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary at ${BINARY_SRC} is empty (0 bytes) - build may have failed"
    echo "Check build logs: ./scripts/docker-build-linux-zh.sh linux64-deploy"
    exit 1
fi

# Check if DXVK libraries exist
if [[ ! -d "${DXVK_LIB_DIR}" ]]; then
    echo "ERROR: DXVK libraries not found at ${DXVK_LIB_DIR}"
    echo "Configure first: ./scripts/docker-configure-linux.sh linux64-deploy"
    exit 1
fi

# Check if SDL3 libraries exist
if [[ ! -d "${SDL3_LIB_DIR}" ]]; then
    echo "ERROR: SDL3 libraries not found at ${SDL3_LIB_DIR}"
    echo "Build first: ./scripts/docker-build-linux-zh.sh linux64-deploy"
    exit 1
fi

if [[ ! -d "${SDL3_IMAGE_LIB_DIR}" ]]; then
    echo "ERROR: SDL3_image libraries not found at ${SDL3_IMAGE_LIB_DIR}"
    echo "Build first: ./scripts/docker-build-linux-zh.sh linux64-deploy"
    exit 1
fi

# Check if GameSpy library exists
if [[ ! -f "${GAMESPY_LIB}" ]]; then
    echo "ERROR: GameSpy library not found at ${GAMESPY_LIB}"
    echo "Build first: ./scripts/docker-build-linux-zh.sh linux64-deploy"
    exit 1
fi

# Create runtime directory if needed
mkdir -p "${RUNTIME_DIR}"

# Copy executable (deploy as GeneralsXZH for run script compatibility)
echo "  Copying GeneralsXZH..."
cp -v "${BINARY_SRC}" "${RUNTIME_DIR}/GeneralsXZH"
chmod +x "${RUNTIME_DIR}/GeneralsXZH"

# Copy DXVK libraries
echo "  Copying DXVK libraries..."
cp -v "${DXVK_LIB_DIR}"/libdxvk_d3d8.so* "${RUNTIME_DIR}/"
cp -v "${DXVK_LIB_DIR}"/libdxvk_d3d9.so* "${RUNTIME_DIR}/" 2>/dev/null || true

# Copy SDL3 and SDL3_image libraries (for cursor loading and window management)
echo "  Copying SDL3 libraries..."
cp -v "${SDL3_LIB_DIR}"/libSDL3.so* "${RUNTIME_DIR}/"
cp -v "${SDL3_IMAGE_LIB_DIR}"/libSDL3_image.so* "${RUNTIME_DIR}/"

# Copy GameSpy library (for online multiplayer)
echo "  Copying GameSpy library..."
cp -v "${GAMESPY_LIB}" "${RUNTIME_DIR}/"

# Set RPATH so executable finds libraries in same directory
echo "  Setting RPATH to \$ORIGIN..."
patchelf --set-rpath '$ORIGIN' "${RUNTIME_DIR}/GeneralsXZH" 2>/dev/null || {
    echo "WARNING: patchelf not found. Install with: sudo apt install patchelf"
    echo "    Libraries will need LD_LIBRARY_PATH or manual RPATH setting"
}

# Copy run wrapper script
echo "  Copying run.sh wrapper..."
cat > "${RUNTIME_DIR}/run.sh" << 'EOF'
#!/bin/bash
# GeneralsX @build felipebraz 16/02/2026 - Wrapper script for runtime directory
# Sets LD_LIBRARY_PATH to find DXVK libraries

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Set LD_LIBRARY_PATH to current directory (where DXVK libs are)
export LD_LIBRARY_PATH="${SCRIPT_DIR}:${LD_LIBRARY_PATH:-}"

# Set DXVK environment
export DXVK_WSI_DRIVER="SDL3"
export DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-info}"
export DXVK_HUD="${DXVK_HUD:-0}"

# GeneralsX @feature felipebraz 25/02/2026 Auto-detect base Generals install path
# Set CNC_GENERALS_INSTALLPATH if not already set and ../Generals/ exists
if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" && -d "${SCRIPT_DIR}/../Generals" ]]; then
    export CNC_GENERALS_INSTALLPATH="${SCRIPT_DIR}/../Generals/"
fi

# Run game with all arguments
exec "${SCRIPT_DIR}/GeneralsXZH" "$@"
EOF
chmod +x "${RUNTIME_DIR}/run.sh"

echo ""
echo "Deploy complete"
echo "   Executable: ${RUNTIME_DIR}/GeneralsXZH"
echo "   Libraries:  ${RUNTIME_DIR}/libdxvk_*.so*"
echo "   SDL3 libs:  ${RUNTIME_DIR}/libSDL3*.so* + ${RUNTIME_DIR}/libSDL3_image*.so*"
echo "   GameSpy:    ${RUNTIME_DIR}/libgamespy.so"
echo "   Wrapper:    ${RUNTIME_DIR}/run.sh"
echo ""
echo "Run with:"
echo "  cd ~/GeneralsX/GeneralsMD && ./run.sh -win"
echo "  or"
echo "  ${PROJECT_ROOT}/scripts/run-linux-zh.sh -win"
