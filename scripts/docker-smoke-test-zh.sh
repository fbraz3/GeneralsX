#!/usr/bin/env bash
# Smoke test GeneralsXZH Linux binary (launch and check output)
# Usage: ./scripts/docker-smoke-test-zh.sh [preset]

set -e

PRESET="${1:-linux64-deploy}"
BINARY="build/${PRESET}/GeneralsMD/GeneralsXZH"
LOG_FILE="logs/smoke_test_zh_${PRESET}.log"

echo "🧪 Smoke testing GeneralsXZH (Linux)..."
mkdir -p logs

if [ ! -f "$BINARY" ]; then
    echo "❌ Binary not found: $BINARY"
    echo "Run: ./scripts/docker-build-linux-zh.sh first"
    exit 1
fi

echo "ℹ️  Binary found: $BINARY"
echo "ℹ️  Note: This will likely fail (needs game assets, libraries, etc.)"
echo "ℹ️  Goal: Check initialization output and crash logs"
echo ""

docker run --rm \
    -v "$PWD:/work" \
    -w /work \
    ubuntu:22.04 \
    bash -c "
        echo '📦 Installing runtime dependencies...'
        apt-get update -qq
        apt-get install -y -qq lib64stdc++6 curl
        
        echo '🚀 Launching GeneralsXZH...'
        echo '   (Expect crash, we want to see initialization output)'
        echo ''
        
        ${BINARY} -win || echo '⚠️  Crashed (expected for now)'
    " 2>&1 | tee "$LOG_FILE"

echo ""
echo "✅ Smoke test complete. Log: $LOG_FILE"
echo "ℹ️  Check log for:"
echo "   - SDL3/OpenAL initialization messages"
echo "   - Missing library errors (libdxvk_d3d8.so, etc.)"
echo "   - Crash location (helps debug)"
