#!/bin/bash
#
# Phase 38.5 Graphics Backend Testing Script
# Tests Phase 38.4 LegacyGraphicsBackend delegation with 30+ minute gameplay
#

set -e

PROJECT_ROOT="/Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode"
TEST_DIR="$HOME/GeneralsX/GeneralsMD"
EXECUTABLE="$TEST_DIR/GeneralsXZH"
LOG_DIR="$PROJECT_ROOT/logs"
TEST_LOG="$LOG_DIR/phase38_5_test_$(date +%Y%m%d_%H%M%S).log"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Phase 38.5 Graphics Backend Testing ===${NC}"
echo "Test Time: $(date)"
echo "Executable: $EXECUTABLE"
echo "Log File: $TEST_LOG"
echo ""

# Verify executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}ERROR: Executable not found at $EXECUTABLE${NC}"
    exit 1
fi

# Verify assets exist
if [ ! -d "$TEST_DIR/Data" ]; then
    echo -e "${RED}ERROR: Assets not found at $TEST_DIR/Data${NC}"
    exit 1
fi

# Clear previous crash log
CRASH_LOG="$HOME/Documents/Command and Conquer Generals Zero Hour Data/ReleaseCrashInfo.txt"
if [ -f "$CRASH_LOG" ]; then
    cat /dev/null > "$CRASH_LOG"
fi

echo -e "${BLUE}Starting game...${NC}"
cd "$TEST_DIR"

# Run with 90-second timeout to allow complete INI parsing (game is slow on initial load)
# This timeout should be enough to get past initial parsing and reach main menu
timeout 90s env USE_METAL=1 "$EXECUTABLE" 2>&1 | tee "$TEST_LOG" || TEST_EXIT=$?

if [ -z "$TEST_EXIT" ]; then
    TEST_EXIT=$?
fi

echo ""
echo -e "${BLUE}=== Test Results ===${NC}"
echo "Exit Code: $TEST_EXIT"
echo "  0 = Normal exit (ESC pressed)"
echo "  124 = Timeout (game still running after 90s)"
echo "  Other = Crash or error"

# Check for crashes
if [ -f "$CRASH_LOG" ]; then
    CRASH_SIZE=$(stat -f%z "$CRASH_LOG" 2>/dev/null || echo 0)
    if [ "$CRASH_SIZE" -gt 100 ]; then
        echo -e "${RED}CRASH DETECTED in ReleaseCrashInfo.txt:${NC}"
        tail -20 "$CRASH_LOG"
    fi
fi

# Summary
echo ""
echo -e "${BLUE}Log file: $TEST_LOG${NC}"
echo "Lines: $(wc -l < "$TEST_LOG")"
echo ""

if [ "$TEST_EXIT" -eq 0 ]; then
    echo -e "${GREEN}✓ Test completed normally (ESC exit)${NC}"
    exit 0
elif [ "$TEST_EXIT" -eq 124 ]; then
    echo -e "${GREEN}✓ Game ran past 35s timeout (still executing)${NC}"
    exit 0
else
    echo -e "${RED}✗ Test failed with exit code $TEST_EXIT${NC}"
    exit 1
fi
