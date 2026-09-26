#!/usr/bin/env bash
# Stage and verify universal Liberation fonts for GeneralsX
#
# Usage:
#   ./scripts/env/setup-fonts.sh [DEST_DIR]
#
# Environment:
#   GX_FONTS          Destination directory for fonts (default: assets/fonts)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

LIB_VERSION="2.1.5"
LIB_SHA256="7191c669bf38899f73a2094ed00f7b800553364f90e2637010a69c0e268f25d0"
DEST="${1:-${GX_FONTS:-${PROJECT_ROOT}/assets/fonts}}"
TMP="$(mktemp -d)"
trap 'rm -rf "${TMP}"' EXIT

mkdir -p "${DEST}"

if [[ -f "${DEST}/arial.ttf" && -f "${DEST}/arialbold.ttf" && -f "${DEST}/couriernew.ttf" && -f "${DEST}/timesnewroman.ttf" && -f "${DEST}/fa-brands-400.ttf" ]]; then
    echo "==> Fonts already present at ${DEST}"
    exit 0
fi

if [[ ! -f "${DEST}/arial.ttf" || ! -f "${DEST}/arialbold.ttf" || ! -f "${DEST}/couriernew.ttf" || ! -f "${DEST}/timesnewroman.ttf" ]]; then
    echo "==> Downloading Liberation fonts ${LIB_VERSION}"
    if ! curl -fL -o "${TMP}/liberation.tar.gz" \
        "https://github.com/liberationfonts/liberation-fonts/files/7261482/liberation-fonts-ttf-${LIB_VERSION}.tar.gz" &&
       ! curl -fL -o "${TMP}/liberation.tar.gz" \
        "https://github.com/liberationfonts/liberation-fonts/releases/download/${LIB_VERSION}/liberation-fonts-ttf-${LIB_VERSION}.tar.gz"; then
        echo "ERROR: Could not download Liberation fonts ${LIB_VERSION} (check network / URLs)." >&2
        exit 1
    fi

    echo "${LIB_SHA256}  ${TMP}/liberation.tar.gz" | shasum -a 256 -c -
    tar -xzf "${TMP}/liberation.tar.gz" -C "${TMP}"
    SRC="$(find "${TMP}" -name "LiberationSans-Regular.ttf" -exec dirname {} \; | head -1)"
    [[ -n "${SRC}" ]] || { echo "ERROR: Liberation fonts not found in extracted archive" >&2; exit 1; }

    cp "${SRC}/LiberationSans-Regular.ttf"   "${DEST}/arial.ttf"
    cp "${SRC}/LiberationSans-Bold.ttf"      "${DEST}/arialbold.ttf"
    cp "${SRC}/LiberationMono-Regular.ttf"   "${DEST}/couriernew.ttf"
    cp "${SRC}/LiberationSerif-Regular.ttf"  "${DEST}/timesnewroman.ttf"
    cp "${SRC}/LICENSE"                      "${DEST}/LICENSE.liberation"
fi

if [[ ! -f "${DEST}/fa-brands-400.ttf" ]]; then
    echo "==> Downloading Font Awesome Brands font"
    curl -fL -o "${DEST}/fa-brands-400.ttf" \
        "https://raw.githubusercontent.com/FortAwesome/Font-Awesome/6.x/webfonts/fa-brands-400.ttf"
    curl -fL -o "${DEST}/LICENSE.fontawesome" \
        "https://raw.githubusercontent.com/FortAwesome/Font-Awesome/6.x/LICENSE.txt"
fi

echo "==> Staged universal fonts to ${DEST}"
