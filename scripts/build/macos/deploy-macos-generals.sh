#!/bin/bash
# GeneralsX @build BenderAI 15/03/2026 Deploy macOS base-game build to runtime directory
# Copies GeneralsX binary and required dylibs to ~/GeneralsX/Generals

set -e

# GeneralsX @build BenderAI 15/03/2026 Resolve repository root for macOS base deploy script.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/macos-vulkan"
SDL3_LIB_DIR="${BUILD_DIR}/_deps/sdl3-build"
SDL3_IMAGE_LIB_DIR="${BUILD_DIR}/_deps/sdl3_image-build"
GAMESPY_LIB="${BUILD_DIR}/libgamespy.dylib"
DXVK_D3D8_LIB_INSTALL="${BUILD_DIR}/libdxvk_d3d8.0.dylib"
DXVK_D3D9_LIB_INSTALL="${BUILD_DIR}/libdxvk_d3d9.0.dylib"
DXVK_D3D8_LIB_MESON="${BUILD_DIR}/_deps/dxvk-build-macos/src/d3d8/libdxvk_d3d8.0.dylib"
DXVK_D3D9_LIB_MESON="${BUILD_DIR}/_deps/dxvk-build-macos/src/d3d9/libdxvk_d3d9.0.dylib"
RUNTIME_DIR="${HOME}/GeneralsX/Generals"
BINARY_SRC="${BUILD_DIR}/Generals/GeneralsX"

VULKAN_SDK_ROOT=""
for sdk_candidate in "${HOME}/VulkanSDK"/*/macOS; do
    if [[ -f "${sdk_candidate}/lib/libvulkan.dylib" ]]; then
        VULKAN_SDK_ROOT="${sdk_candidate}"
    fi
done

DXVK_D3D8_LIB="${DXVK_D3D8_LIB_INSTALL}"
DXVK_D3D9_LIB="${DXVK_D3D9_LIB_INSTALL}"
if [[ ! -f "${DXVK_D3D8_LIB}" && -f "${DXVK_D3D8_LIB_MESON}" ]]; then
    DXVK_D3D8_LIB="${DXVK_D3D8_LIB_MESON}"
fi
if [[ ! -f "${DXVK_D3D9_LIB}" && -f "${DXVK_D3D9_LIB_MESON}" ]]; then
    DXVK_D3D9_LIB="${DXVK_D3D9_LIB_MESON}"
fi

echo "Deploying GeneralsX (macOS) to ${RUNTIME_DIR}"

if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary not found at ${BINARY_SRC}"
    echo "Build first: ./scripts/build/macos/build-macos-generals.sh"
    exit 1
fi
if [[ ! -s "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary at ${BINARY_SRC} is empty - build may have failed"
    exit 1
fi

mkdir -p "${RUNTIME_DIR}"

echo "  Copying GeneralsX..."
cp -v "${BINARY_SRC}" "${RUNTIME_DIR}/GeneralsX"
chmod +x "${RUNTIME_DIR}/GeneralsX"

echo "  Copying SDL3 libraries..."
cp -v "${SDL3_LIB_DIR}"/libSDL3.0.dylib "${RUNTIME_DIR}/"
ln -sf libSDL3.0.dylib "${RUNTIME_DIR}/libSDL3.dylib" 2>/dev/null || true
cp -v "${SDL3_IMAGE_LIB_DIR}"/libSDL3_image.0.4.0.dylib "${RUNTIME_DIR}/"
ln -sf libSDL3_image.0.4.0.dylib "${RUNTIME_DIR}/libSDL3_image.0.dylib" 2>/dev/null || true
ln -sf libSDL3_image.0.4.0.dylib "${RUNTIME_DIR}/libSDL3_image.dylib" 2>/dev/null || true

echo "  Copying GameSpy library..."
cp -v "${GAMESPY_LIB}" "${RUNTIME_DIR}/"

echo "  Copying DXVK libraries (d3d9 + d3d8)..."
if [[ ! -f "${DXVK_D3D9_LIB}" || ! -f "${DXVK_D3D8_LIB}" ]]; then
    echo "ERROR: Required DXVK dylibs were not found in expected locations:"
    echo "  d3d9 install: ${DXVK_D3D9_LIB_INSTALL}"
    echo "  d3d8 install: ${DXVK_D3D8_LIB_INSTALL}"
    echo "  d3d9 meson:   ${DXVK_D3D9_LIB_MESON}"
    echo "  d3d8 meson:   ${DXVK_D3D8_LIB_MESON}"
    echo "Build DXVK first: cmake --build build/macos-vulkan --target dxvk_d3d8_install"
    exit 1
fi
cp -v "${DXVK_D3D9_LIB}" "${RUNTIME_DIR}/libdxvk_d3d9.0.dylib"
ln -sf libdxvk_d3d9.0.dylib "${RUNTIME_DIR}/libdxvk_d3d9.dylib" 2>/dev/null || true
cp -v "${DXVK_D3D8_LIB}" "${RUNTIME_DIR}/libdxvk_d3d8.0.dylib"
ln -sf libdxvk_d3d8.0.dylib "${RUNTIME_DIR}/libdxvk_d3d8.dylib" 2>/dev/null || true

echo "  Deploying Vulkan + MoltenVK libraries..."
if [[ -n "${VULKAN_SDK_ROOT}" ]]; then
    cp -v "${VULKAN_SDK_ROOT}/lib/libvulkan.dylib" "${RUNTIME_DIR}/"
    cp -v "${VULKAN_SDK_ROOT}/lib/libvulkan.1.dylib" "${RUNTIME_DIR}/" 2>/dev/null || true
    cp -v "${VULKAN_SDK_ROOT}/lib/libMoltenVK.dylib" "${RUNTIME_DIR}/"
    cat > "${RUNTIME_DIR}/MoltenVK_icd.json" <<'EOF'
{
    "file_format_version": "1.0.0",
    "ICD": {
        "library_path": "./libMoltenVK.dylib",
        "api_version": "1.4.0",
        "is_portability_driver": true
    }
}
EOF
    echo "  Vulkan SDK libs deployed from: ${VULKAN_SDK_ROOT}"
else
    echo "WARNING: Vulkan SDK not found at ~/VulkanSDK/*/macOS."
    echo "  Install the Vulkan SDK from https://vulkan.lunarg.com/"
