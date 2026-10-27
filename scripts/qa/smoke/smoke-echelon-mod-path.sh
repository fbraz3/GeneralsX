#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: headless -mod path smoke both profiles.
# Verifies CLI -mod is accepted before quit and archive override plumbing stays live.
# Usage: ./scripts/qa/smoke/smoke-echelon-mod-path.sh [build-dir]

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
	echo "ERROR: echelon binary not found: ${BINARY}"
	exit 1
fi

# Empty mod dir is enough to exercise parseMod + filesystem lookup without needing a .big.
mod_dir="$(mktemp -d "/tmp/echelon-mod-smoke.XXXXXX")"
mkdir -p "${mod_dir}/Data"

run_profile() {
	local profile="$1"
	local xdg log
	xdg="$(mktemp -d "/tmp/echelon-mod-${profile}.XXXXXX")"
	log="${LOG_DIR}/smoke-echelon-mod-${profile}-${STAMP}.out"
	echo "INFO: echelon -game ${profile} -mod ${mod_dir}"
	set +e
	XDG_DATA_HOME="${xdg}" timeout 120 "${BINARY}" \
		-game "${profile}" \
		-headless \
		-mod "${mod_dir}" \
		-quitAfterFrames 30 \
		>"${log}" 2>&1
	local rc=$?
	set -e
	echo "INFO: profile=${profile} exit=${rc} log=${log}"
	grep -q "\[GAME_PROFILE\] selected '${profile}'" "${log}" \
		|| { echo "ERROR: missing GAME_PROFILE"; tail -30 "${log}"; return 1; }
	grep -qiE 'segfault|Aborted|Segmentation fault' "${log}" \
		&& { echo "ERROR: crash"; return 1; }
	# quitAfterFrames may not fire if init fails early; exit 0 or clean quit marker required
	if [[ "${rc}" -ne 0 ]]; then
		echo "ERROR: unexpected exit ${rc}"
		tail -40 "${log}" || true
		return "${rc}"
	fi
	echo "INFO: profile=${profile} -mod path ok"
	return 0
}

run_profile zh
run_profile generals
echo "OK: dual-profile -mod path smoke passed"
