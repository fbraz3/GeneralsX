#!/usr/bin/env bash
# GeneralsX @build cheviiot 23/10/2026 PLAN-023 Gate 6: deploy unified echelon binary.
# Usage: ./scripts/build/linux/deploy-linux-echelon.sh [build-dir]
# Reuses deploy-linux-zh.sh library/RPATH/run.sh logic via env overrides.
# Also installs GeneralsXZH alias next to echelon for legacy wrappers.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${BUILD_DIR:-${PROJECT_ROOT}/build/linux64-unified-baseline}}"
BINARY_SRC="${BUILD_DIR}/GeneralsMD/echelon"
PREFERRED_RUNTIME_DIR="${HOME}/GeneralsX/Echelon"
LEGACY_ZH_DIR="${HOME}/GeneralsX/GeneralsZH"

if [[ ! -f "${BINARY_SRC}" ]] || [[ ! -s "${BINARY_SRC}" ]]; then
	echo "ERROR: echelon binary missing or empty: ${BINARY_SRC}"
	echo "Build: cmake --build ${BUILD_DIR} --target echelon"
	exit 1
fi

# Prefer a dedicated Echelon runtime; fall back to ZH asset tree when present.
RUNTIME_DIR="${PREFERRED_RUNTIME_DIR}"
if [[ -d "${LEGACY_ZH_DIR}" && -n "$(compgen -G "${LEGACY_ZH_DIR}/*.big" 2>/dev/null || true)" ]]; then
	RUNTIME_DIR="${LEGACY_ZH_DIR}"
	echo "INFO: Deploying echelon beside Zero Hour assets in ${RUNTIME_DIR}"
elif [[ -d "${PREFERRED_RUNTIME_DIR}" ]]; then
	RUNTIME_DIR="${PREFERRED_RUNTIME_DIR}"
fi
mkdir -p "${RUNTIME_DIR}"

export BUILD_DIR
export BINARY_SRC
export DEPLOY_BINARY_NAME=echelon
export RUNTIME_DIR
export RUNTIME_DIR_FORCED=1

"${SCRIPT_DIR}/deploy-linux-zh.sh"

# Legacy alias: same bits as GeneralsXZH for older run-linux-zh helpers.
cp -v "${RUNTIME_DIR}/echelon" "${RUNTIME_DIR}/GeneralsXZH"
chmod +x "${RUNTIME_DIR}/GeneralsXZH"
if command -v patchelf >/dev/null 2>&1; then
	patchelf --set-rpath '$ORIGIN' "${RUNTIME_DIR}/GeneralsXZH" 2>/dev/null || true
fi

echo ""
echo "Echelon deploy complete"
echo "  Primary:  ${RUNTIME_DIR}/echelon"
echo "  Alias:    ${RUNTIME_DIR}/GeneralsXZH"
echo "  Profiles: ./run.sh -game zh -win"
echo "            ./run.sh -game generals -win"
echo "  Smoke:    ./scripts/qa/smoke/smoke-echelon-dual-profile.sh ${BUILD_DIR}"
