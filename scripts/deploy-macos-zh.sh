#!/bin/bash
# GeneralsX @build BenderAI 24/02/2026 Deploy macOS build to runtime directory
# Copies GeneralsXZH binary and required dylibs to ~/GeneralsX/GeneralsMD

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/macos-vulkan"
GAMESPY_LIB="${BUILD_DIR}/libgamespy.dylib"
RUNTIME_DIR="${HOME}/GeneralsX/GeneralsMD"

# Locate the installed Vulkan SDK (tries ~/VulkanSDK/ by convention)
VULKAN_SDK_ROOT=""
for sdk_candidate in "${HOME}/VulkanSDK"/*/macOS; do
    if [[ -f "${sdk_candidate}/lib/libvulkan.dylib" ]]; then
        VULKAN_SDK_ROOT="${sdk_candidate}"
    fi
done
BINARY_SRC="${BUILD_DIR}/GeneralsMD/GeneralsXZH"

echo "Deploying GeneralsXZH (macOS) to ${RUNTIME_DIR}"

if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary not found at ${BINARY_SRC}"
    echo "Build first: cmake --build build/macos-vulkan --target z_generals"
    exit 1
fi
if [[ ! -s "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary at ${BINARY_SRC} is empty - build may have failed"
    exit 1
fi

mkdir -p "${RUNTIME_DIR}"

echo "  Copying GeneralsXZH..."
cp -v "${BINARY_SRC}" "${RUNTIME_DIR}/GeneralsXZH"
chmod +x "${RUNTIME_DIR}/GeneralsXZH"

echo "  Copying dylibs from _deps (SDL3, DXVK, OpenAL, etc.)..."
# GeneralsX @bugfix felipebraz 05/03/2026 Use find+cp-L to copy all dylibs from _deps; -L dereferences symlinks.
find "${BUILD_DIR}/_deps" -name "*.dylib" 2>/dev/null | while IFS= read -r f; do
    cp -Lv "$f" "${RUNTIME_DIR}/" || true
done

echo "  Copying GameSpy library..."
cp -Lv "${GAMESPY_LIB}" "${RUNTIME_DIR}/"

echo "  Deploying Vulkan + MoltenVK libraries..."
if [[ -n "${VULKAN_SDK_ROOT}" ]]; then
    # Copy libvulkan loader (DXVK dlopen's "libvulkan.dylib" on macOS)
    cp -v "${VULKAN_SDK_ROOT}/lib/libvulkan.dylib" "${RUNTIME_DIR}/"
    cp -v "${VULKAN_SDK_ROOT}/lib/libvulkan.1.dylib" "${RUNTIME_DIR}/" 2>/dev/null || true
    # Copy MoltenVK ICD driver (provides vkGetInstanceProcAddr via libvulkan.dylib)
    cp -v "${VULKAN_SDK_ROOT}/lib/libMoltenVK.dylib" "${RUNTIME_DIR}/"
    # Write MoltenVK ICD manifest (Vulkan loader needs VK_ICD_FILENAMES to point here)
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
    echo "  DXVK will fail to find vkGetInstanceProcAddr at runtime."
fi

# Write wrapper run script that sets DYLD_LIBRARY_PATH at launch time
echo "  Deploying dxvk.conf..."
# GeneralsX @bugfix BenderAI 25/02/2026 Deploy DXVK configuration
# Forces forceSamplerTypeSpecConstants=True to fix terrain shader MSL generation bug.
DXVK_CONF_SRC="${PROJECT_ROOT}/dxvk.conf"
if [[ -f "${DXVK_CONF_SRC}" ]]; then
    cp -v "${DXVK_CONF_SRC}" "${RUNTIME_DIR}/dxvk.conf"
else
    echo "WARNING: ${DXVK_CONF_SRC} not found - terrain shaders may fail to compile on macOS."
fi

echo "  Writing run.sh wrapper..."
cat > "${RUNTIME_DIR}/run.sh" << WRAPPER
#!/bin/bash
# GeneralsX @build BenderAI 24/02/2026 - macOS wrapper for runtime directory
SCRIPT_DIR="\$(cd "\$(dirname "\$0")" && pwd)"

# SDL3 and gamespy dylibs are in same dir; Vulkan/MoltenVK stays in SDK
export DYLD_LIBRARY_PATH="\${SCRIPT_DIR}:\${DYLD_LIBRARY_PATH:-}"

# MoltenVK ICD manifest — deployed alongside the binary by deploy-macos-zh.sh
if [[ -f "\${SCRIPT_DIR}/MoltenVK_icd.json" ]]; then
    export VK_ICD_FILENAMES="\${SCRIPT_DIR}/MoltenVK_icd.json"
fi

# Auto-detect base Generals install path
if [[ -z "\${CNC_GENERALS_INSTALLPATH:-}" && -d "\${SCRIPT_DIR}/../Generals" ]]; then
    export CNC_GENERALS_INSTALLPATH="\${SCRIPT_DIR}/../Generals/"
fi

exec "\${SCRIPT_DIR}/GeneralsXZH" "\$@"
WRAPPER
chmod +x "${RUNTIME_DIR}/run.sh"

echo ""
echo "Deploy complete"
echo "   Executable: ${RUNTIME_DIR}/GeneralsXZH"
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
echo "Run with:"
echo "  ${PROJECT_ROOT}/scripts/run-macos-zh.sh -win"
echo "  or: cd ~/GeneralsX/GeneralsMD && ./run.sh -win"
