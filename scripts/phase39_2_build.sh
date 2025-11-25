#!/bin/bash
# Phase 39.2 Build Script

set -e

cd /Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode

echo "=== Phase 39.2: Build Validation ==="
echo "Starting clean build..."

# Configure
cmake --preset macos

# Build with tee to logs
cmake --build build/macos -j 4 2>&1 | tee logs/phase39_2_build_full.log

# Extract errors
echo ""
echo "=== Build Errors Summary ==="
grep -E "error:" logs/phase39_2_build_full.log || echo "No errors found!"

echo ""
echo "=== Build Warnings Summary ==="
grep -c "warning:" logs/phase39_2_build_full.log && echo "Warnings detected" || echo "No warnings"

echo ""
echo "Build complete."
