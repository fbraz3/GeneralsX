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
LIB_ARIAL_SHA256="76d04c18ea243f426b7de1f3ad208e927008f961dc5945e5aad352d0dfde8ee8"
LIB_ARIALBOLD_SHA256="788abee4c806d660e8aee46689dd8540cd4bb98da03dcc9d171ce3efd99a9173"
LIB_COURIER_SHA256="f2b83c763e8afd21709333370bed4774337fae82267937e2b5aea7e2fbd922c1"
LIB_TIMES_SHA256="058ea80864aef09a23f45cbec2bb5400bc3dfbdea01c3f10538a21fcb497fb74"
LIB_LIC_SHA256="93fed46019c38bbe566b479d22148e2e8a1e85ada614accb0211c37b2c61c19b"

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
    [[ -f "${DEST}/LICENSE.liberation" ]] && \
    sha256_verify "${LIB_ARIAL_SHA256}" "${DEST}/arial.ttf" >/dev/null 2>&1 && \
    sha256_verify "${LIB_ARIALBOLD_SHA256}" "${DEST}/arialbold.ttf" >/dev/null 2>&1 && \
    sha256_verify "${LIB_COURIER_SHA256}" "${DEST}/couriernew.ttf" >/dev/null 2>&1 && \
    sha256_verify "${LIB_TIMES_SHA256}" "${DEST}/timesnewroman.ttf" >/dev/null 2>&1 && \
    sha256_verify "${LIB_LIC_SHA256}" "${DEST}/LICENSE.liberation" >/dev/null 2>&1
}

if is_valid_liberation && is_valid_fa; then
    echo "==> Fonts and licenses already present and verified at ${DEST}"
    exit 0
fi

if ! is_valid_liberation; then
    if [[ -f "${PROJECT_ROOT}/assets/fonts/arial.ttf" ]] && \
       sha256_verify "${LIB_ARIAL_SHA256}" "${PROJECT_ROOT}/assets/fonts/arial.ttf" >/dev/null 2>&1 && \
       sha256_verify "${LIB_ARIALBOLD_SHA256}" "${PROJECT_ROOT}/assets/fonts/arialbold.ttf" >/dev/null 2>&1 && \
       sha256_verify "${LIB_COURIER_SHA256}" "${PROJECT_ROOT}/assets/fonts/couriernew.ttf" >/dev/null 2>&1 && \
       sha256_verify "${LIB_TIMES_SHA256}" "${PROJECT_ROOT}/assets/fonts/timesnewroman.ttf" >/dev/null 2>&1 && \
       sha256_verify "${LIB_LIC_SHA256}" "${PROJECT_ROOT}/assets/fonts/LICENSE.liberation" >/dev/null 2>&1; then
        echo "==> Staging Liberation fonts from repository assets/"
        cp "${PROJECT_ROOT}/assets/fonts/arial.ttf" "${TMP}/arial.ttf"
        cp "${PROJECT_ROOT}/assets/fonts/arialbold.ttf" "${TMP}/arialbold.ttf"
        cp "${PROJECT_ROOT}/assets/fonts/couriernew.ttf" "${TMP}/couriernew.ttf"
        cp "${PROJECT_ROOT}/assets/fonts/timesnewroman.ttf" "${TMP}/timesnewroman.ttf"
        cp "${PROJECT_ROOT}/assets/fonts/LICENSE.liberation" "${TMP}/LICENSE.liberation"
    else
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

        cp "${SRC}/LiberationSans-Regular.ttf"   "${TMP}/arial.ttf"
        cp "${SRC}/LiberationSans-Bold.ttf"      "${TMP}/arialbold.ttf"
        cp "${SRC}/LiberationMono-Regular.ttf"   "${TMP}/couriernew.ttf"
        cp "${SRC}/LiberationSerif-Regular.ttf"  "${TMP}/timesnewroman.ttf"
        cp "${SRC}/LICENSE"                      "${TMP}/LICENSE.liberation"
    fi

    sha256_verify "${LIB_ARIAL_SHA256}" "${TMP}/arial.ttf"
    sha256_verify "${LIB_ARIALBOLD_SHA256}" "${TMP}/arialbold.ttf"
    sha256_verify "${LIB_COURIER_SHA256}" "${TMP}/couriernew.ttf"
    sha256_verify "${LIB_TIMES_SHA256}" "${TMP}/timesnewroman.ttf"
    sha256_verify "${LIB_LIC_SHA256}" "${TMP}/LICENSE.liberation"

    cp "${TMP}/arial.ttf"          "${DEST}/arial.ttf"
    cp "${TMP}/arialbold.ttf"      "${DEST}/arialbold.ttf"
    cp "${TMP}/couriernew.ttf"     "${DEST}/couriernew.ttf"
    cp "${TMP}/timesnewroman.ttf"  "${DEST}/timesnewroman.ttf"
    cp "${TMP}/LICENSE.liberation" "${DEST}/LICENSE.liberation"
fi

if ! is_valid_fa; then
    if [[ -f "${PROJECT_ROOT}/assets/fonts/fa-brands-400.ttf" && -f "${PROJECT_ROOT}/assets/fonts/LICENSE.fontawesome" ]] && \
       sha256_verify "${FA_SHA256}" "${PROJECT_ROOT}/assets/fonts/fa-brands-400.ttf" >/dev/null 2>&1 && \
       sha256_verify "${FA_LIC_SHA256}" "${PROJECT_ROOT}/assets/fonts/LICENSE.fontawesome" >/dev/null 2>&1; then
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
