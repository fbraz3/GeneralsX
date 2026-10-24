#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: headless save then load smoke both profiles.
# Usage: ./scripts/qa/smoke/smoke-echelon-save-load.sh [build-dir] [profile] [map] [timeout-sec]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${PROJECT_ROOT}/build/linux64-unified-baseline}"
PROFILE="${2:-zh}"
MAP_NAME="${3:-Maps/Alpine Assault/Alpine Assault.map}"
TIMEOUT_SEC="${4:-300}"
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

xdg="$(mktemp -d "/tmp/echelon-saveload-${PROFILE}.XXXXXX")"
save_log="${LOG_DIR}/smoke-echelon-save-${PROFILE}-${STAMP}.out"
load_log="${LOG_DIR}/smoke-echelon-load-${PROFILE}-${STAMP}.out"

echo "INFO: save skirmish profile=${PROFILE}"
set +e
XDG_DATA_HOME="${xdg}" timeout "${TIMEOUT_SEC}" "${BINARY}" \
	-game "${PROFILE}" \
	-headless \
	-skirmish "${MAP_NAME}" \
	-saveAtFrame 120 \
	-quitAfterFrames 150 \
	>"${save_log}" 2>&1
save_rc=$?
set -e
echo "INFO: save exit=${save_rc} log=${save_log}"

grep -q "\[GAME_PROFILE\] selected '${PROFILE}'" "${save_log}" \
	|| { echo "ERROR: missing GAME_PROFILE"; tail -30 "${save_log}"; exit 1; }
grep -q '\[SMOKE\] saveAtFrame=.* result=0' "${save_log}" \
	|| { echo "ERROR: save failed"; grep '\[SMOKE\] save' "${save_log}" || true; tail -40 "${save_log}"; exit 1; }

mapfile -t savs < <(find "${xdg}" -type f -name 'echelon_smoke.sav' 2>/dev/null)
if [[ "${#savs[@]}" -eq 0 ]]; then
	echo "ERROR: echelon_smoke.sav not found under ${xdg}"
	find "${xdg}" -name '*.sav' 2>/dev/null | head -20 || true
	exit 1
fi
sav="${savs[0]}"
echo "INFO: saved ${sav}"

# loadQueuedSaveGame expects a filename (leaf) relative to Save/, or path handled by GameState
sav_leaf="$(basename "${sav}")"
# Copy into a fresh XDG Save dir under same profile tree layout is fragile; reuse same XDG.
echo "INFO: load ${sav_leaf}"
set +e
XDG_DATA_HOME="${xdg}" timeout "${TIMEOUT_SEC}" "${BINARY}" \
	-game "${PROFILE}" \
	-headless \
	-loadsave "${sav_leaf}" \
	-quitAfterFrames 80 \
	>"${load_log}" 2>&1
load_rc=$?
set -e
echo "INFO: load exit=${load_rc} log=${load_log}"

grep -q "\[GAME_PROFILE\] selected '${PROFILE}'" "${load_log}" \
	|| { echo "ERROR: missing GAME_PROFILE on load"; tail -30 "${load_log}"; exit 1; }
# Success: reached quit after being in-game, or explicit load breadcrumbs
if ! grep -q '\[SMOKE\] quitAfterFrames reached' "${load_log}"; then
	echo "ERROR: load did not reach in-game quitAfterFrames"
	tail -50 "${load_log}" || true
	exit 1
fi
if [[ "${load_rc}" -ne 0 ]]; then
	echo "ERROR: load exit ${load_rc}"
	exit "${load_rc}"
fi

echo "OK: save+load profile=${PROFILE}"
