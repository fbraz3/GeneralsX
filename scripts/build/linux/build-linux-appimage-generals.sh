#!/usr/bin/env bash
# GeneralsX @build GitHubCopilot 09/04/2026 Build a portable AppImage package for GeneralsX on Linux.
# Usage:
#   ./scripts/build/linux/build-linux-appimage-generals.sh [preset]
set -euo pipefail

PRESET="${1:-linux64-deploy}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/${PRESET}"
APPIMAGE_ROOT="${PROJECT_ROOT}/build/appimage"
APPDIR="${APPIMAGE_ROOT}/GeneralsX.AppDir"
OUTPUT_APPIMAGE="${PROJECT_ROOT}/build/GeneralsX-${PRESET}-x86_64.AppImage"

DXVK_LIB_DIR="${BUILD_DIR}/_deps/dxvk-src/lib"
SDL3_LIB_DIR="${BUILD_DIR}/_deps/sdl3-build"
SDL3_IMAGE_LIB_DIR="${BUILD_DIR}/_deps/sdl3_image-build"
OPENAL_LIB_DIR="${BUILD_DIR}/_deps/openal_soft-build"
BINARY_SRC="${BUILD_DIR}/Generals/GeneralsX"
GAMESPY_LIB="${BUILD_DIR}/libgamespy.so"
DXVK_CONF_SRC="${PROJECT_ROOT}/resources/dxvk/dxvk.conf"
ICON_SRC="${PROJECT_ROOT}/assets/generalsx_icon.png"

