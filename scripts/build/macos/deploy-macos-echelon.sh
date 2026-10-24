#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: deploy unified echelon on macOS.
# Usage: ./scripts/build/macos/deploy-macos-echelon.sh [build-dir]
# Reuses deploy-macos-zh.sh via BINARY_SRC / DEPLOY_BINARY_NAME overrides, then
# installs an echelon copy alongside the GeneralsXZH alias.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${BUILD_DIR:-${PROJECT_ROOT}/build/macos-vulkan}}"
BINARY_SRC="${BUILD_DIR}/GeneralsMD/echelon"

if [[ ! -f "${BINARY_SRC}" ]] || [[ ! -s "${BINARY_SRC}" ]]; then
	echo "ERROR: echelon binary missing or empty: ${BINARY_SRC}"
	echo "Build: cmake --build ${BUILD_DIR} --target echelon"
	exit 1
fi

export BUILD_DIR
export BINARY_SRC
export DEPLOY_BINARY_NAME="${DEPLOY_BINARY_NAME:-GeneralsXZH}"

"${SCRIPT_DIR}/deploy-macos-zh.sh"

RUNTIME_DIR="${HOME}/GeneralsX/GeneralsZH"
if [[ ! -d "${RUNTIME_DIR}" ]]; then
	RUNTIME_DIR="${HOME}/GeneralsX/GeneralsMD"
fi

cp -v "${BINARY_SRC}" "${RUNTIME_DIR}/echelon"
chmod +x "${RUNTIME_DIR}/echelon"

echo ""
echo "Echelon macOS deploy complete"
echo "  Primary alias: ${RUNTIME_DIR}/${DEPLOY_BINARY_NAME}"
echo "  Unified name:  ${RUNTIME_DIR}/echelon"
echo "  Profiles: ./run.sh -game zh -win"
echo "            ./run.sh -game generals -win"
