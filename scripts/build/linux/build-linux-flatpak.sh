#!/usr/bin/env bash
# GeneralsX @build GitHubCopilot 09/04/2026 Build Flatpak bundles for GeneralsX/GeneralsXZH from Linux build artifacts.
# Usage:
#   ./scripts/build/linux/build-linux-flatpak.sh [preset] [game]
#   game: GeneralsMD (default) or Generals
set -euo pipefail

PRESET="${1:-linux64-deploy}"
GAME="${2:-GeneralsMD}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_ROOT="${PROJECT_ROOT}/build/${PRESET}"
FLATPAK_DIR="${PROJECT_ROOT}/flatpak"
STAGING_DIR="${FLATPAK_DIR}/staging"
RUNTIME_DIR="${STAGING_DIR}/runtime"
FLATPAK_BUILD_DIR="${PROJECT_ROOT}/build/flatpak-builddir"
FLATPAK_REPO_DIR="${PROJECT_ROOT}/build/flatpak-repo"
RUNTIME_REPO_URL="${RUNTIME_REPO_URL:-https://flathub.org/repo/flathub.flatpakrepo}"
DXVK_CONF_SRC="${PROJECT_ROOT}/resources/dxvk/dxvk.conf"
DXVK_LIB_DIR="${BUILD_ROOT}/_deps/dxvk-src/lib"
SDL3_LIB_DIR="${BUILD_ROOT}/_deps/sdl3-build"
SDL3_IMAGE_LIB_DIR="${BUILD_ROOT}/_deps/sdl3_image-build"
OPENAL_LIB_DIR="${BUILD_ROOT}/_deps/openal_soft-build"
FFMPEG_LIB_DIR="/usr/lib/x86_64-linux-gnu"
FFMPEG_DEP_LIB_DIR="/lib/x86_64-linux-gnu"
LIBXCB_POC_DIR="${LIBXCB_POC_DIR:-}"

case "${GAME}" in
    GeneralsMD)
        MANIFEST="${FLATPAK_DIR}/com.generals.GeneralsXZH.yml"
        APP_ID="com.generals.GeneralsXZH"
        OUTPUT_BUNDLE="${PROJECT_ROOT}/build/GeneralsXZH-${PRESET}.flatpak"
        BINARY_SRC="${BUILD_ROOT}/GeneralsMD/GeneralsXZH"
        STAGING_BINARY="GeneralsXZH"
        BUILD_HINT="./scripts/build/linux/docker-build-linux-zh.sh ${PRESET}"
        ;;
    Generals)
        MANIFEST="${FLATPAK_DIR}/com.generals.GeneralsX.yml"
        APP_ID="com.generals.GeneralsX"
        OUTPUT_BUNDLE="${PROJECT_ROOT}/build/GeneralsX-${PRESET}.flatpak"
        BINARY_SRC="${BUILD_ROOT}/Generals/GeneralsX"
        STAGING_BINARY="GeneralsX"
        BUILD_HINT="./scripts/build/linux/docker-build-linux-generals.sh ${PRESET}"
        ;;
    *)
        echo "ERROR: Unsupported game '${GAME}'. Use GeneralsMD or Generals." >&2
        exit 1
        ;;
esac

echo "Preparing Flatpak staging for game ${GAME} and preset: ${PRESET}"

if [[ ! -f "${BINARY_SRC}" ]]; then
    echo "ERROR: Missing binary ${BINARY_SRC}" >&2
    echo "Build first: ${BUILD_HINT}" >&2
    exit 1
fi
if [[ ! -f "${MANIFEST}" ]]; then
    echo "ERROR: Missing Flatpak manifest ${MANIFEST}" >&2
    exit 1
fi
if ! command -v flatpak-builder >/dev/null 2>&1; then
    echo "ERROR: flatpak-builder is not installed." >&2
    echo "Install with: sudo apt-get install flatpak flatpak-builder" >&2
    exit 1
fi

# GeneralsX @build GitHubCopilot 09/04/2026 Ensure required Flatpak runtime and SDK exist for local task execution.
if ! command -v flatpak >/dev/null 2>&1; then
    echo "ERROR: flatpak is not installed." >&2
    echo "Install with: sudo apt-get install flatpak" >&2
    exit 1
fi

if ! flatpak --user remote-list | awk '{print $1}' | grep -qx "flathub"; then
    echo "Adding flathub remote for current user..."
    flatpak --user remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
