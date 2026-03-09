#!/bin/bash
# GeneralsX @build BenderAI 02/03/2026 Bundle macOS GeneralsXZH binary + dylibs into a zip archive
# Packages the same files as deploy-macos-zh.sh into GeneralsXZH-macos-arm64.zip

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/macos-vulkan"
SDL3_LIB_DIR="${BUILD_DIR}/_deps/sdl3-build"
SDL3_IMAGE_LIB_DIR="${BUILD_DIR}/_deps/sdl3_image-build"
GAMESPY_LIB="${BUILD_DIR}/libgamespy.dylib"
DXVK_D3D8_LIB="${BUILD_DIR}/libdxvk_d3d8.0.dylib"
DXVK_D3D9_LIB="${BUILD_DIR}/libdxvk_d3d9.0.dylib"
BINARY_SRC="${BUILD_DIR}/GeneralsMD/GeneralsXZH"
DXVK_CONF_SRC="${PROJECT_ROOT}/GeneralsMD/Run/dxvk.conf"
OUTPUT_ZIP="${PROJECT_ROOT}/GeneralsXZH-macos-arm64.zip"

# Locate installed Vulkan SDK (tries ~/VulkanSDK/ by convention)
VULKAN_SDK_ROOT=""
for sdk_candidate in "${HOME}/VulkanSDK"/*/macOS; do
    if [[ -f "${sdk_candidate}/lib/libvulkan.dylib" ]]; then
        VULKAN_SDK_ROOT="${sdk_candidate}"
    fi
done

echo "Bundling GeneralsXZH (macOS ARM64)"

# Validate binary
if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary not found at ${BINARY_SRC}"
    echo "Build first: ./scripts/build-macos-zh.sh"
    exit 1
fi
if [[ ! -s "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary at ${BINARY_SRC} is empty - build may have failed"
    exit 1
fi

# Prepare temp staging directory
STAGE_DIR="$(mktemp -d)"
trap 'rm -rf "${STAGE_DIR}"' EXIT
BUNDLE_DIR="${STAGE_DIR}/GeneralsXZH-macos"
mkdir -p "${BUNDLE_DIR}"

echo "  Staging files to ${BUNDLE_DIR}..."

# Binary
echo "  + GeneralsXZH"
cp "${BINARY_SRC}" "${BUNDLE_DIR}/GeneralsXZH"
chmod +x "${BUNDLE_DIR}/GeneralsXZH"

# SDL3
echo "  + libSDL3"
cp "${SDL3_LIB_DIR}"/libSDL3.0.dylib "${BUNDLE_DIR}/"
ln -sf libSDL3.0.dylib "${BUNDLE_DIR}/libSDL3.dylib"

echo "  + libSDL3_image"
cp "${SDL3_IMAGE_LIB_DIR}"/libSDL3_image.0.4.0.dylib "${BUNDLE_DIR}/"
ln -sf libSDL3_image.0.4.0.dylib "${BUNDLE_DIR}/libSDL3_image.0.dylib"
ln -sf libSDL3_image.0.4.0.dylib "${BUNDLE_DIR}/libSDL3_image.dylib"

# GameSpy
echo "  + libgamespy"
cp "${GAMESPY_LIB}" "${BUNDLE_DIR}/"

# DXVK
if [[ -f "${DXVK_D3D9_LIB}" ]]; then
    echo "  + libdxvk_d3d9"
    cp "${DXVK_D3D9_LIB}" "${BUNDLE_DIR}/"
    ln -sf libdxvk_d3d9.0.dylib "${BUNDLE_DIR}/libdxvk_d3d9.dylib"
else
    echo "WARNING: ${DXVK_D3D9_LIB} not found - d3d8 will fail to load without it"
fi
if [[ -f "${DXVK_D3D8_LIB}" ]]; then
    echo "  + libdxvk_d3d8"
    cp "${DXVK_D3D8_LIB}" "${BUNDLE_DIR}/"
    ln -sf libdxvk_d3d8.0.dylib "${BUNDLE_DIR}/libdxvk_d3d8.dylib"
else
    echo "WARNING: ${DXVK_D3D8_LIB} not found - run cmake --build build/macos-vulkan --target dxvk_d3d8_install"
fi

# Vulkan + MoltenVK
if [[ -n "${VULKAN_SDK_ROOT}" ]]; then
    echo "  + libvulkan + libMoltenVK (from ${VULKAN_SDK_ROOT})"
    cp "${VULKAN_SDK_ROOT}/lib/libvulkan.dylib" "${BUNDLE_DIR}/"
    cp "${VULKAN_SDK_ROOT}/lib/libvulkan.1.dylib" "${BUNDLE_DIR}/" 2>/dev/null || true
    cp "${VULKAN_SDK_ROOT}/lib/libMoltenVK.dylib" "${BUNDLE_DIR}/"
    cat > "${BUNDLE_DIR}/MoltenVK_icd.json" <<'EOF'
{
    "file_format_version": "1.0.0",
    "ICD": {
        "library_path": "./libMoltenVK.dylib",
        "api_version": "1.4.0",
        "is_portability_driver": true
    }
}
EOF
else
    echo "WARNING: Vulkan SDK not found at ~/VulkanSDK/*/macOS - Vulkan libs will be missing from bundle"
    echo "  Install from: https://vulkan.lunarg.com/"
fi

# DXVK config
if [[ -f "${DXVK_CONF_SRC}" ]]; then
    echo "  + dxvk.conf"
    cp "${DXVK_CONF_SRC}" "${BUNDLE_DIR}/dxvk.conf"
else
    echo "WARNING: ${DXVK_CONF_SRC} not found - terrain shaders may fail on macOS"
fi

# Run wrapper
echo "  + run.sh"
cat > "${BUNDLE_DIR}/run.sh" << 'WRAPPER'
#!/bin/bash
# GeneralsX @build BenderAI 02/03/2026 - macOS wrapper for bundled runtime
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

export DYLD_LIBRARY_PATH="${SCRIPT_DIR}:${DYLD_LIBRARY_PATH:-}"

if [[ -f "${SCRIPT_DIR}/MoltenVK_icd.json" ]]; then
    export VK_ICD_FILENAMES="${SCRIPT_DIR}/MoltenVK_icd.json"
fi

if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" && -d "${SCRIPT_DIR}/../Generals" ]]; then
    export CNC_GENERALS_INSTALLPATH="${SCRIPT_DIR}/../Generals/"
fi

exec "${SCRIPT_DIR}/GeneralsXZH" "$@"
WRAPPER
chmod +x "${BUNDLE_DIR}/run.sh"

# Create zip
echo ""
echo "Creating ${OUTPUT_ZIP}..."
rm -f "${OUTPUT_ZIP}"
(cd "${STAGE_DIR}" && zip -r "${OUTPUT_ZIP}" GeneralsXZH-macos/)

echo ""
echo "Bundle complete: ${OUTPUT_ZIP}"
echo "Contents:"
unzip -l "${OUTPUT_ZIP}" | tail -n +4 | head -n -2
echo ""
echo "To use: extract the zip alongside your game data directory (GeneralsMD/)"
echo "  and run: ./GeneralsXZH-macos/run.sh -win"
