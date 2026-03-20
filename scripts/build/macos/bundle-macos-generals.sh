#!/bin/bash
# GeneralsX @build Copilot 19/03/2026 Bundle macOS GeneralsX binary + dylibs into a zip archive
# Packages a self-contained .app for local distribution testing.

set -e

# GeneralsX @build Copilot 19/03/2026 Resolve repository root correctly from scripts/build/macos.
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
BINARY_SRC="${BUILD_DIR}/Generals/GeneralsX"
DXVK_CONF_SRC="${PROJECT_ROOT}/resources/dxvk/dxvk.conf"
OUTPUT_ZIP="${PROJECT_ROOT}/GeneralsX-macos-arm64.zip"

DXVK_D3D8_LIB="${DXVK_D3D8_LIB_INSTALL}"
DXVK_D3D9_LIB="${DXVK_D3D9_LIB_INSTALL}"
if [[ ! -f "${DXVK_D3D8_LIB}" && -f "${DXVK_D3D8_LIB_MESON}" ]]; then
    DXVK_D3D8_LIB="${DXVK_D3D8_LIB_MESON}"
fi
if [[ ! -f "${DXVK_D3D9_LIB}" && -f "${DXVK_D3D9_LIB_MESON}" ]]; then
    DXVK_D3D9_LIB="${DXVK_D3D9_LIB_MESON}"
fi

# GeneralsX @bugfix Copilot 19/03/2026 Resolve @rpath/@loader_path deps while collecting external dylibs for portable bundles.
extract_rpaths() {
    local target="$1"
    otool -l "${target}" 2>/dev/null | awk '
        $1 == "cmd" && $2 == "LC_RPATH" { in_rpath = 1; next }
        in_rpath && $1 == "path" { print $2; in_rpath = 0 }
    '
}