fi

if ! flatpak --user info org.freedesktop.Platform//25.08 >/dev/null 2>&1 || \
   ! flatpak --user info org.freedesktop.Sdk//25.08 >/dev/null 2>&1; then
    echo "Installing required Flatpak runtime and SDK (25.08) for current user..."
    flatpak --user install -y flathub org.freedesktop.Platform//25.08 org.freedesktop.Sdk//25.08
fi

rm -rf "${STAGING_DIR}" "${FLATPAK_BUILD_DIR}" "${FLATPAK_REPO_DIR}"
mkdir -p "${RUNTIME_DIR}"

cp "${BINARY_SRC}" "${STAGING_DIR}/${STAGING_BINARY}"
chmod +x "${STAGING_DIR}/${STAGING_BINARY}"

if [[ -f "${DXVK_CONF_SRC}" ]]; then
    cp "${DXVK_CONF_SRC}" "${STAGING_DIR}/dxvk.conf"
else
    echo "WARNING: Missing ${DXVK_CONF_SRC}, creating empty dxvk.conf"
    : > "${STAGING_DIR}/dxvk.conf"
fi

copy_optional_libs() {
    local source_dir="$1"
    local pattern="$2"
    if [[ -d "${source_dir}" ]]; then
        # Preserve symlink chains so SONAME entries (for example libSDL3_image.so.0) exist in Flatpak runtime.
        local matches=()
        shopt -s nullglob
        matches=("${source_dir}"/${pattern})
        shopt -u nullglob
        if (( ${#matches[@]} > 0 )); then
            cp -a "${matches[@]}" "${RUNTIME_DIR}/"
        fi
    fi
}

copy_optional_libs "${DXVK_LIB_DIR}" "libdxvk_d3d8.so*"
copy_optional_libs "${DXVK_LIB_DIR}" "libdxvk_d3d9.so*"
copy_optional_libs "${SDL3_LIB_DIR}" "libSDL3.so*"
copy_optional_libs "${SDL3_IMAGE_LIB_DIR}" "libSDL3_image.so*"
copy_optional_libs "${OPENAL_LIB_DIR}" "libopenal.so*"
copy_optional_libs "${FFMPEG_LIB_DIR}" "libavcodec.so*"
copy_optional_libs "${FFMPEG_LIB_DIR}" "libavformat.so*"
copy_optional_libs "${FFMPEG_LIB_DIR}" "libavutil.so*"
copy_optional_libs "${FFMPEG_LIB_DIR}" "libswresample.so*"
copy_optional_libs "${FFMPEG_LIB_DIR}" "libswscale.so*"

copy_codec_dep() {
    local pattern="$1"
    copy_optional_libs "${FFMPEG_DEP_LIB_DIR}" "${pattern}"
    copy_optional_libs "${FFMPEG_LIB_DIR}" "${pattern}"
}

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

# GeneralsX @feature GitHubCopilot 09/04/2026 Add short-term PoC path to inject newer libxcb stack into Flatpak runtime.
# If LIBXCB_POC_DIR is set, copy libxcb/X11 companion libs from that directory and skip host libxcb injection.
if [[ -n "${LIBXCB_POC_DIR}" ]]; then
    if [[ ! -d "${LIBXCB_POC_DIR}" ]]; then
        echo "ERROR: LIBXCB_POC_DIR is set but directory does not exist: ${LIBXCB_POC_DIR}" >&2
        exit 1
    fi

    echo "Using external libxcb PoC directory: ${LIBXCB_POC_DIR}"
    copy_optional_libs "${LIBXCB_POC_DIR}" "libxcb.so*"
    copy_optional_libs "${LIBXCB_POC_DIR}" "libxcb-*.so*"
    copy_optional_libs "${LIBXCB_POC_DIR}" "libX11.so*"
    copy_optional_libs "${LIBXCB_POC_DIR}" "libX11-xcb.so*"
    copy_optional_libs "${LIBXCB_POC_DIR}" "libxshmfence.so*"
    copy_optional_libs "${LIBXCB_POC_DIR}" "libXau.so*"
    copy_optional_libs "${LIBXCB_POC_DIR}" "libXdmcp.so*"
fi

# GeneralsX @bugfix GitHubCopilot 09/04/2026 Bundle XCB/Wayland WSI libraries for Vulkan surface extension support in Flatpak.
# Those libraries are needed by Intel Vulkan driver (libvulkan_intel.so) for VK_KHR_surface, VK_KHR_xcb_surface, VK_KHR_wayland_surface.
if [[ -z "${LIBXCB_POC_DIR}" ]]; then
    copy_codec_dep "libxcb.so*"
    copy_codec_dep "libxcb-present.so*"
    copy_codec_dep "libxcb-xfixes.so*"
    copy_codec_dep "libxcb-sync.so*"
    copy_codec_dep "libxcb-randr.so*"
    copy_codec_dep "libxcb-shm.so*"
    copy_codec_dep "libxcb-dri3.so*"
    copy_codec_dep "libxshmfence.so*"
    copy_codec_dep "libX11-xcb.so*"
    copy_codec_dep "libX11.so*"
    copy_codec_dep "libxcb-dri2.so*"
    copy_codec_dep "libwayland-client.so*"
fi

if ! compgen -G "${RUNTIME_DIR}/libxcb.so*" > /dev/null; then
    echo "ERROR: Missing required runtime library libxcb.so* (set LIBXCB_POC_DIR or ensure host xcb libs are available)." >&2
    exit 1
fi

copy_ldd_deps() {
    local root="$1"
    [[ -e "${root}" ]] || return 0

    while IFS= read -r dep; do
        case "${dep}" in
            /lib64/ld-linux* | /lib/*/ld-linux* | /lib/*/libc.so.* | /lib/*/libm.so.* | /lib/*/libpthread.so.* | /lib/*/librt.so.* | /lib/*/libdl.so.*)
                continue
                ;;
        esac

        cp -a "${dep}" "${RUNTIME_DIR}/" 2>/dev/null || true
        if [[ -L "${dep}" ]]; then
            local resolved
            resolved="$(readlink -f "${dep}")"
            cp -a "${resolved}" "${RUNTIME_DIR}/" 2>/dev/null || true
        fi
    done < <(ldd "${root}" | awk '{for (i = 1; i <= NF; ++i) { if ($i ~ /^\//) { print $i; break } }}' | sort -u)
}

# GeneralsX @bugfix GitHubCopilot 09/04/2026 Bundle Vulkan ICD libraries from GL provider (Intel, NVIDIA, AMD drivers).
# These are referenced by /usr/share/vulkan/icd.d/*.json manifests and provide VK_KHR_surface implementations.
if [[ -d "/usr/lib/x86_64-linux-gnu/GL/default/lib" ]]; then
    copy_optional_libs "/usr/lib/x86_64-linux-gnu/GL/default/lib" "libvulkan*.so*"
    copy_optional_libs "/usr/lib/x86_64-linux-gnu/GL/default/lib" "libdrm*.so*"
fi

shopt -s nullglob
for ffmpeg_root in "${RUNTIME_DIR}"/libavcodec.so* "${RUNTIME_DIR}"/libavformat.so* "${RUNTIME_DIR}"/libavutil.so*; do
    copy_ldd_deps "${ffmpeg_root}"
done
shopt -u nullglob

find "${BUILD_ROOT}" -name "libgamespy.so*" -type f -exec cp {} "${RUNTIME_DIR}/" \;

if ! compgen -G "${RUNTIME_DIR}/libdxvk_d3d8.so*" > /dev/null; then
    echo "ERROR: Missing required runtime library libdxvk_d3d8.so*" >&2
    exit 1
fi
if ! compgen -G "${RUNTIME_DIR}/libSDL3.so*" > /dev/null; then
    echo "ERROR: Missing required runtime library libSDL3.so*" >&2
    exit 1
fi
if ! compgen -G "${RUNTIME_DIR}/libgamespy.so*" > /dev/null; then
    echo "ERROR: Missing required runtime library libgamespy.so*" >&2
    exit 1
fi

echo "Building Flatpak..."
flatpak-builder --force-clean --repo="${FLATPAK_REPO_DIR}" "${FLATPAK_BUILD_DIR}" "${MANIFEST}"

flatpak build-bundle --runtime-repo="${RUNTIME_REPO_URL}" "${FLATPAK_REPO_DIR}" "${OUTPUT_BUNDLE}" "${APP_ID}"

echo "Flatpak bundle generated: ${OUTPUT_BUNDLE}"
echo "Install example:"
echo "  flatpak --user remote-add --if-not-exists flathub ${RUNTIME_REPO_URL}"
echo "  flatpak --user install -y \"${OUTPUT_BUNDLE}\""
