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
FFMPEG_LIB_DIR="/usr/lib/x86_64-linux-gnu"
FFMPEG_DEP_LIB_DIR="/lib/x86_64-linux-gnu"
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

copy_codec_dep() {
    local pattern="$1"
    copy_optional_libs "${FFMPEG_DEP_LIB_DIR}" "${pattern}"
    copy_optional_libs "${FFMPEG_LIB_DIR}" "${pattern}"
}

copy_ldd_deps() {
    local root="$1"
    [[ -e "${root}" ]] || return 0

    while IFS= read -r dep; do
        case "${dep}" in
            /lib64/ld-linux* | /lib/*/ld-linux* | /lib/*/libc.so.* | /lib/*/libm.so.* | /lib/*/libpthread.so.* | /lib/*/librt.so.* | /lib/*/libdl.so.*)
                continue
                ;;
        esac

        cp -a "${dep}" "${APPDIR}/usr/lib/" 2>/dev/null || true
        if [[ -L "${dep}" ]]; then
            local resolved
            resolved="$(readlink -f "${dep}")"
            cp -a "${resolved}" "${APPDIR}/usr/lib/" 2>/dev/null || true
        fi
    done < <(ldd "${root}" | awk '{for (i = 1; i <= NF; ++i) { if ($i ~ /^\//) { print $i; break } }}' | sort -u)
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

# GeneralsX @bugfix GitHubCopilot 10/04/2026 Bundle FFmpeg SONAME-compatible libs to avoid host version mismatch (e.g. Ubuntu 25.10).
copy_codec_dep "libavcodec.so*"
copy_codec_dep "libavformat.so*"
copy_codec_dep "libavutil.so*"
copy_codec_dep "libswresample.so*"
copy_codec_dep "libswscale.so*"

# Include transitive codec dependencies required by FFmpeg libs.
copy_codec_dep "libzvbi.so*"
copy_codec_dep "libsnappy.so*"
copy_codec_dep "libaom.so*"
copy_codec_dep "libcodec2.so*"
copy_codec_dep "libgsm.so*"
copy_codec_dep "libjxl.so*"
copy_codec_dep "libjxl_threads.so*"
copy_codec_dep "libmp3lame.so*"
copy_codec_dep "libopenjp2.so*"
copy_codec_dep "libopus.so*"
copy_codec_dep "librav1e.so*"
copy_codec_dep "libshine.so*"
copy_codec_dep "libspeex.so*"
copy_codec_dep "libSvtAv1Enc.so*"
copy_codec_dep "libtheoraenc.so*"
copy_codec_dep "libtheoradec.so*"
copy_codec_dep "libtwolame.so*"
copy_codec_dep "libvorbis.so*"
copy_codec_dep "libvorbisenc.so*"
copy_codec_dep "libwebp.so*"
copy_codec_dep "libwebpmux.so*"
copy_codec_dep "libx264.so*"
copy_codec_dep "libx265.so*"
copy_codec_dep "libxvidcore.so*"
copy_codec_dep "libsoxr.so*"
copy_codec_dep "libvpl.so*"
copy_codec_dep "libva.so*"
copy_codec_dep "libva-drm.so*"
copy_codec_dep "libva-x11.so*"
copy_codec_dep "libvdpau.so*"
copy_codec_dep "libOpenCL.so*"

shopt -s nullglob
for ffmpeg_root in "${APPDIR}"/usr/lib/libavcodec.so* "${APPDIR}"/usr/lib/libavformat.so* "${APPDIR}"/usr/lib/libavutil.so*; do
    copy_ldd_deps "${ffmpeg_root}"
done
shopt -u nullglob

if ! compgen -G "${APPDIR}/usr/lib/libavcodec.so*" > /dev/null; then
    echo "ERROR: Missing required AppImage runtime library libavcodec.so*" >&2
    exit 1
fi
if ! compgen -G "${APPDIR}/usr/lib/libavformat.so*" > /dev/null; then
    echo "ERROR: Missing required AppImage runtime library libavformat.so*" >&2
    exit 1
fi
if ! compgen -G "${APPDIR}/usr/lib/libavutil.so*" > /dev/null; then
    echo "ERROR: Missing required AppImage runtime library libavutil.so*" >&2
    exit 1
fi

if [[ -f "${DXVK_CONF_SRC}" ]]; then
    mkdir -p "${APPDIR}/usr/share/generalsx"
    cp "${DXVK_CONF_SRC}" "${APPDIR}/usr/share/generalsx/dxvk.conf"
fi

if [[ ! -f "${ICON_SRC}" ]]; then
    echo "ERROR: Missing icon asset: ${ICON_SRC}" >&2
    exit 1
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

cp "${ICON_SRC}" "${APPDIR}/GeneralsX.png"
cp "${ICON_SRC}" "${APPDIR}/usr/share/icons/hicolor/512x512/apps/GeneralsX.png"

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