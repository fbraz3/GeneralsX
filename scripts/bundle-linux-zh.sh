#!/bin/bash
# GeneralsX @build BenderAI 03/03/2026 Bundle Linux GeneralsXZH binary + .so libs into a tarball archive
# Packages the same files as deploy-linux-zh.sh into GeneralsXZH-linux-x86_64.tar.gz

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/linux64-deploy"
DXVK_LIB_DIR="${BUILD_DIR}/_deps/dxvk-src/lib"
SDL3_LIB_DIR="${BUILD_DIR}/_deps/sdl3-build"
SDL3_IMAGE_LIB_DIR="${BUILD_DIR}/_deps/sdl3_image-build"
GAMESPY_LIB="${BUILD_DIR}/libgamespy.so"
BINARY_SRC="${BUILD_DIR}/GeneralsMD/GeneralsXZH"
DXVK_CONF_SRC="${PROJECT_ROOT}/GeneralsMD/Run/dxvk.conf"
OUTPUT_TARBALL="${PROJECT_ROOT}/GeneralsXZH-linux-x86_64.tar.gz"

echo "Bundling GeneralsXZH (Linux x86_64)"

# Validate binary
if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary not found at ${BINARY_SRC}"
    echo "Build first: ./scripts/docker-build-linux-zh.sh linux64-deploy"
    exit 1
fi
if [[ ! -s "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary at ${BINARY_SRC} is empty - build may have failed"
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

# Prepare temp staging directory
STAGE_DIR="$(mktemp -d)"
trap 'rm -rf "${STAGE_DIR}"' EXIT
BUNDLE_DIR="${STAGE_DIR}/GeneralsXZH-linux"
mkdir -p "${BUNDLE_DIR}"

echo "  Staging files to ${BUNDLE_DIR}..."

# Binary
echo "  + GeneralsXZH"
cp "${BINARY_SRC}" "${BUNDLE_DIR}/GeneralsXZH"
chmod +x "${BUNDLE_DIR}/GeneralsXZH"

# DXVK libraries
echo "  + DXVK libraries"
cp "${DXVK_LIB_DIR}"/libdxvk_d3d8.so* "${BUNDLE_DIR}/" 2>/dev/null || echo "    (WARNING: libdxvk_d3d8.so not found)"
cp "${DXVK_LIB_DIR}"/libdxvk_d3d9.so* "${BUNDLE_DIR}/" 2>/dev/null || true

# SDL3 and SDL3_image libraries
echo "  + SDL3 libraries"
cp "${SDL3_LIB_DIR}"/libSDL3.so* "${BUNDLE_DIR}/"
cp "${SDL3_IMAGE_LIB_DIR}"/libSDL3_image.so* "${BUNDLE_DIR}/"

# GameSpy library
echo "  + GameSpy library"
cp "${GAMESPY_LIB}" "${BUNDLE_DIR}/"

# DXVK config
if [[ -f "${DXVK_CONF_SRC}" ]]; then
    echo "  + dxvk.conf"
    cp "${DXVK_CONF_SRC}" "${BUNDLE_DIR}/dxvk.conf"
else
    echo "WARNING: ${DXVK_CONF_SRC} not found - terrain shaders may fail"
fi

# Run wrapper
echo "  + run.sh"
cat > "${BUNDLE_DIR}/run.sh" << 'WRAPPER'
#!/bin/bash
# GeneralsX @build BenderAI 03/03/2026 - Linux wrapper for bundled runtime
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Set LD_LIBRARY_PATH to find DXVK, SDL3, and other libs in same directory
export LD_LIBRARY_PATH="${SCRIPT_DIR}:${LD_LIBRARY_PATH:-}"

# Set DXVK environment
export DXVK_WSI_DRIVER="SDL3"
export DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-info}"
export DXVK_HUD="${DXVK_HUD:-0}"

# Auto-detect base Generals install path
if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" && -d "${SCRIPT_DIR}/../Generals" ]]; then
    export CNC_GENERALS_INSTALLPATH="${SCRIPT_DIR}/../Generals/"
fi

exec "${SCRIPT_DIR}/GeneralsXZH" "$@"
WRAPPER
chmod +x "${BUNDLE_DIR}/run.sh"

# Create tarball
echo ""
echo "Creating ${OUTPUT_TARBALL}..."
rm -f "${OUTPUT_TARBALL}"
(cd "${STAGE_DIR}" && tar -czf "${OUTPUT_TARBALL}" GeneralsXZH-linux/)

echo ""
echo "Bundle complete: ${OUTPUT_TARBALL}"
echo "Contents:"
tar -tzf "${OUTPUT_TARBALL}" | head -30
echo ""
echo "To use: extract alongside your game data directory (GeneralsMD/)"
echo "  tar -xzf GeneralsXZH-linux-x86_64.tar.gz"
echo "  ./GeneralsXZH-linux/run.sh -win"
