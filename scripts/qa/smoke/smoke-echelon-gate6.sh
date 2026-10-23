#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6 acceptance battery for echelon.
# Runs dual-profile, map-load, save/load, record→playback, and CRC-clean fixture playback.
# Usage: ./scripts/qa/smoke/smoke-echelon-gate6.sh [build-dir]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${PROJECT_ROOT}/build/linux64-unified-baseline}"
LOG_DIR="${PROJECT_ROOT}/logs"
STAMP="$(date +%Y%m%d-%H%M%S)"
SUMMARY="${LOG_DIR}/smoke-gate6-summary-${STAMP}.txt"

mkdir -p "${LOG_DIR}"
: >"${SUMMARY}"

run() {
	local name="$1"
	shift
	echo "INFO: === ${name} ===" | tee -a "${SUMMARY}"
	set +e
	"$@" 2>&1 | tee "${LOG_DIR}/smoke-gate6-${name}-${STAMP}.log" | tail -12
	local rc=${PIPESTATUS[0]}
	set -e
	echo "INFO: ${name} rc=${rc}" | tee -a "${SUMMARY}"
	if [[ "${rc}" -ne 0 ]]; then
		echo "FAIL: ${name}" | tee -a "${SUMMARY}"
		exit "${rc}"
	fi
}

echo "INFO: Gate 6 battery build=${BUILD_DIR}" | tee -a "${SUMMARY}"

run dual "${SCRIPT_DIR}/smoke-echelon-dual-profile.sh" "${BUILD_DIR}"
run map-alpine "${SCRIPT_DIR}/smoke-echelon-map-load.sh" "${BUILD_DIR}" "Maps/Alpine Assault/Alpine Assault.map" 90 180
run map-usa01 "${SCRIPT_DIR}/smoke-echelon-map-load.sh" "${BUILD_DIR}" "Maps/USA01/USA01.map" 90 180
run mod-path "${SCRIPT_DIR}/smoke-echelon-mod-path.sh" "${BUILD_DIR}"
run lan "${SCRIPT_DIR}/smoke-echelon-lan.sh" "${BUILD_DIR}"
run saveload-zh "${SCRIPT_DIR}/smoke-echelon-save-load.sh" "${BUILD_DIR}" zh
run saveload-generals "${SCRIPT_DIR}/smoke-echelon-save-load.sh" "${BUILD_DIR}" generals
run rr-zh "${SCRIPT_DIR}/smoke-echelon-record-replay.sh" "${BUILD_DIR}" zh
run rr-generals "${SCRIPT_DIR}/smoke-echelon-record-replay.sh" "${BUILD_DIR}" generals

# Strict CRC-clean playback of self-recorded Gate 6 fixtures (must exit 0, not soft-pass 2).
ZH_FIX="${PROJECT_ROOT}/logs/fixtures/replays/zh/echelon_gate6_crc_clean.rep"
G_FIX="${PROJECT_ROOT}/logs/fixtures/replays/generals/echelon_gate6_crc_clean.rep"
if [[ ! -f "${ZH_FIX}" || ! -f "${G_FIX}" ]]; then
	echo "INFO: seeding Gate 6 CRC-clean fixtures from latest record→playback reps" | tee -a "${SUMMARY}"
	mkdir -p "$(dirname "${ZH_FIX}")" "$(dirname "${G_FIX}")"
	zh_src="$(ls -t "${PROJECT_ROOT}"/logs/fixtures/replays/zh/echelon_skirmish_*.rep 2>/dev/null | head -1 || true)"
	g_src="$(ls -t "${PROJECT_ROOT}"/logs/fixtures/replays/generals/echelon_skirmish_*.rep 2>/dev/null | head -1 || true)"
	[[ -n "${zh_src}" && -n "${g_src}" ]] || { echo "ERROR: no skirmish fixtures to seed"; exit 1; }
	cp -f "${zh_src}" "${ZH_FIX}"
	cp -f "${g_src}" "${G_FIX}"
fi

run fixture-zh "${SCRIPT_DIR}/smoke-echelon-replay.sh" "${BUILD_DIR}" "${ZH_FIX}" zh
run fixture-generals "${SCRIPT_DIR}/smoke-echelon-replay.sh" "${BUILD_DIR}" "${G_FIX}" generals

echo "OK: Gate 6 echelon acceptance battery passed" | tee -a "${SUMMARY}"
echo "INFO: summary=${SUMMARY}"
