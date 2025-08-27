#!/bin/bash

# Convenience script to run OpenGL tests from project root
# This script forwards to the actual test runner in tests/opengl/

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_SCRIPT="$SCRIPT_DIR/tests/opengl/run_opengl_tests.sh"

if [ -f "$TEST_SCRIPT" ]; then
    echo "🔀 Running OpenGL tests from: $TEST_SCRIPT"
    echo ""
    exec "$TEST_SCRIPT" "$@"
else
    echo "❌ Error: Test script not found at $TEST_SCRIPT"
    echo "   Make sure you're running this from the project root directory."
    exit 1
fi
