#!/usr/bin/env bash
# Build GeneralsXZH (Zero Hour) for Linux using Docker
# Usage: ./scripts/docker-build-linux-zh.sh [preset]

set -e

PRESET="${1:-linux64-deploy}"
LOG_FILE="logs/build_zh_${PRESET}_docker.log"

echo "🐳 Building GeneralsXZH (Linux, preset: ${PRESET})..."
mkdir -p logs

docker run --rm \
    -v "$PWD:/work" \
    -w /work \
    ubuntu:22.04 \
    bash -c "
        set -e
        echo '📦 Installing dependencies...'
        apt-get update -qq
        apt-get install -y -qq build-essential ninja-build git curl
        
        echo '📦 Installing CMake 3.25+ (required for CMakePresets.json v6)...'
        curl -sL https://github.com/Kitware/CMake/releases/download/v3.25.0/cmake-3.25.0-linux-aarch64.tar.gz | tar -xz -C /usr/local --strip-components=1
        cmake --version
        
        echo '⚙️  Configuring CMake (if needed)...'
        cmake --preset ${PRESET}
        
        echo '🔨 Building GeneralsXZH...'
        cmake --build build/${PRESET} --target z_generals
        
        echo '✅ Build complete!'
        ls -lh build/${PRESET}/GeneralsMD/GeneralsXZH || echo '⚠️  Binary not found'
    " 2>&1 | tee "$LOG_FILE"

echo "✅ Build complete. Log: $LOG_FILE"
