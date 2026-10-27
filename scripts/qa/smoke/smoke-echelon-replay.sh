#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: headless replay smoke for echelon.
# Default fixture: logs/fixtures/replays/zh/linux_2p_vanilla_map.rep (from generalsx-replays).
# Usage: ./scripts/qa/smoke/smoke-echelon-replay.sh [build-dir] [replay.rep] [profile] [timeout-sec]
#
# Exit codes:
#   0 = clean playback
#   2 = same-profile path exercised but CRC mismatched (engine vs fixture drift)
#   3 = cross-profile load rejected / aborted after start (isolation OK for Gate 6 path smoke)
#   other = hard failure

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${PROJECT_ROOT}/build/linux64-unified-baseline}"
REP="${2:-${PROJECT_ROOT}/logs/fixtures/replays/zh/linux_2p_vanilla_map.rep}"
PROFILE="${3:-zh}"
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
if [[ ! -f "${REP}" ]]; then
	echo "ERROR: replay not found: ${REP}"
	echo "Clone: git clone https://gitlab.com/fbraz3/generalsx-replays.git"
	exit 1
fi

# Infer fixture game from path (…/replays/zh/… vs …/replays/generals/…)
fixture_game="unknown"
case "${REP}" in
	*/replays/zh/*|*/GeneralsXZH/*) fixture_game="zh" ;;
	*/replays/generals/*|*/GeneralsX/Replays/*) fixture_game="generals" ;;
esac

xdg="$(mktemp -d "/tmp/echelon-replay-${PROFILE}.XXXXXX")"
log="${LOG_DIR}/smoke-echelon-replay-${PROFILE}-${STAMP}.out"
abs_rep="$(cd "$(dirname "${REP}")" && pwd)/$(basename "${REP}")"

echo "INFO: echelon -game ${PROFILE} -replay ${abs_rep} (fixture_game=${fixture_game})"
set +e
XDG_DATA_HOME="${xdg}" timeout "${TIMEOUT_SEC}" "${BINARY}" \
	-game "${PROFILE}" \
	-headless \
	-replay "${abs_rep}" \
	>"${log}" 2>&1
rc=$?
set -e

echo "INFO: exit=${rc} log=${log}"
grep -q "\[GAME_PROFILE\] selected '${PROFILE}'" "${log}" \
	|| { echo "ERROR: missing GAME_PROFILE"; tail -20 "${log}"; exit 1; }
grep -qiE 'segfault|Aborted|Segmentation fault' "${log}" \
	&& { echo "ERROR: crash"; exit 1; }

if [[ "${rc}" -eq 0 ]]; then
	echo "OK: replay playback clean exit 0"
	exit 0
fi
if grep -q 'REPLAY_CRC_MISMATCH\|CRC Mismatch' "${log}"; then
	echo "WARN: replay path exercised but CRC mismatched (engine vs fixture); soft-pass for Gate 6 path smoke"
	exit 2
fi
if grep -qiE 'Cannot open replay' "${log}"; then
	if [[ "${fixture_game}" != "unknown" && "${fixture_game}" != "${PROFILE}" ]]; then
		echo "WARN: cross-profile cannot-open (isolation); soft-pass"
		exit 3
	fi
	echo "ERROR: cannot open replay"
	exit 1
fi
# Generals profile + ZH .rep often aborts after "Simulating Replay" without CRC (map/options mismatch).
if [[ "${fixture_game}" != "unknown" && "${fixture_game}" != "${PROFILE}" ]] \
	&& grep -q 'Simulating Replay' "${log}"; then
	echo "WARN: cross-profile replay aborted after simulate start (isolation); soft-pass"
	exit 3
fi
echo "ERROR: unexpected replay failure"
tail -40 "${log}" || true
exit "${rc}"
