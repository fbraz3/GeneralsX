#!/usr/bin/env bash
# Validate replay files in batch using headless playback and classify outcomes.
#
# Usage:
#   ./scripts/qa/debug/validate-replays-headless.sh [REPLAY_DIR] [RUNTIME_DIR]
#
# Environment:
#   TIMEOUT_SEC   Timeout per replay execution (default: 120)
#   OUTPUT_DIR    Directory where logs/reports are stored (default: logs/replay_validation)
#
# Exit codes:
#   0 if at least one replay is playable
#   1 if no replay files were found or none were playable
#   2 for setup/runtime errors

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

REPLAY_DIR="${1:-$HOME/Library/Application Support/GeneralsX/GeneralsZH/Replays}"
RUNTIME_DIR="${2:-$HOME/GeneralsX/GeneralsZH}"
BINARY_PATH="$RUNTIME_DIR/GeneralsXZH"
TIMEOUT_SEC="${TIMEOUT_SEC:-120}"
OUTPUT_BASE_DIR="${OUTPUT_DIR:-$PROJECT_ROOT/logs/replay_validation}"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
RUN_OUTPUT_DIR="$OUTPUT_BASE_DIR/$TIMESTAMP"
REPORT_PATH="$RUN_OUTPUT_DIR/report.txt"

if [[ ! -d "$REPLAY_DIR" ]]; then
  echo "ERROR: Replay directory not found: $REPLAY_DIR" >&2
  exit 2
fi

if [[ ! -x "$BINARY_PATH" ]]; then
  echo "ERROR: Binary not found or not executable: $BINARY_PATH" >&2
  exit 2
fi

mkdir -p "$RUN_OUTPUT_DIR"

echo "Replay validation started"
echo "Replay dir: $REPLAY_DIR"
echo "Runtime dir: $RUNTIME_DIR"
echo "Binary: $BINARY_PATH"
echo "Timeout per replay: ${TIMEOUT_SEC}s"
echo "Output dir: $RUN_OUTPUT_DIR"
echo

replay_count=0
playable_count=0

# shellcheck disable=SC2012
while IFS= read -r replay_path; do
  replay_count=$((replay_count + 1))
  replay_file="$(basename "$replay_path")"
  safe_name="$(printf '%s' "$replay_file" | tr ' /' '__')"
  replay_log="$RUN_OUTPUT_DIR/${safe_name}.log"

  echo "[$replay_count] Testing: $replay_file"

  (
    cd "$RUNTIME_DIR"
    timeout "$TIMEOUT_SEC" "$BINARY_PATH" -headless -replay "$replay_path" > "$replay_log" 2>&1
  ) || true

  status="UNKNOWN"
  detail=""

  if rg -q "Replay header parse failed" "$replay_log"; then
    status="INVALID_HEADER"
    detail="header parse failed"
  elif rg -q "Replay header open failed|Cannot open replay" "$replay_log"; then
    status="INVALID_OPEN"
    detail="open failed"
  elif rg -q "CRC Mismatch in Frame" "$replay_log"; then
    status="PLAYABLE_CRC_MISMATCH"
    frame_line="$(rg -m1 "CRC Mismatch in Frame" "$replay_log" || true)"
    detail="$frame_line"
    playable_count=$((playable_count + 1))
  elif rg -q "Exiting with code 0" "$replay_log"; then
    status="PLAYABLE_EXIT0"
    detail="clean exit"
    playable_count=$((playable_count + 1))
  else
    status="UNKNOWN"
    detail="no known replay marker"
  fi

  printf '%-36s | %-22s | %s\n' "$replay_file" "$status" "$detail" | tee -a "$REPORT_PATH"
done < <(find "$REPLAY_DIR" -maxdepth 1 -type f -name '*.rep' | sort)

if [[ "$replay_count" -eq 0 ]]; then
  echo "ERROR: No .rep files found in $REPLAY_DIR" >&2
  exit 1
fi

echo
echo "Summary: playable=$playable_count total=$replay_count"
echo "Report: $REPORT_PATH"

if [[ "$playable_count" -gt 0 ]]; then
  exit 0
fi

exit 1