resolve_dep_path() {
    local target="$1"
    local dep="$2"
    local executable_dir="$3"
    local loader_dir dep_name rpath candidate

    loader_dir="$(dirname "${target}")"

    if [[ "${dep}" == /* ]]; then
        [[ -f "${dep}" ]] && { echo "${dep}"; return 0; }
        return 1
    fi

    if [[ "${dep}" == @loader_path/* ]]; then
        candidate="${loader_dir}/${dep#@loader_path/}"
        [[ -f "${candidate}" ]] && { echo "${candidate}"; return 0; }
        return 1
    fi

    if [[ "${dep}" == @executable_path/* ]]; then
        candidate="${executable_dir}/${dep#@executable_path/}"
        [[ -f "${candidate}" ]] && { echo "${candidate}"; return 0; }
        return 1
    fi

    if [[ "${dep}" == @rpath/* ]]; then
        dep_name="${dep#@rpath/}"
        while IFS= read -r rpath; do
            [[ -z "${rpath}" ]] && continue
            rpath="${rpath//@loader_path/${loader_dir}}"
            rpath="${rpath//@executable_path/${executable_dir}}"
            candidate="${rpath}/${dep_name}"
            [[ -f "${candidate}" ]] && { echo "${candidate}"; return 0; }
        done < <(extract_rpaths "${target}")

        for candidate in \
            "/opt/homebrew/lib/${dep_name}" \
            "/usr/local/lib/${dep_name}"; do
            [[ -f "${candidate}" ]] && { echo "${candidate}"; return 0; }
        done

        if command -v brew >/dev/null 2>&1; then
            local brew_prefix
            brew_prefix="$(brew --prefix 2>/dev/null || true)"
            if [[ -n "${brew_prefix}" ]]; then
                for candidate in \
                    "${brew_prefix}/lib/${dep_name}" \
                    "${brew_prefix}/opt/webp/lib/${dep_name}"; do
                    [[ -f "${candidate}" ]] && { echo "${candidate}"; return 0; }
                done
            fi
        fi
    fi

    return 1
}

collect_external_dylibs() {
    local lib_dir="$1"
    shift
    local roots=("$@")
    local executable_dir

    executable_dir="$(dirname "${roots[0]}")"

    if ! command -v otool >/dev/null 2>&1; then
        echo "WARNING: otool not available; skipping external dylib collection"
        return 0
    fi

    local pending_file processed_file
    pending_file="${STAGE_DIR}/.bundle_pending.txt"
    processed_file="${STAGE_DIR}/.bundle_processed.txt"
    : > "${pending_file}"
    : > "${processed_file}"

    local root
    for root in "${roots[@]}"; do
        if [[ -f "${root}" ]]; then
            echo "${root}" >> "${pending_file}"
        fi
    done

    while [[ -s "${pending_file}" ]]; do
        local target
        target="$(head -n 1 "${pending_file}")"
        tail -n +2 "${pending_file}" > "${pending_file}.next" && mv "${pending_file}.next" "${pending_file}"

        if grep -Fqx "${target}" "${processed_file}"; then
            continue
        fi
        echo "${target}" >> "${processed_file}"

        local dep resolved_dep
        while IFS= read -r dep; do
            [[ -z "${dep}" ]] && continue
            resolved_dep="$(resolve_dep_path "${target}" "${dep}" "${executable_dir}" || true)"
            [[ -z "${resolved_dep}" ]] && continue
            [[ "${resolved_dep}" == /System/Library/* ]] && continue
            [[ "${resolved_dep}" == /usr/lib/* ]] && continue
            [[ ! -f "${resolved_dep}" ]] && continue

            local dep_name dep_dst
            dep_name="$(basename "${resolved_dep}")"
            dep_dst="${lib_dir}/${dep_name}"

            if [[ ! -f "${dep_dst}" ]]; then
                echo "  + external ${dep_name} (from ${resolved_dep})"
                cp "${resolved_dep}" "${dep_dst}"
            fi

            if ! grep -Fqx "${dep_dst}" "${processed_file}" && ! grep -Fqx "${dep_dst}" "${pending_file}"; then
                echo "${dep_dst}" >> "${pending_file}"
            fi
        done < <(otool -L "${target}" 2>/dev/null | awk 'NR>1 {print $1}')
    done
}

# Locate installed Vulkan SDK (tries ~/VulkanSDK/ by convention)
VULKAN_SDK_ROOT=""
for sdk_candidate in "${HOME}/VulkanSDK"/*/macOS; do
    if [[ -f "${sdk_candidate}/lib/libvulkan.dylib" ]]; then
        VULKAN_SDK_ROOT="${sdk_candidate}"
    fi
done

# GeneralsX @feature Copilot 19/03/2026 Produce a macOS .app bundle with internal runtime env defaults.
APP_NAME="GeneralsX"
APP_DIR_NAME="${APP_NAME}.app"
INCLUDE_EXTERNAL_DYLIBS="${GX_BUNDLE_INCLUDE_EXTERNAL_DYLIBS:-1}"

echo "Bundling ${APP_NAME} (macOS ARM64)"

# Validate binary
if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary not found at ${BINARY_SRC}"
    echo "Build first: ./scripts/build/macos/build-macos-generals.sh"
    exit 1
fi
if [[ ! -s "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary at ${BINARY_SRC} is empty - build may have failed"
    exit 1
fi

# Prepare temp staging directory
STAGE_DIR="$(mktemp -d)"
trap 'rm -rf "${STAGE_DIR}"' EXIT
APP_DIR="${STAGE_DIR}/${APP_DIR_NAME}"
CONTENTS_DIR="${APP_DIR}/Contents"
MACOS_DIR="${CONTENTS_DIR}/MacOS"
RESOURCES_DIR="${CONTENTS_DIR}/Resources"
BIN_DIR="${RESOURCES_DIR}/bin"
LIB_DIR="${RESOURCES_DIR}/lib"
mkdir -p "${MACOS_DIR}" "${BIN_DIR}" "${LIB_DIR}"

# Info.plist
cat > "${CONTENTS_DIR}/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>GeneralsX</string>
    <key>CFBundleDisplayName</key>
    <string>GeneralsX</string>
    <key>CFBundleIdentifier</key>
    <string>com.generalsx.generalsx</string>
    <key>CFBundleVersion</key>
    <string>0.1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>0.1.0</string>
    <key>CFBundleExecutable</key>
    <string>run.sh</string>
    <key>CFBundleIconFile</key>
    <string>generalsx_icon.png</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>LSMinimumSystemVersion</key>
    <string>15.0</string>
</dict>
</plist>
PLIST

echo "  Staging files to ${APP_DIR}..."

# Binary
echo "  + GeneralsX"
cp "${BINARY_SRC}" "${BIN_DIR}/GeneralsX"
chmod +x "${BIN_DIR}/GeneralsX"

# Icon
echo "  + Icon (generalsx_icon.png)"
cp "${PROJECT_ROOT}/assets/generalsx_icon.png" "${RESOURCES_DIR}/"

# SDL3
echo "  + libSDL3"
cp "${SDL3_LIB_DIR}"/libSDL3.0.dylib "${LIB_DIR}/"
ln -sf libSDL3.0.dylib "${LIB_DIR}/libSDL3.dylib"

echo "  + libSDL3_image"
cp "${SDL3_IMAGE_LIB_DIR}"/libSDL3_image.0.4.0.dylib "${LIB_DIR}/"
ln -sf libSDL3_image.0.4.0.dylib "${LIB_DIR}/libSDL3_image.0.dylib"
ln -sf libSDL3_image.0.4.0.dylib "${LIB_DIR}/libSDL3_image.dylib"

# GameSpy
echo "  + libgamespy"
cp "${GAMESPY_LIB}" "${LIB_DIR}/"

# DXVK
if [[ ! -f "${DXVK_D3D9_LIB}" || ! -f "${DXVK_D3D8_LIB}" ]]; then
    echo "ERROR: Required DXVK dylibs were not found in expected locations:"
    echo "  d3d9 install: ${DXVK_D3D9_LIB_INSTALL}"
    echo "  d3d8 install: ${DXVK_D3D8_LIB_INSTALL}"
    echo "  d3d9 meson:   ${DXVK_D3D9_LIB_MESON}"
    echo "  d3d8 meson:   ${DXVK_D3D8_LIB_MESON}"
    echo "Build DXVK first: cmake --build build/macos-vulkan --target dxvk_d3d8_install"
    exit 1
fi
echo "  + libdxvk_d3d9"
cp "${DXVK_D3D9_LIB}" "${LIB_DIR}/libdxvk_d3d9.0.dylib"
ln -sf libdxvk_d3d9.0.dylib "${LIB_DIR}/libdxvk_d3d9.dylib"
echo "  + libdxvk_d3d8"
cp "${DXVK_D3D8_LIB}" "${LIB_DIR}/libdxvk_d3d8.0.dylib"
ln -sf libdxvk_d3d8.0.dylib "${LIB_DIR}/libdxvk_d3d8.dylib"

if [[ "${INCLUDE_EXTERNAL_DYLIBS}" == "1" ]]; then
    echo "  + scanning for external dylibs (Homebrew/system extras)"
    collect_external_dylibs "${LIB_DIR}" \
        "${BIN_DIR}/GeneralsX" \
        "${LIB_DIR}/libSDL3.0.dylib" \
        "${LIB_DIR}/libSDL3_image.0.4.0.dylib" \
        "${LIB_DIR}/libgamespy.dylib" \
        "${LIB_DIR}/libdxvk_d3d8.0.dylib" \
        "${LIB_DIR}/libdxvk_d3d9.0.dylib"
else
    echo "  + skipping external dylib scan (GX_BUNDLE_INCLUDE_EXTERNAL_DYLIBS=${INCLUDE_EXTERNAL_DYLIBS})"
fi

# Vulkan + MoltenVK
if [[ -n "${VULKAN_SDK_ROOT}" ]]; then
    echo "  + libvulkan + libMoltenVK (from ${VULKAN_SDK_ROOT})"
    cp "${VULKAN_SDK_ROOT}/lib/libvulkan.dylib" "${LIB_DIR}/"
    cp "${VULKAN_SDK_ROOT}/lib/libvulkan.1.dylib" "${LIB_DIR}/" 2>/dev/null || true
    cp "${VULKAN_SDK_ROOT}/lib/libMoltenVK.dylib" "${LIB_DIR}/"
    cat > "${RESOURCES_DIR}/MoltenVK_icd.json" <<'EOF'
{
    "file_format_version": "1.0.0",
    "ICD": {
        "library_path": "./lib/libMoltenVK.dylib",
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
    cp "${DXVK_CONF_SRC}" "${RESOURCES_DIR}/dxvk.conf"
else
    echo "WARNING: ${DXVK_CONF_SRC} not found - terrain shaders may fail on macOS"
fi

# App launcher wrapper
echo "  + App launcher"
cat > "${MACOS_DIR}/run.sh" << 'WRAPPER'
#!/bin/bash
# GeneralsX @feature Copilot 19/03/2026 - App launcher with bundled runtime + default asset paths
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONTENTS_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
RESOURCES_DIR="${CONTENTS_DIR}/Resources"
BIN_DIR="${RESOURCES_DIR}/bin"
LIB_DIR="${RESOURCES_DIR}/lib"

export DYLD_LIBRARY_PATH="${LIB_DIR}:${BIN_DIR}:${DYLD_LIBRARY_PATH:-}"

if [[ -f "${RESOURCES_DIR}/MoltenVK_icd.json" ]]; then
    export VK_ICD_FILENAMES="${RESOURCES_DIR}/MoltenVK_icd.json"
fi

# Default asset path for local/distribution testing (allow user override)
export CNC_GENERALS_PATH="${CNC_GENERALS_PATH:-${HOME}/Generalsx/data/Generals}"

# Backward compatibility for existing runtime readers
if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" ]]; then
    export CNC_GENERALS_INSTALLPATH="${CNC_GENERALS_PATH}"
fi

if [[ -f "${RESOURCES_DIR}/dxvk.conf" ]]; then
    export DXVK_CONFIG_FILE="${RESOURCES_DIR}/dxvk.conf"
fi

# Run from the detected Generals asset root when available.
if [[ -d "${CNC_GENERALS_PATH}" ]]; then
    cd "${CNC_GENERALS_PATH}"
fi

exec "${BIN_DIR}/GeneralsX" "$@"
WRAPPER
chmod +x "${MACOS_DIR}/run.sh"

# Keep compatibility with direct calls using the game name.
ln -sf run.sh "${MACOS_DIR}/GeneralsX"

# Helper CLI launcher for terminal-based local tests.
echo "  + run.sh"
cat > "${STAGE_DIR}/run.sh" << 'RUNNER'
#!/bin/bash
# GeneralsX @feature Copilot 19/03/2026 Helper runner for launching the generated .app from terminal.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "${SCRIPT_DIR}/GeneralsX.app/Contents/MacOS/run.sh" "$@"
RUNNER
chmod +x "${STAGE_DIR}/run.sh"

# Create zip
echo ""
echo "Creating ${OUTPUT_ZIP}..."
rm -f "${OUTPUT_ZIP}"
(cd "${STAGE_DIR}" && zip -r "${OUTPUT_ZIP}" "${APP_DIR_NAME}" run.sh)

echo ""
echo "Bundle complete: ${OUTPUT_ZIP}"
echo "Contents:"
unzip -l "${OUTPUT_ZIP}" | sed '1,3d;$d'
echo ""
echo "To use locally:"
echo "  1) unzip ${OUTPUT_ZIP}"
echo "  2) run: ./run.sh -win"
echo "  3) or open: open ${APP_DIR_NAME}"
echo ""
echo "Runtime env defaults inside app launcher:"
echo '  CNC_GENERALS_PATH=$HOME/Generalsx/data/Generals'
