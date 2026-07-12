#!/bin/bash
# GeneralsX @test AI 11/07/2026 Run unit tests script
# This script configures the project for testing, builds the test executable, and runs the tests.

set -e

WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"

if [[ "$OSTYPE" == "darwin"* ]]; then
    PRESET="macos-vulkan"
else
    PRESET="linux64-deploy"
fi

echo "--- Configuring tests (Preset: $PRESET) ---"
cmake --preset $PRESET -DRTS_BUILD_TESTS=ON -DRTS_BUILD_COVERAGE=ON

echo "--- Building tests ---"
cmake --build build/$PRESET --target GeneralsXTests

echo "--- Running tests ---"
cd build/$PRESET
ctest --output-on-failure -V

echo "--- Generating coverage reports ---"
# Check if gcovr is installed
if command -v gcovr >/dev/null 2>&1; then
    GCOVR_CMD="gcovr"
elif python3 -m gcovr --version >/dev/null 2>&1; then
    GCOVR_CMD="python3 -m gcovr"
else
    GCOVR_CMD=""
fi

if [ -n "$GCOVR_CMD" ]; then
    mkdir -p coverage
    echo "Generating Cobertura XML report (for CI pipelines)..."
    $GCOVR_CMD -r ../../.. --xml-pretty -o coverage/coverage.xml .
    
    echo "Generating HTML report (for local viewing)..."
    $GCOVR_CMD -r ../../.. --html --html-details -o coverage/coverage.html .
    
    echo "Coverage reports generated in build/$PRESET/coverage/"
else
    echo "WARNING: gcovr is not installed. Skipping coverage report generation."
    echo "To install gcovr, run: pip3 install gcovr"
fi
