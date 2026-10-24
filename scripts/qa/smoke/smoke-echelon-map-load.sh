#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: headless map-load smoke for echelon both profiles.
# Uses release -map (sets m_initialFile) + -quitAfterFrames for clean exit.
# Usage: ./scripts/qa/smoke/smoke-echelon-map-load.sh [build-dir] [map] [quit-frames] [timeout-sec]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${PROJECT_ROOT}/build/linux64-unified-baseline}"
MAP_PATH="${2:-Maps/Alpine Assault/Alpine Assault.map}"
QUIT_FRAMES="${3:-90}"
TIMEOUT_SEC="${4:-180}"
BINARY="${BUILD_DIR}/GeneralsMD/echelon"
LOG_DIR="${PROJECT_ROOT}/logs"
STAMP="$(date +%Y%m%d-%H%M%S)"

export CNC_GENERALS_PATH="${CNC_GENERALS_PATH:-${HOME}/.local/share/generalsx/game-data/Generals}"
export CNC_GENERALS_ZH_PATH="${CNC_GENERALS_ZH_PATH:-${HOME}/.local/share/generalsx/game-data/ZeroHour}"

mkdir -p "${LOG_DIR}"

if [[ ! -x "${BINARY}" ]]; then
	echo "ERROR: echelon binary not found or not executable: ${BINARY}"
	exit 1
fi

run_map() {
	local profile="$1"
	local xdg
	xdg="$(mktemp -d "/tmp/echelon-map-${profile}.XXXXXX")"
	local log="${LOG_DIR}/smoke-echelon-map-${profile}-${STAMP}.out"
	echo "INFO: echelon -game ${profile} -map '${MAP_PATH}' -quitAfterFrames ${QUIT_FRAMES}"
	set +e
	XDG_DATA_HOME="${xdg}" timeout "${TIMEOUT_SEC}" "${BINARY}" \
		-game "${profile}" \
		-headless \
		-noshellmap \
		-map "${MAP_PATH}" \
		-quitAfterFrames "${QUIT_FRAMES}" \
		>"${log}" 2>&1
	local rc=$?
	set -e
	if [[ "${rc}" -eq 124 ]]; then
		echo "ERROR: profile=${profile} timed out (${TIMEOUT_SEC}s) log=${log}"
		tail -40 "${log}" || true
		return 1
	fi
	if [[ "${rc}" -ne 0 ]]; then
		echo "ERROR: profile=${profile} unexpected exit=${rc} log=${log}"
		tail -40 "${log}" || true
		return 1
	fi
	grep -q "\[GAME_PROFILE\] selected '${profile}'" "${log}" \
		|| { echo "ERROR: missing GAME_PROFILE in ${log}"; return 1; }
	grep -q '\[SMOKE\] quitAfterFrames reached frame' "${log}" \
		|| { echo "ERROR: missing quitAfterFrames marker in ${log}"; return 1; }
	grep -qiE 'FATAL|segfault|Aborted|Segmentation fault' "${log}" \
		&& { echo "ERROR: fatal/crash markers in ${log}"; return 1; }
	echo "INFO: profile=${profile} exit=${rc} map-load ok log=${log}"
	return 0
}

echo "INFO: Map-load smoke using ${BINARY} map='${MAP_PATH}' quitFrames=${QUIT_FRAMES} timeout=${TIMEOUT_SEC}s"
run_map zh
run_map generals
echo "OK: echelon dual-profile map-load smoke passed"
