#!/usr/bin/env bash
# Run the native libdatachannel transport against a real Playwright browser.
#
# Usage:
#   scripts/qa/smoke/native-browser-webrtc-interop.sh direct [BUILD_DIR]
#   TURN_KEY_ID=... TURN_KEY_API_TOKEN=... \
#     scripts/qa/smoke/native-browser-webrtc-interop.sh turn [BUILD_DIR]
#
# Environment:
#   STATIC_PORT       Static harness port (default: 18765)
#   SIGNALING_PORT    Local Worker port (default: 18787)

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
MODE="${1:-direct}"
BUILD_DIR="${2:-${ROOT}/build/macos-webrtc-determinism}"
STATIC_PORT="${STATIC_PORT:-18765}"
SIGNALING_PORT="${SIGNALING_PORT:-18787}"
LOG_DIR="${ROOT}/logs/wasm-determinism/native-browser-${MODE}"
mkdir -p "${LOG_DIR}"

if [[ "${MODE}" != "direct" && "${MODE}" != "turn" ]]; then
  echo "ERROR: mode must be direct or turn" >&2
  exit 2
fi
if [[ "${MODE}" == "turn" && ( -z "${TURN_KEY_ID:-}" || -z "${TURN_KEY_API_TOKEN:-}" ) ]]; then
  echo "ERROR: TURN_KEY_ID and TURN_KEY_API_TOKEN are required for turn mode" >&2
  exit 3
fi
python3 - "${STATIC_PORT}" "${SIGNALING_PORT}" <<'PY'
import socket
import sys

for port in map(int, sys.argv[1:]):
    with socket.socket() as listener:
        try:
            listener.bind(("127.0.0.1", port))
        except OSError as error:
            raise SystemExit(f"ERROR: localhost port {port} is unavailable: {error}")
PY

PROBE="$(find "${BUILD_DIR}" -type f -name core_native_webrtc_integration_probe -perm -111 -print -quit)"
if [[ -z "${PROBE}" ]]; then
  echo "ERROR: build core_native_webrtc_integration_probe in ${BUILD_DIR} first" >&2
  exit 4
fi
PROFILE="${BUILD_DIR}/GeneralsXZH.compatibility.json"
if [[ ! -f "${PROFILE}" ]]; then
  echo "ERROR: missing CMake-generated compatibility profile ${PROFILE}" >&2
  exit 4
fi
read -r ENGINE_COMPAT PROTOCOL_COMPAT DETERMINISM_COMPAT CONTENT_MISMATCH DETERMINISM_MISMATCH < <(
  python3 - "${PROFILE}" "${ROOT}/web/packages/shared/src/lockstep-compatibility.json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    current = json.load(stream)
with open(sys.argv[2], encoding="utf-8") as stream:
    versions = json.load(stream)
print(
    current["engine"],
    current["protocol"],
    current["determinism"],
    versions["engine"]["generals"]
        if current["engine"] == versions["engine"]["zeroHour"]
        else versions["engine"]["zeroHour"],
    versions["determinism"]["platformMath"]
        if current["determinism"] == versions["determinism"]["gameMath"]
        else versions["determinism"]["gameMath"],
)
PY
)

ROOM="N$(printf '%06X' "$$")"
STATIC_PID=""
WORKER_PID=""
NATIVE_PID=""
ENV_FILE=""

cleanup() {
  local status=$?
  if [[ -n "${NATIVE_PID}" ]] && kill -0 "${NATIVE_PID}" 2>/dev/null; then kill "${NATIVE_PID}" 2>/dev/null || true; fi
  if [[ -n "${WORKER_PID}" ]] && kill -0 "${WORKER_PID}" 2>/dev/null; then kill "${WORKER_PID}" 2>/dev/null || true; fi
  if [[ -n "${STATIC_PID}" ]] && kill -0 "${STATIC_PID}" 2>/dev/null; then kill "${STATIC_PID}" 2>/dev/null || true; fi
  if [[ -n "${ENV_FILE}" ]]; then rm -f "${ENV_FILE}"; fi
  exit "${status}"
}
trap cleanup EXIT INT TERM

