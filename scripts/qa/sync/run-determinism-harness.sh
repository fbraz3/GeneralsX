#!/bin/bash
# GeneralsX Determinism Harness
# Runs the game with a specified replay file to capture sync logs for diffing.

if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_replay_file>"
    exit 1
fi

CURRENT_DIR="$(pwd)"

REPLAY_FILE="$1"
LOG_DIR="${CURRENT_DIR}/logs/sync_harness"
mkdir -p "$LOG_DIR"

echo "Running determinism harness on replay: $REPLAY_FILE"

# Run the game in a controlled, quickstart mode to force replay playback
# and generate SyncCrash or crc logs.
cd ~/GeneralsX/GeneralsZH && \
./run.sh -win -xres 800 -yres 600 -quickstart -replay "$REPLAY_FILE" 2>&1 | tee "$LOG_DIR/harness_run.log"

echo "Harness run complete. Check $LOG_DIR and the generated SyncCrash files."
echo "Compare the SyncCrash outputs between your Mac ARM64 build and Linux x64 build to find the exact diverging frame."
