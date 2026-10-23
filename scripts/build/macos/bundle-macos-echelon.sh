#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: bundle unified echelon as GeneralsXZH.app.
# Keeps existing artifact name macos-arm64-GeneralsXZH.zip for CI/replay consumers.
# Usage: ./scripts/build/macos/bundle-macos-echelon.sh [build-dir]

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
export OUTPUT_ZIP="${OUTPUT_ZIP:-${PROJECT_ROOT}/macos-arm64-GeneralsXZH.zip}"

echo "INFO: Bundling echelon as GeneralsXZH.app (dual-profile -game zh|generals)"
exec "${SCRIPT_DIR}/bundle-macos-zh.sh"
