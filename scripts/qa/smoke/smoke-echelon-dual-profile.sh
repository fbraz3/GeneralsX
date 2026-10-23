#!/usr/bin/env bash
# GeneralsX @build cheviiot 23/10/2026 PLAN-023 Gate 6: headless smoke for echelon both profiles.
# Usage: ./scripts/qa/smoke/smoke-echelon-dual-profile.sh [build-dir]
# Expects game assets via CNC_GENERALS_PATH / CNC_GENERALS_ZH_PATH (or defaults under XDG).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${PROJECT_ROOT}/build/linux64-unified-baseline}"
BINARY="${BUILD_DIR}/GeneralsMD/echelon"
LOG_DIR="${PROJECT_ROOT}/logs"
STAMP="$(date +%Y%m%d-%H%M%S)"

export CNC_GENERALS_PATH="${CNC_GENERALS_PATH:-${HOME}/.local/share/generalsx/game-data/Generals}"
export CNC_GENERALS_ZH_PATH="${CNC_GENERALS_ZH_PATH:-${HOME}/.local/share/generalsx/game-data/ZeroHour}"

mkdir -p "${LOG_DIR}"

if [[ ! -x "${BINARY}" ]]; then
	echo "ERROR: echelon binary not found or not executable: ${BINARY}"
	echo "Build: cmake --build ${BUILD_DIR} --target echelon"
	exit 1
fi

run_profile() {
	local profile="$1"
	local xdg
	xdg="$(mktemp -d "/tmp/echelon-smoke-${profile}.XXXXXX")"
	local log="${LOG_DIR}/smoke-echelon-${profile}-${STAMP}.out"
	echo "INFO: echelon -game ${profile} (XDG_DATA_HOME=${xdg})"
	set +e
	XDG_DATA_HOME="${xdg}" timeout 120 "${BINARY}" \
		-game "${profile}" \
		-headless \
		-replay '__unified_smoke_no_matches_*.rep' \
		>"${log}" 2>&1
	local rc=$?
	set -e
	echo "INFO: profile=${profile} exit=${rc} log=${log}"
	if [[ "${rc}" -ne 0 ]]; then
		tail -40 "${log}" || true
		return "${rc}"
	fi
	grep -q "\[GAME_PROFILE\] selected '${profile}'" "${log}" \
		|| { echo "ERROR: missing GAME_PROFILE selected '${profile}' in ${log}"; return 1; }
	return 0
}

check_mismatch() {
	local bin="$1"
	local wrong_profile="$2"
	local label="$3"
	if [[ ! -x "${bin}" ]]; then
		echo "WARN: skip mismatch check; missing ${bin}"
		return 0
	fi
	local log="${LOG_DIR}/smoke-mismatch-${label}-${STAMP}.out"
	local xdg
	xdg="$(mktemp -d "/tmp/echelon-mismatch-${label}.XXXXXX")"
	echo "INFO: expect FATAL mismatch: ${bin} -game ${wrong_profile}"
	set +e
	XDG_DATA_HOME="${xdg}" timeout 60 "${bin}" \
		-game "${wrong_profile}" \
		-headless \
		-replay '__unified_smoke_no_matches_*.rep' \
		>"${log}" 2>&1
	local rc=$?
	set -e
	if [[ "${rc}" -eq 0 ]]; then
		echo "ERROR: ${label} accepted wrong profile ${wrong_profile} (exit 0); log=${log}"
		tail -20 "${log}" || true
		return 1
	fi
	echo "INFO: ${label} mismatch exit=${rc} (ok) log=${log}"
	return 0
}

echo "INFO: Dual-profile smoke using ${BINARY}"
run_profile zh
run_profile generals
# GeneralsX @build cheviiot 24/10/2026 Migration binaries must still FATAL on wrong -game.
check_mismatch "${BUILD_DIR}/Generals/GeneralsX" zh generalsx
check_mismatch "${BUILD_DIR}/GeneralsMD/GeneralsXZH" generals generalsxzh
echo "OK: echelon dual-profile headless smoke passed"
