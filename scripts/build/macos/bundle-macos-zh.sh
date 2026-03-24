#!/bin/bash
# GeneralsX @build BenderAI 02/03/2026 Bundle macOS GeneralsXZH binary + dylibs into a zip archive
# Packages a self-contained .app for local distribution testing.

set -e

# GeneralsX @bugfix BenderAI 09/03/2026 Resolve repository root correctly from scripts/build/macos.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/macos-vulkan"
SDL3_LIB_DIR="${BUILD_DIR}/_deps/sdl3-build"
SDL3_IMAGE_LIB_DIR="${BUILD_DIR}/_deps/sdl3_image-build"
FONTCONFIG_ETC_DIR="${BUILD_DIR}/vcpkg_installed/arm64-osx/etc/fonts"
GAMESPY_LIB="${BUILD_DIR}/libgamespy.dylib"
# GeneralsX @bugfix BenderAI 09/03/2026 Resolve DXVK dylib paths from both install copy and Meson output to avoid stale/incomplete bundles.
DXVK_D3D8_LIB_INSTALL="${BUILD_DIR}/libdxvk_d3d8.0.dylib"
DXVK_D3D9_LIB_INSTALL="${BUILD_DIR}/libdxvk_d3d9.0.dylib"
DXVK_D3D8_LIB_MESON="${BUILD_DIR}/_deps/dxvk-build-macos/src/d3d8/libdxvk_d3d8.0.dylib"
DXVK_D3D9_LIB_MESON="${BUILD_DIR}/_deps/dxvk-build-macos/src/d3d9/libdxvk_d3d9.0.dylib"
BINARY_SRC="${BUILD_DIR}/GeneralsMD/GeneralsXZH"
DXVK_CONF_SRC="${PROJECT_ROOT}/resources/dxvk/dxvk.conf"
OUTPUT_ZIP="${PROJECT_ROOT}/GeneralsXZH-macos-arm64.zip"

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

        # GeneralsX @bugfix Copilot 24/03/2026 Resolve @rpath deps already staged in the same lib directory before scanning LC_RPATH.
        candidate="${loader_dir}/${dep_name}"
        [[ -f "${candidate}" ]] && { echo "${candidate}"; return 0; }

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
            # Skip known system libraries based on the raw otool entry before resolution.
            if [[ "${dep}" == /System/Library/* ]] || [[ "${dep}" == /usr/lib/* ]]; then
                continue
            fi
            resolved_dep="$(resolve_dep_path "${target}" "${dep}" "${executable_dir}" || true)"
            if [[ -z "${resolved_dep}" ]]; then
                # GeneralsX @bugfix Copilot 20/03/2026 Warn when a non-system dep cannot be resolved instead of silently dropping it.
                echo "WARNING: Unable to resolve dependency '${dep}' for target '${target}'" >&2
                continue
            fi
            [[ ! -f "${resolved_dep}" ]] && continue

            local dep_name dep_dst
            dep_name="$(basename "${resolved_dep}")"
            dep_dst="${lib_dir}/${dep_name}"

            if [[ ! -f "${dep_dst}" ]]; then
                echo "  + external ${dep_name} (from ${resolved_dep})"
                # GeneralsX @bugfix Copilot 20/03/2026 Dereference symlinks so we copy the actual dylib, not a dangling symlink.
                cp -L "${resolved_dep}" "${dep_dst}"
            fi

            if ! grep -Fqx "${dep_dst}" "${processed_file}" && ! grep -Fqx "${dep_dst}" "${pending_file}"; then
                echo "${dep_dst}" >> "${pending_file}"
            fi
        done < <(otool -L "${target}" 2>/dev/null | awk 'NR>1 {print $1}')
    done
}

# Locate installed Vulkan SDK:
#  1) Prefer explicit env vars (VULKAN_SDK, then VULKAN_SDK_ROOT)
#  2) Fall back to ~/VulkanSDK/*/macOS by convention
# GeneralsX @bugfix Copilot 20/03/2026 Honor VULKAN_SDK/VULKAN_SDK_ROOT before scanning ~/VulkanSDK.
if [[ -n "${VULKAN_SDK:-}" ]] && [[ -f "${VULKAN_SDK}/lib/libvulkan.dylib" ]]; then
    VULKAN_SDK_ROOT="${VULKAN_SDK}"
elif [[ -n "${VULKAN_SDK_ROOT:-}" ]] && [[ -f "${VULKAN_SDK_ROOT}/lib/libvulkan.dylib" ]]; then
    : # pre-exported VULKAN_SDK_ROOT is valid; keep as-is
else
    VULKAN_SDK_ROOT=""
    for sdk_candidate in "${HOME}/VulkanSDK"/*/macOS; do
        if [[ -f "${sdk_candidate}/lib/libvulkan.dylib" ]]; then
            VULKAN_SDK_ROOT="${sdk_candidate}"
        fi
    done
fi

# GeneralsX @feature Copilot 19/03/2026 Produce a macOS .app bundle with internal runtime env defaults.
APP_NAME="GeneralsXZH"
APP_DIR_NAME="${APP_NAME}.app"
INCLUDE_EXTERNAL_DYLIBS="${GX_BUNDLE_INCLUDE_EXTERNAL_DYLIBS:-1}"

echo "Bundling ${APP_NAME} (macOS ARM64)"

# Validate binary
if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "ERROR: Binary not found at ${BINARY_SRC}"
    echo "Build first: ./scripts/build/macos/build-macos-zh.sh"
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
    <string>GeneralsXZH</string>
    <key>CFBundleDisplayName</key>
    <string>GeneralsXZH</string>
    <key>CFBundleIdentifier</key>
    <string>com.generalsx.generalsxzh</string>
    <key>CFBundleVersion</key>
    <string>0.1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>0.1.0</string>
    <key>CFBundleExecutable</key>
    <string>run.sh</string>
    <key>CFBundleIconFile</key>
    <string>generalsx-zh_icon.png</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>LSMinimumSystemVersion</key>
    <string>15.0</string>
</dict>
</plist>
PLIST

echo "  Staging files to ${APP_DIR}..."

# Binary
echo "  + GeneralsXZH"
cp "${BINARY_SRC}" "${BIN_DIR}/GeneralsXZH"
chmod +x "${BIN_DIR}/GeneralsXZH"

# Icon
echo "  + Icon (generalsx-zh_icon.png)"
cp "${PROJECT_ROOT}/assets/generalsx-zh_icon.png" "${RESOURCES_DIR}/"

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
        "${BIN_DIR}/GeneralsXZH" \
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
    # GeneralsX @build Copilot 20/03/2026 Fail bundle when Vulkan SDK is missing unless explicitly allowed.
    if [[ "${GX_ALLOW_MISSING_VULKAN_IN_BUNDLE:-0}" == "1" ]]; then
        echo "WARNING: Vulkan SDK not found at ~/VulkanSDK/*/macOS - Vulkan libs will be missing from bundle"
        echo "  GX_ALLOW_MISSING_VULKAN_IN_BUNDLE=1 set; producing bundle without Vulkan for local experiments"
        echo "  Note: This .app/.zip may be non-functional on machines without a system Vulkan/MoltenVK install."
    else
        echo "ERROR: Vulkan SDK not found at ~/VulkanSDK/*/macOS - Vulkan libs cannot be copied into bundle" >&2
        echo "  Install from: https://vulkan.lunarg.com/ and ensure VULKAN_SDK_ROOT is set," >&2
        echo "  or set GX_ALLOW_MISSING_VULKAN_IN_BUNDLE=1 to intentionally produce a non-Vulkan bundle." >&2
        exit 1
    fi
fi

# DXVK config
if [[ -f "${DXVK_CONF_SRC}" ]]; then
    echo "  + dxvk.conf"
    cp "${DXVK_CONF_SRC}" "${RESOURCES_DIR}/dxvk.conf"
else
    echo "WARNING: ${DXVK_CONF_SRC} not found - terrain shaders may fail on macOS"
fi

# GeneralsX @bugfix Copilot 24/03/2026 Bundle Fontconfig config so FreeType/Fontconfig font matching works in app launcher runtime.
if [[ -f "${FONTCONFIG_ETC_DIR}/fonts.conf" ]]; then
    echo "  + Fontconfig config"
    mkdir -p "${RESOURCES_DIR}/fontconfig"
    cp "${FONTCONFIG_ETC_DIR}/fonts.conf" "${RESOURCES_DIR}/fontconfig/fonts.conf"
    rm -rf "${RESOURCES_DIR}/fontconfig/conf.d"
    cp -R "${FONTCONFIG_ETC_DIR}/conf.d" "${RESOURCES_DIR}/fontconfig/conf.d"
else
    echo "WARNING: ${FONTCONFIG_ETC_DIR}/fonts.conf not found - in-game font lookup may fail on macOS"
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

# GeneralsX @bugfix fbraz3 20/03/2026 DXVK requires this env var on non-Win32; SDL3 matches game windowing layer
export DXVK_WSI_DRIVER="SDL3"

if [[ -f "${RESOURCES_DIR}/MoltenVK_icd.json" ]]; then
    export VK_ICD_FILENAMES="${RESOURCES_DIR}/MoltenVK_icd.json"
    # GeneralsX @bugfix fbraz3 20/03/2026 Vulkan Loader 1.3.236+ uses VK_DRIVER_FILES; keep VK_ICD_FILENAMES for older loaders
    export VK_DRIVER_FILES="${RESOURCES_DIR}/MoltenVK_icd.json"
fi

# Default asset paths matching the standard macOS deploy layout (allow user override)
export CNC_GENERALS_PATH="${CNC_GENERALS_PATH:-${HOME}/GeneralsX/Generals}"
export CNC_GENERALS_ZH_PATH="${CNC_GENERALS_ZH_PATH:-${HOME}/GeneralsX/GeneralsMD}"

# Backward compatibility for existing runtime readers
if [[ -z "${CNC_GENERALS_INSTALLPATH:-}" ]]; then
    export CNC_GENERALS_INSTALLPATH="${CNC_GENERALS_PATH}"
fi

if [[ -f "${RESOURCES_DIR}/dxvk.conf" ]]; then
    export DXVK_CONFIG_FILE="${RESOURCES_DIR}/dxvk.conf"
fi

# GeneralsX @bugfix Copilot 24/03/2026 Set bundled Fontconfig config path to avoid "Cannot load default config file: (null)" on macOS.
if [[ -f "${RESOURCES_DIR}/fontconfig/fonts.conf" ]]; then
    export FONTCONFIG_FILE="${RESOURCES_DIR}/fontconfig/fonts.conf"
    export FONTCONFIG_PATH="${RESOURCES_DIR}/fontconfig"
fi

# Run from the detected Zero Hour asset root when available.
if [[ -d "${CNC_GENERALS_ZH_PATH}" ]]; then
    cd "${CNC_GENERALS_ZH_PATH}"
fi

exec "${BIN_DIR}/GeneralsXZH" "$@"
WRAPPER
chmod +x "${MACOS_DIR}/run.sh"

# Keep compatibility with direct calls using the game name.
ln -sf run.sh "${MACOS_DIR}/GeneralsXZH"

# Helper CLI launcher for terminal-based local tests.
echo "  + run.sh"
cat > "${STAGE_DIR}/run.sh" << 'RUNNER'
#!/bin/bash
# GeneralsX @feature Copilot 19/03/2026 Helper runner for launching the generated .app from terminal.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "${SCRIPT_DIR}/GeneralsXZH.app/Contents/MacOS/run.sh" "$@"
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
echo '  CNC_GENERALS_PATH=$HOME/GeneralsX/Generals'
echo '  CNC_GENERALS_ZH_PATH=$HOME/GeneralsX/GeneralsMD'
