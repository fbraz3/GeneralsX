#!/bin/bash
# GeneralsX @build felipebraz 16/02/2026 Deploy Linux build to runtime directory

set -e

# Directories
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/linux64-deploy"
DXVK_LIB_DIR="${BUILD_DIR}/_deps/dxvk-src/lib"
RUNTIME_DIR="${HOME}/GeneralsX/GeneralsMD"
BINARY_SRC="${BUILD_DIR}/GeneralsMD/GeneralsXZH"

echo "📦 Deploying GeneralsXZH (Linux) to ${RUNTIME_DIR}"

# Check if binary exists
if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "❌ ERROR: Binary not found at ${BINARY_SRC}"
    echo "Build first: ./scripts/docker-build-linux-zh.sh linux64-deploy"
    exit 1
fi

# Check if DXVK libraries exist
if [[ ! -d "${DXVK_LIB_DIR}" ]]; then
    echo "❌ ERROR: DXVK libraries not found at ${DXVK_LIB_DIR}"
    echo "Configure first: ./scripts/docker-configure-linux.sh linux64-deploy"
    exit 1
fi

# Create runtime directory if needed
mkdir -p "${RUNTIME_DIR}"

# Copy executable
echo "  → Copying GeneralsXZH..."
cp -v "${BINARY_SRC}" "${RUNTIME_DIR}/GeneralsXZH"
chmod +x "${RUNTIME_DIR}/GeneralsXZH"

# Copy DXVK libraries
echo "  → Copying DXVK libraries..."
cp -v "${DXVK_LIB_DIR}"/libdxvk_d3d8.so* "${RUNTIME_DIR}/"
cp -v "${DXVK_LIB_DIR}"/libdxvk_d3d9.so* "${RUNTIME_DIR}/" 2>/dev/null || true

# Set RPATH so executable finds libraries in same directory
echo "  → Setting RPATH to \$ORIGIN..."
patchelf --set-rpath '$ORIGIN' "${RUNTIME_DIR}/GeneralsXZH" 2>/dev/null || {
    echo "⚠️  WARNING: patchelf not found. Install with: sudo apt install patchelf"
    echo "    Libraries will need LD_LIBRARY_PATH or manual RPATH setting"
}

# Copy run wrapper script
echo "  → Copying run.sh wrapper..."
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

# Run game with all arguments
exec "${SCRIPT_DIR}/GeneralsXZH" "$@"
EOF
chmod +x "${RUNTIME_DIR}/run.sh"

echo ""
echo "✅ Deploy complete!"
echo "   Executable: ${RUNTIME_DIR}/GeneralsXZH"
echo "   Libraries:  ${RUNTIME_DIR}/libdxvk_*.so*"
echo "   Wrapper:    ${RUNTIME_DIR}/run.sh"
echo ""
echo "Run with:"
echo "  cd ~/GeneralsX/GeneralsMD && ./run.sh -win"
echo "  or"
echo "  ${PROJECT_ROOT}/scripts/run-linux-zh.sh -win"
