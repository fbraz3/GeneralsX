#!/bin/bash

##############################################################################
# Phase 49: Robust File-Based Logging Test Script
# 
# This script runs GeneralsX with file-based logging to trace initialization
# and pinpoint where execution hangs or freezes.
#
# Usage:
#   ./run_with_logging.sh [timeout_seconds]
#
# The log file will be written to: /tmp/phase49_crash.log
##############################################################################

set -e

# Default timeout is 30 seconds
TIMEOUT=${1:-30}

echo "=========================================="
echo "Phase 49: Initialization Hang Debugging"
echo "=========================================="
echo ""
echo "Starting GeneralsX with file-based logging..."
echo "Log file: /tmp/phase49_crash.log"
echo "Timeout: ${TIMEOUT} seconds"
echo ""

# Clear old log file
rm -f /tmp/phase49_crash.log

# Change to the GeneralsX directory
cd $HOME/GeneralsX/GeneralsMD || {
    echo "ERROR: Cannot find $HOME/GeneralsX/GeneralsMD"
    exit 1
}

# Copy the executable if it exists in the build directory
if [ -f "/Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode/build/macos-arm64/GeneralsMD/GeneralsXZH" ]; then
    echo "Copying fresh GeneralsXZH executable..."
    cp "/Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode/build/macos-arm64/GeneralsMD/GeneralsXZH" ./
fi

echo "Running: timeout ${TIMEOUT} ./GeneralsXZH 2>&1"
echo ""

# Run with timeout
timeout ${TIMEOUT} bash -c 'USE_METAL=0 ./GeneralsXZH 2>&1' || true

echo ""
echo "=========================================="
echo "Execution completed (may be timeout)"
echo "=========================================="
echo ""

# Display the log file
if [ -f /tmp/phase49_crash.log ]; then
    echo "Log file contents:"
    echo "=========================================="
    cat /tmp/phase49_crash.log
    echo ""
    echo "=========================================="
    echo ""
    echo "Last 20 lines of log (to spot where it hangs):"
    echo "=========================================="
    tail -20 /tmp/phase49_crash.log
    echo "=========================================="
else
    echo "ERROR: Log file not created!"
    exit 1
fi

echo ""
echo "Analysis:"
echo "- If log ends abruptly, execution likely hangs at that point"
echo "- Look for ERROR markers in the log"
echo "- Check which CHECKPOINT is the last one reached"
echo ""
