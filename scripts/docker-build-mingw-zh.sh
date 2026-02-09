#!/usr/bin/env bash
# Build GeneralsXZH for Windows using MinGW cross-compiler in Docker
# Usage: ./scripts/docker-build-mingw-zh.sh [preset]

set -e

PRESET="${1:-mingw-w64-i686}"
LOG_FILE="logs/build_zh_${PRESET}_docker.log"

echo "🐳 Building GeneralsXZH (Windows/MinGW, preset: ${PRESET})..."
mkdir -p logs

docker run --rm \
    -v "$PWD:/work" \
    -w /work \
    ubuntu:22.04 \
    bash -c "
        set -e
        echo '📦 Installing dependencies (MinGW cross-compiler)...'
        apt-get update -qq
        apt-get install -y -qq build-essential ninja-build git mingw-w64 curl
        
        echo '📦 Installing CMake 3.25+ (required for CMakePresets.json v6)...'
        curl -sL https://github.com/Kitware/CMake/releases/download/v3.25.0/cmake-3.25.0-linux-aarch64.tar.gz | tar -xz -C /usr/local --strip-components=1
        
        echo '⚙️  Configuring CMake (MinGW cross-compile)...'
        cmake --preset ${PRESET}
        
        echo '🔨 Building GeneralsXZH (Windows .exe)...'
        cmake --build build/${PRESET} --target z_generals
        
        echo '✅ Build complete!'
        ls -lh build/${PRESET}/GeneralsMD/GeneralsXZH.exe || echo '⚠️  Binary not found'
    " 2>&1 | tee "$LOG_FILE"

echo "✅ Build complete. Log: $LOG_FILE"
echo "ℹ️  To test: Run in Windows VM or Wine"
