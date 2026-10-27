#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: record skirmish .rep then play back with CRC check.
# Usage: ./scripts/qa/smoke/smoke-echelon-record-replay.sh [build-dir] [profile] [map] [frames] [timeout-sec]
# Default map: Alpine Assault (short name resolved by ConvertShortMapPathToLongMapPath).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${PROJECT_ROOT}/build/linux64-unified-baseline}"
PROFILE="${2:-zh}"
MAP_NAME="${3:-Maps/Alpine Assault/Alpine Assault.map}"
FRAMES="${4:-250}"
TIMEOUT_SEC="${5:-360}"
BINARY="${BUILD_DIR}/GeneralsMD/echelon"
LOG_DIR="${PROJECT_ROOT}/logs"
STAMP="$(date +%Y%m%d-%H%M%S)"
FIXTURE_DIR="${PROJECT_ROOT}/logs/fixtures/replays/${PROFILE}"

export CNC_GENERALS_PATH="${CNC_GENERALS_PATH:-${HOME}/.local/share/generalsx/game-data/Generals}"
export CNC_GENERALS_ZH_PATH="${CNC_GENERALS_ZH_PATH:-${HOME}/.local/share/generalsx/game-data/ZeroHour}"

mkdir -p "${LOG_DIR}" "${FIXTURE_DIR}"

if [[ ! -x "${BINARY}" ]]; then
	echo "ERROR: echelon binary not found: ${BINARY}"
	exit 1
fi

xdg="$(mktemp -d "/tmp/echelon-record-${PROFILE}.XXXXXX")"
rec_log="${LOG_DIR}/smoke-echelon-record-${PROFILE}-${STAMP}.out"

echo "INFO: record skirmish profile=${PROFILE} map='${MAP_NAME}' frames=${FRAMES}"
set +e
XDG_DATA_HOME="${xdg}" timeout "${TIMEOUT_SEC}" "${BINARY}" \
	-game "${PROFILE}" \
	-headless \
	-skirmish "${MAP_NAME}" \
	-quitAfterFrames "${FRAMES}" \
	>"${rec_log}" 2>&1
rec_rc=$?
set -e
echo "INFO: record exit=${rec_rc} log=${rec_log}"

grep -q "\[GAME_PROFILE\] selected '${PROFILE}'" "${rec_log}" \
	|| { echo "ERROR: missing GAME_PROFILE"; tail -30 "${rec_log}"; exit 1; }
grep -q '\[SMOKE\] CLI skirmish start' "${rec_log}" \
	|| { echo "ERROR: skirmish did not start"; tail -40 "${rec_log}"; exit 1; }
grep -q '\[SMOKE\] quitAfterFrames reached' "${rec_log}" \
	|| { echo "ERROR: quitAfterFrames not reached"; tail -40 "${rec_log}"; exit 1; }

# Locate recorded .rep under XDG (path uses Replays/ with OS separators)
mapfile -t reps < <(find "${xdg}" -type f -name '*.rep' 2>/dev/null | sort)
if [[ "${#reps[@]}" -eq 0 ]]; then
	echo "ERROR: no .rep written under ${xdg}"
	find "${xdg}" -type d | head -40 || true
	tail -60 "${rec_log}" || true
	exit 1
fi
src_rep="${reps[-1]}"
dest_rep="${FIXTURE_DIR}/echelon_skirmish_${STAMP}.rep"
cp -f "${src_rep}" "${dest_rep}"
echo "INFO: recorded ${src_rep} -> ${dest_rep}"

# Playback must CRC-match (same binary that recorded)
play_log="${LOG_DIR}/smoke-echelon-playback-${PROFILE}-${STAMP}.out"
echo "INFO: playback ${dest_rep}"
set +e
XDG_DATA_HOME="$(mktemp -d "/tmp/echelon-play-${PROFILE}.XXXXXX")" timeout "${TIMEOUT_SEC}" "${BINARY}" \
	-game "${PROFILE}" \
	-headless \
	-replay "${dest_rep}" \
	>"${play_log}" 2>&1
play_rc=$?
set -e
echo "INFO: playback exit=${play_rc} log=${play_log}"

if grep -q 'REPLAY_CRC_MISMATCH\|CRC Mismatch' "${play_log}"; then
	echo "ERROR: self-recorded replay CRC mismatched"
	grep -E 'REPLAY_CRC_MISMATCH|CRC Mismatch' "${play_log}" | head -5
	exit 1
fi
if [[ "${play_rc}" -ne 0 ]]; then
	echo "ERROR: playback failed exit=${play_rc}"
	tail -40 "${play_log}" || true
	exit "${play_rc}"
fi

echo "OK: record+playback CRC-clean profile=${PROFILE} rep=${dest_rep}"