fi

echo "  Deploying dxvk.conf..."
DXVK_CONF_SRC="${PROJECT_ROOT}/resources/dxvk/dxvk.conf"
if [[ -f "${DXVK_CONF_SRC}" ]]; then
    cp -v "${DXVK_CONF_SRC}" "${RUNTIME_DIR}/dxvk.conf"
else
    echo "WARNING: ${DXVK_CONF_SRC} not found; DXVK will use defaults."
fi

echo "  Writing run.sh wrapper..."
cat > "${RUNTIME_DIR}/run.sh" << 'WRAPPER'
#!/bin/bash
# GeneralsX @build BenderAI 15/03/2026 - macOS wrapper for base-game runtime directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

export DYLD_LIBRARY_PATH="${SCRIPT_DIR}:${DYLD_LIBRARY_PATH:-}"

# GeneralsX @bugfix fbraz3 20/03/2026 DXVK requires DXVK_WSI_DRIVER on non-Win32; must match game windowing (SDL3)
export DXVK_WSI_DRIVER="SDL3"

if [[ -f "${SCRIPT_DIR}/MoltenVK_icd.json" ]]; then
    export VK_ICD_FILENAMES="${SCRIPT_DIR}/MoltenVK_icd.json"
    # GeneralsX @bugfix fbraz3 20/03/2026 Vulkan Loader 1.3.236+ uses VK_DRIVER_FILES; keep VK_ICD_FILENAMES for older loaders
    export VK_DRIVER_FILES="${SCRIPT_DIR}/MoltenVK_icd.json"
fi

exec "${SCRIPT_DIR}/GeneralsX" "$@"
WRAPPER
chmod +x "${RUNTIME_DIR}/run.sh"

echo ""
echo "Deploy complete"
echo "   Executable: ${RUNTIME_DIR}/GeneralsX"
echo "   SDL3 libs:  ${RUNTIME_DIR}/libSDL3*.dylib"
echo "   GameSpy:    ${RUNTIME_DIR}/libgamespy.dylib"
echo "   DXVK d3d9:  ${RUNTIME_DIR}/libdxvk_d3d9.0.dylib"
echo "   DXVK d3d8:  ${RUNTIME_DIR}/libdxvk_d3d8.0.dylib"
echo "   Vulkan:     ${RUNTIME_DIR}/libvulkan.dylib"
echo "   MoltenVK:   ${RUNTIME_DIR}/libMoltenVK.dylib"
echo "   VK ICD:     ${RUNTIME_DIR}/MoltenVK_icd.json"
echo "   DXVK conf:  ${RUNTIME_DIR}/dxvk.conf"
echo "   Wrapper:    ${RUNTIME_DIR}/run.sh"
echo ""
# GeneralsX @tweak BenderAI 28/04/2026 Print post-deploy run instructions for base game like Zero Hour deploy flow.
echo "Run with:"
echo "  cd ~/GeneralsX/Generals && ./run.sh -win"
echo "  or: ${RUNTIME_DIR}/run.sh -win"