python3 -m http.server "${STATIC_PORT}" --bind 127.0.0.1 --directory "${ROOT}" \
  >"${LOG_DIR}/static-server.log" 2>&1 &
STATIC_PID=$!

WRANGLER_ARGS=(
  dev --local --ip 127.0.0.1 --port "${SIGNALING_PORT}"
  --persist-to "${LOG_DIR}/worker-state"
  --var "ALLOWED_ORIGINS:http://127.0.0.1:${STATIC_PORT}"
  --var "SIGNALING_ORIGIN:http://127.0.0.1:${SIGNALING_PORT}"
  --var "ASSET_ORIGIN:http://127.0.0.1:${STATIC_PORT}"
  --log-level warn
)
if [[ "${MODE}" == "turn" ]]; then
  ENV_FILE="${LOG_DIR}/turn.env"
  umask 077
  {
    printf 'TURN_KEY_ID=%s\n' "${TURN_KEY_ID}"
    printf 'TURN_KEY_API_TOKEN=%s\n' "${TURN_KEY_API_TOKEN}"
  } >"${ENV_FILE}"
  WRANGLER_ARGS+=(--env-file "${ENV_FILE}")
fi
(
  cd "${ROOT}/web/apps/worker"
  exec env -u TURN_KEY_ID -u TURN_KEY_API_TOKEN \
    "${ROOT}/web/node_modules/.bin/wrangler" "${WRANGLER_ARGS[@]}"
) >"${LOG_DIR}/worker.log" 2>&1 &
WORKER_PID=$!

for _ in {1..60}; do
  if ! kill -0 "${STATIC_PID}" 2>/dev/null || ! kill -0 "${WORKER_PID}" 2>/dev/null; then
    echo "ERROR: local interoperability service exited during startup" >&2
    cat "${LOG_DIR}/static-server.log" "${LOG_DIR}/worker.log" >&2
    exit 5
  fi
  if curl --silent --output /dev/null "http://127.0.0.1:${STATIC_PORT}/wasm/native_browser_interop.html" &&
     curl --silent --output /dev/null "http://127.0.0.1:${SIGNALING_PORT}/not-found"; then
    break
  fi
  sleep 0.25
done
kill -0 "${STATIC_PID}"
kill -0 "${WORKER_PID}"
curl --fail --silent --output /dev/null "http://127.0.0.1:${STATIC_PORT}/wasm/native_browser_interop.html"

NATIVE_ENV=(
  "GENERALSX_WEBRTC=1"
  "GENERALSX_WEBRTC_SIGNALING_URL=http://127.0.0.1:${SIGNALING_PORT}"
  "GENERALSX_WEBRTC_ROOM=${ROOM}"
  "GENERALSX_WEBRTC_PLAYER_NAME=playwright-native"
  "GENERALSX_WEBRTC_CAPACITY=2"
)
if [[ "${MODE}" == "direct" ]]; then
  NATIVE_ENV+=("GENERALSX_WEBRTC_DISABLE_TURN=1")
else
  NATIVE_ENV+=("GENERALSX_WEBRTC_FORCE_RELAY=1")
fi
env -u TURN_KEY_ID -u TURN_KEY_API_TOKEN \
  "${NATIVE_ENV[@]}" "${PROBE}" -webrtc >"${LOG_DIR}/native.log" 2>&1 &
NATIVE_PID=$!

env -u TURN_KEY_ID -u TURN_KEY_API_TOKEN \
  node "${ROOT}/web/apps/launcher/e2e/native-browser-interop.mjs" \
  "${MODE}" "${ROOM}" "http://127.0.0.1:${STATIC_PORT}" "http://127.0.0.1:${SIGNALING_PORT}" \
  "${ENGINE_COMPAT}" "${PROTOCOL_COMPAT}" "${DETERMINISM_COMPAT}" \
  "${CONTENT_MISMATCH}" "${DETERMINISM_MISMATCH}" \
  | tee "${LOG_DIR}/browser.log"

wait "${NATIVE_PID}"
NATIVE_PID=""
grep -q "INTEROP_OK" "${LOG_DIR}/native.log"
cat "${LOG_DIR}/native.log"
