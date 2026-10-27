#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: headless LAN bind/discovery probe both profiles.
# Usage: ./scripts/qa/smoke/smoke-echelon-lan.sh [build-dir] [timeout-sec]
# Runs echelon -lanProbe sequentially per profile (shared UDP 8086; not parallel).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${PROJECT_ROOT}/build/linux64-unified-baseline}"
TIMEOUT_SEC="${2:-180}"
BINARY="${BUILD_DIR}/GeneralsMD/echelon"
LOG_DIR="${PROJECT_ROOT}/logs"
STAMP="$(date +%Y%m%d-%H%M%S)"

export CNC_GENERALS_PATH="${CNC_GENERALS_PATH:-${HOME}/.local/share/generalsx/game-data/Generals}"
export CNC_GENERALS_ZH_PATH="${CNC_GENERALS_ZH_PATH:-${HOME}/.local/share/generalsx/game-data/ZeroHour}"

mkdir -p "${LOG_DIR}"

if [[ ! -x "${BINARY}" ]]; then
	echo "ERROR: echelon binary not found: ${BINARY}"
	exit 1
fi

run_lan_probe() {
	local profile="$1"
	local xdg
	xdg="$(mktemp -d "/tmp/echelon-lan-${profile}.XXXXXX")"
	local log="${LOG_DIR}/smoke-echelon-lan-${profile}-${STAMP}.out"
	echo "INFO: echelon -game ${profile} -lanProbe"
	set +e
	XDG_DATA_HOME="${xdg}" timeout "${TIMEOUT_SEC}" "${BINARY}" \
		-game "${profile}" \
		-headless \
		-lanProbe \
		>"${log}" 2>&1
	local rc=$?
	set -e
	echo "INFO: profile=${profile} exit=${rc} log=${log}"
	grep -q "\[GAME_PROFILE\] selected '${profile}'" "${log}" \
		|| { echo "ERROR: missing GAME_PROFILE"; tail -30 "${log}"; return 1; }
	grep -q '\[LAN_PROBE\] SetLocalIP ok' "${log}" \
		|| { echo "ERROR: SetLocalIP failed"; tail -40 "${log}"; return 1; }
	grep -q '\[LAN_PROBE\] ok' "${log}" \
		|| { echo "ERROR: LAN probe did not complete"; tail -40 "${log}"; return 1; }
	# peers=0 is fine for sequential single-instance bind smoke
	if [[ "${rc}" -ne 0 ]]; then
		echo "ERROR: lanProbe exit ${rc}"
		tail -40 "${log}" || true
		return "${rc}"
	fi
	echo "OK: LAN probe ${profile}"
	return 0
}

echo "INFO: LAN probe smoke using ${BINARY}"
run_lan_probe zh
run_lan_probe generals
echo "OK: dual-profile LAN probe"
