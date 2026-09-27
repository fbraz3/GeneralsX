#!/usr/bin/env bash
# Stage and verify universal Liberation and Font Awesome fonts for GeneralsX
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

FA_VERSION="6.5.2"
FA_SHA256="e28096fa75a96ac77020155ea3a6dd7312983e84115366d4cf49a0c312ec6d51"
FA_LIC_SHA256="9b914eae88817d63b576eab5aafde7068c7a1abae125d7cdfb034f1da43a9afc"

DEST="${1:-${GX_FONTS:-${PROJECT_ROOT}/assets/fonts}}"
TMP="$(mktemp -d)"
trap 'rm -rf "${TMP}"' EXIT

sha256_verify() {
    local checksum="$1"
    local file="$2"
    if command -v sha256sum >/dev/null 2>&1; then
        echo "${checksum}  ${file}" | sha256sum -c -
    else
        echo "${checksum}  ${file}" | shasum -a 256 -c -
    fi
}

mkdir -p "${DEST}"

is_valid_fa() {
    [[ -f "${DEST}/fa-brands-400.ttf" ]] && \
    [[ -f "${DEST}/LICENSE.fontawesome" ]] && \
    sha256_verify "${FA_SHA256}" "${DEST}/fa-brands-400.ttf" >/dev/null 2>&1 && \
    sha256_verify "${FA_LIC_SHA256}" "${DEST}/LICENSE.fontawesome" >/dev/null 2>&1
}

is_valid_liberation() {
    [[ -f "${DEST}/arial.ttf" ]] && \
    [[ -f "${DEST}/arialbold.ttf" ]] && \
    [[ -f "${DEST}/couriernew.ttf" ]] && \
    [[ -f "${DEST}/timesnewroman.ttf" ]] && \
    [[ -f "${DEST}/LICENSE.liberation" ]]
}

if is_valid_liberation && is_valid_fa; then
    echo "==> Fonts and licenses already present and verified at ${DEST}"
    exit 0
fi

if ! is_valid_liberation; then
    echo "==> Downloading Liberation fonts ${LIB_VERSION}"
    if ! curl -fL -o "${TMP}/liberation.tar.gz" \
        "https://github.com/liberationfonts/liberation-fonts/files/7261482/liberation-fonts-ttf-${LIB_VERSION}.tar.gz" &&
       ! curl -fL -o "${TMP}/liberation.tar.gz" \
        "https://github.com/liberationfonts/liberation-fonts/releases/download/${LIB_VERSION}/liberation-fonts-ttf-${LIB_VERSION}.tar.gz"; then
        echo "ERROR: Could not download Liberation fonts ${LIB_VERSION} (check network / URLs)." >&2
        exit 1
    fi

    sha256_verify "${LIB_SHA256}" "${TMP}/liberation.tar.gz"
    tar -xzf "${TMP}/liberation.tar.gz" -C "${TMP}"
    SRC="$(find "${TMP}" -name "LiberationSans-Regular.ttf" -exec dirname {} \; | head -1)"
    [[ -n "${SRC}" ]] || { echo "ERROR: Liberation fonts not found in extracted archive" >&2; exit 1; }

    cp "${SRC}/LiberationSans-Regular.ttf"   "${DEST}/arial.ttf"
    cp "${SRC}/LiberationSans-Bold.ttf"      "${DEST}/arialbold.ttf"
    cp "${SRC}/LiberationMono-Regular.ttf"   "${DEST}/couriernew.ttf"
    cp "${SRC}/LiberationSerif-Regular.ttf"  "${DEST}/timesnewroman.ttf"
    cp "${SRC}/LICENSE"                      "${DEST}/LICENSE.liberation"
fi

if ! is_valid_fa; then
    if [[ -f "${PROJECT_ROOT}/assets/fonts/fa-brands-400.ttf" && -f "${PROJECT_ROOT}/assets/fonts/LICENSE.fontawesome" ]]; then
        echo "==> Staging Font Awesome assets from repository assets/"
        cp "${PROJECT_ROOT}/assets/fonts/fa-brands-400.ttf" "${TMP}/fa-brands-400.ttf"
        cp "${PROJECT_ROOT}/assets/fonts/LICENSE.fontawesome" "${TMP}/LICENSE.fontawesome"
    else
        echo "==> Downloading Font Awesome ${FA_VERSION} web archive"
        if ! curl -fL -o "${TMP}/fa-web.zip" \
            "https://github.com/FortAwesome/Font-Awesome/releases/download/${FA_VERSION}/fontawesome-free-${FA_VERSION}-web.zip"; then
            echo "ERROR: Could not download Font Awesome ${FA_VERSION}." >&2
            exit 1
        fi
        unzip -q -j "${TMP}/fa-web.zip" "fontawesome-free-${FA_VERSION}-web/webfonts/fa-brands-400.ttf" -d "${TMP}"
        unzip -q -j "${TMP}/fa-web.zip" "fontawesome-free-${FA_VERSION}-web/LICENSE.txt" -d "${TMP}"
        mv "${TMP}/LICENSE.txt" "${TMP}/LICENSE.fontawesome"
    fi

    sha256_verify "${FA_SHA256}" "${TMP}/fa-brands-400.ttf"
    sha256_verify "${FA_LIC_SHA256}" "${TMP}/LICENSE.fontawesome"
    cp "${TMP}/fa-brands-400.ttf" "${DEST}/fa-brands-400.ttf"
    cp "${TMP}/LICENSE.fontawesome" "${DEST}/LICENSE.fontawesome"
fi

echo "==> Staged universal fonts to ${DEST}"