copy_optional_libs() {
    local source_dir="$1"
    local pattern="$2"
    if [[ -d "${source_dir}" ]]; then
        local matches=()
        shopt -s nullglob
        matches=("${source_dir}"/${pattern})
        shopt -u nullglob
        if (( ${#matches[@]} > 0 )); then
            cp -a "${matches[@]}" "${APPDIR}/usr/lib/"
        fi
    fi
}

if [[ ! -f "${BINARY_SRC}" || ! -s "${BINARY_SRC}" ]]; then
    echo "ERROR: Missing or empty binary: ${BINARY_SRC}" >&2
    echo "Build first: ./scripts/build/linux/docker-build-linux-generals.sh ${PRESET}" >&2
    exit 1
fi
if [[ ! -d "${DXVK_LIB_DIR}" ]]; then
    echo "ERROR: Missing DXVK libs dir: ${DXVK_LIB_DIR}" >&2
    exit 1
fi
if [[ ! -d "${SDL3_LIB_DIR}" || ! -d "${SDL3_IMAGE_LIB_DIR}" ]]; then
    echo "ERROR: Missing SDL3/SDL3_image build dirs under ${BUILD_DIR}" >&2
    exit 1
fi
if [[ ! -f "${GAMESPY_LIB}" ]]; then
    echo "ERROR: Missing GameSpy lib: ${GAMESPY_LIB}" >&2
    exit 1
fi

rm -rf "${APPDIR}"
mkdir -p "${APPDIR}/usr/bin" "${APPDIR}/usr/lib" "${APPDIR}/usr/share/applications" "${APPDIR}/usr/share/icons/hicolor/512x512/apps"

cp "${BINARY_SRC}" "${APPDIR}/usr/bin/GeneralsX"
chmod +x "${APPDIR}/usr/bin/GeneralsX"
cp "${GAMESPY_LIB}" "${APPDIR}/usr/lib/"
copy_optional_libs "${DXVK_LIB_DIR}" "libdxvk_d3d8.so*"
copy_optional_libs "${DXVK_LIB_DIR}" "libdxvk_d3d9.so*"
copy_optional_libs "${SDL3_LIB_DIR}" "libSDL3.so*"
copy_optional_libs "${SDL3_IMAGE_LIB_DIR}" "libSDL3_image.so*"
copy_optional_libs "${OPENAL_LIB_DIR}" "libopenal.so*"

if [[ -f "${DXVK_CONF_SRC}" ]]; then
    mkdir -p "${APPDIR}/usr/share/generalsx"
    cp "${DXVK_CONF_SRC}" "${APPDIR}/usr/share/generalsx/dxvk.conf"
fi

cat > "${APPDIR}/AppRun" << 'EOF'
#!/usr/bin/env bash
# GeneralsX @build GitHubCopilot 09/04/2026 AppImage runtime launcher for GeneralsX.
# GeneralsX @bugfix GitHubCopilot 09/04/2026 Honor CNC_GENERALS_PATH / CNC_GENERALS_INSTALLPATH with deterministic precedence.
set -euo pipefail

APPDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="${APPDIR}/usr/lib:${LD_LIBRARY_PATH:-}"
export DXVK_WSI_DRIVER="SDL3"
export DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-info}"
export DXVK_HUD="${DXVK_HUD:-0}"

with_trailing_slash() {
    local path="$1"
    if [[ "${path}" == */ ]]; then
        printf '%s' "${path}"
    else
        printf '%s/' "${path}"
    fi
}

APPIMAGE_HOST_DIR=""
if [[ -n "${APPIMAGE:-}" ]]; then
    APPIMAGE_HOST_DIR="$(cd "$(dirname "${APPIMAGE}")" && pwd)"
fi
LAUNCH_DIR="$(pwd)"

if [[ -n "${CNC_GENERALS_PATH:-}" ]]; then
    if [[ ! -d "${CNC_GENERALS_PATH}" ]]; then
        echo "WARNING: CNC_GENERALS_PATH='${CNC_GENERALS_PATH}' does not exist; falling back to auto-detection"
        unset CNC_GENERALS_PATH
    else
        export CNC_GENERALS_PATH="$(with_trailing_slash "${CNC_GENERALS_PATH}")"
    fi
fi

if [[ -z "${CNC_GENERALS_PATH:-}" && -n "${CNC_GENERALS_INSTALLPATH:-}" && -d "${CNC_GENERALS_INSTALLPATH}" ]]; then
    export CNC_GENERALS_PATH="$(with_trailing_slash "${CNC_GENERALS_INSTALLPATH}")"
fi
if [[ -z "${CNC_GENERALS_PATH:-}" && -n "${APPIMAGE_HOST_DIR}" && -d "${APPIMAGE_HOST_DIR}" ]]; then
    export CNC_GENERALS_PATH="$(with_trailing_slash "${APPIMAGE_HOST_DIR}")"
fi
if [[ -z "${CNC_GENERALS_PATH:-}" && -d "${LAUNCH_DIR}" ]]; then
    export CNC_GENERALS_PATH="$(with_trailing_slash "${LAUNCH_DIR}")"
fi
if [[ -z "${CNC_GENERALS_PATH:-}" && -d "${HOME}/GeneralsX/Generals" ]]; then
    export CNC_GENERALS_PATH="$(with_trailing_slash "${HOME}/GeneralsX/Generals")"
fi

if [[ -n "${CNC_GENERALS_PATH:-}" && -z "${CNC_GENERALS_INSTALLPATH:-}" ]]; then
    export CNC_GENERALS_INSTALLPATH="$(with_trailing_slash "${CNC_GENERALS_PATH}")"
fi

if [[ -n "${CNC_GENERALS_PATH:-}" ]]; then
    echo "INFO: AppImage base Generals path: ${CNC_GENERALS_PATH}"
    cd "${CNC_GENERALS_PATH}"
fi

if [[ -z "${ALSOFT_DISABLE_CPU_EXTS:-}" ]]; then
    export ALSOFT_DISABLE_CPU_EXTS="all"
fi
if [[ -z "${ALSOFT_DRIVERS:-}" ]]; then
    export ALSOFT_DRIVERS="pulse,alsa,oss,jack,null,wave"
fi

exec "${APPDIR}/usr/bin/GeneralsX" "$@"
EOF
chmod +x "${APPDIR}/AppRun"

cat > "${APPDIR}/GeneralsX.desktop" << 'EOF'
[Desktop Entry]
Type=Application
Name=Command & Conquer Generals (GeneralsX)
Comment=Cross-platform Generals runtime
Exec=GeneralsX
Icon=GeneralsX
Categories=Game;StrategyGame;
Terminal=false
EOF
cp "${APPDIR}/GeneralsX.desktop" "${APPDIR}/usr/share/applications/GeneralsX.desktop"

if [[ -f "${ICON_SRC}" ]]; then
    cp "${ICON_SRC}" "${APPDIR}/GeneralsX.png"
    cp "${ICON_SRC}" "${APPDIR}/usr/share/icons/hicolor/512x512/apps/GeneralsX.png"
fi

if command -v appimagetool >/dev/null 2>&1; then
    APPIMAGETOOL_BIN="$(command -v appimagetool)"
else
    APPIMAGETOOL_BIN="${APPIMAGE_ROOT}/appimagetool.AppImage"
    mkdir -p "${APPIMAGE_ROOT}"
    if [[ ! -x "${APPIMAGETOOL_BIN}" ]]; then
        echo "Downloading appimagetool..."
        curl -L -o "${APPIMAGETOOL_BIN}" "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
        chmod +x "${APPIMAGETOOL_BIN}"
    fi
fi

ARCH=x86_64 "${APPIMAGETOOL_BIN}" "${APPDIR}" "${OUTPUT_APPIMAGE}"

echo "AppImage generated: ${OUTPUT_APPIMAGE}"
echo "Run example:"
echo "  chmod +x ${OUTPUT_APPIMAGE}"
echo "  ${OUTPUT_APPIMAGE} -win"