#!/bin/bash

# Phase 47 Test Execution Script
# Runs all tests and generates reports

set -e

PROJECT_ROOT="/Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode"
BUILD_DIR="$PROJECT_ROOT/build/macos-arm64"
TEST_EXECUTABLE="$BUILD_DIR/phase47_tests"
LOG_DIR="$PROJECT_ROOT/logs"

echo "═══════════════════════════════════════════════════════════"
echo "Phase 47: Test Execution"
echo "═══════════════════════════════════════════════════════════"

# Create logs directory if it doesn't exist
mkdir -p "$LOG_DIR"

# Ensure build exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory not found at $BUILD_DIR"
    echo "Running CMake configuration..."
    cd "$PROJECT_ROOT"
    cmake --preset macos-arm64
fi

# Build test target
echo ""
echo "Building test infrastructure..."
cd "$PROJECT_ROOT"
cmake --build "$BUILD_DIR" --target phase47_tests -j 4 2>&1 | tee "$LOG_DIR/phase47_build.log"

if [ $? -ne 0 ]; then
    echo "Build failed! Check $LOG_DIR/phase47_build.log"
    exit 1
fi

# Run tests
echo ""
echo "Running tests..."
"$TEST_EXECUTABLE" 2>&1 | tee "$LOG_DIR/phase47_tests.log"

TEST_RESULT=$?

# Print summary
echo ""
echo "═══════════════════════════════════════════════════════════"
echo "Test Execution Complete"
echo "═══════════════════════════════════════════════════════════"
echo "Logs available in: $LOG_DIR/"
echo ""

if [ $TEST_RESULT -eq 0 ]; then
    echo "✅ All tests passed!"
else
    echo "❌ Some tests failed. Check logs for details."
fi

exit $TEST_RESULT
