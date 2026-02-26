#!/bin/bash
# Script to test ccache effectiveness

set -e

PROJECT_DIR="/Users/felipebraz/PhpstormProjects/pessoal/GeneralsX"
BUILD_DIR="${PROJECT_DIR}/build/macos-vulkan"

echo "========================================"
echo "CCACHE TEST - GeneralsX Project"
echo "========================================"
echo ""

# Clear ccache stats
echo "[1/5] Clearing ccache statistics..."
ccache -z
echo ""

# Check initial state
echo "[2/5] Initial ccache state:"
ccache -s | head -10
echo ""

# First compilation
echo "[3/5] FIRST compilation of z_ww3d2..."
time cmake --build "${BUILD_DIR}" --target z_ww3d2 2>&1 | tail -5
echo ""

# Check stats after first compilation
echo "[4/5] Stats after first compilation:"
ccache -s | head -15
echo ""

# Second compilation - should use cache
echo "[5/5] SECOND compilation of z_ww3d2 (should use cache!)..."
time cmake --build "${BUILD_DIR}" --target z_ww3d2 2>&1 | tail -5
echo ""

# Final stats
echo "========================================"
echo "FINAL CCACHE STATS:"
echo "========================================"
ccache -s | head -20
echo ""
echo "If Hits is high (>50%), ccache is working!"
echo "If it's low, we need to investigate the issue..."
