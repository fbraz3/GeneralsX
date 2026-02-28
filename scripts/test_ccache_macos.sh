#!/bin/bash
# GeneralsX @build BenderAI 27/02/2026
# Test ccache effectiveness on macOS
# Compares build times and cache statistics before/after cache warm-up

set -e

PRESET="${1:-macos-vulkan}"
TARGET="${2:-z_ww3d2}"
JOBS="${3:-4}"

echo "========================================"
echo "TEST: ccache Effectiveness (macOS)"
echo "========================================"
echo ""

# Check ccache installed
if ! command -v ccache &> /dev/null; then
    echo "ERROR: ccache not found"
    echo "Install with: brew install ccache"
    exit 1
fi

# Create log directory
mkdir -p logs

echo "[1] Initial ccache state:"
ccache -s | head -10
echo ""

# Check build directory
if [ ! -d "build/$PRESET" ]; then
    echo "ERROR: Build directory not found: build/$PRESET"
    echo "Run first: cmake --preset $PRESET"
    exit 1
fi

echo "[2] Clearing cache for clean test..."
ccache -z
echo "✓ Cache cleared"
echo ""

echo "[3] FIRST BUILD (cold cache - NOT cached)..."
START_TIME=$(date +%s)
if ! cmake --build "build/$PRESET" --target $TARGET -j $JOBS 2>&1 | tee "logs/cache-test-first-build.log"; then
    echo "ERROR: First build failed"
    exit 1
fi
END_TIME=$(date +%s)
FIRST_TIME=$((END_TIME - START_TIME))

echo ""
echo "First build time: ${FIRST_TIME}s"
echo ""

echo "[4] ccache state after first build:"
ccache -s | head -15
echo ""

echo "[5] Clean build (remove intermediate files but keep cache)..."
cmake --build "build/$PRESET" --target $TARGET --clean-first 2>&1 | tee "logs/cache-test-clean.log" > /dev/null
echo ""

echo "[6] SECOND BUILD (warm cache - should be faster!)..."
START_TIME=$(date +%s)
if ! cmake --build "build/$PRESET" --target $TARGET -j $JOBS 2>&1 | tee "logs/cache-test-second-build.log"; then
    echo "ERROR: Second build failed"
    exit 1
fi
END_TIME=$(date +%s)
SECOND_TIME=$((END_TIME - START_TIME))

echo ""
echo "Second build time: ${SECOND_TIME}s"
echo ""

echo "[7] Final ccache state:"
ccache -s | head -15
echo ""

# Calculate speedup
echo "========================================"
echo "RESULTS"
echo "========================================"
echo "First build:  ${FIRST_TIME}s"
echo "Second build: ${SECOND_TIME}s"

if [ $SECOND_TIME -gt 0 ]; then
    SPEEDUP=$(echo "scale=2; $FIRST_TIME / $SECOND_TIME" | bc)
    TIME_SAVED=$(($FIRST_TIME - $SECOND_TIME))
    
    echo "Speedup:      ${SPEEDUP}x faster"
    echo "Time saved:   ${TIME_SAVED}s per build"
    echo ""
    
    if (( $(echo "$SPEEDUP >= 2.0" | bc -l) )); then
        echo "✓ Cache is working WELL (2x+ speedup)"
    elif (( $(echo "$SPEEDUP >= 1.5" | bc -l) )); then
        echo "⚠ Cache is working but could be better (1.5x speedup)"
    else
        echo "✗ Cache effectiveness is low (<1.5x speedup)"
        echo "Check: sloppiness=time_macros,locale is configured"
        echo "  ccache -p | grep sloppiness"
    fi
else
    echo "SECOND_TIME is 0, cannot calculate speedup"
fi

echo ""
echo "Logs saved to: logs/cache-test-*.log"
